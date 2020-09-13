#pragma once

#include <cmath>
#include <numeric>
#include <algorithm>
#include <queue>
#include <list>
#include <tuple>

#ifdef ParallelProvider_IntelTBB
#include <tbb/parallel_for.h>
#else
#include <omp.h>
#endif

auto SNNDPC(int k, int n, int d, int nc, float* data) {
	const int unassigned = -1;
	const float infinity = std::numeric_limits<float>::infinity();

	// Compute distance
	// --------------------------------------------------------------------------------

	const auto distance = new float[n * n];
	std::fill(distance, distance + n * n, 0);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < i; j++) {
			for (int u = 0; u < d; u++)
				distance[i * n + j] += pow(data[i * d + u] - data[j * d + u], 2);
			distance[i * n + j] = sqrt(distance[i * n + j]);
			distance[j * n + i] = distance[i * n + j];
		}

	// Compute neighbor
	// --------------------------------------------------------------------------------

	const auto indexDistanceAsc = new int[n * n];
	const auto indexNeighbor = new int[n * k];
#ifdef ParallelProvider_IntelTBB
	tbb::parallel_for(0, n, [&](int i) {
#else // @formatter:off
	#pragma omp parallel for
	for (int i = 0; i < n; i++) {
#endif // @formatter:on
		std::iota(indexDistanceAsc + i * n, indexDistanceAsc + i * n + n, 0);
		std::sort(indexDistanceAsc + i * n, indexDistanceAsc + i * n + n, [&](int a, int b) { return distance[i * n + a] < distance[i * n + b]; });
		std::copy(indexDistanceAsc + i * n, indexDistanceAsc + i * n + k, indexNeighbor + i * k);
		std::sort(indexNeighbor + i * k, indexNeighbor + (i + 1) * k); // For set_intersection()
#ifdef ParallelProvider_IntelTBB
	});
#else // @formatter:off
	}
#endif // @formatter:on

	// Compute shared neighbor
	// --------------------------------------------------------------------------------

	const auto indexSharedNeighbor = new int[n * n * k];
	const auto numSharedNeighbor = new int[n * n];
#ifdef ParallelProvider_IntelTBB
	tbb::parallel_for(0, n, [&](int i) {
#else // @formatter:off
		#pragma omp parallel for
	for (int i = 0; i < n; i++) {
#endif // @formatter:on
		numSharedNeighbor[i * n + i] = 0;
		for (int j = 0; j < i; j++) {
			numSharedNeighbor[j * n + i] = numSharedNeighbor[i * n + j] = std::set_intersection(
				indexNeighbor + i * k, indexNeighbor + (i + 1) * k,
				indexNeighbor + j * k, indexNeighbor + (j + 1) * k,
				indexSharedNeighbor + i * n * k + j * k
			) - (indexSharedNeighbor + i * n * k + j * k);
			std::copy(
				indexSharedNeighbor + i * n * k + j * k,
				indexSharedNeighbor + i * n * k + j * k + numSharedNeighbor[i * n + j],
				indexSharedNeighbor + j * n * k + i * k
			);
		}
#ifdef ParallelProvider_IntelTBB
	});
#else // @formatter:off
	}
