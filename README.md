# Shared-Nearest-Neighbor-Based Clustering by Fast Search and Find of Density Peaks

C++ and Python implementation of [SNNDPC](https://www.sciencedirect.com/science/article/pii/S0020025518302093) algorithm. 

The Matlab version is moved to the branch [`MatlabImplementation`](https://github.com/liurui39660/SNNDPC/tree/MatlabImplementation).

## Demo

If you use Windows:

1. Open a Visual Studio developer command prompt
    - Because of the toolchain paths
1. `cd` to project root `SNNDPC/`
1. `cmake -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" -S . -B build\release`
1. `cmake --build build\release --target Demo`
1. `build\release\Demo.exe`

The demo runs on the S2 dataset.

If CMake complains about its version, modify `SNNDPC/CMakeLists.txt:1` to fit your CMake version.

To use other datasets, see [Customization](#customization) 

## Environment

Some highlighted requirements.

- Python: 3.8, because of `:=` syntax
- IntelTBB: Optional, for parallelization
- OpenMP: Optional, for parallelization

## Customization

### Provided Dataset

To use other provided datasets in demo:

1. Modify variable `pathData` at `SNNDPC/Demo.cpp:11`.
	- Macro `SOLUTION_DIR` is the absolute path to `SNNDPC/`. 
1. Modify variables `k`, `n`, `d`, and `nc` according to the paper

	| Variable | Reference                         |
	| -------- | --------------------------------- |
	| `k`      | Table 4, column Arg-              |
	| `n`      | Table 2, column No. of records    |
	| `d`      | Table 2, column No. of attributes |
	| `nc`     | Table 2, column No. of clusters   |

### External Dataset + `Demo.cpp`

To use external datasets in `Demo.cpp`:

1. Make sure your dataset has exactly 3 columns: x, y, and label.
	- If you want to use more attributes, you need to edit the `fscanf` call at `Demo.cpp:24`.

### External Dataset + Custom Runner

To use external datasets in a custom runner:

1. Load your dataset into a (flattened) 1D C-style array `data`, shape `[n×d]`.
	- For the logical 2D array, each row is a record, and each column is an attribute.
1. Include `SNNDPC.hpp`.
1. Call the `SNNDPC()` with parameters `k`, `n`, `d`, `nc`, and `data`.
1. The function will return two pointers, the centroids and the assignment.
    - Both are a 1D array.
    - Both are created by `new`, you need to manually delete them to prevent memory leak.

## Contact

If you have any inquiries, please open an issue instead of sending emails directly to me. My email address on the paper is no longer frequently checked.

