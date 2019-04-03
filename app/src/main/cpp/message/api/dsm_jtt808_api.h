//
// Created by public on 19-3-27.
//

#ifndef DSM_JTT808_DSM_JTT808_API_H
#define DSM_JTT808_DSM_JTT808_API_H


#include "base/config_file.h"
#include "hp_socket_helper.h"
#include "client_conn_manager.h"
#include "singleton.h"


class CDsmJTT808_API :
        public Singleton<CDsmJTT808_API>
{
public:
    CDsmJTT808_API() {m_listener = NULL; m_client = NULL;}
    ~CDsmJTT808_API() {}

    bool Inialise()
    {
        CConfigFileReader::GetInstance()->LoadFromFile("/sdcard/Android/data/com.ut.sdk/files/dsm_jtt808.cfg");
//        char* szServerIp = CConfigFileReader::GetInstance()->GetConfigName("server_ip");
//        char* szServerPort = CConfigFileReader::GetInstance()->GetConfigName("server_port");
        CDSMLog::GetInstance()->InitialiseLog4z("/sdcard/Android/data/com.ut.sdk/files/dsm_log.cfg");
//        UT_TRACE("Server IP[%s] Port[%s]",szServerIp,szServerPort);

        // 连接Server
        m_strIp = "106.14.186.44";
        m_nPort = 7000;

        m_nClientFd = CClientConnManager::GetInstance()->Connect(m_strIp,m_nPort,true);
		//m_nClientFd = CClientConnManager::GetInstance()->Connect("106.14.186.44",13880);
        if (m_nClientFd != -1) {
//            UT_TRACE("Connect to server IP[%s] Port[%s] success!",szServerIp,szServerPort);
            return true;
        }else{
//            UT_TRACE("Connect to server IP[%s] Port[%s] failed!",szServerIp,szServerPort);
        }

        return false;
    }

    void OnTimer()
    {
        uint64_t curr_tick = CUtil::GetInstance()->get_tick_count();
        CClientConnManager::GetInstance()->OnTimer(curr_tick);
        DWORD millSecond = 1000;

        if (CClientConnManager::GetInstance()->GetClientConnStatus(m_nClientFd) == NET_DISCONNECTED) {
            // 重新连接
            UT_TRACE("Start reconnect...");
            m_nClientFd = CClientConnManager::GetInstance()->ClientReconnect(m_nClientFd,(char*)m_strIp.c_str(),m_nPort);
        }

        enNetStatus euStatus = CClientConnManager::GetInstance()->GetClientConnStatus(m_nClientFd);
        if (euStatus == NET_CONNECTED) {
            // 已经建立连接成功了,但是还没有注册成功,发起注册
            CClientConnManager::GetInstance()->DoRegister(m_nClientFd);
        }

        if (euStatus >= NET_REGISTED && euStatus != NET_AUTHENTICATED) {
            // 已经注册成功了,但是还没有监权成功,发起监权
            CClientConnManager::GetInstance()->DoAuth(m_nClientFd);
        }

        usleep(millSecond * 1000); // 让出millSecond毫秒的时间片
    }

    void SetGpsInfo(uint64_t latitude,uint64_t longitude,uint32_t  height/*,const  char* szFileName,euFileType fileType*/)
    {
        CClientConnManager::GetInstance()->SetLocation(m_nClientFd,latitude,longitude,height);
    }

    void UnInialise()
    {
        DestroyHPSocketObjects();
        m_nClientFd = -1;
        m_listener = NULL;
        m_client = NULL;
    }

    int  ClientSend_API(const BYTE* buff, size_t nBuffLen)
    {
        return ClientSend(m_client,buff,nBuffLen);
    }

    int StopTcpClient_API()
    {
        return StopTcpClient(m_client);
    }

    int StartTcpClient_API(const char *svr_ip, unsigned short int port)
    {
       return StartTcpClient(m_client,svr_ip,port);
    }

private:
    unsigned int m_nClientFd;
    std::string m_strIp;
    unsigned int m_nPort;

    HP_TcpPullClientListener m_listener;
    HP_TcpPullClient m_client;
};



#endif //DSM_JTT808_DSM_JTT808_API_H
