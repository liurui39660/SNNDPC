#include <chrono>

#include "SNNDPC.hpp"

using namespace std::chrono;

int main(int argc, char* argv[]) {
	// Parameter
	// --------------------------------------------------------------------------------

	// const auto pathDatabase = SOLUTION_DIR"data/Flame.tsv";
	// const int k = 5, numPoint = 240, numDim = 2, numCentroid = 2;

	const auto pathDatabase = SOLUTION_DIR"data/S2.tsv";
	const int k = 35, numPoint = 5000, numDim = 2, numCentroid = 15;

	// Read dataset
	// --------------------------------------------------------------------------------

	int label[numPoint];
	float data[numPoint * numDim];
	const auto fileData = fopen(pathDatabase, "r");
	for (int i = 0; i < numPoint; i++)
		fscanf(fileData, "%f %f %d\n", &data[i * numDim], &data[i * numDim + 1], &label[i]);
	fclose(fileData);

	// Export ground truth
	// --------------------------------------------------------------------------------

	const auto pathGroundTruth = SOLUTION_DIR"temp/GroundTruth.txt";
	const auto fileGroundTruth = fopen(pathGroundTruth, "w");
	for (int i: label)
		fprintf(fileGroundTruth, "%d\n", i);
	fclose(fileGroundTruth);

	// Normalize
	// --------------------------------------------------------------------------------

	float least[numDim], most[numDim];
	const auto infinity = std::numeric_limits<float>::infinity();
	std::fill(least, least + numDim, infinity);
	std::fill(most, most + numDim, -infinity);
	for (int i = 0; i < numPoint; i++)
		for (int j = 0; j < numDim; j++) {
			least[j] = std::min(least[j], data[i * numDim + j]);
			most[j] = std::max(most[j], data[i * numDim + j]);
		}
	for (int i = 0; i < numPoint; i++)
		for (int j = 0; j < numDim; j++)
			data[i * numDim + j] = (data[i * numDim + j] - least[j]) / (most[j] - least[j]);

	// Do the magic
	// --------------------------------------------------------------------------------

	const auto time = high_resolution_clock::now();
	const auto[centroid, assignment] = SNNDPC(k, numPoint, numDim, numCentroid, data);
	printf("Time Cost = %lldms\n", duration_cast<milliseconds>(high_resolution_clock::now() - time).count());

	// Export centroid
	// --------------------------------------------------------------------------------

	const auto fileCentroid = fopen(SOLUTION_DIR"temp/Centroid.txt", "w");
	for (int i = 0; i < numCentroid; i++)
		fprintf(fileCentroid, "%d\n", centroid[i]);
	fclose(fileCentroid);

	// Export assignment
	// --------------------------------------------------------------------------------

	const auto pathAssignment = SOLUTION_DIR"temp/Assignment.txt";
	const auto fileAssignment = fopen(pathAssignment, "w");
	for (int i = 0; i < numPoint; i++)
		fprintf(fileAssignment, "%d\n", assignment[i]);
	fclose(fileAssignment);

	// Evaluate
	// --------------------------------------------------------------------------------

	char command[1024];
	sprintf(command, "python %s %s %s", SOLUTION_DIR"EvaluateAssignment.py", pathGroundTruth, pathAssignment);
	system(command);

	// Clean up
	// --------------------------------------------------------------------------------

	delete[] centroid;
	delete[] assignment;
}
