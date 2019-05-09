//
// Created by public on 19-3-25.
//

#include "client_conn_manager.h"
#include "ut_timer.h"

CClientConnManager::CClientConnManager():
        m_bInialsed(false)
{
    gps_timer_callBack =
            std::bind(&CClientConnManager::_DoDevLocUp,this,std::placeholders::_1);

    check_connect_timer_callBack =
            std::bind(&CClientConnManager::_CheckConnected,this,std::placeholders::_1);

    alarm_timer_callBack =
            std::bind(&CClientConnManager::_DoAlarmUp,this,std::placeholders::_1);

    UTTimer::GetInstance()->AddTimer(&check_connect_timer_callBack,CONNECTED_TIMER);
};

CClientConnManager::~CClientConnManager()
{
//    if (NULL != m_pClientConn) {
//        delete m_pClientConn;
//        m_pClientConn = NULL;
//    }

    ITER_CLIENT_CONNS pIter = m_mapClients.begin();
    for (; pIter != m_mapClients.end(); pIter++) {
        delete pIter->second;
    }
    m_mapClients.clear();
    RemoveCheckConnectTimer();
    UTTimer::GetInstance()->RemoveTimer(&gps_timer_callBack);
    UTTimer::GetInstance()->RemoveTimer(&alarm_timer_callBack);
}


void CClientConnManager::AddCheckConnectTimer()
{
    UT_TRACE("Add CheckConnect Timer");
    _CheckConnected();
    UTTimer::GetInstance()->AddTimer(&check_connect_timer_callBack,CONNECTED_TIMER);
}

void CClientConnManager::RemoveCheckConnectTimer()
{
    UT_TRACE("Remove CheckConnect Timer");
    UTTimer::GetInstance()->RemoveTimer(&check_connect_timer_callBack);
}


void CClientConnManager::AddGpsUpLoadTimer()
{
    UT_TRACE("Add Gps UpLoad Timer");
    UTTimer::GetInstance()->AddTimer(&gps_timer_callBack,GPS_UPLOAD_TIMER);
}

void CClientConnManager::RemoveGpsUpLoadTimer()
{
    UT_TRACE("Remove Gps UpLoad Timer");
    UTTimer::GetInstance()->RemoveTimer(&gps_timer_callBack);
}

void CClientConnManager::AddAlarmUpLoadTimer()
{
    UT_TRACE("Add Alarm UpLoad Timer");
    UTTimer::GetInstance()->AddTimer(&alarm_timer_callBack,ALARM_UPLOAD_TIMER);
}

void CClientConnManager::RemoveAlarmUpLoadTimer()
{
    UT_TRACE("Remove Alarm UpLoad Timer");
    UTTimer::GetInstance()->RemoveTimer(&alarm_timer_callBack);
}


void CClientConnManager::CacheAlarmUpLoadInfo(WORD msgId, WORD seqNo,DevUploadGPSAlarmInfo** ppAlarmInfo) // 缓存报警信息　
{
	std::lock_guard<std::mutex> locker(m_cache_alarmInfo_lock);
    DWORD dwMsgNo = msgId << sizeof(WORD) | seqNo;
    std::pair<ITER_UP_ALARM,bool> ret =
            m_mapUpAlarmInfo.insert(std::make_pair(dwMsgNo,*ppAlarmInfo));

    if (ret.second == false) {
        UT_WARN("MsgId[%hu] seqNo[%hu] have already existed,cached failed!",msgId,seqNo);
		return;
    }
	
	UT_INFO("MsgId[%hu] seqNo[%hu] have cached sucess.",msgId,seqNo);
	return;
}

BOOL CClientConnManager::RemoveAlarmUpLoadInfo(WORD msgId, WORD seqNo,DevUploadGPSAlarmInfo** pAlarmInfo)
{
	std::lock_guard<std::mutex> locker(m_cache_alarmInfo_lock);
	DWORD dwMsgNo = msgId << sizeof(WORD) | seqNo;
	ITER_UP_ALARM pIterFound = m_mapUpAlarmInfo.find(dwMsgNo);
	if (pIterFound == m_mapUpAlarmInfo.end()) {
		UT_TRACE("Can`t find MsgId[%hu] seqNo[%hu] from m_mapUpAlarmInfo",msgId, seqNo);
		return FALSE;
	}

	*pAlarmInfo = pIterFound->second;
	UT_INFO("MsgId[%hu] seqNo[%hu] have delete success.",msgId,seqNo);
	m_mapUpAlarmInfo.erase(pIterFound);
	UT_INFO("Now alarm info left %lu items.",m_mapUpAlarmInfo.size());
	return TRUE;
}

