%%
clc;clear;close all;
%读取图像,为灰度图
imag0 = imread('test.tif');%原图
imag1 = imag0;%用于全局直方图均衡化
imag2 = imag0;%用于局部统计特征增强方法1
imag3 = imag0;%用于局部统计特征增强方法2
%%
%全局直方图均衡化
[p,q]= size(imag0);%统计像素点
[count,x] = imhist(imag0);%count表示每个灰度级别有多少个像素，x表示有多少个灰度级别
p1 = count/(p*q);%P1表示每个灰度级别出现的概率，一共有256级别
P1 = cumsum(p1);%P1表示逐行相加的概率，也就是累加概率
for i=1:256
    imag1(imag0==i)=P1(i)*(256-1);%每一个原本灰度级别为i的像素，灰度级别改为累加出现概率*256再取整
end
figure(1)
subplot(121),imhist(imag0);
title('原图像直方图');
subplot(122),imhist(imag1);
title('均衡化后的直方图');
figure(2)
subplot(121),imshow(imag0);
title('原图');
subplot(122),imshow(imag1);
title('均衡化后图');
%%
%局部统计特征增强
%方法1
M = mean2(imag0);%原图均值
u0 = 3;
M_core = ones(u0,u0)/u0^2;
tem = conv2(double(imag0),M_core);
theta = tem((u0+1)/2:p+(u0-1)/2,(u0+1)/2:q+(u0-1)/2);%领域内灰度均值
tem = conv2((double(imag0)-theta).^2,M_core);
m = sqrt(tem((u0+1)/2:p+(u0-1)/2,(u0+1)/2:q+(u0-1)/2));%领域内均方差
A = 0.8*M./theta;
A(A<1)=1;
tem = A.*(double(imag0)-m)+m;
imag2 = uint8(tem);
figure(3)
subplot(121),imshow(imag0);
title('原图');
subplot(122),imshow(imag2);
title('均衡化后图');

%方法2
STD = std2(imag0);%原图标准差
E = 4;
K0 = 0.4;
K1 = 0.02;
K2 = 0.4;
imag3((theta<=K0*M)&(m<=K2*STD)&(m>=K1*STD))=E*imag0((theta<=K0*M)&(m<=K2*STD)&(m>=K1*STD));
figure(4)
subplot(121),imshow(imag0);
title('原图');
subplot(122),imshow(imag3);
title('均衡化后图');





