package org.opencv.samples.tutorial2

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.app.ProgressDialog
import android.content.ContentValues
import android.content.ContentValues.TAG
import android.content.Context
import android.content.SharedPreferences
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.PixelFormat
import android.hardware.Camera
import android.hardware.camera2.CameraManager
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
import android.view.*
import android.widget.FrameLayout
import android.widget.TextView
import com.op.dm.ImageServer
import com.op.dm.ImageServer2
import com.op.dm.Utils
import com.op.dm.Utils.getGpsLoaalTime
import com.tencent.bugly.Bugly.init
import com.ut.sdk.R
import com.ut.sdk.R.id.tutorial2_activity_surface_view
import kotlinx.android.synthetic.main.tutorial2_surface_view.*
import org.opencv.android.BaseLoaderCallback
import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.LoaderCallbackInterface
import org.opencv.android.OpenCVLoader
import org.opencv.core.Core
import org.opencv.core.CvType
import org.opencv.core.Mat
import org.opencv.core.MatOfByte
import org.opencv.imgcodecs.Imgcodecs
import org.opencv.imgproc.Imgproc
import java.io.File
import java.io.IOException
import java.lang.StringBuilder
import java.util.*
import java.util.concurrent.Executors
import kotlin.concurrent.timerTask

/**
 * Created by chris on 1/4/19.
 */

