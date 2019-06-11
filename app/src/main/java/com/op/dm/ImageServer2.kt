package com.op.dm

import com.google.gson.Gson
import org.opencv.core.Mat
import org.zeromq.ZContext
import org.zeromq.ZMQ

class ImageServer2 {
    var port = 6600
    var socket: ZMQ.Socket? = null
    var thread:Thread? = null
    var run = false
    var timeControl = 0L
    val gson = Gson()

    constructor(){
        Thread{
            var contex = ZContext()
            socket = contex.createSocket(ZMQ.PUSH)
            socket?.bind("tcp://*:6600")
        }.start()
    }

    fun sendMat(data :Mat,status:IntArray){
        if(System.currentTimeMillis() - timeControl < 40 || data.total() < 10000){
            return
        }
        //InetAddress.getByName("192.168.2.65"),port
        socket?.send(data.toByte())

        var buffer = StringBuffer()
        status?.forEach {
            buffer.append(it)
        }
        buffer.append(":")
        buffer.append(gson.toJson(SettingServer.instance.data))
//        buffer.replace(0,1,"2")
        socket?.send(buffer.toString())
//        var receive = socket?.recvStr()
//        Log.e("receive :", receive)
        timeControl = System.currentTimeMillis()
    }

    fun stop(){
        Thread{ socket?.close()}.start()
    }

}