int CClientConnManager::StartNewConnect(const std::string &strServerIp, unsigned int nPort, const std::string &strSimNo,
                                        const std::string &strDevMode,euConnType connType)
{
    CClientConn* pClientConn = InialiseConnect(strServerIp,nPort,strSimNo,strDevMode,connType);
    return Connect(pClientConn);
}

CClientConn* CClientConnManager::InialiseConnect(const std::string &strServerIp, unsigned int nPort,
                                         const std::string &strSimNo, const std::string &strDevMode,euConnType connType)
{
    CClientConn* pClientConn = new CClientConn;
    pClientConn->Inialise(strServerIp,nPort,strSimNo,strDevMode,connType);
    m_bInialsed = true;
    return pClientConn;
}


CClientConn* CClientConnManager::GetClientConnByFd(unsigned int nClientFd)
{
    ITER_CLIENT_CONNS pIter = m_mapClients.find(nClientFd);
    if (pIter != m_mapClients.end()) {
        return  pIter->second;
    }

    UT_FATAL("Get Client[%d] failed!",nClientFd);
    return  NULL;
}


/**
 *  删除客户端连接的实例
 * @param nClientFd
 * @return
 */
bool CClientConnManager::DeleteClientConnect(int nClientFd)
{
    ITER_CLIENT_CONNS pIter = m_mapClients.find(nClientFd);
    if (pIter != m_mapClients.end()) {
        UT_TRACE("Delete client fd[%d] type[%d] success",
                 pIter->first,pIter->second->GetConnType());

        delete (pIter->second);
        pIter = m_mapClients.erase(pIter);

        UT_TRACE("Now left [%lu] connects", m_mapClients.size());
        return true;
    }
    return  false;
}


/**
 * 清空客户端连接实例，并不真正删除．
 * @param nClientFd
 * @return
 */
bool CClientConnManager::ReleaseClientConnect(int nClientFd)
{
    ITER_CLIENT_CONNS pIter = m_mapClients.find(nClientFd);
    if (pIter != m_mapClients.end()) {
//        if (pIter->second->GetConnType() == euConnectAccessory) {
//            UT_TRACE("Delete the accessory server!");
//            delete (pIter->second);
//        } else {
//            pIter->second->Reset();
//        }

        UT_TRACE("Releasae old client fd[%d] type[%d] success",
                 pIter->first,pIter->second->GetConnType());
        pIter->second->Reset();
        pIter = m_mapClients.erase(pIter);

        UT_TRACE("Now left [%lu] connects", m_mapClients.size());
        return true;
    }
    return  false;
}

void CClientConnManager::ReceivePkt(unsigned int nClientFd,BYTE* buf, size_t nLen)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        return;
    }
    pClientConn->ProcessMsg(buf,nLen);
}


bool CClientConnManager::UpdateConnStatus(unsigned int nClientFd,enNetStatus euStatus)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        UT_ERROR("Client[%d] update status[%d] failed!",nClientFd,euStatus);
        return false;
    }

    pClientConn->UpdateConnStatus(euStatus);
    UT_TRACE("Client[%d] update status[%d] success!",nClientFd,euStatus);
    return true;
}


enNetStatus CClientConnManager::GetClientConnStatus(unsigned int nClientFd) // 取客户端连接的状态
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        UT_ERROR("Client[%d] get status failed!",nClientFd);
        return NET_INIT;
    }

    return  pClientConn->GetConnStatus();
}


bool CClientConnManager::DoRegister(unsigned int nClientFd)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        UT_ERROR("Client[%d] call DoRegister() failed!",nClientFd);
        return false;
    }
    pClientConn->DoRegister();

    return  true;
}


bool CClientConnManager::DoAuth(unsigned int nClientFd)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        UT_ERROR("Client[%d] call DoAuth() failed!",nClientFd);
        return false;
    }
    pClientConn->DoAuth();

    return  true;
}


BOOL CClientConnManager::CheckPltConnectStatus()
{
    ITER_CLIENT_CONNS pIter = m_mapClients.begin();
    for (; pIter != m_mapClients.end() ; pIter++) {
        if (pIter->second->GetConnType() == euConnectPlt && pIter->second->GetConnStatus() >= NET_AUTHENTICATED)
            return true;
    }
    return false;
}

