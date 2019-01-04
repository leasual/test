//
// Created by slam on 18-10-23.
//

#ifndef DMS_FACEALIGN_H
#define DMS_FACEALIGN_H

#include <iostream>
#include <opencv2/opencv.hpp>

class AlignMethod;

class FaceAlign{
public:
    FaceAlign(AlignMethod *am);
    ~FaceAlign();
    cv::Mat DoAlign();

private:
    AlignMethod *_am;
};

#endif //DMS_FACEALIGN_H
