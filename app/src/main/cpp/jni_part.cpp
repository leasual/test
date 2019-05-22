#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include "total_flow.h"
#include <vector>
#include "dsm_jtt808_api.h"
#include "ut_timer.h"
#include <opencv2/highgui.hpp>
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))


void sendData(string& szFilePath, euFileType type,euDSMAlarmType warn);

void doi(euDSMAlarmType warn);

template<typename T>
std::string to_string(T value) {
    std::ostringstream os;
    os << value;
    return os.str();
}

extern "C" {


JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_Cali(JNIEnv *, jobject, jlong addrGray,
                                                       jint index);
JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_Detect(JNIEnv *, jobject, jlong addrGray,
                                                         jint index);

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures(JNIEnv *, jobject, jlong addrGray,
                                                               jint index);
JNIEXPORT jintArray JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject,
                                                                jlong addrGray, jlong addrRgba,
                                                                jboolean regis, jboolean);
JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject);

JNIEXPORT jboolean JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_OnMessage(JNIEnv *jniEnv, jobject, jlong lati,
                                                            jlong alti ,jint,jshort,jboolean);

JNIEXPORT jboolean JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_CHECK(JNIEnv *jniEnv, jobject, jstring);

TotalFlow *totalFlow = nullptr;
Result *result = nullptr;
cv::Rect *bboxd;
cv::Rect *bboxf;
cv::Rect *bboxs;
cv::Rect *bboxc;
bool *caliDone;
int *featureNum;
chrono::steady_clock::time_point old;
long *lati;
long *longi;
unsigned int * hei;
unsigned short * speed;

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_Cali(JNIEnv *jniEnv, jobject obj, jlong copyMat,
                                                       jint index) {
//    LOGE("before Calibration is %d", *caliDone);
    bool done = *caliDone;
    if (totalFlow != nullptr && !done) {
//        LOGE(" Calibration");
        if (totalFlow->Calibration(*(Mat *) copyMat)) {
            *caliDone = true;
            jclass cl = jniEnv->FindClass("org/opencv/samples/tutorial2/DetectActitvity");
            jmethodID meth = jniEnv->GetMethodID(cl, "caliDone", "()V");
            jniEnv->CallVoidMethod(obj, meth);
//            LOGE(" Calibration is %d", *caliDone);

        }
    }
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_Detect(JNIEnv *jniEnv, jobject obj, jlong copyMat,
                                                         jint index) {
//    LOGE("before!!! RegistFeature is %d", *featureNum);
    bool ndone = (*featureNum < 25);
    if (totalFlow != nullptr && (*caliDone) && ndone) {
//        LOGE(" RegistFeature");
        if (totalFlow->RegistFeature(*(Mat *) copyMat)) {
            *featureNum = *featureNum + 1;
//            LOGE(" RegistFeature is %d", *featureNum);
            if (*featureNum >= 25) {
                jclass cl = jniEnv->FindClass("org/opencv/samples/tutorial2/DetectActitvity");
                jmethodID meth = jniEnv->GetMethodID(cl, "RegistDone", "()V");
                jniEnv->CallVoidMethod(obj, meth);
            }
        }
    }

}


JNIEXPORT jintArray JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject obj,
                                                                jlong copyMat, jlong addrRgba,
                                                                jboolean regis, jboolean picture) {
    jintArray re1;
    jint *index2;
    if (totalFlow != nullptr && (*caliDone) && ((*featureNum) >= 25)) {

//        UT_TRACE("before Run  ");
        totalFlow->Run(*(Mat *) copyMat, *result);
        totalFlow->isSave = picture == JNI_TRUE;
        int yawn, dis, fat, smoke, call, abnorm, unknown;
        std::string showFaceid = "name : ";
        std::string distration = "dis  : ";
        std::string fatigue = "fat  : ";
        std::string showSmoke = "smoke: ";
        std::string showCall = "call : ";
        std::string showAbnorm = "abnm : ";
        std::string showCalibrt = "calibrate : ";
        std::string faceid;

        cv::Rect face_bbox;
        std::vector<cv::Point2f> landmarks;

        result->GetFaceId(faceid);
//        LOGE(" face id -------- %s", faceid.data());
        unknown = 0;
        if (faceid == "UnknowFace")
            unknown = 2;
        else
            unknown = 0;
        result->GetDistraction(dis, *bboxd);//左右
        result->GetFatigueFirst(fat, *bboxf);//分神
        result->GetSmoke(smoke, *bboxs);
        result->GetCall(call, *bboxc);
        result->GetAbnormal(abnorm);
        result->GetYawn(yawn);//haqi
        result->GetLandmarks(landmarks);
        result->GetFaceBbox(face_bbox);
//        LOGE(" fat dis yawn   -------- %d, %d ,%d ",fat, dis, yawn);


        if(picture && (fat != 0 || dis!= 0 || call != 0|| smoke != 0|| abnorm != 0 || yawn != 0)){

            auto diff = chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - old);
            if(diff.count() > 10000L){
                euDSMAlarmType warn = euDsmAlarmInit;
                if(fat != 0 || yawn != 0 )
                    warn = euFatigue;
                if(call != 0)
                    warn = euCall;
                if(smoke != 0)
                    warn = euSmoking;
                if(dis != 0)
                    warn = euDistract;
                thread t(doi,warn);
                t.detach();
            }
        }

//        UT_TRACE("before Run result ");

        re1 = jniEnv->NewIntArray(7);
        index2 = jniEnv->GetIntArrayElements(re1, NULL);
        index2[0] = dis;
        index2[1] = fat;
        index2[2] = smoke;
        index2[3] = call;
        index2[4] = abnorm;
        index2[5] = unknown;
        index2[6] = yawn;
        if (smoke != 0) {
            cv::rectangle(*(Mat *) addrRgba, *bboxs, cv::Scalar(255, 1, 1), 1);
        }
        if (call != 0) {
             LOGE(" call box   --------  %d ",bboxc->width);
            cv::rectangle(*(Mat *) addrRgba, *bboxc, cv::Scalar(255, 1, 1), 1);
        }
//        if( fat != 0){
//            cv::rectangle(*(Mat*)addrRgba,*bboxf,cv::Scalar(255,0,0),2);
//        }
//        if(dis != 0){
//            cv::rectangle(*(Mat*)addrRgba,*bboxd,cv::Scalar(255,0,0),2);
//        }
//        if(!face_bbox.empty())
//            cv::rectangle(*(Mat*)addrRgba, face_bbox, cv::Scalar(255,0,0), 1, 1);

        if(landmarks.size() > 0){
//            for (auto &p : landmarks) {
//                cv::circle(*(Mat*)addrRgba, p, 1, cv::Scalar(0, 0, 255), 1, 1);
//            }

        }


    }

    if (index2 != nullptr) {
        jniEnv->ReleaseIntArrayElements(re1, index2, 0);
    }

