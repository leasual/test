//
// Created by fricke on 8/20/17.
//

#ifndef OPENCV_NDK_UTIL_H
#define OPENCV_NDK_UTIL_H
#include <stdlib.h>
#include <unistd.h>
#include <android/log.h>
#include <jni.h>
#include <vector>

// used to get logcat outputs which can be regex filtered by the LOG_TAG we give
// So in Logcat you can filter this example by putting OpenCV-NDK
#define LOG_TAG "OpenCV-NDK-Native"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
//#define ASSERT(cond, fmt, ...)                                \
  if (!(cond)) {                                              \
    __android_log_assert(#cond, LOG_TAG, fmt, ##__VA_ARGS__); \
  }

//jstring stoJstring(JNIEnv* env, const char* pat)
//{
//  jclass strClass = env->FindClass("Ljava/lang/String;");
//  jmethodID ctorID = env->GetMethodID(strClass, "<init>", "([BLjava/lang/String;)V");
//  jbyteArray bytes = env->NewByteArray(strlen(pat));
//  env->SetByteArrayRegion(bytes, 0, strlen(pat), (jbyte*)pat);
//  jstring encoding = env->NewStringUTF("utf-8");
//  return (jstring)env->NewObject(strClass, ctorID, bytes, encoding);
//}
//
//string js2string(JNIEnv *env, jstring jStr){
//  const char *cstr = env->GetStringUTFChars(jStr, NULL);
//  string str = string(cstr);
//  env->ReleaseStringUTFChars(jStr, cstr);
//  return str;
//}
//void callback2(JNIEnv *jniEnv,jobject obj) {
//  jclass cl = jniEnv ->FindClass("org/opencv/samples/tutorial2/Tutorial2Activity");
//  jmethodID meth = jniEnv->GetMethodID(cl,"print","()V");
//  jniEnv->CallVoidMethod(obj,meth);
//}
//
//void callbackEnd2(JNIEnv *jniEnv,jobject obj) {
//  jclass cl = jniEnv ->FindClass("org/opencv/samples/tutorial2/Tutorial2Activity");
//  jmethodID meth = jniEnv->GetMethodID(cl,"printend","()V");
//  jniEnv->CallVoidMethod(obj,meth);
//}

/**
 * A helper class to assist image size comparison, by comparing the absolute
 * size
 * regardless of the portrait or landscape mode.
 */

#endif  // OPENCV_NDK_UTIL_H