void CClientConnManager::SetAlarmInfo(DevUploadGPSAlarmInfo* spAlarmInfo)
{
    if (m_queueAlarm.IsEmpty()) {
        AddAlarmUpLoadTimer();
    }
    m_queueAlarm.PushBack(spAlarmInfo);

    // 生产环境中,如下代码需要打开.
//    if (CheckPltConnectStatus()) {
//        m_queueAlarm.PushBack(spAlarmInfo.get());
//    } else {
//        UT_INFO("Client disconnected,push alarm info failed!");
//    }
}

void CClientConnManager::SetLocation(std::shared_ptr<UploadGPSInfo>& spGpsInfo)
{
    m_queueGps.PushBack(spGpsInfo.get());

    // 生产环境中,如下代码需要打开.
//    if (CheckPltConnectStatus()) {
//        m_queueGps.PushBack(spGpsInfo.get());
//    } else {
//        UT_INFO("Client disconnected,push gps info failed!");
//    }
}

void CClientConnManager::UpdateAlarmFlag(const std::string& strAlarmFlag,std::vector<AlarmAccessory>& refAccessories)
{
    UT_TRACE("Update alarm flag[%s] success.",strAlarmFlag.c_str());
    m_mapAccessories.insert(std::make_pair(strAlarmFlag,refAccessories));
}

bool CClientConnManager::GetAlarmAccessory(const std::string& strAlarmFlag, std::vector<AlarmAccessory> &refAccessories)
{
    std::map<std::string,std::vector<AlarmAccessory> >::iterator pIterFound =
            m_mapAccessories.find(strAlarmFlag);

    if (pIterFound == m_mapAccessories.end()) {
        UT_ERROR("!!!Cant find alarm flag[%s]",strAlarmFlag.c_str());
        return false;
    }
    refAccessories = pIterFound->second;

    UT_TRACE("Find alarm flag[%s] success.",strAlarmFlag.c_str());
    return true;
}

/**
 * 从报警附件容器中删除报警标识对应的附件
 * @param strAlarmFlag  报警标识的16进制字符串
 * @return
 */
bool CClientConnManager::DelAlarmFlag(std::string strAlarmFlag)
{
    std::map<std::string,std::vector<AlarmAccessory> >::iterator pIterFound =
            m_mapAccessories.find(strAlarmFlag);
    if (pIterFound != m_mapAccessories.end()) {
        m_mapAccessories.erase(pIterFound);
        UT_TRACE("Erase alarm flag[%s] success.",strAlarmFlag.c_str());
        return true;
    }
    UT_TRACE("!!!Erase alarm flag[%s] failed.",strAlarmFlag.c_str());
    return false;
}

int CClientConnManager::Connect(CClientConn* pClientConn)
{
    if (!m_bInialsed) {
        UT_ERROR("Haven`t inialised!");
        return -1;
    }

    if (NULL == pClientConn) {
        UT_FATAL("Pointer to pClientConn is NULL.");
        return -1;
    }

    int nOldClientFd = pClientConn->GetClientHandle();
    int nClientFd =  pClientConn->Connect();
    if (nClientFd > 0) {
        // 连接成功了,从map中删除之前的缓存
        this->ReleaseClientConnect(nOldClientFd);

        m_mapClients.insert(std::make_pair(nClientFd, pClientConn));
        UT_TRACE("Insert new client fd[%d] type[%d] success,Now left [%lu] connects",
                 nClientFd,pClientConn->GetConnType(),m_mapClients.size());

        pClientConn->UpdateConnectFd(nClientFd);
    }
    return nClientFd;
}

void CClientConnManager::_DoDevLocUp(uint64_t curr_tick)
{
    UT_TRACE("###_DoDevLocUp");
    ITER_CLIENT_CONNS pIter = m_mapClients.begin();
    for (;pIter != m_mapClients.end();pIter++) {
        if(pIter->second->GetConnType() == euConnectPlt) {
            pIter->second->DoLocationUp();
        }
    }
}

void CClientConnManager::_DoAlarmUp(uint64_t)
{
    UT_TRACE("###_DoAlarmUp");
    ITER_CLIENT_CONNS pIter = m_mapClients.begin();
    for (;pIter != m_mapClients.end();pIter++) {
        if(pIter->second->GetConnType() == euConnectPlt) {
            pIter->second->DoAlarmInfoUp();
        }
    }
}

