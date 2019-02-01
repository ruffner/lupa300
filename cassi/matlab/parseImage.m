function [R,S,x,A,Ai,dM,dN,dR,dS,X] = parseImage(str, lvl)
%[R,S,x,A,Ai,dM,dN,dR,dS] = parseImage(str, lvl)
% 	
% 	str = filename of the *.lau file storing the image sequence
% 	lvl = how many levels of dots to search for
% 	
% 	R  = row coordinate in the DMD array for each pixel in the cropped Ximea image
% 	C  = col coordinate in the DMD array for each pixel in the cropped Ximea image
% 	x  = loaded images from disk up to level lvl
%   A  = coefficients for mapping mirrors from the DMD to pixels in the cropped Ximea image
%   Ai = coefficients for mapping pixels from the cropped Ximea image to mirrors in the DMD
% 	dM = average spacing up and down between mirrors in the cropped Ximea image
% 	dN = average spacing left and right between mirrors in the cropped Ximea image
% 	dR = average spacing up and down between mirrors in the DMD
% 	dS = average spacing left and right between mirrors in the DMD

%%% CREATE A 3D MATRIX TO HOLD THE INCOMING IMAGE FRAME BY FRAME
x = zeros(2048,2048,lvl,'single');
for m=1:lvl
    x(:,:,m) = imread(str, 'tif', 'index', m+1);
end;

%%% CROP THE IMAGE DOWN TO JUST THE DMD 
x = x(260:1440,87:1964,:)/65535;

%%% GET THE ROW AND COLUMN COORDINATES OF THE DETECTED DOTS
[M,N] = segment(x,1,2,lvl);

%%% GET THE MIRROR COORDINATES FOR THE MATCHING LEVEL
[R,S] = createImage(lvl);

%%% GET TRANSFORM FROM MIRRORS TO CAMERA PIXELS
A  = getTransformDMDtoXIM(R,S,M,N);
Ai = getTransformDMDtoXIM(M,N,R,S);

%%% GET THE SPACING BETWEEN VISIBLE DOTS
dM = M(3)-M(2);
dN = N(2)-N(1);

%%% GET THE SPACING BETWEEN THE MIRRORS
dR = R(3)-R(2);
dS = S(2)-S(1);

%%% SORT THE PSFS FROM TOP TO BOTTOM AND LEFT TO RIGHT
[S,I] = sort(S);
 M = M(I);
 N = N(I);
 R = R(I);

[R,I] = sort(R);
 N = N(I);
 M = M(I);
 S = S(I);

if (nargout > 9)
    X = zeros(dM+1, dN+1, length(M));
    for l = 1:length(M)
        X(:,:,l) = x(M(l)-dM/2:M(l)+dM/2, N(l)-dN/2:N(l)+dN/2, end);
    end;
end;

%%% NORMALIZE THE ROW AND COLUMN COORDINATES 
M = M/dM;
N = N/dN;

%%% NORMALIZE THE DMD COORDINATES
R = R/dR - 0.5;
S = S/dS - 0.5;

%%% MAP IMAGE PIXEL COORDINATES TO DMD MIRROR COORDINATES
[R,S] = segmentImage(M,N,R,S,x(:,:,1),dM,dN);

%%% DISPLAY THE INPUT IMAGE PIXELS IN DMD MIRROR COORDINATES
imagesc((R-round(R)).*(S-round(S)));
%imagesc(x(:,:,end));
colormap(gray); hold on;
plot(N*dN,M*dM,'ro');

%%% QUANTIZE THE IMAGE PIXELS TO INTEGER MIRROR COORDINATES
x = x(:,:,end)-x(:,:,1);

return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% RECURSIVE FUNCTION TO SUBDIVIDE THE INPUT IMAGE INTO QUADRANTS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [M,N] = segment(x, a, b, c)

[y,m,n,l] = find_center((x(:,:,b) - x(:,:,a)) > 0.01);

M = [];
N = [];
if (~isempty(l))
    l = min(find(l == max(l)));
    M = round(m(l));
    N = round(n(l));

    if (b < c)
        xa = x(1:M,1:N,:);
        xb = x(1:M,N:end,:);
        xc = x(M:end,1:N,:);
        xd = x(M:end,N:end,:);

        [Ma, Na] = segment(xa, a, b+1, c);
        [Mb, Nb] = segment(xb, a, b+1, c);
        [Mc, Nc] = segment(xc, a, b+1, c);
        [Md, Nd] = segment(xd, a, b+1, c);
        
        Ma = Ma;    Na = Na;
        Mb = Mb;    Nb = Nb+N;
        Mc = Mc+M;  Nc = Nc;
        Md = Md+M;  Nd = Nd+N;
        
        M = [Ma; Mb; Mc; Md];
        N = [Na; Nb; Nc; Nd];
    end;
