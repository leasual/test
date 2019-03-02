//
// Created by untouch on 19-1-17.
//

#include "smoke_judger.h"

bool SmokeJudger::Detect(bool smoke) {
    if(states_.size() == size_)
        states_.pop_front();
    states_.emplace_back(smoke);

    int smoke_count = 0;
    for(auto iter = states_.rbegin() ; iter != states_.rend(); ++iter){
        if(*iter){ if(++smoke_count > threshold_) return true;}//疲劳判决直接返回
    }
    return false;
}
