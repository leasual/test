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
Java_com_op_dm_ui_DetectActitvity_Cali(JNIEnv *, jobject, jlong addrGray,
                                                               jint index);
JNIEXPORT void JNICALL
Java_com_op_dm_ui_DetectActitvity_Detect(JNIEnv *, jobject, jlong addrGray,
                                                       jint index);

JNIEXPORT void JNICALL
Java_com_op_dm_ui_DetectActitvity_FindFeatures(JNIEnv *, jobject, jlong addrGray,
                                                               jint index);
JNIEXPORT jintArray JNICALL
Java_com_op_dm_ui_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject,
                                                                jlong addrGray, jlong addrRgba,
                                                                jboolean regis,jboolean);
JNIEXPORT void JNICALL
Java_com_op_dm_ui_DetectActitvity_stop(JNIEnv *jniEnv, jobject);
JNIEXPORT jboolean JNICALL
Java_com_op_dm_ui_DetectActitvity_CHECK(JNIEnv *jniEnv, jobject,jstring);

TotalFlow *totalFlow = nullptr;
Result *result = nullptr;
cv::Rect *bboxd;
cv::Rect *bboxf;
cv::Rect *bboxs;
cv::Rect *bboxc;
bool * caliDone;
int * featureNum;

JNIEXPORT void JNICALL
Java_com_op_dm_ui_DetectActitvity_Cali(JNIEnv * jniEnv, jobject obj, jlong copyMat,
                                                       jint index){
    LOGE("before Calibration is %d",*caliDone);
    bool done = *caliDone;
    if(totalFlow != nullptr && !done){
        LOGE(" Calibration");
        if(totalFlow->Calibration(*(Mat *) copyMat)){
            *caliDone = true;
            jclass cl = jniEnv ->FindClass("com/op/dm/ui/DetectActitvity");
            jmethodID meth = jniEnv->GetMethodID(cl,"caliDone","()V");
            jniEnv->CallVoidMethod(obj,meth);
            LOGE(" Calibration is %d",*caliDone);
        }
    }
}

JNIEXPORT void JNICALL
Java_com_op_dm_ui_DetectActitvity_Detect(JNIEnv * jniEnv, jobject obj, jlong copyMat,
                                                         jint index){
    LOGE("before!!! RegistFeature is %d",*featureNum);
    bool ndone = (*featureNum < 25);
    if(totalFlow != nullptr && (*caliDone) && ndone){
        LOGE(" RegistFeature");
        if(totalFlow->RegistFeature(*(Mat *) copyMat)){
            *featureNum = *featureNum +1;
            LOGE(" RegistFeature is %d",*featureNum);
            if(*featureNum >= 25){
                jclass cl = jniEnv ->FindClass("com/op/dm/ui/DetectActitvity");
                jmethodID meth = jniEnv->GetMethodID(cl,"RegistDone","()V");
                jniEnv->CallVoidMethod(obj,meth);
            }
        }

    }

}


