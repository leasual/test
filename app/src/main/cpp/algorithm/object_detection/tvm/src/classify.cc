#include "classify.h"
//#include <utils/time/timer.h>
//#include "cmake_config.h"

std::vector<DetResult> DetResult::CreateDetResultFromOriData(std::shared_ptr<TvmData> &rois_p,
                                                             std::shared_ptr<TvmData> &scores_p,
                                                             std::shared_ptr<TvmData> &bbox_delta_p) {
    std::vector<DetResult> result;
    float* rois_pp = static_cast<float*>(rois_p->dltensor->data);
    float* scores_pp = static_cast<float*>(scores_p->dltensor->data);
    float* bbox_delta_pp = static_cast<float*>(bbox_delta_p->dltensor->data);

    for(int i=0;i<3;i++) {
        DetResult t1;
        for(int j=0;j<5;j++) {
            t1.roi.push_back(rois_pp[i*5+j]);
        }
        for(int j=0;j<2;j++) {
            t1.score.push_back(scores_pp[i*2+j]);
        }
        for(int j=0;j<8;j++) {
            t1.bbox_delta.push_back(bbox_delta_pp[i*8+j]);
        }
        result.push_back(t1);
    }
    return result;
}


bool Classification::CompareBBox(const ObjInfo & a, const ObjInfo & b){
    return a.score_ > b.score_;
}

std::vector<ObjInfo> Classification::Nms(std::vector<ObjInfo>& objs, float threshold)  {
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
            float iouScore = Iou(objs[i], objs[j]);
            if (iouScore > threshold)
            {
                mask[j] = true;
            }
        }
    }
    return result;
}

void Classification::InitModel(
        std::string resize_module_path,std::string resize_json_path,std::string resize_params_path,
        std::string det_module_path,std::string det_json_path,std::string det_params_path,
        std::string cls_module_path,std::string cls_json_path,std::string cls_params_path,float cand) {
    resize_runner_p = std::make_shared<TvmRunner>(resize_module_path,resize_json_path,resize_params_path);
    det_runner_p = std::make_shared<TvmRunner>(det_module_path,det_json_path,det_params_path);
    cls_runner_p = std::make_shared<TvmRunner>(cls_module_path,cls_json_path,cls_params_path);
    cand_ = cand;

    im_info_p = std::make_shared<TvmData>("im_info",(const int64_t[]){1,3},2);
    im_info_p->SetData((const float[]){240.0f,320.0f,0.5f},3);
}

void Classification::ResizeTo320240(cv::Mat &bgr,std::shared_ptr<TvmData>& resize_image) {
    static int64_t resize_in_shape[4] = {1,3,480,640};
    static int64_t resize_out_shape[4] = {1,3,240,320};
    auto image_in_td = std::make_shared<TvmData>("data",resize_in_shape,4);
    auto image_out_td = std::make_shared<TvmData>("data",resize_out_shape,4);
    std::vector<std::shared_ptr<TvmData>> inputs;
    std::vector<std::shared_ptr<TvmData>> outputs;
#if defined(COUNT_TIME)
    Timer set_image_timer("Set Image : ");
#endif
    image_in_td->SetImage(bgr);
#if defined(COUNT_TIME)
    std::cout << set_image_timer << std::endl;
#endif

    inputs.push_back(image_in_td);
    outputs.push_back(image_out_td);
    resize_runner_p->operator()(inputs,outputs);
    resize_image = outputs.at(0);
}

