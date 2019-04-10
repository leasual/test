#include "config_tracker.h"

//分神的配置参数
float ConfigTracker::distraction_threshold_ = 0.f;
int ConfigTracker::distraction_left_angle_ = 0;
int ConfigTracker::distraction_right_angle_ = 0;
int ConfigTracker::distraction_up_angle_ = 0;
int ConfigTracker::distraction_down_angle_ = 0;
int ConfigTracker::distraction_video_times_ = 0;
int ConfigTracker::distraction_photo_numbers_ = 0;
int ConfigTracker::distraction_photo_interval_ = 0;

//打哈欠检测的配置参数
float ConfigTracker::yawn_threshold_ = 0.f;

//疲劳检测的配置参数
float ConfigTracker::fatigue_threshold_ = 0.f;
int ConfigTracker::fatigue_video_times_ = 0;
int ConfigTracker::fatigue_photo_numbers_ = 0;
int ConfigTracker::fatigue_photo_interval_ = 0;

//抽烟检测的配置参数
float ConfigTracker::smoke_threshold_ = 0.f;
int ConfigTracker::smoke_alarm_interval_ = 0;
int ConfigTracker::smoke_video_times_ = 0;
int ConfigTracker::smoke_photo_numbers_ = 0;
int ConfigTracker::smoke_photo_interval_ = 0;

//打电话的配置参数
float ConfigTracker::call_threshold_ = 0.f;
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

//路径设置
std::string ConfigTracker::path_load_img_ = "";
std::string ConfigTracker::path_save_img_ = "";

//摄像头是否在线
int ConfigTracker::camera_online_  = 0;

//摄像头ID
int ConfigTracker::camera_index_ = 0;
//摄像头帧率
int ConfigTracker::camera_fps_= 0;

//数据处理帧率
int ConfigTracker::process_fps_= 0;

// 保存结果标志位
int ConfigTracker::write_result_ = 0;

//保存结果路径
std::string ConfigTracker::path_save_result_ = "";

bool ConfigTracker::debug_ = false;   //显示landmark和bbox

ConfigTracker::ConfigTracker(const std::string& tracker_file) : Config(tracker_file) {
    LoadData();
}

void ConfigTracker::LoadData() {
//#分神的配置参数
    distraction_threshold_ = param<float>("distraction_threshold", 4);
    distraction_left_angle_ = param<int>("distraction_left_angle", 25);
    distraction_right_angle_ = param<int>("distraction_right_angle", -35);
    distraction_up_angle_ = param<int>("distraction_up_angle", 25);
    distraction_down_angle_ = param<int>("distraction_down_angle", -35);
    distraction_video_times_ = param<int>("distraction_video_times", 5);
    distraction_photo_numbers_ = param<int>("distraction_photo_numbers", 3);
    distraction_photo_interval_ = param<int>("distraction_photo_interval", 2);

//#打哈欠检测的配置参数
    yawn_threshold_ = param<float>("yawn_threshold", 0.5);

//#疲劳检测的配置参数.
    fatigue_threshold_ = param<float>("fatigue_threshold", 0.5);
    fatigue_video_times_ = param<int>("fatigue_video_times", 5);
    fatigue_photo_numbers_ = param<int>("fatigue_photo_numbers", 3);
    fatigue_photo_interval_ = param<int>("fatigue_photo_interval", 2);

//#抽烟检测的配置参数.
    smoke_threshold_ = param<float>("smoke_threshold", 0.5);
    smoke_alarm_interval_ = param<int>("smoke_alarm_interval", 180);
    smoke_video_times_ = param<int>("smoke_video_times", 5);
    smoke_photo_numbers_ = param<int>("smoke_photo_numbers", 3);
    smoke_photo_interval_ = param<int>("smoke_photo_interval", 2);

//#打电话的配置参数.
    call_threshold_ = param<float>("call_threshold", 0.5);
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

//路径设置
	path_save_img_ = param<std::string>("path_save_img","");
	path_load_img_ = param<std::string>("path_load_img","");

//摄像头是否在线
	camera_online_ = param<int>("camera_online",0);

//摄像头ID
    camera_index_ = param<int>("camera_index", 0);

//摄像头帧率
    camera_fps_ = param<int>("camera_fps", 25);

//数据处理帧率
    process_fps_ = param<int>("process_fps", 25);

// 保存结果标志位
    write_result_ = param<int>("write_result", 0);

//保存结果路径
    path_save_result_ = param<std::string>("path_save_result", "");

// 保存结果标志位
    write_result_ = param<int>("write_result", 0);

//保存结果路径
    path_save_result_ = param<std::string>("path_save_result", "");

    debug_ = param<int>("debug", 0) == 1;
}
