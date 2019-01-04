//
// Created by zhangbo on 18-5-16.
//
#include <iostream>
#include "real_time_detect.h"

RealTimeDetect::RealTimeDetect(double warn_percent_thres, double judge_percent_thres, int warn_time_thres,
                               int judge_time_thres, bool warn_judge_flag) :
        warn_percent_thres_(warn_percent_thres),
        judge_percent_thres_(
                judge_percent_thres),
        warn_time_thres_(warn_time_thres),
        judge_time_thres_(judge_time_thres),
        wan_judge_flag_(warn_judge_flag) {
    current_state_ = StrategyResult::NORMAL;
    cur_state_sum_ = 0;
}

void RealTimeDetect::ResetDetect() {
    current_state_ = StrategyResult::NORMAL;
    cur_state_sum_ = 0;
    while (!data_list_.empty()) {
        data_list_.pop();
    }
}

void RealTimeDetect::SetParam(double warn_percent_thres, double judge_percent_thres,
                              int warn_time_thres, int judge_time_thres) {
    warn_percent_thres_ = warn_percent_thres;
    judge_percent_thres_ = judge_percent_thres;
    warn_time_thres_ = warn_time_thres;
    judge_time_thres_ = judge_time_thres;
}


int RealTimeDetect::Detect(bool cur_state) {
    int state = static_cast<int>(cur_state);
    //如果当前队列为空,而且输入的值为0,则直接返回正常状态
    if (current_state_ == StrategyResult::NORMAL and data_list_.empty() and state == 0) {
        return static_cast<int>(current_state_);
    }

    //获取当前时间戳,添加到队列
    chrono::steady_clock::time_point cur_time = chrono::steady_clock::now();
    data_list_.push({state, cur_time});
    cur_state_sum_ += state;

    //获取当前时间和队列起始时间的时间差（以秒为单位）
    auto diff = chrono::duration_cast<std::chrono::milliseconds>(cur_time - data_list_.front().second);
    double diff_seconds = diff.count() / 1000.0f;

    // 根据当前状态的类型,分别采取不同的操作
    switch (current_state_) {
        case StrategyResult::NORMAL:
            // 如果时间差超过判断是否预警的时间,则判断是否进入预警状态
            if (diff_seconds > warn_time_thres_) {
                // 计算当前的百分比
                double percent = cur_state_sum_ / static_cast<double >(data_list_.size());

                // 如果当前百分比超过判断预警的最大值,则进入预警状态
                if (percent >= warn_percent_thres_) {
                    if (wan_judge_flag_ == true) {
                        current_state_ = StrategyResult::WARN;
                    } else {
                        current_state_ = StrategyResult::JUDGEMENT;
                        while (!data_list_.empty()) {
                            data_list_.pop();
                        }
                        cur_state_sum_ = 0;
                    }
                } else {    // 如果当前非预警状态,则弹出队列的第一个元素
                    cur_state_sum_ -= data_list_.front().first;
                    data_list_.pop();
                    // 弹出第一个元素后面所有状态为0的元素
                    while (!data_list_.empty() and data_list_.front().first == 0) {
                        cur_state_sum_ -= data_list_.front().first;
                        data_list_.pop();
                    }
                    current_state_ = StrategyResult::NORMAL;
                }
            } else {
                current_state_ = StrategyResult::NORMAL;
            }
            break;
        case StrategyResult::WARN:
            // 如果当前时间差超过判断判决状态的时间,则根据百分比判断是否进入判决状态
            if (diff_seconds > judge_time_thres_) {
                // 根据judge_time_thres_时间段内的百分比进行判断
                double percent = cur_state_sum_ / static_cast<double >(data_list_.size());
                if (percent >= judge_percent_thres_) {
                    current_state_ = StrategyResult::JUDGEMENT;
                } else {
                    current_state_ = StrategyResult::NORMAL;
                }
                // 清空当前队列
                while (!data_list_.empty()) {
                    data_list_.pop();
                }
                cur_state_sum_ = 0;
            } else {
                current_state_ = StrategyResult::WARN;
            }
            break;
        case StrategyResult::JUDGEMENT:
            // 进入判决状态后,根据每1秒内的百分比判断继续维持判决状态或进入正常状态
            if (diff_seconds > 1) {
                double percent = cur_state_sum_ / static_cast<double >(data_list_.size());
                if (percent < judge_percent_thres_) {
                    current_state_ = StrategyResult::NORMAL;
                } else {
                    current_state_ = StrategyResult::JUDGEMENT;
                }
                // 清空队列内容
                while (!data_list_.empty()) {
                    data_list_.pop();
                }
                cur_state_sum_ = 0;
            } else {
                current_state_ = StrategyResult::JUDGEMENT;
            }
            break;
    }

    switch (current_state_) {
        case StrategyResult::NORMAL:
            return 0;
            break;
        case StrategyResult::WARN:
            return 1;
            break;
        case StrategyResult::JUDGEMENT:
            return 2;
            break;
    }

}