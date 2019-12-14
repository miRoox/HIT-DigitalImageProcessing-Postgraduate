%%
clc;clear;close all;
%��ȡͼ��,Ϊ�Ҷ�ͼ
imag0 = imread('test.tif');%ԭͼ
imag1 = imag0;%����ȫ��ֱ��ͼ���⻯
imag2 = imag0;%���ھֲ�ͳ��������ǿ����1
imag3 = imag0;%���ھֲ�ͳ��������ǿ����2
%%
%ȫ��ֱ��ͼ���⻯
[p,q]= size(imag0);%ͳ�����ص�
[count,x] = imhist(imag0);%count��ʾÿ���Ҷȼ����ж��ٸ����أ�x��ʾ�ж��ٸ��Ҷȼ���
p1 = count/(p*q);%P1��ʾÿ���Ҷȼ�����ֵĸ��ʣ�һ����256����
P1 = cumsum(p1);%P1��ʾ������ӵĸ��ʣ�Ҳ�����ۼӸ���
for i=1:256
    imag1(imag0==i)=P1(i)*(256-1);%ÿһ��ԭ���Ҷȼ���Ϊi�����أ��Ҷȼ����Ϊ�ۼӳ��ָ���*256��ȡ��
end
figure(1)
subplot(121),imhist(imag0);
title('ԭͼ��ֱ��ͼ');
subplot(122),imhist(imag1);
title('���⻯���ֱ��ͼ');
figure(2)
subplot(121),imshow(imag0);
title('ԭͼ');
subplot(122),imshow(imag1);
title('���⻯��ͼ');
%%
%�ֲ�ͳ��������ǿ
%����1
M = mean2(imag0);%ԭͼ��ֵ
u0 = 3;
M_core = ones(u0,u0)/u0^2;
tem = conv2(double(imag0),M_core);
theta = tem((u0+1)/2:p+(u0-1)/2,(u0+1)/2:q+(u0-1)/2);%�����ڻҶȾ�ֵ
tem = conv2((double(imag0)-theta).^2,M_core);
m = sqrt(tem((u0+1)/2:p+(u0-1)/2,(u0+1)/2:q+(u0-1)/2));%�����ھ�����
A = 0.8*M./theta;
A(A<1)=1;
tem = A.*(double(imag0)-m)+m;
imag2 = uint8(tem);
figure(3)
subplot(121),imshow(imag0);
title('ԭͼ');
subplot(122),imshow(imag2);
title('���⻯��ͼ');

%����2
STD = std2(imag0);%ԭͼ��׼��
E = 4;
K0 = 0.4;
K1 = 0.02;
K2 = 0.4;
imag3((theta<=K0*M)&(m<=K2*STD)&(m>=K1*STD))=E*imag0((theta<=K0*M)&(m<=K2*STD)&(m>=K1*STD));
figure(4)
subplot(121),imshow(imag0);
title('ԭͼ');
subplot(122),imshow(imag3);
title('���⻯��ͼ');