end;

return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% USE A FOURTH ORDER POLYNOMIAL TO MATCH POINTS FROM INPUT TO OUTPUT
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [P,Q] = segmentImage(M,N,R,S,x,dP,dQ)

M = M(:);
N = N(:);
R = R(:);
S = S(:);

if (length(M) <= 16)
    A = [ M.^2.*N.^0 M.^1.*N.^1 M.^0.*N.^2 ... 
          M.^1.*N.^0 M.^0.*N.^1 ... 
          M.^0 ];
    v = (A'*A)\A'*R;
    w = (A'*A)\A'*S;
elseif (length(M) <= 32)
    A = [ M.^3.*N.^0 M.^2.*N.^1 M.^1.*N.^2 M.^0.*N.^3 ... 
          M.^2.*N.^0 M.^1.*N.^1 M.^0.*N.^2 ... 
          M.^1.*N.^0 M.^0.*N.^1 ... 
          M.^0 ];
    v = (A'*A)\A'*R;
    w = (A'*A)\A'*S;
else
    A = [ M.^4.*N.^0 M.^3.*N.^1 M.^2.*N.^2 M.^1.*N.^3 M.^0.*N.^4 ...
          M.^3.*N.^0 M.^2.*N.^1 M.^1.*N.^2 M.^0.*N.^3 ... 
          M.^2.*N.^0 M.^1.*N.^1 M.^0.*N.^2 ... 
          M.^1.*N.^0 M.^0.*N.^1 ... 
          M.^0 ];
    v = (A'*A)\A'*R;
    w = (A'*A)\A'*S;
end;

while (size(v,1) < 15)
    v = [0; v];
    w = [0; w];
end;

[Q,P] = meshgrid([1:size(x,2)]/dQ, [1:size(x,1)]/dP);
A = [ P(:).^4.*Q(:).^0 P(:).^3.*Q(:).^1 P(:).^2.*Q(:).^2 P(:).^1.*Q(:).^3 P(:).^0.*Q(:).^4 ...
      P(:).^3.*Q(:).^0 P(:).^2.*Q(:).^1 P(:).^1.*Q(:).^2 P(:).^0.*Q(:).^3 ... 
      P(:).^2.*Q(:).^0 P(:).^1.*Q(:).^1 P(:).^0.*Q(:).^2 ... 
      P(:).^1.*Q(:).^0 P(:).^0.*Q(:).^1 ... 
      P(:).^0.*Q(:).^0 ];

P(:) = A * v;
Q(:) = A * w;

return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% USE A FOURTH ORDER POLYNOMIAL TO MATCH POINTS FROM INPUT TO OUTPUT
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function A = getTransformDMDtoXIM(R,S,M,N)

M = M(:)/1000;
N = N(:)/1000;
R = R(:)/1000;
S = S(:)/1000;

if (length(M) <= 16)
    A = [ R.^2.*S.^0 R.^1.*S.^1 R.^0.*S.^2 ... 
          R.^1.*S.^0 R.^0.*S.^1 ... 
          R.^0.*S.^0 ];
    v = (A'*A)\A'*M;
    w = (A'*A)\A'*N;
elseif (length(M) <= 32)
    A = [ R.^3.*S.^0 R.^2.*S.^1 R.^1.*S.^2 R.^0.*S.^3 ... 
          R.^2.*S.^0 R.^1.*S.^1 R.^0.*S.^2 ... 
          R.^1.*S.^0 R.^0.*S.^1 ... 
          R.^0.*S.^0 ];
    v = (A'*A)\A'*M;
    w = (A'*A)\A'*N;
else
    A = [ R.^4.*S.^0 R.^3.*S.^1 R.^2.*S.^2 R.^1.*S.^3 R.^0.*S.^4 ...
          R.^3.*S.^0 R.^2.*S.^1 R.^1.*S.^2 R.^0.*S.^3 ... 
          R.^2.*S.^0 R.^1.*S.^1 R.^0.*S.^2 ... 
          R.^1.*S.^0 R.^0.*S.^1 ... 
          R.^0.*S.^0 ];
    v = (A'*A)\A'*M;
    w = (A'*A)\A'*N;
end;

A = [v w];
while (size(A,1) < 15)
    A = [0 0; A];
end;

return;
