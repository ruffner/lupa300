function [M,N,x] = createImage(lvl, str)

x = zeros(1140,912,24);

for l=2:lvl
    [M,N] = segment(size(x,1),size(x,2),2,l);
    x(round(M(:)),round(N(:)),l) = 1;
end;
x(:,:,lvl+1) = 1;
x = x(:,:,1:lvl+1);

if (nargin > 1)
    for m=1:size(x,3)
        filename = [str num2str(m) '.png'];
        rgb(:,:,1)=x(:,:,m);
        rgb(:,:,2)=x(:,:,m);
        rgb(:,:,3)=x(:,:,m);
        imwrite(rgb, filename);
    end;
end;


return;

function [M,N] = segment(R, S, b, c)

M = R/2;
N = S/2;

if (b < c)
    [Ma, Na] = segment(M, N, b+1, c);
    [Mb, Nb] = segment(M, N, b+1, c);
    [Mc, Nc] = segment(M, N, b+1, c);
    [Md, Nd] = segment(M, N, b+1, c);

    Ma = Ma;    Na = Na;
    Mb = Mb;    Nb = Nb+N;
    Mc = Mc+M;  Nc = Nc;
    Md = Md+M;  Nd = Nd+N;

    M = [Ma; Mb; Mc; Md];
    N = [Na; Nb; Nc; Nd];
end;

return;
