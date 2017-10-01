function [image]=OlivettiWriter(cluster,center,height,width)
%

%If new size isn't assigned, it won't resize
if nargin<3
  height=112;
  width=92;
end

%Read images begin
%Disable useless warning: file contains extra data
warning('off','all');

%Main process
image=uint8(zeros(40*height,10*width));
loc='..\..\res\ClusteringDataset\Realworld\OlivettiFaces\';
folderList=dir(loc);
folderCount=0;
for p=1:length(folderList)
  folder=folderList(p);
  if folder.name(1)=='s'
    folderCount=folderCount+1;
    fileList=dir(strcat(loc,folder.name));
    fileCount=0;
    for o=1:length(fileList)
      file=fileList(o);
      if ~file.isdir
        fileCount=fileCount+1;
        singleImage=imresize(imread(strcat(loc,folder.name,'\',file.name)),[height,width]);
        image((folderCount-1)*height+1:folderCount*height,(fileCount-1)*width+1:fileCount*width)=singleImage;
      end
    end
  end
end

%Enable disabled warning
warning('on','all');

%Show result begin
%If a person is classified into more than one cluster, the cluster whose number of elements 
%(for this person) is greater than 'least' will be shown, and others will remain grayscale
for p=1:40
  cluster((p-1)*10+1:p*10)=Filter(cluster((p-1)*10+1:p*10),4);
end

%Recode the id of each cluster in ascending order
cluster=Recode(cluster);

%Obtain hue(color) map
NC=max(cluster);
hueCountHalf=ceil(NC/4);
hue=[1:hueCountHalf;hueCountHalf+1:2*hueCountHalf];
hue=hue(:)';
hue=1/(2*hueCountHalf)*hue;

%Convert grayscale into hsv
image=cat(3,zeros(size(image)),double(image)/255/2,double(image)/255);

for p=1:400
  [sub(2),sub(1)]=ind2sub([10,40],p);
  [row,col]=LocateImage(sub(1),sub(2),height,width);
  if cluster(p)>2*hueCountHalf
    image(row:row+height-1,col:col+width-1,1)=hue(cluster(p)-2*hueCountHalf);
  elseif cluster(p)>0
    image(row:row+height-1,col:col+width-1,1)=hue(cluster(p));
    image(row:row+height-1,col:col+width-1,2)=image(row:row+height-1,col:col+width-1,2)*0.5+0.5;
  else
    image(row:row+height-1,col:col+width-1,2)=0;
  end
end

%Show center begin
image=hsv2rgb(image);
for p=1:length(center)
  if cluster(center(p))
    [sub(2),sub(1)]=ind2sub([10,40],center(p));
    [row,col]=LocateImage(sub(1),sub(2),height,width);
    image=insertShape(image,'FilledCircle',[col+0.85*width,row+0.15*width,0.05*height],'Color','white','Opacity',1);
  end
end

%Output begin
%Resize image to decrease heigh and increase width
image=[image(1:20*height,:,:),image(20*height+1:40*height,:,:)];

%Save to file
loc=input('Where to save the image?\nDefault:..\\..\\doc\\Photo,Olivetti.png\nType # if you do not want to save\n','s');
if isempty(loc)
  imwrite(image,'..\\..\\doc\\Photo,Olivetti.png');
  disp('Saved');
elseif loc(1)=='#'
  disp('Save canceled');
else
  imwrite(image,loc);
  disp('Saved');
end

%Show image
imshow(image);

end

function [row,col]=LocateImage(row,col,height,width)
row=(row-1)*height+1;
col=(col-1)*width+1;
end

function [out]=Filter(in,least)
count=zeros(size(in));
for p=1:length(in)
  count(p)=sum(in==in(p));
end
out=zeros(size(in));
out(count>least)=in(count>least);
end

function [cluster]=Recode(cluster)
count=0;
cluster(~logical(cluster))=81;
for p=1:400
  if cluster(p)<=40
    count=count+1;
    cluster(cluster==cluster(p))=40+count;
  end
end
cluster(cluster==81)=40;
cluster=cluster-40;
end