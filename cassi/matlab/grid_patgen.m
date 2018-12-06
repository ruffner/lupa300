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

imageCount = 0; % for file naming

 for tr=1:tSize
    for tc=1:tSize
        pat = repmat(ones(tSize,tSize)*255,1); % generate the 16x16 tile
        pat(tr,tc)=0;
        I=repmat(pat,gRows,gCols); % create full size image
        I=[I;ones(4,912)*255]; % pad with 4 rows at bottom
        
        % write to disk
        if imageCount<10
            imwrite(I, ['gridpat/pattern00' num2str(imageCount) '.bmp']);
        elseif imageCount>=10 && imageCount<100
            imwrite(I, ['gridpat/pattern0' num2str(imageCount) '.bmp']);
        else
            imwrite(I, ['gridpat/pattern' num2str(imageCount) '.bmp']);
        end
        
        imageCount=imageCount+1
   end
end

