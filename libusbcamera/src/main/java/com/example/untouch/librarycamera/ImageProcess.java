package com.example.untouch.librarycamera;

import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.hardware.Camera;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.media.ImageReader;
import android.os.AsyncTask;
import android.util.Log;

/**
 * Created by wangjian on 18-2-1.
 */

public class ImageProcess extends AsyncTask<Void, Void, Void> implements Camera.PreviewCallback,
        ImageReader.OnImageAvailableListener, SensorEventListener {
    private final String TAG="ImageProcess";
    private byte[] mData;
    private ImageProcess mImageTask = null;
    private Camera mCamera;
    private int[] rgbBytes = null;
    private Bitmap rgbFrameBitmap;

    private static final Object mReceiveListenerLock = new Object();
    private static OnReceiveImageAvailableListener mReceiveListener;
    private static Thread receiveThread;
    private static final Object mGestureListenerLock = new Object();
    private static OnGestureAvailableListener mGestureListener;
    private static Thread gestureThread;
    private static final Object mFaceListenerLock = new Object();
    private static OnFaceAvailableListener mFaceListener;
    private static Thread faceThread;
    public static boolean runGesture = true;
    private final Object mSensorDataLock = new Object();
    private static boolean isEffective = true;
    public ImageProcess()
    {

    }

    public ImageProcess(byte[] data, Camera camera)
    {
        this.mData = data;
        this.mCamera = camera;
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
//        //Log.i(TAG,"onPreviewFrame");
        if(null != mImageTask)
        {
            switch(mImageTask.getStatus())
            {
                case RUNNING:
//                    Log.e("mImageTask.  ", "RUNNING " );
                    return;
                case PENDING:
                    Log.e("mImageTask.  ", "PENDING " );
                    mImageTask.cancel(false);
                    break;
            }
        }
        mImageTask = new ImageProcess(data, camera);
        mImageTask.execute((Void)null);
    }


    @Override
    protected Void doInBackground(Void... voids) {
        Camera.Size size = mCamera.getParameters().getPreviewSize();

        try {
            // Initialize the storage bitmaps once when the resolution is known.
            if (rgbBytes == null) {
                rgbBytes = new int[size.width * size.height];
                rgbFrameBitmap = Bitmap.createBitmap(size.width, size.height, Bitmap.Config.ARGB_8888);
            }
        } catch (final Exception e) {
            //Log.e(TAG,"Exception!");
            return null;
        }
        ImageUtils.convertYUV420SPToARGB8888(mData, size.width, size.height, rgbBytes);
        rgbFrameBitmap.setPixels(rgbBytes, 0, size.width, 0, 0, size.width, size.height);
//        ImageUtils.saveBitmap(rgbFrameBitmap, System.currentTimeMillis()+".png");

        //Send bitmap to face task
        if(faceThread != null && faceThread.getState() == Thread.State.TERMINATED)
        {
            faceThread = null;
        }

        if(faceThread == null)
        {
            faceThread = new Thread(){
                @Override
                public void run(){
//                    sendFaceImage(rgbFrameBitmap);
                    boolean proximityFlag;
                    synchronized (mSensorDataLock){
                        proximityFlag = getProximityFlag();
                    }
//                    Bitmap img = rotateBitmap(rgbFrameBitmap, 90);
                    sendFaceImage(rgbFrameBitmap, mCamera, proximityFlag);
                }
            };
            faceThread.start();
        }

        //Send bitmap to gesture task
        if(gestureThread != null && gestureThread.getState() == Thread.State.TERMINATED)
        {
//            Log.e("gestureThread.curr  ", "null " );
            gestureThread = null;
//            Log.e("gestureThread != null ", "关闭 gestureThread");
        }

        if(gestureThread == null)
        {
//            Log.e("gestureThread == null ", "start gestureThread");
            gestureThread = new Thread(){
                @Override
                public void run(){
                    long now = System.currentTimeMillis() - time;
//                    Log.e("System.curr  ", " " + now);
                    sendGestureImage(rgbFrameBitmap, mCamera);
//                    if(now > 300){
//                        time = now;
//                        sendGestureImage(rgbFrameBitmap, mCamera);
//
//                    }
//                    while (runGesture){
//                        long now = System.currentTimeMillis() - time;
//                        if(now > 300){
//                            time = now;
//                            sendGestureImage(rgbFrameBitmap, mCamera);
//                            boolean proximityFlag;
////                            synchronized (mSensorDataLock){
////                                proximityFlag = getProximityFlag();
////                            }
////                    Bitmap img = rotateBitmap(rgbFrameBitmap, 180);
////                            sendGestureImage(rgbFrameBitmap, mCamera, proximityFlag);
//                        }
////                        sendGestureImage(rgbFrameBitmap, mCamera);
//                    }
//
                }
            };
            gestureThread.start();
        }

        //Send bitmap to viomi task
        if(receiveThread != null && receiveThread.getState() == Thread.State.TERMINATED)
        {
            receiveThread = null;
        }

        if(receiveThread == null)
        {
            receiveThread = new Thread(){
                @Override
                public void run(){
                    sendReceiveImage(rgbFrameBitmap, mCamera);
                }
            };
            receiveThread.start();
        }
        return null;
    }
    private long time = 0;

    private Bitmap rotateBitmap(Bitmap bitmap, int degress)
    {
        if(bitmap != null)
        {
//            Matrix m = ImageUtils.getTransformationMatrix(bitmap.getWidth(), bitmap.getHeight(), 640, 480, degress, true);
            Matrix m = new Matrix();
            m.postRotate(degress);
            bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), m, true);
            return bitmap;
        }
        return bitmap;
    }

    @Override
    public void onImageAvailable(ImageReader reader) {
        //Log.i(TAG,"onImageAvailable");
    }

    public static void setOnReceiveImageAvailableListener(OnReceiveImageAvailableListener listener)
    {
        synchronized (mReceiveListenerLock){
            if(listener != null)
            {
                mReceiveListener = listener;
            }else
            {
                mReceiveListener = null;
            }
        }
    }

    private static void sendReceiveImage(final Bitmap bitmap, final Camera camera)
    {
        OnReceiveImageAvailableListener listener;
        synchronized (mReceiveListenerLock){
            listener = mReceiveListener;
        }
        if(listener != null)
        {
            listener.onReceiveFrame(bitmap, camera);
        }
    }

    private static void sendReceiveImage(final byte[] data, final Camera camera)
    {
        OnReceiveImageAvailableListener listener;
        synchronized (mReceiveListenerLock){
            listener = mReceiveListener;
        }
        if(listener != null)
        {
            listener.onReceiveFrame(data, camera);
        }
    }

    public static void setOnGestureAvailableListener(OnGestureAvailableListener listener)
    {
        synchronized (mGestureListenerLock){
            if(listener != null)
            {
                mGestureListener = listener;
            }else
            {
                mGestureListener = null;
            }
        }
    }

    private static void sendGestureImage(final Bitmap bitmap)
    {
        OnGestureAvailableListener listener;
        synchronized (mGestureListenerLock){
            listener = mGestureListener;
        }
        if(listener != null)
        {
            listener.onGestureFrame(bitmap);
        }
    }

    private static void sendGestureImage(final Bitmap bitmap, final Camera camera)
    {

        if(mGestureListener!= null){
            mGestureListener.onGestureFrame(bitmap, camera, false);
        }
//        OnGestureAvailableListener listener;
//        synchronized (mGestureListenerLock){
//            listener = mGestureListener;
//        }
//        if(listener != null)
//        {
//            listener.onGestureFrame(bitmap, camera);
//        }
    }

    private static void sendGestureImage(final Bitmap bitmap, final Camera camera, final boolean isEffect)
    {
        if(mGestureListener!= null){
            mGestureListener.onGestureFrame(bitmap, camera, isEffect);
        }
//        OnGestureAvailableListener listener;
//        synchronized (mGestureListenerLock){
//            listener = mGestureListener;
//        }
//        if(listener != null)
//        {
//            Log.e("sendGestureImage ", "listener 不为空 ");
//            listener.onGestureFrame(bitmap, camera, isEffect);
//        }
    }


    public static void setOnFaceAvailableListener(OnFaceAvailableListener listener)
    {
        synchronized (mFaceListenerLock){
            if(listener != null)
            {
                mFaceListener = listener;
            }else
            {
                mFaceListener = null;
            }
        }
    }

    private static void sendFaceImage(final Bitmap bitmap)
    {
        OnFaceAvailableListener listener;
        synchronized (mFaceListenerLock){
            listener = mFaceListener;
        }
        if(listener != null)
        {
            listener.onFaceFrame(bitmap);
        }
    }

    private static void sendFaceImage(final Bitmap bitmap, final Camera camera)
    {
        OnFaceAvailableListener listener;
        synchronized (mFaceListenerLock){
            listener = mFaceListener;
        }
        if(listener != null)
        {
            listener.onFaceFrame(bitmap, camera);
        }
    }

    private static void sendFaceImage(final Bitmap bitmap, final Camera camera, final boolean isEffect)
    {
        OnFaceAvailableListener listener;
        synchronized (mFaceListenerLock){
            listener = mFaceListener;
        }
        if(listener != null)
        {
            listener.onFaceFrame(bitmap, camera, isEffect);
        }
    }

    private static void setProximityFlag(boolean flag)
    {
        isEffective = flag;
    }

    private static boolean getProximityFlag()
    {
        return isEffective;
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        synchronized (mSensorDataLock){
            if(event.values[0] < 5.0f)
            {
                setProximityFlag(true);
            }else
            {
                setProximityFlag(false);
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }
}
