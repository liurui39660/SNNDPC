from numpy import loadtxt, ndarray, unique
from sklearn.metrics import adjusted_mutual_info_score, adjusted_rand_score, fowlkes_mallows_score

from SNNDPC import Option, Classifier

if __name__ == '__main__':
	data: ndarray = loadtxt("data/Aggregation.tsv")
	truth = data[:, -1]
	data: ndarray = data[:, :-1]
	n, d = data.shape
	option = Option()
	option.shouldUseOrdinalAssignment = False
	centroid, assignment = Classifier(15, n, d, 7).Run(data)  # Please refer to the article for those two constant parameters
	print(f"Centroids = {unique(assignment).tolist()}")
	print(f"AMI = {adjusted_mutual_info_score(truth, assignment):.4f}")
	print(f"ARI = {adjusted_rand_score(truth, assignment):.4f}")
	print(f"FMI = {fowlkes_mallows_score(truth, assignment):.4f}")
