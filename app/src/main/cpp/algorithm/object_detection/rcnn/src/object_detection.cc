//
// Created by untouch on 19-1-17.
//

#include "object_detection.h"

ObjectDetect::ObjectDetect(const std::string &path_root):
        index_(0) ,
        classification_(new std::vector<std::shared_ptr<Classification>>),
        object_result_(new ObjectResult){

    std::shared_ptr<Classification> smoke_classify(new Classification);
    std::shared_ptr<Classification> call_classify(new Classification);
    std::shared_ptr<Classification> face_classify(new Classification);

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

    smoke_classify->InitModel(smoke_param, smoke_model, cls_param, cls_model, 0.8f);
    call_classify->InitModel(call_param, call_model, cls_param, cls_model, 0.9f);
    face_classify->InitModel(face_param, face_model, cls_param, cls_model, 0.6f);

    classification_->push_back(smoke_classify);
    classification_->push_back(call_classify);
    classification_->push_back(face_classify);
}

std::shared_ptr<ObjectResult> ObjectDetect::Detect(const cv::Mat &image) {
    cv::Mat frame_ = image.clone();
    auto idx = index();
    auto classify = classification_->at(idx);
    int label = idx + 1;
    if(label == classify->Classify(frame_)) {
        //由于classify并没有返回分数，这里传一个假值。
        classify->objinfo.score_ = 1.f;
        object_result_->result_[idx] = classify->objinfo;
    }
    else
        object_result_->result_[idx] = ObjInfo();

    return object_result_;
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
    return result_[0].rect;
}

cv::Rect ObjectResult::call_bbox() const {
    return result_[1].rect;
}

cv::Rect ObjectResult::face_bbox() const {
    return result_[2].rect;
}

ObjectResult::ObjectResult() :result_(3){
}
