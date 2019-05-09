package com.op.dm

import android.graphics.Bitmap
import android.util.Log
import java.io.ByteArrayOutputStream
import java.io.DataOutputStream
import java.net.ServerSocket
import java.net.Socket


class ImageServer {
    var port = 30000
    var serverSocket: ServerSocket? = null
    var socket: Socket? = null
    var thread:Thread? = null
    var run = false

    constructor(){
        serverSocket = ServerSocket(port)
        socket = serverSocket?.accept()
        socket?.keepAlive = true
        socket?.tcpNoDelay = true
    }


    fun sendImage(data :Bitmap){

        socket?.apply {
            Log.e("not conect ","" + this.isConnected)
            if (!this.isConnected){
                return
            }
            val stream = DataOutputStream(this.getOutputStream())
            val bout = ByteArrayOutputStream()
            data.compress(Bitmap.CompressFormat.WEBP,70,bout)
            stream.writeInt(bout.size())
            stream.write(bout.toByteArray(),0,bout.size())
            Log.e("sendd ","" + bout.size())
        }
    }

    fun sendMat(data :ByteArray){
        socket?.apply {
            Log.e("not conect ","" + this.isConnected)
            if (!this.isConnected){
                return
            }
            val stream = DataOutputStream(this.getOutputStream())
//            val bout = ByteArrayOutputStream()
//            bout.write(data)
            stream.writeInt(data.size)
            stream.write(data,0,data.size)
            Log.e("sendd ","" + data.size)
        }
    }

    fun stop(){
        socket?.close()
        serverSocket?.close()
    }

}