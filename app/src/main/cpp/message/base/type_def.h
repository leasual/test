//
// Created by public on 19-3-18.
//

#ifndef DSM_JTT808_TYPEDEF_H
#define DSM_JTT808_TYPEDEF_H

#include <stdint.h>

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

#endif //DSM_JTT808_TYPEDEF_H
