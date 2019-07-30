package org.opencv.samples.tutorial2;

import android.app.Activity;
import android.content.Context;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.WindowManager;
import android.widget.TextView;

import com.op.dm.Utils;
import com.ut.sdk2.R;

import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewFrame;
import org.opencv.android.CameraBridgeViewBase.CvCameraViewListener2;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Mat;
import org.opencv.core.Point;
import org.opencv.core.Scalar;
import org.opencv.imgproc.Imgproc;

public class Tutorial2Activity extends Activity implements CvCameraViewListener2 {
    private static final String    TAG = "OCVSample::Activity";
    private Mat                    mRgba;
    private Mat                    mIntermediateMat;
    private Mat                    mGray;
    private CameraBridgeViewBase   mOpenCvCameraView;
    private byte s;
    private TextView[] views;
    private String[] name = {"用户","分神","疲劳","吸烟","打电话","异常","校准"};
//    private int[] ids = {R.id.name,R.id.dis,R.id.fat,R.id.smoke,R.id.call,R.id.abnm,R.id.calibrate};
    private BaseLoaderCallback  mLoaderCallback = new BaseLoaderCallback(this) {
        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                {
                    Log.e(TAG, "OpenCV loaded successfully");
                    System.loadLibrary("native-lib");
                    mOpenCvCameraView.enableView();
                    if(!totalDone){
                        Context context = mAppContext;
                        new AsyncTaskInitFile().execute((Tutorial2Activity) context);
                    }
//                    FindFeatures(0,0);
                } break;
                default:
                {
                    super.onManagerConnected(status);
                } break;
            }
        }
    };
     static class AsyncTaskInitFile extends AsyncTask<Tutorial2Activity,Integer,Tutorial2Activity>  {
         @Override
         protected void onPostExecute(Tutorial2Activity integer) {
             super.onPostExecute(integer);
             Log.e(TAG, "AsyncTaskInitFile  successfully");
             new AsyncTaskInitTotalFlow().execute(integer);
         }

         @Override
         protected Tutorial2Activity doInBackground(Tutorial2Activity... contexts) {
             Utils.addModeles(contexts[0]);
             return contexts[0];
         }
     }



    private String[] strings = new String[6];
    static class AsyncTaskInitTotalFlow extends AsyncTask<Tutorial2Activity,Integer,Tutorial2Activity>  {
        @Override
        protected void onPostExecute(Tutorial2Activity integer) {
            super.onPostExecute(integer);
            integer.totalDone = true;
            Log.e(TAG, "AsyncTaskInitTotalFlow  successfully");

        }
        @Override
        protected Tutorial2Activity doInBackground(Tutorial2Activity... contexts) {
            contexts[0].FindFeatures(0,0);
            return contexts[0];
        }
    }
    public Tutorial2Activity() {
        Log.i(TAG, "Instantiated new " + this.getClass());
    }

    /** Called when the activity is first created. */
//    ProgressDialog progressDialog;
    boolean totalDone = false;
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.i(TAG, "called onCreate");
        super.onCreate(savedInstanceState);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.tutorial2_surface_view);
        mOpenCvCameraView = (CameraBridgeViewBase) findViewById(R.id.tutorial2_activity_surface_view);
        mOpenCvCameraView.setVisibility(CameraBridgeViewBase.VISIBLE);
        mOpenCvCameraView.setCvCameraViewListener(this);
        mOpenCvCameraView.setCameraIndex(0);
//        mOpenCvCameraView.setMaxFrameSize(640,480);
        mOpenCvCameraView.setMaxFrameSize(1280 ,720);

    }




    @Override
    public void onPause()
    {
        super.onPause();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
    }

    @Override
    public void onResume()
    {
        super.onResume();
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization");
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION, this, mLoaderCallback);
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!");
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS);
        }
    }

    public void onDestroy() {
        super.onDestroy();
        stop();
        if (mOpenCvCameraView != null)
            mOpenCvCameraView.disableView();
        android.os.Process.killProcess(android.os.Process.myPid());
        System.exit(0);
    }
    long start;
    long onCamer;
    public void onCameraViewStarted(int width, int height) {
        start =  System.currentTimeMillis();
//        Log.e("-onCameraViewStarted", "--" +start);
        mRgba = new Mat();
        mGray = new Mat();
        if(rgb == null){
          rgb  = new Mat();
        }
        mIntermediateMat = new Mat();
    }

    public void onCameraViewStopped() {
        mRgba.release();
        mGray.release();
        mIntermediateMat.release();
        if(rgb!= null)
            rgb.release();
    }

    long onCamera;
    long lastTime;
    Mat rgb ;
    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        onCamera = System.currentTimeMillis();
//        Log.e("-onCameraFrame", "--" + (onCamera-lastTime));
        lastTime = onCamer;
//        print2();
        mRgba = inputFrame.rgba();
        mGray = inputFrame.gray();
        Imgproc.cvtColor(mRgba,rgb,Imgproc.COLOR_RGBA2RGB);
        if(totalDone)
            FindFeatures2(rgb.getNativeObjAddr(), mRgba.getNativeObjAddr(),(onCamera-lastTime));
//        lastTime = onCamera;
//        else
//            Imgproc.putText(mRgba,"正在初始化",new Point(120,120),2,2,new Scalar(122,255,50));
        return mRgba;
    }

    long old = 0;
    long now = 0;
    public void print(){
         old = System.currentTimeMillis();
//        Log.e("-time every --- ", "--" + (now - old));
    }
    public void printend(){
        now = System.currentTimeMillis();
        Log.e("-time every --- ", "--" + (now - old));
    }


    long old1 = 0;
    long now1 = 0;
    long [] times = new long[100];
    int index = 0;
    public void print2(){
        now1 = System.currentTimeMillis();
        Log.e("- frame --- ", "--" + 1000/(now1 - old1) );
        times[index] = now1 - old1;

        old1 = now1;

    }

    public native void stop();
    public native void CHECK(String mac);
    public native void FindFeatures(long matAddrGr, long matAddrRgba);
    public native long FindFeatures2(long matAddrGr, long matAddrRgba,long time);
}
