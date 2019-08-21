package org.opencv.samples.tutorial2

import android.Manifest
import android.annotation.SuppressLint
import android.app.Activity
import android.app.ProgressDialog
import android.content.ContentValues.TAG
import android.content.Context
import android.content.pm.PackageManager
import android.graphics.Rect
import android.media.MediaPlayer
import android.os.Bundle
import android.support.v4.app.ActivityCompat
import android.support.v4.content.ContextCompat
import android.text.TextUtils
import android.util.Log
import android.view.KeyEvent
import android.view.View
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import android.widget.Toast
import com.google.protobuf.ByteString
import com.op.dm.ImageDataProto
import com.op.dm.Utils
import com.tencent.bugly.Bugly.init
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
import java.io.FileOutputStream
import java.io.FileWriter
import java.util.concurrent.LinkedBlockingQueue

/**
 * Created by chris on 1/4/19.
 */


class DetectActitvity : Activity(), CameraBridgeViewBase.CvCameraViewListener2 {
    var rgb: Mat? = null
    private var mRgba: Mat? = null
    private var mGray: Mat? = null
    private var progressDialog: ProgressDialog? = null
    //这个优先级会有变动，为了不修改jni里返回值顺序和ui的顺序，引入这个数组，之后只改这里就可以修改警告播报优先级-> 从names[3],names[2],names[1]..这个顺序遍历names数组
    private var players = arrayOfNulls<MediaPlayer>(5)//每秒有n次检测，即时响应，分多个实例
    private var sdPlayer:MediaPlayer? = null
    private var detectFacePlayer:MediaPlayer? = null
    var page = "test"
//    var interalPath = "/sdcard/imgs"
    val interalPath = "/baidu_map/imgs"
    val usbPath ="/storage/usb0/imgs"
    //    var pathRoot = "/baidu_map/imgs"
    var pathRoot = interalPath

    var matQueue = LinkedBlockingQueue<Mat>(30)

    var  path = ""
    var arg1 = "1"
    var arg2 = "IR"
    var captureThread :Thread? = null

    var CONTROL_NUM = true
    var OneFile = false

    var timeControl = 0L

    init {
        Log.i(TAG, "Instantiated new " + this.javaClass)
    }

    var builder : ImageDataProto.ImageData.Builder = ImageDataProto.ImageData.newBuilder()
    var time:String = ""
    var imgData : ImageDataProto.ImageData? = null
    @SuppressLint("MissingPermission")
    public override fun onCreate(savedInstanceState: Bundle?) {
        Log.i(TAG, "called onCreate")
        super.onCreate(savedInstanceState)
        count = 0
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        window.addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN)
        setContentView(R.layout.tutorial2_surface_view)
        var check = intent.getIntExtra("store",R.id.internal)
        if(check == R.id.usb){
            pathRoot = usbPath
            Log.e("is use ", " true --------")
        }
        var permissionCheck = ContextCompat.checkSelfPermission(this,
        Manifest.permission.CAMERA)
        if(permissionCheck == PackageManager.PERMISSION_GRANTED){
            init()

        }else{
            ActivityCompat.requestPermissions(this,
                    arrayOf(Manifest.permission.CAMERA), 100)
        }

        start.postDelayed({
//            var rect = Rect()
            val arry = IntArray(2)

            start.getLocationOnScreen(arry)
            Log.e(" start", "start "+arry[0] + " "+ arry[1])

            file_name.getLocationOnScreen(arry)
            Log.e(" file_name", "start "+arry[0] + " "+ arry[1])

            pic_count.getLocationOnScreen(arry)
            Log.e(" pic_count", "start "+arry[0] + " "+ arry[1])

            delete_name.getLocationOnScreen(arry)
            Log.e(" delete_name", "start "+arry[0] + " "+ arry[1])

            delete_btn.getLocationOnScreen(arry)
            Log.e(" delete_btn", "start "+arry[0] + " "+ arry[1])

            clear_name.getLocationOnScreen(arry)
            Log.e(" clear_name", "start "+arry[0] + " "+ arry[1])

            clear_count.getLocationOnScreen(arry)
            Log.e(" clear_count", "start "+arry[0] + " "+ arry[1])

        },1000)

