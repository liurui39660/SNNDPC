close all;clear;

%Read dataset from files
[data,answer,dist1]=ReadDataset();

%Execution
% percent=0.1:0.1:5;
percent=3.8;
for p=1:length(percent)
  %If you want to choose centers manually, set AutoPick to 0, otherwise, number of centers
  result(p)=Dpc(data,answer,percent(p),'AutoPick',3,'Distance',dist1,'Ui',true,'Kernel','Gaussian');
%   fprintf('percent=%f,AMI=%f,ARI=%f,FMI=%f\n',percent(p),result(p).ami,result(p).ari,result(p).fmi);
end

dashboard=table([result(:).ami]',[result(:).ari]',[result(:).fmi]',percent','VariableNames',{'AMI','ARI','FMI','Ratio'});
resultBest=dashboard(dashboard.ARI==max([result(:).ari]),:)
