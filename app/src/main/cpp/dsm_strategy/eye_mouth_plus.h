//
// Created by untouch on 18-12-17.
//

#ifndef DSM_CPP_EYE_MOUTH_PLUS_H
#define DSM_CPP_EYE_MOUTH_PLUS_H

#include <iostream>
#include <opencv2/opencv.hpp>

class EyeMouthStatusPlus{
public:
    explicit EyeMouthStatusPlus(float eye_threshold = 0.40f, float mouth_threshold = 1.2f):
            left_eye_threshold_(eye_threshold), right_eye_threshold_(eye_threshold),
            mouth_threshold_(mouth_threshold), calibration_flag_(true){};
    ~EyeMouthStatusPlus() = default;
    void Feed(std::vector<cv::Point2f>& landmarks);
    bool GetEyeStatus();
    bool GetMouthStatus();
    bool Calibrate();
private:
    float GetLeftEyeRatio();
    float GetRightEyeRatio();
    float GetMouthRatio();
private:
    float left_eye_thresh_open,right_eye_thresh_open;
    float left_eye_thresh_close,right_eye_thresh_close;
    float left_eye_threshold_;
    float right_eye_threshold_;
    float mouth_threshold_;
    bool calibration_flag_;

    std::vector<float> right_eye_ratios_;
    std::vector<float> left_eye_ratios_;
    std::vector<float> mouth_ratios_;
    std::vector<cv::Point2f> landmarks_;
};


#endif //DSM_CPP_EYE_MOUTH_PLUS_H
