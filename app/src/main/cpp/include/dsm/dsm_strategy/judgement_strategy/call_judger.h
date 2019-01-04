
/*! \file call_judger.h
*  \brief 打电话检测的策略
*  \author wangjian
*  \date 2018-4-27
*/

#ifndef DSM_STRATEGY_CALL_JUDGER_H
#define DSM_STRATEGY_CALL_JUDGER_H

#include "real_time_detect.h"

/// \brief 打电话检测的策略，继承了RealTimeDetect的接口
class CallJudger : public RealTimeDetect {
public:
    /// \brief 构造函数
    CallJudger(float warn_percent = 0, float judge_percent = 0, int warn_time = 0, int judge_time = 0);

    /// \brief 检测打电话策略的判定状态
    int CheckCalling(bool);

    /// \brief 设置岑数
    void SetParam(float warn_percent = 0, float judge_percent = 0, int warn_time = 0, int judge_time = 0);
};

#endif //DSM_STRATEGY_CALL_JUDGER_H
