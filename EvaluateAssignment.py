from sys import argv
from numpy import loadtxt
from sklearn.metrics import adjusted_mutual_info_score, adjusted_rand_score, fowlkes_mallows_score

if __name__ == '__main__':
	if len(argv) < 3:
		print("Usage: python EvaluateAssignment.py <pathGroundTruth> <pathAssignment>")
	else:
		label = loadtxt(argv[1], int)
		assignment = loadtxt(argv[2], int)
		print(f"AMI = {adjusted_mutual_info_score(label, assignment, average_method='max'):.4f}")
		print(f"ARI = {adjusted_rand_score(label, assignment):.4f}")
		print(f"FMI = {fowlkes_mallows_score(label, assignment):.4f}")