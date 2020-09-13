# Shared-Nearest-Neighbor-Based Clustering by Fast Search and Find of Density Peaks

C++ and Python implementation of [SNNDPC](https://www.sciencedirect.com/science/article/pii/S0020025518302093) algorithm. 

The Matlab version is moved to the branch [`MatlabImplementation`](https://github.com/liurui39660/SNNDPC/tree/MatlabImplementation).

## Demo

If you use Windows:

1. Open a Visual Studio developer command prompt (prefer x64 native)
1. `cd` to project root `SNNDPC/`
1. `cmake -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" -S . -B build\release`
1. `cmake --build build\release --target Demo`
1. `build\release\src\Demo.exe`

The demo runs on the S2 dataset with OpenMP enabled.

If CMake complains about its version, modify `SNNDPC/CMakeLists.txt:1` to fit your CMake version.

To use other datasets, see [Customization](#customization) 

## Environment

Below is my development environment.

- CMake: 3.16.3
- MSVC: v142, x64, c++20
- Windows 10 SDK: 10.0.18362.0
- Python: 3.7.4
	- For SNNDPC algorithm only:
		- `numpy`: 1.17.4
		- `scipy`: 1.3.2
	- For evaluating assignments:
		- `numpy`: 1.17.4
		- `scikit-learn`: 0.21.3, for AMI, ARI and FMI.
- Vcpkg: Nightly build from source
	- Optional, only TBB actually uses it
- TBB: 2020_U1, via vcpkg
	- Optional, to replace OpenMP
- NVCC: v10.2.89
	- Optional, if you want to try GPU mode

## Customization

### Provided Dataset

To use other provided datasets in demo:

1. Modify variable `pathDatabase` at `SNNDPC/test/Demo.cpp:13`.
	- Macro `SOLUTION_DIR` is the absolute path to `SNNDPC/`. 
	- Macro `PROJECT_DIR` is the absolute path to `SNNDPC/CPU/` or `SNNDPC/GPU/`
1. Modify variables `k`, `n`, `d`, and `nc` according to the paper

	| Variable | Reference                         |
	| -------- | --------------------------------- |
	| `k`      | Table 4, column Arg-              |
	| `n`      | Table 2, column No. of records    |
	| `d`      | Table 2, column No. of attributes |
	| `nc`     | Table 2, column No. of clusters   |

### External Dataset + `Demo.cpp`

To use external datasets in `Demo.cpp`:

1. Make sure your dataset has exactly 3 columns: x, y, and ground-truth cluster.
	- If you want to use more attributes, you need to edit the `fscanf_s` call at `Demo.cpp:21`.
	- If your dataset contains scientific notations, e.g. 3.5e-2, there's no guarantee it can be successfully read.

### External Dataset + Custom Runner

To use external datasets in a custom runner on CPU:

1. Load your dataset into a (flattened) 1D C-style array, shape `[n×d]`.
	- For the logical 2D array, each row is a record, and each column is an attribute.
1. Include `CPU/Classifier.h`
1. Instantiate an `SNNDPC::CPU::Classifier<T>` with parameters `k`, `n`, `d`, `nc`, and an optional [`SNNDPC::Option`](util/Option.h)
	- `T` is the type of your data array, can be `float` or `double`, for additional types, see [Additional Data Type](#additional-data-type).
1. Call method `.Run()` with the input data array, output centroid array, and output assignment array as parameters.
	- There's no guarantee the data array won't be changed, check `SNNDPC::Option.shallCopyInput` for confirmation.
	- The two output arrays should be able to store at least `n` integers, where `n` is the number of records in the dataset.
	- The assignments start from 0, and the largest value should be `nc-1`.
1. (Optional) Save your results to local files.
	- You can use the provided function `SNNDPC::ExportVariable()`, parameters are
		- A string of the full path to the file to be created, if it already exists, it will be overwritten without notice.
		- A (flattened) 1D array of any type.
		- If it's logically a 2D array, the 3rd and 4th parameters are the number of rows and columns, respectively.
		- If it's logically a 1D array, the 3rd parameter is the number of elements, and no need to specify the 4th parameter.

### Additional Data Type

To add other numerical data types:

1. Add `template class Classifier<T>` above the last closing brace, where `T` is the intended type.

## Contact

If you have any inquiries, please open an issue instead of sending emails directly to me. My email address on the paper is no longer frequently checked.

