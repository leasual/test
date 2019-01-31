//
// Created by untouch on 18-12-5.
//

#include "total_flow.h"
#include "interface.h"

void InterFace::Run( cv::Mat &img, bool regist, std::string name) {
//    cv::Mat frame = img.clone();
//    cv::putText(frame, "Test Pass!", cv::Point(100,100), 2, 2,cv::Scalar(0,255,255));
//    cv::imshow(name, frame);
//    cv::waitKey(1);
    cv::putText(img, "Test Pass!", cv::Point(100,100), 2, 2,cv::Scalar(0,255,255));
//    TotalFlow totalFlow("") ;
//    Result result;
//    totalFlow.Run(img,result, false,"");
}
