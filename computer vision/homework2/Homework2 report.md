# Homework2 report

### 阮正鑫 120032910012

#### 1、collecting mouse click

**points.m** is the script to choose corresponding points between two images and store the coordinates in a mat file.

#### 2、computing homography matrix H

**computeH.m**: This function takes a set of corresponding image points t1,  t2 (both t1 and t2 are 2xN matrices) and computes the associated 3 x 3 homography matrix H.

#### 3、image warping

**warpImage.m**: This function takes as input an image inputIm, a reference image refIm, and a 3x3 homography matrix H, and returns 2 images as outputs. The first image is warpIm, which is the input image inputIm warped according to H to be in the frame of the reference image refIm. The second output image is mergeIm, a single mosaic image with a larger field of view containing both the input images. To avoid holes, an inverse warp is used. 

All Matlab codes and test image are provided in the attachment.

#### 4、Create the output mosaic  

We apply our system in the provided image uttower1\2 to get the output mosaic

#### 5、display the output mosaic of provided image

<img src="C:\Users\chensicong\Desktop\rzx\cv\uttower_result.bmp" alt="result" style="zoom:50%;" />

####   6、another example

consider the crop scene picture

![crop1](C:\Users\chensicong\Desktop\rzx\cv\crop1.jpg)

![crop2](C:\Users\chensicong\Desktop\rzx\cv\crop2.jpg)

applying our system to get the mosaic

<img src="C:\Users\chensicong\Desktop\rzx\cv\crop_result.JPG" alt="crop_result" style="zoom:50%;" />

#### 7、Warp one image into a “frame” region 

We insert a teaching Powerpoints slide into a billboard

![6](C:\Users\chensicong\Desktop\rzx\cv\6.bmp)

