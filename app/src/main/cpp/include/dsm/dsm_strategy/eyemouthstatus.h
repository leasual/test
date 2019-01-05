#pragma once

#include <vector>
#include <list>
#include <opencv2/opencv.hpp>

class EyeMouthStatus {

public:

    EyeMouthStatus();

    void Feed(std::vector<cv::Point2f>& landmarks);
    void Feed(const cv::Mat& landmarks);

    bool GetLeftEyeStatus();
    bool GetRightEyeStatus();
    bool GetMouthStatus();
    void Calibrate();
    void Calibrate1();

    bool Calibrated() {
        return isCalibrated;
    }
//    float GetLeftEyeThresh() {
//
//        return left_eye_thresh;
//    }
//    float GetRightEyeThresh() {
//
//        return right_eye_thresh;
//    }
//    float GetMouthThresh() {
//
//        return mouth_thresh;
//    }

private:
    float getLeftEyeRatio(const std::vector<cv::Point2f> & lm);
    float getRightEyeRatio(const std::vector<cv::Point2f> & lm);
    float getMouthRatio(const std::vector<cv::Point2f> & lm);

private:

    float left_eye_thresh_open,right_eye_thresh_open;
    float left_eye_thresh_close,right_eye_thresh_close;
    float mouth_thresh;

    bool isCalibrated;
    std::list<std::vector<cv::Point2f> > landmark_history;
};