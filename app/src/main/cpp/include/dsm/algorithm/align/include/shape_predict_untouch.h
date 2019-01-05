//
// Created by slam on 18-10-17.
//

#ifndef HEAD_INFO_SHAPE_PREDICT_H
#define HEAD_INFO_SHAPE_PREDICT_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/shape_predictor.h>
#include <dlib/opencv.h>

class ShapePredictor{
public:
    ShapePredictor(const std::string& model_path);
    std::vector<cv::Point2f> predict(const cv::Mat &img, const cv::Rect2i &face);
    dlib::full_object_detection const get_shape();
private:
    dlib::shape_predictor _sp;
    dlib::full_object_detection _dlib_shape;
};

#endif //HEAD_INFO_SHAPE_PREDICT_H
