from typing import Callable

from numpy import max, min, ndarray

class Option:
	def __init__(self) -> None:
		self.shouldCopyInputData = True
		"""
		If True or self.Preprocess is not None, a copy of the original data will be used.

		Otherwise, the original input data will be used.

		"""

		self.shouldUseOrdinalAssignment = True
		"""
		If True, assignment will use ordinals like 0, 1, 2, ….

		If False, will use indices of centroids.

		"""

		self.Preprocess: Callable[[ndarray], ndarray] = lambda data: (data - min(data, axis=0)) / (max(data, axis=0) - min(data, axis=0))
		"""
		If None, no preprocessing will be applied.

		Otherwise, should be a function that consumes the input data, and returns the normalized data.

		By default, it linearly maps every dimension into [0, 1].
		
		Args:
			data: The original data to be processed.
			
		Returns:
			Pre-processed data.

		"""

		self.AdjustNumCentroid: Callable[[bool, int, int, int, int], int] = lambda isSuccessful, numNeighbor, numUnassigned, numFailedAttempt, numAttempt: numNeighbor if isSuccessful else numNeighbor + 1
		"""
		The function to change numNeighbor (k) after each failed attempt in the assignment step 2.

		See the default function for its signature.

		By default, it increases numNeighbor by 1.
		
		Args:
			isSuccessful: 
				True if the most recent attempt is successful.
				Usually you only need to update when it is failed.
			numNeighbor:
				The current numNeighbor, a.k.a. k.
				This is what you need to update.
			numUnassigned:
				Number of unassigned non-centroids after the most recent attempt.
			numFailedAttempt:
				Number of failed attempts, including the most recent one.
			numAttempt:
				Total number of attempts, including successful and failed ones.
		
		Returns:
			The updated numNeighbor to be used for the next attempt.
		"""
