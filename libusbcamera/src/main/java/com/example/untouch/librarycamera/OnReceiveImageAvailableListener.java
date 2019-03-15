package com.example.untouch.librarycamera;

import android.graphics.Bitmap;
import android.hardware.Camera;

/**
 * Created by wangjian on 18-2-24.
 */

public interface OnReceiveImageAvailableListener {
    void onReceiveFrame(Bitmap bitmap, Camera camera);
    void onReceiveFrame(byte[] data, Camera camera);
}
