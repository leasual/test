//
// Created by public on 19-3-19.
//

#include <stdio.h>
#include <stdlib.h>
//#include <sstream>

#include "client_conn.h"
#include "base/dsm_log.h"
#include "base/type_def.h"
#include "msg_process.h"
#include "client_conn_manager.h"
#include "ut_timer.h"


CClientConn::CClientConn():
        m_euConnType(euConnectInit),
        m_nClientFd(-1),
        m_nServerPort(0),
        m_conn_status(NET_DISCONNECTED),// 初始状态置为连接断开
        m_latitude(0),
        m_longitude(0),
        m_height(0),
		m_bTimer(true),
		m_bInialised(false)
{
    uint64_t nCurrentTick = CUtil::GetInstance()->get_tick_count();
    m_last_send_loc_tick = nCurrentTick;
    m_last_recv_tick = nCurrentTick;
    m_last_keepAlive_send_tick = nCurrentTick;

	m_listener = Create_HP_TcpPullClientListener();
	m_client = Create_HP_TcpPullClient(m_listener);

	HP_Set_FN_Client_OnConnect(m_listener, OnConnect);
	HP_Set_FN_Client_OnSend(m_listener, OnSend);
	HP_Set_FN_Client_OnPullReceive(m_listener, OnReceive);
	HP_Set_FN_Client_OnClose(m_listener, OnClose);

	HP_Client_SetExtra(m_client, &m_pkgInfo);
	HP_TcpClient_SetSocketBufferSize(m_client,64*1024);
    DWORD  dwSize = HP_TcpClient_GetSocketBufferSize(m_client);
	UT_TRACE("Set buffersize to %d",dwSize);

	HP_TcpClient_SetKeepAliveTime(m_client, 0);

	keepAlive_tiemr_callBack =
			std::bind(&CClientConn::_DoHeartBeat,this,std::placeholders::_1);

	server_timeout_callBack =
			std::bind(&CClientConn::_CheckServerTimeout,this,std::placeholders::_1);
}

bool CClientConn::Inialise(const std::string& strServerIp, unsigned int nPort,const std::string& strSimNo,
						  const std::string& strDevMode,const euConnType& connType)
{
	m_euConnType = connType;
	m_strSimNo = strSimNo;
	m_strDevModel = strDevMode;
	m_strServerIp = strServerIp;
	m_nServerPort = nPort;
	m_bInialised = true;

	if (connType == euConnectPlt) {
        //UTTimer::GetInstance()->AddTimer(&keepAlive_tiemr_callBack,SERVER_HEARTBEAT_INTERVAL);
        UTTimer::GetInstance()->AddTimer(&server_timeout_callBack,SERVER_TIMEOUT);
	}
	return true;
}

CClientConn::~CClientConn()
{
	ReleaseHPSocket();
	m_nClientFd = -1;
	if (m_euConnType == euConnectPlt) {
		UTTimer::GetInstance()->RemoveTimer(&keepAlive_tiemr_callBack);
		UTTimer::GetInstance()->RemoveTimer(&server_timeout_callBack);		
	}

	//UT_TRACE("Release client[fd=%d] connect.",m_nClientFd);
}

void CClientConn::AddKeepAliveTimer()
{
	if (m_euConnType == euConnectPlt) {
		UTTimer::GetInstance()->AddTimer(&keepAlive_tiemr_callBack,SERVER_HEARTBEAT_INTERVAL);
	}
}

void CClientConn::RemoveKeepAliveTimer()
{
    if (m_euConnType == euConnectPlt) {
        UT_TRACE("Start remove keep aliver timer.");
        UTTimer::GetInstance()->RemoveTimer(&keepAlive_tiemr_callBack);
    }
}


En_HP_HandleResult CClientConn::OnConnect(HP_Client pSender, HP_CONNID dwConnID)
{
	TCHAR szAddress[50] = { 0 };
	int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
	USHORT usPort;

	HP_Client_GetRemoteHost(pSender, szAddress, &iAddressLen, &usPort);
	UT_TRACE("Connect remoter IP[%s] Port[%d]", szAddress, usPort);
	return HR_OK;
}


