#include "config_tracker.h"

//分神的配置参数
int ConfigTracker::distraction_threshold_ = 0;
int ConfigTracker::distraction_left_angle_ = 0;
int ConfigTracker::distraction_right_angle_ = 0;
int ConfigTracker::distraction_up_angle_ = 0;
int ConfigTracker::distraction_down_angle_ = 0;
int ConfigTracker::distraction_video_times_ = 0;
int ConfigTracker::distraction_photo_numbers_ = 0;
int ConfigTracker::distraction_photo_interval_ = 0;

//疲劳检测的配置参数
int ConfigTracker::fatigue_threshold_ = 0;
int ConfigTracker::fatigue_video_times_ = 0;
int ConfigTracker::fatigue_photo_numbers_ = 0;
int ConfigTracker::fatigue_photo_interval_ = 0;

//抽烟检测的配置参数
int ConfigTracker::smoke_threshold_ = 0;
int ConfigTracker::smoke_alarm_interval_ = 0;
int ConfigTracker::smoke_video_times_ = 0;
int ConfigTracker::smoke_photo_numbers_ = 0;
int ConfigTracker::smoke_photo_interval_ = 0;

//打电话的配置参数
int ConfigTracker::call_threshold_ = 0;
int ConfigTracker::call_alarm_interval_ = 0;
int ConfigTracker::call_video_times_ = 0;
int ConfigTracker::call_photo_numbers_ = 0;
int ConfigTracker::call_photo_interval_ = 0;

//驾驶员异常
int ConfigTracker::abnormal_video_times_ = 0;
int ConfigTracker::abnormal_photo_numbers_ = 0;
int ConfigTracker::abnormal_photo_interval_ = 0;

//报警设置
int ConfigTracker::speed_threshold_ = 0;
int ConfigTracker::alarm_interval_ = 0;
int ConfigTracker::alarm_volume_ = 0;

//主动拍照设置
int ConfigTracker::photo_strategy_ = 0;
int ConfigTracker::photo_time_interval_ = 0;
int ConfigTracker::photo_distance_interval_ = 0;
int ConfigTracker::photo_numbers_ = 0;
int ConfigTracker::photo_interval_ = 0;

//拍照分辨率设置
int ConfigTracker::photo_resolution_ = 0;

//视频录制分辨率设置
int ConfigTracker::video_resolution_ = 0;

ConfigTracker::ConfigTracker(const std::string& tracker_file) : Config(tracker_file) {
    LoadData();
}

void ConfigTracker::LoadData() {
//#分神的配置参数
    distraction_threshold_ = param<int>("distraction_threshold", 60);
    distraction_left_angle_ = param<int>("distraction_left_angle", 25);
    distraction_right_angle_ = param<int>("distraction_right_angle", -35);
    distraction_up_angle_ = param<int>("distraction_up_angle", 25);
    distraction_down_angle_ = param<int>("distraction_down_angle", -35);
    distraction_video_times_ = param<int>("distraction_video_times", 5);
    distraction_photo_numbers_ = param<int>("distraction_photo_numbers", 3);
    distraction_photo_interval_ = param<int>("distraction_photo_interval", 2);
//#疲劳检测的配置参数.
    fatigue_threshold_ = param<int>("fatigue_threshold", 60);
    fatigue_video_times_ = param<int>("fatigue_video_times", 5);
    fatigue_photo_numbers_ = param<int>("fatigue_photo_numbers", 3);
    fatigue_photo_interval_ = param<int>("fatigue_photo_interval", 2);

//#抽烟检测的配置参数.
    smoke_threshold_ = param<int>("smoke_threshold", 60);
    smoke_alarm_interval_ = param<int>("smoke_alarm_interval", 180);
    smoke_video_times_ = param<int>("smoke_video_times", 5);
    smoke_photo_numbers_ = param<int>("smoke_photo_numbers", 3);
    smoke_photo_interval_ = param<int>("smoke_photo_interval", 2);

//#打电话的配置参数.
    call_threshold_ = param<int>("call_threshold", 60);
    call_alarm_interval_ = param<int>("call_alarm_interval", 180);
    call_video_times_ = param<int>("call_video_times", 5);
    call_photo_numbers_ = param<int>("call_photo_numbers", 3);
    call_photo_interval_ = param<int>("call_photo_interval", 2);

//#驾驶员异常
    abnormal_video_times_ = param<int>("abnormal_video_times", 5);
    abnormal_photo_numbers_ = param<int>("abnormal_photo_numbers", 3);
    abnormal_photo_interval_ = param<int>("abnormal_photo_interval", 2);

//#报警设置
    speed_threshold_ = param<int>("speed_threshold", 60);
    alarm_interval_ = param<int>("alarm_interval", 10);
    alarm_volume_ = param<int>("alarm_volume", 10);

//#主动拍照设置
    photo_strategy_ = param<int>("photo_strategy", 0);
    photo_time_interval_ = param<int>("photo_time_interval", 3600);
    photo_distance_interval_ = param<int>("photo_distance_interval", 200);
    photo_numbers_ = param<int>("photo_numbers", 3);
    photo_interval_ = param<int>("photo_interval", 2);

//#拍照分辨率设置
    photo_resolution_ = param<int>("photo_resolution", 0);

//#视频录制分辨率设置
    video_resolution_ = param<int>("video_resolution", 0);
}
