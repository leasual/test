package com.op;

import android.app.Application;

import com.tencent.bugly.crashreport.CrashReport;


public class MyApp extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
        CrashReport.initCrashReport(getApplicationContext(), "50cd1cb403", true);
        //tag
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
