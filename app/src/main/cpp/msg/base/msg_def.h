//
// Created by public on 19-3-18.
//

#ifndef DSM_JTT808_MSG_DEF_H
#define DSM_JTT808_MSG_DEF_H

#include "public_def.h"
#include <algorithm>
#include "type_def.h"

template <class T>
void endswap(T *objp)
{
    unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
    std::reverse(memp, memp + sizeof(T));
}

#define MSG_FLAG 0x7e

/*! 错误码 */
typedef enum
{
    RET_OK = 0, 		  			//!< 成功
    RET_FAILED,					//!< 失败
    RET_MESSAGE_ERROR,	        //!< 消息有误
    RET_NOT_SUPPORT,		    //!< 不支持
    RET_ALARM,					//!< 报警处理确认
    RET_PARAMETER_ERROR         //!< 参数错误
}ERROR_CODE;

/*! 注册结果 */
typedef enum
{
    REGISTER_OK,                //!<  成功
    REGISTER_CAR_REGISTERED,    //!< 车辆已被注册
    REGISTER_HAVE_NO_CAR,       //!< 数据库中无该车辆
    REGISTER_DEV_REGISTERED,    //!< 终端已被注册
    REGISTER_HAVE_NO_DEV,	    //!< 数据库中无该终端
}STR_REGISTER_RESULT;

/*! 查询指定终端参数消息体数据格式 */
typedef struct
{
    BYTE count;   //!< 参数总数
    BYTE *idList; //!< 参数ID 列表
}STR_PARAMETER_LIST;

typedef struct
{
    DWORD id;
    BYTE  len;
    STR_PARAMETER_LIST  *data;
}STR_PARAMETER;

typedef enum JTT_MSG_TYPE_
{
    /**
     * 平台通用消息
     */
    JTT_DEV_GENERAL_ACK = 0x0001, // 终端通用应答
    JTT_PLAT_GENERAL_ACK = 0x8001, // 平台通用应答

    JTT_SEND_SPLIT_PKT = 0x8003, // 补传分包请求

    JTT_DEV_HEARTBEAT = 0x0002, // 终端心跳

    JTT_DEV_REGISTER = 0x0100, // 终端注册
    JTT_PLAT_REGISTER_ACK = 0x8100, // 终端注册回应

    JTT_DEV_LOGOUT = 0x0003, // 终端注销

    JTT_DEV_AUTH = 0x0102, // 终端Auth

    JTT_PLT_GET_PARA = 0x8104,  // 查询终端参数
    JTT_PLT_GET_PARA_RESP = 0x0104, // 查询终端参数应答
    JTT_DEV_SET_PARA = 0x8103, // 设置终端参数

    JTT_PLT_GET_DEV_ATTRI_RESP = 0x0107, // 查询终端属性应答
    JTT_DEV_LOC_REP = 0x0200, // 终端位置上报

    JTT_SU_PLT_ACCESSORIES_UP = 0x9208, // 报警附件上传指令

    JTT_CMD_INVALID = 0xFFFF
}JTT_MSG_TYPE;

#pragma pack( push ,1 )

