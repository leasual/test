//
// Created by slam on 18-10-8.
//

#ifndef HEAD_POSE_ESTIMATOR_HEAD_POSE_EXTIMATOR_H
#define HEAD_POSE_ESTIMATOR_HEAD_POSE_EXTIMATOR_H

#include <iostream>
#include <opencv2/core.hpp>
#include <fstream>

class HeadPoseEstimator {
public:
    HeadPoseEstimator(cv::Mat &im);
    HeadPoseEstimator(std::string &model_path, const cv::Size &s);
    cv::Vec3d estimate_pose(std::vector<cv::Point2f> image_points);

private:
    void load_3D_facemodel_68(std::string &model_path, std::vector<cv::Point3d> &model_points_3D);
    cv::Vec3d RotationMatrix2Euler(const cv::Matx33d &rotation_matrix);
    cv::Vec3d AxisAngle2Euler(const cv::Vec3d &axis_angle);

    std::vector<cv::Point3d> model_points_;
    cv::Mat camera_matrix_;
    cv::Mat dist_coeffs_;
};
#endif //HEAD_POSE_ESTIMATOR_HEAD_POSE_EXTIMATOR_H
