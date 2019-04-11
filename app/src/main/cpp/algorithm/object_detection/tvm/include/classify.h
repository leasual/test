#pragma once

#include <memory>
#include <string>
#include <algorithm>
#include <vector>
#include <opencv2/opencv.hpp>
#include "tvm_runner.h"

struct ObjInfo {
    cv::Rect rect;
    int label_;
    float score_;
};

class DetResult {
public:
    std::vector<float> roi;
    std::vector<float> score;
    std::vector<float> bbox_delta;
    static std::vector<DetResult> CreateDetResultFromOriData(std::shared_ptr<TvmData>& rois_p,
            std::shared_ptr<TvmData>& scores_p,std::shared_ptr<TvmData>& bbox_delta_p);

    void PrintResult() {
        std::cout << "rois:" << " ";
        for(auto &d:roi)
            std::cout << d << " ";
        std::cout << std::endl;
        std::cout << "scores:" << " ";
        for(auto &d:score)
            std::cout << d << " ";
        std::cout << std::endl;
        std::cout << "bbox_delta:" << " ";
        for(auto &d:bbox_delta)
            std::cout << d << " ";
        std::cout << std::endl;
        std::cout << "------------" << std::endl;
    }
};


class Classification {
public:
    void InitModel(std::string resize_module_path,std::string resize_json_path,std::string resize_params_path,
            std::string det_module_path,std::string det_json_path,std::string det_params_path,
            std::string cls_module_path,std::string cls_json_path,std::string cls_params_path,float cand);
    int Classify(cv::Mat& mat,ObjInfo& objinfo);
private:
    static bool CompareBBox(const ObjInfo & a, const ObjInfo & b);

private:
    float ComputeArea(const ObjInfo& obj) {
        return (obj.rect.width) * (obj.rect.height);
    }
    float Iou(const ObjInfo& obj1, const ObjInfo& obj2) {
        int xL = std::min(obj1.rect.x, obj2.rect.x);
        int yL = std::min(obj1.rect.y, obj2.rect.y);
        int xR = std::max(obj1.rect.x + obj1.rect.width -1, obj2.rect.x + obj2.rect.width -1);
        int yR = std::max(obj1.rect.y + obj1.rect.height -1, obj2.rect.y + obj2.rect.height -1);
    }

    void Detect(cv::Mat &bgr, std::vector<ObjInfo> &clazzes, float threshold);

    void ResizeTo320240(cv::Mat &bgr,std::shared_ptr<TvmData>& resize_image);

    int FilterBboxWithCls(cv::Mat& img,std::vector<ObjInfo>& class_candidate);

    std::vector<ObjInfo> Nms(std::vector<ObjInfo>& objs, float threshold = 0.65);

private:


private:
    std::shared_ptr<TvmRunner> resize_runner_p,det_runner_p,cls_runner_p;
    std::shared_ptr<TvmData> im_info_p;

    ObjInfo objinfo;
    float cand_;
};