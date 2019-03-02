//
// Created by untouch on 19-1-17.
//

#ifndef DSM_CPP_SMOKE_JUDGER_H
#define DSM_CPP_SMOKE_JUDGER_H

#include "judger.h"

class SmokeJudger: public Judger{
public:
    bool Detect(bool smoke);
};


#endif //DSM_CPP_SMOKE_JUDGER_H
