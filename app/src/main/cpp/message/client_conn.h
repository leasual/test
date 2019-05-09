//
// Created by public on 19-3-19.
//

#ifndef DSM_JTT808_HP_TCP_COMM_H
#define DSM_JTT808_HP_TCP_COMM_H


#include "hpsocket/HPSocket4C.h"
#include "hpsocket/SocketInterface.h"

#include "base/public_def.h"
#include "base/singleton.h"
#include "base/util.h"
#include "base/msg_def.h"
#include "file_helper.h"
#include <string>


class CClientConn
{
public:
    CClientConn();
    ~CClientConn();

    int Connect();
    bool Inialise(const std::string& strServerIp, unsigned int nPort,const std::string& strSimNo, const std::string& strDevMode,const euConnType& connType = euConnectPlt);
    void ReleaseHPSocket();
	int  ClientSend(const BYTE* buff, size_t nBuffLen);
    //void OnTimer(uint64_t curr_tick);
    void UpdateConnStatus(enNetStatus status);  // 更新連接狀態
    enNetStatus GetConnStatus();  // 取当前socket的连接状态
    void UpdateKeepAliveTick(uint64_t keepAliveTm);  // 更新保活时间
    void UpdateRecvPktTick(uint64_t recvPktTick);  // 更新最近一次接收报文的时间
    void ProcessMsg(BYTE* buf, size_t nLen);
    void DoRegister();
    void DoAuth();

    void DoLocationUp(); // 位置信息上报
    void DoAlarmInfoUp(); // 报警信息上报
    void ResetLocation(); // 重置GPS的值
    bool IsLocationSet(); // GPS是否有值
    unsigned int GetClientHandle() {return m_nClientFd;}
	int UpdateConnectFd(int nClientFd);

    void UpdateUpFileInfo(const std::string& strFileName, const FileInfo& fileInfo);
    const std::map<std::string,FileInfo>& GetUpFileInfo(){ return  m_mapFileInfo;};
    bool UpdateUpFileStatus(const std::string& strFileName,const euFileUpStatus& st);
    void ClearUpFileInfo();
    void PrintFileStatus(); // 输出文件的状态信息
    FileInfo* GetUpFileInfo(const std::string &strFileName);
    bool DelFileInfo(const std::string &strFileName);
    const char* GetSimNo();
    const char* GetDevModel();
    void SetAlarmFlag(const std::string& refAlarmFalg);
    const std::string& GetAlarmFlag(){return  m_strAlarmFlag;};
    const euConnType& GetConnType() { return m_euConnType;};
    const std::string& GetServerIp() const { return m_strServerIp;}
    const int GetServerPort() const { return m_nServerPort;}
    void Reset();
    void AddKeepAliveTimer();
    void RemoveKeepAliveTimer();

	std::string m_strAccessSvrIp;   //  附件服务器的iP地址
	int m_nAccessSvrPort; // 附件服务器的端口号
	BYTE m_btAlarmFlag[16]; // 报警标识号,用于附件服务器重新连接时使用.
	BYTE m_btAlarmId[32]; // 报警编号,用于附件服务器重新连接时使用.

private:
	void _DoHeartBeat(uint64_t curr_tick);
    void _CheckServerTimeout(uint64_t curr_tick);

private:
	// Callback
	static En_HP_HandleResult __HP_CALL OnConnect(HP_Client pSender, HP_CONNID dwConnID);
	static En_HP_HandleResult __HP_CALL OnReceive(HP_Client pSender, HP_CONNID dwConnID, int iLength);
	static En_HP_HandleResult __HP_CALL OnSend(HP_Client pSender, HP_CONNID dwConnID, const BYTE* pData, int iLength);
	static En_HP_HandleResult __HP_CALL OnClose(HP_Client pSender, HP_CONNID dwConnID, En_HP_SocketOperation enOperation, int iErrorCode);

private:
	HP_TcpPullClientListener m_listener;
	HP_TcpPullClient  m_client;

    std::string m_strServerIp;   // 服务器端的IP地址
    unsigned  int m_nServerPort;  // 服务器端的端口号

    uint64_t m_last_keepAlive_send_tick;  // 最近发送保活报文的时间
    uint64_t m_last_recv_tick; // 最近接收报文的时间
    uint64_t m_last_send_loc_tick; // 最近一次上报位置信息的时间

	unsigned int m_nClientFd; // 客户端和服务器端的通信句柄
    std::atomic<enNetStatus> m_conn_status;  // 當前網絡連接狀態
	euConnType m_euConnType;  // 连接类型
    bool m_bTimer;  // 是否启动timer
    // gps 信息
    uint64_t  m_latitude;
    uint64_t  m_longitude;
    uint32_t  m_height;

    std::string m_strSimNo;
    std::string m_strDevModel;
    std::string m_strAlarmFlag; // 报警标识

    std::map<std::string,FileInfo> m_mapFileInfo;  // 需要上传的文件信息容器,key=平台文件名, value=文件信息
	HpktInfo m_pkgInfo;

	bool m_bInialised;

    std::function<void(uint64_t)> keepAlive_tiemr_callBack;
    std::function<void(uint64_t)> server_timeout_callBack;
};


#endif //DSM_JTT808_HP_TCP_COMM_H
