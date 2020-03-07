from typing import Tuple

from numpy import ndarray

from .Core import Core
from .Option import Option

class Classifier(Core):
	def __init__(self, numNeighbor: int, numRecord: int, numDimension: int, numCentroid: int, option: Option = Option()) -> None:
		"""
		Initialize a SNN-DPC classifier.

		Args:
			numNeighbor: A.k.a. k, number of neighbors should be evaluated.
			numRecord: A.k.a. n, number of records in your dataset
			numDimension: A.k.a. d, number of dimensions in your dataset
			numCentroid: A.k.a. nc, number of centroids
			option: Other options, see the documentation of corresponding fields
		"""
		super().__init__(numNeighbor, numRecord, numDimension, numCentroid, option)

	def Run(self, data: ndarray) -> Tuple[ndarray, ndarray]:
		"""
		End-to-end execution.

		No more confusion on how to use this classifier, just call this and enjoy.

		Args:
			data: Input data of size [numRecord, numDimension].

		Returns:
			Indices of centroids, assignment of non-centroids.
		"""
		self.Input(data)
		self.ComputeDistance()
		self.ComputeNeighbor()
		self.ComputeSharedNeighbor()
		self.ComputeSimilarity()
		self.ComputeRho()
		self.ComputeDelta()
		self.ComputeGamma()
		self.ComputeCentroid()
		self.AssignNonCentroidStep1()
		self.AssignNonCentroidStep2()
		return self.Output()

	def Input(self, data: ndarray) -> None:
		"""
		Input the dataset.

		Args:
			data: Input data of size [numRecord, numDimension].
		"""
		if self.option.Preprocess:
			self.data: ndarray = self.option.Preprocess(data)
		elif self.option.shouldCopyInputData:
			self.data: ndarray = data.copy()
		else:
			self.data: ndarray = data

	def Output(self) -> Tuple[ndarray, ndarray]:
		"""
		Output the result.

		Returns:
			Indices of centroids and the assignment of non-centroids.
		"""

		if self.option.shouldUseOrdinalAssignment:
			return self.indexCentroid, self.indexCentroid[self.indexAssignment]
		else:
			return self.indexCentroid, self.indexAssignment
