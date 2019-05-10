//
// Created by public on 19-3-18.
//

#ifndef DSM_JTT808_MSG_DEF_H
#define DSM_JTT808_MSG_DEF_H

#include <algorithm>
#include "public_def.h"
#include "log4z.h"

template <class T>
void endswap(T *objp)
{
    unsigned char *memp = reinterpret_cast<unsigned char*>(objp);
    std::reverse(memp, memp + sizeof(T));
}

#define MSG_FLAG  0x7e

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

    // 置危险驾驶标志位
    void EnableDangerousFlag()
    {
        alarm_flag |= 0x00000008;
        endswap(&alarm_flag);
    }

    // 取消危险驾驶标志位
    void DisableDangerousFlag()
    {
        alarm_flag &= (~0x00000008);
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

/*! 苏标 驾驶员状态监测报警(DSM) */
struct JTT808_SU_DSM_Alarm
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
#define LEN_DSM_ALARM sizeof(JTT808_SU_DSM_Alarm)



/*! 苏标 高级驾驶辅助系统报警(ADAS) */
struct JTT808_SU_ADAS_Alarm {
    DWORD dwAlarmId;       // 0->
    BYTE btStatus;  // 4->
    BYTE btAlarmType;  // 5->报警类型:0x01:前向碰撞报警 0x02:车道偏离报警 0x03:车距过近报警 0x04:行人碰撞报警 0x06:道路标识超限报警 ...
    BYTE btAlarmGrade;  // 6->报警等级:0x01:一级报警 0x02:二级报警
    BYTE btFrontSpeed; // 7 -> 前车车速 单位 Km/h。范围 0~250,仅报警类型为 0x01 和 0x02 时有效
    BYTE btFrontDistance; // 8 -> 前车/行人 距离,单位 100ms,范围 0~100,仅报警类型为 0x01、0x02\ 0x04时有效
    BYTE btDeviationType; // 9 -> 偏离类型 0x01:左侧偏离 0x02:右侧偏离 仅报警类型为 0x02 时有效
    BYTE btRoadSignType;  // 10 -> 道路标志识别类型, 0x01:限速标志 0x02:限高标志 0x03:限重标志 仅报警类型为 0x06 和 0x10 时有效
    BYTE btRoadSignData;  // 11 -> 识别到道路标志的数据
    BYTE btSpeed;  // 12 -> 车速
    WORD height;  // 13 -> 高度
    DWORD latitude ;    // 15 -> 纬度
    DWORD longitude;    // 19 -> 经度
    BCD time[6];        //23 -> 时间
    WORD status;      // 29 -> 车辆状态
    BYTE btAlarmFlag[16]; // 31 -> 报警标识号

    void SetAlarmId(DWORD dwAlarm)
    {
        dwAlarmId = dwAlarm;
        endswap(&dwAlarmId);
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

    void SetHeight(WORD nHeight)
    {
        height = nHeight;
        endswap(&height);
    }
};
#define LEN_ADAS_ALARM sizeof(JTT808_SU_ADAS_Alarm)


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

#define HEX_VALUE_TO_CHAR(n)			(n <= 9 ? n + '0' : (n <= 'F' ? n + 'A' - 0X0A : n + 'a' - 0X0A))

/*! 苏标 报警标识号格式 */
struct JTT808_SU_Alarm_Flag
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

    void Format(BYTE* pBody)
    {
        if (NULL == pBody)
            return;

        int nOffset = 0;
        memset(dev_ID,0,7);
        memcpy(dev_ID,pBody+nOffset,7);
        nOffset += 7;

        memset(time,0,6);
        memcpy(time,pBody+nOffset,6);
        nOffset += 6;

        btSeqNo = *((BYTE*)(pBody + nOffset));
        nOffset += 1;

        btAccessories = *((BYTE*)(pBody + nOffset));
        nOffset += 1;

        btReserved = *((BYTE*)(pBody + nOffset));
        nOffset += 1;
    }

    std::string FormatToString()
    {
        char buf[1]={0};
        std::string strTemp;
        for (int i = 0; i < 7; i++)
        {
            sprintf(buf,"%c",(unsigned char)dev_ID[i]);
            strTemp.append(buf);
        }

        char tmp[18] = {0};
        const BYTE *bcd = time;
        BYTE *asc = (BYTE*)tmp;
        BYTE bcd2ascii[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
        BYTE c = 0;
        BYTE i;
        for(i = 0; i < 6; i++) {
            //first BCD
            c = *bcd >> 4;
            *asc++ = bcd2ascii[c];

            //second
            c = *bcd & 0x0f;
            *asc++ = bcd2ascii[c];
            bcd++;
        }
        strTemp.append((char*)tmp);

        sprintf(buf,"%hhu",btSeqNo);
        strTemp.append(buf);

        sprintf(buf,"%hhu",btAccessories);
        strTemp.append(buf);

        sprintf(buf,"%hhu",btReserved);
        strTemp.append(buf);

        return strTemp;
    }
};
#define LEN_SU808_ALARM_FLAG  sizeof(JTT808_SU_Alarm_Flag)  // 报警标识的长度

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



// 报警附件结构体定义
struct AlarmAccessory
{
    char stFileName[256]; // 报警文件路径
    euFileType stFileType;  // 报警文件类型:0-图片 2-视频
    char stPltFileName[50];  // 平台文件名称
    AlarmAccessory()
    {
        memset(stFileName,0,256);
        memset(stPltFileName,0,50);
    }
};


/**
 * 终端上传的位置信息,如果有报警信息,还需要带上报警信息
 */
struct DevLocInfo
{
    uint64_t stLatitude;      // 纬度
    uint64_t stLongitude;     // 经度
    uint32_t  stHeight;       // 高度
    WORD stSpeed;  // 速度
    bool stHasAlarm; // 是否有报警信息
    euDSMAlarmType stDsmAlarmType; // DSM报警事件类型
    euADASAlarmType stAdasAlarmType; // Alarm报警事件类型
    int stAlarmChannel; // 报警通道号
    std::vector<AlarmAccessory>  stAccessories; // 附件数量

    DevLocInfo():stLatitude(0),stLongitude(0),stHeight(0),stSpeed(0),
                 stHasAlarm(false),stDsmAlarmType(euDsmAlarmInit),stAdasAlarmType(euAdasAlarmInit),stAlarmChannel(DSM_ALARM_FLAG)
    {

    }

    DevLocInfo(uint64_t latitude,uint64_t longitude,uint32_t  height,WORD speed,bool bAlarm,
               euDSMAlarmType dsmAlarmType, euADASAlarmType adasAlarmType,int alarmChannel, std::vector<AlarmAccessory>& accessories)
    {
        stLatitude = latitude;
        stLongitude = longitude;
        stHeight = height;
        stSpeed = speed;
        stHasAlarm = bAlarm;
        stAccessories = accessories;
        stDsmAlarmType = dsmAlarmType;
        stAdasAlarmType = adasAlarmType;
        stAlarmChannel = alarmChannel;
    }

    ~DevLocInfo()
    {
        stAccessories.clear();
    }
};


struct UploadGPSInfo
{
    uint64_t stLatitude;      // 纬度
    uint64_t stLongitude;     // 经度
    uint32_t  stHeight;       // 高度
    WORD stSpeed;             // 速度
    WORD stDirection;         // 方向 0-359,正北为 0,顺时针

    bool stHasAlarm;          // 是否有报警信息
    int stAlarmChannel;       // 报警通道号

    UploadGPSInfo():stLatitude(0)
            ,stLongitude(0)
            ,stHeight(0)
            ,stSpeed(0)
            ,stDirection(0)
            ,stHasAlarm(false)
            ,stAlarmChannel(DSM_ALARM_FLAG){}

    UploadGPSInfo(uint64_t latitude,uint64_t longitude,uint32_t  height,WORD speed,WORD direction,bool bAlarm, int alarmChannel )
            :stLatitude(latitude)
            ,stLongitude(longitude)
            ,stHeight(height)
            ,stSpeed(speed)
            ,stDirection(direction)
            ,stHasAlarm(bAlarm)
            ,stAlarmChannel(alarmChannel){ }


    ~UploadGPSInfo(){
        //UT_TRACE("Deconstruct UploadGPSInfo.");
    }
};


struct UploadADASAlarmInfo
{
    uint8_t stStatus;  // 4->
    euADASAlarmType stAlarmType;  // 5->报警类型:0x01:前向碰撞报警 0x02:车道偏离报警 0x03:车距过近报警 0x04:行人碰撞报警 0x06:道路标识超限报警 ...
    euAlarmGradeType stAlarmGrade;  // 6->报警等级:0x01:一级报警 0x02:二级报警
    uint8_t stFrontSpeed; // 7 -> 前车车速 单位 Km/h。范围 0~250,仅报警类型为 0x01 和 0x02 时有效
    uint8_t stFrontDistance; // 8 -> 前车/行人 距离,单位 100ms,范围 0~100,仅报警类型为 0x01、0x02\ 0x04时有效
    euADASAlarmDeviationType stDeviationType; // 9 -> 偏离类型 0x01:左侧偏离 0x02:右侧偏离 仅报警类型为 0x02 时有效
    euADASAlarmRoadSignType stRoadSignType;  // 10 -> 道路标志识别类型, 0x01:限速标志 0x02:限高标志 0x03:限重标志 仅报警类型为 0x06 和 0x10 时有效
    uint8_t stRoadSignData;  // 11 -> 识别到道路标志的数据

    UploadADASAlarmInfo():
            stStatus(0)
            ,stAlarmType(euAdasAlarmInit)
            ,stAlarmGrade(euAlarmGrade1)
            ,stFrontSpeed(0)
            ,stFrontDistance(0)
            ,stDeviationType(euADASAlarmDeviationInit)
            ,stRoadSignType(euADASAlarmRoadSignInit)
            ,stRoadSignData(0){}

    UploadADASAlarmInfo(uint8_t status, euADASAlarmType alarmType, euAlarmGradeType alarmGrade,uint8_t frontSpeed, uint8_t frontDistance,
                        euADASAlarmDeviationType alarmDeviation, euADASAlarmRoadSignType alarmRoadSign, uint8_t roadSignData ):
            stStatus(status)
            ,stAlarmType(alarmType)
            ,stAlarmGrade(alarmGrade)
            ,stFrontSpeed(frontSpeed)
            ,stFrontDistance(frontDistance)
            ,stDeviationType(alarmDeviation)
            ,stRoadSignType(alarmRoadSign)
            ,stRoadSignData(roadSignData){}
};

struct UploadDSMAlarmInfo
{
    uint8_t btStatus;  // 4->
    euDSMAlarmType btAlarmType;  // 5->报警类型:0x01:疲劳驾驶报警 0x02:接打电话报警 0x03:抽烟报警 0x04:分神驾驶报警
    euAlarmGradeType btAlarmGrade;  // 6->报警等级:0x01:一级报警 0x02:二级报警
    uint8_t btFatiqueDegree; // 7->疲劳程度 1~10数值越大表示疲劳程度越严重

    UploadDSMAlarmInfo():
            btStatus(0)
            ,btAlarmType(euDsmAlarmInit)
            ,btAlarmGrade(euAlarmGradeInit)
            ,btFatiqueDegree(0){}

    UploadDSMAlarmInfo(uint8_t status,euDSMAlarmType alarmType,euAlarmGradeType alarmGrade,uint8_t fatiqueDegree):
            btStatus(status)
            ,btAlarmType(alarmType)
            ,btAlarmGrade(alarmGrade)
            ,btFatiqueDegree(fatiqueDegree){}

};


struct AppendAlarmInfoUnion
{
    std::shared_ptr<UploadADASAlarmInfo>  spADASAlarm;
    std::shared_ptr<UploadDSMAlarmInfo>  spDSMAlarm;

    //AppendAlarmInfoUnion():spADASAlarm(nullptr){}

};


struct DevUploadGPSAlarmInfo
{
    UploadGPSInfo stGPSInfo;
    std::shared_ptr<AppendAlarmInfoUnion>  stAlarmUnion;  // 报警信息指针
    std::vector<AlarmAccessory>  stAccessories;           // 附件
    DevUploadGPSAlarmInfo():stGPSInfo(UploadGPSInfo()) {
    }

    DevUploadGPSAlarmInfo( UploadGPSInfo gpsInfo,std::shared_ptr<UploadADASAlarmInfo> adasAlarmInfo,
                           std::vector<AlarmAccessory>& accessories ){
        stAlarmUnion.reset(new AppendAlarmInfoUnion()) ;
        stGPSInfo = gpsInfo;
        stAlarmUnion->spADASAlarm=adasAlarmInfo;
        stAccessories  = accessories;
    }

    DevUploadGPSAlarmInfo( UploadGPSInfo gpsInfo,std::shared_ptr<UploadDSMAlarmInfo> dsmAlarmInfo,
                           std::vector<AlarmAccessory>& accessories ){

        stAlarmUnion.reset(new AppendAlarmInfoUnion()) ;
        stGPSInfo = gpsInfo;
        stAlarmUnion->spDSMAlarm = dsmAlarmInfo;
        stAccessories  = accessories;
    }

    ~DevUploadGPSAlarmInfo(){
        stAccessories.clear();
		UT_TRACE("Deconstructor DevUploadGPSAlarmInfo");
    }
};

#endif //DSM_JTT808_MSG_DEF_H
