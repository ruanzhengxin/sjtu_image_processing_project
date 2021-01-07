#!/usr/bin/env python 
# -*- coding:utf-8 -*-
import cv2
import numpy as np

img = cv2.imread('a1.bmp')

# 灰度化处理图像
grayImage = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
# 先通过高斯滤波降噪
gaussian = cv2.GaussianBlur(grayImage, (3, 3), 0)

# 再通过拉普拉斯算子做边缘检测
dst = cv2.Laplacian(gaussian, cv2.CV_16S, ksize=3)
LOG = cv2.convertScaleAbs(dst)


cv2.imshow('LOG', LOG)
cv2.waitKey(0)
cv2.destroyAllWindows()