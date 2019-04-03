#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "api/dsm_jtt808_api.h"

#include "msg_process.h"  // should be delete
#include "client_conn.h"

int main(int argc, char* const argv[])
{
	if (!CDsmJTT808_API::GetInstance()->Inialise()) {
		UT_FATAL("Inialise failed!");
		return -1;
	}
	CDsmJTT808_API::GetInstance()->SetGpsInfo(45890000,23480000,100);



//	CMsgProcess::GetInstance()->_ConstructLocInfoPkt(0,0,0);
//	return 1;

	while(1) {
		CDsmJTT808_API::GetInstance()->OnTimer();
	}

	//CDsmJTT808_API::GetInstance()->UnInialise();

	return  1;
}

//int main(int argc, char* const argv[])
//{
//	HPSocketHelper::CreateHPSocketObjects();
//	CConfigFileReader::GetInstance()->LoadFromFile("server.conf");
//	char* szServerIp = CConfigFileReader::GetInstance()->GetConfigName("server_ip");
//	char* szServerPort = CConfigFileReader::GetInstance()->GetConfigName("server_port");
//	CDSMLog::GetInstance()->InitialiseLog4z("./dsm_log.cfg");
//	CDSMLog::Trace("Server IP[%s] Port[%s]",szServerIp,szServerPort);
//
//	//CMsgProcess::GetInstance()->DevRegister();
//	//return 1;
//    CClientConn objClientConn;
//	// 初始化socket
//	HPSocketHelper::CreateHPSocketObjects();
//
//	// 连接Server
//	objClientConn.Inialise(szServerIp,atoi(szServerPort));
//	unsigned int nClientFd = objClientConn.Connect();
//	if (nClientFd != -1)
//		CClientConnManager::GetInstance()->RegisterClientConn(nClientFd,&objClientConn);
//
//	DWORD millSecond = 1000;
//	while(1) {
//		uint64_t curr_tick = CUtil::GetInstance()->get_tick_count();
//		objClientConn.OnTimer(curr_tick);
//
//		if (CClientConnManager::GetInstance()->GetClientConnStatus(nClientFd) == NET_DISCONNECTED) {
//			// 重新连接
//			CDSMLog::Trace("Start reconnect...");
//			CClientConn* pClientConn = CClientConnManager::GetInstance()->GetClientConnByFd(nClientFd);
//			CClientConnManager::GetInstance()->_DeleteClientConnect(nClientFd);
//			nClientFd = pClientConn->Connect();
//			CClientConnManager::GetInstance()->RegisterClientConn(nClientFd,pClientConn);
//			continue;
//		}
//
//		enNetStatus euStatus = CClientConnManager::GetInstance()->GetClientConnStatus(nClientFd);
//		if (euStatus == NET_CONNECTED) {
//			// 已经建立连接成功了,但是还没有注册成功,发起注册
//			CClientConnManager::GetInstance()->DoRegister(nClientFd);
//		}
//
//		if (euStatus >= NET_REGISTED && euStatus != NET_AUTHENTICATED) {
//			// 已经注册成功了,但是还没有监权成功,发起监权
//			CClientConnManager::GetInstance()->DoAuth(nClientFd);
//		}
//
//		usleep(millSecond * 1000); // 让出millSecond毫秒的时间片
//	}
//
//	HPSocketHelper::DestroyHPSocketObjects();
//
//    return 0;//EXIT_CODE_OK;
//}
