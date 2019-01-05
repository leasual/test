//
// Created by untouch on 18-12-5.
//

#ifndef MAIN_INTERFACE_H
#define MAIN_INTERFACE_H
#include <iostream>
#include <opencv2/opencv.hpp>

class InterFace {
public:
    InterFace() = default;
    InterFace(const InterFace& other) = delete;
    InterFace&operator=(const InterFace& other) = delete;
    ~InterFace() = default;

    void Run( cv::Mat& img, bool regist, std::string name = "");
};


#endif //MAIN_INTERFACE_H
