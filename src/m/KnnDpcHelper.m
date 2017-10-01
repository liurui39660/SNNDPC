close all;clear;

%Read dataset from files
[data,answer,dist1]=ReadDataset();

%Execution
% K=5:30;
K=9;
for p=1:length(K)
  %If you want to choose centers manually, set AutoPick to 0, otherwise, number of centers
  result(p)=KnnDpc(data,answer,K(p),'AutoPick',3,'Distance',dist1,'Ui',true);
%   fprintf('K=%i,AMI=%f,ARI=%f,FMI=%f\n',K(p),result(p).ami,result(p).ari,result(p).fmi);
end

dashboard=table([result(:).ami]',[result(:).ari]',[result(:).fmi]',K','VariableNames',{'AMI','ARI','FMI','K'});
resultBest=dashboard(dashboard.ARI==max([result(:).ari]),:)
