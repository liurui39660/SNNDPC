#include "Core.h"

#include <queue>
#include <list>

#include <cuda_runtime.h>
#include <thrust/execution_policy.h>
#include <thrust/sort.h>
#include <thrust/sequence.h>
#include <thrust/set_operations.h>
#include <thrust/binary_search.h>
#include <thrust/reduce.h>
#include <thrust/extrema.h>
#include <thrust/functional.h>

using namespace SNNDPC::GPU;

template<typename T>
Core<T>::Core(int k, int n, int d, int nc, const SNNDPC::Option& option):
	k(k), n(n), d(d), nc(nc), option(option),
	type{std::is_same<T, double>::value ? CUDA_R_64F : (std::is_same<T, float>::value ? CUDA_R_32F : CUDA_R_16F)} {
	cublasCreate(&handle);
}

template<typename T>
Core<T>::~Core() {
	cublasDestroy(handle);
	cudaFree(data);
	cudaFree(distance);
	cudaFree(indexDistanceAsc);
	cudaFree(indexNeighbor);
	cudaFree(indexSharedNeighbor);
	cudaFree(numSharedNeighbor);
	cudaFree(similarity);
	cudaFree(rho);
	cudaFree(delta);
	cudaFree(gamma);
	cudaFree(indexAssignment);
	cudaFree(indexCentroid);
}

template<typename T>
void Core<T>::ComputeDistance() {
	T* dot;
	cudaMallocManaged(&dot, n * sizeof(T));
	for (int i = 0; i < n; i++)
		cublasDotEx(handle, d, data + i * d, type, 1, data + i * d, type, 1, dot + i, type, type);
	T* mult;
	cudaMallocManaged(&mult, n * n * sizeof(T));
	const T One = 1;
	cublasGemmEx(handle, CUBLAS_OP_T, CUBLAS_OP_N, n, n, d, &One, data, type, d, data, type, d, &One, mult, type, n, type, CUBLAS_GEMM_DEFAULT);
	const T MinusTwo = -2;
	cublasScalEx(handle, n * n, &MinusTwo, type, mult, type, 1, type);
	cudaDeviceSynchronize();
	// TODO: Use kernel
	cudaMallocManaged(&distance, n * n * sizeof(T));
	for (int i = 0; i < n; i++) {
		distance[i * n + i] = 0;
		for (int j = 0; j < i; j++)
			distance[j * n + i] = distance[i * n + j] = std::sqrt(dot[i] + dot[j] + mult[i * n + j]);
	}
	cudaFree(dot);
	cudaFree(mult);
}

template<typename T>
void Core<T>::ComputeNeighbor() {
	cudaMallocManaged(&indexDistanceAsc, n * n * sizeof(int));
	cudaMallocManaged(&indexNeighbor, n * k * sizeof(int));
	for (int i = 0; i < n; i++) {
		thrust::sequence(indexDistanceAsc + i * n, indexDistanceAsc + i * n + n);
		thrust::sort(indexDistanceAsc + i * n, indexDistanceAsc + i * n + n, [&](int a, int b) { return distance[i * n + a] < distance[i * n + b]; });
		thrust::copy(indexDistanceAsc + i * n, indexDistanceAsc + i * n + k, indexNeighbor + i * k);
		thrust::sort(indexNeighbor + i * k, indexNeighbor + i * k + k);
	}
}

template<typename T>
void Core<T>::ComputeSharedNeighbor() {
	cudaMallocManaged(&indexSharedNeighbor, n * n * k * sizeof(int));
	cudaMallocManaged(&numSharedNeighbor, n * n * sizeof(int));
	for (int i = 0; i < n; i++) {
		numSharedNeighbor[i * n + i] = 0;
		// TODO: Use triangular matrix
		for (int j = 0; j < i; j++) {
			numSharedNeighbor[j * n + i] = numSharedNeighbor[i * n + j] = thrust::set_intersection(
				indexNeighbor + i * k, indexNeighbor + (i + 1) * k,
				indexNeighbor + j * k, indexNeighbor + (j + 1) * k,
				indexSharedNeighbor + i * n * k + j * k
			) - (indexSharedNeighbor + i * n * k + j * k);
			thrust::copy(
				indexSharedNeighbor + i * n * k + j * k,
				indexSharedNeighbor + i * n * k + j * k + numSharedNeighbor[i * n + j],
				indexSharedNeighbor + j * n * k + i * k
			);
		}
	}
}

template<typename T>
void Core<T>::ComputeSimilarity() {
	cudaMallocManaged(&similarity, n * n * sizeof(T));
	for (int i = 0; i < n; i++) {
		similarity[i * n + i] = 0;
		for (int j = 0; j < i; j++) {
			const auto first = indexSharedNeighbor + i * n * k + j * k;
			const auto last = indexSharedNeighbor + i * n * k + j * k + numSharedNeighbor[i * n + j];
			if (thrust::binary_search(first, last, i) && thrust::binary_search(first, last, j)) {
				T sum = 0;
				for (int u = 0; u < numSharedNeighbor[i * n + j]; u++) {
					const int shared = indexSharedNeighbor[i * n * k + j * k + u];
					sum += distance[i * n + shared] + distance[j * n + shared];
				}
				similarity[j * n + i] = similarity[i * n + j] = pow(numSharedNeighbor[i * n + j], 2) / sum;
			} else {
				similarity[j * n + i] = similarity[i * n + j] = T{0};
			}
		}
	}
}

