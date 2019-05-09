//
// Created by public on 19-3-20.
//

#ifndef DSM_JTT808_PUBLIC_DEF_H
#define DSM_JTT808_PUBLIC_DEF_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iomanip>
#include "type_def.h"
#include "msg_def.h"
#include "config_file.h"
#include "dsm_log.h"


#define SERVER_HEARTBEAT_INTERVAL	60 *1000    //  心跳的時間間隔爲3分钟
#define SERVER_TIMEOUT				1 * SERVER_HEARTBEAT_INTERVAL    // 3倍的保活时间如果收不到回应则认为服务器离线
#define CHECK_TIMER                 10*1000    // 10秒check一次timer
#define LOC_UP_TIMER                20*1000    // 30秒上报一次位置信息
#define FILE_SIZE_PER_PACK          40 * 1024  //传文件时单个包最大传输40K
#define MAX_SMALL_FILE_SIZE			0x3FFFFF  //最大传输4M大小的文件
#define GPS_UPLOAD_TIMER            30 * 1000  // 30秒
#define ALARM_UPLOAD_TIMER          1000   // 1秒，报警信息１秒发一次
#define CONNECTED_TIMER             5*1000  // 5秒


#define ASSERT							assert

#endif //DSM_JTT808_PUBLIC_DEF_H
