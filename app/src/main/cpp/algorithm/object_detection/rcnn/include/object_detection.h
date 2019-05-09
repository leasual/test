//
// Created by untouch on 19-1-17.
//

#ifndef HAND_MODEL_TEST_OBJECT_DETECTION_H
#define HAND_MODEL_TEST_OBJECT_DETECTION_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include "net.h"
#include "classify.h"


class ObjectResult;

class ObjectDetect{
public:
    explicit ObjectDetect(const std::string & model);
    std::shared_ptr<ObjectResult> Detect(const cv::Mat& image);
private:
    size_t index_;
    std::shared_ptr<std::vector<std::shared_ptr<Classification>>>classification_;
    std::shared_ptr<ObjectResult> object_result_;
private:
    size_t index();
};

class ObjectResult{
public:
    friend ObjectDetect;
    bool smoke() const;
    bool call() const;
    bool face() const ;
    cv::Rect smoke_bbox()const;
    cv::Rect call_bbox() const;
    cv::Rect face_bbox() const;

private:
    ObjectResult();
    std::vector<ObjInfo> result_;
};

#endif //HAND_MODEL_TEST_OBJECT_DETECTION_H