JNIEXPORT jintArray JNICALL
Java_com_op_dm_ui_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject obj,
                                                                jlong copyMat, jlong addrRgba,
                                                                jboolean regis, jboolean picture) {
    jintArray re1;
    jint *index2;
    if (totalFlow != nullptr && (*caliDone) && ((*featureNum) >= 25)) {
        totalFlow->Run(*(Mat *) copyMat, *result);
        totalFlow->isSave = picture == JNI_TRUE;
        int cal, dis, fat, smoke, call, abnorm, unknown;
        std::string showFaceid = "name : ";
        std::string distration = "dis  : ";
        std::string fatigue = "fat  : ";
        std::string showSmoke = "smoke: ";
        std::string showCall = "call : ";
        std::string showAbnorm = "abnm : ";
        std::string showCalibrt = "calibrate : ";
        std::string faceid;

        result->GetFaceId(faceid);
        unknown = 0;
        if(faceid == "UnknowFace")
            unknown = 2;
        else
            unknown = 0;
        result->GetDistraction(dis, *bboxd);
        result->GetFatigueFirst(fat, *bboxf);
        result->GetSmoke(smoke, *bboxs);
        result->GetCall(call, *bboxc);
        result->GetAbnormal(abnorm);

        LOGE(" fat ,dis, id -------- %d,%d, %s",fat,dis,faceid.data());


//        if(regis && !faceid.empty()){
//            LOGE(" face id -------- %s",faceid.data());
//            jclass cl = jniEnv ->FindClass("org/opencv/samples/tutorial2/DetectActitvity");
//            jmethodID meth = jniEnv->GetMethodID(cl,"RegistDone","()V");
//            jniEnv->CallVoidMethod(obj,meth);
//        }

        re1 = jniEnv->NewIntArray(6);
        index2 = jniEnv->GetIntArrayElements(re1, NULL);
        index2[0] = dis;
        index2[1] = fat;
        index2[2] = smoke;
        index2[3] = call;
        index2[4] = abnorm;
        index2[5] = unknown;
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
//    cv::imwrite("/sdcard/Android/data/com.ut.sdk/files/" + to_string(copyMat) + ".jpg",*(Mat *) copyMat);
    cv::putText(*(Mat *) copyMat, to_string(totalFlow->isSave), cv::Point(220,80),1,1,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, to_string(totalFlow->keep_running_flag_), cv::Point(220,130),1,1,cv::Scalar(122,255,50));


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
Java_com_op_dm_ui_DetectActitvity_stop(JNIEnv *jniEnv, jobject) {
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
JNIEXPORT jboolean JNICALL
Java_com_op_dm_ui_DetectActitvity_CHECK(JNIEnv *jniEnv, jobject, jstring mac){
    string ss = "00:08";
    string ss2 = "00:08";
    string ss3 = "00:08";

    const char *cstr = jniEnv->GetStringUTFChars(mac, NULL);

    string str = string(cstr);
    jniEnv->ReleaseStringUTFChars(mac, cstr);
    if(ss == str || ss2 == str || ss3 == str){
        LOGE(" equal -----------------%s    %s",ss.data(),str.data());
        return JNI_TRUE;
    } else
        return JNI_FALSE;

}

JNIEXPORT void JNICALL
Java_com_op_dm_ui_DetectActitvity_FindFeatures(JNIEnv *jniEnv, jobject obj,
                                                               jlong addrGray, jint index) {
//    CreateHPSocketObjects();
//      OnCmdStart();
//    OnCmdSend();
//    OnCmdStop();
//    DestroyHPSocketObjects();

//    DSM_JTT808_Start("112.64.116.41",20005,0);
//    DSM_JTT808_Event_Callback(1,1,"http://220.194.43.233:8080/1.jpeg");
//    sleep(5);
//    DSM_JTT808_Stop(1);


    srand(time(0));
    if (totalFlow == nullptr) {
        totalFlow = new TotalFlow("/sdcard/Android/data/com.ut.sdk/files");
//        string path = "/storage/sdcard1/img"+ to_string(index) + "/";

        string path = "/sdcard/img"+ to_string(index) + "/";
        totalFlow->path = path;
        totalFlow-> pathDis = path + "distract/";
        totalFlow-> pathFat = path +"fat/";
        totalFlow-> pathCall = path +"call/";
        totalFlow-> pathSmoke =path + "smoke/";
        totalFlow-> pathAbnormal =path + "abnormal/";
        totalFlow-> pathUnknow =path + "unknown/";
//        time_t nSrc;
//        nSrc = addrGray - time(NULL);
//        totalFlow->time_diff = nSrc;

        totalFlow->SetSpeed(80);
        result = new Result();
//        newMat = new Mat();
        bboxd = new cv::Rect();
        bboxf = new cv::Rect();
        bboxs = new cv::Rect();
        bboxc = new cv::Rect();
        caliDone = new bool;
        *caliDone = false;
        featureNum = new int;
        *featureNum = 0;
        LOGE("JNI abnormal -- init TotalFlow ----");
    }
}





}



