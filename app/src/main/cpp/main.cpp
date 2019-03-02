#include <iostream>
#include "cameras.h"
#include <memory>
#include <unistd.h>
#include "total_flow.h"

void DrawImage(const Result& result, cv::Mat& frame);
void WriteResult(const Result& result, const std::string& timestamp, std::ofstream& ofstream);

std::string LastName;
int main(int argc, char** argv){
    assert(argc == 2);
    TotalFlow total_flow("../assets/path_root");
    std::shared_ptr<Camera> cam_ = std::make_shared<CameraOffLine>(argv[1]);
    std::ofstream ofstream("../assets/test.txt");
    std::shared_ptr<std::ofstream> shared_ptr(&ofstream, [](std::ofstream* os){
        std::cout << "os->close()" <<std::endl;
        os->close(); });
    assert(ofstream.is_open());
//    std::shared_ptr<Camera> cam_ = std::make_shared<CameraOnLine>();
    cv::Mat frame;
    Result result;

    for(size_t i = 0; i < 10; ++i){
        cam_->Read(frame);
        total_flow.Calibration(frame);
        total_flow.RegistFeature(frame);
    }

    while(true) {
        std::string image_path;
        try{
            image_path = cam_->Read(frame);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        catch (exception& e){
            std::cout << e.what() << std::endl;
            break;
        }
        if (frame.empty()) {
            usleep(1);
            continue;
        }

        total_flow.Run(frame, result);
        DrawImage(result, frame);
//        WriteResult(result, timestamp, ofstream);

        cv::imshow("UNTOUCH", frame);
        char ch = cv::waitKey(1);
        if (ch == 27)
            break;
    }

    return 0;
}


void DrawImage(const Result& result, cv::Mat& frame){
    std::string faceid;
    cv::Rect bbox;
    cv::Rect smoke_bbox;
    cv::Rect call_bbox;
    cv::Vec3f angles;
    std::vector<cv::Point2f> landmarks;

    int cal(0), dis(0), fat(0), smoke(0), call(0), abnorm(0);
    std::string DriverID        = "DriverID : ";
    std::string TrackingStatus  = "Tracking status : Tracking";
    std::string FaceStatus      = "Face status : ";
    std::string Roll            = "Roll : ";
    std::string Yaw             = "Yaw : ";
    std::string Pitch           = "Pitch : ";
    std::string DrowSiness      = "Drowsiness : ";
    std::string Attention       = "Attention : ";
    std::string Smoking         = "Smoking : ";
    std::string Calling         = "Calling : ";
    std::string IllegalID       = "Illegal ID";
    std::string Normal          = "Normal";
    std::string Warning         = "Warning";

    result.GetLandmarks(landmarks);
    result.GetFaceId(faceid);
    result.GetDistraction(dis, bbox);
    result.GetFatigue(fat, bbox);
    result.GetSmoke(smoke, smoke_bbox);
    result.GetCall(call, call_bbox);
    result.GetAbnormal(abnorm);
    result.GetCalibration(cal);
    result.GetAngles(angles);

    Pitch += std::to_string(angles[0]).substr(0, 6);
    Yaw += std::to_string(angles[1]).substr(0, 6);
    Roll += std::to_string(angles[2]).substr(0, 6);


//    std::cout << "fat : " << fat << std::endl;
//    std::cout << "dis: " << dis<< std::endl;
//    std::cout << "smoke: " << smoke<< std::endl;
//    std::cout << "call: " << call<< std::endl;

    cv::putText(frame, DriverID, cv::Point(20,20),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, TrackingStatus, cv::Point(20,40),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, FaceStatus, cv::Point(20,60),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, Roll, cv::Point(20,100),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, Yaw, cv::Point(20,120),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, Pitch, cv::Point(20,140),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, DrowSiness, cv::Point(20,200),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, Attention, cv::Point(20,220),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, Smoking, cv::Point(20,240),1,1,cv::Scalar(0,255,0));
    cv::putText(frame, Calling, cv::Point(20,260),1,1,cv::Scalar(0,255,0));

    if(faceid == "NoFace") {
        if (LastName.empty())
            cv::putText(frame, IllegalID, cv::Point(100, 20),1,1,cv::Scalar(0,255,0));
        else
            cv::putText(frame, LastName, cv::Point(100, 20),1,1,cv::Scalar(0,255,0));

        cv::putText(frame, "Not found", cv::Point(140, 60),1,1,cv::Scalar(0,255,0));
    } else {
        LastName = faceid;
        cv::putText(frame, LastName, cv::Point(100, 20),1,1,cv::Scalar(0,255,0));
        cv::putText(frame, "Found", cv::Point(140, 60),1,1,cv::Scalar(0,255,0));
    }
//    if(fat != 0)
//        cv::putText(frame, Warning, cv::Point(130, 200),1,1,cv::Scalar(0, 0, 255));
//    else
//        cv::putText(frame, Normal, cv::Point(130, 200),1,1,cv::Scalar(0, 255, 0));
//    if(dis != 0)
//        cv::putText(frame, Warning, cv::Point(120, 220),1,1,cv::Scalar(0, 0, 255));
//    else
//        cv::putText(frame, Normal, cv::Point(120, 220),1,1,cv::Scalar(0, 255, 0));
//    if(smoke != 0)
//        cv::putText(frame, Warning, cv::Point(110, 240),1,1,cv::Scalar(0, 0, 255));
//    else
//        cv::putText(frame, Normal, cv::Point(110, 240),1,1,cv::Scalar(0, 255, 0));
//    if(call != 0)
//        cv::putText(frame, Warning, cv::Point(100, 260),1,1,cv::Scalar(0, 0, 255));
//    else
//        cv::putText(frame, Normal, cv::Point(100, 260),1,1,cv::Scalar(0, 255, 0));

    if(fat == 1)
        cv::putText(frame, "Primary Warning", cv::Point(130, 200),1,1,cv::Scalar(0, 0, 255));
    else if(fat == 2)
        cv::putText(frame, "Secondary Warning", cv::Point(130, 200),1,1,cv::Scalar(0, 0, 255));
    else
        cv::putText(frame, Normal, cv::Point(130, 200),1,1,cv::Scalar(0, 255, 0));

    if(dis == 1)
        cv::putText(frame, "Primary Warning", cv::Point(120, 220),1,1,cv::Scalar(0, 0, 255));
    else if(dis == 2)
        cv::putText(frame, "Secondary Warning", cv::Point(120, 220),1,1,cv::Scalar(0, 0, 255));
    else
        cv::putText(frame, Normal, cv::Point(120, 220),1,1,cv::Scalar(0, 255, 0));

    if(smoke == 1)
        cv::putText(frame, "Primary Warning", cv::Point(110, 240),1,1,cv::Scalar(0, 0, 255));
    else if(smoke == 2)
        cv::putText(frame, "Secondary Warning", cv::Point(110, 240),1,1,cv::Scalar(0, 0, 255));
    else
        cv::putText(frame, Normal, cv::Point(110, 240),1,1,cv::Scalar(0, 255, 0));

    if(call == 1)
        cv::putText(frame, "Primary Warning", cv::Point(100, 260),1,1,cv::Scalar(0, 0, 255));
    else if(call == 2)
        cv::putText(frame, "Secondary Warning", cv::Point(100, 260),1,1,cv::Scalar(0, 0, 255));
    else
        cv::putText(frame, Normal, cv::Point(100, 260),1,1,cv::Scalar(0, 255, 0));

    if(smoke != 0) cv::rectangle(frame, smoke_bbox, cv::Scalar(0, 255, 0));
    if(call != 0) cv::rectangle(frame, call_bbox, cv::Scalar(0, 255, 0));

//    std::vector<cv::Point2f> mouth_vec;
//    if(!landmarks.empty()) {
//        for (size_t index = 60; index != 73; ++index) {
//            mouth_vec.push_back(landmarks[index]);
//        }
//    }
//    auto mouth_bbox = cv::boundingRect(mouth_vec);
//
//    if(mouth_bbox.area() > 10) {
//        mouth_bbox.x = mouth_bbox.x - 0.5f * mouth_bbox.width;
//        mouth_bbox.y = mouth_bbox.y - 0.5f * mouth_bbox.height;
//        mouth_bbox.width = mouth_bbox.width * 2;
//        mouth_bbox.height = mouth_bbox.height * 2;
//        cv::rectangle(frame, mouth_bbox, cv::Scalar(0, 0, 255));
//        if(smoke != 0) {
//            auto i = smoke_bbox & mouth_bbox;
//            float iou = static_cast<float>(i.area()) / (smoke_bbox.area() + mouth_bbox.area() - i.area());
//            std::cout << iou << std::endl;
//        }
//    }
    for(auto& p : landmarks){
        cv::circle(frame,p,1,cv::Scalar(0, 0, 255),1,1);
    }
}

void WriteResult(const Result &result, const std::string& timestamp, std::ofstream &ofstream) {
    std::string faceid;
    cv::Rect bbox;
    cv::Rect smoke_bbox;
    cv::Rect call_bbox;
    cv::Vec3f angles;

    int cal(0), dis(0), fat(0), smoke(0), call(0), abnorm(0);
    result.GetFaceId(faceid);
    result.GetDistraction(dis, bbox);
    result.GetFatigue(fat, bbox);
    result.GetSmoke(smoke, smoke_bbox);
    result.GetCall(call, call_bbox);
    result.GetAbnormal(abnorm);
    result.GetCalibration(cal);
    result.GetAngles(angles);

//    if(dis > 0)
//        ofstream << timestamp << " dis\n";
    if(fat > 0)
        ofstream << timestamp << " fatigue\n";
    if(smoke > 0)
        ofstream << timestamp << " smoke\n";
    if(call > 0)
        ofstream << timestamp << " call\n";
    if(abnorm > 0)
        ofstream << timestamp << " abnorm\n";
}