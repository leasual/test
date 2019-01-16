package org.opencv.samples.tutorial2

import android.app.Activity
import android.app.AlertDialog
import android.app.ProgressDialog
import android.content.ContentValues.TAG
import android.content.DialogInterface
import android.media.AudioManager
import android.media.MediaPlayer
import android.net.Uri
import android.os.AsyncTask
import android.os.Bundle
import android.os.Looper
import android.util.Log
import android.view.WindowManager
import android.widget.TextView
import com.op.dm.Utils
import com.ut.sdk.R
import com.ut.sdk.R.id.calibrate
import com.ut.sdk.R.id.tutorial2_activity_surface_view
import kotlinx.android.synthetic.main.tutorial2_surface_view.*
import org.opencv.android.BaseLoaderCallback
import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.LoaderCallbackInterface
import org.opencv.android.OpenCVLoader
import org.opencv.core.Mat
import org.opencv.imgproc.Imgproc
/**
 * Created by chris on 1/4/19.
 */


class DetectActitvity : Activity(), CameraBridgeViewBase.CvCameraViewListener2 {
    internal var index = 0
    var rgb: Mat? = null
    private var mRgba: Mat? = null
    private var mGray: Mat? = null
    private var progressDialog: ProgressDialog? = null
    //这个优先级会有变动，为了不修改jni里返回值顺序和ui的顺序，引入这个数组，之后只改这里就可以修改警告播报优先级-> 从names[3],names[2],names[1]..这个顺序遍历names数组
    private val priority = arrayOf(3,2,1,0,4)
    private val names = arrayOf("分神", "疲劳", "吸烟", "打电话", "画面异常")
    private val lastTime = arrayOf(0L,0L,0L,0L,0L)
    private val audio = arrayOf(R.raw.fenshen, R.raw.pilao, R.raw.chouyan, R.raw.dadianhua, R.raw.huamianyichang)
    private var views: Array<TextView>? = null
    private val strings = arrayOfNulls<String>(5)
    internal var totalDone = false
    internal var register = false
    private var players = arrayOfNulls<MediaPlayer>(5)//每秒有n次检测，即时响应，分多个实例


    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }

    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.tutorial2_surface_view)
        views = arrayOf(dis, fat, smoke, call, abnm)
        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCvCameraViewListener(this@DetectActitvity)
            setCameraIndex(0)
            setMaxFrameSize(1280, 720)
        }

        progressDialog = ProgressDialog(this)
        progressDialog?.setTitle("加载中,请稍后")
        progressDialog?.show()

        var builder: AlertDialog.Builder? = AlertDialog.Builder(this)
        builder?.let {
            with(it) {
                setTitle("是否进行注册?")
                setMessage("注册时请保持摄像头能清晰照到脸部")
                setPositiveButton("注册") { _, _ ->
                    register = true
                    totalDone = true
                }
                setNegativeButton("直接进入") { _, _ ->
                    register = false
                    totalDone = true
                }
                setCancelable(false)
                alertDialog = it.create()
                alertDialog?.setCanceledOnTouchOutside(false)
            }
        }
    }

    private fun getStringResult(result: IntArray?) {
        result?.let { array ->
            priority.forEach {// access the uiText and Jni result by the order of 'priority'
                when(array[it]){//jni function return value which represents each item's status --> fat, call, smoke etc.
                    2 -> {
                        strings[it] = "报警"
                        playWarnning(it)
                    }
                    1 -> strings[it] = "警告"
                    else -> strings[it] = "正常"
                }
            }
        }
        writeViews()
    }

    private fun playWarnning(index: Int) {//每种提示音分开计算，4秒内不重复播放同一种
        var time = System.currentTimeMillis()- lastTime[0]
        if(time > 4000){
            players[index]?.apply {
                if (!this.isPlaying){
                    lastTime[0] = System.currentTimeMillis()
                    start()
                }
            }
        }

    }

    private fun writeViews() {
        views?.forEachIndexed { index, textView ->
            with(textView) {
                post {
                    text = names[index] + " : " + strings[index]
                    if (text.contains("报警")) {
                        setTextColor(resources.getColor(R.color.red))
                    } else
                        setTextColor(resources.getColor(R.color.green))
                }
            }
        }
    }

    private fun writeViews2(result: IntArray?) {
        result?.let {
            views?.forEachIndexed { index, textView ->
                textView.post {
                    textView.text = names[index] + " : " + result[index]
                }
            }
        }
    }


    public override fun onPause() {
        super.onPause()
        tutorial2_activity_surface_view?.disableView()
    }

    public override fun onResume() {
        super.onResume()
        if (!OpenCVLoader.initDebug()) {
            Log.d(TAG, "Internal OpenCV library not found. Using OpenCV Manager for initialization")
            OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION, this, mLoaderCallback)
        } else {
            Log.d(TAG, "OpenCV library found inside package. Using it!")
            mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS)
        }
    }

    public override fun onDestroy() {
        super.onDestroy()
        stop()
        tutorial2_activity_surface_view?.disableView()
        android.os.Process.killProcess(android.os.Process.myPid())
        players.forEach {
            it?.stop()
            it?.release()
        }
        System.exit(0)
    }

    override fun onCameraViewStarted(width: Int, height: Int) {
        mRgba = Mat()
        mGray = Mat()
        if (rgb == null) {
            rgb = Mat()
        }
    }

    override fun onCameraViewStopped() {
        mRgba?.release()
        mGray?.release()
        if (rgb != null)
            rgb?.release()
    }

    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat {
//        onCamera = System.currentTimeMillis()
        //        Log.e("-onCameraFrame", "--" + (onCamera-lastTime));
//        lastTime = onCamer
        //        print2();
        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()
        rgb?.let { it1 ->
            Imgproc.cvtColor(mRgba, it1, Imgproc.COLOR_RGBA2RGB)
            if (totalDone)
                mRgba?.let {
                    //                    if(index % 2 == 0){//每两帧调用一次算法
                    if (index % 4 == 0)//减少uitext更新频率，没必要每帧都改变
                        getStringResult(FindFeatures2(it1.nativeObjAddr, it.nativeObjAddr, register))
                    else
                        FindFeatures2(it1.nativeObjAddr, it.nativeObjAddr, register)
//                    }
                }
        }
        if (index >= 10000)
            index = 0
        index++
        return mRgba!!
    }

    private val mLoaderCallback = object : BaseLoaderCallback(this) {
        override fun onManagerConnected(status: Int) {
            when (status) {
                LoaderCallbackInterface.SUCCESS -> {
                    Log.e(TAG, "OpenCV loaded successfully")
                    System.loadLibrary("native-lib")
                    tutorial2_activity_surface_view?.enableView()
                    if (!totalDone) {
                        val context = mAppContext
                        AsyncTaskInitFile().execute(context as DetectActitvity)
                    }
                }
                else -> {
                    super.onManagerConnected(status)
                }
            }
        }
    }

    internal class AsyncTaskInitFile : AsyncTask<DetectActitvity, Int, DetectActitvity>() {
        override fun onPostExecute(integer: DetectActitvity) {
            super.onPostExecute(integer)
            Log.e(TAG, "AsyncTaskInitFile  successfully")
            AsyncTaskInitTotalFlow().execute(integer)
        }

        override fun doInBackground(vararg contexts: DetectActitvity): DetectActitvity {
            Utils.addModeles(contexts[0])
            return contexts[0]
        }
    }

    private var alertDialog: AlertDialog? = null

    internal class AsyncTaskInitTotalFlow : AsyncTask<DetectActitvity, Int, DetectActitvity>() {
        override fun onPostExecute(integer: DetectActitvity) {
            super.onPostExecute(integer)
//            integer.totalDone = true
            integer.views?.forEachIndexed { index, textView ->
                textView.text = integer.names[index] + " : " + "正常"
            }
            integer.progressDialog?.dismiss()
            integer.alertDialog?.show()
            Log.e(TAG, "AsyncTaskInitTotalFlow  successfully")
        }

        override fun doInBackground(vararg contexts: DetectActitvity): DetectActitvity {
            for (index in 0..4){
                contexts[0].players[index] = MediaPlayer.create(contexts[0],contexts[0].audio[index])
            }
            contexts[0].FindFeatures(0, 0)
            return contexts[0]
        }
    }

    external fun stop()
    external fun FindFeatures(matAddrGr: Long, matAddrRgba: Long)
    external fun FindFeatures2(matAddrGr: Long, matAddrRgba: Long, time: Boolean): IntArray

    companion object {
        private const val TAG = "OCVSample::Activity"
    }
}
