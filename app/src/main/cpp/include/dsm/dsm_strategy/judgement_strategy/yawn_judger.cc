//
// Created by untouch on 18-7-16.
//

#include "yawn_judger.h"
#include <iostream>

YawnJudger::YawnJudger(float warn_percent, float judge_percent, int warn_time, int judge_time) :
    RealTimeDetect(warn_percent, judge_percent, warn_time, judge_time) {
}


void YawnJudger::ResetDetect(){
    current_state_ = StrategyResult::NORMAL;
    cur_state_sum_ = 0;
    while (!data_list_yawn_.empty()) {
        data_list_yawn_.pop_front();
    }
}

int YawnJudger::Detect(bool cur_state) {
    int state = static_cast<int>(cur_state);
    //如果当前队列为空,而且输入的值为0,则直接返回正常状态
    if (current_state_ == StrategyResult::NORMAL and data_list_yawn_.empty() and state == 0) {
        return static_cast<int>(current_state_);
    }

    //获取当前时间戳,添加到队列
    chrono::steady_clock::time_point cur_time = chrono::steady_clock::now();
    data_list_yawn_.push_back({state, cur_time});

    // zh : here , calculate once every time.
    // cur_state_sum_ += state;
    int max_mouth_open_time = FindMaxMouthOpenTime();
//    std::cout << "max_mouth_open_time : " << max_mouth_open_time << "/" << data_list_yawn_.size() << std::endl;
    //获取当前时间和队列起始时间的时间差（以秒为单位）
    auto diff = chrono::duration_cast<std::chrono::milliseconds>(cur_time - data_list_yawn_.front().second);
    double diff_seconds = diff.count() / 1000.0f;

    // 根据当前状态的类型,分别采取不同的操作
    switch (current_state_) {
        case StrategyResult::NORMAL:
            // 如果时间差超过判断是否预警的时间,则判断是否进入预警状态
            if (diff_seconds > warn_time_thres_) {
                // 计算当前的百分比
                double percent = max_mouth_open_time / static_cast<double >(data_list_yawn_.size());

                // 如果当前百分比超过判断预警的最大值,则进入预警状态
                if (percent >= warn_percent_thres_) {
                    if (wan_judge_flag_ == true) {
                        current_state_ = StrategyResult::WARN;
                    } else {
                        current_state_ = StrategyResult::JUDGEMENT;
                        while (!data_list_yawn_.empty()) {
                            data_list_yawn_.pop_front();
                        }
                    }
                } else {    // 如果当前非预警状态,则弹出队列的第一个元素

                    // zh : 因为被加到cur_state_sum_上的不是0就量1，而这样做的目的是为了计算比例方便．
                    // 第一个不知道是什么，所以要减去 0无所谓，减1就减1
                    // 这一个.first一定是1吗？
                    data_list_yawn_.pop_front();


                    // 弹出第一个元素后面所有状态为0的元素
                    // zh: 为什么这样做? 为了减低这个cur_state_sum_　阈值
                    while (!data_list_yawn_.empty() and data_list_yawn_.front().first == 0) {
                        data_list_yawn_.pop_front();
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
                double percent = max_mouth_open_time / static_cast<double >(data_list_yawn_.size());
                if (percent >= judge_percent_thres_) {
                    current_state_ = StrategyResult::JUDGEMENT;
                } else {
                    current_state_ = StrategyResult::NORMAL;
                }
                // 清空当前队列
                while (!data_list_yawn_.empty()) {
                    data_list_yawn_.pop_front();
                }
            } else {
                current_state_ = StrategyResult::WARN;
            }
            break;
        case StrategyResult::JUDGEMENT:
            // 进入判决状态后,根据每1秒内的百分比判断继续维持判决状态或进入正常状态
            if (diff_seconds > 1) {
                double percent = max_mouth_open_time / static_cast<double >(data_list_yawn_.size());
                if (percent < judge_percent_thres_) {
                    current_state_ = StrategyResult::NORMAL;
                } else {
                    current_state_ = StrategyResult::JUDGEMENT;
                }
                // 清空队列内容
                while (!data_list_yawn_.empty()) {
                    data_list_yawn_.pop_front();
                }
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


int YawnJudger::CheckYawn(bool yawnState) {
    int retState = Detect(yawnState);
    return retState;
}

void YawnJudger::SetParam(float warn_percent, float judge_percent, int warn_time, int judge_time) {
    RealTimeDetect::SetParam(warn_percent, judge_percent, warn_time, judge_time);
}

/**
 * @author zhanhe
 * @note need test.
 */
int YawnJudger::FindMaxMouthOpenTime() {
    // find max continuous mouth open times.
    int max_time = 0;
    int local_max = 0;

    for(int i=0;i<data_list_yawn_.size();i++) {
        if(data_list_yawn_.at(i).first) {
            local_max++;
        }
        else {
            if(local_max > max_time) {
                max_time = local_max;
            }
            local_max = 0;
        }
    }
    if(local_max > max_time)
        max_time = local_max;
    return max_time;
}