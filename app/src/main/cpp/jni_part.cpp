#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include "total_flow.h"
#include <vector>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

#include "dsm_transmit_jtt.h"
#include "DSM_JTT_API.h"

template<typename T>
std::string to_string(T value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

extern "C" {
JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures(JNIEnv *, jobject, jlong addrGray,
                                                               jlong addrRgba);
JNIEXPORT jintArray JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject,
                                                                jlong addrGray, jlong addrRgba,
                                                                jboolean regis);
JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject);


TotalFlow *totalFlow = nullptr;
Result *result = nullptr;
cv::Rect *bboxd;
cv::Rect *bboxf;
cv::Rect *bboxs;
cv::Rect *bboxc;
JNIEXPORT jintArray JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject obj,
                                                                jlong copyMat, jlong addrRgba,
                                                                jboolean regis) {
    jintArray re1;
    jint *index2;
    if (totalFlow != nullptr) {
        totalFlow->Run(*(Mat *) copyMat, *result, regis, "user");

        int cal, dis, fat, smoke, call, abnorm;
        std::string showFaceid = "name : ";
        std::string distration = "dis  : ";
        std::string fatigue = "fat  : ";
        std::string showSmoke = "smoke: ";
        std::string showCall = "call : ";
        std::string showAbnorm = "abnm : ";
        std::string showCalibrt = "calibrate : ";
        std::string faceid;

        result->GetFaceId(faceid);
        result->GetDistraction(dis, *bboxd);
        result->GetFatigue(fat, *bboxf);
        result->GetSmoke(smoke, *bboxs);
        result->GetCall(call, *bboxc);
        result->GetAbnormal(abnorm);
        result->GetCalibration(cal);

        re1 = jniEnv->NewIntArray(5);
        index2 = jniEnv->GetIntArrayElements(re1, NULL);
        index2[0] = dis;
        index2[1] = fat;
        index2[2] = smoke;
        index2[3] = call;
        index2[4] = abnorm;
        if(smoke != 0 ){
            cv::rectangle(*(Mat*)addrRgba,*bboxs,cv::Scalar(255,0,0),2);
        }
        if(call != 0 ){
            cv::rectangle(*(Mat*)addrRgba,*bboxc,cv::Scalar(255,0,0),2);
        }
//        if( fat != 0){
//            cv::rectangle(*(Mat*)addrRgba,*bboxf,cv::Scalar(255,0,0),2);
//        }
//        if(dis != 0){
//            cv::rectangle(*(Mat*)addrRgba,*bboxd,cv::Scalar(255,0,0),2);
//        }



    }

    if (index2 != nullptr) {
        jniEnv->ReleaseIntArrayElements(re1, index2, 0);
    }


//    cv::putText(*(Mat*)addrRgba, faceid, cv::Point(120,80),1,1,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, distration+to_string(dis), cv::Point(120,110),1,1,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, fatigue+to_string(fat), cv::Point(120,140),1,1,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, showSmoke+to_string(smoke), cv::Point(120,170),1,1,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, showCall+to_string(call), cv::Point(120,200),1,1,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, showAbnorm+to_string(abnorm), cv::Point(120,230),1,1,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, showCalibrt+to_string(cal), cv::Point(120,260),1,1,cv::Scalar(122,255,50));

    return re1;
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject) {
    if (totalFlow != nullptr) {
        delete totalFlow;
    }
    delete result;
    delete bboxd;
    delete bboxs;
    delete bboxf;
    delete bboxc;
//    std::abort();
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures(JNIEnv *jniEnv, jobject obj,
                                                               jlong addrGray, jlong addrRgba) {
//    CreateHPSocketObjects();
//      OnCmdStart();
//    OnCmdSend();
//    OnCmdStop();
//    DestroyHPSocketObjects();

//    DSM_JTT808_Start("112.64.116.41",20005,0);
//    DSM_JTT808_Event_Callback(1,1,"http://220.194.43.233:8080/1.jpeg");
//    sleep(5);
//    DSM_JTT808_Stop(1);

    if (totalFlow == nullptr) {
        totalFlow = new TotalFlow("/sdcard/Android/data/com.ut.sdk/files");
        result = new Result();
//        newMat = new Mat();
        bboxd = new cv::Rect();
        bboxf = new cv::Rect();
        bboxs = new cv::Rect();
        bboxc = new cv::Rect();
        LOGE("JNI abnormal -- init TotalFlow ----");
    }
}

}



