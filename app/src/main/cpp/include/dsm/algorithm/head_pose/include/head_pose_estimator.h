//
// Created by slam on 18-10-8.
//

#ifndef HEAD_POSE_ESTIMATOR_HEAD_POSE_EXTIMATOR_H
#define HEAD_POSE_ESTIMATOR_HEAD_POSE_EXTIMATOR_H

#include <iostream>
#include <opencv2/core.hpp>

class HeadPoseEstimator {
public:
    HeadPoseEstimator(cv::Mat im);
    cv::Vec3d estimate_pose(std::vector<cv::Point2f> image_points);

private:
    cv::Vec3d RotationMatrix2Euler(const cv::Matx33d &rotation_matrix);
    cv::Vec3d AxisAngle2Euler(const cv::Vec3d &axis_angle);

    std::vector<cv::Point3d> model_points_;
    cv::Mat camera_matrix_;
    cv::Mat dist_coeffs_;
};
#endif //HEAD_POSE_ESTIMATOR_HEAD_POSE_EXTIMATOR_H