//    cv::putText(*(Mat *) addrRgba, to_string(totalFlow->isSave), cv::Point(220, 80), 1, 1,
//                cv::Scalar(122, 255, 50));
//    cv::putText(*(Mat *) copyMat, to_string(totalFlow->isSave), cv::Point(220, 80), 1, 1,
//                cv::Scalar(122, 255, 50));
//    cv::putText(*(Mat*)addrRgba, to_string(totalFlow->keep_running_flag_), cv::Point(220,130),1,1,cv::Scalar(122,255,50));


    return re1;
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject) {
    if (totalFlow != nullptr) {
        delete totalFlow;
        totalFlow = nullptr;
    }
    delete result;
    delete bboxd;
    delete bboxs;
    delete bboxf;
    delete bboxc;
    CDsmJTT808_API::GetInstance()->StopTimer();
    CDsmJTT808_API::GetInstance()->Destroy();
//    std::abort();
}

string js2string(JNIEnv *env, jstring jStr){
  const char *cstr = env->GetStringUTFChars(jStr, NULL);
  string str = string(cstr);
  env->ReleaseStringUTFChars(jStr, cstr);
  return str;
}

JNIEXPORT jboolean JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_CHECK(JNIEnv *jniEnv, jobject, jstring mac) {
//    return JNI_FALSE;

        string mod = js2string(jniEnv, mac);
        string sim = "0" + mod;
    UT_TRACE("sim  model  is ! %s , %s " ,sim.c_str(),mod.c_str());
    string s = "106.14.186.44";

    CConfigFileReader::GetInstance()->LoadFromFile( "/sdcard/Android/data/com.ut.sdk/files/dsm_jtt808.cfg");
    CDSMLog::GetInstance()->InitialiseLog4z("/sdcard/Android/data/com.ut.sdk/files/dsm_log.cfg");

    CDsmJTT808_API::GetInstance()->Start((char*)sim.c_str(),(char*)mod.c_str(),(char*)s.c_str(), 7000);

//    if (!CDsmJTT808_API::GetInstance()->Inialise((char*)sim.c_str(), (char*)mod.c_str(),(char*)s.c_str(),7000)) {
//        UT_FATAL("Inialise failed!");
//        return JNI_FALSE;
//    }
//
//
//    if (!CDsmJTT808_API::GetInstance()->Connect()) {
//        UT_FATAL("Connect failed!");
//        return JNI_FALSE;
//    }

    lati = new  long;
    longi = new  long;
    *lati = 0L;
    *longi = 0L;
    speed = new unsigned short;
    hei = new unsigned int;
    *speed = 10;
    *hei = 10;

//    VideoCapture cap(1);
//    if (!cap.isOpened())
//    {
//        LOGE("is opened ");
//        return JNI_FALSE;
//    }
//
//    Mat frame;
//    // 按Q键退出时，键盘需要调为英文模式
//    while(1) {
//        // 通过流操作符把视频转化为一帧帧图片
//        cap >> frame;
//        LOGE("is frame col %d", frame.cols);
//        int  i = rand();
//        imwrite("/storage/sdcard1/img101/temp/"+ to_string(i) +".jpg",frame);
//    }

//    VideoWriter video("/storage/sdcard1/img101/test.avi", CV_FOURCC('M', 'J', 'P', 'G'), 6.0, Size(640, 480));
//    // 从一个文件夹下读取多张jpg图片
//    String pattern = "/storage/sdcard1/img101/*.png";
//    vector<String> fn;
//
//    glob(pattern, fn, false);
//
//    size_t count = fn.size();
//    for (size_t i = 0; i < count; i++)
//    {
//        Mat image = imread(fn[i]);
//        // 这个大小与VideoWriter构造函数中的大小一致。
//        resize(image, image, Size(640, 480));
//        // 流操作符，把图片传入视频
//        video << image;
//    }




    return JNI_TRUE;
}

