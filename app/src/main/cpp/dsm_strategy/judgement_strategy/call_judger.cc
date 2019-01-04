//
// Created by wangjian on 18-4-27.
//

#include <iostream>
#include "call_judger.h"

CallJudger::CallJudger(float warn_percent, float judge_percent, int warn_time, int judge_time) :
        RealTimeDetect(warn_percent, judge_percent, warn_time, judge_time) {

}


int CallJudger::CheckCalling(bool is_phone) {
    int retState = Detect(is_phone);
//    cout << "input: " << is_phone << "  ,data length: " << data_list_.size() << "  ,cur_state:  " << retState
//         << std::endl;
    return retState;
}

void CallJudger::SetParam(float warn_percent, float judge_percent, int warn_time, int judge_time) {
    RealTimeDetect::SetParam(warn_percent, judge_percent, warn_time, judge_time);
}
