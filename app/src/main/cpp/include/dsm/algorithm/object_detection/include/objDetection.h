#ifndef OBJ_INFO_H
#define OBJ_INFO_H
#include <opencv2/dnn.hpp>
#include <string>
#include <vector>

enum Action
{
    SMOKE,
    CALL,
    FACE,
    PALM
};

struct ObjInfo
{
    Action action_;
    //int classNun_;
    float score_;
    int xL_;
    int yL_;
    int xR_;
    int yR_;
};

cv::dnn::Net createNet(const std::string& model, const std::string& weight);

std::vector<ObjInfo> objDetection(cv::dnn::Net& net, const cv::Mat& img, float detectThresh = 0.4);

bool getSmokeState();

bool getCallState();

bool getPalState();

bool getFace();

#endif //OBJ_INFO_H