        file_name.setOnKeyListener { v, keyCode, event ->
            if(keyCode == KeyEvent.KEYCODE_CLEAR){
                file_name.setText("")
                Log.e("clear "," is clicked! ")
            }
            false
        }
        pic_count.setOnKeyListener { v, keyCode, event ->
            if(keyCode == KeyEvent.KEYCODE_CLEAR){
                pic_count.setText("")
                Log.e("clear "," is clicked! ")
            }
            false
        }

        start.setOnClickListener {
            val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager //得到InputMethodManager的实例
            if (imm.isActive) {//如果开启
                imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, InputMethodManager.HIDE_NOT_ALWAYS)//关闭软键盘，开启方法相同，这个方法是切换开启与关闭状态的
            }
        }

    }

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>?, grantResults: IntArray?) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if(grantResults!![0] == PackageManager.PERMISSION_GRANTED){
            init()
        }else{
            ActivityCompat.requestPermissions(this,
                    arrayOf(Manifest.permission.CAMERA), 100)
        }

    }

    var px = 640
    private fun init(){
        Thread{
            Utils.addModeles(this)
        }.start()
        var file = File(pathRoot)
        if(!file.exists())
            initDir(pathRoot)
        path = pathRoot+ "/"
        px = intent.getIntExtra("px",640)
        with(tutorial2_activity_surface_view) {
            visibility = CameraBridgeViewBase.VISIBLE
            setCameraIndex( 1)
            setMaxFrameSize(1280, 960)
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

        time = Utils.time2()

        captureThread = Thread{
            while (true){
                if(!stop)
                    recordFile()
            }
        }
        captureThread?.start()

        start.setOnClickListener {
            page = file_name.text.toString()
            if(TextUtils.isEmpty(page))
                page = "test"
            if(!TextUtils.isEmpty( pic_count.text))
                max = pic_count.text?.toString()?.toInt() ?: 20
//            addIndex(this)
            textv.text = "录制中。。。"
            count = 0
            stop = false
            val imm = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager //得到InputMethodManager的实例
            if (imm.isActive) {//如果开启
                imm.toggleSoftInput(InputMethodManager.SHOW_IMPLICIT, InputMethodManager.HIDE_NOT_ALWAYS)//关闭软键盘，开启方法相同，这个方法是切换开启与关闭状态的
            }
        }

        end.setOnClickListener {
            stop = true
            textv.text = "结束录制。。。"
            textv.postDelayed({matQueue.clear()},200)
        }


        sdPlayer = MediaPlayer.create(this,R.raw.sdcard)
        detectFacePlayer = MediaPlayer.create(this,R.raw.detect)

        cmdControl()

        delete_btn.setOnClickListener { view ->
            var filename = delete_name.text.toString()
            if(!TextUtils.isEmpty(filename)){
                var file = File(pathRoot + "/" + filename.trim()+".bin")
                var filetxt = File(pathRoot + "/" + filename.trim()+".txt")
                if(file.exists()){
                    file.delete()
                    filetxt.delete()
                    Toast.makeText(this@DetectActitvity,"删除成功",Toast.LENGTH_LONG).show()
                }else{
                    Toast.makeText(this@DetectActitvity,"不存在这个文件",Toast.LENGTH_LONG).show()
                }

            }
        }

        back_btn.setOnClickListener {
            finish()
        }

        file_name.setOnKeyListener { v, keyCode, event ->
            if(keyCode == KeyEvent.KEYCODE_CLEAR){
                file_name.setText("")
                Log.e("clear "," is clicked! ")
            }
            false
        }
        pic_count.setOnKeyListener { v, keyCode, event ->
            if(keyCode == KeyEvent.KEYCODE_CLEAR){
                pic_count.setText("")
                Log.e("clear "," is clicked! ")
            }
            false
        }

        Utils.writeFlag(arg2 + arg1)


    }

    private fun recordFile() {
        if (!stop) {
            if (System.currentTimeMillis() - timeControl < 50)
                return
            timeControl = System.currentTimeMillis()
            if (CONTROL_NUM) {
                if (count < max) {
                    Log.e("  count == ", "" + count + " queue size = " + matQueue.size)
                    if(OneFile){
                        if(saveImageIn1File())
                            count++
                    }else{
                        if(saveImageInMultipleFiles())
                            count++
                    }
                }else{
                    stop = true
                    runOnUiThread {
                        textv.text = "结束录制。。。"
                    }
                    matQueue.clear()
                }
            } else {
                if(OneFile)
                    saveImageIn1File()
                else
                    saveImageInMultipleFiles()

            }
        }
    }

    private fun saveImageIn1File() :Boolean{
        if(matQueue.isEmpty())
            return false
        val times = System.currentTimeMillis().toString()
        builder.diretory = page
        builder.fileName = times
        builder.deleted = false
//        builder.data = ByteString.copyFrom(rgb?.toByte())
        builder.data = ByteString.copyFrom(matQueue.remove().toByte())
        imgData = builder.build()
        val f = File(path + time + ".bin")
        val tagFile = File(path + time + ".txt")
        if (f.exists()) {
            val o = FileOutputStream(f, true)
            o.write(imgData?.toByteArray())
            o.close()
            val timeout = FileWriter(tagFile, true)
            timeout.write(imgData?.serializedSize?.toString() + "|")
            timeout.close()
        } else {
            val o = FileOutputStream(path + time + ".bin")
            o.write(builder.build().toByteArray())
            o.close()
            val timeout = FileWriter(tagFile)
            timeout.write(imgData?.serializedSize?.toString() + "|")
            timeout.close()
        }

//        var img = File(path + page + "/" + times + "img.jpg")
//        var io = FileOutputStream(img)
//        io.write(rgb?.toByte())
//        io.close()
        return true
    }

    private fun saveImageInMultipleFiles() :Boolean{
        if(matQueue.isEmpty())
            return false
        val times = System.currentTimeMillis().toString()
        builder.diretory = page
        builder.fileName = times
        builder.deleted = false
//        builder.data = ByteString.copyFrom(rgb?.toByte())
        builder.data = ByteString.copyFrom(matQueue.remove().toByte())

        imgData = builder.build()
        val f = File(path + page + ".bin")
        val tagFile = File(path + page + ".txt")
        Log.e("saveImage path:",path + page + ".bin")
        if (f.exists()) {
            val o = FileOutputStream(f, true)
            o.write(imgData?.toByteArray())
            o.close()
            val timeout = FileWriter(tagFile, true)
            timeout.write(imgData?.serializedSize?.toString() + "|")
            timeout.close()
        } else {
            val o = FileOutputStream(path + page + ".bin")
            o.write(builder.build().toByteArray())
            o.close()
            val timeout = FileWriter(path + page + ".txt")
            timeout.write(imgData?.serializedSize?.toString() + "|")
            timeout.close()
        }
        return true

    }

    private fun cmdControl() {
        on.setOnClickListener {
            arg1 = "1"
            on.setTextColor(resources.getColor(R.color.green))
            off.setTextColor(resources.getColor(R.color.black))
            Utils.writeFlag(arg2 + arg1)
        }

        off.setOnClickListener {
            off.setTextColor(resources.getColor(R.color.green))
            on.setTextColor(resources.getColor(R.color.black))
            arg1 = "0"
            Utils.writeFlag(arg2 + arg1)
        }

        rgb1.setOnClickListener {
            arg2 = "RGB"
            rgb1.setTextColor(resources.getColor(R.color.green))
            ir.setTextColor(resources.getColor(R.color.black))
            Utils.writeFlag(arg2 + arg1)
        }

        ir.setOnClickListener {
            arg2 = "IR"
            ir.setTextColor(resources.getColor(R.color.green))
            rgb1.setTextColor(resources.getColor(R.color.black))
            Utils.writeFlag(arg2 + arg1)
        }
    }

    var count = 10000
    var max = 20
    var last = 0L
    override fun onCameraFrame(inputFrame: CameraBridgeViewBase.CvCameraViewFrame): Mat {
        mRgba = inputFrame.rgba()
        mGray = inputFrame.gray()
        Imgproc.cvtColor(mRgba,rgb,Imgproc.COLOR_RGBA2BGR)
//        recordFile()
        if(!stop){
            if(px == 640){
                rgb = rgb?.submat(0,720,160,960+160)
                var size = Size(640.0,480.0)
                Imgproc.resize(rgb,rgb,size)
            }

            if(matQueue.size == 30)
                matQueue.remove()
            matQueue.add(rgb!!.clone())
        }
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


    companion object {
        private const val TAG = "OCVSample::Activity"
    }


}
