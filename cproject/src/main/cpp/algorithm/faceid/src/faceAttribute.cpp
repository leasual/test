//
// Created by amanda on 18-10-17.
//

#include <faceAttribute.h>

FaceAttribute::FaceAttribute(const std::string model_folder_path) {
    std::string model_path = model_folder_path+"/gender_emotion_age.caffemodel";
    std::string proto_path = model_folder_path+"/test_gender_emotion_age.prototxt";
    dnn_face_attribute_net = cv::dnn::readNetFromCaffe( proto_path, model_path );
}

/*std::vector<cv::Mat> FaceAttribute::GetFaceAttribute(const cv::Mat &align){
//    if (align.empty())

    std::vector<cv::Mat> outputBlobs;
    cv::Mat input_blob = cv::dnn::blobFromImage( align, 1.,
                                                 cv::Size(96,96), cv::Scalar(104,117,123), 0 );//for mobilefacenet_96
    dnn_face_attribute_net.setInput( input_blob, "data" );
    dnn_face_attribute_net.forward(outputBlobs,outBlobNames);
    std::vector<cv::Mat> class_prob(3);
    for (int i = 0;i < outputBlobs.size(); i++)
    {
        float* prob_data = (float*)outputBlobs[i].data;
        class_prob[i] = cv::Mat::zeros(1,outputBlobs[i].total(),CV_32FC1);
        for(int j=0;j < outputBlobs[i].total();j++)
            class_prob[i].at<float>(0,j) = *(prob_data+j);
    }
    return class_prob;
}*/
std::map<std::string,std::string> const FaceAttribute::GetFaceAttribute(const cv::Mat &align) {

    std::map<std::string,std::string> faceattribute_key_value;

    std::vector<cv::Mat> outputBlobs;
    cv::Mat input_blob = cv::dnn::blobFromImage( align, 1.,
                                                 cv::Size(96,96), cv::Scalar(104,117,123), 0 );//for mobilefacenet_96
    dnn_face_attribute_net.setInput( input_blob, "data" );
    dnn_face_attribute_net.forward(outputBlobs,outBlobNames);
    std::vector<cv::Mat> class_prob(3);
    for (int i = 0;i < outputBlobs.size(); i++)
    {
        float* prob_data = (float*)outputBlobs[i].data;
        class_prob[i] = cv::Mat::zeros(1,outputBlobs[i].total(),CV_32FC1);
        for(int j=0;j < outputBlobs[i].total();j++)
            class_prob[i].at<float>(0,j) = *(prob_data+j);
        double min,max;
        cv::Point min_pos,max_pos;
        cv::minMaxLoc(class_prob[i],&min,&max,&min_pos,&max_pos);
        int max_index = max_pos.x;
        faceattribute_key_value[faceattribute_key[i]] = faceattribute[i][max_index];
    }
    return faceattribute_key_value;
}

