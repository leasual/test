//
// Created by untouch.
//

#include <cmath>
#include <faceID.h>
//#include <faceAttribute.h>
#include "total_flow.h"
#include <pthread.h>
#include <sys/prctl.h>

const int TotalFlow::FEATURE_LENGTH = 128;

const float TotalFlow:: same_face_thresh = 0.65;
const std::string TotalFlow::UnknowFace = std::string("UnknowFace");
const std::string TotalFlow::NoFace = std::string("NoFace");

TotalFlow::TotalFlow(const std::string& path) :
        path_root_(path),
        config_path_(path_root_ + "/configure.yaml"),
        mtcnn_path_(path_root_ + "/mtcnn"),
#if defined(USE_TVM)
        sp_model_path_(path_root_ + "/landmark/x86_64-collections/landmark_deploy"),
        object_detect_(std::make_shared<ObjectDetect>(path_root_ + "/object_detection/x86_64-collections")),
#else
        sp_model_path_(path_root_ + "/landmark"),
        object_detect_(std::make_shared<ObjectDetect>(path_root_)),
#endif
        head_pose_model_path_(path_root_ + "/3D_model/model.txt"),
        faceid_path_(path_root_ + "/faceid"),
        align_method_(std::make_shared<FivePtsAlign>(96, 96)),
        face_align_(std::make_shared<FaceAlign>(align_method_.get())),
        predictor_(std::make_shared<ShapePredictor>(sp_model_path_)),
        head_pose_estimator_(std::make_shared<HeadPoseEstimator>(head_pose_model_path_, cv::Size(640,480))),
        faceid_(std::make_shared<FaceID>(faceid_path_)),
        config_(config_path_),
        first_time_flage_(true),
        keep_running_flag_(true),
        calib_flag_(true),
        regist_over_flag_(false),
        mark_no_face_(false),
        face_match_flag_(true),
        face_match_count_(0),
        REGISTNUM(10),
        current_regist_num_(0){

    result_.SetParam(config_.alarm_interval_, config_.speed_threshold_);

    smoke_judger_.SetParam(config_.process_fps_ * 5,
                           config_.process_fps_ * config_.smoke_threshold_);
    call_judger_.SetParam(config_.process_fps_ * 5,
                          config_.process_fps_ * config_.call_threshold_);
    close_eye_judger_.SetParam(config_.process_fps_ * 5,
                               config_.process_fps_ * config_.fatigue_threshold_,
                               config_.process_fps_ * 2);
    open_mouth_judger_.SetParam(config_.process_fps_ * 5,
                                config_.process_fps_ * config_.yawn_threshold_);
    head_left_right_judger_.SetParam(config_.process_fps_ * 5,
                                     config_.process_fps_ * config_.distraction_threshold_);
    head_up_down_judger_.SetParam(config_.process_fps_ * 5,
                                  config_.process_fps_ * config_.fatigue_threshold_,
                                  config_.process_fps_ * 2);
}


/**
 * 检测frame中的人脸信息
 * @param image
 * @return
 */
bool TotalFlow::DetectFrame(const cv::Mat &image) {
    auto obj_result = object_detect_->Detect(image);
    if(!obj_result->face()){
        std::lock_guard<std::mutex> lock_guard(landmark_mutex_);
        landmarks_.clear();
        face_bbox_ = cv::Rect();
        cerr << "[Error]:detect frame failed to find face" << endl;
        if(mark_no_face_) no_face_time_ = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - no_face_time_).count() > 3000) {
            std::lock_guard<std::mutex>lk(result_mutex_);
            result_.SetAbnormal(2);
        }
        mark_no_face_ = false;
        face_match_flag_ = true;
        name_mutex_.lock();
        name_ = NoFace;
        name_mutex_.unlock();
        return false;
    }
    if(!mark_no_face_) mark_no_face_ = true;
    result_mutex_.lock();
    result_.SetAbnormal(0);
    result_mutex_.unlock();

    face_bbox_ = obj_result->face_bbox();

    std::vector<cv::Point2f> shape = predictor_->predict(image, face_bbox_);

    std::vector<cv::Point2f> six_points;
    six_points.emplace_back(shape[38]); //鼻尖
    six_points.emplace_back(shape[8]);  //下巴尖
    six_points.emplace_back(shape[44]); //左外眼角
    six_points.emplace_back(shape[56]); //右外眼角
    six_points.emplace_back(shape[60]); //左外嘴角
    six_points.emplace_back(shape[66]); //右外嘴角
    std::unique_lock<std::mutex>uniqueLock(landmark_mutex_);
    landmarks_.clear();
    for(auto& point:shape){
        landmarks_.emplace_back(point);
    }
    uniqueLock.unlock();

    cv::Vec3d angles = head_pose_estimator_->estimate_pose(shape);
    std::unique_lock<std::mutex> uniqueLockAngles(angle_mutex_);
    angles_[0] = static_cast<float>(angles[0] * 180 / M_PI);
    angles_[1] = static_cast<float>(angles[1] * 180 / M_PI);
    angles_[2] = static_cast<float>(angles[2] * 180 / M_PI);
    angles_[2] = angles_[2] > 0 ? angles_[2] - 180: angles_[2] + 180;
    uniqueLockAngles.unlock();

