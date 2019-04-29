//
// Created by book on 19-3-11.
//

#ifndef DMS_H
#define DMS_H

#pragma once

#include <opencv2/opencv.hpp>
#include <memory>
#include <string>
#include <algorithm>
#include <vector>
#include "net.h"

struct ObjInfo
{
    cv::Rect rect;
    int label_;
    float score_;
};

class Classification {
private:
    float cand_;
    ncnn::Net det_net, cls_net;
    float computeArea(const ObjInfo& obj);
    float iou(const ObjInfo& obj1, const ObjInfo& obj2);
    static bool CompareBBox(const ObjInfo & a, const ObjInfo & b);
    std::vector<ObjInfo> nms(std::vector<ObjInfo>& objs, float threshold = 0.65);
    void Detect(ncnn::Net &net, cv::Mat &bgr, std::vector<ObjInfo> &clazzes, float threshold);
    int FilterBboxWithCls_new(ncnn::Net& cls_net,cv::Mat& img,std::vector<ObjInfo>& class_candidate);

public:

    ObjInfo objinfo;
    Classification(void);

    ~Classification();

    /*param_path1: param path
     * bin_path1 : bin path
     * param_path2: cls param path
     * bin_path2 : cls bin path
     * */
    void InitModel(std::string param_path1,std::string bin_path1, std::string param_path2,std::string bin_path2,float cand);

    /**
     * @param cv::Mat
     * @return detid 0 ~ 3
     */
    int Classify(cv::Mat& mat);
};


#endif //DMS_H
