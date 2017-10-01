function [data,answer]=OlivettiReader(width,height,pcaPercentage)
%

%Disable useless warning: file contains extra data
warning('off','all');

%Main process
data=zeros(400,width*height);
answer=zeros(400,1);
rowCount=0;
loc='..\..\res\ClusteringDataset\Realworld\OlivettiFaces\';
folderList=dir(loc);
for p=1:length(folderList)
  folder=folderList(p);
  if folder.name(1)=='s'
    fileList=dir(strcat(loc,folder.name));
    for o=1:length(fileList)
      file=fileList(o);
      if ~file.isdir
        rowCount=rowCount+1;
        %Read, resize, reshape, assign
        data(rowCount,:)=reshape(imresize(imread(strcat(loc,folder.name,'\',file.name)),[height,width]),1,width*height);
        answer(rowCount)=str2double(folder.name(2:end));
      end
    end
  end
end

%Apply Pca to decrease dimension
if pcaPercentage
  [~,score,~,~,explained]=pca(data);
  data=score(:,1:find(cumsum(explained)<=pcaPercentage,1,'last'));
end

%Save to file
save ..\..\res\ClusteringDataset\Realworld\OlivettiFaces\OlivettiFaces.data data -ascii
save ..\..\res\ClusteringDataset\Realworld\OlivettiFaces\OlivettiFaces.answer answer -ascii

%Enable disabled warning
warning('on','all');

end