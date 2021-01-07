import cv2
import numpy as np
from matplotlib import pyplot as plt

# 二维坐标系
class Point(object):
    def __init__(self, x, y):
        self.x = x
        self.y = y

    def getX(self):
        return self.x

    def getY(self):
        return self.y

def whole_hist(image):
	'''
	绘制整幅图像的直方图
	'''
	plt.hist(image.ravel(), 256, [0, 256]) #numpy的ravel函数功能是将多维数组降为一维数组
	plt.show()

def draw_hist(myList,Title,Xlabel,Ylabel,Xmin,Xmax,Ymin,Ymax):
    plt.hist(myList,100)
    plt.xlabel(Xlabel)
    plt.xlim(Xmin,Xmax)
    plt.ylabel(Ylabel)
    plt.ylim(Ymin,Ymax)
    plt.title(Title)
    plt.show()
if __name__ == '__main__':
    name_list = [  '8_gray.bmp','14.bmp','6.jpg','13.bmp','23.bmp','40.png', '22.bmp','1_gray.bmp','8_gray.bmp','14.bmp',
                ]
    # 八邻域图
    connects = [Point(-1, -1), Point(0, -1), Point(1, -1), Point(1, 0), Point(1, 1), Point(0, 1), Point(-1, 1),
                Point(-1, 0)]

    for name in name_list:

        threshold_list = []

        img = cv2.imread(name_list[0], 0)
        #img_gray = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)
        height = img.shape[0]
        width = img.shape[1]
        whole_hist(img)
        img_ = img.copy()
        edges = cv2.Canny(img_, 125, 250)  # canny边缘检测

        for i in range(height):
            for j in range(width):
                for k in range(8):
                    if edges[i][j] == 0:
                        continue

                    tmpX = i + connects[k].x
                    tmpY = j + connects[k].y

                    # 越界
                    if (tmpX < 0 or tmpY < 0 or tmpX >= height or tmpY >= width):
                        continue

                    else:
                        threshold_list.append(img[tmpX][tmpY])

        threshold = np.mean(threshold_list)
        print(threshold)
        #draw_hist(threshold_list,'1','x','y',0,256,0,5000)
        _, img_treated = cv2.threshold(img, int(threshold), 255, cv2.THRESH_BINARY)

        new_name = name[:-4] + ' clustered.jpg'

        while True:

            # cv2.ShowImage("Image", image)

            cv2.imshow("result", edges)


            if cv2.waitKey(1) == ord('e'):
                cv2.imwrite(new_name, edges)
                #cv2.imwrite(new_name, img_treated)
                break
