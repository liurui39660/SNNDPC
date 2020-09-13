from numpy import loadtxt, ndarray, min, max
from sklearn.metrics import adjusted_mutual_info_score, adjusted_rand_score, fowlkes_mallows_score

from SNNDPC import SNNDPC

if __name__ == '__main__':
	# Parameter
	# --------------------------------------------------------------------------------

	# pathData = "../data/Flame.tsv"
	# k = 5
	# nc = 2

	pathData = "../data/Aggregation.tsv"
	k = 15
	nc = 7

	# Run
	# --------------------------------------------------------------------------------

	data: ndarray = loadtxt(pathData)
	label = data[:, -1]
	data: ndarray = data[:, :-1]
	data = (data - min(data, axis=0)) / (max(data, axis=0) - min(data, axis=0))
	centroid, assignment = SNNDPC(k, nc, data)
	print(f"Centroids = {centroid.tolist()}")
	print(f"AMI = {adjusted_mutual_info_score(label, assignment):.4f}")
	print(f"ARI = {adjusted_rand_score(label, assignment):.4f}")
	print(f"FMI = {fowlkes_mallows_score(label, assignment):.4f}")
