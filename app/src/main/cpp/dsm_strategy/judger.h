//
// Created by untouch on 19-1-16.
//

#ifndef DSM_CPP_JUDGER_H
#define DSM_CPP_JUDGER_H

#include <iostream>
#include <deque>

class Judger{
public:
    void SetParam(size_t size, size_t threshold){ size_ = size; threshold_ = threshold;}
    bool Detect(bool state);

protected:
    std::deque<bool>states_;
    size_t size_;
    size_t threshold_;
};


#endif //DSM_CPP_JUDGER_H
