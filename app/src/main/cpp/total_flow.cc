//
// Created by zhangbo on 18-5-8.
//

#include <cmath>
#include <faceID.h>
#include <faceAttribute.h>
#include "total_flow.h"
#include <time.h>

#if defined(USE_NCNN)
#include <objDetectionNCNN.h>
#else

#include <objDetection.h>

#endif

const int TotalFlow::FEATURE_LENGTH = 128;

const float TotalFlow::same_face_thresh = 0.55;
const std::string TotalFlow::UnknowFace = std::string("UnknowFace");
const std::string TotalFlow::NoFace = std::string("NoFace");

TotalFlow::TotalFlow(const std::string &path) :
        path_root_(path),
        config_path_(path_root_ + "/configure.yaml"),
        mtcnn_path_(path_root_ + "/mtcnn"),
        sp_model_path_(path_root_ + "/align/0102-1635-4000-800.dat"),
        head_pose_model_path_(path_root_ + "/3D_model/model.txt"),
        faceid_path_(path_root_ + "/faceid"),
        gaze_tracking_path_(path_root_ + "/gaze_tracking"),
        detector_(std::make_shared<MTCNNDetector>(mtcnn_path_)),
        align_method_(std::make_shared<FivePtsAlign>(96, 96)),
        face_align_(std::make_shared<FaceAlign>(align_method_.get())),
        predictor_(std::make_shared<ShapePredictor>(sp_model_path_)),
        head_pose_estimator_(
                std::make_shared<HeadPoseEstimator>(head_pose_model_path_, cv::Size(640, 480))),
        faceid_(std::make_shared<FaceID>(faceid_path_)),
        faceAttribute_(std::make_shared<FaceAttribute>(faceid_path_)),
        arguments_(1, gaze_tracking_path_),
        config_(config_path_),
        first_time_flage_(true),
        keep_running_flag_(true),
        regist_over_flag_(false),
        mark_no_face_(false),
        face_match_flag_(true),
        current_regist_num_(0) {

    main_step_ = RunStep::CalibrationStep;
    alignment_time_ = config_.alignment_time_;
//    cur_driver_state_ = DetectionState::Normal;  //当前状态为正常状态

    // 初始化疲劳检测策略,吸烟检测策略,打电话检测策略,手掌检测策略,可疑行为检测策略,聊天检测策略
    // 分神检测的策略需要等到校准完成后再初始化
#if defined(ZUOLEI)
    fatigue_judger_.SetParam(config_.fatigue_warn_thres_ / 100.0, config_.fatigue_judge_thres_ / 100.0,
                             config_.fatigue_warn_time_, config_.fatigue_judge_time_);
#else
    fatigue_judger_.SetParam(8, 12, 8, 12);
#endif

    smoke_judger_.SetParam(config_.smoke_judge_thres_ / 100.0, config_.smoke_judge_thres_ / 100.0,
                           config_.smoke_judge_time_, config_.smoke_judge_time_);
//    smoke_judger_.SetParam(2);
    call_judger_.SetParam(config_.call_warn_thres_ / 100.0, config_.call_judge_thres_ / 100.0,
                          config_.call_warn_time_, config_.call_judge_time_);

    // 初始化校准检测
    alignment_judger_.SetParam(config_.alignment_time_);
#if defined(USE_NCNN)
    net_.load_param("yolo.param");
    net_.load_model("yolo.bin");
#else
    net_ = createNet(path_root_ + "/object_detection/oldMobileNetSSD_deploy.prototxt",
                     path_root_ + "/object_detection/oldshuffle_iter_300000.caffemodel");
#endif
}


/**
 * 检测frame中的人脸信息
 * @param image
 * @return
 */
