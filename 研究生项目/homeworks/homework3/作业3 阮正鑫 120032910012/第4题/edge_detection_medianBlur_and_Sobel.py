#!/usr/bin/env python 
# -*- coding:utf-8 -*-
import cv2
import numpy as np

img = cv2.imread('d2.jpg')

# 灰度化处理图像
grayImage = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
grayImage=cv2.medianBlur(grayImage, 6)
# Sobel算子
x = cv2.Sobel(grayImage, cv2.CV_16S, 1, 0)  # 对x求一阶导
y = cv2.Sobel(grayImage, cv2.CV_16S, 0, 1)  # 对y求一阶导
absX = cv2.convertScaleAbs(x)
absY = cv2.convertScaleAbs(y)
Sobel = cv2.addWeighted(absX, 0.5, absY, 0.5, 0)
cv2.imshow('grayImage',grayImage)
cv2.imshow('Sobel', Sobel)
cv2.waitKey(0)
cv2.destroyAllWindows()