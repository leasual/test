//
// Created by public on 19-3-18.
//

#ifndef DSM_JTT808_TYPEDEF_H
#define DSM_JTT808_TYPEDEF_H

#include <stdint.h>
#include <vector>
#include <memory>

#define ADAS_ALARM_FLAG 0x64      //高级驾驶辅助系统
#define DSM_ALARM_FLAG  0x65      //驾驶员状态监控系统
#define TPMS_ALARM_FLAG 0x66      //轮胎气压监测系统
#define BSD_ALARM_FLAG  0x67      //盲点监测系统

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
enum euDSMAlarmType
{
    euDsmAlarmInit = 0,
    euFatigue, // 疲劳
    euCall, // 打电话
    euSmoking, //抽烟
    euDistract, // 分神
};

enum euADASAlarmType
{
    euAdasAlarmInit = 0,
    euFrontCrash,    // 1前向碰撞报警
    euDeviateLane,   // 2车道偏离报警
    euDistNear,  // 3车距过近报警
    euBumpPeople,  // 4行人碰撞报警
    euChangLaneFrequently, // 5 频繁变道
    euIdentiOver,  // 6 道路标识超限
    euBarrier,  // 7 障碍物报警
};

//连接类型
enum euConnType
{
    euConnectInit = 0,
    euConnectPlt,       // 平台的连接类型
    euConnectAccessory, // 附件服务器的连接类型
};


//报警级别
enum euAlarmGradeType
{
    euAlarmGradeInit = 0,
    euAlarmGrade1, // 一级
    euAlarmGrade2, // 二级
};

enum euADASAlarmDeviationType
{
    euADASAlarmDeviationInit = 0,
    euLeftDevication,    // 1前向碰撞报警
    euRightDevication,    // 1前向碰撞报警
    //UINT8_t stDeviationType; // 9 -> 偏离类型 0x01:左侧偏离 0x02:右侧偏离 仅报警类型为 0x02 时有效
};

enum euADASAlarmRoadSignType
{
    euADASAlarmRoadSignInit = 0,
    euRoadSignSpeed,    // 1前向碰撞报警
    euRoadSignHeight,    // 1前向碰撞报警
    euRoadSignWight,    // 1前向碰撞报警
    //UINT8_t stRoadSignType;  // 10 -> 道路标志识别类型, 0x01:限速标志 0x02:限高标志 0x03:限重标志 仅报警类型为 0x06 和 0x10 时有效
};



#endif //DSM_JTT808_TYPEDEF_H