//    if(calib_flag_){
//        calib_flag_ = false;
//        float left, right, up, down;
//        left = config_.distraction_left_angle_ + angles_[1];
//        right = config_.distraction_right_angle_ + angles_[1];
//        up = config_.distraction_up_angle_ + angles_[0];
//        down = config_.distraction_down_angle_ + angles_[0];
//
//        std::cout << "left ： " << left << "  right: " << right <<std::endl;
//        std::cout << "up： " << up<< "  down: " << down<<std::endl;
//        head_pose_detector_.SetParam(left, right, up, down);
//    }
    call_state_ = obj_result->call();
    call_bbox_ = obj_result->call_bbox();
    smoke_state_ = obj_result->smoke();
    smoke_bbox_ = obj_result->smoke_bbox();

    std::vector<cv::Point2f> mouth_vec;
    for (size_t index = 60; index != 73; ++index) {
        mouth_vec.push_back(shape[index]);
    }
    auto mouth_bbox = cv::boundingRect(mouth_vec);

    if(mouth_bbox.area() > 10) {
        mouth_bbox.x = mouth_bbox.x - 0.5f * mouth_bbox.width;
        mouth_bbox.y = mouth_bbox.y - 0.5f * mouth_bbox.height;
        mouth_bbox.width = mouth_bbox.width * 2;
        mouth_bbox.height = mouth_bbox.height * 2;
        if(smoke_state_){
            auto i = smoke_bbox_ & mouth_bbox;
            float iou = static_cast<float>(i.area()) / (smoke_bbox_.area() + mouth_bbox.area() - i.area());
            if(iou == 0.f) smoke_state_ = false;
        }
    }

    landmark_mutex_.lock();
    eye_mouth_detector_.Feed(landmarks_);
    landmark_mutex_.unlock();

    name_mutex_.lock();
    if (name_ == TotalFlow::NoFace and angles_[1] > -30 and angles_[1] < 30) {
        face_match_flag_ = true;
        face_match_count_ = 0;
    } else if (name_ == TotalFlow::UnknowFace and face_match_count_++ < config_.process_fps_)
        face_match_flag_ = true;

    name_mutex_.unlock();
    if(face_match_flag_){
        face_match_flag_ = false;
        std::vector<cv::Point2f> cv_pts;
        cv::Mat aligned_img;
        cv_pts.emplace_back(shape[44]);
        cv_pts.emplace_back(shape[56]);
        cv_pts.emplace_back(shape[38]);
        cv_pts.emplace_back(shape[60]);
        cv_pts.emplace_back(shape[66]);
        align_method_->set_im(image);
        align_method_->set_pts(cv_pts);
        aligned_img = face_align_->DoAlign();
        cv::Mat feat;
        faceid_->GetFaceFeature(aligned_img,feat);
        std::string name = GetName(feat);
        name_mutex_.lock();
        name_ = name;
        name_mutex_.unlock();
    }
    return true;
}


void CropAndResize(cv::Mat& src) {
    if (src.cols != 1280) {
        return;
    }
    int target_half_length = 960 / 2;
    int half_width = src.cols / 2;
    int left_x = half_width - target_half_length;
    int right_x = half_width + target_half_length;
    cv::Rect bb(left_x,0,960,720);
    src = src(bb);
    cv::resize(src,src,cv::Size(640,480));
}





/**
 * 程序运行入口
 */
