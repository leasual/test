#include <iostream>
#include <chrono>
#include "net.h"
#include <opencv2/opencv.hpp>
#include <vector>

#include "classify.h"



float Classification::computeArea(const ObjInfo& obj)  {
    return (obj.rect.width) * (obj.rect.height);
}

float Classification::iou(const ObjInfo& obj1, const ObjInfo& obj2) {
    int xL = std::min(obj1.rect.x, obj2.rect.x);
    int yL = std::min(obj1.rect.y, obj2.rect.y);
    int xR = std::max(obj1.rect.x + obj1.rect.width -1, obj2.rect.x + obj2.rect.width -1);
    int yR = std::max(obj1.rect.y + obj1.rect.height -1, obj2.rect.y + obj2.rect.height -1);

    float intersection = (xR - xL + 1) * (yR - yL + 1);

    float area1 = computeArea(obj1);
    float area2 = computeArea(obj2);
    return intersection / (area1 + area2 - intersection);
}

bool Classification::CompareBBox(const ObjInfo & a, const ObjInfo & b) {
    return a.score_ > b.score_;
}

std::vector<ObjInfo> Classification::nms(std::vector<ObjInfo>& objs, float threshold)  {
    std::sort(objs.begin(), objs.end(), CompareBBox);

    std::vector<ObjInfo> result;
    int objNum = objs.size();
    std::vector<bool> mask(objNum, false);

    for (int i = 0; i < objNum; i++)
    {
        if(!mask[i])
        {
            result.push_back(objs[i]);
            mask[i] = true;
        }
        for (int j = i + 1; j < objNum; j++)
        {
            if(mask[j])
            {
                continue;
            }
            float iouScore = iou(objs[i], objs[j]);
            if (iouScore > threshold)
            {
                mask[j] = true;
            }
        }
    }
    return result;
}


void Classification::Detect(ncnn::Net &net, cv::Mat &bgr, std::vector<ObjInfo> &clazzes, float threshold) {
    const int target_size = 320;
    int org_w = bgr.cols;
    int org_h = bgr.rows;

//    float threshold = 0.9;
    float scale = 1.f;
    float scale_x = (float) target_size / org_w;
    float scale_y = (float) 240 / org_h;
    float target_w = 320, target_h = 240;


    target_w = 320;
    target_h = 240;
    cv::Mat input_img;
    try {
        cv::resize(bgr, input_img, cv::Size(target_w, target_h));
    }catch (...){
        std::cout << "fuck" << std::endl;
        return;
    }

//    cv::imshow("showimage", input_img);
//    cv::waitKey(0);
    ncnn::Mat in = ncnn::Mat::from_pixels(input_img.data, ncnn::Mat::PIXEL_BGR, target_w, target_h);

    ncnn::Mat im_info(3);
    im_info[0] = org_h;
    im_info[1] = org_w;
    im_info[2] = scale;

    ncnn::Extractor ex1 = net.create_extractor();
    ex1.set_num_threads(1);

    ex1.input("data", in);
    ex1.input("im_info", im_info);

    ncnn::Mat cls_prob, bbox_pred, rois;

    ncnn::Mat conv2_3, conv3_4;
    ex1.extract("conv2_3", conv2_3);
    ex1.extract("conv3_4", conv3_4);
    ex1.extract("rois", rois);


    /*************************************************/
//    int index = 0;
//    std::ofstream roiout("/home/book/CLionProjects/zyc/rcnn-object-detection/rois.txt",std::fstream::out|std::fstream::binary);
//    assert(roiout.is_open());
//
//    for(int i = 0; i<rois.c; i++){
//        for(int j = 0; j<rois.w; j++){
//            for(int k = 0; k<rois.h;k++){
//                float data = ((float*)(rois))[index++];
//                roiout<<data<<'\t';
//                std::cout<<data<<std::endl;
//            }
////            roiout<<std::endl;
//        }
//        roiout<<std::endl;
//    }
//
//    roiout.close();

    /***************************************************/


    // step2, extract bbox and score for each roi
    std::vector<ObjInfo> class_candidates;
    for (int i = 0; i < rois.c; i++) {
        ncnn::Extractor ex2 = net.create_extractor();

        // get single roi
        ncnn::Mat roi = rois.channel(i);

        // according different roi check different cls
        ex2.input("conv2_3", conv2_3);
        ex2.input("conv3_4", conv3_4);
        ex2.input("rois", roi);

        ncnn::Mat bbox_pred;
        ncnn::Mat cls_prob;
        ex2.extract("bbox_pred", bbox_pred);
        ex2.extract("cls_prob", cls_prob);

        float score = cls_prob.channel(0)[1];

        if (score <= threshold) {
            continue;
        }

        int num_class = cls_prob.w;
        class_candidates.resize(num_class);

        // unscale to image size

        float x1 = roi[0] / scale_x;
        float y1 = roi[1] / scale_y;
        float x2 = roi[2] / scale_x;
        float y2 = roi[3] / scale_y;

        float pb_w = x2 - x1 + 1;
        float pb_h = y2 - y1 + 1;

        // apply bbox regression
        float dx = bbox_pred.channel(0)[4];
        float dy = bbox_pred.channel(0)[5];
        float dw = bbox_pred.channel(0)[6];
        float dh = bbox_pred.channel(0)[7];

        float cx = x1 + pb_w * 0.5f;
        float cy = y1 + pb_h * 0.5f;

        float obj_cx = cx + pb_w * dx;
        float obj_cy = cy + pb_h * dy;

        float obj_w = pb_w * exp(dw);
        float obj_h = pb_h * exp(dh);

        float obj_x1 = obj_cx - obj_w * 0.5f;
        float obj_y1 = obj_cy - obj_h * 0.5f;
        float obj_x2 = obj_cx + obj_w * 0.5f;
        float obj_y2 = obj_cy + obj_h * 0.5f;

        // clip
        obj_x1 = std::max(std::min(obj_x1, (float) (bgr.cols - 1)), 0.f);
        obj_y1 = std::max(std::min(obj_y1, (float) (bgr.rows - 1)), 0.f);
        obj_x2 = std::max(std::min(obj_x2, (float) (bgr.cols - 1)), 0.f);
        obj_y2 = std::max(std::min(obj_y2, (float) (bgr.rows - 1)), 0.f);

        // append object
        ObjInfo obj;

        obj.rect.x = obj_x1;
        obj.rect.y = obj_y1;
        obj.rect.width = obj_x2 - obj_x1 +1;
        obj.rect.height= obj_y2 - obj_y1 +1;

        obj.score_ = score;
        clazzes.push_back(obj);
    }
    clazzes = nms(clazzes);
}


