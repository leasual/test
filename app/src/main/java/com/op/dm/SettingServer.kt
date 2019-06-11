package com.op.dm

import android.content.Context
import android.content.SharedPreferences
import android.util.Log
import com.google.gson.Gson
import com.op.dm.data.WarningData
import java.io.DataInputStream
import java.lang.ref.SoftReference
import java.net.ServerSocket
import java.net.Socket


class SettingServer {
    var port = 8888
    var serverSocket: ServerSocket? = null
    var socket: Socket? = null
    var run = false
    var control = 0L
    var data:WarningData? = null
    val WARNING = "Warning"
    var sp:SoftReference<SharedPreferences>? = null

    companion object {
        val instance : SettingServer by lazy(LazyThreadSafetyMode.SYNCHRONIZED){ SettingServer()}
    }

    private constructor(){
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
                                   data = Gson().fromJson(setting, WarningData::class.java)
                                    data?.beans?.forEach {
                                        var type = it.type
                                        var typeJni = 0
                                        when(type){
                                            WarningData.WarningType.SMOKE -> typeJni = 0
                                            WarningData.WarningType.CALL -> typeJni = 1
                                            WarningData.WarningType.DIS -> typeJni = 2
                                            WarningData.WarningType.YAWN -> typeJni = 3
                                            WarningData.WarningType.LR -> typeJni = 4

                                            WarningData.WarningType.NOFACE -> typeJni = 5
                                            WarningData.WarningType.WRONGID -> typeJni = -1
                                        }
                                        Log.e(" typejni , type ", "" + typeJni + " type " + type)
                                        SetParam(typeJni,it.second)
                                    }
                                    saveParam()
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

    fun loadParam(context: Context){
        var edit = context.getSharedPreferences("un", Context.MODE_PRIVATE)
        var warningJson = edit.getString(WARNING, Gson().toJson(WarningData()))
        sp = SoftReference(edit)
        data = Gson().fromJson(warningJson,WarningData::class.java)
    }

    fun saveParam(){
       sp?.get()?.let {
           var size = data?.beans?.size?:0
           if(size > 0){
               it.edit().putString(WARNING,Gson().toJson(data)).apply()
           }
       }
    }

    external fun SetParam(param: Int, value: Int)
}