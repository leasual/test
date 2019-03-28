//
// Created by public on 19-3-25.
//

#include "client_conn_manager.h"


void CClientConnManager::RegisterClientConn(unsigned int nClientFd, CClientConn* pClientConn)
{
    m_mapClients.insert(std::make_pair(nClientFd, pClientConn));
}

CClientConn* CClientConnManager::GetClientConnByFd(unsigned int nClientFd)
{
    std::map<unsigned int, CClientConn*>::iterator pIter = m_mapClients.find(nClientFd);
    if (pIter != m_mapClients.end()) {
        return  pIter->second;
    }

    CDSMLog::Fatal("Get Client[%d] failed!",nClientFd);
    return  NULL;
}


bool CClientConnManager::DeleteClientConnect(unsigned int nClientFd)
{
    std::map<unsigned int, CClientConn*>::iterator pIter = m_mapClients.find(nClientFd);
    if (pIter != m_mapClients.end()) {
        m_mapClients.erase(pIter);
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
        CDSMLog::Error("Client[%d] update status[%d] failed!",nClientFd,euStatus);
        return false;
    }

    pClientConn->UpdateConnStatus(euStatus);
    CDSMLog::Trace("Client[%d] update status[%d] success!",nClientFd,euStatus);
    return true;
}


enNetStatus CClientConnManager::GetClientConnStatus(unsigned int nClientFd) // 取客户端连接的状态
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        CDSMLog::Error("Client[%d] get status failed!",nClientFd);
        return NET_INIT;
    }

    return  pClientConn->GetConnStatus();
}


bool CClientConnManager::DoRegister(unsigned int nClientFd)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        CDSMLog::Error("Client[%d] call DoRegister() failed!",nClientFd);
        return false;
    }
    pClientConn->DoRegister();

    return  true;
}


bool CClientConnManager::DoAuth(unsigned int nClientFd)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        CDSMLog::Error("Client[%d] call DoAuth() failed!",nClientFd);
        return false;
    }
    pClientConn->DoAuth();

    return  true;
}


bool CClientConnManager::DoLocationUp(unsigned int nClientFd)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        CDSMLog::Error("Client[%d] call DoAuth() failed!",nClientFd);
        return false;
    }
    pClientConn->DoLocationUp();
    return  true;
}

void CClientConnManager::SetLocation(unsigned int nClientFd,uint64_t latitude,uint64_t longitude,uint32_t  height)
{
    CClientConn* pClientConn = this->GetClientConnByFd(nClientFd);
    if (NULL == pClientConn) {
        CDSMLog::Error("Client[%d] call DoAuth() failed!",nClientFd);
        return;
    }
    pClientConn->SetLocationInfo(latitude,longitude,height);
}
