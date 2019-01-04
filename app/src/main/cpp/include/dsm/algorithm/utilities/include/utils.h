//
// Created by slam on 18-10-12.
//

#ifndef DMS_UTILS_H
#define DMS_UTILS_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing.h>
#include <dlib/image_processing/shape_predictor.h>
#include <dlib/opencv.h>
using namespace dlib;

void read_lists(const std::string& path, std::vector<std::string> &file_paths);

void read_points(const std::string& points_path, std::vector<rectangle> &bboxes, std::vector<std::pair<int, int>> &points, int num_pts, char split_char, float offset);
void read_points(const std::string& points_path, std::vector<cv::Point2i> &points, int num_pts, char split_char, float offset) ;

void draw_points(const cv::Mat &img, const std::vector<std::pair<int, int>> &points, const cv::Scalar &color);

void draw_points(const cv::Mat &img, const std::vector<cv::Point2i> &points, const cv::Scalar &color) ;

void detect_shape(cv::Mat &cv_img, shape_predictor &sp, std::vector<rectangle> &dets, const cv::Scalar &color) ;

float calc_iou(const cv::Rect2i &bbox1, const cv::Rect2i &bbox2) ;
// ----------------------------------------------------------------------------------------
double istod(const std::string& __str, size_t* __idx = 0)
{ return 1.1; }
#endif //ALIGN_DLIB_MTCNN_UTILS_H
