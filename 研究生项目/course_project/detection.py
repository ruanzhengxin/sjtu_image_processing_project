from __future__ import absolute_import, division, print_function
import os
import sys
import cv2
import tensorflow as tf
import numpy as np
import h5py
from scipy.special import comb
from scipy import optimize


class VehicleAndPeopleDetection:
    def __init__(self):
        currentpath, _ = os.path.split(os.path.abspath(sys.argv[0]))
        self.modelpath = currentpath
        self.prob_threshold = 0.7
        self.nms_iou_threshold = 0.6
        self.predict_fn = tf.contrib.predictor.from_saved_model(self.modelpath, signature_def_key='predict_object')
        with h5py.File(os.path.join(self.modelpath, 'index'), 'r') as h5f:
            self.labels_list = h5f['labels_list'][:]

    def predict(self, img):
        img = img[np.newaxis, :, :, :]
        output = self.predict_fn({'images': img})
        num_boxes = len(output['detection_classes'])
        classes = []
        boxes = []
        scores = []
        result_return = dict()
        for i in range(num_boxes):
            if output['detection_scores'][i] > self.prob_threshold:
                class_id = output['detection_classes'][i] - 1
                classes.append(self.labels_list[int(class_id)])
                boxes.append(output['detection_boxes'][i])
                scores.append(output['detection_scores'][i])

        bounding_boxes = boxes
        confidence_score = scores
        # Bounding boxes
        boxes = np.array(bounding_boxes)
        picked_boxes = []
        picked_score = []
        picked_classes = []
        if len(boxes) != 0:
            # coordinates of bounding boxes
            start_x = boxes[:, 0]
            start_y = boxes[:, 1]
            end_x = boxes[:, 2]
            end_y = boxes[:, 3]
            # Confidence scores of bounding boxes
            score = np.array(confidence_score)
            # Picked bounding boxes
            # # Compute areas of bounding boxes
            areas = (end_x - start_x + 1) * (end_y - start_y + 1)
            # Sort by confidence score of bounding boxes
            order = np.argsort(score)
            # Iterate bounding boxes
            while order.size > 0:
                # The index of largest confidence score
                index = order[-1]
                # Pick the bounding box with largest confidence score
                picked_boxes.append(bounding_boxes[index])
                picked_score.append(confidence_score[index])
                picked_classes.append(classes[index])
                # Compute ordinates of intersection-over-union(IOU)
                x1 = np.maximum(start_x[index], start_x[order[:-1]])
                x2 = np.minimum(end_x[index], end_x[order[:-1]])
                y1 = np.maximum(start_y[index], start_y[order[:-1]])
                y2 = np.minimum(end_y[index], end_y[order[:-1]])
                # Compute areas of intersection-over-union
                w = np.maximum(0.0, x2 - x1 + 1)
                h = np.maximum(0.0, y2 - y1 + 1)
                intersection = w * h
                # Compute the ratio between intersection and union
                ratio = intersection / (areas[index] + areas[order[:-1]] - intersection)
                left = np.where(ratio < self.nms_iou_threshold)
                order = order[left]

        result_return['detection_classes'] = picked_classes
        result_return['detection_boxes'] = picked_boxes
        result_return['detection_scores'] = picked_score
        return result_return

    def visualize(self, img, result):
        img = cv2.cvtColor(img, cv2.COLOR_RGB2BGR)
        detection_classes = result['detection_classes']
        detection_boxes = result['detection_boxes']
        if detection_boxes:
            for i in range(len(detection_boxes)):
                start_x = detection_boxes[i][0]
                start_y = detection_boxes[i][1]
                end_x = detection_boxes[i][2]
                end_y = detection_boxes[i][3]
                detection_class = detection_classes[i].decode('utf-8')              
                if detection_class == "vehicle":
                    cv2.rectangle(img, (start_y, start_x), (end_y, end_x), (0, 255, 0), 2)
                elif detection_class == "people":
                    cv2.rectangle(img, (start_y, start_x), (end_y, end_x), (255, 0, 0), 2)
                font = cv2.FONT_HERSHEY_SIMPLEX
                cv2.putText(img, detection_class , (start_y, start_x), font, 0.6, (0, 0, 255))
        return img


def linear(points, ntimes=1000):
    def f_1(x, A, B):
        return A * x + B
    xPoints = np.array([p[0] for p in points])
    yPoints = np.array([p[1] for p in points])
    A1, B1 = optimize.curve_fit(f_1, xPoints, yPoints)[0]
    y1 = A1 * xPoints + B1
    xvals = xPoints
    yvals = y1
    return xvals.astype('int32'), yvals.astype('int32')


