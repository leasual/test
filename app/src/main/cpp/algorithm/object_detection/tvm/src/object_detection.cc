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

    std::string resize_module_path = path_root + "/resize_deploy/deploy.so";
    std::string resize_json_path = path_root + "/resize_deploy/deploy_graph.json";
    std::string resize_params_path = path_root + "/resize_deploy/deploy_param.params";

    std::string cls_module_path = path_root + "/cls_deploy/deploy.so";
    std::string cls_json_path = path_root + "/cls_deploy/deploy_graph.json";
    std::string cls_params_path = path_root + "/cls_deploy/deploy_param.params";

    std::string face_module_path = path_root + "/face_deploy/deploy.so";
    std::string face_json_path = path_root + "/face_deploy/deploy_graph.json";
    std::string face_params_path = path_root + "/face_deploy/deploy_param.params";

    std::string smoke_module_path = path_root + "/smoking_deploy/deploy.so";
    std::string smoke_json_path = path_root + "/smoking_deploy/deploy_graph.json";
    std::string smoke_params_path = path_root + "/smoking_deploy/deploy_param.params";

    std::string call_module_path = path_root + "/phone_deploy/deploy.so";
    std::string call_json_path = path_root + "/phone_deploy/deploy_graph.json";
    std::string call_params_path = path_root + "/phone_deploy/deploy_param.params";

    smoke_classify->InitModel(resize_module_path,resize_json_path,resize_params_path,
                             smoke_module_path,smoke_json_path,smoke_params_path,
                             cls_module_path,cls_json_path,cls_params_path,0.8f);

    call_classify->InitModel(resize_module_path,resize_json_path,resize_params_path,
              call_module_path,call_json_path,call_params_path,
              cls_module_path,cls_json_path,cls_params_path,0.9f);

    face_classify->InitModel(resize_module_path,resize_json_path,resize_params_path,
              face_module_path,face_json_path,face_params_path,
              cls_module_path,cls_json_path,cls_params_path,0.6f);

    classification_->push_back(smoke_classify);
    classification_->push_back(call_classify);
    classification_->push_back(face_classify);
}

std::shared_ptr<ObjectResult> ObjectDetect::Detect(const cv::Mat &image) {
    cv::Mat frame_ = image.clone();
    auto idx = index();
    auto classify = classification_->at(idx);
    int label = idx + 1;
//    std::cout <<"idx : " << idx << "  " << " label : " << label << std::endl;
    ObjInfo objInfo;
    if(label == classify->Classify(frame_, objInfo)) {
        //由于classify并没有返回分数，这里传一个假值。
        objInfo.score_ = 1.f;
        object_result_->result_[idx] = objInfo;
    }
    else
        object_result_->result_[idx] = ObjInfo();

    /*
    {
        auto idx = index();
        auto classify = classification_->at(idx);
        int label = idx + 1;
        ObjInfo objInfo;
        if(label == classify->Classify(frame_, objInfo)) {
            //由于classify并没有返回分数，这里传一个假值。
            objInfo.score_ = 1.f;
            object_result_->result_[idx] = objInfo;
        }
        else
            object_result_->result_[idx] = ObjInfo();

        std::cout <<"idx : " << idx << "  " << " label : " << label << std::endl;
    }

    {
        auto idx = index();
        auto classify = classification_->at(idx);
        int label = idx + 1;
        ObjInfo objInfo;
        if(label == classify->Classify(frame_, objInfo)) {
            //由于classify并没有返回分数，这里传一个假值。
            objInfo.score_ = 1.f;
            object_result_->result_[idx] = objInfo;
        }
        else
            object_result_->result_[idx] = ObjInfo();
        std::cout <<"idx : " << idx << "  " << " label : " << label << std::endl;
    }

     */
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
