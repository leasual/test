//
// Created by untouch on 18-7-16.
//
#pragma once

// 检测逻辑：
// 连续有多少帧的张嘴　判断其为打哈欠．
// 又张嘴又闭嘴　　　　判断其为说话．

#include "real_time_detect.h"

class YawnJudger : public RealTimeDetect {
public:
    YawnJudger(float warn_percent = 0, float judge_percent = 0, int warn_time = 0, int judge_time = 0);

    int CheckYawn(bool yawnState);

    void SetParam(float warn_percent = 0, float judge_percent = 0, int warn_time = 0, int judge_time = 0);

    virtual int Detect(bool cur_state);

    virtual void ResetDetect();
private:
    int FindMaxMouthOpenTime();

private:
    deque<pair<int, chrono::steady_clock::time_point>> data_list_yawn_;
};