void TotalFlow::Run(cv::Mat& frame, Result& result) {
    auto start = std::chrono::steady_clock::now();
    frame_mutex_.lock();
    frame_ = frame.clone();
    CropAndResize(frame_);
    frame_mutex_.unlock();

    if(first_time_flage_) {
        first_time_flage_ = false;
        LoadFeature();
        if(Features_.empty()) {
            std::cerr << "Please register first!" << std::endl;
            std::abort();
        }
        name_ = NoFace;
        process_image_thread_ = thread(mem_fn(&TotalFlow::ProcessImageThread), this);
        process_image_thread_.detach();
        process_picture_thread_ = thread(mem_fn(&TotalFlow::ProcessPictureThread), this);
        process_picture_thread_.detach();
        //当前没有人脸识别和规定动作,直接运行校准操作
        no_face_time_ = std::chrono::steady_clock::now();
    }

    name_mutex_.lock();
    result_.SetFaceId(name_);
    name_mutex_.unlock();

    landmark_mutex_.lock();
    result_.SetLandmarks(landmarks_);
    landmark_mutex_.unlock();

    angle_mutex_.lock();
    result_.SetAngles(angles_);
    angle_mutex_.unlock();

    bbox_mutex_.lock();
    result_.SetFaceBbox(face_bbox_);
    bbox_mutex_.unlock();

    result_mutex_.lock();
    result = result_;
//    result_.ResetResult();
    result_mutex_.unlock();

    auto end = std::chrono::steady_clock::now();
    auto time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    auto control_fps_cost = 1000 / config_.camera_fps_;
//    std::cout << "camera sleep : " << control_fps_cost-time_cost << std::endl;
    if(control_fps_cost > time_cost)
        std::this_thread::sleep_for(std::chrono::milliseconds(control_fps_cost-time_cost));
}

void TotalFlow::ProcessPictureThread() {
    cv::Rect bboxd;
    cv::Rect bboxf;
    cv::Rect bboxs;
    cv::Rect bboxc;
    std::string showFaceid = "name: ";
    std::string distration = "dis: ";
    std::string fatigue = "fat: ";
    std::string showSmoke = "smoke: ";
    std::string showCall = "call: ";
    std::string showAbnorm = "abnm: ";
    std::string faceid;
    int unknown = 0;
    while (true) {
        if (!keep_running_flag_) {
            break;
        }
//        LOGE(" TotalFlow::ProcessPictureThread() %d", isSave);
        if (isSave) {
            std::unique_lock<std::mutex> uniqueLock(frame_mutex_);
            cv::Mat frame = frame_.clone();
            uniqueLock.unlock();
            if (!frame.empty()) {
                index++;
                if (index % 2 == 0) {
                    auto start1 = std::chrono::steady_clock::now();

                    struct timeval tv;
                    gettimeofday(&tv, NULL);
                    long milli = tv.tv_usec / 1000;

                    struct tm p;
                    time_t nSrc;
                    nSrc = time(NULL) + time_diff;

                    p = *localtime(&nSrc);
                    char str[80];
                    strftime(str, 1000, "%Y-%m-%d %H-%M-%S", &p);
                    queue<string> queue1;

                    int yawn, dis, fat,fat2, smoke, call, abnorm, unknown;
                    result_.GetDistraction(dis, bboxd);//左右
                    result_.GetFatigueFirst(fat, bboxf);//疲劳
                    result_.GetFatigueSecond(fat2, bboxf);//疲劳

                    result_.GetSmoke(smoke, bboxs);
                    result_.GetCall(call, bboxc);
                    result_.GetAbnormal(abnorm);
                    result_.GetYawn(yawn);//haqi



                    result_.GetFaceId(faceid);

                    if (faceid == "UnknowFace")
                        unknown = 2;
                    else
                        unknown = 0;


                    if (dis != 0) {
                        currentPath = pathDis;
                    } else if (call != 0) {
                        currentPath = pathCall;
                    } else if (fat != 0 || fat2 != 0) {
                        currentPath = pathFat;
                    } else if (yawn != 0) {
                        currentPath = pathYawn;
                    }else if (smoke != 0) {
                        currentPath = pathSmoke;
                    } else if (abnorm != 0) {
                        currentPath = pathAbnormal;
                    } else if (unknown != 0) {
                        currentPath = pathUnknow;
                    } else
                        currentPath = path;

//                    LOGE("warning --  fat %d,dis %d,call %d,", fat,dis,call);

//                    cv::putText(frame, distration + to_string(dis), cv::Point(10, 10),
//                                1, 1, cv::Scalar(122, 255, 50));
//                    cv::putText(frame, fatigue + to_string(fat), cv::Point(10, 25), 1,
//                                1, cv::Scalar(122, 255, 50));
//                    cv::putText(frame, showSmoke + to_string(smoke),
//                                cv::Point(10, 40), 1, 1, cv::Scalar(122, 255, 50));
//                    cv::putText(frame, showCall + to_string(call), cv::Point(10, 55),
//                                1, 1, cv::Scalar(122, 255, 50));
//                    cv::putText(frame, showAbnorm + to_string(abnorm),
//                                cv::Point(10, 70), 1, 1, cv::Scalar(122, 255, 50));
//                    cv::putText(frame, "id :" + faceid,
//                                cv::Point(10, 85), 1, 1, cv::Scalar(122, 255, 50));
//
//                    if (fat != 0) {
//                        cv::rectangle(frame, bboxf, cv::Scalar(255, 0, 0), 2);
//                    }
//                    if (dis != 0) {
//                        cv::rectangle(frame, bboxd, cv::Scalar(255, 0, 0), 2);
//                    }
//                    if (smoke != 0) {
//                        cv::rectangle(frame, bboxs, cv::Scalar(255, 0, 0), 2);
//                    }
//                    if (call != 0) {
//                        cv::rectangle(frame, bboxc, cv::Scalar(255, 0, 0), 2);
//                    }

                    string file = currentPath + str + "-" + to_string(milli) + ".png";
//                    auto end1 = std::chrono::steady_clock::now();
//                    auto coun1t = std::chrono::duration_cast<std::chrono::milliseconds>(
//                            end1 - start1).count();
//                    LOGE("calculate time ---- %ld", coun1t);

                    auto start = std::chrono::steady_clock::now();
//                    string file = path + "test" + to_string(start.time_since_epoch().count()) + ".png";
//                    LOGE("copy image ---- %s", file.data());



//                    LOGE("result value - %s", file.data());
                    if(!stopQueue){
                        if(pictures.size() > 50)
                            pictures.pop();
                        pictures.push(file);
                    }
                    cv::imwrite(file, frame);



                    auto end = std::chrono::steady_clock::now();
                    auto count = std::chrono::duration_cast<std::chrono::milliseconds>(
                            end - start).count();
//                    LOGE("copy image ---- %ld", count);
//                    LOGE("copy image ---- %d,%d", frame.cols,frame.rows);
                }
                if (index > 1000)
                    index = 0;
            }
        }
    }
    std::abort();
}

