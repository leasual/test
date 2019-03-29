//
// Created by public on 19-3-19.
//

#ifndef DSM_JTT808_MSG_PROCESS_H
#define DSM_JTT808_MSG_PROCESS_H

#include "base/public_def.h"
#include "base/singleton.h"
#include "base/jtt808_base.h"
#include "client_conn.h"

#define DATA_LEN   512


class CMsgProcess :
		public  JTT808PktBase,
		public  Singleton<CMsgProcess>
{
public:
    CMsgProcess();
    ~CMsgProcess();
    void ProcessMsg(CClientConn* pClientConn,BYTE* buf, size_t nLen);

    int DevRegister(void);
    int DevHeartBeat(void); //终端心跳
    int DevUnregister(void); //终端注销
	int DevAuthentication(); // 终端监权
	int DevGetParameterResp(WORD serialID,BYTE argNum,STR_PARAMETER *dev_arg);
	int DevGetAttriResp(STR_DEV_ATTR attr);
	int DevLocationUp(uint64_t latitude,uint64_t longitude,uint32_t  height); // 位置上报
	int DevCommResp(WORD serialID,WORD answerID,BYTE result);  // 终端通用应答

private:
	void _ConstructDevRegisterPkt();
	void _ConstructDevHeartBeatPkt();
	void _ConstructDevAuthPkt(const char* authentication_code);
	void _ConstructLocInfoPkt(uint64_t latitude,uint64_t longitude,uint32_t  height); //构造位置基本信息汇报报文
	void _ConstructPktHeader(WORD nCmd, WORD nBodyLen);
	int _ProcessGeneralResp(CClientConn* pClientConn,BYTE *pGeneralResp, int nLen);  // 处理通用应答的返回结果
	int _ProcessRegisterResp(BYTE *pRegResp, int nLen);  //处理注册回应的消息
	int _ProcessPltAccessoriesResp(BYTE* pResp, int nLen);

    int _SendToPlt();

private:
    BYTE m_sendBuffer[DATA_LEN];
    BYTE m_respBuffer[DATA_LEN*2];
	char m_szAuthCode[32];
};

#endif //DSM_JTT808_MSG_PROCESS_H
