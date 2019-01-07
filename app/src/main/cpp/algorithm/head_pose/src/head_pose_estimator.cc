//
// Created by slam on 18-10-8.
//

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include "head_pose_estimator.h"

HeadPoseEstimator::HeadPoseEstimator(cv::Mat &im) {
    double focal_length = im.cols; // Approximate focal length.
    cv::Point2f center = cv::Point2f(im.cols / 2, im.rows / 2);
    camera_matrix_ = (cv::Mat_<double>(3, 3)
            << focal_length, 0., center.x, 0., focal_length, center.y, 0., 0., 1.);
    dist_coeffs_ = cv::Mat::zeros(4, 1, cv::DataType<double>::type); // Assuming no lens distortion

    model_points_.emplace_back(cv::Point3d(0.0f, 0.0f, 0.0f));               // Nose tip
    model_points_.emplace_back(cv::Point3d(0.0f, -330.0f, -65.0f));          // Chin
    model_points_.emplace_back(cv::Point3d(-225.0f, 170.0f, -135.0f));       // Left eye left corner
    model_points_.emplace_back(cv::Point3d(225.0f, 170.0f, -135.0f));        // Right eye right corner
    model_points_.emplace_back(cv::Point3d(-150.0f, -150.0f, -125.0f));      // Left Mouth corner
    model_points_.emplace_back(cv::Point3d(150.0f, -150.0f, -125.0f));       // Right mouth corner
}


HeadPoseEstimator::HeadPoseEstimator(std::string &model_path, const cv::Size &s){
    std::vector<cv::Point3d> model_points_3D;
    load_3D_facemodel_68(model_path, model_points_3D);

    std::vector<int> used_id_3D{36, 39, 42, 45, 30, 31, 32, 33, 34, 35, 52};
    for (auto & i : used_id_3D){
        model_points_.emplace_back(model_points_3D[i]);
    }

    double focal_length = s.width; // Approximate focal length.
    cv::Point2f center = cv::Point2f(s.width / 2, s.height / 2);
    camera_matrix_ = (cv::Mat_<double>(3, 3)
            << focal_length, 0., center.x, 0., focal_length, center.y, 0., 0., 1.);
    dist_coeffs_ = cv::Mat::zeros(4, 1, cv::DataType<double>::type); // Assuming no lens distortion
}

void HeadPoseEstimator::load_3D_facemodel_68(std::string &model_path, std::vector<cv::Point3d> &model_points_3D) {
    std::ifstream infile(model_path.c_str());
    std::string line;
    std::vector<float> temp;
    while(!infile.eof()){
        infile >> line;
        temp.emplace_back(atof(line.c_str()));
    }
    for (int i = 0; i < temp.size() / 3; i++) {
        model_points_3D.emplace_back(cv::Point3d(temp[i], temp[i + 68], -temp[i + 136]));
    }
}
cv::Vec3d HeadPoseEstimator::RotationMatrix2Euler(const cv::Matx33d &rotation_matrix) {
    double q0 = sqrt(1 + rotation_matrix(0, 0) + rotation_matrix(1, 1) + rotation_matrix(2, 2)) / 2.0;
    double q1 = (rotation_matrix(2, 1) - rotation_matrix(1, 2)) / (4.0 * q0);
    double q2 = (rotation_matrix(0, 2) - rotation_matrix(2, 0)) / (4.0 * q0);
    double q3 = (rotation_matrix(1, 0) - rotation_matrix(0, 1)) / (4.0 * q0);

    double t1 = 2.0 * (q0 * q2 + q1 * q3);

    double yaw = asin(2.0 * (q0 * q2 + q1 * q3));
    double pitch = atan2(2.0 * (q0 * q1 - q2 * q3), q0 * q0 - q1 * q1 - q2 * q2 + q3 * q3);
    double roll = atan2(2.0 * (q0 * q3 - q1 * q2), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3);

    return cv::Vec3d(pitch, yaw, roll);
}

cv::Vec3d HeadPoseEstimator::AxisAngle2Euler(const cv::Vec3d &axis_angle) {
    cv::Matx33d rotation_matrix;
    cv::Rodrigues(axis_angle, rotation_matrix);
    return RotationMatrix2Euler(rotation_matrix);
}

cv::Vec3d HeadPoseEstimator::estimate_pose(std::vector<cv::Point2f> image_points) {
    std::vector<int> used_id_2D{44, 48, 52, 56, 38, 39, 40, 41, 42, 43, 63};
    std::vector<cv::Point2f> used_points_2D;
    for (auto & i : used_id_2D){
        used_points_2D.emplace_back(image_points[i]);
    }

    if(used_points_2D.size() != model_points_.size()){
        std::cout << "Numbers of 3D and 2D points are not matched!" << std::endl;
        exit(0);
    }

    // Output rotation and translation
    cv::Mat rotation_vector; // Rotation in axis-angle form
    cv::Mat translation_vector;

    // Solve for pose
    cv::solvePnP(model_points_, used_points_2D, camera_matrix_, dist_coeffs_, rotation_vector, translation_vector);
    cv::Vec3d euler = AxisAngle2Euler(rotation_vector);
    return euler;
}