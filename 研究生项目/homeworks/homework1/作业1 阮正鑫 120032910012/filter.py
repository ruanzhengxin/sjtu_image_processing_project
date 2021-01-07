#!/usr/bin/env python 
# -*- coding:utf-8 -*-

import numpy as np
import cv2
import matplotlib.pyplot as plt

img = cv2.imread('j4.jpg')
i=9
# 均值滤波
img_mean = cv2.blur(img, (i,i))
#img_mean = cv2.blur(img, (3, 3))
# 高斯滤波
img_Guassian = cv2.GaussianBlur(img,(i,i),0)
#img_Guassian = cv2.GaussianBlur(img,(3, 3),0)

# 中值滤波
img_median = cv2.medianBlur(img, i)
#img_median = cv2.medianBlur(img, 3)



# 展示不同的图片
titles = ['srcImg','mean', 'Gaussian', 'median']
imgs = [img, img_mean, img_Guassian, img_median]
#cv2.namedWindow('mean9*9', 0)
#cv2.resizeWindow('mean9*9', 400, 500)
#cv2.namedWindow('Guassian9*9', 0)
#cv2.resizeWindow('Guassian9*9', 400, 500)
#cv2.namedWindow('median9*9', 0)
#cv2.resizeWindow('median9*9', 400, 500)
#cv2.imencode('mean9*9.png',img_mean)
#cv2.imwrite('Guassian9*9.jpg',img_Guassian)
#cv2.imwrite('median9*9.jpg',img_median)
#cv2.imshow('mean9*9',img_mean)
#cv2.imshow('Guassian9*9',img_Guassian)
#cv2.imshow('median9*9',img_median)
#cv2.waitKey(0)
for i in range(4):
    plt.subplot(1,4,i+1)
    plt.imshow(imgs[i])
    plt.title(titles[i])
plt.show()