void CClientConnManager::_CheckConnected(uint64_t curr_tick)
{
    UT_TRACE("###_CheckConnected");

    int nClientFd = -1;
//    if (NULL != m_pClientConn && m_pClientConn->GetConnStatus() == NET_DISCONNECTED) {
//        UT_TRACE("Start reconnect...");
//
//        ITER_CLIENT_CONNS pIter = m_mapClients.find(m_pClientConn->GetClientHandle());
//        if (pIter != m_mapClients.end()) {
//            UT_TRACE("Releasae client fd[%d] type[%d] success",
//                     pIter->first, pIter->second->GetConnType());
//            pIter->second->Reset();
//            pIter = m_mapClients.erase(pIter);
//            UT_TRACE("Now left [%lu] connects", m_mapClients.size());
//        } // end if
//
//        nClientFd = Connect(m_pClientConn);
//    }


    ITER_CLIENT_CONNS pIter = m_mapClients.begin();
    for (; pIter != m_mapClients.end(); pIter++) {
        nClientFd = pIter->first;
        if (GetClientConnStatus(pIter->first) == NET_DISCONNECTED) {
            // 重新连接
            UT_TRACE("Start reconnect...");
            nClientFd = CClientConnManager::GetInstance()->ClientReconnect(pIter->first);
            return;
        }

        if (GetClientConnType(nClientFd) == euConnectAccessory)
            continue;  // 附件服务器不涉及到注册 监权

        enNetStatus euStatus = CClientConnManager::GetInstance()->GetClientConnStatus(nClientFd);
        if (euStatus == NET_CONNECTED) {
            // 已经建立连接成功了,但是还没有注册成功,发起注册
            CClientConnManager::GetInstance()->DoRegister(nClientFd);
        }

        if (euStatus >= NET_REGISTED && euStatus != NET_AUTHENTICATED) {
            // 已经注册成功了,但是还没有监权成功,发起监权
            CClientConnManager::GetInstance()->DoAuth(nClientFd);
        }
    }
}

/*
void CClientConnManager::OnTimer(uint64_t curr_tick)
{
    DWORD millSecond = 500;
    std::map<unsigned int, CClientConn*>::iterator pIter = m_mapClients.begin();
    unsigned int nClientFd = -1;
    for (; pIter != m_mapClients.end() ; pIter++) {
        pIter->second->OnTimer(curr_tick); // Check timer,keep_alive

        nClientFd = pIter->first;
        if (GetClientConnStatus(pIter->first) == NET_DISCONNECTED) {
            // 重新连接
            UT_TRACE("Start reconnect...");
            nClientFd = CClientConnManager::GetInstance()->ClientReconnect(pIter->first);
            usleep(millSecond * 1000);
            return;
        }

        if (GetClientConnType(nClientFd) == euConnectAccessory)
            continue;  // 附件服务器不涉及到注册 监权

        enNetStatus euStatus = CClientConnManager::GetInstance()->GetClientConnStatus(nClientFd);
        if (euStatus == NET_CONNECTED) {
            // 已经建立连接成功了,但是还没有注册成功,发起注册
            CClientConnManager::GetInstance()->DoRegister(nClientFd);
        }

        if (euStatus >= NET_REGISTED && euStatus != NET_AUTHENTICATED) {
            // 已经注册成功了,但是还没有监权成功,发起监权
            CClientConnManager::GetInstance()->DoAuth(nClientFd);
        }
    }

    usleep(millSecond * 1000); // 让出millSecond毫秒的时间片
}
*/

/**
 *
 * 重新发起连接
 * @param nClientFd 断开连接的通信句柄
 * @return
 */
int CClientConnManager::ClientReconnect(unsigned int nClientFd)
{
    CClientConn* pClientConn = GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        UT_FATAL("Client reconnect failed! Can`t find the connect instance!");
        return -1;
    }

    //this->ReleaseClientConnect(nClientFd);
    UT_INFO("Start Reconnect server{strIp[%s] nPort[%d], type[%d]}",
            pClientConn->GetServerIp().c_str(),pClientConn->GetServerPort(),pClientConn->GetConnType());
    return Connect(pClientConn);
}

const euConnType  CClientConnManager::GetClientConnType(unsigned int nClientFd)
{
    CClientConn* pClientConn = GetClientConnByFd(nClientFd);
    if (NULL == pClientConn)
        return euConnectInit;
    return pClientConn->GetConnType();
}
