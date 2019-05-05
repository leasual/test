package org.opencv.samples.tutorial2

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.app.ProgressDialog
import android.content.ContentValues
import android.content.Context
import android.content.SharedPreferences
import android.hardware.Camera
import android.icu.lang.UCharacter.GraphemeClusterBreak.L
import android.location.*
import android.media.CamcorderProfile
import android.media.MediaPlayer
import android.media.MediaRecorder
import android.net.wifi.WifiManager
import android.os.*
import android.provider.MediaStore.Files.FileColumns.MEDIA_TYPE_VIDEO
import android.telephony.SubscriptionInfo
import android.telephony.SubscriptionManager
import android.telephony.TelephonyManager
import android.text.TextUtils
import android.util.Log
import android.view.View
import android.view.WindowManager
import android.widget.TextView
import com.op.dm.Utils
import com.op.dm.Utils.getGpsLoaalTime
import com.tencent.bugly.Bugly.init
import com.ut.sdk.R
import kotlinx.android.synthetic.main.tutorial2_surface_view.*
import org.opencv.android.BaseLoaderCallback
import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.LoaderCallbackInterface
import org.opencv.android.OpenCVLoader
import org.opencv.core.Core
import org.opencv.core.Mat
import org.opencv.imgproc.Imgproc
import java.io.File
import java.io.IOException
import java.util.*
import kotlin.concurrent.timerTask

/**
 * Created by chris on 1/4/19.
 */


class DetectActitvity : Activity(), CameraBridgeViewBase.CvCameraViewListener2 {
    internal var index = 0
    var rgb: Mat? = null
    private var mRgba: Mat? = null
    private var mRgba2: Mat? = null
    private var mGray: Mat? = null
    private var progressDialog: ProgressDialog? = null
    //这个优先级会有变动，为了不修改jni里返回值顺序和ui的顺序，引入这个数组，之后只改这里就可以修改警告播报优先级-> 从names[3],names[2],names[1]..这个顺序遍历names数组
    private val priority = arrayOf(3, 6, 0, 1, 2, 5, 4)
    private val names = arrayOf("左顾右盼", "分神" , "吸烟", "打电话", "画面异常","身份异常","打哈欠")
    private val lastTime = arrayOf(0L, 0L, 0L, 0L, 0L, 0L, 0L)
    private val audio = arrayOf(R.raw.dis2,R.raw.fenshen, R.raw.chouyan, R.raw.dadianhua, R.raw.huamianyichang,R.raw.shenfenyichang, R.raw.yawn)
    private var views: Array<TextView>? = null
    private val strings = arrayOfNulls<String>(7)
    internal var totalDone = false
    internal var register = false
    private var players = arrayOfNulls<MediaPlayer>(7)//每秒有n次检测，即时响应，分多个实例
    private var sdPlayer: MediaPlayer? = null
    private var detectFacePlayer: MediaPlayer? = null
    private var caliPlayer: MediaPlayer? = null
    private var haveface = false
    private var save = false
    private var timer: Timer? = null
    private var timerTask: TimerTask? = null
    var page = 0
    var mode = 0L
    var locationManager: LocationManager? = null
    var location: Location? = null
    var beginCali = false
    var totaltime = 0L
    var firsttime = 0L
    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }


    var  locationListener = object: LocationListener {
        override fun onLocationChanged(location: Location?) {
        }

        override fun onStatusChanged(provider: String?, status: Int, extras: Bundle?) {
            var loc: Location? = getLastKnownLocation()
            Log.e(ContentValues.TAG, "onLocationChanged time is " + getGpsLoaalTime(loc?.time ?: 0))
            Log.e(ContentValues.TAG, "onLocationChanged lat is " + loc?.latitude)
        }

        override fun onProviderEnabled(provider: String?) {
        }

        override fun onProviderDisabled(provider: String?) {
        }

    }


    var modeln:String? = null
    var simno :String? = null
    var mMediaRecorder: MediaRecorder? = null
    var mCamera: Camera? = null


    @SuppressLint("MissingPermission")
    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)
