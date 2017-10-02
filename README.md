# Shared Nearest Neighbor-based Clustering by Fast Search and Find of Density Peaks 

## Requirement

The source code is written by Matlab r2017a. The versions lower than it have not been tested.

Some functions like `squareform()` and `pdist()` require "Statistics and Machine Learning Toolbox" product.

## Simplest Demo

There are 3 entrance file, `SnnDpcHelper.m`, `KnnDpcHelper.m` and `DpcHelper.m`.
To run the simplest demo which shows the best result of three clustering algorithms on dataset Pathbased, you just need to open one of the three source file in Matlab, then click "Run" in the "Editor' tab.

## Customization

All modification should only be made in the entrance files and `ReadDataset.m`.

### Basic Parameters

For all three algorithm, the parameter (`K` or `percent`) is assigned at the line 7,8 in each entrance file. 

The parameter should be a single number or a vector array. 

The commented line 7 in each entrance file indicates a recommended range of parameter if you need a loop to find a better result.

### Dataset

To use a new dataset instead of the default one, you need to edit `ReadDataset.m`.

#### Those Provided in the Repository

For shape sets, you only need to modify the file name (not the directory path or suffix) at the line 7, case insensitive. But if you want to use other datasets, you need to comment line 7-9 before following the instruction below.

For other synthetic datasets (DIM512, A1 and S2), uncomment the line 10-11, 12-13, 14-15, respectively. Further modification is not necessary.

For UCI real-world datasets, you need to refer to another file `/SnnDpc/res/ClusteringDataset/ReadRealworld.txt`. This file is arranged in the format below

>Dataset name (1 line)<br/>
>Number of clusters (1 line)<br/>
>How to read this dataset (2 or 3 lines)

You need to copy the code from the part *How to read this dataset* to `ReadDataset.m`. Line 26-28 is an example to read the dataset Wine.

For datset Olivetti Faces, we provide a function to gather original data and answer. At the line 31, it require 3 parameters: `width` and `heigh` for the needs to scale the original photos, `pcaPercentage` for principal component analysis (PCA) to keep the componets account for `pcaPercentage`% of the variance.

#### Those Provided by You

The function `ReadDataset()` returns 2 required variables `data` and `answer` and 1 optional variable `dist1`.

The `data` is a table-like matrix where each row represents an entry and each column represents an attribute. Notice that `answer` shouldn't be included in `data`

The `answer` is a numerical vector array representing labels. We recommend that it should range only from 1 to the number of clusters.

The `dist1` is calculated at the line 40-42. Since the Euclidean distance is a constant for any two points in the dataset, if we pre-compute the distance matrix and directly pass it to the clustering algorithm, time for calculating it in every function call would be saved.

In case that you need to perform PCA for your own dataset, you can uncomment the line 34-36 and comment the line 40. The `pcaPercentage` is same as the third parameter passed to `OlivettiReader()`.

There is another method to read your own dataset. At the line 3-4, you can direct enter the file name of your dataset and labels, as long as they already satisfy the format requirement for `data` and `answer`.

### Advanced Parameters

In this section, we focus on other parameters passed to the algorithm at the line 11 in each entrance file.

#### AutoPick

It indicates how many centers should be automatically chosen. If omitted or assigned with 0, you can choose centers manually from the rho-delta decision graph or sorted gamma graph.

For manually choosing, if you click on the decision graph, the points at the upper right of where you click will be marked as centers, and a red horizontal line will be drawn on the gamma graph indicating the gamma value of where you just click.

If you click on the gamma graph, points with higher gamma value than where you click will be marked as centers, and a red line will also be drawn.

#### Distance

You can pass the pre-computed distance matrix to the algorithm to save time from repeated calculations. 

The default value is `[]`, which means the algorithm needs to calculate the distance matrix everytime it is called.

It will check whether the matrix passed is symmetric and the distance to a point itself is 0. If not, it will raise an error.

#### Ui

If true, a figure with 4 subgraph will be displayed after each call to `SnnDpc()`, `KnnDpc()` or `Dpc()`. It is usually applied when `K` is a constant (including 0) and `AutoPick` is 0.

If false, nothing will be displayed after each call. It is usually applied when `K` is an array and `AutoPick` is not 0.