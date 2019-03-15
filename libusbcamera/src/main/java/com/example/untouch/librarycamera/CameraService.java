package com.example.untouch.librarycamera;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.graphics.PixelFormat;
import android.hardware.Camera;
import android.hardware.Sensor;
import android.hardware.SensorManager;
import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.IBinder;
import android.os.ResultReceiver;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.List;

public class CameraService extends Service {
    private static final String TAG = CameraService.class.getSimpleName();

    public static final String RESULT_RECEIVER = "resultReceiver";
    public static final String VIDEO_PATH = "recordedVideoPath";

//    public static final int RECORD_RESULT_OK = 0;
//    public static final int RECORD_RESULT_DEVICE_NO_CAMERA= 1;
//    public static final int RECORD_RESULT_GET_CAMERA_FAILED = 2;
//    public static final int RECORD_RESULT_ALREADY_RECORDING = 3;
//    public static final int RECORD_RESULT_NOT_RECORDING = 4;
//    public static final int RECORD_RESULT_UNSTOPPABLE = 5;
    public static final int PHOTO_RESULT_OK = 0;
    public static final int PHOTO_RESULT_DEVICE_NO_CAMERA= 1;
    public static final int PHOTO_RESULT_GET_CAMERA_FAILED = 2;
    public static final int PHOTO_RESULT_ALREADY_PHOTOING = 3;
    public static final int PHOTO_RESULT_NOT_PHOTOING = 4;
    public static final int PHOTO_RESULT_UNSTOPPABLE = 5;

    private static final String START_SERVICE_COMMAND = "startServiceCommands";
    private static final int COMMAND_NONE = -1;
    private static final int COMMAND_START_PHOTOING = 0;
    private static final int COMMAND_STOP_PHOTOING = 1;

//    private static final String SELECTED_CAMERA_FOR_RECORDING = "cameraForRecording";
    private static final String SELECTED_CAMERA_FOR_PHOTOING = "cameraForPhotoing";

    private Camera mCamera;
    private MediaRecorder mMediaRecorder;

    private boolean isPreview = false;
    private String mRecordingPath = null;
    private ImageProcess imageProcess;

    private SensorManager sensorManager;
    private Sensor proximitySensor;

    public CameraService() {
    }

    public static void startToStartRecording(Context context, int cameraId,
                                             ResultReceiver resultReceiver) {
        Intent intent = new Intent(context.getApplicationContext(), CameraService.class);
        intent.putExtra(START_SERVICE_COMMAND, COMMAND_START_PHOTOING);
        intent.putExtra(SELECTED_CAMERA_FOR_PHOTOING, cameraId);
        intent.putExtra(RESULT_RECEIVER, resultReceiver);
        context.getApplicationContext().startService(intent);
    }

    public static void startToStopRecording(Context context, ResultReceiver resultReceiver) {
        Intent intent = new Intent(context.getApplicationContext(), CameraService.class);
        intent.putExtra(START_SERVICE_COMMAND, COMMAND_STOP_PHOTOING);
        intent.putExtra(RESULT_RECEIVER, resultReceiver);
        context.getApplicationContext().startService(intent);
    }

