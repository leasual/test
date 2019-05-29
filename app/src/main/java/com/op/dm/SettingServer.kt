package com.op.dm

import android.os.Handler
import android.os.Looper
import android.util.Log
import java.io.DataInputStream
import java.net.ServerSocket
import java.net.Socket


class SettingServer {
    var port = 8888
    var serverSocket: ServerSocket? = null
    var socket: Socket? = null
    var run = false
    var control = 0L
    constructor(){
        Thread{
            serverSocket = ServerSocket(port)
            socket = serverSocket?.accept()
            socket?.keepAlive = true
            socket?.tcpNoDelay = true
            while (true){
                if(System.currentTimeMillis() - control > 1000){
                    if(run ){
                        control = System.currentTimeMillis()
                        try {
                            if(socket?.isClosed == false){
                                var receive = DataInputStream(socket?.getInputStream())
                                if(receive.available() > 0){
                                    var setting = receive.readUTF()
                                    receive.close()
                                    Log.e("receive from app  ","" + setting)
                                }
                            }

                        }catch (e:Exception){
                            Log.e("Setting exception ", e.toString())
                            Thread.sleep(800)
                            if(run){
                                socket?.close()
                                serverSocket?.close()
                                serverSocket = ServerSocket(port)
                                socket = serverSocket?.accept()
                                socket?.keepAlive = true
                                socket?.tcpNoDelay = true
                            }
                        }
                    }
                }
            }
        }.start()
    }

    fun run(){
        this.run = true
    }

    fun stop(){
        this.run = false
        socket?.close()
        serverSocket?.close()
    }

}