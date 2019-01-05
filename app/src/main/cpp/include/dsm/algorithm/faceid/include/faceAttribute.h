//
// Created by amanda on 18-10-17.
//

#ifndef MTCNN_FACEATTRIBUTE_H
#define MTCNN_FACEATTRIBUTE_H

#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

static const std::vector<cv::String> outBlobNames{"age","gender","emotion"};

class FaceAttribute {
public:
    FaceAttribute(const std::string model_folder_path);
    std::map<std::string,std::string> const GetFaceAttribute(const cv::Mat &align);

private:
    cv::dnn::Net dnn_face_attribute_net;
    std::vector<std::vector<std::string>> faceattribute{
            {"0-10","11-20","21-30","31-40","41-50","51-60","61-70","71-100"},{"female","male"},{"angry","surprise","happy","neutral","sad"}};
    std::vector<std::string> faceattribute_key {"age","gender","emotion"};
};

#endif //MTCNN_FACEATTRIBUTE_H
