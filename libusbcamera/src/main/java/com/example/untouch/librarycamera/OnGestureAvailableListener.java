package com.example.untouch.librarycamera;

import android.graphics.Bitmap;
import android.hardware.Camera;

/**
 * Created by wangjian on 18-2-8.
 */

public interface OnGestureAvailableListener {
    void onGestureFrame(Bitmap bitmap);
    void onGestureFrame(Bitmap bitmap, Camera camera);
    void onGestureFrame(Bitmap bitmap, Camera camera, boolean isEffectiveRange);
}
