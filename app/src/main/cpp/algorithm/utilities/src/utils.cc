//
// Created by slam on 18-10-12.
//
#ifdef USE_DLIB

#ifndef ALIGN_DLIB_MTCNN_UTILS_H
#define ALIGN_DLIB_MTCNN_UTILS_H

#include "utils.h"
void read_lists(const std::string& path, std::vector<std::string> &file_paths){
    std::string line;
    std::ifstream infile(path.c_str());
    while(!infile.eof()){
        infile >> line;
        file_paths.emplace_back(line);
    }
}

void read_points(const std::string& points_path, std::vector<rectangle> &bboxes, std::vector<std::pair<int, int>> &points, int num_pts, char split_char, float offset){
    std::string line, item;
    std::ifstream infile(points_path.c_str());
    int cnt = 0;
    std::string x, y;
    points.clear();
    std::vector<int> x_vec, y_vec;
    while (std::getline(infile, line) && cnt < num_pts) {
        cnt += 1;
        if(split_char != NULL) {
            size_t pos = line.find(split_char);
            x = line.substr(0, pos);
            y = line.substr(pos + 1, pos);
        }
        else {
            std::istringstream ss(line);
            ss >> x >> y;
        }
        points.emplace_back(std::make_pair(round(std::stod(x)) + offset, round(std::stod(y)) + offset));
        x_vec.emplace_back(round(std::stod(x)));
        y_vec.emplace_back(round(std::stod(x)));
    }
    std::sort(x_vec.begin(), x_vec.end());
    std::sort(y_vec.begin(), y_vec.end());
    int x_min = x_vec[0];
    int x_max = x_vec[num_pts-1];
    int y_min = y_vec[0];
    int y_max = y_vec[num_pts-1];
    std::cout << x_min << ' ' << x_max << ' ' << y_min << ' ' << y_max << std::endl;
    rectangle rect(x_min, y_min, x_max, y_max);
    bboxes.emplace_back(rect);
}
void read_points(const std::string& points_path, std::vector<cv::Point2i> &points, int num_pts, char split_char, float offset) {
    std::string line, item;
    std::ifstream infile(points_path.c_str());
    int cnt = 0;
    std::string x, y;
    points.clear();
    std::vector<int> x_vec, y_vec;
    while (std::getline(infile, line) && cnt < num_pts) {
        cnt += 1;
        if(split_char != NULL) {
            size_t pos = line.find(split_char);
            x = line.substr(0, pos);
            y = line.substr(pos + 1, pos);
        }
        else {
            std::istringstream ss(line);
            ss >> x >> y;
        }
        points.emplace_back(cv::Point2i(std::round(atof(x.c_str())) + offset, std::round(atof(y.c_str())) + offset));
    }
}

void draw_points(const cv::Mat &img, const std::vector<std::pair<int, int>> &points, const cv::Scalar &color){
    for (size_t i = 0; i < points.size(); i++){
        cv::circle(img, cv::Point2i(points[i].first, points[i].second), 2, color);
    }
}

void draw_points(const cv::Mat &img, const std::vector<cv::Point2i> &points, const cv::Scalar &color) {
    for (size_t i = 0; i < points.size(); i++){
        cv::circle(img, points[i], 2, color);
    }
}

void detect_shape(cv::Mat &cv_img, shape_predictor &sp, std::vector<rectangle> &dets, const cv::Scalar &color) {
    dlib::cv_image<dlib::bgr_pixel> img(cv_img);
    for (unsigned long j = 0; j < dets.size(); ++j)
    {
        full_object_detection shape = sp(img, dets[j]);
        std::vector<std::pair<int, int>> cv_shape;
        for (size_t k = 0; k < shape.num_parts(); k++) {
            cv_shape.emplace_back(std::make_pair(shape.part(k).x(), shape.part(k).y()));
        }
        draw_points(cv_img, cv_shape, color);
    }
}

float calc_iou(const cv::Rect2i &bbox1, const cv::Rect2i &bbox2) {
    int x_l = std::max(bbox1.x, bbox2.x);
    int y_t = std::max(bbox1.y, bbox2.y);
    int x_r = std::min(bbox1.x+bbox1.width, bbox2.x+bbox2.width);
    int y_d = std::min(bbox1.y+bbox1.height, bbox2.y+bbox2.height);

    int inter_area = (x_r - x_l) * (y_d - y_t);
    int union_area = bbox1.area() + bbox2.area() - inter_area;

    return float(inter_area) / float(union_area);
}
// ----------------------------------------------------------------------------------------

#endif //ALIGN_DLIB_MTCNN_UTILS_H
#endif
