#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <string>
#include <iostream>
#include <map>
#include "objDetection.h"

using namespace std;
using namespace cv;
using namespace dnn;

static bool g_isSomke = false;
static bool g_isCall = false;
static bool g_isPalm = false;
static bool g_isFace = false;

void resetState()
{
    g_isSomke = false;
    g_isCall = false;
    g_isFace = false;
    g_isPalm = false;
}
void setState(const vector<ObjInfo>& objInfos)
{
    for(size_t i = 0; i < objInfos.size(); i++)
    {
        const Action& action = objInfos[i].action_;
        if (action == SMOKE)
        {
            g_isSomke = true;
        }
        else if(action == CALL)
        {
            g_isCall = true;
        }
        else if(action == FACE)
        {
            g_isFace = true;
        }
        else if(action == PALM)
        {
            g_isPalm = true;
        }
    }
}

float computeArea(const ObjInfo& obj)
{
    return (obj.xR_ - obj.xL_ + 1) * (obj.yR_ - obj.yL_ + 1);
}

float iou(const ObjInfo& obj1, const ObjInfo& obj2)
{
    int xL = std::min(obj1.xL_, obj2.xL_);
    int yL = std::min(obj1.yL_, obj2.yL_);
    int xR = std::max(obj1.xR_, obj2.xR_);
    int yR = std::max(obj2.yR_, obj2.yR_);

    float intersection = (xR - xL + 1) * (yR - yL + 1);

    float area1 = computeArea(obj1);
    float area2 = computeArea(obj2);
    return intersection / (area1 + area2 - intersection);
}

bool CompareBBox(const ObjInfo & a, const ObjInfo & b) {
    return a.score_ > b.score_;
}

vector<ObjInfo> nms(vector<ObjInfo>& objs, float threshold = 0.7)
{
    std::sort(objs.begin(), objs.end(), CompareBBox);

    vector<ObjInfo> result;
    int objNum = objs.size();
    vector<bool> mask(objNum, false);

    for (int i = 0; i < objNum; i++)
    {
        if(!mask[i])
        {
            result.push_back(objs[i]);
            mask[i] = true;
        }
        for (int j = i + 1; j < objNum; j++)
        {
            if(mask[j])
            {
                continue;
            }
            float iouScore = iou(objs[i], objs[j]);
            if (iouScore > threshold)
            {
                mask[j] = true;
            }
        }
    }
    return result;
}

map<int, Action> g_action;

Net createNet(const string& model, const string& weight)
{
    g_action[1] = SMOKE;
    g_action[2] = CALL;
    g_action[3] = FACE;
    g_action[4] = PALM;
    return readNetFromCaffe(model, weight);
}

vector<ObjInfo> objDetection(Net& net, const Mat& img, float detectThresh)
{
    resetState();
    net.setInput(blobFromImage(img, 1.0 / 127.5, Size(300, 300), Scalar(127.5, 127.5, 127.5), true, false));
    Mat cvOut = net.forward();

    Mat detectionMat(cvOut.size[2], cvOut.size[3], CV_32F, cvOut.ptr<float>());
    vector<ObjInfo> objInfos;
    for (int i = 0; i < detectionMat.rows; i++)
    {
        float confidence = detectionMat.at<float>(i, 2);

        if (confidence > detectThresh)
        {
            ObjInfo objInfo;
            objInfo.action_ = g_action[(int)(detectionMat.at<float>(i, 1))];

            objInfo.xL_ = static_cast<int>(detectionMat.at<float>(i, 3) * img.cols);
            objInfo.yL_ = static_cast<int>(detectionMat.at<float>(i, 4) * img.rows);
            objInfo.xR_ = static_cast<int>(detectionMat.at<float>(i, 5) * img.cols);
            objInfo.yR_ = static_cast<int>(detectionMat.at<float>(i, 6) * img.rows);
            objInfos.push_back(objInfo);
        }
    }
    setState(objInfos);
//    return nms(objInfos);
    return objInfos;
}

bool getSmokeState()
{
    return g_isSomke;
}

bool getCallState()
{
    return g_isCall;
}

bool getFace()
{
    return g_isFace;
}

bool getPalState()
{
    return g_isPalm;
}
