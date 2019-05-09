//
// Created by public on 19-3-27.
//

#ifndef DSM_JTT808_DSM_JTT808_API_H
#define DSM_JTT808_DSM_JTT808_API_H


#include "base/config_file.h"
#include "client_conn_manager.h"
#include "singleton.h"


class CDsmJTT808_API :
        public Singleton<CDsmJTT808_API>
{
public:
    //CDsmJTT808_API() {m_listener = NULL; m_client = NULL;}
    CDsmJTT808_API() {m_bInialised = false;}
    ~CDsmJTT808_API() {}

//    bool Inialise(char* szSimNo, char* szDevModel, char* szIp, int nPort)
//    {
//        if (NULL == szSimNo || NULL == szDevModel || NULL == szIp)
//            return false;
//
//        CDSMLog::GetInstance()->InitialiseLog4z("./dsm_log.cfg");
//        UT_TRACE("Server IP[%s] Port[%d]",szIp,nPort);
//
//        // 连接Server
//        m_strIp = szIp;
//        m_nPort = nPort;
//        m_strSimNo = szSimNo;
//        m_strDevModel = szDevModel;
//
//        CClientConnManager::GetInstance()->InialiseConnect(m_strIp,m_nPort,m_strSimNo,m_strDevModel);
//        m_bInialised = true;
//        return true;
//    }
//
//    bool Connect()
//    {
//        if (!m_bInialised)
//            return false;
//
//        m_nClientFd = CClientConnManager::GetInstance()->Connect();
//        if (m_nClientFd != -1) {
//            UT_TRACE("Connect to server IP[%s] Port[%d] success!",m_strIp.c_str(),m_nPort);
//            return true;
//        } else {
//            UT_TRACE("Connect to server IP[%s] Port[%d] failed!",m_strIp.c_str(),m_nPort);
//        }
//        return false;
//    }

    void SetGpsInfo(uint64_t latitude,uint64_t longitude,uint32_t  height,WORD speed,WORD stDirection,bool bAlarm)
    {
        UT_INFO("Set Gps information.");
        std::shared_ptr<UploadGPSInfo> spGpsInfo =
                std::make_shared<UploadGPSInfo>(latitude,longitude,height,speed,stDirection,bAlarm,0);
        CClientConnManager::GetInstance()->SetLocation(spGpsInfo);
    }

    void SetGPSAlarmInfo(UploadGPSInfo& gpsInfo,std::shared_ptr<UploadADASAlarmInfo> adasAlarmInfo,
                         std::vector<AlarmAccessory>& accessories)
    {
        DevUploadGPSAlarmInfo * gpsAlarmInfo(nullptr);
        gpsAlarmInfo = new DevUploadGPSAlarmInfo(gpsInfo,adasAlarmInfo,accessories);
        CClientConnManager::GetInstance()->SetAlarmInfo(gpsAlarmInfo);
    }


    void SetGPSAlarmInfo(UploadGPSInfo& gpsInfo,std::shared_ptr<UploadDSMAlarmInfo> dsmAlarmInfo,
                         std::vector<AlarmAccessory>& accessories)
    {
        DevUploadGPSAlarmInfo * gpsAlarmInfo(nullptr);
        gpsAlarmInfo = new DevUploadGPSAlarmInfo(gpsInfo,dsmAlarmInfo,accessories);
        CClientConnManager::GetInstance()->SetAlarmInfo(gpsAlarmInfo);
    }

    void Start(char* szSimNo, char* szDevModel, char* szIp, int nPort)
    {
        if (NULL == szSimNo || NULL == szDevModel || NULL == szIp) {
            UT_FATAL("Function parameter is NULL");
            return;
        }

        CDSMLog::GetInstance()->InitialiseLog4z("./dsm_log.cfg");
        UT_TRACE("Server IP[%s] Port[%d]",szIp,nPort);

        // 连接Server
        m_strIp = szIp;
        m_nPort = nPort;
        m_strSimNo = szSimNo;
        m_strDevModel = szDevModel;

        m_nClientFd = CClientConnManager::GetInstance()->StartNewConnect(m_strIp,m_nPort,m_strSimNo,m_strDevModel);
        //CClientConnManager::GetInstance()->Inialise(m_strIp,m_nPort,m_strSimNo,m_strDevModel);
    }

    void StartTimer()
    {

    }

    void StopTimer()
    {

    }

    void OnTimer()
    {
        uint64_t curr_tick = CUtil::GetInstance()->get_tick_count();
        //CClientConnManager::GetInstance()->OnTimer(curr_tick);
        //DWORD millSecond = 100;

//        if (CClientConnManager::GetInstance()->GetClientConnStatus(m_nClientFd) == NET_DISCONNECTED) {
//            // 重新连接
//            UT_TRACE("Start reconnect...");
//            m_nClientFd = CClientConnManager::GetInstance()->ClientReconnect(m_nClientFd,m_strSimNo,m_strDevModel,m_strIp,m_nPort);
//        }
//
//        enNetStatus euStatus = CClientConnManager::GetInstance()->GetClientConnStatus(m_nClientFd);
//        if (euStatus == NET_CONNECTED) {
//            // 已经建立连接成功了,但是还没有注册成功,发起注册
//            CClientConnManager::GetInstance()->DoRegister(m_nClientFd);
//        }
//
//        if (euStatus >= NET_REGISTED && euStatus != NET_AUTHENTICATED) {
//            // 已经注册成功了,但是还没有监权成功,发起监权
//            CClientConnManager::GetInstance()->DoAuth(m_nClientFd);
//        }
//        usleep(millSecond * 1000); // 让出millSecond毫秒的时间片
    }


    void UnInialise()
    {
        //DestroyHPSocketObjects();
        m_nClientFd = -1;
    }

//    int  ClientSend_API(const BYTE* buff, size_t nBuffLen)
//    {
//        return ClientSend(m_client,buff,nBuffLen);
//    }
//
//    int StopTcpClient_API()
//    {
//        return StopTcpClient(m_client);
//    }
//
//    int StartTcpClient_API(const char *svr_ip, unsigned short int port)
//    {
//       return StartTcpClient(m_client,svr_ip,port);
//    }

private:
    unsigned int m_nClientFd;
    std::string m_strIp;
    unsigned int m_nPort;
    std::string m_strSimNo;
    std::string m_strDevModel;
    bool m_bInialised;
};



#endif //DSM_JTT808_DSM_JTT808_API_H
