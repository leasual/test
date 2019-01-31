//
// Created by untouch on 19-1-17.
//

#include "object_detection.h"

ObjectDetect::ObjectDetect(const std::string &path_root):
        index_(0) ,
        net_(new std::vector<std::shared_ptr<ncnn::Net>>),
        result_(new ObjectResult) ,
        cls_net_(new ncnn::Net){
    std::shared_ptr<ncnn::Net> smoke_net(new ncnn::Net);
    std::shared_ptr<ncnn::Net> call_net(new ncnn::Net);
    std::shared_ptr<ncnn::Net> face_net(new ncnn::Net);

    std::string smoke_param, smoke_model, call_param, call_model,
            face_param, face_model, cls_param, cls_model;
    smoke_param = path_root + "/object_detection/tiny_prelu2_smoking.param";
    smoke_model = path_root + "/object_detection/tiny_prelu2_smoking.bin";
    call_param = path_root + "/object_detection/tiny_prelu2_phone.param";
    call_model = path_root + "/object_detection/tiny_prelu2_phone.bin";
    face_param = path_root + "/object_detection/tiny_prelu2_face.param";
    face_model = path_root + "/object_detection/tiny_prelu2_face.bin";
    cls_param = path_root + "/object_detection/cls.param";
    cls_model = path_root + "/object_detection/cls.bin";

    smoke_net->load_param(smoke_param.c_str());
    smoke_net->load_model(smoke_model.c_str());
    call_net->load_param(call_param.c_str());
    call_net->load_model(call_model.c_str());
    face_net->load_param(face_param.c_str());
    face_net->load_model(face_model.c_str());
    cls_net_->load_param(cls_param.c_str());
    cls_net_->load_model(cls_model.c_str());

    net_->push_back(smoke_net);
    net_->push_back(call_net);
    net_->push_back(face_net);
}

std::shared_ptr<ObjectResult> ObjectDetect::Detect(const cv::Mat &image) {
    auto idx = index();
    auto net = net_->at(idx);
    std::vector<ObjInfo>clazzes;
    const int target_size = 320;
    int org_w = image.cols;
    int org_h = image.rows;
    float threshold = 0.9;
    float scale = 1.f;
    float target_w = 320, target_h = 240;
    if (org_w > org_h) {
        scale = (float) target_size / org_w;
        target_w = target_size;
        target_h = org_h * scale;
    } else {
        scale = (float) target_size / org_h;
        target_h = target_size;
        target_w = org_w * scale;
    }

    cv::Mat input_img;
    cv::resize(image, input_img, cv::Size(target_w, target_h));

    ncnn::Mat in = ncnn::Mat::from_pixels(input_img.data, ncnn::Mat::PIXEL_BGR, target_w, target_h);

    ncnn::Mat im_info(3);
    im_info[0] = org_h;
    im_info[1] = org_w;
    im_info[2] = scale;

    ncnn::Extractor ex1 = net->create_extractor();
    ex1.set_num_threads(1);

    ex1.input("data", in);
    ex1.input("im_info", im_info);

    ncnn::Mat cls_prob, bbox_pred, rois;

    ncnn::Mat conv2_3, conv3_4;
    ex1.extract("conv2_3", conv2_3);
    ex1.extract("conv3_4", conv3_4);
    ex1.extract("rois", rois);

    // step2, extract bbox and score for each roi
    std::vector<ObjInfo> class_candidates;
    for (int i = 0; i < rois.c; i++) {
        ncnn::Extractor ex2 = net->create_extractor();

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
        float x1 = roi[0] / scale;
        float y1 = roi[1] / scale;
        float x2 = roi[2] / scale;
        float y2 = roi[3] / scale;

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
        obj_x1 = std::max(std::min(obj_x1, (float) (image.cols - 1)), 0.f);
        obj_y1 = std::max(std::min(obj_y1, (float) (image.rows - 1)), 0.f);
        obj_x2 = std::max(std::min(obj_x2, (float) (image.cols - 1)), 0.f);
        obj_y2 = std::max(std::min(obj_y2, (float) (image.rows - 1)), 0.f);

        // append object
        ObjInfo obj;
        obj.xL_ = obj_x1;
        obj.yL_ = obj_y1;
        obj.xR_ = obj_x2;
        obj.yR_ = obj_y2;

        obj.score_ = score;
        clazzes.push_back(obj);
    }
    clazzes = nms(clazzes);
    std::vector<cv::Rect> rects;
    for(auto& obj: clazzes){
        rects.emplace_back(cv::Point(obj.xL_, obj.yL_), cv::Point(obj.xR_, obj.yR_));
    }
    int label = idx + 1;
    auto bboxs = FilterBboxWithCls(image,rects,label);
//    if(label == 2)
//        std::cout << label<< " " << clazzes.size() << " " << bboxs.size() << std::endl;

    std::vector<ObjInfo> classes;
    for(auto& bbox : bboxs){
        for(auto& obj: clazzes){
            cv::Rect rect(cv::Point(obj.xL_, obj.yL_), cv::Point(obj.xR_, obj.yR_));
            if(rect == bbox)
                classes.push_back(obj);
        }
    }

    if(bboxs.empty()) result_->result_[idx] = ObjInfo();
    else{
        std::partial_sort(classes.begin(), classes.begin()+1, classes.end(), [](const ObjInfo& a, const ObjInfo& b){ return a.score_ > b.score_;});
        result_->result_[idx] = classes[0];
    }
    return result_;
}