template<typename T>
void Core<T>::ComputeRho() {
	T* similarityDesc;
	cudaMallocManaged(&similarityDesc, n * sizeof(T));
	cudaMallocManaged(&rho, n * sizeof(T));
	for (int i = 0; i < n; i++) {
		thrust::copy(similarity + i * n, similarity + (i + 1) * n, similarityDesc);
		thrust::sort(similarityDesc, similarityDesc + n, thrust::greater<T>());
		rho[i] = thrust::reduce(similarityDesc, similarityDesc + k, T{0}, thrust::plus<T>());
	}
	cudaFree(similarityDesc);
}

template<typename T>
void Core<T>::ComputeDelta() {
	cudaMallocManaged(&delta, n * sizeof(T));
	thrust::fill(delta, delta + n, std::numeric_limits<T>::infinity());
	T* distanceNeighborSum;
	cudaMallocManaged(&distanceNeighborSum, n * sizeof(T));
	thrust::fill(distanceNeighborSum, distanceNeighborSum + n, T{0});
	for (int i = 0; i < n; i++)
		for (int j = 0; j < k; j++)
			distanceNeighborSum[i] += distance[i * n + indexNeighbor[i * k + j]];
	int* indexRhoDesc;
	cudaMallocManaged(&indexRhoDesc, n * sizeof(int));
	thrust::sequence(indexRhoDesc, indexRhoDesc + n);
	thrust::sort(indexRhoDesc, indexRhoDesc + n, [&](int a, int b) { return rho[a] > rho[b]; });
	for (int i = 1; i < n; i++) {
		int a = indexRhoDesc[i];
		for (int j = 0; j < i; j++) {
			int b = indexRhoDesc[j];
			delta[a] = thrust::min(delta[a], distance[a * n + b] * (distanceNeighborSum[a] + distanceNeighborSum[b]));
		}
	}
	delta[indexRhoDesc[0]] = -std::numeric_limits<T>::infinity();
	delta[indexRhoDesc[0]] = *thrust::max_element(delta, delta + n);
	cudaFree(distanceNeighborSum);
	cudaFree(indexRhoDesc);
}

template<typename T>
void Core<T>::ComputeGamma() {
	cudaMallocManaged(&gamma, n * sizeof(T));
	thrust::transform(rho, rho + n, delta, gamma, thrust::multiplies<T>());
}

template<typename T>
void Core<T>::ComputeCentroid() {
	cudaMallocManaged(&indexAssignment, n * sizeof(int));
	cudaMallocManaged(&indexCentroid, nc * sizeof(int));
	thrust::fill(indexAssignment, indexAssignment + n, Unassigned);
	int* indexGammaDesc;
	cudaMallocManaged(&indexGammaDesc, n * sizeof(int));
	thrust::sequence(indexGammaDesc, indexGammaDesc + n);
	thrust::sort(indexGammaDesc, indexGammaDesc + n, [&](int a, int b) { return gamma[a] > gamma[b]; });
	thrust::copy(indexGammaDesc, indexGammaDesc + nc, indexCentroid);
	thrust::sort(indexCentroid, indexCentroid + nc);
	for (int i = 0; i < nc; i++)
		indexAssignment[indexCentroid[i]] = i;
	cudaFree(indexGammaDesc);
}

template<typename T>
void Core<T>::AssignNonCentroidStep1() {
	std::queue<int> queue;
	for (int i = 0; i < nc; i++)
		queue.push(indexCentroid[i]);
	while (!queue.empty()) {
		int a = queue.front();
		queue.pop();
		for (int i = 0; i < k; i++) {
			int b = indexNeighbor[a * k + i];
			if (indexAssignment[b] == Unassigned && numSharedNeighbor[a * n + b]>= k / 2.0) {
				indexAssignment[b] = indexAssignment[a];
				queue.push(b);
			}
		}
	}
}

template<typename T>
void Core<T>::AssignNonCentroidStep2() {
	int k = this->k;
	std::list<int> indexUnassigned;
	for (int i = 0; i < n; i++)
		if (indexAssignment[i] == Unassigned)
			indexUnassigned.push_back(i);
	int numUnassigned = indexUnassigned.size();
	int* numNeighborAssignment;
	cudaMallocManaged(&numNeighborAssignment, numUnassigned * nc * sizeof(int));
	while (numUnassigned) {
		thrust::fill(numNeighborAssignment, numNeighborAssignment + numUnassigned * nc, 0);
		int i=0;
		for (const auto& a : indexUnassigned){
			for (int j = 0; j < k; j++) {
				int b = indexDistanceAsc[a * n + j];
				if (indexAssignment[b] != Unassigned)
					++numNeighborAssignment[i * nc + indexAssignment[b]];
			}
			i++;
		}
		if (int most = *thrust::max_element(numNeighborAssignment, numNeighborAssignment + numUnassigned * nc)) {
			auto it = indexUnassigned.begin();
			for (int i = 0; i < numUnassigned; i++) {
				auto first = numNeighborAssignment + i * nc;
				auto last = numNeighborAssignment + (i + 1) * nc;
				auto current = thrust::find(first, last, most); // In MATLAB, if multiple hits, the last will be used
				if (current == last) ++it;
				else {
					indexAssignment[*it] = current - first;
					it = indexUnassigned.erase(it);
				}
			}
			numUnassigned = indexUnassigned.size();
		} else {
			++k; // TODO: Replace with custom function
			assert(k < n);
		}
	}
	cudaFree(numNeighborAssignment);
}
