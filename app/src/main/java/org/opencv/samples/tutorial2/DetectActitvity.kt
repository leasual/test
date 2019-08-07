package org.opencv.samples.tutorial2

import android.annotation.SuppressLint
import android.app.Activity
import android.app.AlertDialog
import android.app.ProgressDialog
import android.media.MediaPlayer
import android.os.*
import android.text.TextUtils
import android.util.Log
import android.view.WindowManager
import android.widget.TextView
import com.op.dm.CompressUtil
import com.op.dm.Utils
import com.ut.sdk2.R
import kotlinx.android.synthetic.main.tutorial2_surface_view.*
import org.opencv.android.BaseLoaderCallback
import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.LoaderCallbackInterface
import org.opencv.android.OpenCVLoader
import org.opencv.core.Mat
import org.opencv.core.Size
import org.opencv.imgproc.Imgproc
import java.io.File
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
    private val names = arrayOf("分神", "疲劳", "吸烟", "打电话", "画面异常")
    private val lastTime = arrayOf(0L,0L,0L,0L,0L)
    private val audio = arrayOf(R.raw.fenshen, R.raw.pilao, R.raw.chouyan, R.raw.dadianhua, R.raw.huamianyichang)
    private var views: Array<TextView>? = null
    internal var totalDone = false
    internal var register = false
    private var players = arrayOfNulls<MediaPlayer>(5)//每秒有n次检测，即时响应，分多个实例
    private var sdPlayer:MediaPlayer? = null
    private var detectFacePlayer:MediaPlayer? = null
    var page = "test"
    var pathRoot = "/storage/sdcard1"
    var arg1 = 1
    var arg2 = "-r"
    var external = false
    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }


    @SuppressLint("MissingPermission")
    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)
//        var mac = checks()
//        Log.e("mac  dizhi   " , mac)
        count = 0
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)

        if(true){
//           pathRoot = "/sdcard"
           pathRoot = "/baidu_map/" + "imgs"

//            pathRoot += "/record_pic"

            var file = File(pathRoot)
            if(!file.exists())
                initDir(pathRoot)
            pathRoot = pathRoot+ "/"

//            stop = false

        }

        setContentView(R.layout.tutorial2_surface_view)
//        views = arrayOf(dis, fat, smoke, call, abnm)
        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCameraIndex(1)
            setMaxFrameSize(1280, 960)
            setCvCameraViewListener(this@DetectActitvity)
//            setMaxFrameSize(640, 480)
        }

        start.setOnClickListener {
            stop = false
            page = file_name.text.toString()
            if(TextUtils.isEmpty(page))
                page = "test"
            if(!TextUtils.isEmpty( pic_count.text))
                max = pic_count.text?.toString()?.toInt() ?: 5
            addIndex(this)
            recorde = true
            textv.text = "录制中。。。"
            count = 0
//            Thread{
//                var time = System.currentTimeMillis()
//               for (i in 0..10){
//                   Thread.sleep(200)
//                   Log.e("thread  is  sleeping ", "  iiuuu  ")
//               }
//                recorde = false
//                runOnUiThread {
//                    textv.text = "结束录制。。。"
//                }
//            }.start()

        }

        end.setOnClickListener {
            stop = true
            runOnUiThread {
                textv.text = "结束录制。。。"
            }
        }

        stop_btn.setOnClickListener {
            if(!comTask.isCompressing){
                progressDialog = ProgressDialog(this)
                progressDialog?.setTitle("加载中,请稍后")
                progressDialog?.show()
                progressDialog?.setCanceledOnTouchOutside(false)
                comTask = ComTask()
                comTask.execute(this@DetectActitvity)
            }
        }

        sdPlayer = MediaPlayer.create(this,R.raw.sdcard)
        detectFacePlayer = MediaPlayer.create(this,R.raw.detect)


        on.setOnClickListener {
            Utils.rumCmd(1,arg2)
            arg1 = 1
            on.setTextColor(resources.getColor(R.color.green))
            off.setTextColor(resources.getColor(R.color.black))
        }

        off.setOnClickListener {
            Utils.rumCmd(0,arg2)
            off.setTextColor(resources.getColor(R.color.green))
            on.setTextColor(resources.getColor(R.color.black))
            arg1 = 0
        }

        rgb1.setOnClickListener {
            Utils.rumCmd(arg1,"-r")
            arg2 = "-r"
            rgb1.setTextColor(resources.getColor(R.color.green))
            ir.setTextColor(resources.getColor(R.color.black))
        }

        ir.setOnClickListener {
            Utils.rumCmd(arg1,"-i")
            arg2 = "-i"
            ir.setTextColor(resources.getColor(R.color.green))
            rgb1.setTextColor(resources.getColor(R.color.black))
        }



