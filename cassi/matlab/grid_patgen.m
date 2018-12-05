% pushbroom GRID  pattern generator
% dmd dimensions: 1140 x 912 
% grid spacing: 16px
% off pixels (0) correspond to the DMD mirror reflecting light into the
% camera, on pixels (255) mean that mirror has no contribution to the image

rows = 1140;    % rows in DMD
cols = 912;     % columns in DMD
tSize = 16;      % grid tile size in pixels

gRows = floor(rows / 16);   % number of tiles vertically
gCols = floor(cols / 16);   % number of tiles horizontally

rowActual = gRows*tSize;
colActual = gCols*tSize;

for n=1:gRows
    x = ones(rows, cols, 3, 'uint8')*255;
    
    for m=n+8:32:cols-8
        x(:,m,:)=repmat(rem(1:rows,2)'*255,1,3);
    end
    
    
    if (n<10)
        imwrite(x, ['pattern0' num2str(n) '.bmp']);
    else
        imwrite(x, ['pattern' num2str(n) '.bmp']);
    end

end