En_HP_HandleResult CClientConn::OnReceive(HP_Client pSender, HP_CONNID dwConnID, int iLength)
{
	struct HpktInfo * pInfo = (struct HpktInfo*)HP_Client_GetExtra(pSender);
	if (pInfo == NULL) {
		UT_FATAL("Null packet Error\n");
		return HR_OK;
	}

	char buffer[1024 * 4] = { 0 };
	UT_TRACE("Received buffer which length[%d]", iLength);
	En_HP_FetchResult result = HP_TcpPullClient_Fetch(pSender, (BYTE*)buffer, (int)iLength);
	if (result == FR_OK)
	{
		UT_TRACE("HP_TcpPullClient_Fetch %d bytes successfully.\n", iLength);
		//UT_DUMP(buffer, iLength);

		CClientConnManager::GetInstance()->ReceivePkt(dwConnID, (BYTE*)buffer, iLength);
	}
	return HR_OK;
}

En_HP_HandleResult CClientConn::OnSend(HP_Client pSender, HP_CONNID dwConnID, const BYTE* pData, int iLength)
{
	return HR_OK;
}


En_HP_HandleResult CClientConn::OnClose(HP_Client pSender, HP_CONNID dwConnID, En_HP_SocketOperation enOperation, int iErrorCode)
{
    UT_TRACE("-->Recived OnClose single!");
	CClientConnManager::GetInstance()->UpdateConnStatus(dwConnID, NET_DISCONNECTED);

	return HR_OK;
}

/**
 *
 * @param strServerIp
 * @param nPort
 * @param bTimer  是否需要启动定时器
 * @return
 */
int CClientConn::Connect()
{
	if (!m_bInialised) {
		UT_FATAL("Connect to server before inialised.");
		return -1;
	}

	UT_TRACE("StartTcpClient server ip addr %s port %d ", m_strServerIp.c_str(), m_nServerPort);
	if (HP_Client_Start(m_client, m_strServerIp.c_str(), m_nServerPort, 0)) {
		UT_TRACE("Start serverip ....ok.");
		m_nClientFd = (WORD)HP_Client_GetConnectionID(m_client);
		UT_TRACE("Connect successful,peer handle[%d]", m_nClientFd);
//		m_bTimer = true;
//		if (m_bTimer) {
//			// 是否启动timer
//			m_next_timer_tick = CUtil::GetInstance()->get_tick_count() + CHECK_TIMER;
//		}
		UpdateConnStatus(NET_CONNECTED);
		return m_nClientFd;
	}
	else {
		UT_ERROR("Start server ip failed errno: %d , error desc %s\n",
			HP_Client_GetLastError(m_client), HP_Client_GetLastErrorDesc(m_client));
		UpdateConnStatus(NET_DISCONNECTED);
		return -1;
	}

    //if ((m_nClientFd = CDsmJTT808_API::GetInstance()->StartTcpClient_API(m_strServerIp.c_str(),m_nServerPort)) != -1) {
    ////if ((m_nClientFd = ::StartTcpClient(refClient,m_strServerIp.c_str(),m_nServerPort)) != -1) {
    //    // 连接成功
    //    UpdateConnStatus(NET_CONNECTED);
    //    return m_nClientFd;
    //} else {
    //    //连接失败
    //    UpdateConnStatus(NET_DISCONNECTED);
    //    return -1;
    //}
}

void CClientConn::Reset()
{
	m_mapFileInfo.clear();
}

int  CClientConn::ClientSend(const BYTE* buff, size_t nBuffLen)
{
	if (NULL == buff || nBuffLen <= 0) {
		UT_FATAL("Parameter buff or nBuffLen[%zu] is Error!", nBuffLen);
		return -1;
	}

	if (HP_Client_Send(m_client, buff, nBuffLen)) {
		UT_TRACE("send peer[%lu] buffer[%lu]\n", HP_Client_GetConnectionID(m_client), nBuffLen);
	}
	else {
		UT_ERROR("send failed %lu, %d %s\n", HP_Client_GetConnectionID(m_client), SYS_GetLastError(), SYS_GetLastErrorStr());
		return -1;
	}
	return 1;
}

