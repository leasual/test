//
// Created by amanda on 18-10-17.
//

#include <faceID.h>

FaceID::FaceID(const std::string model_folder_path) {
    std::string model_path = model_folder_path+"/wd_face_reg_v3.caffemodel";
    std::string proto_path = model_folder_path+"/wd_face_reg_v3.prototxt";
    dnn_face_net = cv::dnn::readNetFromCaffe( proto_path, model_path );
}

void FaceID::GetFaceFeature(const cv::Mat &align, cv::Mat &feat) {
    if(align.empty()) {
        return;
    }
    cv::Mat input_blob = cv::dnn::blobFromImage( align, 1.,
                                                 cv::Size(96,96), cv::Scalar(104,117,123), 0 );//for mobilefacenet_96
    dnn_face_net.setInput( input_blob, "data" );
    cv::Mat m = dnn_face_net.forward( "output" );
    float* data = (float*)m.data;
    feat = cv::Mat::zeros(1,m.total(),CV_32FC1);
    for(int i=0;i < m.total();i++)
        feat.at<float>(0,i) = *(data+i);
}

float FaceID::CalcCosScore(const cv::Mat& lr, const cv::Mat& rr)  {
    cv::Mat mult = lr * rr.t();
    double score = mult.at<float>(0,0)/ ( cv::norm( lr, cv::NORM_L2 ) * cv::norm(rr,cv::NORM_L2) );
    if(score < 0)
        score = 0.f;
    return score;
}

float FaceID::CalcEuclideanScore(const cv::Mat& lr,const cv::Mat& rr) {
    return 0.0f;
}

