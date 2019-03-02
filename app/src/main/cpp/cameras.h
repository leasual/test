//
// Created by untouch on 18-11-30.
//

#ifndef MAIN_CAMERAS_H
#define MAIN_CAMERAS_H

#include <iostream>
#include <string>
#include <algorithm>

#include <dirent.h>
#include <opencv2/opencv.hpp>


class Camera{
public:
    Camera() = default;
    virtual ~Camera(){};
    virtual std::string Read(cv::Mat& frame) = 0;
};

class CameraOnLine: public Camera{
public:
    explicit CameraOnLine(int index = 0);
    std::string Read(cv::Mat& frame) override;

protected:
    cv::VideoCapture cam_;
};

class CameraOffLine: public Camera{
public:
    explicit CameraOffLine(const std::string& path = "");

    std::string Read(cv::Mat& frame) override;

private:
    std::vector<std::string>Listdir(const std::string& folder);

    std::vector<std::string>Listfile(const std::string& folder);

private:
    int index_;
    std::vector<std::string> files_;
};


#endif //MAIN_CAMERAS_H