bool TotalFlow::DetectFrame(const cv::Mat &image) {
//    std::vector<Face> faces = detector_->detect(image);
    std::vector<ObjInfo> faces;
    std::vector<ObjInfo> obj_state = objDetection(net_, image, 0.4);
//    chrono::steady_clock::time_point old = chrono::steady_clock::now();
//    auto time =  chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - old);
//    LOGE("objDetection(net_, image, 0.4) is %ld", time.count());

    for (auto &obj : obj_state) {
        if (obj.action_ == Action::FACE)
            faces.push_back(obj);
    }

    if (faces.empty()) {
        std::lock_guard<std::mutex> lock_guard(landmark_mutex_);
        landmarks_.clear();
        face_bbox_ = cv::Rect();
        cerr << "[Error]:detect frame failed to find face" << endl;
        if (mark_no_face_) no_face_time_ = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::milliseconds>(
                chrono::steady_clock::now() - no_face_time_).count() > 3000) {
            std::lock_guard<std::mutex> lk(result_mutex_);
            result_.SetAbnormal(2);
        }
        mark_no_face_ = false;
        face_match_flag_ = true;
        name_mutex_.lock();
        name_ = NoFace;
        name_mutex_.unlock();
        return false;
    }
    if (!mark_no_face_) mark_no_face_ = true;
    result_mutex_.lock();
    result_.SetAbnormal(0);
    result_mutex_.unlock();

    auto Psort = [&](const ObjInfo &a, const ObjInfo &b) {
        return a.score_ < b.score_;
    };
    std::partial_sort(faces.begin(), faces.begin() + 1, faces.end(), Psort);
    auto face = faces[0];
    face_bbox_ = cv::Rect(cv::Point(face.xL_, face.yL_), cv::Point(face.xR_, face.yR_));
//    Face face= detector_->get_largest_face(faces);
//    face_bbox_ = face.bbox.getRect();

    std::vector<cv::Point2f> shape = predictor_->predict(image, face_bbox_);
//    chrono::steady_clock::time_point old2 = chrono::steady_clock::now();
//    auto time2 =  chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - old2);
//    LOGE("predictor_->predict(image, face_bbox_) is %ld", time2.count());

    std::vector<cv::Point2f> six_points;
    six_points.emplace_back(shape[38]); //鼻尖
    six_points.emplace_back(shape[8]);  //下巴尖
    six_points.emplace_back(shape[44]); //左外眼角
    six_points.emplace_back(shape[56]); //右外眼角
    six_points.emplace_back(shape[60]); //左外嘴角
    six_points.emplace_back(shape[66]); //右外嘴角
    std::unique_lock<std::mutex> uniqueLock(landmark_mutex_);
    landmarks_.clear();
    for (auto &point:shape) {
        landmarks_.emplace_back(point);
    }
    uniqueLock.unlock();

    cv::Vec3d angles = head_pose_estimator_->estimate_pose(shape);
    std::unique_lock<std::mutex> uniqueLockAngles(angle_mutex_);
    angles_[0] = static_cast<float>(angles[0] * 180 / M_PI);
    angles_[1] = static_cast<float>(angles[1] * 180 / M_PI);
    angles_[2] = static_cast<float>(angles[2] * 180 / M_PI);
//    angles_[0] = angles_[0] > 0 ? angles_[0] - 180 : angles_[0] + 180;
    angles_[2] = angles_[2] > 0 ? angles_[2] - 180 : angles_[2] + 180;
    uniqueLockAngles.unlock();

//    cv::Mat img = image.clone();
//    std::vector<ObjInfo>obj_state = objDetection(net_, image, 0.4);
    for (auto obj: obj_state) {
        if (obj.action_ == Action::CALL) {
            call_bbox_ = cv::Rect(cv::Point(obj.xL_, obj.yL_), cv::Point(obj.xR_, obj.yR_));
        } else if (obj.action_ == Action::SMOKE) {
            smoke_bbox_ = cv::Rect(cv::Point(obj.xL_, obj.yL_), cv::Point(obj.xR_, obj.yR_));
        }
    }
    call_state_ = getCallState();
    smoke_state_ = getSmokeState();
//    std::cout << "call state : " << call_state_ << std::endl;
//    std::cout << "smoke state: " << smoke_state_ << std::endl;

    landmark_mutex_.lock();
    eye_mouth_detector_.Feed(landmarks_);
    landmark_mutex_.unlock();

    name_mutex_.lock();
    if (name_ == TotalFlow::UnknowFace) face_match_flag_ = true;
    name_mutex_.unlock();
    if (face_match_flag_) {
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

        faceid_->GetFaceFeature(aligned_img, feat);
//        chrono::steady_clock::time_point old3 = chrono::steady_clock::now();
//        auto time3 =  chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - old3);
//        LOGE("GetFaceFeature(aligned_img,feat) is %ld", time3.count());

        std::string name = GetName(feat);
        name_mutex_.lock();
        name_ = name;
        name_mutex_.unlock();
    }
    return true;
}


void CropAndResize(cv::Mat &src) {
    if (src.cols != 1280) {
        return;
    }
    int target_half_length = 960 / 2;
    int half_width = src.cols / 2;
    int left_x = half_width - target_half_length;
    int right_x = half_width + target_half_length;
    cv::Rect bb(left_x, 0, 960, 720);
    src = src(bb);
    cv::resize(src, src, cv::Size(640, 480));
}