int CClientConn::UpdateConnectFd(int nClientFd)
{
    if (nClientFd != -1) {
        m_nClientFd = nClientFd;
        UpdateConnStatus(NET_CONNECTED);
    } else {
        UpdateConnStatus(NET_DISCONNECTED);
        m_nClientFd = -1;
    }

    return m_nClientFd;
}

void CClientConn::ReleaseHPSocket()
{
    Destroy_HP_TcpPullClient(m_client);
    Destroy_HP_TcpPullClientListener(m_listener);
}

void CClientConn::ProcessMsg(BYTE* buf, size_t nLen)
{
    m_last_recv_tick = CUtil::GetInstance()->get_tick_count();

	UT_TRACE("-->The last received time:%s",CUtil::GetInstance()->GetCurrentTm().c_str());
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


//void CClientConn::OnTimer(uint64_t curr_tick)
//{
////	if (!m_bTimer || curr_tick < m_next_timer_tick)
////		return;
//
//	if (curr_tick < m_next_timer_tick)
//		return;
//
//    UT_TRACE("Client[%d] to server {Type[%d] IP[%s] Port[%d]} OnTimer", m_nClientFd,m_euConnType,m_strServerIp.c_str(), m_nServerPort);
//    if (m_euConnType == euConnectPlt  && curr_tick > m_last_keepAlive_send_tick + SERVER_HEARTBEAT_INTERVAL) {
//        // 超时发送保活报文
//        UT_TRACE("Send HeartBeat pkt.");
//        CMsgProcess::GetInstance()->DevHeartBeat(this); // 发保活报文
//        UpdateKeepAliveTick(CUtil::GetInstance()->get_tick_count());// 更新發包的時間
//    }
//
//    //通过更新收到包的时间来确定服务端是否掉线.
//    if ((curr_tick > m_last_recv_tick + SERVER_TIMEOUT)) {
//    	if (GetConnStatus() != NET_DISCONNECTED) {
//			UT_ERROR("Connect to  server Type[%d] timeout",m_euConnType);
//			UpdateConnStatus(NET_DISCONNECTED); // 更新状态至"掉线"
//			HP_Client_Stop(m_client);
//			UpdateRecvPktTick(CUtil::GetInstance()->get_tick_count());
//    	}
//    }
//
//	if (m_euConnType == euConnectPlt  && (curr_tick > m_last_send_loc_tick + LOC_UP_TIMER)) {
//    	if (GetConnStatus() >= NET_AUTHENTICATED) {
//			if ((curr_tick > m_last_send_loc_tick + LOC_UP_TIMER)) {
//				UT_TRACE("Send device location pkt.");
//				this->DoLocationUp();
//				m_last_send_loc_tick = CUtil::GetInstance()->get_tick_count();
//			}
//    	}
//    }
//
//    m_next_timer_tick = CUtil::GetInstance()->get_tick_count() + CHECK_TIMER;
//}

void CClientConn::_DoHeartBeat(uint64_t curr_tick)
{
	UT_TRACE("Start _DoHeartBeat");
	CMsgProcess::GetInstance()->DevHeartBeat(this);
}

void CClientConn::_CheckServerTimeout(uint64_t curr_tick)
{
	UT_TRACE("-->_CheckServerTimeout:%s",CUtil::GetInstance()->GetCurrentTm().c_str());
    //通过更新收到包的时间来确定服务端是否掉线.
    if ((curr_tick > m_last_recv_tick + SERVER_TIMEOUT)) {
        if (GetConnStatus() != NET_DISCONNECTED) {
            UT_ERROR("Connect to  server Type[%d] timeout",m_euConnType);
            UpdateConnStatus(NET_DISCONNECTED); // 更新状态至"掉线"
            HP_Client_Stop(m_client);
            UpdateRecvPktTick(CUtil::GetInstance()->get_tick_count());

			// 立即启动重新连接的操作
			CClientConnManager::GetInstance()->AddCheckConnectTimer();
            if (GetConnType() == euConnectPlt) {
                this->RemoveKeepAliveTimer();  // 已经掉线,不需要再发保活报文
                CClientConnManager::GetInstance()->RemoveGpsUpLoadTimer();
                CClientConnManager::GetInstance()->RemoveAlarmUpLoadTimer();
            }
        }
    }
}

//void CClientConn::SetLocationInfo(uint64_t latitude,uint64_t longitude,uint32_t  height)
//{
//    m_latitude = latitude;
//    m_longitude = longitude;
//    m_height = height;
//}

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
    //UT_TRACE("%x Update connect status[%d]",this,status);
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
    UT_TRACE("Start DoRegister");
    CMsgProcess::GetInstance()->DevRegister(this);
}

