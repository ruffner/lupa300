% pushbroom cassi pattern generator
% dmd dimensions: 1140 x 912 
% column spacing: 64px

I={};
n=floor(912/64);
col=64;
for im=1:n
    name=sprintf('pbpat/pbpat_%d.png',im);
    I{im}=ones(1140,912);
    I{im}(:,col)=rem(1:1140,2)';
    col=col+64;
    rgb(:,:,1)=I{im};
    rgb(:,:,2)=I{im};
    rgb(:,:,3)=I{im};
    imwrite(rgb,name);
end

cal=zeros(1140,912);
rgb(:,:,1)=cal;
rgb(:,:,3)=cal;
rgb(:,:,3)=cal;
imwrite(rgb,'pbpat/cal.png');


