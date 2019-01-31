package org.opencv.samples.tutorial2

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.app.ProgressDialog
import android.content.Context
import android.content.SharedPreferences
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
    private val priority = arrayOf(3, 2, 0, 1, 4)
    private val names = arrayOf("分神", "疲劳", "吸烟", "打电话", "画面异常")
    private val lastTime = arrayOf(0L, 0L, 0L, 0L, 0L)
    private val audio = arrayOf(R.raw.fenshen, R.raw.pilao, R.raw.chouyan, R.raw.dadianhua, R.raw.huamianyichang)
    private var views: Array<TextView>? = null
    private val strings = arrayOfNulls<String>(5)
    internal var totalDone = false
    internal var register = false
    private var players = arrayOfNulls<MediaPlayer>(5)//每秒有n次检测，即时响应，分多个实例
    private var sdPlayer: MediaPlayer? = null
    private var detectFacePlayer: MediaPlayer? = null
    private var haveface = false
    private var save = false
    private var timer: Timer? = null
    private var timerTask: TimerTask? = null
    var page = 0
    var lm: LocationManager? = null

    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }

    private fun getAndroidLowVersionMac(wifiManager: WifiManager): String {
        try {
            val wifiInfo = wifiManager.connectionInfo
            val mac = wifiInfo.macAddress
            return if (TextUtils.isEmpty(mac)) {
                "null"
            } else {
                mac.substring(0, 5)
            }
        } catch (e: Exception) {
            e.printStackTrace()
            Log.e("mac", "get android low version mac error:" + e.message)
            return "null"
        }

    }

    var  locationListener = object: LocationListener {
        override fun onLocationChanged(location: Location?) {
        }

        override fun onStatusChanged(provider: String?, status: Int, extras: Bundle?) {
            when (status) {
                // GPS状态为可见时
                LocationProvider.AVAILABLE->
                Log.i(TAG, "当前GPS状态为可见状态")
                // GPS状态为服务区外时
                 LocationProvider.OUT_OF_SERVICE->
                Log.i(TAG, "当前GPS状态为服务区外状态");
            }
        }

        override fun onProviderEnabled(provider: String?) {
        }

        override fun onProviderDisabled(provider: String?) {
        }

    }


    var location:Location? = null
    @SuppressLint("MissingPermission")
    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)
//        var mac = checks()
//        Log.e("mac  dizhi   " , mac)
        lm = getSystemService(Context.LOCATION_SERVICE) as LocationManager?
        location = lm?.getLastKnownLocation(lm?.getBestProvider(getCriteria(), true))
        lm?.requestLocationUpdates(LocationManager.GPS_PROVIDER, 1000, 1f, locationListener)


        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.tutorial2_surface_view)
        views = arrayOf(dis, fat, smoke, call, abnm)

//        val cameraManager = getSystemService(Context.CAMERA_SERVICE) as CameraManager
//        val cameraId = cameraManager.cameraIdList[0]
//        cameraManager.cameraIdList.forEach {
//            Log.e("  camera  inex ", it)
//        }


        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCvCameraViewListener(this@DetectActitvity)
            setCameraIndex(1)

//            setMaxFrameSize(1280, 720)
            setMaxFrameSize(640, 480)

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
        sdPlayer = MediaPlayer.create(this, R.raw.sdcard)
        detectFacePlayer = MediaPlayer.create(this, R.raw.detectdone)

        timerTask = kotlin.concurrent.timerTask {
            var size = Utils.getSDAvailableSize(this@DetectActitvity)
            save = size > 500
            Log.e(" save  ", save.toString())
            if (size > 0 && size < 500 && !(sdPlayer?.isPlaying() ?: false)) {
                runOnUiThread {
                    sdPlayer?.start()
                }
            }
        }
        timer = Timer()
