//
// Created by public on 19-3-19.
//

#include "client_conn.h"

//using namespace HPSocketHelper;

CClientConn::CClientConn() :m_nClientFd(0),
                            m_nServerPort(0),
                            m_conn_status(NET_DISCONNECTED),// 初始状态置为连接断开
                            m_bInialised(false),
                            m_latitude(0),
                            m_longitude(0),
                            m_height(0)
{
    uint64_t nCurrentTick = CUtil::GetInstance()->get_tick_count();
    m_last_send_loc_tick = nCurrentTick;
    m_last_recv_tick = nCurrentTick;
    m_last_keepAlive_send_tick = nCurrentTick;
    m_next_timer_tick = nCurrentTick + CHECK_TIMER;
}

CClientConn::~CClientConn()
{

}


int CClientConn::Connect()
{


    if (!m_bInialised) {
        CDSMLog::Fatal("Inialised failed before connect.");
        return -1;
    }

    if ((m_nClientFd = StartTcpClient(m_strServerIp.c_str(),m_nServerPort)) != -1) {
        // 连接成功
        LOGE(" 连接成功 ----");
        //UpdateConnStatus(NET_CONNECTED);
        return m_nClientFd;
    } else {
        //连接失败
        LOGE(" 连接失败 ----");
        //UpdateConnStatus(NET_DISCONNECTED);
        return -1;
    }
}


bool CClientConn::Inialise(std::string strServerIp, unsigned int nPort)
{
    LOGE("JNI inside Inialise -- call ----");
    m_nServerPort = nPort;
    LOGE("JNI inside Inialise -- nPort ----");

    m_strServerIp = strServerIp;
    LOGE("JNI inside Inialise -- strServerIp ----");

    m_bInialised = true;
    LOGE("JNI inside Inialise -- m_bInialised ----");

    return m_bInialised;
}

//int CClientConn::StartTcpClient(const char *svr_ip, unsigned short int port)
//{
//    CDSMLog::Trace("StartTcpClient server ip addr %s port %d ...\n", svr_ip, port);
//    ut_uint16 conn_fd = 0;
//    if(HP_Client_Start(m_client, svr_ip, port, 0)){
//        CDSMLog::Info("Start serverip ....ok\n");
//
//        /* set heart break time, default 20 second now */
//        //dsmapp_srv_heart_set(20);
//
//        /*  connected session maintain --> ( one client connection ==  one  connect fd )  */
//        conn_fd = (WORD)HP_Client_GetConnectionID(m_client);
//
//        /* notice the event that DSM client has been connected  */
//        //dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_CONNECTED);
//        return 1;
//    }
//    else {
//        CDSMLog::Error("start server ip failed errno: %d , error desc %s\n", HP_Client_GetLastError(m_client), HP_Client_GetLastErrorDesc(m_client));
//        return -1;
//    }
//}
//
//int CClientConn::StopTcpClient(void)
//{
//    if(HP_Client_Stop(m_client)){
//
//        ut_uint16 conn_fd = 0;
//        conn_fd = (ut_uint16)HP_Client_GetConnectionID(m_client);
//
//        CDSMLog::Trace("Shutdown the connect socketFD %d !\n", conn_fd);
//        //dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_PIPE_CLOSED);
//
//        return 1;
//    }
//    else {
//        CDSMLog::Error("Stop Tcp Client error :%d %s \n", HP_Client_GetLastError(m_client), HP_Client_GetLastErrorDesc(m_client));
//        return -1;
//    }
//}
//
//
//
//void CClientConn::SetCallBackToHP()
//{
//    m_listener      = Create_HP_TcpPullClientListener();
//    m_client        = Create_HP_TcpPullClient(m_listener);
//
//    HP_Set_FN_Client_OnConnect(m_listener, HPSocketHelper::OnConnect);
//    HP_Set_FN_Client_OnSend(m_listener, HPSocketHelper::OnSend);
//    HP_Set_FN_Client_OnPullReceive(m_listener, HPSocketHelper::OnReceive);
//    HP_Set_FN_Client_OnClose(m_listener, HPSocketHelper::OnClose);
//
//    HP_Client_SetExtra(m_client, &m_pkgInfo);
//    HP_TcpClient_SetKeepAliveTime(m_client, 0 );
//
//    return;
//}
//
//
//void CClientConn::ReleaseHPSocket()
//{
//    Destroy_HP_TcpPullClient(m_client);
//    Destroy_HP_TcpPullClientListener(m_listener);
//}

