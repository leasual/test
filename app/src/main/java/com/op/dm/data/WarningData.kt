package com.op.dm.data

class WarningBean {
    constructor(name:String,type: WarningData.WarningType){
        this.name = name
        this.type = type
        if(type == WarningData.WarningType.LR)
            second = 2
    }
    var name = ""
    var checked = true
    var type = WarningData.WarningType.DEF
    var second = 1
}

class WarningData{
    enum class WarningType {
        DEF,LR, DIS,SMOKE ,CALL,NOFACE,WRONGID,YAWN
    }
    var names = arrayOf("左顾右盼","分神","吸烟","打电话","画面异常","身份异常","打哈欠")
    var types = arrayOf(WarningType.LR, WarningType.DIS, WarningType.SMOKE, WarningType.CALL, WarningType.NOFACE, WarningType.WRONGID, WarningType.YAWN)
    var beans = arrayListOf<WarningBean>()

    init {
        names.forEachIndexed { index, s ->
            beans.add(WarningBean(s,types[index]))
        }
    }
}