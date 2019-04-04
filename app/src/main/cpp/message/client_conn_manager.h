//
// Created by public on 19-3-25.
//

#ifndef DSM_JTT808_CLIENT_CONN_MANAGER_H
#define DSM_JTT808_CLIENT_CONN_MANAGER_H

#include <map>
#include <list>
#include <queue>
#include <mutex>

#include "base/public_def.h"
#include "base/singleton.h"
#include "client_conn.h"
#include "util.h"


class CClientConnManager :
        public Singleton<CClientConnManager>
{
public:
    CClientConnManager();
    ~CClientConnManager(){};

	int Connect(std::string strIp, unsigned int nPort,bool bTimer = false);
    CClientConn* GetClientConnByFd(unsigned int nClientFd);
    bool UpdateConnStatus(unsigned int nClientFd,enNetStatus euStatus); // 更新连接的状态
    void ReceivePkt(unsigned int nClientFd,BYTE* buf, size_t nLen);
    enNetStatus GetClientConnStatus(unsigned int nClientFd); // 取客户端连接的状态
    bool DoRegister(unsigned int nClientFd);
    bool DoAuth(unsigned int nClientFd);
    void SetLocation(DevLocInfo& pDevLocInfo);
    void SetLocation(unsigned int nClientFd,uint64_t latitude,uint64_t longitude,uint32_t height,WORD speed, bool bAlarm,
                     euAlarmType alarmType,std::vector<AlarmAccessory>& accessories);
    //bool GetLocation(DevLocInfo* pDevLocInfo);
    void OnTimer(uint64_t curr_tick);
    int  ClientReconnect(unsigned int nClientFd,std::string strIp, unsigned int nPort); // 客户端重新连接
	void UpdateAlarmFlag(std::string strAlarmFlag,std::vector<AlarmAccessory>& refAccessories);
	bool GetAlarmFlag(std::string strAlarmFlag,std::vector<AlarmAccessory>& refAccessories);
	bool DelAlarmFlag(std::string strAlarmFlag);
    CCASQueueX<DevLocInfo>  m_queueDevLoc;  // 位置信息队列

private:
    void _RegisterClientConn(unsigned int nClientFd, CClientConn* pClientConn);
    bool _DeleteClientConnect(unsigned int nClientFd);  // 删除客户端连接对象

private:
    std::map<unsigned int, CClientConn*> m_mapClients;
    //std::list<DevLocInfo> m_listAlarm
	std::map<std::string,std::vector<AlarmAccessory> > m_mapAccessories;
};

#endif //DSM_JTT808_CLIENT_CONN_MANAGER_H