void TotalFlow::ProcessImageThread() {
    prctl(PR_SET_NAME, "DSM_Algorithm");
    while (true) {
        if (!keep_running_flag_) {
            break;
        }
        auto beg = std::chrono::steady_clock::now();
        RunProcess();
        auto time_cost = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now()-beg).count();
        LOGE("RunProcess cost %ld",time_cost);
        auto control_fps_cost = 1000 / config_.process_fps_;
//        std::cout << "process sleep : " << control_fps_cost-time_cost << std::endl;
        if(control_fps_cost > time_cost)
            std::this_thread::sleep_for(std::chrono::milliseconds(control_fps_cost-time_cost));
    }
    std::cout << "keep_running_flag_ : " << keep_running_flag_ << std::endl;
    std::abort();
}


/**
 * 根据当前运行阶段,执行不同的程序
 */
void TotalFlow::RunProcess() {
    std::unique_lock<std::mutex> uniqueLock(frame_mutex_);
    cv::Mat frame =frame_.clone();
    uniqueLock.unlock();
    if (frame.empty()) {
        return;
    }
    bool success_flag = DetectFrame(frame);

    if(success_flag) RunMainStep();
    else {
        cv::Rect bbox;
        lock_guard<mutex>lock_guard(result_mutex_);
        result_.SetSmoke(0, bbox);
        result_.SetCall(0,bbox);
        result_.SetFatigueFirst(0,bbox);
        result_.SetFatigueSecond(0,bbox);
        result_.SetDistraction(0,bbox);
        result_.SetYawn(0);
    }
}

/**
 * 程序主函数
 */