class DetectActitvity : Activity(), CameraBridgeViewBase.CvCameraViewListener2 , SurfaceHolder.Callback{
    var openFront = false
    var switchMode = false
    var openTransfer = false
    var camerFront =0
    var camerFace = 1
    internal var index = 0
    var rgb: Mat? = null
    private var mRgba: Mat? = null
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
    var imageServer: ImageServer? = null



    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }
    var singleThreadExecutor = Executors.newSingleThreadExecutor()
    var singleThreadExecutorDsm = Executors.newSingleThreadExecutor()

    var modeln:String? = null
    var simno :String? = null



    @SuppressLint("MissingPermission")
    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)

        firsttime = System.currentTimeMillis()
        locationManager = getSystemService(Context.LOCATION_SERVICE) as LocationManager?
        location = locationManager?.getLastKnownLocation(locationManager?.getBestProvider(getCriteria(), true))



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
        if (openTransfer){
            Thread{
                imageServer = ImageServer()
            }.start()
        }
        getSimId()
        initModeSwitch()

        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCvCameraViewListener(this@DetectActitvity)
            setCameraIndex(camerFace)
            setMaxFrameSize(640, 480)
        }



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

    @SuppressLint("MissingPermission")
    private fun getSimId() {
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP_MR1) {
//            var sm = SubscriptionManager.from(this)
//            // it returns a list with a SubscriptionInfo instance for each simcard
//            // there is other methods to retrieve SubscriptionInfos (see [2])
//            var sis: List<SubscriptionInfo?>? = sm.getActiveSubscriptionInfoList()
//            // getting first SubscriptionInfo
//            var si = sis?.get(0)
//            // getting iccId
//            var iccId = si?.getIccId() ?: "8986043910180000000"
////            var iccId = si?.getIccId() ?: "8986a4c910180000000"
//            Log.e("  iccid  is  ", iccId)
//            if (iccId.length > 11) {
//                var builder = StringBuilder()
//                iccId.toCharArray().forEach {
//                    if(!it.isDigit()){
//                        Log.e(" isDigit ",it.toString())
//                        builder.append(it.toUpperCase())
//                    }else
//                        builder.append(it)
//                }
//
//                modeln = builder.toString().substring(0, 11)
//                simno = "0" + modeln
//                Log.e("  model  inex ", modeln + " - " + simno)
//                mode_text.visibility = View.VISIBLE
//                mode_text.text = "sim: $modeln --dev: $simno"
//            }
//
//        } else {
//            var iccId ="8986043910180000000"
//            if (iccId.length > 11) {
//                modeln = iccId.substring(0, 11)
//                simno = "0" + modeln
//                Log.e("  model  inex ", modeln + " - " + simno)
//                mode_text.text = modeln + " -- " + simno
//            }
//        }

        var iccId ="8986043910180000000"
        if (iccId.length > 11) {
            modeln = iccId.substring(0, 11)
            simno = "0" + modeln
            Log.e("  model  inex ", modeln + " - " + simno)
            mode_text.text = modeln + " -- " + simno
        }

    }

    private fun initModeSwitch() {
        if(!switchMode){
            normal_btn.visibility = View.INVISIBLE
            fast_btn.visibility = View.INVISIBLE
        }
        normal_btn.setOnClickListener {
            Thread {
                var sp = getPreferences(Context.MODE_PRIVATE)
                var ed = sp.edit()
                ed.putLong("mode", 0L)
                ed.apply()
            }.start()
            setResult(RESULT_OK)
            stop()
            totalDone = false
            mode = 0L
            tutorial2_activity_surface_view.visibility = View.INVISIBLE
            tutorial2_activity_surface_view.postDelayed({
                finish()
            }, 500)
        }

        fast_btn.setOnClickListener {
            Thread {
                var sp = getPreferences(Context.MODE_PRIVATE)
                var ed = sp.edit()
                ed.putLong("mode", 1L)
                ed.apply()
            }.start()
            setResult(RESULT_OK)
            stop()
            totalDone = false
            mode = 1L
            tutorial2_activity_surface_view.visibility = View.INVISIBLE
            tutorial2_activity_surface_view.postDelayed({
                finish()
            }, 500)
        }
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


    var lati = 0L
    var longi= 0L
    var alti = 0
    var speeds :Short = 0






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
                    Log.e("sss", "playWarnning ---")
                }
            }
        }
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
            runOnUiThread {
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
        imageServer?.stop()
        onDestroyCamera()
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
    var bm:Bitmap? = null
    override fun onCameraViewStarted(width: Int, height: Int) {
        mRgba = Mat(480,640,CvType.CV_8U)
//        mGray = Mat(480,640,CvType.CV_8U)
        mGray = Mat()
        bm = Bitmap.createBitmap(640, 480, Bitmap.Config.ARGB_8888)
        if (rgb == null) {
            rgb = Mat()
        }
//        th.start()
    }

    override fun onCameraViewStopped() {
        mRgba?.release()
        mGray?.release()
        if (rgb != null)
            rgb?.release()
    }
    var lastdetect = 0L
    var lastFrame = 0L
    var lastFrameDsm = 0L
    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat {
        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()

        if(openTransfer)
            singleThreadExecutor.execute {
                var time = System.currentTimeMillis()
               var byteArray = mRgba!!.toByte()
//                Log.e("sendmat "," " + byteArray.size  + " " + mRgba!!.total() )

//                runOnUiThread {
//                    test_img.visibility = View.VISIBLE
//                    test_img.setImageBitmap(BitmapFactory.decodeByteArray(byteArray,0,byteArray.size))
//                }
                imageServer?.sendMat(byteArray)
            }

//        lastFrame = System.currentTimeMillis()
//
//        var now2 = System.currentTimeMillis() - lastFrameDsm
//        if( now2 < 200){
//            return mRgba!!
//        }
//        return mRgba!!
            rgb?.let { it1 ->
                Imgproc.cvtColor(mRgba, it1, Imgproc.COLOR_RGBA2RGB)
                if (totalDone){

                    mRgba?.let {

                        if(!cali && beginCali){
                            singleThreadExecutorDsm.execute {
                                Cali(it1.nativeObjAddr, 0)
                            }
                        }

                        if(cali && !detectDone){
                            playDetecting()
                            var now = System.currentTimeMillis()
                            var diff = now - lastdetect
                            if(diff > 200){
                                singleThreadExecutorDsm.execute {
                                    Detect(it1.nativeObjAddr, 0)
                                    lastdetect = now
                                }
                            }
                        }

                        if (cali && detectDone){
                            if (true){
//                                singleThreadExecutorDsm.execute {
//                                }
                                var array = FindFeatures2(it1.nativeObjAddr, mRgba!!.nativeObjAddr, register, save)
                                array?.let {
                                    if (it.size > 2)
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
        lastFrameDsm = System.currentTimeMillis()
//        return mRgba!!
//        if (index >= 10000)
//            index = 0
//        index++
        return mGray!!
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
                integer.OnMessage(integer.lati, integer.longi,integer.alti, integer.speeds,false)
            }
            var timer = Timer()
            timer.schedule(timerTask,5000,900)

            var timerTaskGps =  timerTask {
                integer.OnMessage(integer.lati, integer.longi,integer.alti, integer.speeds,true)
            }
            var timer2 = Timer()
            timer2.schedule(timerTaskGps,5000,30000)
            if(integer.openFront)
                integer.onCreate2()
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

    internal class AsyncTaskInitTotalFlow : AsyncTask<DetectActitvity, Int, DetectActitvity>() {
        override fun onPostExecute(integer: DetectActitvity) {
            super.onPostExecute(integer)
            integer.views?.forEachIndexed { index, textView ->
                textView.text = integer.names[index] + " : " + "正常"
            }
            integer.progressDialog?.dismiss()
            integer.register = true
            integer.totalDone = true
            var handler = Handler(Looper.getMainLooper())
            handler.postDelayed({
                integer.caliPlayer?.start()
                Log.e(TAG, "play cali ---  ")
            }, 100)

            Handler(Looper.getMainLooper()).postDelayed({
                integer.beginCali = true
            },4000)

            Handler().postDelayed({
                Utils.deleteFile(integer)
            },10000)
            Log.e(TAG, "AsyncTaskInitTotalFlow  successfully")
        }

        override fun doInBackground(vararg contexts: DetectActitvity): DetectActitvity {
            for (index in 0..6) {
                contexts[0].players[index] = MediaPlayer.create(contexts[0], contexts[0].audio[index])
            }
            contexts[0].FindFeatures(contexts[0].mode, contexts[0].page)
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
    external fun OnMessage(lati: Long,alti: Long,height:Int,speed:Short,gps:Boolean): Boolean
    //    external fun CHECK(mac:String)
    external fun FindFeatures(matAddrGr: Long, index: Int)
    external fun Cali(matAddrGr: Long, index: Int)
    external fun Detect(matAddrGr: Long, index: Int)
    external fun FindFeatures2(matAddrGr: Long, matAddrRgba: Long, time: Boolean, save: Boolean): IntArray

    companion object {
        private const val TAG = "OCVSample::Activity"
    }

    //front camera recording

    private var surfaceView: SurfaceView? = null
    private var camera: Camera? = null
    private var mediaRecorder: MediaRecorder? = null

    fun onCreate2() {
        Log.e("oncreate ", "11")
        // Start foreground service to avoid unexpected kill

        // Create new SurfaceView, set its size to 1x1, move it to the top leftcorner and set this service as a callback
        surfaceView = SurfaceView(this)
        val layoutParams = FrameLayout.LayoutParams(
               (208*resources.displayMetrics.density).toInt(), (156*resources.displayMetrics.density).toInt()
        )
        layoutParams.gravity = Gravity.LEFT or Gravity.TOP
        surfaceView?.setZOrderOnTop(true)
        surfacev.addView(surfaceView, layoutParams)
        surfaceView?.getHolder()?.addCallback(this)

    }

    private fun openFrontFacingCameraGingerbread(): Camera? {
        if (camera != null) {
            camera?.stopPreview()
            camera?.release()
        }
        var cam: Camera? = null
        cam = Camera.open(camerFront)
        return cam
    }

    override fun surfaceCreated(surfaceHolder: SurfaceHolder) {
        Log.e("surfaceCreated ", "111")
        camera = openFrontFacingCameraGingerbread()

        mediaRecorder = MediaRecorder()
        camera?.unlock()

//        camera?.setPreviewDisplay(surfaceHolder)
        mediaRecorder?.let {
            it.setPreviewDisplay(surfaceHolder.surface)
            it.setCamera(camera)
            it.setAudioSource(MediaRecorder.AudioSource.CAMCORDER)
            it.setVideoSource(MediaRecorder.VideoSource.CAMERA)
            it.setProfile(CamcorderProfile.get(CamcorderProfile.QUALITY_480P))
//            it.setVideoFrameRate(12)
//            it.setCaptureRate(12.0)
        }


        val imagesFolder = File(
                Environment.getExternalStorageDirectory(), "frontVideo")
        if (!imagesFolder.exists())
            imagesFolder.mkdirs() // <----
        Log.e("folder ", imagesFolder.path)

        val image = File(imagesFolder, System.currentTimeMillis().toString() + ".mp4")  //file name + extension is .mp4

        mediaRecorder?.setOutputFile(image.absolutePath)

        try {
            mediaRecorder?.prepare()
        } catch (e: Exception) {
        }

        try {
            mediaRecorder?.start()
        } catch (e: Exception) {

        }
    }

    // Stop recording and remove SurfaceView
    fun onDestroyCamera() {
        mediaRecorder?.stop()
        mediaRecorder?.reset()
        mediaRecorder?.release()

        camera?.lock()
        camera?.release()

//        windowManager.removeView(surfaceView)
    }

    override fun surfaceChanged(surfaceHolder: SurfaceHolder, format: Int, width: Int, height: Int) {}

    override fun surfaceDestroyed(surfaceHolder: SurfaceHolder) {}

}
