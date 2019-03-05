#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include "Util.h"
#include "faceid.h"
#include <vector>
#include "PictureProcessor.h"
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
                                                               jint index);
JNIEXPORT jint JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject,
                                                                jlong addrGray, jlong addrRgba,
                                                                jboolean regis,jboolean);
JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject);
JNIEXPORT jboolean JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_CHECK(JNIEnv *jniEnv, jobject,jstring);

FaceID* faceID;
PictureProcessor* processor;

JNIEXPORT jint JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures2(JNIEnv *jniEnv, jobject obj,
                                                                jlong copyMat, jlong addrRgba,
                                                                jboolean regis, jboolean picture) {
//    std::string image_path1 = "/sdcard/Android/data/com.ut.sdk/files/1.png";
//    std::string image_path3 = "/sdcard/Android/data/com.ut.sdk/files/cl.png";
//    string a[] = {image_path1, image_path3};
    string path = "/sdcard/Android/data/com.ut.register/files/aa";
    cv::putText(*(Mat*)copyMat, "origin", cv::Point(22,22),1,2,cv::Scalar(122,255,50));
//    cv::putText(*(Mat*)addrRgba, "origin", cv::Point(22,22),1,2,cv::Scalar(122,255,50));

//    LOGE("IN TO   A ");
    if(faceID != nullptr){
//        LOGE("IN TO   B ");
        cv::Mat image = *(Mat*)copyMat;
        cv::Mat feature1, feature2;
        cv::Rect face_bbox;
        std::vector<cv::Point2f> landmarks;
        cv::Vec3d angles;
        bool suc = faceID->FaceDetect(image, face_bbox, landmarks, angles);
//        LOGE(" detect is suc %d ", suc);
//
//        LOGE(" landmark is  %ld ", landmarks.size());
//        LOGE(" frame is  %d , %d", image.rows, image.cols);
        if(suc){
            faceID->FeatureExtract(image, landmarks, feature1);
            faceID->WriteFeature(feature1, path + "/fea.bin");
            faceID->ReadFeature(feature2, path + "/fea.bin");
//            std::cout << "score : " << faceID->Match(feature1, feature2) << std::endl;
//            LOGE(" equal =  %f",faceID->Match(feature1, feature2));

            cv::rectangle(image, face_bbox, cv::Scalar(255,0,0), 1, 1);
            for (auto &p : landmarks) {
                cv::circle(image, p, 1, cv::Scalar(0, 0, 255), 1, 1);
            }

//            cv::imwrite(path+ "/eye" + to_string(processor->eye) + ".png", image);
            processor->eye ++;
//            LOGE("IN TO   C ");
            //eye
            float dx = landmarks[48].x - landmarks[44].x;
            float dy = landmarks[50].y - landmarks[46].y;

            //mouth
            float dxm = landmarks[66].x - landmarks[60].x;
            float dym = landmarks[78].y - landmarks[74].y;
//            LOGE("IN TO   D ");
            float ratio = dy / dx;
            float ratiom = dym / dxm;

            if(ratio > processor->standard){
                processor->standard = ratio;
            }
            if(ratiom < processor->standardMouth){
                processor->standardMouth = ratiom;
            }
            cv::putText(*(Mat*)addrRgba, "open ", cv::Point(220,80),1,1,cv::Scalar(122,255,50));
            if(processor->standard != 0 && processor->standard/ratio > 3){
                processor->eye ++;
                cv::imwrite(path+ "/close_eye" + to_string(processor->eye) + ".png", image);
                LOGE("close eye !!! standard eye is %f , now eye is %f ",processor->standard, ratio );
                LOGE("open mouth !!!  standard mouth is %f , now  is %f ",processor->standardMouth, ratiom );
                cv::putText(*(Mat*)addrRgba, "close ", cv::Point(220,80),1,1,cv::Scalar(122,255,50));

            }

            if(processor->standardMouth != 0 && ratiom/(processor->standardMouth) > 3){
                processor->mouth ++;
                cv::imwrite(path+ "/open_mouth" + to_string(processor->mouth) + ".png", image);
                LOGE("open mouth !!!  standard mouth is %f , now  is %f ",processor->standardMouth, ratiom );
            }

        }




    }

    return 0;
}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_stop(JNIEnv *jniEnv, jobject) {

}
JNIEXPORT jboolean JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_CHECK(JNIEnv *jniEnv, jobject, jstring mac){

}

JNIEXPORT void JNICALL
Java_org_opencv_samples_tutorial2_DetectActitvity_FindFeatures(JNIEnv *jniEnv, jobject obj,
                                                                 jlong addrGray, jint index) {
    string path = "/sdcard/Android/data/com.ut.register/files";
    if(faceID == nullptr){
        faceID = new FaceID(path);
        processor = new PictureProcessor();

        FaceID faceID(path);
        std::string image_path1 = "/sdcard/Android/data/com.ut.register/files/1.png";
        std::string image_path3 = "/sdcard/Android/data/com.ut.register/files/eye104.png";
        string a[] = {image_path1, image_path3};

        for (int i = 0; i < 2; ++i) {
            cv::Mat image = cv::imread(a[i]);

            cv::Mat feature1, feature2;
            cv::Rect face_bbox;
            std::vector<cv::Point2f> landmarks;
            cv::Vec3d angles;

//        cv::Mat imageGray;
//        cv::cvtColor(image,imageGray,COLOR_RGBA2GRAY);

            if(faceID.FaceDetect(image, face_bbox, landmarks, angles)){
                faceID.FeatureExtract(image, landmarks, feature1);
                faceID.WriteFeature(feature1, path + "/fea.bin");
                faceID.ReadFeature(feature2, path + "/fea.bin");
                std::cout << "score : " << faceID.Match(feature1, feature2) << std::endl;
                LOGE(" equal =  %f",faceID.Match(feature1, feature2));
            }

            cv::rectangle(image, face_bbox, cv::Scalar(255,0,0), 1, 1);
            for (auto &p : landmarks) {
                cv::circle(image, p, 1, cv::Scalar(0, 0, 255), 2, 2);
                LOGE("landmark value %f , %f ",p.x,p.y);
            }


            cv::imwrite(path+ "/result" + to_string(i) + ".png", image);
        }

    }

}





}



