package org.opencv.samples.tutorial2

import android.annotation.SuppressLint
import android.app.Activity
import android.app.ProgressDialog
import android.media.MediaPlayer
import android.os.*
import android.text.TextUtils
import android.util.Log
import android.view.View
import android.view.WindowManager
import android.widget.TextView
import android.widget.Toast
import com.op.dm.CompressUtil
import com.op.dm.Utils
import com.ut.sdk2.R
import kotlinx.android.synthetic.main.tutorial2_surface_view.*
import org.opencv.android.BaseLoaderCallback
import org.opencv.android.CameraBridgeViewBase
import org.opencv.android.LoaderCallbackInterface
import org.opencv.android.OpenCVLoader
import org.opencv.core.Mat
import org.opencv.imgproc.Imgproc
import java.io.File

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
//    var pathRoot = "/baidu_map/imgs"
    var pathRoot = "/sdcard/imgs"

    var  path = ""
    var arg1 = 1
    var arg2 = "-r"
    var external = false
    var captureThread :Thread? = null

    var CONTROL_NUM = true


    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }


    @SuppressLint("MissingPermission")
    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)
        count = 0
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        var file = File(pathRoot)
        if(!file.exists())
            initDir(pathRoot)
        path = pathRoot+ "/"
        setContentView(R.layout.tutorial2_surface_view)
        var px = intent.getIntExtra("px",640)
        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCameraIndex(1)
            if(px == 720)
                setMaxFrameSize(1280, 960)
            else
                setMaxFrameSize(640, 480)
            setCvCameraViewListener(this@DetectActitvity)
        }

        if(CONTROL_NUM){
            pic_count.visibility = View.VISIBLE
            end.visibility = View.GONE
        }
        else{
            pic_count.visibility = View.GONE
            end.visibility = View.VISIBLE
        }

        captureThread = Thread{
            while (true){
                if(!stop){
                    if (CONTROL_NUM){
                        if( count < max){//&& now > 480
                            if(count == max-1){
                                stop = true
                                runOnUiThread {
                                    textv.text = "结束录制。。。"
                                }
                            }
                            rgb?.nativeObjAddr?.let { FindFeatures(it,path+ page) }
                            count++
                        }
                    }else{
                        rgb?.nativeObjAddr?.let { FindFeatures(it,path+ page) }
                    }
                }
            }
        }
        captureThread?.start()
        start.setOnClickListener {
            stop = false
            page = file_name.text.toString()
            if(TextUtils.isEmpty(page))
                page = "test"
            if(!TextUtils.isEmpty( pic_count.text))
                max = pic_count.text?.toString()?.toInt() ?: 20
            addIndex(this)
            textv.text = "录制中。。。"
            count = 0
        }

        end.setOnClickListener {
            stop = true
            textv.text = "结束录制。。。"
        }

        //压缩
        compress_btn.setOnClickListener {
           if(!comTask.isCompressing){
               progressDialog = ProgressDialog(this)
               progressDialog?.setTitle("加载中,请稍后")
               progressDialog?.setCanceledOnTouchOutside(false)
               progressDialog?.show()
               comTask = ComTask()
               comTask.execute(this@DetectActitvity)
           }
        }

        sdPlayer = MediaPlayer.create(this,R.raw.sdcard)
        detectFacePlayer = MediaPlayer.create(this,R.raw.detect)


        cmdControl()

        delete_btn.setOnClickListener { view ->
            var filename = delete_name.text.toString()
            if(!TextUtils.isEmpty(filename)){
                var file = File(pathRoot + "/" + filename.trim())
//                var file = File("/sdcard/imgs/" + filename.trim())
                if(file.exists()){
                    file.list().forEach {
                        var tmp = File(file.absolutePath +"/"+ it)
                        tmp.delete()
                    }
                    file.delete()
                    Toast.makeText(this@DetectActitvity,"删除成功",Toast.LENGTH_LONG).show()
                }else{
                    Toast.makeText(this@DetectActitvity,"不存在这个文件夹",Toast.LENGTH_LONG).show()
                }

            }
        }

        back_btn.setOnClickListener {
            finish()
        }

    }

    private fun cmdControl() {
        on.setOnClickListener {
            Utils.rumCmd(1, arg2)
            arg1 = 1
            on.setTextColor(resources.getColor(R.color.green))
            off.setTextColor(resources.getColor(R.color.black))
        }

        off.setOnClickListener {
            Utils.rumCmd(0, arg2)
            off.setTextColor(resources.getColor(R.color.green))
            on.setTextColor(resources.getColor(R.color.black))
            arg1 = 0
        }

        rgb1.setOnClickListener {
            Utils.rumCmd(arg1, "-r")
            arg2 = "-r"
            rgb1.setTextColor(resources.getColor(R.color.green))
            ir.setTextColor(resources.getColor(R.color.black))
        }

        ir.setOnClickListener {
            Utils.rumCmd(arg1, "-i")
            arg2 = "-i"
            ir.setTextColor(resources.getColor(R.color.green))
            rgb1.setTextColor(resources.getColor(R.color.black))
        }
    }


    internal class ComTask : AsyncTask<DetectActitvity,Int,DetectActitvity>(){
        override fun doInBackground(vararg params: DetectActitvity): DetectActitvity {
            isCompressing = true
            var dstFile = File("/baidu_map/imgs.zip")
            if (dstFile.exists())
                dstFile.delete()
//            CompressUtil.compress("/sdcard/imgs","/sdcard/imgs.zip")
            CompressUtil.compress(params[0].pathRoot,params[0].pathRoot + ".zip")
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



    var count = 10000
    var max = 20
    var last = 0L
    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat {
        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()
        Imgproc.cvtColor(mRgba,rgb,Imgproc.COLOR_RGBA2BGR)
        return mRgba!!
    }

    var stop :Boolean = true

    public override fun onPause() {
        super.onPause()
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
        tutorial2_activity_surface_view?.disableView()
        players.forEach {
            it?.stop()
            it?.release()
        }

//        System.exit(0)
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


    fun addIndex(context :Activity){
        var path = path  + page
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
                }
            }
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

    //     suspend fun compress():Boolean{
//        var dstFile = File("/baidu_map/imgs.zip")
//        if (dstFile.exists())
//            dstFile.delete()
//        CompressUtil.compress("/sdcard/aa","/sdcard/aa.zip")
//
////        CompressUtil.compress(pathRoot,"/baidu_map/imgs.zip")
//        delay(2000)
//        return true
//    }
//
//    private fun compress2():asyc {
//        val result = async { compress() }
//        if (result.await()){
//            progressDialog?.dismiss()
//        }
//    }
}
