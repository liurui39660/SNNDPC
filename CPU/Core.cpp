#include "Core.h"

#include <memory>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <queue>
#include <list>
#include <cassert>

using namespace std;
using namespace SNNDPC::CPU;

#ifdef PARALLEL_PROVIDER_IntelTBB
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
using namespace tbb;
#endif

template<typename T>
Core<T>::Core(const int k, const int n, const int d, const int nc, const SNNDPC::Option& option) : k{k}, n{n}, d{d}, nc{nc}, option{option} { }

template<typename T>
Core<T>::~Core() {
	delete[] distance;
	delete[] indexDistanceAsc;
	delete[] indexNeighbor;
	delete[] indexSharedNeighbor;
	delete[] numSharedNeighbor;
	delete[] similarity;
	delete[] rho;
	delete[] delta;
	delete[] gamma;
	delete[] indexAssignment;
	delete[] indexCentroid;
}

template<typename T>
void Core<T>::ComputeDistance() {
	distance = new T[n * n];
	fill(distance, distance + n * n, T{0});
	for (int i = 0; i < n; i++)
		for (int j = 0; j < i; j++) {
			for (int u = 0; u < d; u++)
				distance[i * n + j] += pow(data[i * d + u] - data[j * d + u], 2);
			distance[i * n + j] = sqrt(distance[i * n + j]);
			distance[j * n + i] = distance[i * n + j];
		}
}
#ifdef PARALLEL_PROVIDER_IntelTBB
template<typename T>
struct Core<T>::TBB_ComputeNeighbor {
	const int n, k;
	const T* distance;
	int* indexDistanceAsc;
	int* indexNeighbor;

	TBB_ComputeNeighbor(int n, int k, const T* distance, int* indexDistanceAsc, int* indexNeighbor)
		: n{n}, k{k}, distance{distance}, indexDistanceAsc{indexDistanceAsc}, indexNeighbor{indexNeighbor} { }

