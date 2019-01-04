
/*! \file distraction_judger.h
*  \brief 分神检测的策略
*  \author wangjian
*  \date 2018-4-27
*/

#ifndef DSM_STRATEGY_DISTRACTION_JUDGER_H
#define DSM_STRATEGY_DISTRACTION_JUDGER_H

#include "real_time_detect.h"

/// \brief 分神检测策略，继承了RealTimeDetect的接口
class DistractionJudger : public RealTimeDetect {
public:
    /// \brief 构造函数
    DistractionJudger(float pitch_base = 0, float yaw_base = 0, float warn_percent = 0,
                float judge_percent = 0, float l_diff = 0, float r_diff = 0,
                float u_diff = 0, float d_diff = 0, int warn_time = 0, int judge_time = 0);

    /// \brief 根据当前角度判定策略结果
    int CheckDistraction(float pitch, float yaw);

    /// \brief 设置策略参数
    void SetParam(float pitch_base = 0, float yaw_base = 0, float warn_percent = 0,
                  float judge_percent = 0, float l_diff = 0, float r_diff = 0,
                  float u_diff = 0, float d_diff = 0, int warn_time = 0, int judge_time = 0);

private:
    /// \brief 获取垂直方向的状态
    bool GetRationVert(float);

    /// \brief 获取水平方向的状态
    bool GetRationHori(float);

private:
    //垂直方向的两个临界值
    float vert_upper_thres_;    ///< 垂直方向上方的临界值
    float vert_bottom_thres_;   ///< 垂直方向下方的临界值

    //水平方向的两个临界值
    float hori_left_thres_;        ///< 水平方向左侧的临界值
    float hori_right_thres_;       ///< 水平方向右侧的临界值
};


#endif //DSM_STRATEGY_DISTRACTION_JUDGER_H
