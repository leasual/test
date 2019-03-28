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
#include "dsm_log.h"
#include "config_file.h"



#define SERVER_HEARTBEAT_INTERVAL	2*60*1000    //  心跳的時間間隔爲3分钟
#define SERVER_TIMEOUT				3 * SERVER_HEARTBEAT_INTERVAL    // 3倍的保活时间如果收不到回应则认为服务器离线
#define CHECK_TIMER                 10*1000    // 10秒check一次timer
#define LOC_UP_TIMER                30*1000    // 30秒上报一次位置信息


#endif //DSM_JTT808_PUBLIC_DEF_H
