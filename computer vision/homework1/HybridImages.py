# -*- coding: utf-8 -*-
"""
Created on Fri Mar 26 11:21:34 2021

@author: ruanzhengxin
"""
import numpy as np
import cv2

def convolution(image, kernel):
    # Load the image.
    #image = cv2.imread(img)
    # Flip template before convolution.
    kernel = cv2.flip(kernel, -1)
    # Get size of image and kernel. 3rd value of shape is colour channel.
    (image_h, image_w) = image.shape[:2]
    (kernel_h, kernel_w) = kernel.shape[:2]
    (pad_h, pad_w) = (kernel_h // 2, kernel_w // 2)
    # Create image to write to.
    output = np.zeros(image.shape)
    # Slide kernel across every pixel.
    for y in range(pad_h, image_h - pad_h):
        for x in range(pad_w, image_w - pad_w):
            # If coloured, loop for colours.
            for colour in range(image.shape[2]):
                # Get center pixel.
                center = image[
                    y - pad_h : y + pad_h + 1, x - pad_w : x + pad_w + 1, colour
                ]
                # Perform convolution and map value to [0, 255].
                # Write back value to output image.
                output[y, x, colour] = (center * kernel).sum() / 255

    # Return the result of the convolution.
    return output

def gaussian_blur(image, sigma):
    """ Builds a Gaussian kernel used to perform the LPF on an image.
    """
    # Calculate size of filter.
    size = 8 * sigma + 1
    if not size % 2:
        size = size + 1

    center = size // 2
    kernel = np.zeros((size, size))

    # Generate Gaussian blur.
    for y in range(size):
        for x in range(size):
            diff = (y - center) ** 2 + (x - center) ** 2
            kernel[y, x] = np.exp(-diff / (2 * sigma ** 2))

    kernel = kernel / np.sum(kernel)
    return convolution(image, kernel)
    
def low_pass(image, cutoff):
    """ Generate low pass filter of image.
    """
    print("low_pass")
    return gaussian_blur(image, cutoff)


def high_pass(image, cutoff):
    """ Generate high pass filter of image. This is simply the image minus its
    low passed result.
    """
    print("high_pass")
    return (image / 255) - low_pass(image, cutoff)


def hybrid_image(image, cutoff):
    """ Create a hybrid image by summing together the low and high frequency
    images.
    """
    print("1111")
    # Perform low pass filter and export.
    low = low_pass(image[1], cutoff[0])
    cv2.imwrite("low.jpg", low * 255)
    # Perform high pass filter and export.
    high = high_pass(image[0], cutoff[1])
    cv2.imwrite("high.jpg", (high + 0.5) * 255)

    print("Creating hybrid image...")
    return low + high    

def output_vis(image):
    """ Display hybrid image comparison for report. Visualisation shows 5 images
    reducing in size to simulate viewing the image from a distance.
    """
    print("Creating visualisation...")

    num = 6  # Number of images to display.
    gap = 2  # Gap between images (px).

    # Create list of images.
    image_list = [image]
    max_height = image.shape[0]
    max_width = image.shape[1]

    # Add images to list and increase max width.
    for i in range(1, num):
        tmp = cv2.resize(image, (0, 0), fx=0.5 ** i, fy=0.5 ** i)
        max_width += tmp.shape[1] + gap
        image_list.append(tmp)

    # Add images to stack.
    stack = np.ones((max_height, max_width, 3)) * 255

    # Add images to stack.
    current_x = 0
    for img in image_list:
        stack[
            max_height - img.shape[0] :, current_x : img.shape[1] + current_x, :
        ] = img
        current_x += img.shape[1] + gap

    return stack

def main():
    #img1 = cv2.imread('cat.bmp')
    #img2 = cv2.imread('dog.bmp')
    #img1 = cv2.imread('bicycle.bmp')
    #img2 = cv2.imread('motorcycle.bmp')
    img1 = cv2.imread('plane.bmp')
    img2 = cv2.imread('bird.bmp')
    images = []
    images.append(img1)
    images.append(img2)
    hybrid = hybrid_image(images, [6, 6])
    cv2.imwrite("output2.jpg", output_vis(hybrid) * 255)
    cv2.imshow('hybrid', hybrid)
    cv2.waitKey(0)
    
if __name__ == "__main__":
    main()
    
    
    