//
// 消息结构
// | 标识位 | 消息头 | 消息体 | 检验码 | 标识位 |
// 消息头格式
// | 消息ID | 消息体属性 | 终端手机号 | 消息流水号 | 消息包封装项 |
//
/*JTT协议头*/
typedef struct JT808Protocol
{
    enum { HEADERSIZE = 12 };  /**消息头13个字节*/
    //BYTE    Sign[1] ;     /**<协议头0x7e 1*/
    WORD    Cmd;          /**<消息ID 2*/
    WORD    Mask ;        /**<消息体属性 16bit*/
    BYTE    SimNo[6] ;   /**<手机号* 6*/
    WORD    SeqNo   ;   /**<消息流水号 2*/
    BYTE    MsgBd[2] ;
    /**
    * 消息体长度
    */
    WORD   GetMsgBodyLength()
    {
        WORD ulen = Mask ;
        endswap(&ulen) ;
        ulen =  0X03FF & ulen;
        return ulen;
    }
    /**小字节序设置长度*/
    void   SetMsgBodyLength(WORD wLen)
    {
        endswap(&wLen) ;
        WORD uProp = 0x00FC & Mask ;
        Mask = uProp | wLen ;
    }

    WORD    GetCmd()
    {
        WORD wcmd = Cmd ;
        endswap(&wcmd) ;
        return wcmd ;
    }
    void SetCmd(WORD cmd)
    {
        Cmd = cmd ;
        endswap(&Cmd) ;
    }

    void SetSeqNo(WORD dwSeqNo)
    {
        SeqNo = dwSeqNo;
        endswap(&SeqNo);
    }

    BYTE GetCheckCode()
    {
        if(IsMultiPacket())
        {
            return 0 ; //暂时不处理分包的情况
        }
        else  //不分包
        {
            return  *( &Cmd + HEADERSIZE + GetMsgBodyLength()) ;
        }
    }

    /**
    * 是否分包
    */
    bool IsMultiPacket()
    {
        WORD uData = Mask ;
        endswap(&uData) ;

        WORD uVal = 0x2000 & uData ;
        if( uVal == 0 )
            return false ;
        return true ;
    }

    void DisableMultiPacket()
    {
		Mask = Mask & (~0x0020);
    }

    void EnableMultiPacket()
    {
		Mask = Mask | 0x0020;
    }

    void DisableEncrypt()
    {
		Mask = Mask & (~0x001c);
		
    }

    void EnableEncrypt()
    {
		Mask = Mask | 0x001c;
    }

    /**
    * 是否加密
    */
    bool IsEncrypted()
    {
        WORD uData = Mask ;
        endswap(&uData) ;
        WORD uVal = 0x1c00 & uData ;
        if( uVal == 0 )
            return false ;
        return true ;
    }
}JTT808MsgHead;

/*! 终端注册消息体数据格式 */
typedef struct
{
    WORD province_ID;  				//!< 省域ID
    WORD city_ID;					//!< 市县域ID
    BYTE manufacturer_ID[5];	    //!< 制造商ID
    BYTE dev_model[20];				//!< 终端型号
    BYTE dev_ID[7];				//!< 终端ID
    BYTE license_color;					//!< 车牌颜色
    STRING license_flag;					//!< 车辆标识

    void SetProviceId(WORD id)
    {
        province_ID = id ;
        endswap(&province_ID) ;
    }

    void SetCityId(WORD id)
    {
        city_ID = id ;
        endswap(&city_ID) ;
    }

    void SetManuId(BYTE manuId[5])
    {
        endswap(manuId);
		memcpy(manufacturer_ID,manuId,5);
    }

    void SetDevModelId(BYTE id[20])
    {
        endswap(id);
        memcpy(dev_model,id,20);
    }

    void SetDevId(BYTE id[7])
    {
        endswap(id);
        memcpy(dev_ID,id,7);
    }

    void SetLicenseColor(BYTE nColor)
    {
        license_color = nColor;
    }

	void SetLicenseFlag(BYTE* pFlag, BYTE nLen)
	{
		memcpy(license_flag,pFlag,nLen);
	}
}JTT808RegPktUP;


///**位置基本信息*/
//typedef struct JTT808Body_PositionUP
//{
//    DWORD alarmstate ; //报警标志位
//    DWORD posstate ;   //状态标志位
//    DWORD lat ; //纬度
//    DWORD lon ; //经度
//    WORD  height ; //高程
//    WORD  speed ;   //速度
//    WORD  heading ;//方向
//    BYTE  bcdtime[6] ;//时间
//}JTT808Body_PositionUP;


/*! 终端属性 */
typedef struct
{
    WORD dev_type;                      //!< 终端类型
    BYTE manufacturer_ID[5];            //!< 制造商ID
    BYTE dev_model[20];					//!< 终端型号
    BYTE dev_ID[7];					    //!< 终端ID
    BCD ICCID[10];                      //!< 终端SIM 卡ICCID
    BYTE dev_version_len;               //!< 终端硬件版本号长度
    STRING dev_version;	                //!< 终端硬件版本号
    BYTE firmware_version_len;          //!< 终端固件版本号长度
    STRING firmware_version;	        //!< 终端固件版本号
    BYTE gnns;                          //!< GNSS 模块属性
    BYTE communication_module;          //!< 通信模块属性
}STR_DEV_ATTR;


/*! 位置基本信息 */
struct JTT808Body_PositionUP
{
    DWORD alarm_flag;   //!< 报警标志
    DWORD status;       //!< 状态
    DWORD latitude ;    //!< 纬度
    DWORD longitude;    //!< 经度
    WORD high;          //!< 高程
    WORD speed;         //!< 速度
    WORD direction;     //!< 方向
    BCD time[6];        //!< 时间

