#include <string>
#include <iostream>

#include "tvm_runner.h"
#include "tvm_data.h"

int main(void) {
    cv::Mat image = cv::imread("../assets/imgs/1.jpg");
    std::string model_root = "../assets/models/x86_64-collections/";
    std::string deploy = "deploy.so";
    std::string json_file = "deploy_graph.json";
    std::string params_file = "deploy_param.params";

    std::string resize_module_path = model_root + "sym_bilinear_resize_deploy_x86_64/"+deploy;
    std::string resize_json_path = model_root + "sym_bilinear_resize_deploy_x86_64/"+json_file;
    std::string resize_params_path = model_root + "sym_bilinear_resize_deploy_x86_64/"+params_file;


    std::string module_path = model_root + "sym_face_deploy_x86_64/deploy.so";
    std::string json_path = model_root + "sym_face_deploy_x86_64/deploy_graph.json";
    std::string params_path = model_root + "sym_face_deploy_x86_64/deploy_param.params";

    auto resize_in_data_p = std::make_shared<TvmData>("data",(const int64_t[]){1,3,480,640},4);
    resize_in_data_p->SetImage(image);

    auto resize_out_data_p = std::make_shared<TvmData>("data",(const int64_t[]){1,3,240,320},4);

    std::vector<std::shared_ptr<TvmData>> resize_inputs;
    std::vector<std::shared_ptr<TvmData>> resize_outputs;
    resize_inputs.push_back(resize_in_data_p);
    resize_outputs.push_back(resize_out_data_p);

    TvmRunner resize_runner(resize_module_path,resize_json_path,resize_params_path);
    resize_runner(resize_inputs,resize_outputs);

    auto im_info_p = std::make_shared<TvmData>("im_info",(const int64_t[]){1,3},2);
    im_info_p->SetData((const float[]){240.0f,320.0f,0.5f},3);

    std::vector<std::shared_ptr<TvmData>> det_inputs;
    std::vector<std::shared_ptr<TvmData>> det_outputs;
    det_inputs.push_back(resize_outputs.at(0));
    det_inputs.push_back(im_info_p);

    auto rois_p = std::make_shared<TvmData>("",(const int64_t[]){3,5},2);
    auto scores_p = std::make_shared<TvmData>("",(const int64_t[]){1,3,2},3);
    auto bbox_deltas = std::make_shared<TvmData>("",(const int64_t[]){1,3,8},3);

    det_outputs.push_back(rois_p);
    det_outputs.push_back(scores_p);
    det_outputs.push_back(bbox_deltas);

    TvmRunner det_runner(module_path,json_path,params_path);
    det_runner(det_inputs,det_outputs);

    auto iter = static_cast<float*>(det_outputs.at(0)->dltensor->data);
    for(int i=0;i<3;i++) {
        for(int j=0;j<5;j++) {
            std::cout << iter[i*5+j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "------------------" << std::endl;
    auto iter_scores = static_cast<float*>(det_outputs.at(1)->dltensor->data);
    for(int i=0;i<3;i++) {
        for(int j=0;j<2;j++) {
            std::cout << iter_scores[i*2+j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "------------------" << std::endl;

    auto iter_bbox_deltas = static_cast<float*>(det_outputs.at(2)->dltensor->data);
    for(int i=0;i<3;i++) {
        for(int j=0;j<8;j++) {
            std::cout << iter_bbox_deltas[i*8+j] << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}