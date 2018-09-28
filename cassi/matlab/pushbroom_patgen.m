% pushbroom cassi pattern generator
% dmd dimensions: 1140 x 912 
% column spacing: 32px

rows = 1140;
cols = 912;
for n=1:32
    x = ones(rows, cols, 3, 'uint8')*255;
    for m=n:32:cols
        x(:,m,:)=repmat(rem(1:rows,2)'*255,1,3);
    end
    if (n<10)
        imwrite(x, ['pattern0' num2str(n) '.bmp']);
    else
        imwrite(x, ['pattern' num2str(n) '.bmp']);
    end
end

