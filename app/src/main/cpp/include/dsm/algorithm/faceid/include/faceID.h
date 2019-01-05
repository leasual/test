//
// Created by amanda on 18-10-17.
//

#ifndef MTCNN_FACEID_H
#define MTCNN_FACEID_H

#include <string>
#include <opencv2/opencv.hpp>

class FaceID {
public:
    void GetFaceFeature(const cv::Mat &align, cv::Mat &feat);
    static float CalcCosScore(const cv::Mat &lr, const cv::Mat &rr);

    static float CalcEuclideanScore(const cv::Mat &lr, const cv::Mat &rr);

public:
    FaceID(const std::string model_folder_path);

private:
    cv::dnn::Net dnn_face_net;
};



#endif //MTCNN_FACEID_H
