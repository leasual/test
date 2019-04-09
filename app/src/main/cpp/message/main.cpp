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

	CConfigFileReader::GetInstance()->LoadFromFile("dsm_jtt808.cfg");
    char* strSimNo = CConfigFileReader::GetInstance()->GetConfigName("sim_no");
    char* strDevModel = CConfigFileReader::GetInstance()->GetConfigName("dev_model");
    char* strIp = CConfigFileReader::GetInstance()->GetConfigName("server_ip");
    char* strPort = CConfigFileReader::GetInstance()->GetConfigName("server_port");

	if (!CDsmJTT808_API::GetInstance()->Inialise(strSimNo,strDevModel,strIp, strPort)) {
		UT_FATAL("Inialise failed!");
		return -1;
	}

//	std::vector<std::string> v;
//	v.push_back("hu");
//	v.push_back("wen");

    std::vector<AlarmAccessory> vAccessories;

	AlarmAccessory objAccess;
	objAccess.stFileType = euPIC;
	strcpy(objAccess.stFileName,"/home/public/Work/image/alarm1.jpg");
	vAccessories.emplace_back(objAccess);

    AlarmAccessory objAccess2;
    objAccess2.stFileType = euPIC;
    strcpy(objAccess2.stFileName,"/home/public/Work/image/alarm2.jpg");
    vAccessories.emplace_back(objAccess2);

	CDsmJTT808_API::GetInstance()->SetGpsInfo(45890000,23480000,20,100,true,euFatigue,DSM_ALARM_FLAG,vAccessories);

	//DevLocInfo* pDevLoc = nullptr;
	//CClientConnManager::GetInstance()->GetLocation(pDevLoc);
	//UT_TRACE("item = %zu",objDevLoc.stAccessories.size());
	//CClientConnManager::GetInstance()->m_queueDevLoc.PopFront(&pDevLoc);
	//UT_TRACE("item = %zu",(pDevLoc)->stAccessories.size());
	//return 1;

	if (!CDsmJTT808_API::GetInstance()->Connect(strSimNo,strDevModel)) {
		UT_FATAL("Connect failed!");
		return -1;
	}


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
//			CClientConnManager::GetInstance()->DeleteClientConnect(nClientFd);
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
