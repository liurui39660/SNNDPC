from numpy import loadtxt, ndarray
from sklearn.metrics import adjusted_mutual_info_score, adjusted_rand_score, fowlkes_mallows_score

from SNNDPC import Classifier

if __name__ == '__main__':
	data: ndarray = loadtxt("data/Flame.tsv")
	truth = data[:, -1]
	data: ndarray = data[:, :-1]
	n, d = data.shape
	centroid, assignment = Classifier(5, n, d, 2).Run(data)  # Please refer to the article for those two constant parameters
	print(f"Centroids = {centroid.tolist()}")
	print(f"AMI = {adjusted_mutual_info_score(truth, assignment):.4f}")
	print(f"ARI = {adjusted_rand_score(truth, assignment):.4f}")
	print(f"FMI = {fowlkes_mallows_score(truth, assignment):.4f}")
