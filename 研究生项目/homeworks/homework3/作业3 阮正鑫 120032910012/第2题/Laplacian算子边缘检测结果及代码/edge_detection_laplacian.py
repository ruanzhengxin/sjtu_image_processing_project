import cv2
import matplotlib.pyplot as plt

img1=cv2.imread('d2.jpg')

j=0
i=0
imgs = [img1, img1, img1, img1, img1, img1]
titles = ['3*3','5*5', '7*7', '9*9', '11*11', '13*13']
while i<6:
    imgs[i]=cv2.GaussianBlur(img1,(i*2+3,i*2+3),0)
    grayImage = cv2.cvtColor(imgs[i], cv2.COLOR_BGR2GRAY)
    dst = cv2.Laplacian(grayImage, cv2.CV_16S, ksize=3)
    imgs[i] = cv2.convertScaleAbs(dst)
    i=i+1
for j in range(6):
    plt.subplot(2,3,j+1)
    plt.imshow(imgs[j])
    plt.title(titles[j])
plt.show()