#endif // @formatter:on

	// Compute similarity
	// --------------------------------------------------------------------------------

	const auto similarity = new float[n * n];
	std::fill(similarity, similarity + n * n, 0);
	for (int i = 0; i < n; i++) {
		similarity[i * n + i] = 0;
		for (int j = 0; j < i; j++) {
			const auto first = indexSharedNeighbor + i * n * k + j * k;
			const auto last = indexSharedNeighbor + i * n * k + j * k + numSharedNeighbor[i * n + j];
			if (std::binary_search(first, last, i) && std::binary_search(first, last, j)) {
				float sum = 0;
				for (int u = 0; u < numSharedNeighbor[i * n + j]; u++) {
					const int shared = indexSharedNeighbor[i * n * k + j * k + u];
					sum += distance[i * n + shared] + distance[j * n + shared];
				}
				similarity[j * n + i] = similarity[i * n + j] = pow(numSharedNeighbor[i * n + j], 2) / sum;
			}
		}
	}
	delete[] indexSharedNeighbor;

	// Compute ρ
	// --------------------------------------------------------------------------------

	const auto rho = new float[n];
	const auto similarityDesc = new float[n];
	for (int i = 0; i < n; i++) {
		std::copy(similarity + i * n, similarity + (i + 1) * n, similarityDesc);
		sort(similarityDesc, similarityDesc + n, std::greater<>());
		rho[i] = std::accumulate(similarityDesc, similarityDesc + k, 0);
	}
	delete[] similarity;
	delete[] similarityDesc;

	// Compute δ
	// --------------------------------------------------------------------------------

	const auto delta = new float[n];
	const auto distanceNeighborSum = new float[n];
	const auto indexRhoDesc = new int[n];
	std::fill(delta, delta + n, infinity);
	std::fill(distanceNeighborSum, distanceNeighborSum + n, 0);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < k; j++)
			distanceNeighborSum[i] += distance[i * n + indexNeighbor[i * k + j]];
	std::iota(indexRhoDesc, indexRhoDesc + n, 0);
	std::sort(indexRhoDesc, indexRhoDesc + n, [&](int a, int b) { return rho[a] > rho[b]; });
	for (int i = 1; i < n; i++) {
		int a = indexRhoDesc[i];
		for (int j = 0; j < i; j++) {
			int b = indexRhoDesc[j];
			delta[a] = std::min(delta[a], distance[a * n + b] * (distanceNeighborSum[a] + distanceNeighborSum[b]));
		}
	}
	delta[indexRhoDesc[0]] = -infinity;
	delta[indexRhoDesc[0]] = *std::max_element(delta, delta + n);
	delete[] distance;
	delete[] distanceNeighborSum;
	delete[] indexRhoDesc;

	// Compute γ
	// --------------------------------------------------------------------------------

	const auto gamma = new float[n];
	for (int i = 0; i < n; i++)
		gamma[i] = rho[i] * delta[i];
	delete[] rho;
	delete[] delta;

	// Compute centroid
	// --------------------------------------------------------------------------------

	const auto indexAssignment = new int[n];
	const auto indexCentroid = new int[nc];
	const auto indexGammaDesc = new int[n];
	std::fill(indexAssignment, indexAssignment + n, unassigned);
	std::iota(indexGammaDesc, indexGammaDesc + n, 0);
	std::sort(indexGammaDesc, indexGammaDesc + n, [&](int a, int b) { return gamma[a] > gamma[b]; });
	std::copy(indexGammaDesc, indexGammaDesc + nc, indexCentroid);
	std::sort(indexCentroid, indexCentroid + nc);
	for (int i = 0; i < nc; i++)
		indexAssignment[indexCentroid[i]] = i;
	delete[] gamma;
	delete[] indexGammaDesc;


	// Assign non centroid step 1
	// --------------------------------------------------------------------------------

	std::queue<int> queue;
	for (int i = 0; i < nc; i++)
		queue.push(indexCentroid[i]);
	while (!queue.empty()) {
		int a = queue.front();
		queue.pop();
		for (int i = 0; i < k; i++) {
			int b = indexNeighbor[a * k + i];
			if (indexAssignment[b] == unassigned && numSharedNeighbor[a * n + b] * 2 >= k) {
				indexAssignment[b] = indexAssignment[a];
				queue.push(b);
			}
		}
	}
	delete[] indexNeighbor;
	delete[] numSharedNeighbor;

	// Assign non centroid step 2
	// --------------------------------------------------------------------------------

	std::list<int> indexUnassigned;
	for (int i = 0; i < n; i++)
		if (indexAssignment[i] == unassigned)
			indexUnassigned.push_back(i);

	int numUnassigned = indexUnassigned.size();
	const auto numNeighborAssignment = new int[numUnassigned * nc];
	while (numUnassigned) {
		std::fill(numNeighborAssignment, numNeighborAssignment + numUnassigned * nc, 0);
		int i = 0;
		for (const auto& a : indexUnassigned) {
			for (int j = 0; j < k; j++) {
				int b = indexDistanceAsc[a * n + j];
				if (indexAssignment[b] != unassigned)
					++numNeighborAssignment[i * nc + indexAssignment[b]];
			}
			i++;
		}
		if (int most = *std::max_element(numNeighborAssignment, numNeighborAssignment + numUnassigned * nc)) {
			auto it = indexUnassigned.begin();
			for (int j = 0; j < numUnassigned; j++) {
				const auto first = numNeighborAssignment + j * nc;
				const auto last = numNeighborAssignment + (j + 1) * nc;
				const auto current = std::find(first, last, most); // In MATLAB, if multiple hits, the last will be used
				if (current == last) ++it;
				else {
					indexAssignment[*it] = current - first;
					it = indexUnassigned.erase(it);
				}
			}
			numUnassigned = indexUnassigned.size();
		} else k++;
	}
	delete[] indexDistanceAsc;
	delete[] numNeighborAssignment;

	// Return
	// --------------------------------------------------------------------------------

	return std::tuple{indexCentroid, indexAssignment};
}