void TotalFlow::setPicture(bool save) {
    isSave = save;
}

/**
 * 程序运行入口
 */
void TotalFlow::Run(cv::Mat &frame, Result &result, bool regist, const std::string &registName) {
    frame_mutex_.lock();
    frame_ = frame.clone();
    frame_mutex_.unlock();
    if (regist and not regist_over_flag_) {
        FaceIDRun(registName);
        if (not regist_over_flag_) {
            result.SetCalibration(-1);
            return;
        }
    }
    if (first_time_flage_) {
        first_time_flage_ = false;
        LoadFeature();
        if (Features_.empty()) {
            std::cerr << "Please register first!" << std::endl;
//            std::abort();
        }
        name_ = NoFace;
        process_image_thread_ = thread(mem_fn(&TotalFlow::ProcessImageThread), this);
        process_image_thread_.detach();
        process_picture_thread_ = thread(mem_fn(&TotalFlow::ProcessPictureThread), this);
        process_picture_thread_.detach();
        //当前没有人脸识别和规定动作,直接运行校准操作
        main_step_ = RunStep::CalibrationStep;
        no_face_time_ = std::chrono::steady_clock::now();
    }

    frame_mutex_.lock();
    CropAndResize(frame_);
    frame_mutex_.unlock();

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
    result_mutex_.unlock();

}

