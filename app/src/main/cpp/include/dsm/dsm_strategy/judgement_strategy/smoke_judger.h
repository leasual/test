
/*! \file smoke_judger.h
*  \brief 吸烟检测的策略
*  \author wangjian
*  \date 2018-4-27
*/
#ifndef DSM_STRATEGY_SMOKE_JUDGER_H
#define DSM_STRATEGY_SMOKE_JUDGER_H


#include "real_time_detect.h"

/// \brief 吸烟检测的策略，继承了RealTimeDetect的接口
class SmokeJudger : public RealTimeDetect {
public:
    /// \brief 构造函数
    SmokeJudger(float warn_percent = 0, float judge_percent = 0, int warn_time = 0, int judge_time = 0);

    /// \brief 吸烟检测的策略
    int CheckSmoking(bool);

    /// \brief 设置策略的参数
    void SetParam(float warn_percent = 0, float judge_percent = 0, int warn_time = 0, int judge_time = 0);
};


#endif //DSM_STRATEGY_SMOKE_JUDGER_H
