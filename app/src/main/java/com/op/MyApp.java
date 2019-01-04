package com.op;

import android.app.Application;


public class MyApp extends Application {
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
