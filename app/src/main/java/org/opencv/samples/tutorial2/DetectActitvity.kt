package org.opencv.samples.tutorial2

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.app.ProgressDialog
import android.content.Context
import android.content.SharedPreferences
import android.hardware.camera2.CameraManager
import android.location.*
import android.media.MediaPlayer
import android.net.wifi.WifiManager
import android.os.AsyncTask
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.TextUtils
import android.util.Log
import android.view.WindowManager
import android.widget.TextView
import com.op.dm.Utils
import com.ut.sdk.R
import kotlinx.android.synthetic.main.tutorial2_surface_view.*
import org.opencv.android.BaseLoaderCallback
import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.LoaderCallbackInterface
import org.opencv.android.OpenCVLoader
import org.opencv.core.Core
import org.opencv.core.Mat
import org.opencv.imgproc.Imgproc
import java.util.*

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
//    private val priority = arrayOf(3, 2, 0, 1, 4)
    private val names = arrayOf("请转动头部", "请眨眼", "请张嘴")
    private val lastTime = arrayOf(0L, 0L, 0L)
    private val audio = arrayOf(R.raw.fenshen, R.raw.pilao, R.raw.chouyan, R.raw.dadianhua, R.raw.huamianyichang)
    private var views: Array<TextView>? = null
    internal var totalDone = false
    internal var register = false
    private var players = arrayOfNulls<MediaPlayer>(3)//每秒有n次检测，即时响应，分多个实例
    private var sdPlayer: MediaPlayer? = null
    private var detectFacePlayer: MediaPlayer? = null
    private var haveface = false
    private var save = false
    var rgb2 : Mat? = null
    var rgb3 : Mat? = null

    var page = 0

    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }


    @SuppressLint("MissingPermission")
    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.tutorial2_surface_view)
        views = arrayOf(dis, fat, smoke, call, abnm)

        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCvCameraViewListener(this@DetectActitvity)
            setCameraIndex(1)
            setMaxFrameSize(640, 480)
        }

        progressDialog = ProgressDialog(this)
        progressDialog?.setTitle("加载中,请稍后")
//        progressDialog?.show()

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
        sdPlayer = MediaPlayer.create(this, R.raw.sdcard)
        detectFacePlayer = MediaPlayer.create(this, R.raw.detectdone)


//        timer?.schedule(timerTask, 0, 5000)


    }




    fun playnoface() {
        var time = System.currentTimeMillis() - lastTime[1]
        if (time > 8000) {
            detectFacePlayer?.apply {
                if (register && !this.isPlaying) {
                    lastTime[1] = System.currentTimeMillis()
                    start()
                }
            }
        }
    }

    private fun playWarnning(index: Int) {//每种提示音分开计算，4秒内不重复播放同一种
        var time = System.currentTimeMillis() - lastTime[0]
        if (time > 4000) {
            players[index]?.apply {
                if (!this.isPlaying) {
                    lastTime[0] = System.currentTimeMillis()
                    start()
                }
            }
        }
    }

    fun RegistDone() {
        haveface = true
        register = false
        save = true
        var detectFacePlayerDone = MediaPlayer.create(this, R.raw.detect)
        detectFacePlayerDone.start()
        Log.e("sss", "has registerd --------------- ")
    }

    public override fun onPause() {
        super.onPause()
//        tutorial2_activity_surface_view?.disableView()
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
//        android.os.Process.killProcess(android.os.Process.myPid())
        players.forEach {
            it?.stop()
            it?.release()
        }
        sdPlayer?.stop()
        sdPlayer?.release()
//        timer?.purge()
        System.exit(0)
    }

    override fun onCameraViewStarted(width: Int, height: Int) {
        rgb2 = Mat()
        rgb3 = Mat()
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
        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()
        rgb?.let { it1 ->
//            Imgproc.cvtColor(mGray, it1, Imgproc.COLOR_GRAY2RGB)
            Imgproc.cvtColor(mRgba, it1, Imgproc.COLOR_RGBA2RGB)
//            Core.flip(it1,rgb2,1)
//            Core.rotate(rgb2,rgb3,Core.ROTATE_90_CLOCKWISE)
            if (totalDone)
                mRgba?.let {
//                    if (!haveface && !(detectFacePlayer!!.isPlaying)) {
////                        playnoface()
//                    }
                    FindFeatures2(it1!!.nativeObjAddr,rgb!!.nativeObjAddr, register, save)
                }
        }
        if (index >= 10000)
            index = 0
        index++
        return rgb!!
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
            var sp: SharedPreferences = contexts[0].getPreferences(Context.MODE_PRIVATE)
//            contexts[0].page = sp.getInt("index",0) + 1
            val edi = sp.edit()
            edi.putInt("index", contexts[0].page)
            edi.apply()
            Utils.addModeles(contexts[0], contexts[0].page)
            return contexts[0]
        }
    }

    private var alertDialog: AlertDialog? = null

    internal class AsyncTaskInitTotalFlow : AsyncTask<DetectActitvity, Int, DetectActitvity>() {
        override fun onPostExecute(integer: DetectActitvity) {
            super.onPostExecute(integer)
            integer.totalDone = true
            Log.e(TAG, "AsyncTaskInitTotalFlow  successfully")
        }

        override fun doInBackground(vararg contexts: DetectActitvity): DetectActitvity {
            contexts[0].FindFeatures(0L, contexts[0].page)
            return contexts[0]
        }
    }

    external fun stop()
    //    external fun CHECK(mac:String)
    external fun FindFeatures(matAddrGr: Long, index: Int)

    external fun FindFeatures2(matAddrGr: Long, matAddrRgba: Long, time: Boolean, save: Boolean): Int

    companion object {
        private const val TAG = "OCVSample::Activity"
    }
}
