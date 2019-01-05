#include "GazeEstimation.h"
//#include "RotationHelpers.h"
//#include "opencv2/calib3d/calib3d.hpp"
#include <iostream>
#include "PoseEyes.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "LandmarkDetectorUtils.h"
#include "LandmarkDetectorFunc.h"

using namespace std;





PoseEye::PoseEye(vector<string> & arguments,bool visual,bool useclnf)
{
    face_model=NULL;
    pParameters=NULL;
    visualizer=NULL;

     doVisualize=visual;
    withClnfs=useclnf;

    pParameters= new FaceModelParameters(arguments);
    face_model=new CLNF(pParameters->model_location);
    //face_detector_mtcnn=new FaceDetectorMTCNN
            // (pParameters->mtcnn_face_detector_location);
    visualizer=new Utilities::Visualizer(arguments);
    visualizer->setStatus(true,false,false,false);
    //face_model->patch_experts.WrtCtViews();
//    face_model->readctviews();
    CheckDataS();
}

int PoseEye::CheckDataS()
{
    if (!face_model->loaded_successfully)
    {
        cout << "ERROR: Could not load the landmark detector" << endl;
        return 1;
    }

    if (!face_model->eye_model)
    {
        cout << "WARNING: no Action Unit models found\" << endl;\n"
                "\t}WARNING: no eye model found" << endl;
    }
    return 0;
}
PoseEye::~PoseEye()
{
    delete face_model;
    delete pParameters;
    delete visualizer;
    //delete face_detector_mtcnn;
}


int PoseEye::fillLandmarks(vector<pair<float,float>>& landmarks)
{
    cv::Mat_<float>&markmat=face_model->detected_landmarks;
    int nlandmark=max(markmat.rows,markmat.cols)/2;
    assert(int(landmarks.size())==nlandmark);

    if(markmat.rows==1)
    {
        for(size_t idx=0;idx<landmarks.size();idx++)
        {
            markmat.at<float>(0,idx)=landmarks[idx].first;
            markmat.at<float>(0,idx+nlandmark)=landmarks[idx].second;
        }
    }
    else
    {
        for(size_t idx=0;idx<landmarks.size();idx++)
        {
            markmat.at<float>(idx,0)=landmarks[idx].first;
            markmat.at<float>(idx+nlandmark,0)=landmarks[idx].second;
        }
    }

    return 0;
}

cv::Vec2f PoseEye::estimate( vector<pair<float,float>>&ldmarklist,
                  cv::Mat &rgb_image)
{
    if(rgb_image.empty())
        return -1;
    if(ldmarklist.size()!=face_model->pdm.NumberOfPoints())
        return -1;

    cv::Mat_<uchar> graydata;
    cv::cvtColor(rgb_image,graydata,cv::COLOR_BGR2GRAY);
    IfObt.makeinfos(rgb_image.cols,rgb_image.rows);


    fillLandmarks(ldmarklist);
    int64 t1=getTickCount();
    face_model->DetectLandmarksfx(graydata,*pParameters,withClnfs);
    int64 t2=getTickCount();
//    cout<<"detect time:"<<(t2-t1)/getTickFrequency()<<endl;

    pose_estimate =GetPose(*face_model, IfObt.fx, IfObt.fy, IfObt.cx, IfObt.cy);


    GazeAnalysis::EstimateGaze(*face_model, gaze_direction0,
                               IfObt.fx, IfObt.fy,
                               IfObt.cx, IfObt.cy, true);
    GazeAnalysis::EstimateGaze(*face_model, gaze_direction1,
                               IfObt.fx, IfObt.fy,
                               IfObt.cx, IfObt.cy, false);


    gaze_angle = GazeAnalysis::GetGazeAngle(gaze_direction0, gaze_direction1);

    if(doVisualize)
    {
        visualizer->SetImage(rgb_image, IfObt.fx, IfObt.fy, IfObt.cx, IfObt.cy);

        visualizer->SetObservationLandmarks(face_model->detected_landmarks, 1.0,
                                            face_model->GetVisibilities11());
        visualizer->SetObservationPose(pose_estimate, 1.0);

        visualizer->SetObservationGaze(gaze_direction0, gaze_direction1,
                                       CalculateAllEyeLandmarks(*face_model),
                                       Calculate3DEyeLandmarks(*face_model, IfObt.fx, IfObt.fy, IfObt.cx, IfObt.cy), face_model->detection_certainty);
        visualizer->ShowObservation();
    }

    return this->gaze_angle;


    return 0;
}


int PoseEye::DrawLandmarkGaze(cv::Mat & rgbframe)
{
    visualizer->SetImage(rgbframe, IfObt.fx, IfObt.fy, IfObt.cx, IfObt.cy);

//    visualizer->SetObservationLandmarks1(rgbframe, face_model->detected_landmarks,
//                                        face_model->GetVisibilities11());
//    visualizer->SetObservationPose1(rgbframe, pose_estimate);

    visualizer->SetObservationGaze1(rgbframe, gaze_direction0, gaze_direction1,
                                   CalculateAllEyeLandmarks(*face_model),
    Calculate3DEyeLandmarks(*face_model, IfObt.fx, IfObt.fy, IfObt.cx, IfObt.cy));

    return 0;
}



int PoseEye::GetHeadPose(cv::Vec6d& pose)
{
    for(int i=0;i<6;i++)
        pose(i)=pose_estimate(i);
    return 0;
}


int PoseEye::GetGazeDirect(cv::Vec2f &gzAngle,
                           cv::Point3f &gzD0, cv::Point3f &gzD1)
{
    gzAngle(0)=gaze_angle(0);
    gzAngle(1)=gaze_angle(1);
    gzD0.x=gaze_direction0.x;
    gzD0.y=gaze_direction0.y;
    gzD0.z=gaze_direction0.z;
    gzD1.x=gaze_direction1.x;
    gzD1.y=gaze_direction1.y;
    gzD1.z=gaze_direction1.z;
    return 0;
}


