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

    int DevRegister(CClientConn* pClientConn);
    int DevHeartBeat(CClientConn* pClientConn); //终端心跳
    int DevUnregister(CClientConn* pClientConn); //终端注销
	int DevAuthentication(CClientConn* pClientConn); // 终端监权
	int DevGetParameterResp(WORD serialID,BYTE argNum,STR_PARAMETER *dev_arg);
	int DevGetAttriResp(STR_DEV_ATTR attr);
	int DevLocationUp(CClientConn* pClientConn); // 位置上报
	int DevCommResp(CClientConn* pClientConn,WORD serialID,WORD answerID,BYTE result);  // 终端通用应答
	int DevAlarmAccessoryUp(CClientConn*,BYTE*, BYTE*); // 报警附件上传指令
	int DevFileUpload(CClientConn*,FileInfo*); // 终端文件上传指令

private:
	void _ConstructDevRegisterPkt();
	void _ConstructDevHeartBeatPkt();
	void _ConstructDevAuthPkt(const char* authentication_code);
    void _ConstructLocInfoPkt(DevLocInfo* pDevLocInfo); //构造位置基本信息汇报报文
	void _ConstructPktHeader(WORD nCmd, WORD nBodyLen); // 构造指令头
	int _ProcessGeneralResp(CClientConn* pClientConn,BYTE *pGeneralResp, int nLen);  // 处理通用应答的返回结果
	int _ProcessRegisterResp(BYTE *pRegResp, int nLen);  //处理注册回应的消息
	int _ProcessPltAccessoryReq(CClientConn* pClientConn,BYTE* pResp, int nLen); // 平台请求报警附件上传
	int _ProcessPltAccessoryUpResp(CClientConn* pClientConn); // 报警附件信息上传的回应指令
	int _ProcessPltUpFileOk(CClientConn* pClientConn,BYTE* pResp, int nLen);  // 文件上传完成消息应答

    int _SendToPlt(CClientConn* pClientConn);

private:
    BYTE m_sendBuffer[DATA_LEN];
    BYTE m_respBuffer[DATA_LEN*2];
	char m_szAuthCode[32];
	FileInfo* m_pCurrentUpLoading; // 记录当前正在上传的文件
	std::string m_strSimNo;
};

#endif //DSM_JTT808_MSG_PROCESS_H
