import cv2
import numpy as np
import matplotlib.pyplot as plt


#求两个点的差值
def getGrayDiff(image,currentPoint,tmpPoint):
    return abs(int(image[currentPoint[0],currentPoint[1]]) - int(image[tmpPoint[0],tmpPoint[1]]))
#区域生长算法
def regional_growth (gray,seeds,threshold=10) :
    connects = [(-1, -1), (0, -1), (1, -1), (1, 0), (1, 1), \
                        (0, 1), (-1, 1), (-1, 0)]
    threshold = threshold
    height, weight = gray.shape
    seedMark = np.zeros(gray.shape)
    seedList = []
    for seed in seeds:
        if(seed[0] < gray.shape[0] and seed[1] < gray.shape[1] and seed[0]  > 0 and seed[1] > 0):
            seedList.append(seed)
    print(seedList)
    label = 1
    while(len(seedList)>0):
        currentPoint = seedList.pop(0)
        seedMark[currentPoint[0],currentPoint[1]] = label
        for i in range(8):
            tmpX = currentPoint[0] + connects[i][0]
            tmpY = currentPoint[1] + connects[i][1]
            if tmpX < 0 or tmpY < 0 or tmpX >= height or tmpY >= weight:
                continue
            grayDiff = getGrayDiff(gray,currentPoint,(tmpX,tmpY))
            if grayDiff < threshold and seedMark[tmpX,tmpY] == 0:
                seedMark[tmpX,tmpY] = label
                seedList.append((tmpX,tmpY))
    return seedMark

#初始种子选择
def originalSeed(gray):
    ret, img1 = cv2.threshold(gray, 245, 255, cv2.THRESH_BINARY)
    retval, labels, stats, centroids = cv2.connectedComponentsWithStats(img1)
    centroids = centroids.astype(int)#转化为整数
    return centroids

# if __name__ == "__main__":
img = cv2.imread('b3.jpg')
img = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
seed = originalSeed(img)
img = regional_growth(img,seed)

# 用来正常显示中文标签
plt.rcParams['font.sans-serif'] = ['SimHei']
# 图像显示
plt.figure(figsize=(10, 5))  # width * height
plt.subplot(111), plt.imshow(img, cmap='gray'), plt.title('区域生长以后'), plt.axis("off")
plt.show()
print("ok...")