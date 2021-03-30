# -*- coding: utf-8 -*-
"""
Created on Sun Mar 28 17:02:20 2021

@author: ruanzhengxin
"""
import cv2
sift = cv2.SIFT_create()
img = cv2.imread("3.jpg")
gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
kp, des = sift.detectAndCompute(img, None)
img1 = cv2.drawKeypoints(img, kp, img, color=(255,0,255))
cv2.imshow("point",img1)
cv2.imwrite("scaled of SIFT.png", img1)
cv2.waitKey()