    /**
     * Used to take picture.
     */
    private Camera.PictureCallback mPicture = new Camera.PictureCallback() {
        @Override
        public void onPictureTaken(byte[] data, Camera camera) {
            File pictureFile = Util.getOutputMediaFile(Util.MEDIA_TYPE_IMAGE);

            if (pictureFile == null) {
                return;
            }

            try {
                FileOutputStream fos = new FileOutputStream(pictureFile);
                fos.write(data);
                fos.close();
            } catch (FileNotFoundException e) {
            } catch (IOException e) {
            }
        }
    };

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            throw new IllegalStateException("Must start the service with intent");
        }
        switch (intent.getIntExtra(START_SERVICE_COMMAND, COMMAND_NONE)) {
            case COMMAND_START_PHOTOING:
                handleStartRecordingCommand(intent);
                break;
            case COMMAND_STOP_PHOTOING:
                handleStopRecordingCommand(intent);
                break;
            default:
                throw new UnsupportedOperationException("Cannot start service with illegal commands");
        }

        return START_NOT_STICKY;
    }

    private void handleStartRecordingCommand(Intent intent) {
        if (!Util.isCameraExist(this)) {
            throw new IllegalStateException("There is no device, not possible to start recording");
        }

        //定义接收图片的对象
        imageProcess = new ImageProcess();
        //proximity start
        handleStartProximitySensor();

        final ResultReceiver resultReceiver = intent.getParcelableExtra(RESULT_RECEIVER);

        if (isPreview) {
            // Already recording
            resultReceiver.send(PHOTO_RESULT_ALREADY_PHOTOING, null);
            return;
        }
        isPreview = true;

        final int cameraId = intent.getIntExtra(SELECTED_CAMERA_FOR_PHOTOING,
                Camera.CameraInfo.CAMERA_FACING_BACK);
        mCamera = Util.getCameraInstance(cameraId);


        if (mCamera != null) {
            SurfaceView sv = new SurfaceView(this);

            WindowManager wm = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
            WindowManager.LayoutParams params = new WindowManager.LayoutParams(1, 1,
                    WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY,
                    WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH,
                    PixelFormat.TRANSLUCENT);

            SurfaceHolder sh = sv.getHolder();

            sv.setZOrderOnTop(true);
            sh.setFormat(PixelFormat.TRANSPARENT);

            sh.addCallback(new SurfaceHolder.Callback() {
                @Override
                public void surfaceCreated(SurfaceHolder holder) {
                    try {
                        Camera.Parameters params = mCamera.getParameters();
                        mCamera.setParameters(params);
                        Camera.Parameters p = mCamera.getParameters();

                        List<Camera.Size> listSize;

                        listSize = p.getSupportedPreviewSizes();
//                        Camera.Size mPreviewSize = listSize.get(6); //width:640 height:480  //3399: 1280x720
//                        Camera.Size mPreviewSize = listSize.get(2); //width:640 height:480  //huawei 1280x720
                        Camera.Size mPreviewSize = listSize.get(0);;
                        for(int i = 0; i < listSize.size(); i++)
                        {
                            if(1280 == listSize.get(i).width && 720 == listSize.get(i).height)
                            {
                                mPreviewSize = listSize.get(i); // preview:1280x720
                                break;
                            }
                        }
                        p.setPreviewSize(mPreviewSize.width, mPreviewSize.height); //设置预览照片的大小

                        listSize = p.getSupportedPictureSizes();
//                        Camera.Size mPictureSize = listSize.get(8); //3399:1280x720
//                        Camera.Size mPictureSize = listSize.get(2); //huawei  //1280x720
                        Camera.Size mPictureSize = listSize.get(0);;
                        for(int i = 0; i < listSize.size(); i++)
                        {
                            if(1280 == listSize.get(i).width && 720 == listSize.get(i).height)
                            {
                                mPictureSize = listSize.get(i);  // picture:1280x720
                                break;
                            }
                        }
                        p.setPictureSize(mPictureSize.width, mPictureSize.height);
                        mCamera.setParameters(p);
                        mCamera.setDisplayOrientation(90);

                        try {
                            mCamera.setPreviewDisplay(holder);
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        mCamera.setPreviewCallback(imageProcess);
                        mCamera.startPreview();
                        mCamera.autoFocus(null);
                        mRecordingPath = Util.getOutputMediaFile(Util.MEDIA_TYPE_IMAGE).getPath();
                    }catch(Exception e)
                    {
                        e.printStackTrace();
                    }

                    resultReceiver.send(PHOTO_RESULT_OK, null);
                    //Log.i(TAG, "Photoing is started");
                }

                @Override
                public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
                }

                @Override
                public void surfaceDestroyed(SurfaceHolder holder) {
                }
            });

            sh.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);


            wm.addView(sv, params);

        } else {
            //Log.d(TAG, "Get Camera from service failed");
            resultReceiver.send(PHOTO_RESULT_GET_CAMERA_FAILED, null);
        }
    }

    private void handleStopRecordingCommand(Intent intent) {
        ResultReceiver resultReceiver = intent.getParcelableExtra(RESULT_RECEIVER);

        if(mCamera != null){
            if(isPreview)
            {
                isPreview = false;
                mCamera.stopPreview();
            }
            mCamera.setPreviewCallback(null);
            mCamera.release();
            mCamera = null;
        }
        handleStopProximitySensor();

        Bundle b = new Bundle();
        b.putString(VIDEO_PATH, mRecordingPath);
        resultReceiver.send(PHOTO_RESULT_OK, b);

        //Log.d(TAG, "photoing is finished.");
    }

    private void handleStartProximitySensor()
    {
        sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        proximitySensor = sensorManager.getDefaultSensor(Sensor.TYPE_PROXIMITY);
        if(proximitySensor != null)
        {
            sensorManager.registerListener(imageProcess, proximitySensor, SensorManager.SENSOR_DELAY_NORMAL);
        }
    }

    private void handleStopProximitySensor()
    {
        if(proximitySensor != null)
        {
            sensorManager.unregisterListener(imageProcess);
        }
    }

