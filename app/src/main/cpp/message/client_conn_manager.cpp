//
// Created by public on 19-3-25.
//

#include "client_conn_manager.h"


CClientConnManager::CClientConnManager()
{

};

void CClientConnManager::_RegisterClientConn(unsigned int nClientFd, CClientConn* pClientConn)
{
    m_mapClients.insert(std::make_pair(nClientFd, pClientConn));
}

CClientConn* CClientConnManager::GetClientConnByFd(unsigned int nClientFd)
{
    std::map<unsigned int, CClientConn*>::iterator pIter = m_mapClients.find(nClientFd);
    if (pIter != m_mapClients.end()) {
        return  pIter->second;
    }

    UT_FATAL("Get Client[%d] failed!",nClientFd);
    return  NULL;
}


bool CClientConnManager::DeleteClientConnect(unsigned int nClientFd)
{
    std::map<unsigned int, CClientConn*>::iterator pIter = m_mapClients.find(nClientFd);
    if (pIter != m_mapClients.end()) {
        delete (pIter->second);
        m_mapClients.erase(pIter);
        UT_TRACE("Releasae client fd[%d] success,now left [%lu] connects",nClientFd,m_mapClients.size());
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

void CClientConnManager::SetLocation(DevLocInfo& refDevLocInfo)
{
    m_queueDevLoc.PushBack(&refDevLocInfo);
}

void CClientConnManager::SetLocation(unsigned int nClientFd,uint64_t latitude,uint64_t longitude,uint32_t  height,WORD speed, bool bAlarm,
                                     euAlarmType alarmType,int nAlarmChannel,std::vector<AlarmAccessory>& accessories)
{
//    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
//    if (NULL == pClientConn) {
//        UT_ERROR("Client[%d] call DoAuth() failed!",nClientFd);
//        return;
//    }
//    pClientConn->SetLocationInfo(latitude,longitude,height);

    DevLocInfo* pLocInfo = new DevLocInfo(latitude,longitude,height,speed,bAlarm,alarmType,nAlarmChannel,accessories);
    m_queueDevLoc.PushBack(pLocInfo);
}


//bool CClientConnManager::GetLocation(DevLocInfo* pDevLocInfo)
//{
//    if (m_queueDevLoc.IsEmpty())
//        return  false;
//
//    m_queueDevLoc.PopFront(&pDevLocInfo);
//    return  true;
//}

void CClientConnManager::UpdateAlarmFlag(std::string strAlarmFlag,std::vector<AlarmAccessory>& refAccessories)
{
    UT_TRACE("Update alarm flag[%s] success.",strAlarmFlag.c_str());
    m_mapAccessories.insert(std::make_pair(strAlarmFlag,refAccessories));
}

bool CClientConnManager::GetAlarmAccessory(std::string strAlarmFlag, std::vector<AlarmAccessory> &refAccessories)
{
    std::map<std::string,std::vector<AlarmAccessory> >::iterator pIterFound =
            m_mapAccessories.find(strAlarmFlag);

    if (pIterFound == m_mapAccessories.end()) {
        UT_ERROR("Cant find alarm flag[%s]",strAlarmFlag.c_str());
        return false;
    }
    refAccessories = pIterFound->second;

    UT_TRACE("Find alarm flag[%s] success.",strAlarmFlag.c_str());
    return true;
}


bool CClientConnManager::DelAlarmFlag(std::string strAlarmFlag)
{
    std::map<std::string,std::vector<AlarmAccessory> >::iterator pIterFound =
            m_mapAccessories.find(strAlarmFlag);
    if (pIterFound != m_mapAccessories.end()) {
        m_mapAccessories.erase(pIterFound);
        UT_TRACE("Erase alarm flag[%s] success.",strAlarmFlag.c_str());
        return true;
    }
    UT_TRACE("Erase alarm flag[%s] failed.",strAlarmFlag.c_str());
    return false;
}

int CClientConnManager::Connect(std::string strSimNo, std::string strDevMode, std::string& strIp, unsigned int nPort, bool bTimer)
{
    CClientConn* pClientConn = new CClientConn(strSimNo,strDevMode);
    unsigned int nClientFd =  pClientConn->Connect(strIp,nPort,bTimer);
    m_mapClients.insert(std::make_pair(nClientFd, pClientConn));
    return nClientFd;
}


void CClientConnManager::OnTimer(uint64_t curr_tick)
{
    std::map<unsigned int, CClientConn*>::iterator pIterBeg = m_mapClients.begin();
    for (; pIterBeg != m_mapClients.end() ; pIterBeg++) {
        pIterBeg->second->OnTimer(curr_tick);
    }
}


int CClientConnManager::ClientReconnect(unsigned int nClientFd,std::string& strSimNo,std::string& strDevMode,std::string& strIp, unsigned int nPort)
{
    this->DeleteClientConnect(nClientFd);
    return this->Connect(strSimNo,strDevMode,strIp,nPort);
}