void TotalFlow::RunMainStep() {
    float pitch = angles_[0];
    float yaw = angles_[1];
    bool head_lr_state = head_pose_detector_.GetHeadLeftRightStatus(yaw);
    bool head_ud_state = head_pose_detector_.GetHeadUpDownStatus(pitch);
    bool eye_state = eye_mouth_detector_.GetEyeStatus();
    bool mouth_state = eye_mouth_detector_.GetMouthStatus();

    int smoke_value = smoke_judger_.Detect(smoke_state_);
    int call_value = call_judger_.Detect(call_state_);
    int distract_lr_value = head_left_right_judger_.Detect(head_lr_state);
    int distract_ud_value = head_up_down_judger_.Detect(head_ud_state);
    int yawn_value = open_mouth_judger_.Detect(mouth_state);
    int fatigue_value = close_eye_judger_.Detect(eye_state);

    fatigue_value = std::max(fatigue_value, distract_ud_value);
    int fatigue_first = fatigue_value == 1;
    int fatigue_second = fatigue_value == 2;

    cv::Rect bbox;
    lock_guard<std::mutex>lock_guard(result_mutex_);
    result_.SetSmoke(smoke_value, smoke_bbox_);
    result_.SetCall(call_value, call_bbox_);
    result_.SetDistraction(distract_lr_value, bbox);
    result_.SetFatigueFirst(fatigue_first,bbox);
    result_.SetFatigueSecond(fatigue_second,bbox);
    result_.SetYawn(yawn_value);
}


bool TotalFlow::Regist(const std::string& feature_path) {

    cv::Mat feature;

    frame_mutex_.lock();
    cv::Mat frame = frame_.clone();
    frame_mutex_.unlock();

    auto obj_result = object_detect_->Detect(frame);
    if(!obj_result->face()){
        std::cout << "no face detect in current image!" << std::endl;
        return false;
    }

    face_bbox_ = obj_result->face_bbox();
    cv::Rect bbox = face_bbox_;
    std::vector<cv::Point2f> shape = predictor_->predict(frame, bbox);
    std::vector<cv::Point2f> cv_pts;
    cv::Mat aligned_img;
    cv_pts.emplace_back(shape[44]);
    cv_pts.emplace_back(shape[56]);
    cv_pts.emplace_back(shape[38]);
    cv_pts.emplace_back(shape[60]);
    cv_pts.emplace_back(shape[66]);
    align_method_->set_im(frame);
    align_method_->set_pts(cv_pts);
    aligned_img = face_align_->DoAlign();

    faceid_->GetFaceFeature(aligned_img, feature);

    if(!WriteFeature(feature, feature_path)) {
        std::cout << "[ERROR] : Write feature failed." << std::endl;
        return false;
    }

    return true;

}

bool TotalFlow::LoadFeature() {
    std::vector<std::string> people_name;
    std::string feature_path = path_root_ + "/feature/";

    people_name = Listdir(feature_path);

    for(const auto& name:people_name)
    {
        Feature peple_feature;
        peple_feature.name = name;
        name_times_[name] = 0;

        std::vector<std::string> feature_paths = Listfile(feature_path + name);
        for(const auto& fea: feature_paths)
        {
            std::string fea_name = feature_path + name + "/" + fea;
            cv::Mat feature;
            if(!ReadFeature(feature, fea_name)){
                std::cout << "[ERROR]: read feature failed!" << fea_name << std::endl;
                continue;
            }
            peple_feature.features.push_back(feature);
        }
        Features_.push_back(peple_feature);
    }
    name_times_[UnknowFace] = 0;
    return true;
}

std::string TotalFlow::GetName(const cv::Mat &feature) {
    name_times_.clear();
    std::string name = UnknowFace;

    if(feature.empty()) return name;

    for (const auto &Fea: Features_) {
        for (const auto &fea: Fea.features) {
            float score = faceid_->CalcCosScore(fea, feature);
            if (score > same_face_thresh) {
                name = Fea.name;
                name_times_[Fea.name]++;
            }
        }
    }

    int max_times = 0;
    for(auto& item: name_times_) {
        if(item.second > max_times){
            name = item.first;
            max_times = item.second;
        }
    }
    return name;
}

std::vector<std::string> TotalFlow::Listdir(const std::string &folder) {
    std::vector<std::string> filenames;
    DIR *dir;
    struct dirent *ptr;
    if((dir = opendir(folder.c_str())) == nullptr){
        std::cout << "Open dir(" << folder << ") error..." << std::endl;
        return filenames;
    }

    while ((ptr = readdir(dir)) != nullptr)
    {
        if(strcmp(ptr->d_name, ".") == 0 |
           strcmp(ptr->d_name, "..") == 0)
            continue;
        if(4 == ptr->d_type) {
            std::string dir_name = ptr->d_name;
            filenames.push_back(ptr->d_name);
        }
    }
    closedir(dir);

    return filenames;
}

