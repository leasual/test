#ifndef DSM_STRATEGY_CONFIG_TRACKER_H
#define DSM_STRATEGY_CONFIG_TRACKER_H

#include "utils/file/config.h"

using namespace untouch_utils;


/*! \brief 定义访问配置文件的接口
 *
 * 用于访问配置文件中的各种数据，需要在该类内定义每种配置参数的类型为static,
 * 然后再调用LoadData()从配置文件中读取各个配置参数的值
 */
class ConfigTracker : public Config {
public:
    /// \brief 构造函数
    /// \param [in] tracker_file 配置文件的路径
    ConfigTracker(const std::string &tracker_file);


    /// \brief 加载配置文件内容
    void LoadData() ;

public:
    //分神的配置参数
    static int distraction_threshold_;
    static int distraction_left_angle_;
    static int distraction_right_angle_;
    static int distraction_up_angle_;
    static int distraction_down_angle_;
    static int distraction_video_times_;
    static int distraction_photo_numbers_;
    static int distraction_photo_interval_;

    //疲劳检测的配置参数
    static int fatigue_threshold_;
    static int fatigue_video_times_;
    static int fatigue_photo_numbers_;
    static int fatigue_photo_interval_;

    //抽烟检测的配置参数
    static int smoke_threshold_;
    static int smoke_alarm_interval_;
    static int smoke_video_times_;
    static int smoke_photo_numbers_;
    static int smoke_photo_interval_;

    //打电话的配置参数
    static int call_threshold_;
    static int call_alarm_interval_;
    static int call_video_times_;
    static int call_photo_numbers_;
    static int call_photo_interval_;

    //驾驶员异常
    static int abnormal_video_times_;
    static int abnormal_photo_numbers_;
    static int abnormal_photo_interval_;

    //报警设置_;
    static int speed_threshold_;
    static int alarm_interval_;
    static int alarm_volume_;

    //主动拍照设置
    static int photo_strategy_;
    static int photo_time_interval_;
    static int photo_distance_interval_;
    static int photo_numbers_;
    static int photo_interval_;

    //拍照分辨率设置
    static int photo_resolution_;

    //视频录制分辨率设置
    static int video_resolution_;
};


#endif //DSM_STRATEGY_CONFIG_TRACKER_H
