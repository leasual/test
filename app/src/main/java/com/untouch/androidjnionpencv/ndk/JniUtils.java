package com.untouch.androidjnionpencv.ndk;

import android.util.Log;

import org.opencv.core.Mat;


/**
 * Created by ZLOVE on 16/12/13.
 */
public class JniUtils {
    private String name = "Java";

    public void callbackForJni(String fromNative) {
        Log.d("ZLiZH", "callbackForJni---jni string from native" + fromNative);
    }

    static {
        System.loadLibrary("NameProvider");
    }

    public native boolean Init(String configure_path);
    public native boolean FeatureExtract(Mat image, Mat feature);
    public native float FeatureMatch(Mat feature1, Mat feature2);
    public native void Run(Mat image, STATE state,RESULT result);
    enum   STATE {
        NOFACE ,
        NORMAL,
        DISTRACTION ,
        FATIGUE ,
        SMOKE,
        CALL;
    }

    enum   RESULT {
        NORMAL,
        WARN ,
        JUDGEMENT;
    }

}
