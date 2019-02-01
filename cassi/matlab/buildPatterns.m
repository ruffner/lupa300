function [X,M,N] = buildPatterns(lvl, offset)

if (nargin < 2)
    offset = 0;
end;

%%% calculate the number of points forming the grid
numPoints = 2^lvl;

%%% calculate the spacing between grid points as an even number of rows and
%%% columns
delta = floor([1140 912]/numPoints);
delta = delta - rem(delta, 2);

%%% calculate how much space is at the end of the DMD
spacing = [1140 912] - (numPoints-1)*delta;

%%% calculate the mirror coordinates for the grid
[N,M] = meshgrid([0:numPoints-1]*delta(2) + spacing(2)/2, ...
                 [0:numPoints-1]*delta(1) + spacing(1)/2);

%%% make sure all mirrors land on even rows and even columns
M = M - rem(M,2) + offset;
N = N - rem(N,2) + offset;

%%% create the initial grid pattern based on constraints above
X = zeros(1140, 912);
X(M(:),N(:)) = 1;

R = M(1:end-1,:);
C = N(:,1:end-1);

%%% iteratively combine mirrors until we have no more mirrors to combine
while (size(M,1) > 1)
    %%% merge 2x2 mirror row coordinates into a single mirror coordinate
    M = (M(1:2:end,:) + M(2:2:end,:))/2;
    M = (M(:,1:2:end) + M(:,2:2:end))/2;
    M = round(M);

    %%% merge 2x2 mirror column coordinates into a single mirror coordinate
    N = (N(1:2:end,:) + N(2:2:end,:))/2;
    N = (N(:,1:2:end) + N(:,2:2:end))/2;
    N = round(N);

    %%% generate the new mirror grid pattern
    Y = zeros(1140, 912);
    Y(M(:),N(:)) = 1;
    
    %%% concatenate the new pattern to the previous grid patterns
    X = cat(3, Y, X);
end;

%%% now add all the all white pattern and all black pattern
X = cat(3, zeros(1140, 912), X);
X = cat(3, X, ones(1140, 912));

% %% now let's create a sequence of shifting patterns moving 4 pixels at a
% %% time
% for m=0:4:delta(1)-1
%     for n=0:4:delta(2)-1
%         Y = zeros(1140, 912);
%         Y(R(:)+m,C(:)+n) = 1;
%         X = cat(3, X, Y);
%     end;
% end;

return;