int Classification::FilterBboxWithCls_new(ncnn::Net& cls_net,cv::Mat& img,std::vector<ObjInfo>& class_candidate) {
    std::vector<cv::Rect> result;
    int max_index = 0;

    std::vector<cv::Rect> det_rects;
    for(auto& obj: class_candidate){
        det_rects.emplace_back(obj.rect);

    }
    int num = 0;
    ObjInfo detObj;
    for(auto& rect : det_rects) {

        if(rect.x < 0 )
            rect.x = 0;
        if(rect.y < 0 )
            rect.y = 0;
        if(rect.x + rect.width > img.cols)
            rect.width = img.cols-rect.x;
        if(rect.y + rect.height > img.rows)
            rect.height = img.rows - rect.y;

        cv::Mat roiied_image = img(rect).clone();
        cv::resize(roiied_image,roiied_image,cv::Size(48,48));
        ncnn::Mat in = ncnn::Mat::from_pixels(roiied_image.data, ncnn::Mat::PIXEL_BGR, 48, 48);
        ncnn::Extractor ex1 = cls_net.create_extractor();
        ex1.input("data", in);

        ncnn::Mat pred;
        ex1.extract("pred", pred);

        double max_prob = -1;
        for (int i = 0; i < pred.total(); i++) {
            if (pred[i] > max_prob) {
                max_index = i;
                max_prob = pred[i];
            }
        }
        if(max_index > 0) {
            result.push_back(rect);
            detObj.rect = rect;
            detObj.label_ = max_index;
            detObj.score_ = class_candidate.at(num).score_;

            class_candidate.clear();
            class_candidate.push_back(detObj);

            break;
//            std::cout<<"max_index:"<<max_index-1<<std::endl;
        }
        num++;
    }
    return max_index;
}


Classification::Classification(void) {
    std::cout << "Object is being created" << std::endl;
}
Classification::~Classification(void) {
    std::cout << "Object is being ~" << std::endl;
}
void Classification::InitModel(std::string param_path1, std::string bin_path1, std::string param_path2,std::string bin_path2, float cand){
    det_net.load_param(param_path1.c_str());
    det_net.load_model(bin_path1.c_str());
    cls_net.load_param(param_path2.c_str());
    cls_net.load_model(bin_path2.c_str());
    cand_ = cand;
}

int Classification::Classify(cv::Mat& mat){

    std::vector<ObjInfo> class_candidate;

    Detect(det_net, mat, class_candidate, cand_);
    if(class_candidate.empty())
    {
        return 0;
    }

    auto id= FilterBboxWithCls_new(cls_net,mat,class_candidate);
    if (id > 0){
        objinfo = class_candidate.at(class_candidate.size()-1);
    }

    return id;
}