//        var mac = checks()
//        Log.e("mac  dizhi   " , mac)
        firsttime = System.currentTimeMillis()
        locationManager = getSystemService(Context.LOCATION_SERVICE) as LocationManager?
        location = locationManager?.getLastKnownLocation(locationManager?.getBestProvider(getCriteria(), true))
//        locationManager?.requestLocationUpdates(LocationManager.GPS_PROVIDER, 3000, 1f, locationListener)
      if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP_MR1) {
          var sm =  SubscriptionManager.from(this)
            // it returns a list with a SubscriptionInfo instance for each simcard
            // there is other methods to retrieve SubscriptionInfos (see [2])
           var sis:List<SubscriptionInfo?>? = sm.getActiveSubscriptionInfoList()
            // getting first SubscriptionInfo
           var si = sis?.get(0)
            // getting iccId
           var iccId = si?.getIccId()?:"8986043910180000000"
          if(iccId.length > 11){
              modeln = iccId.substring(0,11)
              simno = "0"+ modeln
              Log.e("  model  inex ", modeln + " - " + simno)
          }

       } else {
           TODO("VERSION.SDK_INT < LOLLIPOP_MR1")
       }

        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        setContentView(R.layout.tutorial2_surface_view)
        views = arrayOf(dis, fat, smoke, call, abnm,unknown,yawn)
        mode = getPreferences(Context.MODE_PRIVATE).getLong("mode",0L)
        if(mode == 1L){
            mode_text.text = "当前:快速模式"
        }
//        val cameraManager = getSystemService(Context.CAMERA_SERVICE) as CameraManager
//        val cameraId = cameraManager.cameraIdList[0]
//        cameraManager.cameraIdList.forEach {
//            Log.e("  camera  inex ", it)
//        }

        normal_btn.setOnClickListener {
            Thread {
                var sp = getPreferences(Context.MODE_PRIVATE)
                var ed = sp.edit()
                ed.putLong("mode",0L)
                ed.apply()
            }.start()
            setResult(RESULT_OK)
            stop()
            totalDone = false
            mode = 0L
            tutorial2_activity_surface_view.visibility = View.INVISIBLE
            tutorial2_activity_surface_view.postDelayed({
                finish()
            },500)
        }

        fast_btn.setOnClickListener {
            Thread {
                var sp = getPreferences(Context.MODE_PRIVATE)
                var ed = sp.edit()
                ed.putLong("mode",1L)
                ed.apply()
            }.start()
            setResult(RESULT_OK)
            stop()
            totalDone = false
            mode = 1L
            tutorial2_activity_surface_view.visibility = View.INVISIBLE
            tutorial2_activity_surface_view.postDelayed({
                finish()
            },500)
        }

        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCvCameraViewListener(this@DetectActitvity)
            setCameraIndex(1)
            setMaxFrameSize(640, 480)
        }

