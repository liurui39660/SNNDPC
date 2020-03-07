#include "Classifier.h"

#include <cuda_runtime.h>

using namespace std;
using namespace SNNDPC::GPU;

template<typename T>
Classifier<T>::Classifier(int numNeighbor, int numRecord, int numDimension, int numCentroid, const SNNDPC::Option& option): Core<T>(numNeighbor, numRecord, numDimension, numCentroid, option) { }

template<typename T>
Classifier<T>::~Classifier() {
	cudaFree(this->data);
}

template<typename T>
void Classifier<T>::Run(const T* data, int* centroid, int* assignment) {
	Input(data);
	Normalize();
	ComputeDistance();
	ComputeNeighbor();
	ComputeSharedNeighbor();
	ComputeSimilarity();
	ComputeRho();
	ComputeDelta();
	ComputeGamma();
	ComputeCentroid();
	AssignNonCentroidStep1();
	AssignNonCentroidStep2();
	Output(centroid, assignment);
}

template<typename T>
void Classifier<T>::Input(const T* data) {
	cudaMallocManaged(&this->data, n * d * sizeof(T));
	cudaMemcpy(this->data, data, n * d * sizeof(T), cudaMemcpyHostToDevice);
}

template<typename T>
void Classifier<T>::Normalize() {
	for (int i = 0; i < d; i++) {
		int indexLeast, indexMost;
		cublasIaminEx(handle, n, data + i, type, d, &indexLeast);
		cublasIamaxEx(handle, n, data + i, type, d, &indexMost);
		const T* least = &data[(indexLeast - 1) * d + i];
		const T* most = &data[(indexMost - 1) * d + i];
		const T diff = T{1} / (*most - *least);
		const T MinusOne = -1;
		cublasAxpyEx(handle, n, &MinusOne, type, least, type, 0, data + i, type, d, type);
		cublasScalEx(handle, n, &diff, type, data + i, type, d, type);
	}
}

template<typename T>
void Classifier<T>::Output(int* centroid, int* assignment) {
	if (centroid) cudaMemcpy(centroid, indexCentroid, nc * sizeof(int), cudaMemcpyDeviceToHost);
	if (assignment) cudaMemcpy(assignment, indexAssignment, n * sizeof(int), cudaMemcpyDeviceToHost);
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
		cudaFree(pointer);
		pointer = nullptr;
	}
}
