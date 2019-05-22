package com.op.dm

import android.graphics.Bitmap
import android.util.Log
import java.io.ByteArrayOutputStream
import java.io.DataInputStream
import java.io.DataOutputStream
import java.lang.Exception
import java.net.ServerSocket
import java.net.Socket


class ImageServer {
    var port = 8888
    var serverSocket: ServerSocket? = null
    var socket: Socket? = null
    var thread:Thread? = null
    var run = false
    var timeControl = 0L

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

    fun sendMat(data :ByteArray,status:IntArray){
        if(System.currentTimeMillis() - timeControl < 50){
            return
        }
        socket?.apply {
            Log.e("not conect ","" + this.isConnected)
            if (!this.isConnected){
                return
            }
            try {
                val stream = DataOutputStream(this.getOutputStream())
//            val bout = ByteArrayOutputStream()
//            bout.write(data)

                stream.writeInt(data.size)
                stream.write(data,0,data.size)
                Log.e("sendd ","" + data.size)
                var buffer = StringBuffer()
                status?.forEach {
                    buffer.append(it)
                }
                buffer.replace(0,1,"2")
                stream.writeUTF(buffer.toString())

//                val inp = DataInputStream(this.getInputStream())
//                Log.e(" recerrive " ,"" + inp.readUTF())
            }catch (e:Exception){
                socket?.close()
                socket = serverSocket?.accept()
            }
            timeControl = System.currentTimeMillis()
        }
    }

    fun stop(){
        socket?.close()
        serverSocket?.close()
    }

}