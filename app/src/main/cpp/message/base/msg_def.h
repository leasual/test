//
// Created by public on 19-3-18.
//

#ifndef DSM_JTT808_MSG_DEF_H
#define DSM_JTT808_MSG_DEF_H

#include "public_def.h"
#include <algorithm>

template <class T>
void endswap(T *objp)
{
    unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
    std::reverse(memp, memp + sizeof(T));
}

#define MSG_FLAG  0x7e
#define ADAS_FLAG 0x64      //高级驾驶辅助系统
#define DSM_FLAG  0x65      //驾驶员状态监控系统
#define TPMS_FLAG 0x66      //轮胎气压监测系统
#define BSD_FLAG  0x67      //盲点监测系统

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

    JTT_SU_PLT_ACCESSORY_UP = 0x9208, // 平台请求报警附件上传指令
    JTT_SU_DEV_ACCESSORY_UP = 0x1210, // 设备报警附件上传指令
    JTT_SU_DEV_FILE_UP = 0x1211, //文件信息上传
    JTT_SU_FILE_UP_COMPLETE = 0x1212, // 文件上传完成
    JTT_SU_PLT_FILE_UP_OK = 0x9212, // 文件上传完成消息应答,由平台发送

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


//0x0001 - JTT_DEV_GENERAL_ACK
typedef struct
{
    WORD  nSeqNo;   // 对应平台的流水号
    WORD  nAckId;   // 对应的平台应答消息ID
    BYTE  nResult;  // 结果 0:成功/确认;1:失败;2:消息有误;3:不支持

    void SetSeqNo(WORD SeqNo)
    {
        nSeqNo = SeqNo ;
        endswap(&nSeqNo) ;
    }

    void SetAckId(WORD ack)
    {
        nAckId = ack;
        endswap(&nAckId);
    }
}DevGeneralAck;
#define LEN_DEV_GEN_ACK  sizeof(DevGeneralAck) // 通用指令体的长度


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

    void ResetAlarmFlag()
    {
        alarm_flag &= 0x00000000;
    }

    void ResetStatus()
    {
        status &= 0x00000000;
    }

    // 置疲劳驾驶的标志位
    void EnableFatigueFlag()
    {
        alarm_flag |= 0x00000004;
        endswap(&alarm_flag);
    }

    // 取消疲劳驾驶的标志位
    void DisableFatigueFlag()
    {
        alarm_flag &= (~0x00000004);
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


//附件上传请求指令 (平台发给终端侧的)
struct JTT808_SU_Accessory_up
{
    BYTE  nIpAddrLen;
	std::string strIp;
    WORD  nTcpPort;
    WORD  nUdpPort;
    BYTE  alarmFlag[16];
    BYTE  alarmNo[32];
    BYTE  reserved[16];

    void Format(BYTE* pBody)
    {
        if (NULL == pBody)
            return;

        int nOffset = 0;
        nIpAddrLen = *(BYTE*)pBody;
        nOffset += 1;

        char szIpAddr[32] = {0};
        memcpy(szIpAddr,pBody+nOffset,nIpAddrLen);
        strIp = szIpAddr;
        nOffset += nIpAddrLen;

        nTcpPort = *((WORD*)(pBody + nOffset));
        endswap(&nTcpPort);
        nOffset += 2;

        nUdpPort = *((WORD*)(pBody + nOffset));
        endswap(&nUdpPort);
        nOffset += 2;

        memcpy(alarmFlag,pBody + nOffset,16);
        nOffset += 16;

        memcpy(alarmNo,pBody + nOffset,32);
        nOffset += 32;
    }
};


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
        memcpy(dev_ID,id,7);
    }

};
#define LEN_SU808_ALARM_FLAG  sizeof(JTT808_SU_Body_DSM_Alarm_Flag)  // 报警标识的长度

// 设备报警附件上传指令
struct JTT808_SU_DEV_ACCESSORY_UP
{
    BYTE dev_ID[7];
    BYTE  alarmFlag[16];
    BYTE  alarmNo[32];
    BYTE  accessoryType; // 0x00:正常的报警文件信息  0x01:补传报警文件信息
    BYTE  accessoryNum;  // 附件数量
};
#define LEN_DEV_ACCESSORY_UP sizeof(JTT808_SU_DEV_ACCESSORY_UP)

struct JTT808_SU_ACCESSORY
{
    BYTE fileNameLen;  // 文件名称长度
    STRING fileName;  // 文件名称
    DWORD fileSize;  // 文件大小
};

struct FileInfo
{
    FileInfo(std::string strOrigFileName, BYTE nType, DWORD dwSize):
            m_strOrigFileName(strOrigFileName),m_euType(nType),m_dwSize(dwSize),m_fileStatus(euInit){
        memset(m_szFileName,0,50);
    }
    char m_szFileName[50]; // 转换之后的文件名称
    std::string m_strOrigFileName; //  原始文件名
    BYTE  m_euType;  // 文件类型
    DWORD m_dwSize; // 文件大小
    euFileUpStatus m_fileStatus; // 文件上传的状态
};


// 终端文件上传完成
struct DevFileUpComplete
{
    BYTE fileNameLen;  // 文件名称长度
    STRING fileName;  // 文件名称
    BYTE  btFileType; // 文件类型
    DWORD fileSize;  // 文件大小
};

struct PltFileUpCompleteAck
{
    BYTE fileNameLen;  // 文件名称长度
    char szFileName[50]; // 文件名称
    BYTE  btFileType; // 文件类型
    BYTE  btUpResult;  // 上传结果 0x00:完成 0x01:需要补传
    BYTE  btReSendPkts; // 补传数据包的数量

    void Format(BYTE* pBody)
    {
        if (NULL == pBody)
            return;

        int nOffset = 0;
        fileNameLen = *(BYTE*)pBody;
        nOffset += 1;

        memset(szFileName,0,50);
        memcpy(szFileName,pBody+nOffset,fileNameLen);
        nOffset += fileNameLen;

        btFileType = *((BYTE*)(pBody + nOffset));
        nOffset += 1;

        btUpResult = *((BYTE*)(pBody + nOffset));
        nOffset += 1;

        btReSendPkts = *((BYTE*)(pBody + nOffset));
        nOffset += 1;
    }
};



#pragma pack( pop )

#define OFFSET(TYPE, MEMBER, OFF) \
    TYPE temp;              \
    OFF = (unsigned long)(&(temp.MEMBER)) - (unsigned long)(&(temp));


struct HpktInfo {
	bool is_header;
	int length;
};

#endif //DSM_JTT808_MSG_DEF_H