JNIEXPORT jboolean JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_OnMessage(JNIEnv *jniEnv, jobject, jlong lat,
                                                            jlong alt,jint he,jshort sp, jboolean gps) {
    UTTimer::GetInstance()->CheckTimer();
    LOGE("affter on timer");
    if(lati== nullptr){
        lati = new long;
        longi = new long;
        hei = new unsigned int;
        speed = new unsigned short;
    }
    *lati = (long)lat;
    *longi = (long)alt;
    *hei = (unsigned int)he;
    *speed = (unsigned short)sp;

    if(gps){
        CDsmJTT808_API::GetInstance()->SetGpsInfo(*lati,*longi,*hei,*speed,0);
    }
    return JNI_TRUE;
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures(JNIEnv *jniEnv, jobject obj,
                                                               jlong addrGray, jint index) {

    LOGE(" mode is %ld", addrGray);
    srand(time(0));
    if (totalFlow == nullptr) {
        if(addrGray == 0L)
            totalFlow = new TotalFlow("/sdcard/Android/data/com.ut.sdk/files");
        else
            totalFlow = new TotalFlow("/sdcard/Android/data/com.ut.sdk/files/second");
        string path = "/storage/sdcard1/img" + to_string(index) + "/";

//        string path = "/sdcard/img"+ to_string(index) + "/";
        totalFlow->path = path;
        totalFlow->pathDis = path + "distract/";
        totalFlow->pathFat = path + "fat/";
        totalFlow->pathCall = path + "call/";
        totalFlow->pathSmoke = path + "smoke/";
        totalFlow->pathAbnormal = path + "abnormal/";
        totalFlow->pathUnknow = path + "unknown/";
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

void sendData(string& szFilePath,euDSMAlarmType warn ,vector<AlarmAccessory> acc) {

    LOGE("sss ---  %s,len=%d %d  ",szFilePath.c_str(),szFilePath.length(),warn);
    UploadGPSInfo gpsInfo2(*lati,*longi,*hei,*speed,180, true,DSM_ALARM_FLAG);
    std::shared_ptr<UploadDSMAlarmInfo>  dsmAlarmInfo(new UploadDSMAlarmInfo(0,warn,euAlarmGrade2,5));
    CDsmJTT808_API::GetInstance()->SetGPSAlarmInfo(gpsInfo2,dsmAlarmInfo,acc);

}

void doi(euDSMAlarmType warn) {
    old = chrono::_V2::steady_clock::now();
    totalFlow->stopQueue = true;
    size_t count = totalFlow->pictures.size();
    int index = rand();
    string filename = totalFlow->currentPath + "w"+ to_string(index)+ "g.avi";
    LOGE("current video path %s",filename.c_str());
    VideoWriter video(filename, CV_FOURCC('M', 'J', 'P', 'G'), 5.0, Size(640, 480));
    string pic;
    for (size_t i = 0; i < count; i++) {
        Mat image = imread(totalFlow->pictures.front());
        if(i ==3)
            pic = totalFlow->pictures.front();
        // 这个大小与VideoWriter构造函数中的大小一致。
        resize(image, image, Size(640, 480));
        // 流操作符，把图片传入视频
        video << image;
        totalFlow->pictures.pop();
    }

    AlarmAccessory objAccess;
    objAccess.stFileType = euPIC;
    strcpy(objAccess.stFileName,pic.c_str());

    AlarmAccessory objAccess2;
    objAccess2.stFileType = euVideo;
    strcpy(objAccess2.stFileName,filename.c_str());


    vector<AlarmAccessory> vAccessories;
    vAccessories.push_back(objAccess);
    vAccessories.push_back(objAccess2);

    LOGE("before send data !@#$%%^^ _______");
    sendData(filename,warn,vAccessories);
    totalFlow->stopQueue =false;
}





