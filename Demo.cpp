#include <chrono>

#include "SNNDPC.hpp"

using namespace std::chrono;

int main(int argc, char* argv[]) {
	// Parameter
	// --------------------------------------------------------------------------------

	const auto pathData = SOLUTION_DIR"data/S2.tsv";
	const int k = 35;
	const int n = 5000; // Number of data points
	const int d = 2; // Dimension
	const int nc = 15; // Number of centroids

	// Read dataset
	// --------------------------------------------------------------------------------

	int label[n];
	float data[n * d];
	const auto fileData = fopen(pathData, "r");
	for (int i = 0; i < n; i++)
		fscanf(fileData, "%f %f %d\n", &data[i * d], &data[i * d + 1], &label[i]);
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

	float least[d], most[d];
	const auto infinity = std::numeric_limits<float>::infinity();
	std::fill(least, least + d, infinity);
	std::fill(most, most + d, -infinity);
	for (int i = 0; i < n; i++)
		for (int j = 0; j < d; j++) {
			least[j] = std::min(least[j], data[i * d + j]);
			most[j] = std::max(most[j], data[i * d + j]);
		}
	for (int i = 0; i < n; i++)
		for (int j = 0; j < d; j++)
			data[i * d + j] = (data[i * d + j] - least[j]) / (most[j] - least[j]);

	// Do the magic
	// --------------------------------------------------------------------------------

	const auto time = high_resolution_clock::now();
	const auto [centroid, assignment] = SNNDPC(k, n, d, nc, data);
	printf("Time Cost = %lldms\n", duration_cast<milliseconds>(high_resolution_clock::now() - time).count());

	// Export centroid
	// --------------------------------------------------------------------------------

	const auto fileCentroid = fopen(SOLUTION_DIR"temp/Centroid.txt", "w");
	for (int i = 0; i < nc; i++)
		fprintf(fileCentroid, "%d\n", centroid[i]);
	fclose(fileCentroid);

	// Export assignment
	// --------------------------------------------------------------------------------

	const auto pathAssignment = SOLUTION_DIR"temp/Assignment.txt";
	const auto fileAssignment = fopen(pathAssignment, "w");
	for (int i = 0; i < n; i++)
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
