//
// Created by slam on 18-10-23.
//

#include "faceAlign.h"
#include "align_method.h"

#include <iostream>

FaceAlign::FaceAlign(AlignMethod *am) {
    this->_am = am;
}

FaceAlign::~FaceAlign(){
    if(!_am)
        delete _am;
}

cv::Mat FaceAlign::DoAlign() {
    return this->_am->Align();
}
