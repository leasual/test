//
// Created by untouch.
//
/*! \file TotalFlow.h
 *  \brief 定义程序的主流程
 *
 *  该文件为程序的入口,并且定义了程序的整体流程
 */

#ifndef STRATEGY_TOTALFLOW_H
#define STRATEGY_TOTALFLOW_H

#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <opencv2/opencv.hpp>

#include "shape_predict_untouch.h"
#include "detector.h"
#include "align_method.h"
#include "faceAlign.h"
#include "faceID.h"
#include "faceAttribute.h"
#include "align_method.h"
#include "faceAlign.h"
#include "head_pose_estimator.h"
#include "object_detection.h"

#include "config_tracker.h"
#include "judger.h"
#include "smoke_judger.h"

#include "head_pose_states.h"
#include "eye_mouth_plus.h"
#include "Util.h"
using namespace std;
using namespace cv;

//TODO : 将值改成枚举类型
class TotalFlow;
class Strategy{
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    Strategy():value_(0), bbox_(), interval_(0){};
    inline void SetSpeed(size_t speed){current_speed_ = speed;}
    inline void SetParam(size_t interval, size_t speed){interval_ = interval * 1000; speed_threshold_ = speed;}
    inline bool GetValue(int& val, cv::Rect& bbox) const {val = value_; bbox = bbox_; return true;}
    inline bool ResetValue(){value_ = 0; bbox_ = cv::Rect(); return true;}
    bool SetValue(bool val, const cv::Rect& bbox) {
        TimePoint now = std::chrono::steady_clock::now();
        auto interval = chrono::duration_cast<chrono::milliseconds>(now - last_alarm_time_).count();
        if (val and interval >= interval_) {
            if(current_speed_ > speed_threshold_) value_ = 2;
            else value_ = 1;

            bbox_ = bbox;
            last_alarm_time_ = now;
        }else{
            value_ = 0;
            bbox_ = cv::Rect();
        }
        return true;}
private:
    size_t interval_;
    size_t speed_threshold_;
    size_t current_speed_;
    TimePoint last_alarm_time_;
    int value_;
    cv::Rect bbox_;
};

class Result{
public:
    explicit Result(size_t interval): interval_(interval){}
    Result():interval_(0){};

    friend class TotalFlow;

    inline bool GetFaceBbox(cv::Rect& bbox) const { bbox = face_rect_; return true;}

    inline bool GetLandmarks(std::vector<cv::Point2f>& landmarks) const { landmarks = landmarks_; return true;}

    inline bool GetAngles(cv::Vec3f& angles) const { angles = angles_; return true;}

    inline bool GetFaceId(std::string& name) const { name = name_; return true;}

    inline bool GetCalibration(int& val) const { cv::Rect bbox; return calibration_.GetValue(val, bbox);}

    inline bool GetDistraction(int& val, cv::Rect& bbox ) const { return distraction_.GetValue(val, bbox);}

    inline bool GetFatigue(int& val, cv::Rect& bbox) const { return fatigue_.GetValue(val, bbox);}

    inline bool GetSmoke(int& val, cv::Rect& bbox) const { return smoke_.GetValue(val, bbox);}

    inline bool GetCall(int& val,cv::Rect& bbox) const { return call_.GetValue(val, bbox);}

    inline bool GetAbnormal(int& val) const { cv::Rect bbox; return abnormal_.GetValue(val, bbox);}

private:

    inline void SetFaceBbox(const cv::Rect& bbox) {face_rect_ = bbox;}

    inline void SetLandmarks(const std::vector<cv::Point2f>& landmarks) {landmarks_ = landmarks;}

    inline void SetAngles(const cv::Vec3f& angles) {angles_ = angles;}

    inline void SetFaceId(const std::string& val) {name_ = val;}

    inline void SetCalibration(int val) {calibration_.SetValue(val, cv::Rect());}

    inline void SetDistraction(int val, cv::Rect& bbox) {distraction_.SetValue(val, bbox);}

    inline void SetFatigue(int val, cv::Rect& bbox) {fatigue_.SetValue(val, bbox);}

    inline void SetSmoke(int val, cv::Rect& bbox) {smoke_.SetValue(val, bbox);}

    inline void SetCall(int val, cv::Rect& bbox) {call_.SetValue(val, bbox);}

    inline void SetAbnormal(int val) {abnormal_.SetValue(val, cv::Rect());}

    inline bool ResetResult(){
        calibration_.ResetValue();
        distraction_.ResetValue();
        fatigue_.ResetValue();
        smoke_.ResetValue();
        call_.ResetValue();
        abnormal_.ResetValue();
        return true;
    }
private:
     void SetParam(size_t interval, size_t speed){
        interval_ = interval;
        speed_threshold_ = speed;
        calibration_.SetParam(interval_, speed_threshold_);
        distraction_.SetParam(interval_, speed_threshold_);
        fatigue_.SetParam(interval_, speed_threshold_);
        smoke_.SetParam(interval_, speed_threshold_);
        call_.SetParam(interval_, speed_threshold_);
        abnormal_.SetParam(interval_, speed_threshold_);
    }

     void SetSpeed(size_t speed){
       current_speed_ = speed;
       distraction_.SetSpeed(current_speed_);
       fatigue_.SetSpeed(current_speed_);
       smoke_.SetSpeed(current_speed_);
       call_.SetSpeed(current_speed_);
       abnormal_.SetSpeed(current_speed_);
    }
private:
    size_t interval_;
    size_t speed_threshold_;
    size_t current_speed_;
    cv::Rect face_rect_;                    //人脸bounding box
    std::vector<cv::Point2f> landmarks_;    //人脸关键点
    cv::Vec3f angles_;                      //人脸角度信息 0：上下， 1：左右
    std::string name_;                      //faceID策略
    Strategy calibration_;                  //校准策略  0：正常校准， 1:暂停校准， 2:完成校准, -1 注册
    Strategy distraction_;                  //分神策略
    Strategy fatigue_;                      //疲劳策略
    Strategy smoke_;                        //抽烟策略
    Strategy call_;                         //打电话策略
    Strategy abnormal_;                     //驾驶员异常策略
};

