//
// Created by wangjian on 18-4-27.
//

#include "smoke_judger.h"

SmokeJudger::SmokeJudger(float warn_percent, float judge_percent, int warn_time, int judge_time) :
        RealTimeDetect(warn_percent, judge_percent, warn_time, judge_time, false) {

}

int SmokeJudger::CheckSmoking(bool smokingState) {
    int retState = Detect(smokingState);
    return retState;
}

void SmokeJudger::SetParam(float warn_percent, float judge_percent, int warn_time, int judge_time) {
    RealTimeDetect::SetParam(warn_percent, judge_percent, warn_time, judge_time);
}
