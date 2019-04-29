//
// Created by slam on 18-10-17.
//
#include "shape_predict.h"

ShapePredictor::ShapePredictor(const std::string &path_root):
    net_(new ncnn::Net){
    std::string param = path_root + "/landmark.param";
    std::string model = path_root + "/landmark.bin";

    net_->load_param(param.c_str());
    net_->load_model(model.c_str());
}

std::vector<cv::Point2f> ShapePredictor::predict(const cv::Mat &img, const cv::Rect2i &face) {
    std::vector<cv::Point2f> shape;
    cv::Mat image = img.clone();
    cv::Mat face_img = image(face);
    cv::cvtColor(face_img, face_img, cv::COLOR_BGR2GRAY);
    cv::rectangle(image,face, cv::Scalar(0,255,0));

    ncnn::Mat in = ncnn::Mat::from_pixels_resize(face_img.data, ncnn::Mat::PIXEL_GRAY, face_img.cols, face_img.rows, 112, 112);
    ncnn::Mat out;

    ncnn::Extractor ex = net_->create_extractor();
    ex.set_num_threads(1);
    ex.input("data", in);
    ex.extract("pts82",out);

    for(size_t i = 0; i != out.total(); i+=2){
        float x = out[i];
        float y = out[i+1];
        x = x/2 + meanshape[i];
        y = y/2 + meanshape[i+1];
        x = x * face.width + face.x;
        y = y * face.height + face.y;
        shape.emplace_back(x,y);
    }
    return shape;
}

