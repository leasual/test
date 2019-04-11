//
// Created by untouch on 19-4-9.
//

#ifndef DSM_CPP_DISTRACT_JUDGER_H
#define DSM_CPP_DISTRACT_JUDGER_H

#include "judger.h"



class DistractJudger:public Judger{
public:
    void SetParam(size_t size, size_t first_threshold, size_t second_threshold){
        Judger::SetParam(size,first_threshold);
        second_threshold_ = second_threshold;
    }
    int Detect(bool state);

private:
    size_t second_threshold_;
};


#endif //DSM_CPP_DISTRACT_JUDGER_H