std::vector<cv::Rect> ObjectDetect::FilterBboxWithCls(const cv::Mat& img, std::vector<cv::Rect>& rects, int label){
    std::vector<cv::Rect> result;
    for(auto& rect : rects) {
        // 1) apply roi and resize image to (48,48)
        // 2) input image to net
        // 3) forward get result
        // 4*) nms
        // 5) append result to result-list.
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
        ncnn::Extractor ex1 = cls_net_->create_extractor();
        ex1.input("data", in);

        ncnn::Mat pred;
        ex1.extract("pred", pred);

        int max_index = 0;
        double max_prob = -1;
        for (size_t i = 0; i < pred.total(); i++) {
            if (pred[i] > max_prob) {
                max_index = i;
                max_prob = pred[i];
            }
        }
        if(max_index == label) {
            result.push_back(rect);
        }
    }
    return result;
}

float ObjectDetect::iou(const ObjInfo &obj1, const ObjInfo &obj2) {
    int xL = std::min(obj1.xL_, obj2.xL_);
    int yL = std::min(obj1.yL_, obj2.yL_);
    int xR = std::max(obj1.xR_, obj2.xR_);
    int yR = std::max(obj2.yR_, obj2.yR_);

    float intersection = (xR - xL + 1) * (yR - yL + 1);

    float area1 = computeArea(obj1);
    float area2 = computeArea(obj2);
    return intersection / (area1 + area2 - intersection);
}


std::vector<ObjInfo> ObjectDetect::nms(std::vector<ObjInfo> &objs, float threshold) {
    std::sort(objs.begin(), objs.end(), [](const ObjInfo & a, const ObjInfo & b){return a.score_ > b.score_;});

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

bool ObjectResult::smoke() const {
    return result_[0].score_ != 0.f;
}

bool ObjectResult::call() const {
    return result_[1].score_ != 0.f;
}

bool ObjectResult::face() const {
    return result_[2].score_ != 0.f;
}

cv::Rect ObjectResult::smoke_bbox() const {
    return cv::Rect(cv::Point(result_[0].xL_, result_[0].yL_), cv::Point(result_[0].xR_, result_[0].yR_));
}

cv::Rect ObjectResult::call_bbox() const {
    return cv::Rect(cv::Point(result_[1].xL_, result_[1].yL_), cv::Point(result_[1].xR_, result_[1].yR_));
}

cv::Rect ObjectResult::face_bbox() const {
    return cv::Rect(cv::Point(result_[2].xL_, result_[2].yL_), cv::Point(result_[2].xR_, result_[2].yR_));
}

ObjectResult::ObjectResult() :result_(3){
}
