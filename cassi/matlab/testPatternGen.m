% checkerboard generator to test alignment 
% between matlab pixels and dmd mirrors

image=zeros(1140,912);
for pattern=1:16
    for row=1:1140
        for col=1:912
            image(row,col)=(mod(row,pattern)>(pattern/2))*(mod(col,pattern)>(pattern/2));
        end
    end
    
    RGB = insertText(image,[400 500],num2str(pattern,'%d'),'FontSize',150,'BoxColor',...
    'black','BoxOpacity',1.0,'TextColor','white');
    
    imwrite(RGB,sprintf('pattern_%d.png',pattern));
end

imagesc(image)
colormap(gray);
axis image

