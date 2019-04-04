//
// Created by public on 19-3-18.
//

#ifndef DSM_JTT808_TYPEDEF_H
#define DSM_JTT808_TYPEDEF_H

#include <stdint.h>
#include <vector>
#include <string>
//#define BYTE  unsigned char     // 无符号单字节,8位
//#define WORD unsigned short   // 无符号双字节,16位
//#define DWORD unsigned long  // 无符号4字节,32位

/*******************************************************************************
* Type Definitions
*******************************************************************************/
typedef uint8_t BYTE;   // 无符号单字节,8位
typedef uint16_t WORD;  // 无符号双字节,16位
typedef uint32_t DWORD; // 无符号4字节,32位
typedef uint8_t BCD;
typedef uint8_t* STRING;
/* portable character for multichar character set */
typedef char                    ut_char;
/* portable wide character for unicode character set */
typedef unsigned short          ut_wchar;

/* portable 8-bit unsigned integer */
typedef unsigned char           ut_uint8;
/* portable 8-bit signed integer */
typedef signed char             ut_int8;
/* portable 16-bit unsigned integer */
typedef unsigned short int      ut_uint16;
/* portable 16-bit signed integer */
typedef signed short int        ut_int16;
/* portable 32-bit unsigned integer */
typedef unsigned int            ut_uint32;
/* portable 32-bit signed integer */
typedef signed int              ut_int32;

//#define LOWORD(l) ((WORD)(l))
//#define HIWORD(l) ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
//#define LOBYTE(w) ((BYTE)(w))
//#define HIBYTE(w) ((BYTE)(((WORD)(w) >> 8) & 0xFF))

#define MSG_FLAG    0x7e


// 網絡狀態定義
enum enNetStatus
{
    NET_INIT = 0,  // 初始状态
    NET_DISCONNECTED,  // 斷開連接
    NET_CONNECTED, // 連接上
    NET_REGISTED,  // 已经成功注册
    NET_AUTHENTICATED,   // 已经成功认证
};

enum euFileType
{
    euPIC, // 图片
    euAudio, // 音频
    euVideo, // 视频
    euText, // 文本
    euOther, // 其它
};

// 文件上传状态
enum euFileUpStatus
{
    euInit, // 初始状态
    euUploading, // 正在上传中
    euLoadSucc, // 上传成功
};

//报警事件类型
enum euAlarmType
{
    euAlarmInit = 0,
    euFatigue, // 疲劳
    euCall, // 打电话
    euSmoking, //抽烟
    euDistract, // 分神
};


// 报警附件结构体定义
struct AlarmAccessory
{
    char stFileName[256]; // 报警文件路径
    //DWORD dwFileSize;   //  文件大小
    euFileType stFileType;  // 报警文件类型:0-图片 2-视频
    AlarmAccessory()
    {
        memset(stFileName,0,256);
        //dwFileSize = 0;
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
    euAlarmType stAlaryType; // 报警事件类型
    std::vector<AlarmAccessory>  stAccessories; // 附件数量

    DevLocInfo():stLatitude(0),stLongitude(0),stHeight(0),stSpeed(0),stHasAlarm(false),stAlaryType(euAlarmInit)
    {

    }

    DevLocInfo(uint64_t latitude,uint64_t longitude,uint32_t  height,WORD speed,bool bAlarm,
               euAlarmType alarmType, std::vector<AlarmAccessory>& accessories)
    {
        stLatitude = latitude;
        stLongitude = longitude;
        stHeight = height;
        stSpeed = speed;
        stHasAlarm = bAlarm;
        stAccessories = accessories;
        stAlaryType = alarmType;
    }

    ~DevLocInfo()
    {
        stAccessories.clear();
    }
};

#endif //DSM_JTT808_TYPEDEF_H
