#include <jni.h>
#include <string>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <android/asset_manager_jni.h>
#include <android/native_window_jni.h>
#include <thread>

using namespace std;
using namespace cv;

extern "C"
{
//JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FindFeatures(JNIEnv*, jobject, jlong addrGray, jlong addrRgba);
//
//JNIEXPORT void JNICALL Java_org_opencv_samples_tutorial2_Tutorial2Activity_FindFeatures(JNIEnv*, jobject, jlong addrGray, jlong addrRgba)
//{
//    Mat& mGr  = *(Mat*)addrGray;
//    Mat& mRgb = *(Mat*)addrRgba;
//    vector<KeyPoint> v;
//
//    Ptr<FeatureDetector> detector = FastFeatureDetector::create(50);
//    detector->detect(mGr, v);
//    for( unsigned int i = 0; i < v.size(); i++ )
//    {
//        const KeyPoint& kp = v[i];
//        circle(mRgb, Point(kp.pt.x, kp.pt.y), 10, Scalar(255,0,0,255));
//    }
//}

void JNICALL Java_com_untouch_androidjnionpencv_MainActivity2_salt(JNIEnv *env, jobject instance,
                                                                  jlong matAddrGray,
                                                                  jint nbrElem) {
    Mat &mGr = *(Mat *) matAddrGray;
    for (int k = 0; k < nbrElem; k++) {
        int i = rand() % mGr.cols;
        int j = rand() % mGr.rows;
        mGr.at<uchar>(j, i) = 255;
    }
}

//bool Init(const std::string &configure_path);
//public native boolean Init(String configure_path);
bool JNICALL
Java_com_untouch_androidjnionpencv_MainActivity_Init(JNIEnv *env, jobject instance,
                                                     jstring configure_path) {

    return true;
}
//bool Run(cv::Mat& frame, bool regist, string name);
//public native boolean run(String configure_path);
bool JNICALL
Java_com_untouch_androidjnionpencv_MainActivity_run(JNIEnv *env, jobject instance,
                                                    jlong frame,
                                                     jboolean regist,
                                                     jstring name) {
    Mat &mGr = *(Mat *) frame;
    return true;
}


//bool FeatureExtract(cv::Mat &image, cv::Mat &feature);
//public native boolean FeatureExtract(Mat image, Mat feature);
bool JNICALL
Java_com_untouch_androidjnionpencv_MainActivity2_featureExtract(JNIEnv *env,
                                                               jobject instance,
                                                               jlong image,
                                                               jlong feature) {

    Mat &tmp = *(Mat *) image;
    *(Mat *) feature = tmp.clone();
    return true;
}

//float FeatureMatch(const cv::Mat &feature1, const cv::Mat &feature2);
//public native float FeatureMatch(Mat feature1, Mat feature2);

float JNICALL
Java_com_untouch_androidjnionpencv_MainActivity_FeatureMatch(JNIEnv *env,
                                                             jobject instance,
                                                             jlong feature1,
                                                             jlong feature2) {
    return 1.0f;
}

//void Run(const cv::Mat &image, STATE &state, RESULT &result);
//public native void Run(Mat image);
//int JNICALL
//Java_com_untouch_androidjnionpencv_MainActivity_Run(JNIEnv *env,
//                                                    jobject instance,
//                                                    jlong image) {
//    return 1;
//
//}

//#src/main/cpp/message/src/atimer.cpp
//#src/main/cpp/message/src/boostIOService.cpp
//#src/main/cpp/message/src/Config.cpp
//#src/main/cpp/message/src/DSM_Socket.cpp
//src/main/cpp/message/src/deviceStatus.cpp
//src/main/cpp/message/src/dsm_transmit_jtt.cpp
//        src/main/cpp/message/src/dsmapp_sms_custom_jtt.cpp
//        src/main/cpp/message/src/dsmapp_srv_jtt.cpp
//        src/main/cpp/message/src/dsmapp_srvinteraction_jtt.cpp
//        src/main/cpp/message/src/hc.cpp
//        src/main/cpp/message/src/http_client.cpp
//        src/main/cpp/message/src/log.cpp
//        src/main/cpp/message/src/main.cpp
//#src/main/cpp/message/src/main_client.cpp
//#src/main/cpp/message/src/main_server.cpp
//#src/main/cpp/message/src/MSGHandle.cpp
//#src/main/cpp/message/src/MSGReceive.cpp
//#src/main/cpp/message/src/MSGSend.cpp
//#src/main/cpp/message/src/others.cpp
//#src/main/cpp/message/src/test.cpp






// Alot of stuff depends on the m_frame_buffer being loaded
// this is done in SetNativeWindow




}
