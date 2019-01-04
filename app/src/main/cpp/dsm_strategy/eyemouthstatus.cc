#include "eyemouthstatus.h"
#include <cmath>
#include <utility>
#include "stats_basic.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

EyeMouthStatus::EyeMouthStatus():isCalibrated(false){}

void EyeMouthStatus::Feed(std::vector <cv::Point2f> &landmarks) {
    landmark_history.push_back(landmarks);
    if (landmark_history.size() >= 200) {
        while (landmark_history.size() >= 200) {
            landmark_history.pop_front();
        }
    }
}

void EyeMouthStatus::Feed(const cv::Mat &landmarks) {
    std::vector<cv::Point2f> landmarks_vec;
    for(int i=0;i<68;i++) {
        if(landmarks.at<float>(i) != 0 && landmarks.at<float>(i+68) !=0 ) {
            landmarks_vec.push_back(cv::Point2f(landmarks.at<float>(i), landmarks.at<float>(i + 68)));
        }
    }
    if(!landmarks_vec.empty())
        Feed(landmarks_vec);
}


void EyeMouthStatus::Calibrate() {

//        std::vector<float> right_eye_ratios;
//        std::vector<float> left_eye_ratios;
//        std::vector<float> mouth_ratios;
//        for (auto &landmarks : landmark_history) {
//            right_eye_ratios.push_back(getRightEyeRatio(landmarks));
//            left_eye_ratios.push_back(getLeftEyeRatio(landmarks));
//            mouth_ratios.push_back(getMouthRatio(landmarks));
//        }
//
//        //sort(right_eye_ratios.begin(), right_eye_ratios.end(), std::greater<float>());
//        //sort(left_eye_ratios.begin(), left_eye_ratios.end(), std::greater<float>());
//        sort(mouth_ratios.begin(), mouth_ratios.end());
//
//        cv::Mat train_data_left;
//        cv::Mat train_labels_left(100, 1, CV_32SC1);   //聚类后的标签数组
//        cv::Mat train_data_right;
//        cv::Mat train_labels_right(100, 1, CV_32SC1);   //聚类后的标签数组
//
//        for (int i = 0; i < left_eye_ratios.size(); i++) {
//            train_data_left.push_back(left_eye_ratios[i]);  //序列化后放入data
//
//        }
//        for (int i = 0; i < right_eye_ratios.size(); i++) {
//            train_data_right.push_back(right_eye_ratios[i]);  //序列化后放入data
//        }
//
//        //K表示需要聚类的类别数
//        int K = 2;
//        //centers 表示聚类后的类别中心
//        cv::Mat centers_left(K, 1, train_data_left.type());
//        cv::Mat centers_right(K, 1, train_data_right.type());
//        //Kmeans算法
//        kmeans(train_data_left, K, train_labels_left,
//               cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
//               3, cv::KMEANS_PP_CENTERS, centers_left);
//        kmeans(train_data_right, K, train_labels_right,
//               cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
//               3, cv::KMEANS_PP_CENTERS, centers_right);
//
//        //cout<< "centers1: "<<centers_left.at<float>(0)<<endl;
//        //cout<< "centers2: "<<centers_left.at<float>(1)<<endl;
//
//        if (centers_right.at<float>(0) > centers_right.at<float>(1)) {
//            right_eye_thresh_open = centers_right.at<float>(0);
//            right_eye_thresh_close = centers_right.at<float>(1);
//        } else {
//            right_eye_thresh_open = centers_right.at<float>(1);
//            right_eye_thresh_close = centers_right.at<float>(0);
//        }
//
//        if (centers_left.at<float>(0) > centers_left.at<float>(1)) {
//            left_eye_thresh_open = centers_left.at<float>(0);
//            left_eye_thresh_close = centers_left.at<float>(1);
//        } else {
//            left_eye_thresh_open = centers_left.at<float>(1);
//            left_eye_thresh_close = centers_left.at<float>(0);
//        }
//
////    right_eye_thresh = computeMean(right_eye_ratios.begin(),
////                                   right_eye_ratios.begin() + right_eye_ratios.size() / 2);
////    left_eye_thresh = computeMean(left_eye_ratios.begin(), left_eye_ratios.begin() + left_eye_ratios.size() / 2);
//
//
//        mouth_thresh = computeMean(mouth_ratios.begin(), mouth_ratios.begin() + mouth_ratios.size() / 2);
//        //std::cout << "right_eye_thresh " << right_eye_thresh << " " << " left_eye_thresh " << left_eye_thresh << std::endl;
//        isCalibrated = true;

}
void EyeMouthStatus::Calibrate1() {

    std::vector<float> right_eye_ratios;
    std::vector<float> left_eye_ratios;
    std::vector<float> mouth_ratios;
    for (auto &landmarks : landmark_history) {
        right_eye_ratios.push_back(getRightEyeRatio(landmarks));
        left_eye_ratios.push_back(getLeftEyeRatio(landmarks));
        mouth_ratios.push_back(getMouthRatio(landmarks));
    }

    //sort(right_eye_ratios.begin(), right_eye_ratios.end(), std::greater<float>());
    //sort(left_eye_ratios.begin(), left_eye_ratios.end(), std::greater<float>());
    sort(mouth_ratios.begin(), mouth_ratios.end());

    cv::Mat train_data_left;
    cv::Mat train_labels_left(100, 1, CV_32SC1);   //聚类后的标签数组
    cv::Mat train_data_right;
    cv::Mat train_labels_right(100, 1, CV_32SC1);   //聚类后的标签数组

    for (int i = 0; i < left_eye_ratios.size(); i++) {
        train_data_left.push_back(left_eye_ratios[i]);  //序列化后放入data

    }
    for (int i = 0; i < right_eye_ratios.size(); i++) {
        train_data_right.push_back(right_eye_ratios[i]);  //序列化后放入data
    }

    //K表示需要聚类的类别数
    int K = 2;
    //centers 表示聚类后的类别中心
    cv::Mat centers_left(K, 1, train_data_left.type());
    cv::Mat centers_right(K, 1, train_data_right.type());
    //Kmeans算法
    kmeans(train_data_left, K, train_labels_left,
           cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
           3, cv::KMEANS_PP_CENTERS, centers_left);
    kmeans(train_data_right, K, train_labels_right,
           cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 10, 1.0),
           3, cv::KMEANS_PP_CENTERS, centers_right);

    //cout<< "centers1: "<<centers_left.at<float>(0)<<endl;
    //cout<< "centers2: "<<centers_left.at<float>(1)<<endl;

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

