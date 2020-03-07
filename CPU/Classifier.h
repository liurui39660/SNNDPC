#pragma once

#include "Core.h"

#ifndef NDEBUG
#define protected public
#endif

namespace SNNDPC::CPU {

template<typename T>
class Classifier : public Core<T> {

public:
	/**
	 * Initialize a SNN-DPC classifier.
	 *
	 * @param numNeighbor A.k.a. k, number of neighbors should be evaluated.
	 * @param numRecord A.k.a. n, number of records in your dataset
	 * @param numDimension A.k.a. d, number of dimensions in your dataset
	 * @param numCentroid A.k.a. nc, number of centroids
	 * @param option Other options, see the documentation of corresponding fields
	 * */
	Classifier(int numNeighbor, int numRecord, int numDimension, int numCentroid, const Option& option);
	~Classifier() override;

	/**
	 * End-to-end execution.
	 *
	 * No more confusion on how to use this classifier, just call this and enjoy.
	 *
	 * @param data Input data of shape [numRecord * numDimension].
	 * @param centroid Indices of centroids, shape [numCentroid].
	 * @param assignment Assignment of non-centroids, shape [numRecord].
	 * */
	void Run(T* data, int* centroid = nullptr, int* assignment = nullptr);

protected:
	void Input(T* data);
	void Normalize();
	void Output(int* centroid = nullptr, int* assignment = nullptr);

private:
	template<typename TFirst, typename... TRest>
	void EarlyRelease(TFirst& pointer, TRest& ...rest);
	template<typename TFirst>
	void EarlyRelease(TFirst& pointer);
};

template
class Classifier<float>;

template
class Classifier<double>;
}

#ifndef NDEBUG
#undef protected
#endif
