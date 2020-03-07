from typing import List

from numpy import arange, argsort, argwhere, empty, full, inf, intersect1d, max, ndarray, sort, sum, zeros
from scipy.spatial.distance import pdist, squareform

from .Option import Option

class Core:
	data: ndarray

	def __init__(self, k: int, n: int, d: int, nc: int, option: Option = Option()) -> None:
		self.UNASSIGNED = -1
		self.k = k
		self.n = n
		self.d = d
		self.nc = nc
		self.option = option

	def ComputeDistance(self) -> None:
		self.distance = squareform(pdist(self.data))

	def ComputeNeighbor(self) -> None:
		self.indexDistanceAsc: ndarray = argsort(self.distance)
		self.indexNeighbor: ndarray = self.indexDistanceAsc[:, :self.k]

	def ComputeSharedNeighbor(self) -> None:
		self.indexSharedNeighbor = empty([self.n, self.n, self.k], int)
		self.numSharedNeighbor = empty([self.n, self.n], int)
		for i in range(self.n):
			self.numSharedNeighbor[i, i] = 0
			for j in range(i):
				# noinspection PyTypeChecker
				shared: ndarray = intersect1d(self.indexNeighbor[i], self.indexNeighbor[j], assume_unique=True)
				self.numSharedNeighbor[j, i] = self.numSharedNeighbor[i, j] = shared.size
				self.indexSharedNeighbor[j, i, :shared.size] = self.indexSharedNeighbor[i, j, :shared.size] = shared

	def ComputeSimilarity(self) -> None:
		self.similarity = zeros([self.n, self.n])  # Diagonal and some elements are 0
		for i in range(self.n):
			for j in range(i):
				if i in self.indexSharedNeighbor[i, j] and j in self.indexSharedNeighbor[i, j]:
					indexShared = self.indexSharedNeighbor[i, j, :self.numSharedNeighbor[i, j]]
					distanceSum = sum(self.distance[i, indexShared] + self.distance[j, indexShared])
					self.similarity[i, j] = self.similarity[j, i] = self.numSharedNeighbor[i, j] ** 2 / distanceSum

	def ComputeRho(self) -> None:
		self.rho = sum(sort(self.similarity)[:, -self.k:], axis=1)

	def ComputeDelta(self) -> None:
		distanceNeighborSum = empty(self.n)
		for i in range(self.n):
			distanceNeighborSum[i] = sum(self.distance[i, self.indexNeighbor[i]])
		indexRhoDesc = argsort(self.rho)[::-1]
		self.delta = full(self.n, inf)
		for i, a in enumerate(indexRhoDesc[1:], 1):
			for b in indexRhoDesc[:i]:
				self.delta[a] = min(self.delta[a], self.distance[a, b] * (distanceNeighborSum[a] + distanceNeighborSum[b]))
		self.delta[indexRhoDesc[0]] = -inf
		self.delta[indexRhoDesc[0]] = max(self.delta)

	def ComputeGamma(self) -> None:
		self.gamma = self.rho * self.delta

	def ComputeCentroid(self):
		self.indexAssignment = full(self.n, self.UNASSIGNED)
		self.indexCentroid: ndarray = sort(argsort(self.gamma)[-self.nc:])
		self.indexAssignment[self.indexCentroid] = arange(self.nc)

	def AssignNonCentroidStep1(self) -> None:
		# noinspection PyTypeChecker
		queue: List[int] = self.indexCentroid.tolist()
		while queue:
			a = queue.pop(0)
			for b in self.indexNeighbor[a]:
				if self.indexAssignment[b] == self.UNASSIGNED and self.numSharedNeighbor[a, b] >= self.k / 2:
					self.indexAssignment[b] = self.indexAssignment[a]
					queue.append(b)

	def AssignNonCentroidStep2(self) -> None:
		k = self.k
		numFailedAttempt = 0
		numAttempt = 0
		indexUnassigned = argwhere(self.indexAssignment == self.UNASSIGNED).flatten()
		while indexUnassigned.size:
			numAttempt += 1
			numNeighborAssignment = zeros([indexUnassigned.size, self.nc], int)
			for i, a in enumerate(indexUnassigned):
				for b in self.indexDistanceAsc[a, :k]:
					if self.indexAssignment[b] != self.UNASSIGNED:
						numNeighborAssignment[i, self.indexAssignment[b]] += 1
			most = max(numNeighborAssignment)
			if most > 0:
				temp = argwhere(numNeighborAssignment == most)
				self.indexAssignment[indexUnassigned[temp[:, 0]]] = temp[:, 1]
				indexUnassigned = argwhere(self.indexAssignment == self.UNASSIGNED).flatten()
			else:
				numFailedAttempt += 1
			k = self.option.AdjustNumCentroid(most > 0, k, indexUnassigned.size, numFailedAttempt, numAttempt)
