//
// Created by slam on 18-10-24.
//
#ifndef DMS_ALIGN_METHOD_H
#define DMS_ALIGN_METHOD_H
#include <iostream>
#include <opencv2/opencv.hpp>
#include <dlib/image_processing/full_object_detection.h>
#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>


class AlignMethod{
public:
    AlignMethod()= default;
    virtual ~AlignMethod(){};
    virtual void set_im(const cv::Mat &im) = 0;
//    virtual void set_pts(const std::vector<cv::Point2f> &pts){};
    virtual void set_pts(const std::vector<cv::Point2f> &pts) = 0;
    virtual void set_shape(const dlib::full_object_detection &dlib_shape){};
    virtual cv::Mat Align() = 0;
};

class FivePtsAlign : public AlignMethod{
public:
    FivePtsAlign(const cv::Size &size);
    FivePtsAlign(const int w, const int h);
    FivePtsAlign(const cv::Mat &im, const std::vector<cv::Point2f> &pts, const cv::Size &size);
    virtual ~FivePtsAlign(){};
    cv::Mat Align();
    void set_im(const cv::Mat &im);
    void set_pts(const std::vector<cv::Point2f> &pts);

private:
    cv::Mat _im;
    std::vector<cv::Point2f> _pts;
    cv::Size _size;
};

class DlibAlign : public AlignMethod{
public:
    DlibAlign(const int size);
    DlibAlign(const int w, const int h);
    DlibAlign(const cv::Mat &im, const dlib::full_object_detection &dlib_shape, const int size);
    virtual ~DlibAlign(){};
    cv::Mat Align();
    void set_im(const cv::Mat &im);
    void set_shape(const dlib::full_object_detection &dlib_shape);
    void set_pts(const std::vector<cv::Point2f> &pts);

private:
    cv::Mat _im;
    dlib::full_object_detection _dlib_shape;
    int _size;
};

#endif //DMS_ALIGN_METHOD_H
