//
// Created by public on 19-3-19.
//

#ifndef DSM_JTT808_HP_TCP_COMM_H
#define DSM_JTT808_HP_TCP_COMM_H

#include "base/public_def.h"
#include "hpsocket/HPSocket4C.h"
#include "base/singleton.h"
#include "base/util.h"
#include <stdio.h>
#include <stdlib.h>

#include "client_conn.h"
#include "base/dsm_log.h"
#include "base/type_def.h"
#include "hp_socket_helper.h"
#include "msg_process.h"

#include <string>


//struct TPkgInfo{
//    bool is_header;
//    int length;
//};


class CClientConn
{
public:
    CClientConn();
    ~CClientConn();

    bool Inialise(std::string strServerIp, unsigned int nPort);
//    void SetCallBackToHP();
//    int  StartTcpClient(const char *svr_ip, unsigned short int port);
//    int  StopTcpClient(void);
//    int  HandleTcpClientEvent(int warn_type, int file_type, const char *path);
//    void ReleaseHPSocket();

    void OnTimer(uint64_t curr_tick);
    void UpdateConnStatus(enNetStatus status);  // 更新連接狀態
    enNetStatus GetConnStatus();  // 取当前socket的连接状态
    void UpdateKeepAliveTick(uint64_t keepAliveTm);  // 更新保活时间
    void UpdateRecvPktTick(uint64_t recvPktTick);  // 更新最近一次接收报文的时间
    void ProcessMsg(BYTE* buf, size_t nLen);
    int Connect();  // 连接服务器
    void DoRegister();
    void DoAuth();
    void DoHeartBeat();
    void DoLocationUp(); // 位置信息上报
    void SetLocationInfo(uint64_t latitude,uint64_t longitude,uint32_t  height); // 设置GPS信息
    void ResetLocation(); // 重置GPS的值
    bool IsLocationSet(); // GPS是否有值

private:
    //HP_TcpPullClientListener m_listener;
    //HP_TcpPullClient m_client;
    //TPkgInfo m_pkgInfo;

    std::string m_strServerIp;   // 服务器端的IP地址
    unsigned  int m_nServerPort;  // 服务器端的端口号
    unsigned int m_nClientFd; // 客户端和服务器端的通信句柄
    bool m_bInialised; // 标记是否已经初始化成功

    uint64_t m_last_keepAlive_send_tick;  // 最近发送保活报文的时间
    uint64_t m_last_recv_tick; // 最近接收报文的时间
    uint64_t m_last_send_loc_tick; // 最近一次上报位置信息的时间
    uint64_t m_next_timer_tick;
    enNetStatus m_conn_status;  // 當前網絡連接狀態

    // gps 信息
    uint64_t  m_latitude;
    uint64_t  m_longitude;
    uint32_t  m_height;
};


#endif //DSM_JTT808_HP_TCP_COMM_H
