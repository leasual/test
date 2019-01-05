//
// Created by public on 18-10-17.
//

#ifndef OPENFACE_POSEEYES_H
#define OPENFACE_POSEEYES_H
#endif //OPENFACE_XPSEYE_H

#include <LandmarkCoreIncludes.h>
#include <GazeEstimation.h>
#include <Visualizer.h>
#include <VisualizationUtils.h>

using namespace std;
using namespace cv;
using namespace LandmarkDetector;


typedef struct StructInfos {
    float cx, cy;
    float fx, fy;

    int makeinfos(int width, int height) {
        cx = width / 2.0f;
        cy = height / 2.0f;

        fx = 500.0f * (float(width) / 640.0f);
        fy = 500.0f * (float(height) / 480.0f);
        fx = (fx + fy) / 2.0f;
        fy = fx;
        return 0;
    }

} StructInfos;


class PoseEye {
public:
    ~PoseEye();

    PoseEye(vector<string> &aruments, bool visual = true, bool useclnf = true);

    cv::Vec2f estimate(vector<pair<float, float>> &ldmarklist,
                       cv::Mat &rgb_image);

    int GetHeadPose(cv::Vec6d &pose);

    int GetGazeDirect(cv::Vec2f &gzAngle, cv::Point3f &gzD0, cv::Point3f &gzD1);

    int DrawLandmarkGaze(cv::Mat &rgbframe);


private:
    bool doVisualize;
    bool withClnfs;
    StructInfos IfObt;
    CLNF *face_model;

    FaceModelParameters *pParameters;
    Utilities::Visualizer *visualizer;
    cv::Vec2f gaze_angle;
    cv::Vec6d pose_estimate;
    cv::Point3f gaze_direction0;
    cv::Point3f gaze_direction1;

private:
    int CheckDataS();

    int fillLandmarks(vector<pair<float, float>> &landmarks);
};


