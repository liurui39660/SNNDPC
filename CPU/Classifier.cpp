#include "Classifier.h"

#include <algorithm>

using namespace std;
using namespace SNNDPC::CPU;

template<typename T>
Classifier<T>::Classifier(int numNeighbor, int numRecord, int numDimension, int numCentroid, const SNNDPC::Option& option): Core<T>(numNeighbor, numRecord, numDimension, numCentroid, option) { }

template<typename T>
Classifier<T>::~Classifier() {
	if (this->option.shallCopyInput) {
		delete[] this->data;
	}
}

template<typename T>
void Classifier<T>::Run(T* data, int* centroid, int* assignment) {
	Input(data);
	if (option.shallNormalize) {
		Normalize();
	}
	ComputeDistance();
	EarlyRelease(this->data);
	ComputeNeighbor();
	ComputeSharedNeighbor();
	ComputeSimilarity();
	EarlyRelease(indexSharedNeighbor);
	ComputeRho();
	EarlyRelease(similarity);
	ComputeDelta();
	EarlyRelease(distance);
	ComputeGamma();
	EarlyRelease(rho, delta);
	ComputeCentroid();
	EarlyRelease(gamma);
	AssignNonCentroidStep1();
	EarlyRelease(indexNeighbor, numSharedNeighbor);
	AssignNonCentroidStep2();
	EarlyRelease(indexDistanceAsc);
	Output(centroid, assignment);
}

template<typename T>
void Classifier<T>::Input(T* data) {
	if (option.shallCopyInput) {
		this->data = new T[n * d];
		copy(data, data + n * d, this->data);
	} else {
		this->data = data;
	}
}

template<typename T>
void Classifier<T>::Normalize() {
	auto least = new T[d];
	auto most = new T[d];
	fill(least, least + d, numeric_limits<T>::infinity());
	fill(most, most + d, -numeric_limits<T>::infinity());
	for (int i = 0; i < n; i++)
		for (int j = 0; j < d; j++) {
			least[j] = min(least[j], data[i * d + j]);
			most[j] = max(most[j], data[i * d + j]);
		}
	for (int i = 0; i < n; i++)
		for (int j = 0; j < d; j++)
			data[i * d + j] = (data[i * d + j] - least[j]) / (most[j] - least[j]);
	delete[] least;
	delete[] most;
}

template<typename T>
void Classifier<T>::Output(int* centroid, int* assignment) {
	if (centroid) copy(indexCentroid, indexCentroid + nc, centroid);
	if (assignment)
		if (option.useOrdinalAssignment)
			copy(indexAssignment, indexAssignment + n, assignment);
		else
			for (int i = 0; i < n; i++)
				assignment[i] = indexCentroid[indexAssignment[i]];
}

template<typename T>
template<typename TFirst, typename... TRest>
void Classifier<T>::EarlyRelease(TFirst& pointer, TRest& ...rest) {
	EarlyRelease(pointer);
	EarlyRelease(rest...);
}

template<typename T>
template<typename TFirst>
void Classifier<T>::EarlyRelease(TFirst& pointer) {
	if (option.useEarlyRelease) {
		delete[] pointer;
		pointer = nullptr;
	}
}