//        timer?.schedule(timerTask, 0, 5000)



    }

    private fun checks(): String {
        var mac = getAndroidLowVersionMac(applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager)
        return mac
    }

    private fun getStringResult(result: IntArray?) {

//        var ts = Utils.getGpsLoaalTime( GetTime())
//        Log.e("Location ______ ", ts)

        result?.let { array ->
            priority.forEach {
                // access the uiText and Jni result by the order of 'priority'
                when (array[it]) {//jni function return value which represents each item's status --> fat, call, smoke etc.
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

    fun playnoface() {
        var time = System.currentTimeMillis() - lastTime[1]
        if(time > 8000){
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

    private fun getCriteria(): Criteria {
        val criteria = Criteria()
        // 设置定位精确度 Criteria.ACCURACY_COARSE比较粗略，Criteria.ACCURACY_FINE则比较精细
        criteria.accuracy = Criteria.ACCURACY_FINE
        // 设置是否要求速度
        criteria.isSpeedRequired = false
        // 设置是否允许运营商收费
        criteria.isCostAllowed = false
        // 设置是否需要方位信息
        criteria.isBearingRequired = false
        // 设置是否需要海拔信息
        criteria.isAltitudeRequired = false
        // 设置对电源的需求
        criteria.powerRequirement = Criteria.POWER_LOW
        return criteria
    }

    @SuppressLint("MissingPermission")
    fun GetTime():Long{
        Log.e("Location ______ ", " "+(location?.time?:0) )
        return location?.time?:0
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
        timer?.cancel()
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

        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()
        rgb?.let { it1 ->
            Imgproc.cvtColor(mRgba, it1, Imgproc.COLOR_RGBA2RGB)
            if (totalDone)
                mRgba?.let {
//                    Log.e("------- onc ----", "" +haveface + " -- " + detectFacePlayer!!.isPlaying)
                    if (!haveface && !(detectFacePlayer!!.isPlaying)) {
                        playnoface()
                    }
                    //                    if(index % 2 == 0){//每两帧调用一次算法
//                    Log.e("save is --", save.toString())
                    if (index % 3 == 0)//减少uitext更新频率，没必要每帧都改变
                        getStringResult(FindFeatures2(it1.nativeObjAddr, it.nativeObjAddr, register, save))
                    else
                        FindFeatures2(it1.nativeObjAddr, it.nativeObjAddr, register, save)
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
            var sp: SharedPreferences = contexts[0].getPreferences(Context.MODE_PRIVATE)
            contexts[0].page = sp.getInt("index",0) + 1
            val edi = sp.edit()
            edi.putInt("index",contexts[0].page)
            edi.apply()
            Utils.addModeles(contexts[0],contexts[0].page)
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
//            integer.alertDialog?.show()
            integer.register = true
            integer.totalDone = true
            Log.e(TAG, "AsyncTaskInitTotalFlow  successfully")
        }

        override fun doInBackground(vararg contexts: DetectActitvity): DetectActitvity {
            for (index in 0..4) {
                contexts[0].players[index] = MediaPlayer.create(contexts[0], contexts[0].audio[index])
            }
            var tim = System.currentTimeMillis()/1000 + 1*60*60*24*10
            contexts[0].FindFeatures(tim, contexts[0].page)

            var handler = Handler(Looper.getMainLooper())
            handler.postDelayed({
                contexts[0].playnoface()
                Log.e(TAG, "play warning ---  ")
            }, 10000)
//            if(contexts[0].CHECK(contexts[0].checks()))
//                contexts[0].FindFeatures(0, 0)
//            else
//                contexts[0].finish()
            return contexts[0]
        }
    }

    external fun stop()
    external fun CHECK(mac: String): Boolean
    //    external fun CHECK(mac:String)
    external fun FindFeatures(matAddrGr: Long, index: Int)

    external fun FindFeatures2(matAddrGr: Long, matAddrRgba: Long, time: Boolean, save: Boolean): IntArray

    companion object {
        private const val TAG = "OCVSample::Activity"
    }
}
