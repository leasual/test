//
// Created by untouch on 19-1-16.
//

#include "judger.h"

bool Judger::Detect(bool state) {
    if(states_.size() == size_)
        states_.pop_front();
    states_.emplace_back(state);

    int count = 0;
    for(auto iter = states_.rbegin() ; iter != states_.rend(); ++iter){
        if(*iter){ if(++count > threshold_) return true;}//
        else break;
    }
    return false;
}
