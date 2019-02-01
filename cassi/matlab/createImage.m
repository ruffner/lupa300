function [R,S] = createImage(lvl)

rows = 1140;    % rows in DMD
cols = 912;     % columns in DMD
tSize = 14;      % grid tile size in pixels

gRows = floor(rows / tSize);   % number of tiles vertically
gCols = floor(cols / tSize);   % number of tiles horizontally

rowActual = gRows*tSize;
colActual = gCols*tSize;

% now build pyramid + all black and all white patterns
% and prepend them to the shifting grid patterns
P=buildPatterns(6,0);

imageCount = 0; % for file naming

for imageCount=1:size(P,3)
    if lvl==imageCount
        [R,S,V]=find(P(:,:,imageCount));
        return;
    end
end

 for tr=1:tSize
    for tc=1:tSize
        pat = repmat(zeros(tSize,tSize),1); % generate the 16x16 tile
        pat(tr,tc)=255;
        I=repmat(pat,gRows,gCols); % create full size image
        
        I=[I;zeros(rows-rowActual,colActual)]; % pad with 4 rows at bottom
        if colActual ~= cols
            I=[I zeros(rows,cols-colActual)]; % pad with 4 rows at right
        end
      
        if lvl==imageCount
           [R,S,V]=find(I);
           return;
        end
        
        imageCount=imageCount+1;
   end
end

end