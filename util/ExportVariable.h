#pragma once

#include <string>

namespace SNNDPC {
template<typename T>
void ExportVariable(const char* filename, const T* var, int dim1, int dim2 = 1) {
	std::string format;
	if (std::is_same<T, char>::value || std::is_same<T, unsigned char>::value) format = "%hhd\t";
	else if (std::is_same<T, short>::value || std::is_same<T, unsigned short>::value) format = "%hd\t";
	else if (std::is_same<T, int>::value || std::is_same<T, unsigned int>::value) format = "%d\t";
	else if (std::is_same<T, long>::value || std::is_same<T, unsigned long>::value) format = "%ld\t";
	else if (std::is_same<T, long long>::value || std::is_same<T, unsigned long long>::value) format = "%lld\t";
	else if (std::is_same<T, float>::value) format = "%.4f\t";
	else if (std::is_same<T, double>::value) format = "%.4lf\t";
	else if (std::is_same<T, long double>::value) format = "%.4Lf\t";
	else throw; // Not implemented

	FILE* file;
	fopen_s(&file, filename, "w");
	for (int i = 0; i < dim1; i++) {
		for (int j = 0; j < dim2; j++)
			fprintf(file, format.c_str(), var[i * dim2 + j]);
		fprintf(file, "\n");
	}
	fclose(file);
}

template<typename T>
void ExportVariable(const std::string& filename, const T* var, int dim1, int dim2 = 1) {
	ExportVariable(filename.c_str(), var, dim1, dim2);
}
}
