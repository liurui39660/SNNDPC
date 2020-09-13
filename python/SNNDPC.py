from typing import List, Tuple

from numpy import arange, argsort, argwhere, empty, full, inf, intersect1d, max, ndarray, sort, sum, zeros
from scipy.spatial.distance import pdist, squareform

def SNNDPC(k: int, nc: int, data: ndarray) -> Tuple[ndarray, ndarray]:
	unassigned = -1
	n, d = data.shape

	# Compute distance
	# --------------------------------------------------------------------------------

	distance = squareform(pdist(data))

	# Compute neighbor
	# --------------------------------------------------------------------------------

	indexDistanceAsc: ndarray = argsort(distance)
	indexNeighbor: ndarray = indexDistanceAsc[:, :k]

	# Compute shared neighbor
	# --------------------------------------------------------------------------------

	indexSharedNeighbor = empty([n, n, k], int)
	numSharedNeighbor = empty([n, n], int)
	for i in range(n):
		numSharedNeighbor[i, i] = 0
		for j in range(i):
			shared: ndarray = intersect1d(indexNeighbor[i], indexNeighbor[j], assume_unique=True)
			numSharedNeighbor[j, i] = numSharedNeighbor[i, j] = shared.size
			indexSharedNeighbor[j, i, :shared.size] = indexSharedNeighbor[i, j, :shared.size] = shared

	# Compute similarity
	# --------------------------------------------------------------------------------

	similarity = zeros([n, n])  # Diagonal and some elements are 0
	for i in range(n):
		for j in range(i):
			if i in indexSharedNeighbor[i, j] and j in indexSharedNeighbor[i, j]:
				indexShared = indexSharedNeighbor[i, j, :numSharedNeighbor[i, j]]
				distanceSum = sum(distance[i, indexShared] + distance[j, indexShared])
				similarity[i, j] = similarity[j, i] = numSharedNeighbor[i, j] ** 2 / distanceSum

	# Compute ρ
	# --------------------------------------------------------------------------------

	rho = sum(sort(similarity)[:, -k:], axis=1)

	# Compute δ
	# --------------------------------------------------------------------------------

	distanceNeighborSum = empty(n)
	for i in range(n):
		distanceNeighborSum[i] = sum(distance[i, indexNeighbor[i]])
	indexRhoDesc = argsort(rho)[::-1]
	delta = full(n, inf)
	for i, a in enumerate(indexRhoDesc[1:], 1):
		for b in indexRhoDesc[:i]:
			delta[a] = min(delta[a], distance[a, b] * (distanceNeighborSum[a] + distanceNeighborSum[b]))
	delta[indexRhoDesc[0]] = -inf
	delta[indexRhoDesc[0]] = max(delta)

	# Compute γ
	# --------------------------------------------------------------------------------

	gamma = rho * delta

	# Compute centroid
	# --------------------------------------------------------------------------------

	indexAssignment = full(n, unassigned)
	indexCentroid: ndarray = sort(argsort(gamma)[-nc:])
	indexAssignment[indexCentroid] = arange(nc)

	# Assign non-centroid step 1
	# --------------------------------------------------------------------------------

	queue: List[int] = indexCentroid.tolist()
	while queue:
		a = queue.pop(0)
		for b in indexNeighbor[a]:
			if indexAssignment[b] == unassigned and numSharedNeighbor[a, b] >= k / 2:
				indexAssignment[b] = indexAssignment[a]
				queue.append(b)

	# Assign non-centroid step 2
	# --------------------------------------------------------------------------------

	indexUnassigned = argwhere(indexAssignment == unassigned).flatten()
	while indexUnassigned.size:
		numNeighborAssignment = zeros([indexUnassigned.size, nc], int)
		for i, a in enumerate(indexUnassigned):
			for b in indexDistanceAsc[a, :k]:
				if indexAssignment[b] != unassigned:
					numNeighborAssignment[i, indexAssignment[b]] += 1
		if most := max(numNeighborAssignment):
			temp = argwhere(numNeighborAssignment == most)
			indexAssignment[indexUnassigned[temp[:, 0]]] = temp[:, 1]
			indexUnassigned = argwhere(indexAssignment == unassigned).flatten()
		else:
			k += 1

	return indexCentroid, indexAssignment
