//
// Created by untouch on 19-4-9.
//

#include "distract_judger.h"


int DistractJudger::Detect(bool state) {
    if(states_.size() == size_)
        states_.pop_front();
    states_.emplace_back(state);

    int count = 0;

    for(auto iter = states_.rbegin() ; iter != states_.rend(); ++iter){
        if(*iter) ++count;
        else break;
    }

//    if(count >= second_threshold_) return 2;
//    else if(count >= threshold_) return 1;
    if(count >= threshold_ ) return 1;
    else return 0;
}