//        timer = Timer()
//        timer?.schedule(timerTask,0,5000)

    }


    internal class ComTask : AsyncTask<DetectActitvity,Int,DetectActitvity>(){
        override fun doInBackground(vararg params: DetectActitvity): DetectActitvity {
            isCompressing = true
            var dstFile = File("/baidu_map/imgs.zip")
            if (dstFile.exists())
                dstFile.delete()
            CompressUtil.compress("/sdcard/aa","/sdcard/aa.zip")
            CompressUtil.compress("/baidu_map/imgs","/baidu_map/imgs.zip")
            return params[0]
        }
        var  isCompressing = false
        override fun onPostExecute(result: DetectActitvity) {
            super.onPostExecute(result)
            isCompressing = false
            result.progressDialog?.dismiss()
        }
    }

    private var comTask = ComTask()

    var count = 0
    var max = 5
    var last = 0L
//    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat {
//        var now = System.currentTimeMillis() - last
//
//        mRgba = inputFrame.rgba()
//        mGray = inputFrame.gray()
//        if( count < max){//&& now > 480
//            if(count == max-1)
//                runOnUiThread {
//                    textv.text = "结束录制。。。"
//                }
//            last = System.currentTimeMillis()
//            Imgproc.cvtColor(mGray,rgb,Imgproc.COLOR_GRAY2BGR)
////            Imgproc.cvtColor(mRgba,rgb,Imgproc.COLOR_RGBA2BGR)
//
//            rgb = rgb?.submat(0,720,160,960+160)
//            var size = Size(640.0,480.0)
//            Imgproc.resize(rgb,rgb,size)
//            rgb?.nativeObjAddr?.let { FindFeatures(it,pathRoot+ page) }
//            count++
//        }
//
//        return mGray!!
//    }

    var stop :Boolean = true

    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat {
        var now = System.currentTimeMillis() - last

        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()
//        Imgproc.cvtColor(mRgba,rgb,Imgproc.COLOR_RGBA2RGB)
        Imgproc.cvtColor(mGray,rgb,Imgproc.COLOR_GRAY2BGR)
        if( !stop && now >40){//&& now > 480
            if(stop)
                runOnUiThread {
                    textv.text = "结束录制。。。"
                }
            last = System.currentTimeMillis()

            Imgproc.cvtColor(mGray,rgb,Imgproc.COLOR_GRAY2BGR)

//            rgb = rgb?.submat(0,720,160,960+160)
//            var size = Size(640.0,480.0)
//            Imgproc.resize(rgb,rgb,size)


            rgb?.nativeObjAddr?.let { FindFeatures(it,pathRoot+ page) }

        }

        return rgb!!
    }



    private fun playWarnning(index: Int) {//每种提示音分开计算，4秒内不重复播放同一种
        var time = System.currentTimeMillis()- lastTime[0]
        if(time > 4000){
            if(index == 100){
                detectFacePlayer?.apply {
                    if(register && !this.isPlaying){
                        start()
                    }
                }
            }else{
                players[index]?.apply {
                    if (!this.isPlaying){
                        lastTime[0] = System.currentTimeMillis()
                        start()
                    }
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
//        stop()
        tutorial2_activity_surface_view?.disableView()
//        android.os.Process.killProcess(android.os.Process.myPid())
        players.forEach {
            it?.stop()
            it?.release()
        }
//        sdPlayer?.stop()
//        sdPlayer?.release()
////        timer?.purge()
//        timer?.cancel()
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

    var recording = false
    var recorde = false

    fun addIndex(context :Activity){
        var path = pathRoot  + page
        initDir(path)
        Log.e(" file  directory ",path)

        path_str.text = path
    }

    private fun initDir(path: String) {
        val images = File(path)
        if (!images.exists()) {
            images.mkdir()
        }
    }


    private val mLoaderCallback = object : BaseLoaderCallback(this) {
        override fun onManagerConnected(status: Int) {
            when (status) {
                LoaderCallbackInterface.SUCCESS -> {
                    Log.e(TAG, "OpenCV loaded successfully")
                    System.loadLibrary("native-lib")
                    tutorial2_activity_surface_view?.enableView()
//                    FindFeatures(0,0)
//                    if (!totalDone) {
//                        val context = mAppContext
//                        AsyncTaskInitFile().execute(context as DetectActitvity)
//                    }
                }
//                else -> {
//                    super.onManagerConnected(status)
//                }
            }
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
            for (index in 0..4){
                contexts[0].players[index] = MediaPlayer.create(contexts[0],contexts[0].audio[index])
            }
//            contexts[0].FindFeatures(0, "")
            var handler = Handler(Looper.getMainLooper());
            handler.postDelayed({
                contexts[0].playWarnning(100)
            },10000)

//            if(contexts[0].CHECK(contexts[0].checks()))
//                contexts[0].FindFeatures(0, 0)
//            else
//                contexts[0].finish()
            return contexts[0]
        }
    }

    external fun stop()
    external fun CHECK(mac: String):Boolean
    //    external fun CHECK(mac:String)
    external fun FindFeatures(matAddrGr: Long, dir: String)
    external fun FindFeatures2(matAddrGr: Long, matAddrRgba: Long, time: Boolean, save: Boolean): IntArray

    companion object {
        private const val TAG = "OCVSample::Activity"
    }
}