void TotalFlow::ProcessPictureThread() {
    while (true) {
        if (!keep_running_flag_) {
            break;
        }
//        auto start1 = std::chrono::steady_clock::now();

        LOGE(" TotalFlow::ProcessPictureThread() %d", isSave);
//        auto end1 = std::chrono::steady_clock::now();
//        auto coun1t = std::chrono::duration_cast<std::chrono::milliseconds>(
//                end1 - start1).count();
//        LOGE("calculate time ---- %ld", coun1t);
        if (isSave) {
//            LOGE(" TotalFlow::ProcessPictureThread() %d", isSave);
            std::unique_lock<std::mutex> uniqueLock(frame_mutex_);
            cv::Mat frame = frame_.clone();
            uniqueLock.unlock();
            if (!frame.empty()) {
                index++;
                if (index % 3 == 0) {
//                    auto start1 = std::chrono::steady_clock::now();
                    struct tm p;
                    time_t nSrc;
                    nSrc = time(NULL);
                    p = *localtime(&nSrc);
                    char str[80];
                    strftime(str, 1000, "%Y-%m-%d %H-%M-%S", &p);
                    string file = path + "test" + str + "-" + to_string(index) + ".png";
//                    auto end1 = std::chrono::steady_clock::now();
//                    auto coun1t = std::chrono::duration_cast<std::chrono::milliseconds>(
//                            end1 - start1).count();
//                    LOGE("calculate time ---- %ld", coun1t);

                    auto start = std::chrono::steady_clock::now();
//                    string file = path + "test" + to_string(start.time_since_epoch().count()) + ".png";
//                    LOGE("copy image ---- %s", file.data());
                    cv::imwrite(file, frame);
                    auto end = std::chrono::steady_clock::now();
                    auto count = std::chrono::duration_cast<std::chrono::milliseconds>(
                            end - start).count();
                    LOGE("copy image ---- %ld", count);
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
    while (true) {
        if (!keep_running_flag_) {
            break;
        }

//        chrono::steady_clock::time_point old = std::chrono::steady_clock::now();
        RunProcess();
//        auto diff = chrono::duration_cast<std::chrono::milliseconds>(
//                std::chrono::steady_clock::now() - old);
//        LOGE("  RunProcess(); is -- %ld", diff.count());
    }
    std::cout << "keep_running_flag_ : " << keep_running_flag_ << std::endl;
    std::abort();
}


/**
 * 根据当前运行阶段,执行不同的程序
 */
void TotalFlow::RunProcess() {
    std::unique_lock<std::mutex> uniqueLock(frame_mutex_);
    cv::Mat frame = frame_.clone();
    uniqueLock.unlock();
//    LOGE("copy RunProcess ---- %d,%d", frame.cols,frame.rows);
    if (frame.empty()) {
        return;
    }
    bool success_flag = DetectFrame(frame);
//    chrono::steady_clock::time_point old = std::chrono::steady_clock::now();
//    auto diff = chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - old);
//    LOGE(" detectFrame is -- %ld", diff.count());

    if (success_flag == false) {
        if (main_step_ == RunStep::CalibrationStep) {
            alignment_judger_.Pause();
            result_mutex_.lock();
            result_.SetCalibration(1);
            result_mutex_.unlock();
        }
//        auto diff = chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - no_face_time_);
//        double diff_time = diff.count() / 1000.0f;

        if (main_step_ == RunStep::MainRunStep) {
            distract_judger_.ResetDetect();
//            fatigue_judger_.ResetDetect();
//            smoke_judger_.ResetDetect();
            call_judger_.ResetDetect();
        }
        return;
    }
    no_face_time_ = std::chrono::steady_clock::now();

    result_mutex_.lock();
    result_.SetCalibration(0);
    result_mutex_.unlock();
    switch (main_step_) {
        case RunStep::RequiredRoutineStep:
//            RunRequiredRoutine();
            break;
        case RunStep::CalibrationStep:
            RunCalibration();
            break;
        case RunStep::MainRunStep:
            sub_step_ = SmokeDetectionStep;
//            sub_step_ = FatigueDetectionStep;
            RunMainStep();
            break;
    }
}

/**
 * 程序主函数
 */
void TotalFlow::RunMainStep() {
    if (main_step_ != RunStep::MainRunStep) {
        cerr << "Error: main_step is not MainRunStep" << endl;
        return;
    }
    while (main_step_ == RunStep::MainRunStep) {
        switch (sub_step_) {
            case RunStep::DistractDetectionStep:    // 分神检测
                RunDistractDetection();
                break;
            case RunStep::FatigueDetectionStep: //疲劳检测
                RunFatigueDectection();
                break;
//            case RunStep::YawnDetectionStep:    //打哈欠（疲劳）检测
//                RunYawnDetection();
//                break;
            case RunStep::SmokeDetectionStep:   // 吸烟检测
                RunSmokeDetection();
                break;
            case RunStep::CallDetectionStep:    //打电话检测
                RunCallDetection();
                break;
            case RunStep::EndDetectionStep:     // 完成检测
            default:
                return;
        }
    }
}


/**
 * 校准阶段
 */
void TotalFlow::RunCalibration() {
    if (main_step_ != RunStep::CalibrationStep) {
        cerr << "Error: main_step is not CalibrationStep" << endl;
        return;
    }

    float angles_0, angles_1;
    std::unique_lock<std::mutex> uniqueLock(angle_mutex_);
    angles_0 = angles_[0];
    angles_1 = angles_[1];
    uniqueLock.unlock();

    cv::Vec3d calibrate_res = alignment_judger_.Alignment(angles_0, angles_1);

    // 完成校准
    if (alignment_time_ - calibrate_res[2] <= 0) {
        // 初始化分神检测策略
        cout << "Info: set the param of DISTRACT judgement " << endl;
        distract_judger_.SetParam(calibrate_res[0], calibrate_res[1],
                                  config_.distraction_warn_thres_ / 100.0f,
                                  config_.distraction_judge_thres_ / 100.0f,
                                  config_.distraction_left_angle_, config_.distraction_right_angle_,
                                  config_.distraction_up_angle_, config_.distraction_down_angle_,
                                  config_.distraction_warn_time_, config_.distraction_judge_time_);
        main_step_ = RunStep::MainRunStep;
        sub_step_ = RunStep::SmokeDetectionStep;
//        eye_mouth_detector_.Calibrate();
        result_mutex_.lock();
        result_.SetCalibration(2);
        result_mutex_.unlock();

    }


}


/**
 * 分神检测策略
 */
void TotalFlow::RunDistractDetection() {
    //TODO: 修改策略的返回类型,这样就不需要Switch操作了
    float angles_0, angles_1;
    std::unique_lock<std::mutex> uniqueLock(angle_mutex_);
    angles_0 = angles_[0];
    angles_1 = angles_[1];
    uniqueLock.unlock();

    int distract_value = distract_judger_.CheckDistraction(angles_0, angles_1);
    cv::Rect bbox;
    result_mutex_.lock();
    result_.SetDistraction(distract_value, bbox);
    result_mutex_.unlock();

    sub_step_ = RunStep::EndDetectionStep;

//    // 当前不是分神状态
//    if (distract_value == 0) {
//        // 如果当前为分神预警或者分神判决状态,则更新当前状态为正常状态
//        if (cur_driver_state_ == DetectionState::DistractionWarn ||
//            cur_driver_state_ == DetectionState::DistractionJudge) {
//            cur_driver_state_ = DetectionState::Normal;     // 修改当前状态
//                    result_mutex_.lock();
//                    result_.SetDistraction(distract_value, )
//        }
//        sub_step_ = RunStep::EndDetectionStep;
//    } else if (distract_value == 1 && cur_driver_state_ != DetectionState::DistractionWarn) {
//        sub_step_ = RunStep::EndDetectionStep;
//        cur_driver_state_ = DetectionState::DistractionWarn;
//
//        fatigue_judger_.ResetDetect();
//        smoke_judger_.ResetDetect();
//        call_judger_.ResetDetect();
//    } else if (distract_value == 2 && cur_driver_state_ != DetectionState::DistractionJudge) {
//        // 结束当前检测
//        sub_step_ = RunStep::EndDetectionStep;
//        cur_driver_state_ = DetectionState::DistractionJudge;
//    } else {
//        // 当前为状态为分神,下一阶段为人脸检测
//        sub_step_ = RunStep::EndDetectionStep;
//    }
}


/**
 * 疲劳检测
 */
void TotalFlow::RunFatigueDectection() {
#if defined(ZUOLEI)
    int righteye_state_ = eye_mouth_detector_.GetRightEyeStatus();
    int lefteye_state_ = eye_mouth_detector_.GetLeftEyeStatus();
    int eye_value = fatigue_judger_.CheckFatigue(righteye_state_, lefteye_state_);
#else
    bool eye_state = eye_mouth_detector_.GetEyeStatus();
    bool mouth_state = eye_mouth_detector_.GetMouthStatus();
    int fatigue_value = fatigue_judger_.CheckFatigue(eye_state, mouth_state);
#endif

//    int yawn_value = yawn_judger_.CheckYawn(yawn_state_);

//    int fatigue_value = MAX(eye_value, yawn_value);
//    std::cout << "eye_value : " << eye_value << " "\
//              << "yawn_value: " << yawn_value << " "\
//              << "fatigue_value : " << fatigue_value << std::endl;
    cv::Rect bbox;
    result_mutex_.lock();
    result_.SetFatigue(fatigue_value, bbox);
//    result_.SetFatigue(eye_value, bbox);
    result_mutex_.unlock();
    sub_step_ = RunStep::DistractDetectionStep;
//    // 当前不是疲劳状态
//    if (fatigue_value == 0) {
//        // 如果当前状态为疲劳预警或疲劳判决状态,则更新当前状态为正常状态
//        if (cur_driver_state_ == DetectionState::FatigueWarn ||
//            cur_driver_state_ == DetectionState::FatigueJudge) {
//            cur_driver_state_ = DetectionState::Normal;
//        }
//        sub_step_ = RunStep::YawnDetectionStep;
//    } else if (fatigue_value == 1 && cur_driver_state_ != DetectionState::FatigueWarn) { // 进入疲劳预警阶段
//        // 结束当前检测
//        sub_step_ = RunStep::EndDetectionStep;
//        cur_driver_state_ = DetectionState::FatigueWarn;
//
//        smoke_judger_.ResetDetect();
//        call_judger_.ResetDetect();
//
//    } else if (fatigue_value == 2 &&
//               cur_driver_state_ != DetectionState::FatigueJudge) {    // 进入疲劳判决状态
//        // 结束当前检测
//        sub_step_ = RunStep::EndDetectionStep;
//        cur_driver_state_ = DetectionState::FatigueJudge;
//    } else {
//        // 结束当前检测
//        sub_step_ = RunStep::EndDetectionStep;
//    }

}


/**
 * 吸烟检测
 */
void TotalFlow::RunSmokeDetection() {
    int smoke_value = smoke_judger_.CheckSmoking(smoke_state_);
//    std::cout << "smoke_state : " << smoke_state_ << "  " \
//              << "smoke_value : " << smoke_value << std::endl;

    result_mutex_.lock();
    result_.SetSmoke(smoke_value, smoke_bbox_);
    result_mutex_.unlock();
    sub_step_ = RunStep::CallDetectionStep;

}

/**
 * 打电话检测
 */
void TotalFlow::RunCallDetection() {
    int call_value = call_judger_.CheckCalling(call_state_);
//    std::cout << "call_state : " << call_state_ << "  " \
//              << "call_value : " << call_value << std::endl;
    result_mutex_.lock();
    result_.SetCall(call_value, call_bbox_);
    result_mutex_.unlock();
    sub_step_ = RunStep::FatigueDetectionStep;
}


bool TotalFlow::Regist(const std::string &feature_path) {

    cv::Mat feature;

    frame_mutex_.lock();
    cv::Mat frame = frame_.clone();
    frame_mutex_.unlock();

    std::vector<ObjInfo> faces;
    std::vector<ObjInfo> obj_state = objDetection(net_, frame, 0.4);
    for (auto &obj : obj_state) {
        if (obj.action_ == Action::FACE)
            faces.push_back(obj);
    }

    if (faces.empty()) {
        std::cout << "no face detect in current image!" << std::endl;
        return false;
    }

    auto Psort = [&](const ObjInfo &a, const ObjInfo &b) {
        return a.score_ < b.score_;
    };
    std::partial_sort(faces.begin(), faces.begin() + 1, faces.end(), Psort);
    auto face = faces[0];
    face_bbox_ = cv::Rect(cv::Point(face.xL_, face.yL_), cv::Point(face.xR_, face.yR_));

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

    if (!WriteFeature(feature, feature_path)) {
        std::cout << "[ERROR] : Write feature failed." << std::endl;
        return false;
    }

    return true;

}

bool TotalFlow::LoadFeature() {
    std::vector<std::string> people_name;
    std::string feature_path = path_root_ + "/feature/";

    people_name = Listdir(feature_path);

    for (const auto &name:people_name) {
        Feature peple_feature;
        peple_feature.name = name;
        name_times_[name] = 0;

        std::vector<std::string> feature_paths = Listfile(feature_path + name);
        for (const auto &fea: feature_paths) {
            std::string fea_name = feature_path + name + "/" + fea;
            cv::Mat feature;
            if (!ReadFeature(feature, fea_name)) {
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

    if (feature.empty()) return name;

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
    for (auto &item: name_times_) {
        if (item.second > max_times) {
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
    if ((dir = opendir(folder.c_str())) == nullptr) {
        std::cout << "Open dir(" << folder << ") error..." << std::endl;
        return filenames;
    }

    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 |
            strcmp(ptr->d_name, "..") == 0)
            continue;
        if (4 == ptr->d_type) {
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
    if ((dir = opendir(folder.c_str())) == nullptr) {
        std::cout << "Open dir(" << folder << ") error..." << std::endl;
        return filenames;
    }

    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 |
            strcmp(ptr->d_name, "..") == 0)
            continue;
        if (8 == ptr->d_type) {
            std::string dir_name = ptr->d_name;
            filenames.push_back(ptr->d_name);
        }
    }
    closedir(dir);

    return filenames;
}
//bool TotalFlow::FaceIDRun() {
//    while(true) {
//        char input = ' ';
//        std::string registName;
//        std::cout << "Do you want to regist picture? (y or n)" << std::endl;
//        input = getchar();
//        if (input == 'n') {
//            LoadFeature();
//            return true;
//        } else if (input == 'y') {
//            std::cout << "Plese input your name:" << std::endl;
//            std::cin >> registName;
//            getchar();
//            Regist(registName, 100);
//            std::cout << "Register over!" <<std::endl;
//        } else
//            continue;
//
//    }
//}

bool TotalFlow::FaceIDRun(const std::string &registName) {
    if (feature_name_path_.empty()) {
        feature_name_path_ = path_root_ + "/feature/" + registName;

        try {
            mkdir(feature_name_path_.c_str(), S_IRWXU | S_IRWXO);
        }
        catch (...) {
            std::cout << "[ERROR]: mkdir failed! >>" << feature_name_path_ << std::endl;
            exit(0);
        }

    }
    std::string feature_path = feature_name_path_ + "/" + to_string(current_regist_num_) + ".bin";
    if (!Regist(feature_path)) return false;
    if (++current_regist_num_ > REGISTNUM) regist_over_flag_ = true;

    return true;
}

bool TotalFlow::WriteFeature(const cv::Mat &feature, const std::string &path) {
    fstream fs;
    try {
        fs.open(path, ios::out | ios::binary | ios::trunc);
        fs.write(reinterpret_cast<char *>(feature.data), TotalFlow::FEATURE_LENGTH * sizeof(float));
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
    cv::Mat fea(1, TotalFlow::FEATURE_LENGTH, CV_32FC1);
    fstream fs;
    try {
        fs.open(path, ios::in | ios::binary);
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