//        with(tutorial2_activity_surface_view2) {
//            visibility = CameraBridgeViewBase.VISIBLE
//            setCvCameraViewListener(object :CameraBridgeViewBase.CvCameraViewListener2{
//                override fun onCameraViewStarted(width: Int, height: Int) {
//                    mRgba2 = Mat()
//                }
//
//                override fun onCameraViewStopped() {
//                }
//
//                override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame?): Mat {
//                    mRgba2 = inputFrame?.rgba()
//                    return mRgba2!!
//                }
//
//            })
//            setCameraIndex(0)
//            setMaxFrameSize(640, 480)
//        }

        progressDialog = ProgressDialog(this)
        progressDialog?.setTitle("加载中,请稍后")
        progressDialog?.show()

        sdPlayer = MediaPlayer.create(this, R.raw.sdcard)

        detectFacePlayer = MediaPlayer.create(this, R.raw.detecting)
        caliPlayer = MediaPlayer.create(this, R.raw.cali)

        timerTask = kotlin.concurrent.timerTask {
            //TODO::加上换卡创建文件夹
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
        timer?.schedule(timerTask, 0, 3000)
    }
    var lati = 0L
    var longi= 0L
    var alti = 0
    var speeds :Short = 0

    private fun releaseMediaRecorder() {
        if (mMediaRecorder != null) {
            mMediaRecorder?.reset()   // clear recorder configuration
            mMediaRecorder?.release() // release the recorder object
            mMediaRecorder = null
            mCamera?.lock()           // lock camera for later use
        }
    }

    private fun prepareVideoRecorder(): Boolean {
        mCamera = Camera.open(0)
        mCamera?.let {
            mMediaRecorder = MediaRecorder()

            // Step 1: Unlock and set camera to MediaRecorder
            mCamera?.unlock()
            mMediaRecorder?.setCamera(mCamera)

            // Step 2: Set sources
//            mMediaRecorder?.setAudioSource(MediaRecorder.AudioSource.DEFAULT)
            mMediaRecorder?.setVideoSource(MediaRecorder.VideoSource.CAMERA)

            // Step 3: Set a CamcorderProfile (requires API Level 8 or higher)
            mMediaRecorder?.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_480P))

            val mediaStorageDir = File(Environment.getExternalStoragePublicDirectory(
                    Environment.DIRECTORY_PICTURES), "MyCameraApp")
            if (!mediaStorageDir.exists()) {
                if (!mediaStorageDir.mkdirs()) {
                    Log.e("MyCameraApp", "failed to create directory")
                    return false
                }
            }
            val mediaFile: File
            mediaFile = File(mediaStorageDir.path + File.separator +
                    "qianshe" + ".mp4")

            //        mMediaRecorder.setOutputFormat(MediaRecorder.OutputFormat.MPEG_4);
            //        mMediaRecorder.setVideoEncoder(MediaRecorder.VideoEncoder.DEFAULT);
            // Step 4: Set output file
            mMediaRecorder?.setOutputFile(mediaFile.toString())
            //        mMediaRecorder.setVideoFrameRate(25);

            // Step 5: Set the preview output