    JTT808Body_PositionUP():alarm_flag(0),status(0){}

    // 置疲劳驾驶的标志位
    void EnableFatigueFlag()
    {
        alarm_flag |= 0x0002;
        endswap(&alarm_flag);
    }

    // 取消疲劳驾驶的标志位
    void DisableFatigueFlag()
    {
        alarm_flag &= ~0x0002;
        endswap(&alarm_flag);
    }

    void SetAlarmFlag(DWORD dwAlarmFlag)
    {
        alarm_flag = dwAlarmFlag;
        endswap(&alarm_flag);
    }

    void SetStatus(DWORD dwStatus)
    {
        status = dwStatus;
        endswap(&status);
    }

    void SetLatitude(DWORD dwLatitude)
    {
        latitude = dwLatitude;
        endswap(&latitude);
    }

    void SetLongitude(DWORD dwLongitude)
    {
        longitude = dwLongitude;
        endswap(&longitude);
    }

    void SetHigh(WORD nHigh)
    {
        high = nHigh;
        endswap(&high);
    }

    void SetSpeed(WORD nSpeed)
    {
        speed = nSpeed;
        endswap(&speed);
    }

    void SetDirection(WORD nDirection)
    {
        direction = nDirection;
        endswap(&direction);
    }

    void SetTime(char* szTime[6])
    {
        memcpy(time,szTime,6);
    }
};


/*! 位置附加信息 */
struct JTT808Body_PositionUP_Extra
{
    BYTE extra_id;  // 附加信息ID
    BYTE extra_len; // 附加信息长度
};

/*! 苏标 驾驶员状态监测报警 */
struct JTT808_SU_Body_DSM_Alarm
{
    DWORD dwAlarmId;       // 0->
    BYTE btStatus;  // 4->
    BYTE btAlarmType;  // 5->报警类型:0x01:疲劳驾驶报警 0x02:接打电话报警 0x03:抽烟报警 0x04:分神驾驶报警
    BYTE btAlarmGrade;  // 6->报警等级:0x01:一级报警 0x02:二级报警
    BYTE btFatiqueDegree; // 7->疲劳程度 1~10数值越大表示疲劳程度越严重
    BYTE btReserved[4];  // 8->保留
    BYTE btSpeed; // 12 -> 车速
    WORD high;  // 13 -> 高程
    DWORD latitude ;    //15 -> 纬度
    DWORD longitude;    //19 -> 经度
    BCD time[6];        //23 -> 时间
    WORD status;      // 29 -> 车辆状态
    BYTE btAlarmFlag[16]; // 31 -> 报警标识号

    void SetAlarmId(DWORD dwAlarm)
    {
        dwAlarmId = dwAlarm;
        endswap(&dwAlarmId);
    }

    void SetVendorStatus(WORD nStatus)
    {
        status = nStatus;
        endswap(&status);
    }

    void SetLatitude(DWORD dwLatitude)
    {
        latitude = dwLatitude;
        endswap(&latitude);
    }

    void SetLongitude(DWORD dwLongitude)
    {
        longitude = dwLongitude;
        endswap(&longitude);
    }

    void SetHigh(WORD nHigh)
    {
        high = nHigh;
        endswap(&high);
    }
};

#define LEN_SU808_ALARM sizeof(JTT808_SU_Body_DSM_Alarm)

/*! 苏标 报警标识号格式 */
struct JTT808_SU_Body_DSM_Alarm_Flag
{
    BYTE dev_ID[7];  // 0 -> 终端 ID ,7 个字节,由大写字母和数字组成
    BCD time[6];        //7 -> 时间
    BYTE btSeqNo;     // 13 -> 同一时间点报警的序号,从 0 循环累加
    BYTE btAccessories; // 14 -> 表示该报警对应的附件数量
    BYTE btReserved;  // 15 -> 预留

    void SetDevId(BYTE id[7])
    {
        endswap(id);
        memcpy(dev_ID,id,7);
    }
};

#define LEN_SU808_ALARM_FLAG  sizeof(JTT808_SU_Body_DSM_Alarm_Flag)  // 报警标识的长度


#pragma pack( pop )

#define OFFSET(TYPE, MEMBER, OFF) \
    TYPE temp;              \
    OFF = (unsigned long)(&(temp.MEMBER)) - (unsigned long)(&(temp));

#endif //DSM_JTT808_MSG_DEF_H
