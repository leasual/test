//
// Created by untouch on 19-1-17.
//

#include "object_detection.h"

ObjectDetect::ObjectDetect(const std::string &path_root):net_(new std::vector<std::shared_ptr<ncnn::Net>>), result_(new ObjectResult){
    std::shared_ptr<ncnn::Net> net(new ncnn::Net);

    std::string param, model;
    param = path_root + "/object_detection/ssd.param";
    model = path_root + "/object_detection/ssd.bin";

    net->load_param(param.c_str());
    net->load_model(model.c_str());

    net_->push_back(net);
}

std::shared_ptr<ObjectResult> ObjectDetect::Detect(const cv::Mat &image) {
    result_->Reset();
    auto net = net_->at(0);
    std::vector<ObjInfo>clazzes;
    const int target_size = 300;
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

    int img_w = image.cols;
    int img_h = image.rows;

    ncnn::Mat in = ncnn::Mat::from_pixels(input_img.data, ncnn::Mat::PIXEL_BGR, target_w, target_h);

    const float mean_vals[3] = {127.5f, 127.5f, 127.5f};
    const float norm_vals[3] = {1.0/127.5,1.0/127.5,1.0/127.5};
    in.substract_mean_normalize(mean_vals, norm_vals);

    ncnn::Extractor ex = net->create_extractor();
    ex.set_num_threads(1);

    ex.input("data", in);

    ncnn::Mat out;
    ex.extract("detection_out",out);

    for (int i=0; i<out.h; i++)
    {
        const float* values = out.row(i);

        ObjInfo object;
        object.label_= values[0];
        object.score_= values[1];
        object.xL_ = values[2] * img_w;
        object.yL_ = values[3] * img_h;
        object.xR_ = values[4] * img_w;
        object.yR_ = values[5] * img_h;

        clazzes.push_back(object);

    }

    for (auto& object: clazzes) {
        if (1 == object.label_ ) { //smoke
            if (object.score_ > result_->result_[0].score_)
                result_->result_[0] = object;
        } else if (2 == object.label_) { //call
            if (object.score_ > result_->result_[1].score_)
                result_->result_[1] = object;
        } else if (3 == object.label_) { //face
            if (object.score_ > result_->result_[2].score_)
                result_->result_[2] = object;
        }
    }
    return result_;
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
