
clc
clear

im1 = imread('2.bmp');
im2 = imread('3.bmp');
imshow(im1);
hold on;
[x y] = ginput(4);
hold off;
imshow(im2);
hold on;
[x1 y1] = ginput(4);
hold off;
points1 = [x,y].';
points2 = [x1,y1].';
save('points.mat','points1', 'points2');