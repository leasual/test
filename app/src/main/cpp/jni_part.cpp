#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include "total_flow.h"
#include <vector>
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#include "dsm_transmit_jtt.h"
#include "DSM_JTT_API.h"

template <typename T>
std::string to_string(T value){
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

extern "C" {
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FindFeatures(JNIEnv*, jobject, jlong addrGray, jlong addrRgba);
JNIEXPORT jlong JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FindFeatures2(JNIEnv* jniEnv, jobject, jlong addrGray, jlong addrRgba,jlong);
JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_stop(JNIEnv* jniEnv, jobject);




TotalFlow* totalFlow = nullptr;
Result* result = nullptr;
Mat* newMat = nullptr;
cv::Rect* bbox;
JNIEXPORT jlong JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FindFeatures2(JNIEnv* jniEnv, jobject obj, jlong copyMat, jlong addrRgba,jlong time)
{
//    callback(jniEnv,obj);
//    cv::cvtColor(*(Mat*)addrGray,*newMat,cv::COLOR_GRAY2RGB);
//    cv::cvtColor(*(Mat*)addrRgba,*newMat,cv::COLOR_RGBA2RGB);
    if(totalFlow!= nullptr){
        totalFlow ->Run(*(Mat*)copyMat,*result, true,"user");
    }
//    if(totalFlow!= nullptr){
//        totalFlow ->Run(*newMat,*result, false,"chr");
//    }

    int cal, dis, fat, smoke, call, abnorm;
    std::string showFaceid = "name : ";
    std::string distration = "dis  : ";
    std::string fatigue    = "fat  : ";
    std::string showSmoke  = "smoke: ";
    std::string showCall   = "call : ";
    std::string showAbnorm = "abnm : ";
    std::string showCalibrt= "calibrate : ";
    std::string faceid;

    result->GetFaceId(faceid);
    result->GetDistraction(dis, *bbox);
    result->GetFatigue(fat, *bbox);
    result->GetSmoke(smoke, *bbox);
    result->GetCall(call, *bbox);
    result->GetAbnormal(abnorm);
    result->GetCalibration(cal);

//    callbackEnd(jniEnv,obj);

    cv::putText(*(Mat*)addrRgba, faceid, cv::Point(120,80),1,1,cv::Scalar(122,255,50));
    cv::putText(*(Mat*)addrRgba, distration+to_string(dis), cv::Point(120,110),1,1,cv::Scalar(122,255,50));
    cv::putText(*(Mat*)addrRgba, fatigue+to_string(fat), cv::Point(120,140),1,1,cv::Scalar(122,255,50));
    cv::putText(*(Mat*)addrRgba, showSmoke+to_string(smoke), cv::Point(120,170),1,1,cv::Scalar(122,255,50));
    cv::putText(*(Mat*)addrRgba, showCall+to_string(call), cv::Point(120,200),1,1,cv::Scalar(122,255,50));
    cv::putText(*(Mat*)addrRgba, showAbnorm+to_string(abnorm), cv::Point(120,230),1,1,cv::Scalar(122,255,50));
    cv::putText(*(Mat*)addrRgba, showCalibrt+to_string(cal), cv::Point(120,260),1,1,cv::Scalar(122,255,50));

//    LOGD("JNI distration -- %d",dis);
//    LOGD("JNI calibrate -- %d",cal);
//    LOGD("JNI fatigure -- %d",fat);
//    LOGD("JNI smoke -- %d",smoke);
//    LOGD("JNI call -- %d",call);
//    LOGD("JNI abnormal -- %d",abnorm);
//    LOGD("JNI faceid -- %s",faceid.c_str());
    return 0;
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_stop(JNIEnv* jniEnv, jobject){
    if(totalFlow != nullptr){
//        totalFlow ->Destroy();
    }
//    std::abort();
}

JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FindFeatures(JNIEnv* jniEnv, jobject obj, jlong addrGray, jlong addrRgba)
{
//    CreateHPSocketObjects();
//      OnCmdStart();
//    OnCmdSend();
//    OnCmdStop();
//    DestroyHPSocketObjects();
    DSM_JTT808_Start("112.64.116.41",20005,0);
    DSM_JTT808_Event_Callback(1,1,"http://220.194.43.233:8080/1.jpeg");
    sleep(100);
    DSM_JTT808_Stop(1);

    if(totalFlow == nullptr){
        totalFlow = new TotalFlow("/sdcard/Android/data/com.ut.sdk/files");
        result = new Result();
//        newMat = new Mat();
        bbox = new cv::Rect();
        LOGE("JNI abnormal -- init TotalFlow ----");
    }
}

}



