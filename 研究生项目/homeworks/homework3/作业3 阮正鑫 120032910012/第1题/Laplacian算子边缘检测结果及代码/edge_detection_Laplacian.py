#!/usr/bin/env python 
# -*- coding:utf-8 -*-
import cv2
import numpy as np

img = cv2.imread('a1.bmp')

# 灰度化处理图像
grayImage = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

dst = cv2.Laplacian(grayImage, cv2.CV_16S, ksize=3)
Laplacian = cv2.convertScaleAbs(dst)

cv2.imshow('Laplacian', Laplacian)
cv2.waitKey(0)
cv2.destroyAllWindows()