if __name__ == "__main__":
    detector=VehicleAndPeopleDetection()
    # value initialize
    first = True
    frame=cv2.imread('16.jpg')
    #cap = cv2.VideoCapture('test8.mp4')
    #fourcc = cv2.VideoWriter_fourcc(*'XVID')
    #out = cv2.VideoWriter('output8.avi', fourcc, 60.0, (1280, 720))
    expt_startLeft = 450
    expt_startRight = 750
    expt_startTop = 520
    left_maxpoint = [0] * 50
    right_maxpoint = [195] * 50

    # convolution filter
    kernel = np.array([
        [-1, 1, 0, 1, -1],
        [-1, 1, 0, 1, -1],
        [-1, 1, 0, 1, -1],
        [-1, 1, 0, 1, -1],
        [-1, 1, 0, 1, -1]
    ])
    #while (cap.isOpened()):
    while True:
        #ret, frame = cap.read()

        # cut away invalid frame area
        valid_frame = frame[expt_startTop:, expt_startLeft:expt_startRight]
        # original_frame = valid_frame.copy()

        # gray scale transform
        gray_frame = cv2.cvtColor(valid_frame, cv2.COLOR_BGR2GRAY)

        # histogram equalization image
        histeqaul_frame = cv2.equalizeHist(gray_frame)

        # apply gaussian blur
        blur_frame = cv2.GaussianBlur(histeqaul_frame, (5, 5), 5)

        # merge current frame and last frame
        if first is True:
            merge_frame = blur_frame
            first = False
            old_valid_frame = merge_frame.copy()
        else:
            merge_frame = cv2.addWeighted(blur_frame, 0.2, old_valid_frame, 0.8, 0)
            old_valid_frame = merge_frame.copy()

            # convolution filter
        conv_frame = cv2.filter2D(merge_frame, -1, kernel)
        # conv_frame = cv2.GaussianBlur(conv_frame, (3, 3), 5)
        # initialization for sliding window property
        sliding_window = [40, 100, 180, 220]
        slide_interval = 10
        slide_height = 10
        slide_width = 40

        # initialization for linear curve variables
        left_points = []
        right_points = []

        # define count value
        count = 0
        for i in range(190, 40, -slide_interval):
            # get edges in sliding window
            left_edge = conv_frame[i:i + slide_height, sliding_window[0]:sliding_window[1]].sum(axis=0)  # 降维 按列相加
            right_edge = conv_frame[i:i + slide_height, sliding_window[2]:sliding_window[3]].sum(axis=0)
            if left_edge.argmax() > 0:
                left_maxindex = sliding_window[0] + left_edge.argmax()
                left_maxpoint[count] = left_maxindex
                cv2.line(frame, (450 + left_maxindex, 550 + i + int(slide_height / 2)),
                         (450 + left_maxindex, 550 + i + int(slide_height / 2)), (255, 255, 255), 5, cv2.LINE_AA)
                left_points.append([left_maxindex + 450, 550 + i + int(slide_height / 2)])
                sliding_window[0] = max(0, left_maxindex - int(slide_width / 4 + (slide_width + 10) / (count + 1)))
                sliding_window[1] = min(390, left_maxindex + int(slide_width / 4 + (slide_width + 10) / (count + 1)))
                cv2.rectangle(frame, (sliding_window[0] + 450, 550 + i + slide_height),
                              (sliding_window[1] + 450, 550 + i), (0, 255, 0),
                              1)

                # right railroad line processing
            if right_edge.argmax() > 0:
                right_maxindex = sliding_window[2] + right_edge.argmax()
                right_maxpoint[count] = right_maxindex
                cv2.line(frame, (450 + right_maxindex, 550 + i + int(slide_height / 2)),
                         (450 + right_maxindex, 550 + i + int(slide_height / 2)), (255, 255, 255), 5, cv2.LINE_AA)
                right_points.append([right_maxindex + 450, 550 + i + int(slide_height / 2)])
                sliding_window[2] = max(0, right_maxindex - int(slide_width / 4 + (slide_width + 10) / (count + 1)))
                sliding_window[3] = min(390, right_maxindex + int(slide_width / 4 + (slide_width + 10) / (count + 1)))
                cv2.rectangle(frame, (sliding_window[2] + 450, 550 + i + slide_height),
                              (sliding_window[3] + 450, 550 + i), (0, 0, 255),
                              1)
            count += 1
        linear_left_xval, linear_left_yval = linear(left_points, 50)
        linear_right_xval, linear_right_yval = linear(right_points, 50)

        linear_left_points = []
        linear_right_points = []
        try:
            old_point = (linear_left_xval[0], linear_left_yval[0])
            for point in zip(linear_left_xval, linear_left_yval):
                cv2.line(frame, old_point, point, (255, 0, 0), 3, cv2.LINE_AA)
                old_point = point
                linear_left_points.append(point)

            old_point = (linear_right_xval[0], linear_right_yval[0])
            for point in zip(linear_right_xval, linear_right_yval):
                cv2.line(frame, old_point, point, (255, 0, 0), 3, cv2.LINE_AA)
                old_point = point
                linear_right_points.append(point)
        except IndexError:
            pass

        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        output = detector.predict(frame)
        frame = detector.visualize(frame, output)
        cv2.imshow("result", frame)
        cv2.imshow("valid frame",conv_frame)
        cv2.waitKey(0)
        #out.write(frame)

cap.release()
out.release()
cv2.destroyAllWindows() 