function [result]=Dpc(data,answer,percent,varargin)
%

%Parse arguments
parser=inputParser;
parser.CaseSensitive=false;
parser.KeepUnmatched=true;
parser.StructExpand=true;
addRequired(parser,'data',@isreal);
addRequired(parser,'answer',@(answer)length(answer)==size(data,1));
addRequired(parser,'percent',@(K)K>=0&&K<=100);
addParameter(parser,'AutoPick',0,@(center)center>=0&&center<=length(answer));
addParameter(parser,'Distance',[],@(dist1)all(~diag(dist1))&&issymmetric(dist1));
addParameter(parser,'Kernel','Gaussian',@(kernel)lower(kernel(1))=='g'||lower(kernel(1))=='c');
addParameter(parser,'Ui',false);
parse(parser,data,answer,percent,varargin{:});

import Library.*

ND=size(data,1);
xx=pdist(data);
N=size(xx,2);
if isempty(parser.Results.Distance)
  dist1=squareform(xx);
else
  dist1=parser.Results.Distance;
end
% percent=2.0;

position=round(N*percent/100);
sda=sort(xx);
dc=sda(position);

for i=1:ND
  rho(i)=0.;
end
if lower(parser.Results.Kernel(1))=='g'
  %
  % Gaussian kernel
  %
  for i=1:ND-1
    for j=i+1:ND
      rho(i)=rho(i)+exp(-(dist1(i,j)/dc)*(dist1(i,j)/dc));
      rho(j)=rho(j)+exp(-(dist1(i,j)/dc)*(dist1(i,j)/dc));
    end
  end
else
  %
  % "Cut off" kernel
  %
  for i=1:ND-1
    for j=i+1:ND
      if (dist1(i,j)<dc)
        rho(i)=rho(i)+1.;
        rho(j)=rho(j)+1.;
      end
    end
  end
end

maxd=max(max(dist1));

[~,ordrho]=sort(rho,'descend');
delta(ordrho(1))=-1.;
nneigh(ordrho(1))=0;

for ii=2:ND
   delta(ordrho(ii))=maxd;
   for jj=1:ii-1
     if(dist1(ordrho(ii),ordrho(jj))<delta(ordrho(ii)))
        delta(ordrho(ii))=dist1(ordrho(ii),ordrho(jj));
        nneigh(ordrho(ii))=ordrho(jj);
     end
   end
end
delta(ordrho(1))=max(delta(:));

% scrsz = get(0,'ScreenSize');
% figure('Position',[6 72 scrsz(3)/4. scrsz(4)/1.3]);
for i=1:ND
  ind(i)=i;
  gamma(i)=rho(i)*delta(i);
end

% cl=-ones(1,ND);
% NCLUST=autoPick;
% [~,gammaOrder]=sort(gamma,'descend');
% cl(gammaOrder(1:NCLUST))=1:NCLUST;
% icl(1:NCLUST)=gammaOrder(1:NCLUST);

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
cl=-ones(1,ND);
%Id of centers
icl=[];
if click==1
  %x->rhoLeast, y->deltaLeast
  icl=intersect(find(rho>x),find(delta>y));
elseif click==2
  %y->gammaLeast
  icl=find(gamma>y);
end
%Number of clusters
NCLUST=length(icl);
cl(icl)=1:NCLUST;

%assignation
for i=1:ND
  if (cl(ordrho(i))==-1)
    cl(ordrho(i))=cl(nneigh(ordrho(i)));
  end
end
%halo
for i=1:ND
  halo(i)=cl(i);
end
if (NCLUST>1)
  for i=1:NCLUST
    bord_rho(i)=0.;
  end
  for i=1:ND-1
    for j=i+1:ND
      if ((cl(i)~=cl(j))&& (dist1(i,j)<=dc))
        rho_aver=(rho(i)+rho(j))/2.;
        if (rho_aver>bord_rho(cl(i))) 
          bord_rho(cl(i))=rho_aver;
        end
        if (rho_aver>bord_rho(cl(j))) 
          bord_rho(cl(j))=rho_aver;
        end
      end
    end
  end
  for i=1:ND
    if (rho(i)<bord_rho(cl(i)))
      halo(i)=0;
    end
  end
end
for i=1:NCLUST
  nc=0;
  nh=0;
  for j=1:ND
    if (cl(j)==i) 
      nc=nc+1;
    end
    if (halo(j)==i) 
      nh=nh+1;
    end
  end
end

cl=cl';
%Evaluation
if answer
  ami=GetAmi(answer,cl);
  ari=Library.GetAri(answer,cl);
  fmi=Library.GetFmi(answer,cl);
else
  ami=nan;
  ari=nan;
  fmi=nan;
end

%Ui
if parser.Results.Ui
  if parser.Results.AutoPick
    [fig,ax1,ax2]=UiShowDecision(rho,delta,x,y,click,nan);
  end
  data=UiReduceDimension(data);
  cmap=UiGetColormap(NCLUST);
  UiDrawMarker(NCLUST,x,y,click,nan,fig,ax1,ax2);
  UiDrawCenter(icl,rho,delta,cmap,fig,ax1,ax2);
  UiPlotResultAndRho(cl,icl,rho,data,cmap,fig,subplot(2,2,3));
  UiPlotResultAndDelta(cl,icl,delta,data,cmap,fig,subplot(2,2,4));
end

%Wrap all data into a structure
result=struct;
result.NC=NCLUST;
result.K=nan;
result.dist2=nan;
result.rho=rho;
result.delta=delta;
result.deltaSelect=nneigh;
result.gamma=gamma;
result.cluster=cl;
result.center=icl;
result.ami=ami;
result.ari=ari;
result.fmi=fmi;
result.x=x;
result.y=y;
result.click=click;

end