void Classification::Detect(cv::Mat &bgr, std::vector<ObjInfo> &clazzes, float threshold) {
    static int64_t rois_shape[2] = {3,5};
    static int64_t scores_shape[3] = {1,3,2};
    static int64_t bbox_deltas[3] = {1,3,8};

    auto rois_p = std::make_shared<TvmData>("",rois_shape,2);
    auto scores_p = std::make_shared<TvmData>("",scores_shape,3);
    auto bbox_deltas_p = std::make_shared<TvmData>("",bbox_deltas,3);
    std::vector<std::shared_ptr<TvmData>> inputs;
    std::vector<std::shared_ptr<TvmData>> outputs;

    // resize
//    std::shared_ptr<TvmData> resized_image;

    cv::Mat resized_mat_;
#if defined(COUNT_TIME)
    Timer cv_resize_time("CV Resize time");
#endif
    cv::resize(bgr,resized_mat_,cv::Size(320,240));
#if defined(COUNT_TIME)
    std::cout << cv_resize_time << std::endl;
#endif

//    Timer resize_time("Resize Time");
//    ResizeTo320240(bgr,resized_image);
//    std::cout << resize_time << std::endl;

    auto resized_image_cv = std::make_shared<TvmData>("data",(const int64_t[]){1,3,240,320},4);
//    Timer cv_set_image_timer("CV Set Image : ");
    resized_image_cv->SetImage(resized_mat_);
//    std::cout << cv_set_image_timer << std::endl;

//    inputs.push_back(resized_image);
    inputs.push_back(resized_image_cv);
    inputs.push_back(im_info_p);
    outputs.push_back(rois_p);
    outputs.push_back(scores_p);
    outputs.push_back(bbox_deltas_p);
    // run
    det_runner_p->operator()(inputs,outputs);

    // analysis result
    // only have 3 value.
    static float scale = 0.5f;
    std::vector<ObjInfo>().swap(clazzes);

    std::vector<DetResult> result = DetResult::CreateDetResultFromOriData(rois_p,scores_p,bbox_deltas_p);
    for(auto& one_result:result) {

        float score = one_result.score.at(1);
        if(score <= threshold) continue;

        float x1 = one_result.roi.at(1) / scale;
        float y1 = one_result.roi.at(2) / scale;
        float x2 = one_result.roi.at(3) / scale;
        float y2 = one_result.roi.at(4) / scale;

        float pb_w = x2 - x1 + 1;
        float pb_h = y2 - y1 + 1;

        // apply bbox regression
        float dx = one_result.bbox_delta.at(4);
        float dy = one_result.bbox_delta.at(5);
        float dw = one_result.bbox_delta.at(6);
        float dh = one_result.bbox_delta.at(7);;

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
    clazzes = Nms(clazzes);
}


int Classification::FilterBboxWithCls(cv::Mat& img,std::vector<ObjInfo>& class_candidate) {
    static int64_t img48_shape[4] = {1,3,48,48};
    static int64_t pred_shape[2] = {1,4};

    std::vector<cv::Rect> result;
    int max_index = 0;

    std::vector<cv::Rect> det_rects;
    for(auto& obj: class_candidate){
        det_rects.emplace_back(obj.rect);
    }
    int num = 0;
    ObjInfo detObj;


    auto in = std::make_shared<TvmData>("data",img48_shape,4);
    auto pred = std::make_shared<TvmData>("",pred_shape,2);

    std::vector<std::shared_ptr<TvmData>> inputs,outputs;
    inputs.push_back(in);
    outputs.push_back(pred);

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
        in->SetImage(roiied_image);

        cls_runner_p->operator()(inputs,outputs);
        float* pred_data = static_cast<float*>(pred->dltensor->data);

        double max_prob = -1;
        for (int i = 0; i < 4 ; i++) {
            if (pred_data[i] > max_prob) {
                max_index = i;
                max_prob = pred_data[i];
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
        }
        num++;
    }
    return max_index;
}

int Classification::Classify(cv::Mat& mat,ObjInfo& info){
    std::vector<ObjInfo> class_candidate;
    Detect(mat,class_candidate,cand_);
    if(class_candidate.empty()) return 0;
    auto id = FilterBboxWithCls(mat,class_candidate);
    if (id > 0){
        objinfo = class_candidate.at(class_candidate.size()-1);
        info = objinfo;
    }
    return id;
}