classdef Library
  %LIBRARY Summary of this class goes here
  %   Detailed explanation goes here
  
  properties
  end
  
  methods(Static)
    
    %Data processing functions
    
    function [varargout]=Swap(varargin)
      varargout=varargin;
    end
    
    function [ari]=GetAri(answer,cluster)
      [p,o]=meshgrid(answer,answer);
      answerPair=~squareform(p~=o);
      [p,o]=meshgrid(cluster,cluster);
      resultPair=~squareform(p~=o);
      a=sum(answerPair&resultPair);
      b=sum(answerPair&~resultPair);
      c=sum(~answerPair&resultPair);
      d=sum(~answerPair&~resultPair);
      ari=2*(a*d-b*c)/((a+b)*(b+d)+(a+c)*(c+d));
    end
    
    function [fmi]=GetFmi(answer,cluster)
      [p,o]=meshgrid(answer,answer);
      answerPair=~squareform(p~=o);
      [p,o]=meshgrid(cluster,cluster);
      resultPair=~squareform(p~=o);
      tp=sum(answerPair&resultPair);
      fp=sum(answerPair&~resultPair);
      fn=sum(~answerPair&resultPair);
      fmi=tp/sqrt((tp+fp)*(tp+fn));      
    end
    
    %User interface functions

    function [fig]=UiShowFigure()
      %Calculate where to display figure window
      screenSize=get(0,'ScreenSize');
      fig=figure('Position',[36 72 screenSize(3)/2.3 screenSize(4)/1.3]);
    end
    
    function [data]=UiReduceDimension(data)
      if size(data,2)>2
        rng default
        data=tsne(data);
      end
    end
    
    function [cmap]=UiGetColormap(NC)
      %Get colormap for plotting below
      colormap hsv
      cmap=colormap;
      cmap=cmap(round(linspace(1,length(cmap),NC+1)),:);
      cmap=cmap(1:end-1,:)*0.5+0.4;
    end

    function [x,y,click,fig,ax1,ax2]=UiMakeDecision(rho,delta,varargin)
      %Specify figure and axes
      if nargin<3
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      set(0,'CurrentFigure',fig);
      if nargin<5
        ax1=subplot(2,2,1,'Parent',fig);
        ax2=subplot(2,2,2,'Parent',fig);
      else
        ax1=varargin{2};
        ax2=varargin{3};
      end
      %Plot decision graph
      scatter(ax1,rho,delta,'.');
      title(ax1,'Decision Graph','FontSize',15);xlabel(ax1,'\rho');ylabel(ax1,'\delta');
      grid(ax1,'on');
      %Show hints
      fprintf('Click on Decision Graph to determine the lower bound of rho and delta\n');
      fprintf('Or click on Gamma Graph to determine the lower bound of gamma\n');
      %Plot gamma graph
      Library.UiPlotGamma(rho.*delta,fig,ax2);
      %Make decision
      [x,y]=ginput(1);
      if gca==ax1
        click=1;
      elseif gca==ax2
        click=2;
      else
        error('You must click at either graph');
      end
    end
        
    function [fig,ax1,ax2]=UiShowDecision(rho,delta,x,y,click,K,varargin)
      if nargin<7
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<9
        ax1=subplot(2,2,1,'Parent',fig);
        ax2=subplot(2,2,2,'Parent',fig);
      else
        ax1=varargin{2};
        ax2=varargin{3};
      end
      scatter(ax1,rho,delta,'.');
      pbaspect(ax1,[1,1,1]);
      title(ax1,'Decision Graph','FontSize',15);xlabel(ax1,'\rho');ylabel(ax1,'\delta');
      grid(ax1,'on');
      Library.UiPlotGamma(rho.*delta,fig,ax2);
      if click==1
        NC=sum(rho>x&delta>y);
        Library.UiDrawMarker(NC,x,y,click,K,fig,ax1,ax2);
        Library.UiDrawCenter(intersect(find(rho>x),find(delta>y)),rho,delta,Library.UiGetColormap(NC),fig,ax1,ax2);
      else
        gamma=rho.*delta;
        Library.UiDrawMarker(sum(gamma>y),x,y,click,K,fig,ax1,ax2);
        Library.UiDrawCenter(find(gamma>=y),rho,delta,Library.UiGetColormap(sum(gamma>y)),fig,ax1,ax2);
      end
    end
    
    function [fig,ax1,ax2]=UiDrawMarker(NC,x,y,click,K,varargin)
      %Draw markers on decision graph and gamma graph
      if nargin<6
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<8
        ax1=subplot(2,2,1,'Parent',fig);
        ax2=subplot(2,2,2,'Parent',fig);
      else
        ax1=varargin{2};
        ax2=varargin{3};
      end
      if click==1
        %x->rhoLeast,y->deltaLeast
        line(ax1,[x,ax1.XLim(2)],[y,y],'Color','Red');
        line(ax1,[x,x],[y,ax1.YLim(2)],'Color','Red');
        line(ax2,ax2.XLim,[x*y,x*y],'Color','Red');
        text(ax2,(ax2.XLim(2)-ax2.XLim(1))*0.1,(ax2.YLim(2)-ax2.YLim(1))*0.8,sprintf('NC = %i, K = %i',NC,K),'FontSize',10);
      elseif click==2
        %y->gammaLeast
        line(ax2,ax2.XLim,[y,y],'Color','Red');
        text(ax2,(ax2.XLim(2)-ax2.XLim(1))*0.1,(ax2.YLim(2)-ax2.YLim(1))*0.8,sprintf('NC = %i, K = %i',NC,K),'FontSize',10);
      end
    end

    function [fig,ax1,ax2]=UiDrawCenter(center,rho,delta,cmap,varargin)
      %Plot centers on decision graph
      if nargin<5
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<7
        ax1=subplot(2,2,1,'Parent',fig);
        ax2=subplot(2,2,2,'Parent',fig);
      else
        ax1=varargin{2};
        ax2=varargin{3};
      end
      gamma=rho.*delta;
      [~,gammaOrder]=sort(gamma);
      [~,gammaOrderReverse]=sort(gammaOrder);
      hold(ax1,'on');
      scatter(ax1,rho(center),delta(center),[],cmap,'o','Filled');
      hold(ax1,'off');
      hold(ax2,'on');
      scatter(ax2,gammaOrderReverse(center),gamma(center),[],cmap,'o','Filled');
      hold(ax2,'off');
    end
            
    function [fig,ax]=UiPlotGamma(gamma,varargin)
      %Plot \gamma graph
      if nargin<2
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<3
        ax=axes(fig);
      else
        ax=varargin{2};
      end
      N=length(gamma);
      gammaSort=sort(gamma);
      scatter(ax,1:N,gammaSort,'.');
      pbaspect(ax,[1,1,1]);
      title(ax,'Distribution of \gamma value','FontSize',15);xlabel(ax,'Number');ylabel(ax,'\gamma');
      grid(ax,'on');
    end
    
    function [fig,ax]=UiPlotResultAndRho(cluster,center,rho,data,cmap,varargin)
      %Plot clustering result and \rho graph
      if nargin<6
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<7
        ax=axes(fig);
      else
        ax=varargin{2};
      end
      NC=length(center);
      hold(ax,'on');
      grid(ax,'on');
      scatter3(ax,data(cluster<0,1),data(cluster<0,2),rho(cluster<0),[],[0.6,0.6,0.6],'x');
      for p=1:NC
        scatter3(ax,data(cluster==p,1),data(cluster==p,2),rho(cluster==p),[],cmap(p,:),'.');
      end
      scatter3(ax,data(center,1),data(center,2),rho(center),40,cmap,'Pentagram','Filled','MarkerEdgeColor','Black');
      pbaspect(ax,[1,1,1]);
      title(ax,'Result and \rho value (3D)','FontSize',15);xlabel('X');ylabel('Y');zlabel('\rho')
      xlabel('X');
      ylabel('Y');
      zlabel('\rho');
      hold(ax,'off');
    end
    
    function [fig,ax]=UiPlotResultAndDelta(cluster,center,delta,data,cmap,varargin)
      %Plot clustering result and \delta graph
      if nargin<6
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<7
        ax=axes(fig);
      else
        ax=varargin{2};
      end
      NC=length(center);
      hold(ax,'on');
      grid(ax,'on');
      scatter3(ax,data(cluster<0,1),data(cluster<0,2),delta(cluster<0),[],[0.6,0.6,0.6],'x');
      for p=1:NC
        scatter3(ax,data(cluster==p,1),data(cluster==p,2),(delta(cluster==p)),[],cmap(p,:),'.');
      end
      scatter3(ax,data(center,1),data(center,2),delta(center),40,cmap,'Pentagram','Filled','MarkerEdgeColor','Black');
      pbaspect(ax,[1,1,1]);
      title(ax,'Result and \delta value (3D)','FontSize',15);xlabel('X');ylabel('Y');zlabel('\delta');
      xlabel('X');
      ylabel('Y');
      zlabel('\delta');
      hold(ax,'off');
    end
    
    function [fig,ax1,ax2,ax3]=UiReshowDecision(result,data,varargin)
      if nargin<3
        screenSize = get(0,'ScreenSize');
        fig=figure('Position',[36 72 screenSize(3)/1.5 screenSize(4)/3]);
      else
        fig=varargin{1};
      end
      if nargin<5
        ax1=subplot(1,3,1,'Parent',fig);
        ax2=subplot(1,3,2,'Parent',fig);
        ax3=subplot(1,3,3,'Parent',fig);
      else
        ax1=varargin{2};
        ax2=varargin{3};
        ax3=varargin{4};
      end
      [fig,ax1]=Library.UiReplotResultAndRho(result,data,fig,ax1);
      title(ax1,'Result');
      [fig,ax2,ax3]=Library.UiShowDecision(result.rho,result.delta,result.x,result.y,result.click,result.K,fig,ax2,ax3);
    end
    
    function [fig,ax]=UiReplotGamma(result,varargin)
      if nargin<2
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<3
        ax=axes(fig);
      else
        ax=varargin{2};
      end
      [fig,ax]=Library.UiPlotGamma(result.gamma,fig,ax);
    end
    
    function [fig,ax]=UiReplotResultAndRho(result,data,varargin)
      if nargin<3
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<4
        ax=axes(fig);
      else
        ax=varargin{2};
      end
      [fig,ax]=Library.UiPlotResultAndRho(result.cluster,result.center,result.rho,data,Library.UiGetColormap(result.NC),fig,ax);
    end
    
    function [fig,ax]=UiReplotResultAndDelta(result,data,varargin)
      if nargin<3
        fig=Library.UiShowFigure();
      else
        fig=varargin{1};
      end
      if nargin<4
        ax=axes(fig);
      else
        ax=varargin{2};
      end
      [fig,ax]=Library.UiPlotResultAndDelta(result.cluster,result.center,result.delta,data,Library.UiGetColormap(result.NC),fig,ax);
    end    

  end
  
end