class TotalFlow {
public:
    /// \brief 构造函数
    explicit TotalFlow(const std::string& path);
    ~TotalFlow();
    bool isSave = true;
    string path = "/storage/sdcard1/img/";
    string pathDis = "/storage/sdcard1/distract/";
    string pathFat = "/storage/sdcard1/fat/";
    string pathCall = "/storage/sdcard1/call/";
    string pathSmoke = "/storage/sdcard1/smoke/";
    string pathAbnormal = "/storage/sdcard1/abnormal/";
    string pathUnknow = "/storage/sdcard1/unknown/";
    string currentPath =  "/storage/sdcard1/img/";
    queue<string> pictures;
    bool stopQueue = false;
    int index = 0;
    long time_diff = 0;
    std::thread process_picture_thread_;

    /// \brief 程序运行入口
    void Run(cv::Mat& frame, Result& result);

    bool RegistFeature(cv::Mat& frame, const std::string& name = "untouch");

    bool Calibration(cv::Mat& frame);

    void SetSpeed(size_t speed){
        lock_guard<mutex>lock_guard(result_mutex_);
        result_.SetSpeed(speed);
    }
private:
    /// \brief 用于处理图像的线程
    void ProcessImageThread();
    void ProcessPictureThread();
    /// \brief 处理程序的函数
    void RunProcess();


    /// \brief 运行驾驶员检测策略的主函数
    /// \details 不同的检测策略之间的优先级从高到低依次为：分神检测，疲劳检测，吸烟检测，打电话检测，可疑行为检测，
    /// 聊天检测。检测策略按照优先级依次执行，只有当某个策略为正常状态时，才继续执行下一个检测策略。
    void RunMainStep();


    /// \brief 检测图像中的landmark,香烟,手掌,打电话,头的旋转角度，获取的数据用于检测驾驶员状态
    /// \param [in] frame 要检测的图像
    /// \retval 检测是否成功
    bool DetectFrame(const Mat &frame);


private:
    std::string path_root_;
    std::string config_path_;
    std::string mtcnn_path_;
    std::string sp_model_path_;
    std::string head_pose_model_path_;
    std::string faceid_path_;
    std::string gaze_tracking_path_;

//    std::shared_ptr<MTCNNDetector> detector_;
    std::shared_ptr<AlignMethod> align_method_;
    std::shared_ptr<FaceAlign> face_align_;
    std::shared_ptr<ShapePredictor> predictor_;
    std::shared_ptr<HeadPoseEstimator> head_pose_estimator_;
    std::shared_ptr<FaceID> faceid_;
    std::shared_ptr<FaceAttribute> faceAttribute_;
    std::shared_ptr<ObjectDetect> object_detect_;
    std::vector<std::string> arguments_;

    ConfigTracker config_;
    HeadPoseStatus head_pose_detector_;
    EyeMouthStatusPlus eye_mouth_detector_;

    bool first_time_flage_;
    bool keep_running_flag_;
    bool calib_flag_;
    bool smoke_state_;
    bool call_state_;

    std::mutex frame_mutex_;
    std::mutex bbox_mutex_;
    std::mutex landmark_mutex_;
    std::mutex angle_mutex_;
    std::mutex result_mutex_;

//    Judger smoke_judger_;
    SmokeJudger smoke_judger_;
    Judger call_judger_;
    Judger close_eye_judger_;
    Judger open_mouth_judger_;
    Judger distract_judger_;

    /// 以下为在图像中检测到的有效信息,用于各种策略判断
    cv::Mat frame_;     ///< 摄像头读取的帧
    cv::Rect face_bbox_;    ///<人脸bbox
    cv::Vec3f angles_;    ///<检测到的人脸角度
    cv::Rect smoke_bbox_;
    cv::Rect call_bbox_;
    vector<cv::Point2f> landmarks_; ///< 检测到的人脸landmarks
    Result result_;

    std::thread process_image_thread_;

    chrono::steady_clock::time_point no_face_time_;

private:
    int REGISTNUM = 10;
    int current_regist_num_;
    bool regist_over_flag_;
    bool mark_no_face_;
    bool face_match_flag_;
    std::string feature_name_path_;
    std::string name_;
    std::mutex name_mutex_;
    struct Feature{
        std::string name;
        std::vector<cv::Mat> features;
    };
    template <typename T>
    std::string to_string(T value){
        std::ostringstream os ;
        os << value ;
        return os.str() ;
    }
    bool FaceIDRun(const std::string& name);
    bool Regist(const std::string& name);
    bool LoadFeature();
    bool WriteFeature(const cv::Mat& feature, const std::string& path);
    bool ReadFeature(cv::Mat &feaure, const std::string& path);

    std::string GetName(const cv::Mat& feature);
    std::vector<std::string>Listdir(const std::string& folder);
    std::vector<std::string>Listfile(const std::string& folder);

    static const int FEATURE_LENGTH;
    static const float same_face_thresh;
    static const std::string UnknowFace;
    static const std::string NoFace;

    std::vector<Feature> Features_;
    std::unordered_map<std::string, int>name_times_;
};


#endif //STRATEGY_TOTALFLOW_H
