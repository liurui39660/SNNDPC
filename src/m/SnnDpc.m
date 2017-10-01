function [result]=SnnDpc(data,answer,K,varargin)
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

%Number of points
N=size(data,1);

%(Not Recommended)If K==0, assign it automatically and show its value
if ~K
  K=round(N/30)
end

%Normalization
data=(data-min(data))./(max(data)-min(data));
data(isnan(data))=0;

%Calculate dist1
if isempty(parser.Results.Distance)
  dist1=squareform(pdist(data));
else
  dist1=parser.Results.Distance;
end

% Calculate dist2
[dist1Sort,dist1Order]=sort(dist1,2);
%ismembc() needs sorted arrays
dist1OrderSort=sort(dist1Order(:,1:K),2);
dist2=zeros(N);
shared=cell(N);
sharedCount=zeros(N);
for p=1:N
  %Use distance to determine whether it is neighbor
  isNeighbor=dist1(p+1:N,dist1OrderSort(p,:))<=dist1Sort(p+1:N,K);
  for o=p+1:N
    %Faster version of intersect(), but still slower than the line below
%     shared{p,o}=dist1OrderSort(p,ismembc(dist1OrderSort(p,:),dist1OrderSort(o,:)));
    shared{p,o}=dist1OrderSort(p,isNeighbor(o-p,:));
    sharedCount(p,o)=length(shared{p,o});
    %ismembc() is much faster, but still slower than the line below
%     if ismembc([p,o],shared{p,o})
    if dist1(p,o)<min(dist1Sort(p,K+1),dist1Sort(o,K+1))
%       Previously, there is no square, and is not sum() but mean()
      dist2(p,o)=sharedCount(p,o)^2/sum(dist1(p,shared{p,o})+dist1(o,shared{p,o}));
    end
  end
end
dist2=dist2+dist2';
sharedCount=sharedCount+sharedCount';

%Calculate \rho
dist2Sort=sort(dist2,2,'descend');
rho=sum(dist2Sort(:,1:K),2)';

%Calculate \delta
delta=inf(1,N);
deltaSelect=zeros(1,N);
[~,rhoOrder]=sort(rho,'descend');
%Sum in advance to speed up
dist1SortSum=sum(dist1Sort(:,1:K),2);
for p=2:N
  for o=1:p-1
    deltaCandi=dist1(rhoOrder(p),rhoOrder(o))*(dist1SortSum(rhoOrder(p))+dist1SortSum(rhoOrder(o)));
    if deltaCandi<delta(rhoOrder(p))
      delta(rhoOrder(p))=deltaCandi;
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

%Assign non-center, inevitable subordinate points
queue=LinkedList;
for p=center
  queue.offerFirst(p);
end
while ~queue.isEmpty()
  this=queue.pollLast();
  for next=dist1Order(this,2:K)
    if cluster(next)<0&&sharedCount(this,next)>=K/2
      cluster(next)=cluster(this);
      queue.push(next);
    end
  end
end

%Assign non-center, possible subordinate points
KBackup=K;
%unassigned points
unas=find(cluster<0);
unasCount=length(unas);
while unasCount
  %recognition matrix, details on the article of KnnDpc
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
  if whichK
    [whichPoint,whichCluster]=ind2sub(size(recog),find(recog==whichK));
    cluster(unas(whichPoint))=whichCluster;
    unas=find(cluster<0);
    unasCount=length(unas);
  else
    K=K+1;
  end
end

%Remain same shape with answer
if ~iscolumn(center)
  center=center';
end
if ~iscolumn(cluster)
  cluster=cluster';
end

%Evaluation
if ~isempty(answer)
  ami=GetAmi(answer,cluster);
  ari=GetAri(answer,cluster);
  fmi=GetFmi(answer,cluster);
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
result.K=KBackup;
result.dist2=dist2;
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