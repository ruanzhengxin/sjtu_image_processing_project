import cv2
import numpy as np
import time

start = time.time()
img_src = cv2.imread("c1.png")

# 2.灰度化与二值化
img_gray = cv2.cvtColor(img_src, cv2.COLOR_RGB2GRAY)  # 将其变成单通道
ret, img_bin = cv2.threshold(img_gray, 127, 255, cv2.THRESH_BINARY)  # 防止变换过程中非0和非255的值出现而影响结果
img_bin = cv2.copyMakeBorder(img_bin, 1, 1, 1, 1, borderType=cv2.BORDER_CONSTANT, value=0)  # 在图片四周各补零一行，即增加一行黑色像素
# 初始化起始点
start_x = -1
start_y = -1
is_start_point = 0  # 判断是否为起始点的标志

# 寻找起始点
h, w = img_bin.shape
for i in range(h):
    for j in range(w):
        if (img_bin[i, j] == 255) and (img_bin[i - 1, j] == 0):
            start_x = i
            start_y = j
            is_start_point = 1
            break
    if is_start_point == 1:
        break
# (1,39)

# 定义链码相对应的增量坐标
neibor = [(0, 1), (-1, 1), (-1, 0), (-1, -1), (0, -1), (1, -1), (1, 0), (1, 1)]  # 邻域点
temp = 2  # 链码值，也是neibor的索引序号，这里是从链码的2号位进行搜索
contours = [(start_x, start_y)]  # 用于存储轮廓点

# 将当前点设为轮廓的开始点
current_x = start_x
current_y = start_y

# temp=2，表示从链码的2方向进行邻域检索，通过当前点和邻域点集以及链码值确定邻域点
neibor_x = current_x + neibor[temp][0]
neibor_y = current_y + neibor[temp][1]

# 因为当前点的值为起始点，而终止检索的条件又是这个，所以会产生冲突，因此先寻找第二个边界点
is_contour_point = 0
while is_contour_point == 0:  # 邻域点循环，当是目标像素点时跳出
    if img_bin[neibor_x, neibor_y] == 255:
        # 将符合条件的邻域点设为当前点进行下一次的边界点搜索
        current_x = neibor_x
        current_y = neibor_y
        is_contour_point = 1
        contours.append((current_x, current_y))
        temp = temp - 2  # 作为下一个边界点的邻域检测起始点,顺时针旋转90度
        print(1)
        if temp < 0:
            temp = len(neibor) + temp
    else:
        temp = temp + 1  # 逆时针旋转45度进行搜索
        if temp >= 8:
            temp = temp - len(neibor)
    neibor_x = current_x + neibor[temp][0]
    neibor_y = current_y + neibor[temp][1]

# 开始第三个及以后的边界点的查找
while not ((current_x == start_x) and (current_y == start_y)):  # 轮廓扫描循环
    is_contour_point = 0
    while is_contour_point == 0:  # 邻域点循环，当是目标像素点时跳出
        if img_bin[neibor_x, neibor_y] == 255:  # 邻域是白点时，即为边界
            # 将符合条件的邻域点设为当前点进行下一次的边界点搜索
            current_x = neibor_x
            current_y = neibor_y
            is_contour_point = 1  # 将判断是否为边界点的标签设置为1，用于跳出循环
            contours.append((current_x, current_y))
            temp = temp - 2  # 作为下一个边界点的邻域检测起始点,顺时针旋转90度
            if temp < 0:
                temp = len(neibor) + temp
        else:
            temp = temp + 1  # 逆时针旋转45度进行搜索
            if temp >= 8:
                temp = temp - len(neibor)
        neibor_x = current_x + neibor[temp][0]
        neibor_y = current_y + neibor[temp][1]

# 对已经检测到的轮廓进行标记
for xy in (contours):
    x = xy[0]
    y = xy[1]
    img_src[x - 1, y - 1] = (0, 0, 255)
cv2.imshow('', img_src)
cv2.waitKey(0)


end = time.time()
print("时长%fms" % ((end - start) * 1000))
