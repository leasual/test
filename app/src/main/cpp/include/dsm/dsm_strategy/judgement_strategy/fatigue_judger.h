
 /*! \file fatigue_judger.h
*  \brief 疲劳检测的策略
*  \author wangjian
*  \date 2018-4-27
*/
#ifndef DSM_STRATEGY_FATIGUE_JUDGER_H
#define DSM_STRATEGY_FATIGUE_JUDGER_H

#include "real_time_detect.h"

/// \brief 疲劳检测策略，继承了RealTimeDetect的接口
class FatigueJudger : public RealTimeDetect {
public:

    /// \brief 构造函数
    FatigueJudger(float warn_percent = 0, float judge_percent = 0,
            int warn_time = 0, int judge_time = 0);

    /// \brief 疲劳检测策略判定
    int CheckFatigue(bool, bool);

    /// \brief 设置策略参数
    void SetParam(float warn_percent = 0, float judge_percent = 0,
                  int warn_time = 0, int judge_time = 0);
};


#endif //DSM_STRATEGY_FATIGUE_JUDGER_H
