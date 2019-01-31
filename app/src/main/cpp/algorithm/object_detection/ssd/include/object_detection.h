//
// Created by untouch on 19-1-17.
//

#ifndef DSM_OBJECT_DETECTION_H
#define DSM_OBJECT_DETECTION_H

#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include "net.h"

struct ObjInfo
{
    ObjInfo():xL_(0),yL_(0),xR_(0),yR_(0),label_(-1),score_(0){}
    int xL_;
    int yL_;
    int xR_;
    int yR_;
    int label_;
    float score_;
};

class ObjectResult;

class ObjectDetect{
public:
    explicit ObjectDetect(const std::string & model);
    std::shared_ptr<ObjectResult> Detect(const cv::Mat& image);

private:
    float computeArea(const ObjInfo& obj)  { return (obj.xR_ - obj.xL_ + 1) * (obj.yR_ - obj.yL_ + 1); }
    float iou(const ObjInfo& obj1, const ObjInfo& obj2);
    std::vector<ObjInfo> nms(std::vector<ObjInfo>& objs, float threshold = 0.65);
private:
    std::shared_ptr<std::vector<std::shared_ptr<ncnn::Net>>> net_;
    std::shared_ptr<ObjectResult> result_;
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
    void Reset(){
        for(auto& obj : result_){
            obj = ObjInfo();
        }
    }
    ObjectResult();
    std::vector<ObjInfo> result_;
};

#endif //DSM_OBJECT_DETECTION_H
