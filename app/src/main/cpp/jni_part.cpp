#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include "Util.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <thread>
#include <opencv2/opencv.hpp>


#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))

using namespace std;
using namespace cv;

template<typename T>
std::string to_string(T value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

extern "C" {
JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures(JNIEnv *, jobject, jlong addrGray,
                                                               jstring dir);
JNIEXPORT jintArray JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject,
                                                                jlong addrGray, jlong addrRgba,
                                                                jboolean regis,jboolean);
JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject);
JNIEXPORT jboolean JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_CHECK(JNIEnv *jniEnv, jobject,jstring);

cv::Rect *bboxd;
cv::Rect *bboxf;
cv::Rect *bboxs;
cv::Rect *bboxc;
JNIEXPORT jintArray JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject obj,
                                                                jlong copyMat, jlong addrRgba,
                                                                jboolean regis, jboolean picture) {
    jintArray re1;
    jint *index2;

    return re1;
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject) {

//    std::abort();
}


string js2string(JNIEnv *env, jstring jStr){
  const char *cstr = env->GetStringUTFChars(jStr, NULL);
  string str = string(cstr);
  env->ReleaseStringUTFChars(jStr, cstr);
  return str;
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures(JNIEnv *jniEnv, jobject obj,
                                                               jlong addrGray, jstring dir) {

    struct timeval tv;
    gettimeofday(&tv, NULL);
    long milli = tv.tv_usec / 1000;

    struct tm p;
    time_t nSrc;
    nSrc = time(NULL);

    p = *localtime(&nSrc);
    char str[80];
    strftime(str, 1000, "%Y_%m_%d_%H_%M_%S", &p);
    string path =js2string(jniEnv,dir) + "/" + str + "_" + to_string(milli) + ".jpg";
    cv::imwrite(path,*(Mat*)addrGray);
    LOGE("jni parh %s",path.c_str());

}





}