//    private void handleStartRecordingCommand(Intent intent) {
//        if (!Util.isCameraExist(this)) {
//            throw new IllegalStateException("There is no device, not possible to start recording");
//        }
//
//        final ResultReceiver resultReceiver = intent.getParcelableExtra(RESULT_RECEIVER);
//
//        if (mRecording) {
//            // Already recording
//            resultReceiver.send(RECORD_RESULT_ALREADY_RECORDING, null);
//            return;
//        }
//        mRecording = true;
//
//        final int cameraId = intent.getIntExtra(SELECTED_CAMERA_FOR_RECORDING,
//                Camera.CameraInfo.CAMERA_FACING_BACK);
//        mCamera = Util.getCameraInstance(cameraId);
//        if (mCamera != null) {
//            SurfaceView sv = new SurfaceView(this.getApplicationContext());
//
//            WindowManager wm = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
//            WindowManager.LayoutParams params = new WindowManager.LayoutParams(1, 1,
//                    WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY,
//                    WindowManager.LayoutParams.FLAG_WATCH_OUTSIDE_TOUCH,
//                    PixelFormat.TRANSLUCENT);
//
//            SurfaceHolder sh = sv.getHolder();
//
//            sv.setZOrderOnTop(true);
//            sh.setFormat(PixelFormat.TRANSPARENT);
//
//            sh.addCallback(new SurfaceHolder.Callback() {
//                @Override
//                public void surfaceCreated(SurfaceHolder holder) {
//                    Camera.Parameters params = mCamera.getParameters();
//                    mCamera.setParameters(params);
//                    Camera.Parameters p = mCamera.getParameters();
//
//                    List<Camera.Size> listSize;
//
//                    listSize = p.getSupportedPreviewSizes();
//                    Camera.Size mPreviewSize = listSize.get(2);
//                    Log.v(TAG, "preview width = " + mPreviewSize.width
//                            + " preview height = " + mPreviewSize.height);
//                    p.setPreviewSize(mPreviewSize.width, mPreviewSize.height);
//
//                    listSize = p.getSupportedPictureSizes();
//                    Camera.Size mPictureSize = listSize.get(2);
//                    Log.v(TAG, "capture width = " + mPictureSize.width
//                            + " capture height = " + mPictureSize.height);
//                    p.setPictureSize(mPictureSize.width, mPictureSize.height);
//                    mCamera.setParameters(p);
//
//                    try {
//                        mCamera.setPreviewDisplay(holder);
//                    } catch (IOException e) {
//                        e.printStackTrace();
//                    }
//                    mCamera.startPreview();
//
//                    mCamera.unlock();
//
//                    mMediaRecorder = new MediaRecorder();
//                    mMediaRecorder.setCamera(mCamera);
//
//                    mMediaRecorder.setAudioSource(MediaRecorder.AudioSource.MIC);
//                    mMediaRecorder.setVideoSource(MediaRecorder.VideoSource.CAMERA);
//
//                    if (cameraId == Camera.CameraInfo.CAMERA_FACING_BACK) {
//                        mMediaRecorder.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_HIGH));
//                    } else {
//                        mMediaRecorder.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_480P));
//                    }
//
//                    mRecordingPath = Util.getOutputMediaFile(Util.MEDIA_TYPE_VIDEO).getPath();
//                    mMediaRecorder.setOutputFile(mRecordingPath);
//
//                    mMediaRecorder.setPreviewDisplay(holder.getSurface());
//
//                    try {
//                        mMediaRecorder.prepare();
//                    } catch (IllegalStateException e) {
//                        //Log.d(TAG, "IllegalStateException when preparing MediaRecorder: " + e.getMessage());
//                    } catch (IOException e) {
//                        //Log.d(TAG, "IOException when preparing MediaRecorder: " + e.getMessage());
//                    }
//                    mMediaRecorder.start();
//
//                    resultReceiver.send(RECORD_RESULT_OK, null);
//                    //Log.d(TAG, "Recording is started");
//                }
//
//                @Override
//                public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
//                }
//
//                @Override
//                public void surfaceDestroyed(SurfaceHolder holder) {
//                }
//            });
//
//
//            wm.addView(sv, params);
//
//        } else {
//            //Log.d(TAG, "Get Camera from service failed");
//            resultReceiver.send(RECORD_RESULT_GET_CAMERA_FAILED, null);
//        }
//    }
//
//    private void handleStopRecordingCommand(Intent intent) {
//        ResultReceiver resultReceiver = intent.getParcelableExtra(RESULT_RECEIVER);
//
//        if (!mRecording) {
//            // have not recorded
//            resultReceiver.send(RECORD_RESULT_NOT_RECORDING, null);
//            return;
//        }
//
//        try {
//            mMediaRecorder.stop();
//            mMediaRecorder.release();
//        } catch (RuntimeException e) {
//            mMediaRecorder.reset();
//            resultReceiver.send(RECORD_RESULT_UNSTOPPABLE, new Bundle());
//            return;
//        } finally {
//            mMediaRecorder = null;
//            mCamera.stopPreview();
//            mCamera.release();
//
//            mRecording = false;
//        }
//
//        Bundle b = new Bundle();
//        b.putString(VIDEO_PATH, mRecordingPath);
//        resultReceiver.send(RECORD_RESULT_OK, b);
//
//        //Log.d(TAG, "recording is finished.");
//    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        throw new UnsupportedOperationException("Not yet implemented");
    }
}
