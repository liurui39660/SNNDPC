function [data,answer,dist1]=ReadDataset()
%Enter file name manually
% data=load(input('File name of source data? (with quotes)\n'));
% answer=load(input('File name of ground truth? (with quotes)\n'));

%Load synthetic dataset
data=load("..\..\res\ClusteringDataset\Synthetic\ShapeSets\pathbased.txt");
answer=data(:,end);
data=data(:,1:end-1);
% data=load("..\..\res\ClusteringDataset\Synthetic\DimSetsHigh\dim512.txt");
% answer=load("..\..\res\ClusteringDataset\Synthetic\DimSetsHigh\dim512.pa");
% data=load("..\..\res\ClusteringDataset\Synthetic\ASets\a1.txt");
% answer=load("..\..\res\ClusteringDataset\Synthetic\ASets\a1-ga.pa");
% data=load("..\..\res\ClusteringDataset\Synthetic\SSets\s2.txt");
% answer=load("..\..\res\ClusteringDataset\Synthetic\SSets\s2-label.pa");

%Load builtin dataset
% data=load('fisheriris');
% answer=grp2idx(categorical(data.species));
% data=data.meas;

%Real-world datasets are of various formats, you need to write your own code.
%Your code need to declare two variables, which are:
%"data" for original data w/o labels, e.g. X and Y coordinate for each column
%"answer" for labels, must be numerical. To convert, use "answer=grp2idx(categorical(answer));"
% data=load("..\..\res\ClusteringDataset\Realworld\Ecoli\ecoli.data");
% answer=data(:,end);
% data=data(:,1:end-1);

%Load OlivettiFaces dataset
% [data,answer]=OlivettiReader(15,15,90);

%PCA
% pcaPercentage=99.8;
% [~,score,~,~,explained]=pca(data);
% data=score(:,1:find(cumsum(explained)<=pcaPercentage,1,'last'));

%Pre-calculate distance matrix
%If you applies PCA in previous step, don't perform min-max normalization again.
data=(data-min(data))./(max(data)-min(data));
data(isnan(data))=0;
dist1=squareform(pdist(data));
end