std::vector<std::string> TotalFlow::Listfile(const std::string &folder) {
    std::vector<std::string> filenames;
    DIR *dir;
    struct dirent *ptr;
    if((dir = opendir(folder.c_str())) == nullptr){
        std::cout << "Open dir(" << folder << ") error..." << std::endl;
        return filenames;
    }

    while ((ptr = readdir(dir)) != nullptr)
    {
        if(strcmp(ptr->d_name, ".") == 0 |
           strcmp(ptr->d_name, "..") == 0)
            continue;
        if(8 == ptr->d_type) {
            std::string dir_name = ptr->d_name;
            filenames.push_back(ptr->d_name);
        }
    }
    closedir(dir);

    return filenames;
}

bool TotalFlow::FaceIDRun(const std::string& registName) {
    if(feature_name_path_.empty()) {
        feature_name_path_ = path_root_ + "/feature/" + registName;

        try {
            mkdir(feature_name_path_.c_str(), S_IRWXU | S_IRWXO);
        }
        catch (...) {
            std::cout << "[ERROR]: mkdir failed! >>" << feature_name_path_ << std::endl;
            exit(0);
        }

    }
    std::string feature_path = feature_name_path_ + "/" + to_string(current_regist_num_++) + ".bin";
//    if(!Regist(feature_path)) return false;
//    if(++current_regist_num_>REGISTNUM) regist_over_flag_ = true;
//    return true;
    return Regist(feature_path);
}

bool TotalFlow::WriteFeature(const cv::Mat& feature, const std::string& path) {
    fstream fs;
    try {
        fs.open(path, ios::out|ios::binary|ios::trunc);
        fs.write(reinterpret_cast<char*>(feature.data), TotalFlow::FEATURE_LENGTH * sizeof(float));
    }
    catch (...) {
        std::cout << "[ERROR]: Write feature failed! " << path << std::endl;
        fs.close();
        return false;
    }

    fs.close();
    return true;
}

bool TotalFlow::ReadFeature(cv::Mat &feaure, const std::string &path) {
    cv::Mat fea(1,TotalFlow::FEATURE_LENGTH, CV_32FC1);
    fstream fs;
    try {
        fs.open(path, ios::in|ios::binary);
        fs.read(reinterpret_cast<char *>(fea.data), TotalFlow::FEATURE_LENGTH * sizeof(float));
    }
    catch (...) {
        std::cout << "[ERROR]: Read feature failed! " << path << std::endl;
        fs.close();
        return false;
    }

    fs.close();
    feaure = fea.clone();
    return true;
}

TotalFlow::~TotalFlow() {
    keep_running_flag_ = false;
}

bool TotalFlow::Calibration(cv::Mat &frame) {
    auto obj_result = object_detect_->Detect(frame);
    if(!obj_result->face()) return false;

    std::vector<cv::Point2f> shape = predictor_->predict(frame, obj_result->face_bbox());
    cv::Vec3f angles = head_pose_estimator_->estimate_pose(shape);
    angles[0] = static_cast<float>(angles[0] * 180 / M_PI);
    angles[1] = static_cast<float>(angles[1] * 180 / M_PI);
    angles[2] = static_cast<float>(angles[2] * 180 / M_PI);
    angles[2] = angles[2] > 0 ? 180 - angles[2] : angles[2];

    float left, right, up, down;
    left = config_.distraction_left_angle_ + angles[1];
    right = config_.distraction_right_angle_ + angles[1];
    up = config_.distraction_up_angle_ + angles[0];
    down = config_.distraction_down_angle_ + angles[0];

//    std::cout << "left ： " << left << "  right: " << right <<std::endl;
//    std::cout << "up： " << up<< "  down: " << down<<std::endl;
    head_pose_detector_.SetParam(left, right, up, down);
    return true;
}

bool TotalFlow::RegistFeature(cv::Mat &frame, const string &name) {
    frame_mutex_.lock();
    frame_ = frame.clone();
    CropAndResize(frame_);
    frame_mutex_.unlock();
    return FaceIDRun(name);
}

