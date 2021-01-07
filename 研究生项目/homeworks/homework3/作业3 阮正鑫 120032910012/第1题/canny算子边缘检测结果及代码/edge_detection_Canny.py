#!/usr/bin/env python 
# -*- coding:utf-8 -*-
import cv2
import numpy as np

img = cv2.imread('a1.bmp')

img1 = cv2.GaussianBlur(img,(3,3),0)
canny = cv2.Canny(img1, 50, 150)

cv2.imshow('canny', canny)
cv2.waitKey(0)
cv2.destroyAllWindows()