void CClientConn::ProcessMsg(BYTE* buf, size_t nLen)
{
    m_last_recv_tick = CUtil::GetInstance()->get_tick_count();
    CMsgProcess::GetInstance()->ProcessMsg(this,buf,nLen);
}

/**
 * 更新最后一次收到保活回应的时间
 * @param keepAliveTm
 */
void CClientConn::UpdateKeepAliveTick(uint64_t keepAliveTm)
{
    m_last_keepAlive_send_tick = keepAliveTm;
}


void CClientConn::OnTimer(uint64_t curr_tick)
{
    if (curr_tick < m_next_timer_tick)
        return;

    CDSMLog::Trace("OnTimer");
    if (curr_tick > m_last_keepAlive_send_tick + SERVER_HEARTBEAT_INTERVAL) {
        // 超时发送保活报文
        CDSMLog::Trace("Send HeartBeat pkt.");
        CMsgProcess::GetInstance()->DevHeartBeat(); // 发保活报文
        UpdateKeepAliveTick(CUtil::GetInstance()->get_tick_count());// 更新發包的時間
    }

    //通过更新收到包的时间来确定服务端是否掉线.
    if ((curr_tick > m_last_recv_tick + SERVER_TIMEOUT) && GetConnStatus() != NET_DISCONNECTED) {
        CDSMLog::Error("connect to  server timeout");
        UpdateConnStatus(NET_DISCONNECTED);
        StopTcpClient();  // 关闭socket连接
        UpdateRecvPktTick(CUtil::GetInstance()->get_tick_count());
    }

    if (GetConnStatus() >= NET_AUTHENTICATED) {
        if ((curr_tick > m_last_send_loc_tick + LOC_UP_TIMER) ) {
            CDSMLog::Trace("Send device location pkt.");
            //CMsgProcess::GetInstance()->DevLocationUp();
            this->DoLocationUp();
            m_last_send_loc_tick = CUtil::GetInstance()->get_tick_count();
        }
    }

    m_next_timer_tick = CUtil::GetInstance()->get_tick_count() + CHECK_TIMER;
}


void CClientConn::SetLocationInfo(uint64_t latitude,uint64_t longitude,uint32_t  height)
{
    m_latitude = latitude;
    m_longitude = longitude;
    m_height = height;
}

/**
 * 更新最近一次接收报文的时间
 * @param sendPktTick
 */
void CClientConn::UpdateRecvPktTick(uint64_t recvPktTick)
{
    m_last_recv_tick = recvPktTick;
}


/**
 * 更新连接状态
 * @param status
 */
void CClientConn::UpdateConnStatus(enNetStatus status)
{
    CDSMLog::Trace("%x Update connect status[%d]",this,status);
    m_conn_status = status;
}


/**
 * 取当前的连接状态
 * @return
 */
enNetStatus CClientConn::GetConnStatus()
{
    return  m_conn_status;
}

void CClientConn::DoRegister()
{
    CDSMLog::Trace("Start DoRegister");
    CMsgProcess::GetInstance()->DevRegister();
}

void CClientConn::DoAuth()
{
    CDSMLog::Trace("Start DoAuth");
    CMsgProcess::GetInstance()->DevAuthentication();
}

void CClientConn::DoHeartBeat()
{
    CDSMLog::Trace("Start DoHeartBeat");
    CMsgProcess::GetInstance()->DevHeartBeat();
}

void CClientConn::DoLocationUp()
{
    CDSMLog::Trace("Start DoLocationUp");
    if (!IsLocationSet()) {
        CDSMLog::Info("information of gps have not been set,can`t up");
        return;
    }

    CMsgProcess::GetInstance()->DevLocationUp(m_latitude,m_longitude,m_height);
}

/**
 * 重置GPS的值
 */
void CClientConn::ResetLocation()
{
    CDSMLog::Trace("Reset location");
    m_latitude = 0;
    m_longitude = 0;
    m_height = 0;
}


/***
 * GPS是否有值
 * @return
 */
bool CClientConn::IsLocationSet()
{
    if (m_longitude != 0 && m_latitude != 0 && m_height != 0)
        return true;

    return false;
}