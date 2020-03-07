import sys
import numpy as np
from sklearn.metrics import adjusted_mutual_info_score, adjusted_rand_score, fowlkes_mallows_score

if __name__ == '__main__':
	if len(sys.argv) < 3: exit(1)
	truth = np.loadtxt(sys.argv[1], int)
	predict = np.loadtxt(sys.argv[2], int)
	print(f"AMI = {adjusted_mutual_info_score(truth, predict, average_method='max'):.4f}")
	print(f"ARI = {adjusted_rand_score(truth, predict):.4f}")
	print(f"FMI = {fowlkes_mallows_score(truth, predict):.4f}")