void CClientConn::DoAuth()
{
    UT_TRACE("Start DoAuth");
    CMsgProcess::GetInstance()->DevAuthentication(this);
}

void CClientConn::DoLocationUp()
{
    UT_TRACE("Start DoLocationUp");
//    if (!IsLocationSet()) {
//        UT_INFO("information of gps have not been set,can`t up");
//        return;
//    }

    if (GetConnStatus() >= NET_AUTHENTICATED)
	    CMsgProcess::GetInstance()->DevLocationUp(this);
}


void CClientConn::DoAlarmInfoUp()
{
	UT_TRACE("Start DoAlarmInfoUp");
	if (GetConnStatus() >= NET_AUTHENTICATED)
	    CMsgProcess::GetInstance()->DevAlarmInfoUp(this);
}


void CClientConn::SetAlarmFlag(const std::string& refAlarmFalg)
{
    m_strAlarmFlag = refAlarmFalg;
}

const char* CClientConn::GetSimNo()
{
    return m_strSimNo.c_str();
}

const char* CClientConn::GetDevModel()
{
    return m_strDevModel.c_str();
}


/**
 * 重置GPS的值
 */
void CClientConn::ResetLocation()
{
    UT_TRACE("Reset location");
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


bool CClientConn::UpdateUpFileStatus(const std::string& strFileName,const euFileUpStatus& st)
{
	std::map<std::string,FileInfo>::iterator pIterFound =
			m_mapFileInfo.find(strFileName);
	if (pIterFound == m_mapFileInfo.end()) {
		UT_ERROR("Update file[%s] status failed!",strFileName.c_str());
		return false;
	} else {
		pIterFound->second.m_fileStatus = st;
		UT_INFO("Update file[%s] status success!",strFileName.c_str());
		return true;
	}
}

FileInfo* CClientConn::GetUpFileInfo(const std::string &strFileName)
{
	std::map<std::string,FileInfo>::iterator pIterFound =
			m_mapFileInfo.find(strFileName);
	if (pIterFound == m_mapFileInfo.end()) {
		UT_ERROR("Get file item[%s] failed!",strFileName.c_str());
		return NULL;
	}else{
		UT_INFO("Get file item[%s] success!",strFileName.c_str());
		return &(pIterFound->second);
	}
}

void CClientConn::UpdateUpFileInfo(const std::string& strFileName, const FileInfo& fileInfo)
{
	m_mapFileInfo.insert(std::make_pair(strFileName,fileInfo));
}

void CClientConn::ClearUpFileInfo()
{
	m_mapFileInfo.clear();
}


void CClientConn::PrintFileStatus()
{
    std::map<std::string,FileInfo>::iterator pIter = m_mapFileInfo.begin();
    for(;pIter != m_mapFileInfo.end();pIter++) {
        UT_TRACE("File[%s] up_status[%d]",pIter->first.c_str(),pIter->second.m_fileStatus);
    }
}

bool CClientConn::DelFileInfo(const std::string &strFileName)
{
	std::map<std::string,FileInfo>::iterator pIterFound =
			m_mapFileInfo.find(strFileName);
	if (pIterFound == m_mapFileInfo.end()) {
		UT_ERROR("Delete file item[%s] failed!",strFileName.c_str());
		return false;
	}else{
		UT_INFO("Delete file item[%s] success!",strFileName.c_str());
		m_mapFileInfo.erase(pIterFound);
		return true;
	}
}

