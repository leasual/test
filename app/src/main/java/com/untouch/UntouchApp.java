package com.untouch;

import android.app.Application;


public class UntouchApp extends Application {
    @Override
    public void onCreate() {
        super.onCreate();
//        CrashReport.initCrashReport(getApplicationContext(), "50cd1cb403", true);
    }

    @Override
    public void onTerminate() {
        super.onTerminate();
    }
}
