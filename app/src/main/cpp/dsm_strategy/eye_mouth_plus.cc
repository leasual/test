//
// Created by untouch on 18-12-17.
//

#include "eye_mouth_plus.h"
#include "stats_basic.h"
#include <mutex>

void EyeMouthStatusPlus::Feed(std::vector<cv::Point2f> &landmarks) {
    landmarks_ = landmarks;
}

bool EyeMouthStatusPlus::GetEyeStatus() {
    auto left = GetLeftEyeRatio();
    auto right = GetRightEyeRatio();

//    std::cout << "left  : " << left << "  " << "threshold : " << left_eye_threshold_ << std::endl;
//    std::cout << "right : " << right << "  " << "threshold : " << right_eye_threshold_ << std::endl;

    return left < left_eye_threshold_ or right < right_eye_threshold_;
}

bool EyeMouthStatusPlus::GetMouthStatus() {
    auto mouth = GetMouthRatio();

//    std::cout << "mouth : " << mouth << "  " << "threshold : " << mouth_threshold_ << std::endl;

    return mouth > mouth_threshold_;
}

float EyeMouthStatusPlus::GetLeftEyeRatio() {
    float dist0 = norm(landmarks_[44], landmarks_[48]);
    float dist1 = norm(landmarks_[45], landmarks_[51]);
    float dist2 = norm(landmarks_[46], landmarks_[50]);
    float dist3 = norm(landmarks_[47], landmarks_[49]);
    return (dist1 + dist2 + dist3) / dist0;
}

float EyeMouthStatusPlus::GetRightEyeRatio() {
    float dist0 = norm(landmarks_[52], landmarks_[56]);
    float dist1 = norm(landmarks_[53], landmarks_[59]);
    float dist2 = norm(landmarks_[54], landmarks_[58]);
    float dist3 = norm(landmarks_[55], landmarks_[57]);
    return (dist1 + dist2 + dist3) / dist0;
}

float EyeMouthStatusPlus::GetMouthRatio() {
    float dist0 = norm(landmarks_[72], landmarks_[76]);
    float dist1 = norm(landmarks_[73], landmarks_[79]);
    float dist2 = norm(landmarks_[74], landmarks_[78]);
    float dist3 = norm(landmarks_[75], landmarks_[77]);
    return (dist1 + dist2 + dist3) / dist0;
}

bool EyeMouthStatusPlus::Calibrate() {
    sort(right_eye_ratios_.begin(), right_eye_ratios_.end());
    sort(left_eye_ratios_.begin(), left_eye_ratios_.end());
    sort(mouth_ratios_.begin(), mouth_ratios_.end());

    size_t right_size = right_eye_ratios_.size()/2;
    size_t left_size = left_eye_ratios_.size()/2;
    size_t mouth_size = mouth_ratios_.size()/2;

    for(auto iter = right_eye_ratios_.begin(); iter != right_eye_ratios_.begin()+right_size; ++iter)
        *(iter + right_size) = *iter * 0.5f;
    for(auto iter = left_eye_ratios_.begin(); iter != left_eye_ratios_.begin()+left_size; ++iter)
        *(iter + left_size) = *iter * 0.5f;
    for(auto iter = mouth_ratios_.begin(); iter != mouth_ratios_.begin()+mouth_size; ++iter)
        *(iter + left_size) = *iter * 10.f;

    cv::Mat train_data_left(left_eye_ratios_, true);
    cv::Mat train_data_right(right_eye_ratios_, true);
    cv::Mat train_labels_left(100, 1, CV_32SC1);
    cv::Mat train_labels_right(100, 1, CV_32SC1);

    cv::Mat centers_left(2, 1, train_data_left.type());
    cv::Mat centers_right(2, 1, train_data_right.type());

    kmeans(train_data_left, 2, train_labels_left,
           cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
           3, cv::KMEANS_PP_CENTERS, centers_left);
    kmeans(train_data_right, 2, train_labels_right,
           cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
           3, cv::KMEANS_PP_CENTERS, centers_right);

    left_eye_threshold_ = (centers_left.at<float>(0) + centers_left.at<float>(1)) / 2;
    right_eye_threshold_ = (centers_right.at<float>(0) + centers_right.at<float>(1)) / 2;
    if (centers_right.at<float>(0) > centers_right.at<float>(1)) {
        right_eye_thresh_open = centers_right.at<float>(0);
        right_eye_thresh_close = centers_right.at<float>(1);
    } else {
        right_eye_thresh_open = centers_right.at<float>(1);
        right_eye_thresh_close = centers_right.at<float>(0);
    }

    if (centers_left.at<float>(0) > centers_left.at<float>(1)) {
        left_eye_thresh_open = centers_left.at<float>(0);
        left_eye_thresh_close = centers_left.at<float>(1);
    } else {
        left_eye_thresh_open = centers_left.at<float>(1);
        left_eye_thresh_close = centers_left.at<float>(0);
    }
    mouth_threshold_ = computeMean(mouth_ratios_.begin() + mouth_size, mouth_ratios_.end());

    calibration_flag_ = false;
}
