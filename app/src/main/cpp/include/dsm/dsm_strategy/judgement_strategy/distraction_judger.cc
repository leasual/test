//
// Created by wangjian on 18-4-27.
//

#include "distraction_judger.h"
#include <iostream>

DistractionJudger::DistractionJudger(float pitch_base, float yaw_base, float warn_percent,
                                     float judge_percent, float l_diff, float r_diff,
                                     float u_diff, float d_diff, int warn_time, int judge_time):
        RealTimeDetect(warn_percent,judge_percent,warn_time,judge_time) {
    //垂直方向的两个临界值
    vert_upper_thres_ = pitch_base + u_diff;
    vert_bottom_thres_ = pitch_base + d_diff;
    //水平方向的两个临界值
    hori_left_thres_ = yaw_base + l_diff;
    hori_right_thres_ = yaw_base + r_diff;
}


//判断垂直方向的角度是否在正常范围内
bool DistractionJudger::GetRationVert(float ver) {
    if(ver <= vert_upper_thres_ || ver >= vert_bottom_thres_)
    {
        return true;
    }else{
        return false;
    }
}

//判断水平方向的角度是否在正常范围内
bool DistractionJudger::GetRationHori(float hori) {
//    if(hori <= hori_left_thres_ || hori >= hori_right_thres_)
    if(hori >= hori_left_thres_ || hori <= hori_right_thres_)
    {
        return true;
    }else{
        return false;
    }
}

int DistractionJudger::CheckDistraction(float pitch, float yaw){
    //把角度转换成1或0
    bool vertAng = GetRationVert(pitch);
    bool horiAng = GetRationHori(yaw);
    //把垂直和水平方向合并成一个值
    bool angState = vertAng || horiAng;
    int retState = Detect(angState);
    return retState;
}


void DistractionJudger::SetParam(float pitch_base, float yaw_base, float warn_percent,
                                 float judge_percent, float l_diff, float r_diff,
                                 float u_diff, float d_diff, int warn_time, int judge_time) {

    RealTimeDetect::SetParam(warn_percent,judge_percent,warn_time,judge_time);
    //垂直方向的两个临界值
    vert_upper_thres_ = pitch_base + u_diff;
    vert_bottom_thres_ = pitch_base + d_diff;
    //水平方向的两个临界值
    hori_left_thres_ = yaw_base + l_diff;
    hori_right_thres_ = yaw_base + r_diff;

    hori_left_thres_ = hori_left_thres_ > 35 ? 35 : hori_left_thres_;
    hori_right_thres_ = hori_right_thres_ < -40 ? -40 : hori_right_thres_;

    std::cout << "垂直上 ：　" << vert_upper_thres_ << std::endl;
    std::cout << "垂直下 ：　" << vert_bottom_thres_ << std::endl;
    std::cout << "水平左 ：　" << hori_left_thres_ << std::endl;
    std::cout << "水平右 ：　" << hori_right_thres_ << std::endl;
}
