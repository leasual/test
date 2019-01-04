
 /*! \file real_time_detect.h
  *  \brief 策略检测的基类，根据用户输入的状态判定正常状态，预警状态，判决状态
  *  \author wangjian
  *  \date 2018-5-16
  */

#ifndef DSM_STRATEGY_REAL_TIME_DETECT_H
#define DSM_STRATEGY_REAL_TIME_DETECT_H

#include <queue>
#include <chrono>

#include "enum_types.h"

using namespace std;


/*! \brief 策略检测的基类，根据用户输入的状态判定用户当前状态
 *
 * 策略检测的状态有三种类型：正常状态，预警状态，判决状态
 * 正常状态可以进入预警状态，预警状态可以进入判决状态
 * 在正常状态下，如果warn_time_thres_时间内，违规行为的百分比超过warn_percent_thres_,则进入预警状态；否则维持正常状态
 * 在预警状态下，如果judge_time_thes_时间内，违规行为的百分比超过judge_percent_thres_,则进入判决状态；否则进入正常状态
 * 在判决状态下，如果judge_time_thres_时间内，违规行为的百分比超过judge_percent_thres_,则维持判决状态；否则进入正常状态
 */
class RealTimeDetect {
public:
    /// \brief 构造函数，对类成员变量进行初始化
    /// \param [in] warn_percent_thres 进入预警状态的违规百分比
    /// \param [in] judge_percent_thres 要进入判决状态的违规百分比
    /// \param [in] warn_time_thres 判定预警状态所需要的时间
    /// \param [in] judge_time_thres 判定判决状态所需要的时间
    // \param [in] 是否同时包含预警和判决状态
    RealTimeDetect(double warn_percent_thres, double judge_percent_thres, int warn_time_thres,
                   int judge_time_thres, bool warn_judge_flag = true);

    /// \brief 重置策略数据
    void ResetDetect();

    /// \brief 根据当前是否为违规状态，判断输出状态
    /// \param [in] cur_state 当前是否为违规状态
    int Detect(bool cur_state);

    /// \brief 设置策略的各种参数
    /// \param [in] warn_percent_thres 进入预警状态的违规百分比
    /// \param [in] judge_percent_thres 要进入判决状态的违规百分比
    /// \param [in] warn_time_thres 判定预警状态所需要的时间
    /// \param [in] judge_time_thres 判定判决状态所需要的时间
    void SetParam(double warn_percent_thres, double judge_percent_thres, int warn_time_thres,
                  int judge_time_thres);

protected:
    double warn_percent_thres_;     ///< 进入预警状态的违规百分比
    double judge_percent_thres_;    ///< 要进入判决状态的违规百分比

    int warn_time_thres_;       ///< 判定预警状态所需要的时间
    int judge_time_thres_;      ///< 判定判决状态所需要的时间
    int cur_state_sum_;     ///< 当前的违规数量统计

    StrategyResult current_state_;      ///< 当前策略的状态

    bool wan_judge_flag_;       ///< 是否同时包含预警和判决两种状态
    queue<pair<int, chrono::steady_clock::time_point>> data_list_;      ///< 缓存，保存数据为 <违规状态，时间戳>
};


#endif //DSM_STRATEGY_REAL_TIME_DETECT_H
