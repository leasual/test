#include "config_tracker.h"

//基准时间
int ConfigTracker::alignment_time_ = 0;

//显示landmarks
bool ConfigTracker::show_landmarks_;

//分神的配置参数
int ConfigTracker::distraction_warn_time_ = 0;
int ConfigTracker::distraction_judge_time_ = 0;
int ConfigTracker::distraction_warn_thres_ = 0;
int ConfigTracker::distraction_judge_thres_ = 0;
int ConfigTracker::distraction_left_angle_ = 0;
int ConfigTracker::distraction_right_angle_ = 0;
int ConfigTracker::distraction_up_angle_ = 0;
int ConfigTracker::distraction_down_angle_ = 0;

//疲劳检测的配置参数
int ConfigTracker::fatigue_warn_time_ = 0;
int ConfigTracker::fatigue_judge_time_ = 0;
int ConfigTracker::fatigue_warn_thres_ = 0;
int ConfigTracker::fatigue_judge_thres_ = 0;

//抽烟检测的配置参数
int ConfigTracker::smoke_judge_time_ = 0;
int ConfigTracker::smoke_judge_thres_ = 0;

//打电话的配置参数
int ConfigTracker::call_warn_time_ = 0;
int ConfigTracker::call_judge_time_ = 0;
int ConfigTracker::call_warn_thres_ = 0;
int ConfigTracker::call_judge_thres_ = 0;


//打哈欠（疲劳）检测的配置参数
int ConfigTracker::yawn_warn_time_ = 0;
int ConfigTracker::yawn_judge_time_ = 0;
int ConfigTracker::yawn_warn_thres_ = 0;
int ConfigTracker::yawn_judge_thres_ = 0;


ConfigTracker::ConfigTracker(const std::string& tracker_file) : Config(tracker_file) {
    LoadData();
}

void ConfigTracker::LoadData() {
    //基准时间(s)
    alignment_time_ = param<int>("alignment_time", 60);

    //显示landmarks
    show_landmarks_ = static_cast<bool>(param<int>("show_landmarks",0));

    //分神的配置参数
    distraction_warn_time_ = param<int>("distraction_warn_time", 2);
    distraction_judge_time_ = param<int>("distraction_judge_time", 4);
    distraction_warn_thres_ = param<int>("distraction_warn_threshold", 60);
    distraction_judge_thres_ = param<int>("distraction_judge_threshold", 60);
    distraction_left_angle_ = param<int>("distraction_left_angle", -20);
    distraction_right_angle_ = param<int>("distraction_right_angle", 20);
    distraction_up_angle_ = param<int>("distraction_up_angle", -20);
    distraction_down_angle_ = param<int>("distraction_down_angle", 20);

    //疲劳检测的配置参数
    fatigue_warn_time_ = param<int>("fatigue_warn_time", 2);
    fatigue_judge_time_ = param<int>("fatigue_judge_time", 4);
    fatigue_warn_thres_ = param<int>("fatigue_warn_threshold", 80);
    fatigue_judge_thres_ = param<int>("fatigue_judge_threshold", 80);

    //抽烟检测的配置参数
    smoke_judge_time_ = param<int>("smoke_judge_time", 1);
    smoke_judge_thres_ = param<int>("smoke_judge_threshold", 60);

    //打电话的配置参数
    call_warn_time_ = param<int>("call_warn_time", 1);
    call_judge_time_ = param<int>("call_judge_time", 2);
    call_warn_thres_ = param<int>("call_warn_threshold", 60);
    call_judge_thres_ = param<int>("call_judge_threshold", 60);


    //说话检测的配置参数
    yawn_warn_time_ = param<int>("yawn_warn_time", 1);
    yawn_judge_time_ = param<int>("yawn_judge_time", 2);
    yawn_warn_thres_ = param<int>("yawn_warn_threshold", 60);
    yawn_judge_thres_ = param<int>("yawn_judge_threshold", 60);

}