//        mMediaRecorder?.setPreviewDisplay(mPreview.getHolder().getSurface())

            // Step 6: Prepare configured MediaRecorder
            try {
                mMediaRecorder?.prepare()
            } catch (e: IllegalStateException) {
                Log.d(TAG, "IllegalStateException preparing MediaRecorder: " + e.message)
                releaseMediaRecorder()
                return false
            } catch (e: IOException) {
                Log.d(TAG, "IOException preparing MediaRecorder: " + e.message)
                releaseMediaRecorder()
                return false
            }
        }
        return true
    }


    @SuppressLint("MissingPermission")
    private fun getLastKnownLocation(): Location? {
//        val locationManager = getSystemService(Context.LOCATION_SERVICE) as LocationManager
        val providers = locationManager?.allProviders
        var bestLocation: Location? = null
        var times = 0L
        if (providers != null) {
            for (provider in providers) {
                val l = locationManager?.getLastKnownLocation(provider) ?: continue
                l?.apply {
                    if(times < time){
                        bestLocation = l
                        times = time
                        if(l.latitude > 1)
                            lati = (l.latitude * 1000000).toLong()
                        if(l.longitude > 1)
                            longi = (l.longitude*1000000).toLong()
                        alti = l.altitude.toInt()
                        speeds = speed.toShort()

                    }
                }
            Log.e(ContentValues.TAG, " time is " + getGpsLoaalTime(l.time ?: 0) + " type- "+provider)
            Log.e(ContentValues.TAG, " gps is " + l.latitude + "  " + l.longitude + " type- "+provider + " speed " + l.speed)

            }
        }
        return bestLocation
    }



    private fun getStringResult(result: IntArray?) {
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

    fun playDetecting() {
        var time = System.currentTimeMillis() - lastTime[1]
        if(time > 5000 && playdetect){
            detectFacePlayer?.apply {
                if (register && !this.isPlaying) {
                    lastTime[1] = System.currentTimeMillis()
                    start()
                }
            }
        }
        Log.e("sss", "playDetecting ---")
    }

    private fun playWarnning(index: Int) {//每种提示音分开计算，4秒内不重复播放同一种
        if(index == 5)
            return
        var time = System.currentTimeMillis() - lastTime[0]
        var playing = false
        players.forEach {

        }
        if (time > 4000) {
            players[index]?.apply {
                if (!this.isPlaying) {
                    lastTime[0] = System.currentTimeMillis()
                    start()
                }
            }
        }
        Log.e("sss", "playWarnning ---")
    }
    var cali = false
    var detectDone = false
    fun caliDone() {

        Handler(Looper.getMainLooper()).postDelayed({
            playDetecting()
        },3000)

        Handler(Looper.getMainLooper()).postDelayed({
            cali = true
        },7000)

    }
    var playdetect = true
    fun RegistDone() {
        var detectFacePlayerDone = MediaPlayer.create(this, R.raw.detectdone)

        Handler(Looper.getMainLooper()).postDelayed({
            if(detectFacePlayer?.isPlaying == true){
                detectFacePlayer?.stop()
            }
            detectFacePlayerDone.start()
            playdetect = false
            Log.e("sss", "detectFacePlayerDone --------------- ")
        },1000)

        Handler(Looper.getMainLooper()).postDelayed({
            haveface = true
            register = false
            detectDone = true
//            save = true
            Log.e("sss", "regist done --------------- ")
        },9000)


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

//    @SuppressLint("MissingPermission")
//    fun GetTime():Long{
//        Log.e("Location ______ ", " "+(location?.time?:0) )
//        return location?.time?:0
//    }

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
    }

    public override fun onResume() {
        super.onResume()
//        mLoaderCallback.onManagerConnected(LoaderCallbackInterface.SUCCESS)
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
        totalDone = false
        stop()
        tutorial2_activity_surface_view.visibility = View.INVISIBLE
        tutorial2_activity_surface_view?.disableView()
//        tutorial2_activity_surface_view2?.disableView()
//        android.os.Process.killProcess(android.os.Process.myPid())
        players.forEach {
            it?.stop()
            it?.release()
        }
        sdPlayer?.stop()
        sdPlayer?.release()
//        timer?.purge()
        timerTask?.cancel()
        timer?.cancel()
        Handler().post {
//            Utils.deleteFileAll(this)
//            System.exit(0)
        }
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
    var lastdetect = 0L
    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat {

        totaltime = System.currentTimeMillis()- firsttime
//        if(totaltime > 1000*60*60*3)
//            finish()

        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()
        rgb?.let { it1 ->
            Imgproc.cvtColor(mRgba, it1, Imgproc.COLOR_RGBA2RGB)
            if (totalDone){
                mRgba?.let {

                    if(!cali && beginCali){
                        Cali(it1.nativeObjAddr,0)
                    }

                    if(cali && !detectDone){
                        playDetecting()
                        var now = System.currentTimeMillis()
                        var diff = now - lastdetect
                        if(diff > 200){
                            Detect(it1.nativeObjAddr,0)
                            lastdetect = now
                        }
                    }

                    if (cali && detectDone){
                        if (true){
                            var array = FindFeatures2(it1.nativeObjAddr, it.nativeObjAddr, register, save)
                            array?.let {
                                if(it.size > 2)
                                    getStringResult(it)
                            }
                        }//减少uitext更新频率，没必要每帧都改变
//                        else{
//                            FindFeatures2(it1.nativeObjAddr, it.nativeObjAddr, register, save)
//                        }
                    }

                }
            }

        }
        if (index >= 10000)
            index = 0
        index++
        return mRgba!!
    }

    private val mLoaderCallback = object : BaseLoaderCallback(this) {
        @SuppressLint("MissingPermission")
        override fun onManagerConnected(status: Int) {
            when (status) {
                LoaderCallbackInterface.SUCCESS -> {
                    Log.e(TAG, "OpenCV loaded successfully")
                    System.loadLibrary("native-lib")
                    tutorial2_activity_surface_view?.enableView()
//                    tutorial2_activity_surface_view2?.enableView()
                    var timerTask =  timerTask {
                        runOnUiThread {
                            locationManager?.requestLocationUpdates(LocationManager.GPS_PROVIDER, 3000, 1f, locationListener)
                            locationManager?.requestLocationUpdates(LocationManager.PASSIVE_PROVIDER, 3000, 1f, locationListener)
                            locationManager?.requestLocationUpdates(LocationManager.NETWORK_PROVIDER, 3000, 1f, locationListener)
                            getLastKnownLocation()
                        }

                    }
                    var timer = Timer()
                    timer.schedule(timerTask,3000,5000)


//                    if(prepareVideoRecorder()){
//                        mMediaRecorder?.start()
//                    }
//
//                    Handler().postDelayed({
//                        mMediaRecorder?.stop()
//                        releaseMediaRecorder() // release the MediaRecorder object
//                        mCamera?.lock()
//                    },8000)

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

            integer.modeln?.let { integer.CHECK(it) }

            Handler().postDelayed({
                AsyncTaskInitTotalFlow().execute(integer)
            },2000)

            var timerTask =  timerTask {
//                integer.runOnUiThread {
//                }
                integer.OnMessage(integer.lati, integer.longi,integer.alti, integer.speeds)

            }
            var timer = Timer()
            timer.schedule(timerTask,5000,3000)


        }

        override fun doInBackground(vararg contexts: DetectActitvity): DetectActitvity {
            var sp: SharedPreferences = contexts[0].getPreferences(Context.MODE_PRIVATE)
            contexts[0].page = sp.getInt("index",0) + 1
            val edi = sp.edit()
            edi.putInt("index",contexts[0].page)
            edi.apply()
//            Utils.deleteFileAll(contexts[0])
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
            var handler = Handler(Looper.getMainLooper())
            handler.postDelayed({
                integer.caliPlayer?.start()
                Log.e(TAG, "play cali ---  ")
            }, 100)

            Handler(Looper.getMainLooper()).postDelayed({
                integer.beginCali = true
            },8000)

            Handler().postDelayed({
//                Utils.deleteFile(integer)
            },10000)
            Log.e(TAG, "AsyncTaskInitTotalFlow  successfully")
        }

        override fun doInBackground(vararg contexts: DetectActitvity): DetectActitvity {
            for (index in 0..6) {
                contexts[0].players[index] = MediaPlayer.create(contexts[0], contexts[0].audio[index])
            }
//            var tim = System.currentTimeMillis()/1000 + 1*60*60*24*10
            contexts[0].FindFeatures(contexts[0].mode, contexts[0].page)
//            contexts[0].CHECK("")
//            if(contexts[0].CHECK(contexts[0].checks()))
//                contexts[0].FindFeatures(0, 0)
//            else
//                contexts[0].finish()
            return contexts[0]
        }
    }

    override fun onBackPressed() {
        super.onBackPressed()
        setResult(-2)
        finish()
    }

    external fun stop()
    external fun CHECK(mac: String): Boolean
    external fun OnMessage(lati: Long,alti: Long,height:Int,speed:Short): Boolean
    //    external fun CHECK(mac:String)
    external fun FindFeatures(matAddrGr: Long, index: Int)
    external fun Cali(matAddrGr: Long, index: Int)
    external fun Detect(matAddrGr: Long, index: Int)
    external fun FindFeatures2(matAddrGr: Long, matAddrRgba: Long, time: Boolean, save: Boolean): IntArray

    companion object {
        private const val TAG = "OCVSample::Activity"
    }
}