	void operator()(const blocked_range<int>& range) const {
		for (auto i = range.begin(); i != range.end(); i++) {
#else
template<typename T>
void Core<T>::ComputeNeighbor() {
	indexDistanceAsc = new int[n * n];
	indexNeighbor = new int[n * k];
	for (int i = 0; i < n; i++) {
#endif // PARALLEL_PROVIDER_IntelTBB
			iota(indexDistanceAsc + i * n, indexDistanceAsc + i * n + n, 0);
			sort(indexDistanceAsc + i * n, indexDistanceAsc + i * n + n, [&](int a, int b) { return distance[i * n + a] < distance[i * n + b]; });
			copy(indexDistanceAsc + i * n, indexDistanceAsc + i * n + k, indexNeighbor + i * k);
			sort(indexNeighbor + i * k, indexNeighbor + (i + 1) * k); // For set_intersection()
		}
	}
#ifdef PARALLEL_PROVIDER_IntelTBB
};

template<typename T>
void Core<T>::ComputeNeighbor() {
	indexDistanceAsc = new int[n * n];
	indexNeighbor = new int[n * k];
	parallel_for(blocked_range<int>(0, n), TBB_ComputeNeighbor(n, k, distance, indexDistanceAsc, indexNeighbor));
}
#endif // PARALLEL_PROVIDER_IntelTBB

#ifdef PARALLEL_PROVIDER_IntelTBB
template<typename T>
struct Core<T>::TBB_ComputeSharedNeighbor {
	const int n, k;
	const int* indexNeighbor;
	int* numSharedNeighbor;
	int* indexSharedNeighbor;

	TBB_ComputeSharedNeighbor(int n, int k, const int* indexNeighbor, int* numSharedNeighbor, int* indexSharedNeighbor)
		: n{n}, k{k}, indexNeighbor{indexNeighbor}, numSharedNeighbor{numSharedNeighbor}, indexSharedNeighbor{indexSharedNeighbor} { }

	void operator()(const blocked_range<int>& range) const {
		for (auto i = range.begin(); i != range.end(); i++) {
#else
template<typename T>
void Core<T>::ComputeSharedNeighbor() {
	indexSharedNeighbor = new int[n * n * k];
	numSharedNeighbor = new int[n * n];
	for (int i = 0; i < n; i++) {
#endif // PARALLEL_PROVIDER_IntelTBB
			numSharedNeighbor[i * n + i] = 0;
			for (int j = 0; j < i; j++) {
				numSharedNeighbor[j * n + i] = numSharedNeighbor[i * n + j] = set_intersection(
					indexNeighbor + i * k, indexNeighbor + (i + 1) * k,
					indexNeighbor + j * k, indexNeighbor + (j + 1) * k,
					indexSharedNeighbor + i * n * k + j * k
				) - (indexSharedNeighbor + i * n * k + j * k);
				copy(
					indexSharedNeighbor + i * n * k + j * k,
					indexSharedNeighbor + i * n * k + j * k + numSharedNeighbor[i * n + j],
					indexSharedNeighbor + j * n * k + i * k
				);
			}
		}
	}
#ifdef PARALLEL_PROVIDER_IntelTBB
};

template<typename T>
void Core<T>::ComputeSharedNeighbor() {
	indexSharedNeighbor = new int[n * n * k];
	numSharedNeighbor = new int[n * n];
	parallel_for(blocked_range<int>(0, n), TBB_ComputeSharedNeighbor(n, k, indexNeighbor, numSharedNeighbor, indexSharedNeighbor));
}
#endif // PARALLEL_PROVIDER_IntelTBB

#ifdef PARALLEL_PROVIDER_IntelTBB
template<typename T>
struct Core<T>::TBB_ComputeSimilarity {
	const int n, k;
	const int* indexSharedNeighbor;
	const int* numSharedNeighbor;
	const T* distance;
	T* similarity;

	TBB_ComputeSimilarity(int n, int k, const int* indexSharedNeighbor, const int* numSharedNeighbor, const T* distance, T* similarity)
		: n{n}, k{k}, indexSharedNeighbor{indexSharedNeighbor}, numSharedNeighbor{numSharedNeighbor}, distance{distance}, similarity{similarity} { }

	void operator()(const blocked_range<int>& range) const {
		for (auto i = range.begin(); i != range.end(); i++) {
#else
template<typename T>
void Core<T>::ComputeSimilarity() {
	similarity = new T[n * n];
	for (int i = 0; i < n; i++) {
#endif // PARALLEL_PROVIDER_IntelTBB
			similarity[i * n + i] = 0;
			for (int j = 0; j < i; j++) {
				const auto first = indexSharedNeighbor + i * n * k + j * k;
				const auto last = indexSharedNeighbor + i * n * k + j * k + numSharedNeighbor[i * n + j];
				if (binary_search(first, last, i) && binary_search(first, last, j)) {
					auto sum = T{0};
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
#ifdef PARALLEL_PROVIDER_IntelTBB
};

template<typename T>
void Core<T>::ComputeSimilarity() {
	similarity = new T[n * n];
	parallel_for(blocked_range<int>(0, n), TBB_ComputeSimilarity(n, k, indexSharedNeighbor, numSharedNeighbor, distance, similarity));
}
#endif // PARALLEL_PROVIDER_IntelTBB

#ifdef PARALLEL_PROVIDER_IntelTBB
template<typename T>
struct Core<T>::TBB_ComputeRho {
	const int n, k;
	const T* similarity;
	T* rho;

	TBB_ComputeRho(int n, int k, const T* similarity, T* rho) : n{n}, k{k}, similarity{similarity}, rho{rho} {}

	void operator()(const blocked_range<int>& range) const {
		auto similarityDesc = new T[n];
		for (auto i = range.begin(); i != range.end(); i++) {
#else
template<typename T>
void Core<T>::ComputeRho() {
	rho = new T[n];
	auto similarityDesc = new T[n];
	for (int i = 0; i < n; i++) {
#endif // PARALLEL_PROVIDER_IntelTBB
			copy(similarity + i * n, similarity + (i + 1) * n, similarityDesc);
			sort(similarityDesc, similarityDesc + n, greater<>());
			rho[i] = accumulate(similarityDesc, similarityDesc + k, T{0});
		}
		delete[] similarityDesc;
	}
#ifdef PARALLEL_PROVIDER_IntelTBB
};

template<typename T>
void Core<T>::ComputeRho() {
	rho = new T[n];
	parallel_for(blocked_range<int>(0, n), TBB_ComputeRho(n, k, similarity, rho));
}
#endif // PARALLEL_PROVIDER_IntelTBB

template<typename T>
void Core<T>::ComputeDelta() {
	delta = new T[n];
	fill(delta, delta + n, numeric_limits<T>::infinity());
	auto distanceNeighborSum = new T[n];
	fill(distanceNeighborSum, distanceNeighborSum + n, T{0});
	for (int i = 0; i < n; i++)
		for (int j = 0; j < k; j++)
			distanceNeighborSum[i] += distance[i * n + indexNeighbor[i * k + j]];
	auto indexRhoDesc = new int[n];
	iota(indexRhoDesc, indexRhoDesc + n, 0);
	sort(indexRhoDesc, indexRhoDesc + n, [&](int a, int b) { return rho[a] > rho[b]; });
	for (int i = 1; i < n; i++) {
		int a = indexRhoDesc[i];
		for (int j = 0; j < i; j++) {
			int b = indexRhoDesc[j];
			delta[a] = min(delta[a], distance[a * n + b] * (distanceNeighborSum[a] + distanceNeighborSum[b]));
		}
	}
	delta[indexRhoDesc[0]] = -numeric_limits<T>::infinity();
	delta[indexRhoDesc[0]] = *max_element(delta, delta + n);
	delete[] distanceNeighborSum;
	delete[] indexRhoDesc;
}

template<typename T>
void Core<T>::ComputeGamma() {
	gamma = new T[n];
	for (int i = 0; i < n; i++)
		gamma[i] = rho[i] * delta[i];
}

template<typename T>
void Core<T>::ComputeCentroid() {
	indexAssignment = new int[n];
	indexCentroid = new int[nc];
	fill(indexAssignment, indexAssignment + n, Unassigned);
	auto indexGammaDesc = new int[n];
	iota(indexGammaDesc, indexGammaDesc + n, 0);
	sort(indexGammaDesc, indexGammaDesc + n, [&](int a, int b) { return gamma[a] > gamma[b]; });
	copy(indexGammaDesc, indexGammaDesc + nc, indexCentroid);
	sort(indexCentroid, indexCentroid + nc);
	for (int i = 0; i < nc; i++)
		indexAssignment[indexCentroid[i]] = i;
	delete[] indexGammaDesc;
}

template<typename T>
void Core<T>::AssignNonCentroidStep1() {
	queue<int> queue;
	for (int i = 0; i < nc; i++)
		queue.push(indexCentroid[i]);
	while (!queue.empty()) {
		int a = queue.front();
		queue.pop();
		for (int i = 0; i < k; i++) {
			int b = indexNeighbor[a * k + i];
			if (indexAssignment[b] == Unassigned && numSharedNeighbor[a * n + b] * 2 >= k) {
				indexAssignment[b] = indexAssignment[a];
				queue.push(b);
			}
		}
	}
}

template<typename T>
void Core<T>::AssignNonCentroidStep2() {
	int k = this->k;
	list<int> indexUnassigned;
	for (int i = 0; i < n; i++)
		if (indexAssignment[i] == Unassigned)
			indexUnassigned.push_back(i);
	int numUnassigned = indexUnassigned.size();
	auto numNeighborAssignment = new int[numUnassigned * nc];
	while (numUnassigned) {
		fill(numNeighborAssignment, numNeighborAssignment + numUnassigned * nc, 0);
		int i = 0;
		for (const auto& a : indexUnassigned) {
			for (int j = 0; j < k; j++) {
				int b = indexDistanceAsc[a * n + j];
				if (indexAssignment[b] != Unassigned)
					++numNeighborAssignment[i * nc + indexAssignment[b]];
			}
			i++;
		}
		if (int most = *max_element(numNeighborAssignment, numNeighborAssignment + numUnassigned * nc)) {
			auto it = indexUnassigned.begin();
			for (int i = 0; i < numUnassigned; i++) {
				auto first = numNeighborAssignment + i * nc;
				auto last = numNeighborAssignment + (i + 1) * nc;
				auto current = find(first, last, most); // In MATLAB, if multiple hits, the last will be used
				if (current == last) ++it;
				else {
					indexAssignment[*it] = current - first;
					it = indexUnassigned.erase(it);
				}
			}
			numUnassigned = indexUnassigned.size();
		} else {
			++k; // TODO: Replace with custom function
			assert(k <= n);
		}
	}
	delete[] numNeighborAssignment;
}
