//
// Created by public on 19-3-25.
//

#ifndef DSM_JTT808_CLIENT_CONN_MANAGER_H
#define DSM_JTT808_CLIENT_CONN_MANAGER_H

#include <map>

#include "base/public_def.h"
#include "base/singleton.h"
#include "client_conn.h"


class CClientConnManager :
        public Singleton<CClientConnManager>
{
public:
    CClientConnManager();
    ~CClientConnManager(){};

    CClientConn* GetClientConnByFd(unsigned int nClientFd);
    bool UpdateConnStatus(unsigned int nClientFd,enNetStatus euStatus); // 更新连接的状态
    void ReceivePkt(unsigned int nClientFd,BYTE* buf, size_t nLen);
    enNetStatus GetClientConnStatus(unsigned int nClientFd); // 取客户端连接的状态
    //void Connect(unsigned int nClientFd);
    bool DoRegister(unsigned int nClientFd);
    bool DoAuth(unsigned int nClientFd);
    //bool DoLocationUp(unsigned int nClientFd);
    void SetLocation(unsigned int nClientFd,uint64_t latitude,uint64_t longitude,uint32_t  height);
    int Inialise(std::string strIp, unsigned int nPort);
    int Inialise(unsigned int nClientFd,std::string strIp, unsigned int nPort);
    void OnTimer(uint64_t curr_tick);
    int ClientReconnect(unsigned int nClientFd,std::string strIp, unsigned int nPort); // 客户端重新连接

private:
    void _RegisterClientConn(unsigned int nClientFd, CClientConn* pClientConn);
    bool _DeleteClientConnect(unsigned int nClientFd);  // 删除客户端连接对象

private:
    std::map<unsigned int, CClientConn*> m_mapClients;
    bool m_bInialised;

};

#endif //DSM_JTT808_CLIENT_CONN_MANAGER_H
