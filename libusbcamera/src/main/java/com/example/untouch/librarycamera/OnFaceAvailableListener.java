package com.example.untouch.librarycamera;

import android.graphics.Bitmap;
import android.hardware.Camera;

/**
 * Created by wangjian on 18-2-8.
 */

public interface OnFaceAvailableListener {
    void onFaceFrame(Bitmap bitmap);
    void onFaceFrame(Bitmap bitmap, Camera camera);
    void onFaceFrame(Bitmap bitmap, Camera camera, boolean isEffectiveRange);
}
