//
// Created by public on 19-3-25.
//

#ifndef DSM_JTT808_CLIENT_CONN_MANAGER_H
#define DSM_JTT808_CLIENT_CONN_MANAGER_H

#include <map>
#include <list>
#include <queue>
#include <mutex>
#include <thread>

#include "base/public_def.h"
#include "base/singleton.h"
#include "client_conn.h"
#include "base/ut_queue.h"


class CClientConnManager :
        public Singleton<CClientConnManager>
{
public:
    CClientConnManager();
    ~CClientConnManager();

	CClientConn* InialiseConnect(const std::string &strServerIp, unsigned int nPort, const std::string &strSimNo,
						 const std::string &strDevMode,euConnType connType = euConnectPlt);
	int Connect(CClientConn* pClientConn = NULL);
	int StartNewConnect(const std::string &strServerIp, unsigned int nPort, const std::string &strSimNo,
						const std::string &strDevMode,euConnType connType = euConnectPlt);
    CClientConn* GetClientConnByFd(unsigned int nClientFd);
    bool UpdateConnStatus(unsigned int nClientFd,enNetStatus euStatus); // 更新连接的状态
    void ReceivePkt(unsigned int nClientFd,BYTE* buf, size_t nLen);
    enNetStatus GetClientConnStatus(unsigned int nClientFd); // 取客户端连接的状态
    bool DoRegister(unsigned int nClientFd);
    bool DoAuth(unsigned int nClientFd);
    void SetLocation(std::shared_ptr<UploadGPSInfo>& spGpsInfo);
    //void OnTimer(uint64_t curr_tick);

    int  ClientReconnect(unsigned int nClientFd); // 客户端重新连接
	const euConnType  GetClientConnType(unsigned int nClientFd); // 取客户端连接的类型
	void UpdateAlarmFlag(const std::string& strAlarmFlag,std::vector<AlarmAccessory>& refAccessories);
	bool GetAlarmAccessory(const std::string& strAlarmFlag, std::vector<AlarmAccessory> &refAccessories);
	bool DelAlarmFlag(std::string strAlarmFlag);
	bool ReleaseClientConnect(int nClientFd);  // 释放客户端连接对象
    bool DeleteClientConnect(int nClientFd);  // 删除客户端连接对象
	void SetAlarmInfo(DevUploadGPSAlarmInfo* spAlarmInfo);
	void AddCheckConnectTimer();
	void RemoveCheckConnectTimer();
	void AddGpsUpLoadTimer();
	void RemoveGpsUpLoadTimer();
	void AddAlarmUpLoadTimer();
	void RemoveAlarmUpLoadTimer();
	void CacheAlarmUpLoadInfo(WORD msgId, WORD seqNo,DevUploadGPSAlarmInfo**);  // 缓存报警信息　
	BOOL RemoveAlarmUpLoadInfo(WORD msgId, WORD seqNo,DevUploadGPSAlarmInfo**);

	BOOL CheckPltConnectStatus(); // check 连接平台的状态
	UTQueue<UploadGPSInfo>  m_queueGps;  // 位置信息队列
	UTQueue<DevUploadGPSAlarmInfo>  m_queueAlarm;  // 报警信息队列

private:
    void _CheckConnected(uint64_t tm = 0);
	void _DoDevLocUp(uint64_t);
	void _DoAlarmUp(uint64_t);
	
private:
    std::map<int, CClientConn*> m_mapClients;
    typedef std::map<int, CClientConn*>::iterator ITER_CLIENT_CONNS;
	std::map<std::string,std::vector<AlarmAccessory> > m_mapAccessories; /*key == alarmFlag*/
	std::function<void(uint64_t)> gps_timer_callBack;
	std::function<void(uint64_t)> alarm_timer_callBack;
	std::function<void(uint64_t)> check_connect_timer_callBack;
	bool m_bInialsed;

	std::mutex m_cache_alarmInfo_lock;
	std::map<DWORD,DevUploadGPSAlarmInfo*> m_mapUpAlarmInfo;  // 上报报警信息的缓存,收到对应的回应消息后，从缓存中删除．
	typedef std::map<DWORD,DevUploadGPSAlarmInfo*>::iterator ITER_UP_ALARM;

};

#endif //DSM_JTT808_CLIENT_CONN_MANAGER_H
