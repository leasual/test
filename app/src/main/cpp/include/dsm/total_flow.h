//
// Created by zhangbo on 18-5-8.
//
/*! \file TotalFlow.h
 *  \brief 定义程序的主流程
 *
 *  该文件为程序的入口,并且定义了程序的整体流程
 */
//#define ZUOLEI

#ifndef STRATEGY_TOTALFLOW_H
#define STRATEGY_TOTALFLOW_H

#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <opencv2/opencv.hpp>
#include <dsm_strategy/judgement_strategy/smoke_judger_plus.h>

#include "shape_predict_untouch.h"
#include "detector.h"
#include "align_method.h"
#include "faceAlign.h"
#include "faceID.h"
#include "faceAttribute.h"
#include "align_method.h"
#include "faceAlign.h"
#include "head_pose_estimator.h"

#include "enum_types.h"
#include "config_tracker.h"
#include "judgement_strategy/alignment_detection.h"
#include "judgement_strategy/distraction_judger.h"
#include "judgement_strategy/smoke_judger.h"
#include "judgement_strategy/call_judger.h"
//#include "judgement_strategy/yawn_judger.h"
#include "Util.h"

#if defined(USE_NCNN)
#include "net.h"
#include "objDetectionNCNN.h"
#else
#include "objDetection.h"
#endif

#if defined(ZUOLEI)
#include "eyemouthstatus.h"
#include "judgement_strategy/fatigue_judger.h"
#else
#include "eye_mouth_plus.h"
#include "fatigure_judger_plus.h"
#endif

using namespace std;
using namespace cv;

//TODO : 将值改成枚举类型
class TotalFlow;
class Strategy{
public:
    Strategy():value_(0), bbox_(){};
    Strategy(const Strategy&) = default;
    Strategy&operator=(const Strategy&) = default;
    ~Strategy(){};
    inline bool SetValue(int val, const cv::Rect& bbox){value_ = val; bbox_ = bbox; return true;}
    inline bool GetValue(int& val, cv::Rect& bbox) const {val = value_; bbox = bbox_; return true;}
private:
    int value_;
    cv::Rect bbox_;
};

class Result{
public:
    Result(){};
    Result(const Result&) = default;
    Result&operator=(const Result&) = default;
    ~Result(){};
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

private:
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

    /// \brief 程序运行入口
    void Run(cv::Mat& frame, Result& result, bool regist = false, const std::string& name = "");
private:
    /// \brief 用于处理图像的线程
    void ProcessImageThread();

    /// \brief 处理程序的函数
    void RunProcess();

    /// \brief 校准阶段的运行程序
    void RunCalibration();

    /// \brief 运行驾驶员检测策略的主函数
    /// \details 不同的检测策略之间的优先级从高到低依次为：分神检测，疲劳检测，吸烟检测，打电话检测，可疑行为检测，
    /// 聊天检测。检测策略按照优先级依次执行，只有当某个策略为正常状态时，才继续执行下一个检测策略。
    void RunMainStep();


    /// \brief 检测图像中的landmark,香烟,手掌,打电话,头的旋转角度，获取的数据用于检测驾驶员状态
    /// \param [in] frame 要检测的图像
    /// \retval 检测是否成功
    bool DetectFrame(const Mat &frame);


    /// \brief 运行分神检测策略
    /// \details 如果分神检测策略为正常状态，则下面将执行疲劳检测策略;
    /// 如果出现预警或判决状态，则需要保存图片文件并发送消息给QT界面
    void RunDistractDetection();

    /// \brief 运行疲劳检测策略
    /// \details 如果疲劳检测策略为正常状态，则下面将执行吸烟检测策略;
    /// 如果疲劳检测策略出现预警或判决行为，则需要保存图片文件并发送消息给QT界面
    void RunFatigueDectection();    // 疲劳检测阶段

    /// \brief 运行吸烟检测策略
    /// \details 如果吸烟检测策略为正常状态，则下一步将执行打电话检测策略；
    /// 如果吸烟检测策略为预警或判决状态，则需要违规图片并发送消息给QT界面
    void RunSmokeDetection();   // 吸烟检测阶段

    /// \brief 运行打电话检测策略
    /// \details 如果打电话检测策略为正常状态，则下一步将执行可疑行为检测策略；
    /// 如打电话检测策略为预警或判决状态，则需要违规图片并发送消息给QT界面
    void RunCallDetection();    // 打电话检测阶段

private:
    std::string path_root_;
    std::string config_path_;
    std::string mtcnn_path_;
    std::string sp_model_path_;
    std::string faceid_path_;
    std::string gaze_tracking_path_;

    std::shared_ptr<MTCNNDetector> detector_;
    std::shared_ptr<AlignMethod> align_method_;
    std::shared_ptr<FaceAlign> face_align_;
    std::shared_ptr<ShapePredictor> predictor_;
    std::shared_ptr<FaceID> faceid_;
    std::shared_ptr<FaceAttribute> faceAttribute_;
    std::vector<std::string> arguments_;

#if defined(USE_NCNN)
    ncnn::Net net_;
#else
    cv::dnn::Net net_;
#endif

    ConfigTracker config_;
#if defined(ZUOLEI)
    EyeMouthStatus eye_mouth_detector_;
#else
    EyeMouthStatusPlus eye_mouth_detector_;
#endif

    bool first_time_flage_;
    bool keep_running_flag_;
    bool smoke_state_;
    bool call_state_;

    std::mutex frame_mutex_;
    std::mutex bbox_mutex_;
    std::mutex landmark_mutex_;
    std::mutex angle_mutex_;
    std::mutex result_mutex_;

    AlignmentDetection alignment_judger_;
    DistractionJudger distract_judger_;
    SmokeJudger smoke_judger_;
//    SmokeJudgerPlus smoke_judger_;
    CallJudger call_judger_;
#if defined(ZUOLEI)
    FatigueJudger fatigue_judger_;
#else
    FatigueJudgerPlus fatigue_judger_;
#endif

    /// 以下为在图像中检测到的有效信息,用于各种策略判断
    cv::Mat frame_;     ///< 摄像头读取的帧
    cv::Rect face_bbox_;    ///<人脸bbox
    cv::Vec3f angles_;    ///<检测到的人脸角度
    cv::Rect smoke_bbox_;
    cv::Rect call_bbox_;
    vector<cv::Point2f> landmarks_; ///< 检测到的人脸landmarks
    Result result_;

    int alignment_time_;
    std::thread process_image_thread_;
    RunStep main_step_; // 检测程序的主阶段，取值范围为：规定动作、校准、检测策略
    RunStep sub_step_;  ///< 当前所处的检测策略，取值范围为：分神检测、疲劳检测、吸烟检测、打电话检测、手掌（可疑行为）检测，聊天检测

    chrono::steady_clock::time_point start_time_;
    chrono::steady_clock::time_point no_face_time_;

private:
    const int REGISTNUM = 10;
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
