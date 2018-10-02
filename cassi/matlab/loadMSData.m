function x = loadMSData(str)

h = imfinfo(str);
x = zeros(h(1).Height, h(1).Width, length(h));
for m=1:size(x,3)
    x(:,:,m) = imread(str, 'index', m);
end;

return;

% yx = loadMSData('ajile_red_target.tif');
% imagesc(max(yx,[],3));
for m=1:32
    imagesc(y(:,:,m));
    pause;
    imagesc(x(:,:,m));
    pause;
end;
%
%
% for m=1:32
%     imagesc(yx(:,:,m));
%     pause;
% end;
