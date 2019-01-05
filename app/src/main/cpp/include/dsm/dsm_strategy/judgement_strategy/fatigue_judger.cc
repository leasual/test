//
// Created by wangjian on 18-4-27.
//

#include "fatigue_judger.h"

FatigueJudger::FatigueJudger(float warn_percent, float judge_percent, int warn_time, int judge_time) :
        RealTimeDetect(warn_percent, judge_percent, warn_time, judge_time) {
}


/**
 *
 * @param reyeState：睁眼为1,闭眼为0
 * @param leyeState：睁眼为1,闭眼为0
 * @return
 */
int FatigueJudger::CheckFatigue(bool reyeState, bool leyeState) {

    //把两只眼睛状态合并成一个值
    bool eyeState = (!reyeState) && (!leyeState);
    int retState = Detect(eyeState);
    return retState;
}

void FatigueJudger::SetParam(float warningPercent, float judgePercent, int warningTime, int judgeTime) {
    RealTimeDetect::SetParam(warningPercent, judgePercent, warningTime, judgeTime);
}
