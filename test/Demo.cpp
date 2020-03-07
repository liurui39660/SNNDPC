#include <chrono>

#include "CPU/Classifier.h"
#include "GPU/Classifier.h"
#include "util/ExportVariable.h"

using namespace std::chrono;
using namespace SNNDPC;

int main(int argc, char* argv[]) {
	// const auto pathDatabase = SOLUTION_DIR"data/Flame.tsv";
	// const int k = 5, n = 240, d = 2, nc = 2;
	// const auto pathDatabase = SOLUTION_DIR"data/S2.tsv";
	// const int k = 35, n = 5000, d = 2, nc = 15;
	const auto pathDatabase = SOLUTION_DIR"data/Birch1.tsv";
	const int k = 35, n = 100000, d = 2, nc = 100;
	// --------------------------------------------------------------------------------
	double data[n * d];
	int truth[n], centroid[nc], assignment[n];;
	FILE* file;
	fopen_s(&file, pathDatabase, "r");
	for (int i = 0; i < n; i++)
		fscanf_s(file, "%lf %lf %d\n", &data[i * d], &data[i * d + 1], &truth[i]);
	fclose(file);
	const auto pathTruth = SOLUTION_DIR"temp/truth.txt";
	ExportVariable(pathTruth, truth, n);
	const auto time = high_resolution_clock::now();
	CONFIGURATION::Classifier<double>{k, n, d, nc, Option{.useEarlyRelease = true}}.Run(data, centroid, assignment);
	printf("Time Cost = %lldms\n", duration_cast<milliseconds>(high_resolution_clock::now() - time).count());
	const auto pathPredict = SOLUTION_DIR"temp/predict.txt";
	ExportVariable(SOLUTION_DIR"temp/centroid.txt", centroid, nc);
	ExportVariable(pathPredict, assignment, n);
	char command[1u << 10u];
	sprintf_s(command, R"(python "%s" "%s" "%s")", SOLUTION_DIR"util/EvaluateAssignment.py", pathTruth, pathPredict);
	system(command);
}