//    right_eye_thresh = computeMean(right_eye_ratios.begin(),
//                                   right_eye_ratios.begin() + right_eye_ratios.size() / 2);
//    left_eye_thresh = computeMean(left_eye_ratios.begin(), left_eye_ratios.begin() + left_eye_ratios.size() / 2);


    mouth_thresh = computeMean(mouth_ratios.begin() + (mouth_ratios.size() * 3 )/4, mouth_ratios.begin() + mouth_ratios.size());
    //std::cout << "right_eye_thresh " << right_eye_thresh << " " << " left_eye_thresh " << left_eye_thresh << std::endl;
    isCalibrated = true;

}


const float FACTOR = 0.615f;
const float MOUTH_FACTOR = 0.9f;

bool EyeMouthStatus::GetLeftEyeStatus() {
    //assert(isCalibrated);
    if(!isCalibrated) {
        Calibrate1();
    }
    float left_eye_ratio = getLeftEyeRatio(landmark_history.back());
//    std::cout << "left eye ratio : " << left_eye_ratio << " " << little_left_eye_thresh << " " << std::endl;
    return (abs(left_eye_ratio - left_eye_thresh_close)< abs(left_eye_thresh_open - left_eye_ratio))?false:true;
}

bool EyeMouthStatus::GetRightEyeStatus() {
    //assert(isCalibrated);
    if(!isCalibrated) {
        Calibrate1();
    }
    float right_eye_ratio = getRightEyeRatio(landmark_history.back());
//    std::cout << "right eye ratio : " << right_eye_ratio << " " << little_right_eye_thresh<< " ";
    return (abs(right_eye_ratio - right_eye_thresh_close)< abs(right_eye_thresh_open - right_eye_ratio))?false:true;
}

bool EyeMouthStatus::GetMouthStatus() {
    //assert(isCalibrated);
    if(!isCalibrated) {
        Calibrate1();
    }
    float mouth_ratio = getMouthRatio(landmark_history.back());
    std::cout << "ratio : " << mouth_ratio << "  thresh: " << mouth_thresh*MOUTH_FACTOR << std::endl;

    return mouth_ratio > mouth_thresh*MOUTH_FACTOR;
}


float EyeMouthStatus::getLeftEyeRatio(const std::vector<cv::Point2f> & lm) {
    float dist1 = norm(lm[43],lm[47]);
    float dist2 = norm(lm[44],lm[46]);
    float dist3 = norm(lm[42],lm[45]);
    return (dist1 + dist2) / (2 * dist3);
}

float EyeMouthStatus::getRightEyeRatio(const std::vector<cv::Point2f> & lm){
    float dist1 = norm(lm[37],lm[41]);
    float dist2 = norm(lm[38],lm[40]);
    float dist3 = norm(lm[36],lm[39]);
    return (dist1 + dist2) / (2 * dist3);
}

float EyeMouthStatus::getMouthRatio(const std::vector<cv::Point2f> & lm){
    float dist1 = norm(lm[61]-lm[67]);
    float dist2 = norm(lm[62]-lm[66]);
    float dist3 = norm(lm[63]-lm[65]);
    float dist4 = norm(lm[60]-lm[64]);
    return (dist1 + dist2 + dist3) / (3 * dist4);
}