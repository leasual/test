//
// Created by slam on 18-10-17.
//
#include "shape_predict_untouch.h"

ShapePredictor::ShapePredictor(const std::string &model_path) {
    dlib::deserialize(model_path) >> _sp;
}

std::vector<cv::Point2f> ShapePredictor::predict(const cv::Mat &img, const cv::Rect2i &face) {
    std::vector<cv::Point2f> shape;
    dlib::cv_image<dlib::bgr_pixel> dlib_img(img);
    dlib::rectangle dlib_face = dlib::rectangle(face.x, face.y, face.x+face.width, face.y+face.height);
    _dlib_shape = _sp(dlib_img, dlib_face);
    for (size_t i = 0; i < _dlib_shape.num_parts(); i++){
        shape.emplace_back(cv::Point2f(_dlib_shape.part(i).x(), _dlib_shape.part(i).y()));
    }
    return shape;
}

dlib::full_object_detection const ShapePredictor::get_shape(){
    return this->_dlib_shape;
}

