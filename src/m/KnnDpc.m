function [result]=KnnDpc(data,answer,K,varargin)
%

%Parse arguments
parser=inputParser;
parser.CaseSensitive=false;
parser.KeepUnmatched=true;
parser.StructExpand=true;
addRequired(parser,'data',@isreal);
addRequired(parser,'answer',@(answer)length(answer)==size(data,1));
addRequired(parser,'K',@(K)K>=0&&K<=length(answer));
addParameter(parser,'AutoPick',0,@(center)center>=0&&center<=length(answer));
addParameter(parser,'Distance',[],@(dist1)all(~diag(dist1))&&issymmetric(dist1));
addParameter(parser,'Ui',false);
parse(parser,data,answer,K,varargin{:});

import java.util.LinkedList
import Library.*

N=size(data,1);

%Normalization
data=(data-min(data))./(max(data)-min(data));
data(isnan(data))=0;

%Calculate dist1
if isempty(parser.Results.Distance)
  dist1=squareform(pdist(data));
else
  dist1=parser.Results.Distance;
end

%Calculate \rho
[dist1Sort,dist1Order]=sort(dist1,2);
rho=sum(exp(-dist1Sort(:,2:K)),2)';

%Calculate \delta
delta=inf(1,N);
deltaSelect=zeros(1,N);
[~,rhoOrder]=sort(rho,'descend');
for p=2:N
  for o=1:p-1
    if dist1(rhoOrder(p),rhoOrder(o))<delta(rhoOrder(p))
      delta(rhoOrder(p))=dist1(rhoOrder(p),rhoOrder(o));
      deltaSelect(rhoOrder(p))=rhoOrder(o);
    end
  end
end
delta(rhoOrder(1))=-1;
delta(rhoOrder(1))=max(delta);

%Calculate \gamma
gamma=rho.*delta;

%Ui
%If AutoPick is set, I assume you want to make a benchmark and will decide centers automatically
if parser.Results.AutoPick
  gammaSort=sort(gamma,'descend');
  x=nan;
  y=mean(gammaSort(parser.Results.AutoPick:parser.Results.AutoPick+1));
  click=2;
else
  [x,y,click,fig,ax1,ax2]=UiMakeDecision(rho,delta);
end

%Assign center
%Cluster to which it belongs, notate unassigned in -1
cluster=-ones(1,N);
%Id of centers
center=[];
if click==1
  %x->rhoLeast, y->deltaLeast
  center=intersect(find(rho>x),find(delta>y));
elseif click==2
  %y->gammaLeast
  center=find(gamma>y);
end
%Number of clusters
NC=length(center);
cluster(center)=1:NC;

%Find outlier
theta=mean(dist1Sort(:,K));
%0 means outlier
cluster(dist1Sort(:,K)>theta)=0;

%Assign strategy 1
queue=LinkedList;
for p=center
  %1 is itself
  cluster(dist1Order(p,2:K))=cluster(p);
  for o=dist1Order(p,2:K)
    queue.offerFirst(o);
  end
  while ~queue.isEmpty()
    this=queue.pollLast();
    for next=dist1Order(this,2:K)
      if cluster(next)<0&&dist1(this,next)<=mean(dist1Sort(next,2:K))
        cluster(next)=cluster(this);
        queue.offerFirst(next);
      end
    end
  end
end

%Assign strategy 2
unas=find(cluster<0);
unasCount=length(unas);
while true
  recog=zeros(unasCount,NC);
  for p=1:unasCount
    for o=2:K
      u=cluster(dist1Order(unas(p),o));
      if u>0
        recog(p,u)=recog(p,u)+1;
      end
    end
  end
  whichK=max(recog(:));
  if whichK==K
    [whichPoint,whichCluster]=ind2sub(size(recog),find(recog==whichK));
    cluster(unas(whichPoint))=whichCluster;
    unas=find(cluster<0);
    unasCount=length(unas);
  elseif whichK>0
    [whichPoint,whichCluster]=ind2sub(size(recog),randsample(find(recog==whichK),1));
    cluster(unas(whichPoint))=whichCluster;
    unas=find(cluster<0);
    unasCount=length(unas);
  else
    break;
  end
end

%Assign outlier
unas=find(cluster<=0);
for p=unas
  %1 is itself
  for o=dist1Order(p,:)
    if cluster(o)>0
      cluster(p)=cluster(o);
      break;
    end
  end
end

%Remain same shape with answer
center=center';
cluster=cluster';

%Evaluation
if answer
  ami=GetAmi(answer,cluster);
  ari=Library.GetAri(answer,cluster);
  fmi=Library.GetFmi(answer,cluster);
else
  ami=nan;
  ari=nan;
  fmi=nan;
end

%Ui
if parser.Results.Ui
  if parser.Results.AutoPick
    [fig,ax1,ax2]=UiShowDecision(rho,delta,x,y,click,K);
  end
  data=UiReduceDimension(data);
  cmap=UiGetColormap(NC);
  UiDrawMarker(NC,x,y,click,K,fig,ax1,ax2);
  UiDrawCenter(center,rho,delta,cmap,fig,ax1,ax2);
  UiPlotResultAndRho(cluster,center,rho,data,cmap,fig,subplot(2,2,3));
  UiPlotResultAndDelta(cluster,center,delta,data,cmap,fig,subplot(2,2,4));
end

%Wrap all data into a structure
result=struct;
result.NC=NC;
result.K=K;
result.dist2=nan;
result.rho=rho;
result.delta=delta;
result.deltaSelect=deltaSelect;
result.gamma=gamma;
result.cluster=cluster;
result.center=center;
result.ami=ami;
result.ari=ari;
result.fmi=fmi;
result.x=x;
result.y=y;
result.click=click;

end