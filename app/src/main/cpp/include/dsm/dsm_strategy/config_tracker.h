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
    void LoadData();

public:
    /// \brief 校准时间
    static int alignment_time_;

    /// \brief show landmarks
    static bool show_landmarks_;

    /// \brief 分神的配置参数
    static int distraction_warn_time_, distraction_judge_time_,
            distraction_warn_thres_, distraction_judge_thres_,
            distraction_left_angle_, distraction_right_angle_,
            distraction_up_angle_, distraction_down_angle_;

    /// \brief 疲劳检测的配置参数
    static int fatigue_warn_time_, fatigue_judge_time_,
            fatigue_warn_thres_, fatigue_judge_thres_;

    /// \brief 抽烟检测的配置参数,吸烟只有判决，没有预警
    static int smoke_judge_time_, smoke_judge_thres_;

    /// \brief 打电话的配置参数
    static int call_warn_time_, call_judge_time_, call_warn_thres_, call_judge_thres_;

    /// \brief 打哈欠检测配置参数
    static int yawn_warn_time_, yawn_judge_time_, yawn_warn_thres_, yawn_judge_thres_;

};


#endif //DSM_STRATEGY_CONFIG_TRACKER_H
