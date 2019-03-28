//
// Created by public on 19-3-19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base/msg_def.h"

#include "msg_process.h"


CMsgProcess::CMsgProcess()
{
    memset(m_respBuffer,0,sizeof(m_respBuffer));
    memset(m_sendBuffer,0,sizeof(m_sendBuffer));
	memset(m_szAuthCode,0,sizeof(m_szAuthCode));
}


CMsgProcess::~CMsgProcess()
{

}


//void CMsgProcess::ResetLocation()
//{
//    m_latitude = 0;
//    m_longitude = 0;
//    m_heigh = 0;
//}
//
//bool CMsgProcess::IsLocationSet()
//{
//    if (m_longitude != 0 && m_latitude != 0 && m_heigh != 0)
//        return true;
//
//    return false;
//}

//
// 处理注册回应的消息
//
int CMsgProcess::_ProcessRegisterResp(BYTE *pRegResp, int nLen)
{
	//81 00 00 05 01 78 87 11 33 01 00 00
	//e1 3d 00 31 30 84
	if (NULL == pRegResp || nLen < 0) {
        CDSMLog::Error("Parameter error!");
        return -1;
    }

    JTT808MsgHead* pMsgHeader = (JTT808MsgHead*)pRegResp;
    if (nLen <= pMsgHeader->HEADERSIZE) {
        // 非法包
        CDSMLog::Error("Package is less than header!");
        return -1;
    }

	BYTE* pTemp = pRegResp + pMsgHeader->HEADERSIZE;
	size_t nBodyLen = pMsgHeader->GetMsgBodyLength();
    //CDSMLog::Trace("The response msgLen[%x]",nBodyLen);
    bool bRegResult = false;  // 注册结果
    pTemp += 2; // 跳过应答流水号
	switch(*pTemp) // 解析注册回应结果
	{
		case REGISTER_OK:   //!<  成功
			CDSMLog::Info("注册返回: 注册成功");
            bRegResult = true;
			break;

		case REGISTER_CAR_REGISTERED:    //!< 车辆已被注册
			CDSMLog::Info("注册返回: 车辆已被注册");
			break;
					
		case REGISTER_HAVE_NO_CAR:       //!< 数据库中无该车辆
			CDSMLog::Info("注册返回: 数据库中无该车辆");
			break;
					
		case REGISTER_DEV_REGISTERED:   //!< 终端已被注册
			CDSMLog::Info("注册返回: 终端已被注册");
			break;
					
		case REGISTER_HAVE_NO_DEV:	    //!< 数据库中无该终端
			CDSMLog::Info("注册返回: 数据库中无该终端");
			break;

        default:
		    CDSMLog::Error("注册返回: Unknown response code[%d]",*pTemp);
			break;
	}
    pTemp++; // 跳过应答的结果

    if (bRegResult)  { // 注册成功
        memcpy(m_szAuthCode,pTemp,nBodyLen-3);
        CDSMLog::dsm_dump((unsigned char*)m_szAuthCode,strlen(m_szAuthCode));
        CConfigFileReader::GetInstance()->SetConfigValue("auth_code",(const char*)m_szAuthCode); //保存到配置文件
        return REGISTER_OK;
    }

	return -1;
}


///  处理通用应答的返回结果
/// \param pGeneralResp
/// \param nLen
/// \return
int CMsgProcess::_ProcessGeneralResp(CClientConn* pClientConn,BYTE *pGeneralResp, int nLen)
{
    //跳过指令头,解析对应应答ID的返回结果
    //80 01 00 05 01 78 87 11 33 01 00 0f
    // 6a dc 00 02 00 e2
    if (NULL == pGeneralResp || nLen < 0) {
        CDSMLog::Error("Parameter error!");
        return -1;
    }

    JTT808MsgHead* pMsgHeader = (JTT808MsgHead*)pGeneralResp;
    if (nLen <= pMsgHeader->HEADERSIZE) {
        // 非法包
        CDSMLog::Error("Package is less than header!");
        return -1;
    }

    //80 01 00 05 01 78 87 11 33 01 00 10
    // 57 af 00 02 00 b3
    BYTE* pTemp = pGeneralResp + pMsgHeader->HEADERSIZE;
    //CDSMLog::Trace("The response msgLen[%x]",pMsgHeader->GetMsgBodyLength());

    pTemp += 2; // 跳过应答流水号
    WORD cmd = (*(pTemp) <<8) + *(pTemp + 1);
    pTemp += 2; // 跳过应答的消息ID

    switch(*pTemp)
    {
        case RET_OK: //0
            CDSMLog::Info("Response result[%s] for MsgID[%x]","成功",cmd);
            if (cmd == JTT_DEV_AUTH) {
                // 收到注册的正确回应
                pClientConn->UpdateConnStatus(NET_AUTHENTICATED); // 更新状态
            }

            if (cmd == JTT_DEV_LOC_REP) {
                pClientConn->ResetLocation();
            }
            break;

        case RET_FAILED:
            CDSMLog::Info("Response result[%s] for MsgID[%x]","失败",cmd);
            break;

        case RET_MESSAGE_ERROR:
            CDSMLog::Info("Response result[%s] for MsgID[%x]","消息有误",cmd);
            break;

        case RET_NOT_SUPPORT:
            CDSMLog::Info("Response result[%s] for MsgID[%x]","不支持",cmd);
            break;

        case RET_ALARM:
            CDSMLog::Info("Response result[%s] for MsgID[%x]","报警处理确认",cmd);
            break;

        case RET_PARAMETER_ERROR:
            CDSMLog::Info("Response result[%s] for MsgID[%x]","参数错误",cmd);
            break;

        default:
            CDSMLog::Error("Unknown response result[%x] for MsgId[%x]",*pTemp,cmd);
            break;
    }

    return 1;
}

void CMsgProcess::ProcessMsg(CClientConn* pClientConn,BYTE* data, size_t nLen)
{
    if (NULL == pClientConn || NULL == data || nLen <= 0) {
        CDSMLog::Error("Invalidate parameter!");
        return;
    }

    if (!this->ReceiveData(data,nLen)) {
        CDSMLog::Error("Recive data failed!");
        return;
    }

    //81 00 00 05 01 78 87 11 33 01 00 00 5f df 00 31 30 d8
    WORD cmd;
    while(GetOnePacket(&cmd))
    {
        switch(cmd) {
            case JTT_PLAT_REGISTER_ACK:  // 平台注册回应
				//81 00 00 05 01 78 87 11 33 01 00 00 e1 3d 00 31 30 84
                CDSMLog::Trace("==>JTT_PLAT_REGISTER_ACK");
                if (_ProcessRegisterResp((BYTE *)m_szRecvOnePkt, m_nRecvPktOffset) == REGISTER_OK) {
                    pClientConn->UpdateConnStatus(NET_REGISTED); // 更新状态
                }
                //
                break;

            case JTT_PLAT_GENERAL_ACK:  // 平台通用应答
                CDSMLog::Trace("==>JTT_PLAT_GENERAL_ACK");
                _ProcessGeneralResp(pClientConn,(BYTE *) m_szRecvOnePkt, m_nRecvPktOffset);
                break;

            case JTT_PLT_GET_PARA: // 查询终端参数
                CDSMLog::Trace("==>JTT_PLT_GET_PARA");
                break;

            default:
                CDSMLog::Error("Unknown cmd[%2x]",cmd);
                break;
        }
        //pClientConn->UpdateRecvPktTick(CUtil::GetInstance()->get_tick_count());
    }
    return;
}




/**
  * @brief  终端通用应答
  * @param  serialID 应答流水号
  * @param  answerID 应答ID
  * @param  result	 结果
  * @retval length of data
  */
//int CMsgProcess::DevCommResp(WORD serialID,WORD answerID,BYTE result)
//{
//    int len = -1;
//
//    WORD msgID = JTT_DEV_GENERAL_ACK;
//    m_sendBuffer[0] = (BYTE) (msgID>>8);
//    m_sendBuffer[1] = (BYTE) (msgID);
//
//    m_sendBuffer[2] = (BYTE) (serialID>>8);
//    m_sendBuffer[3] = (BYTE) (serialID);
//
//    m_sendBuffer[4] = (BYTE) answerID>>8;
//    m_sendBuffer[5] = (BYTE) (answerID);
//
//    m_sendBuffer[6] = result;
//
//    len = 7;
//    return len;
//}


/**
  * @brief  终端鉴权
  * @param  authentication_code 鉴权码
  * @param  length 鉴权码长度
  * @retval length of data
  */
int CMsgProcess::DevAuthentication()
{
    char* szAuthCode = CConfigFileReader::GetInstance()->GetConfigName("auth_code");
    if (szAuthCode == NULL)
        szAuthCode = (char*)m_szAuthCode;
    endswap(szAuthCode);
    this->_ConstructDevAuthPkt(szAuthCode);
    this->_SendToPlt();

    return 1;
}

/**
  * @brief  终端心跳
  * @param  None
  * @retval length of data
  */
int CMsgProcess::DevHeartBeat(void)
{
    CDSMLog::Trace("Start DevHeartBeat");
    _ConstructDevHeartBeatPkt();
    _SendToPlt();

    return 1;
}


int CMsgProcess::DevRegister(void)
{
//    miReplyOnePtr = 0;
//    memcpy( mchReplyOneBuffer , m_szRecvOnePkt , JT808Protocol::HEADERSIZE) ;
//    JT808Protocol *pReply = (JT808Protocol *)mchReplyOneBuffer ;
//    pReply->SetCmd(0x8100) ;
//    WORD dwSeqNo = pReply->SeqNo;   //应答流水号
//    memcpy( pReply->MsgBd ,  &dwSeqNo ,2 ) ; //
//    *(pReply->MsgBd+2) = 0 ;
//    memcpy( pReply->MsgBd + 3 ,  "YCGIS",6 ) ; //
//    pReply->SetMsgBodyLength(9) ;
//    *(pReply->MsgBd +9) =  MakeCheckProtocol(pReply) ;
//    *(pReply->MsgBd +10) = 0x7e ;
//    miReplyOnePtr = JT808Protocol::HEADERSIZE+pReply->GetMsgBodyLength()+1 ; //一个字节的尾标志

    this->_ConstructDevRegisterPkt();
    this->_SendToPlt();

	return 1;
}

/**
  * @brief  终端注销
  * @param  None
  * @retval length of data
  */
int CMsgProcess::DevUnregister(void)
{
    int len = -1;
    WORD msgID = JTT_DEV_LOGOUT;
    m_sendBuffer[0] = (BYTE) (msgID>>8);
    m_sendBuffer[1] = (BYTE) (msgID);

    len = 2;
    return len;
}



/**
  * @brief  查询终端参数应答0x0104
  * @param  serialID 应答流水号
  * @param  argNum 应答参数个数
  * @param  dev_arg 参数项列表
  * @retval length of data
  */
//int CMsgProcess::DevGetParameterResp(WORD serialID,BYTE argNum,STR_PARAMETER *dev_arg)
//{
//    int len = -1;
//    BYTE i,k;
//    WORD j;
//
//    WORD msgID = JTT_PLT_GET_PARA_RESP;
//    m_sendBuffer[0] = (BYTE) (msgID>>8);
//    m_sendBuffer[1] = (BYTE) (msgID);
//
//    m_sendBuffer[2] = (BYTE) serialID>>8;
//    m_sendBuffer[3] = (BYTE) (serialID);
//
//    m_sendBuffer[4] = argNum;
//
//    k = 0;
//    for(i=0;i<argNum;i++)
//    {
//        for(j=0;j<dev_arg->data[i].count;j++)
//        {
//            m_sendBuffer[5+k] = dev_arg->data[i].idList[j];
//        }
//        k += dev_arg->data[i].count;
//    }
//
//    len = k+5;
//    return len;
//}


/**
  * @brief  查询终端属性应答
  * @param  attr 终端属性
  * @retval length of data
  */
//int CMsgProcess::DevGetAttriResp(STR_DEV_ATTR attr)
//{
//    int len = -1;
//    int i,j;
//
//    WORD msgID = JTT_PLT_GET_DEV_ATTRI_RESP;
//    m_sendBuffer[0] = (BYTE) (msgID>>8);
//    m_sendBuffer[1] = (BYTE) (msgID);
//
//    m_sendBuffer[2] = (BYTE)(attr.dev_type>>8);
//    m_sendBuffer[3] = (BYTE) (attr.dev_type);
//
//    for(i=0;i<5;i++)
//        m_sendBuffer[4+i] = attr.manufacturer_ID[i];
//    for(i=0;i<20;i++)
//        m_sendBuffer[9+i] = attr.dev_model[i];
//    for(i=0;i<7;i++)
//        m_sendBuffer[29+i] = attr.dev_ID[i];
//    for(i=0;i<10;i++)
//        m_sendBuffer[44+i] = attr.ICCID[i];
//
//    m_sendBuffer[54] = attr.dev_version_len;
//    for(i=0;i<attr.dev_version_len;i++)
//        m_sendBuffer[55+i] = attr.dev_version[i];
//    j = attr.dev_version_len;
//
//    m_sendBuffer[55+j] = attr.firmware_version_len;
//    for(i=0;i<attr.firmware_version_len;i++)
//        m_sendBuffer[56+j] = attr.firmware_version[i];
//    j += attr.firmware_version_len;
//
//    m_sendBuffer[56+j] = attr.gnns;
//    m_sendBuffer[57+j] = attr.communication_module;
//
//    len = 57+j+1;
//    return len;
//}

/**
  * @brief  位置信息汇报
  * @retval length of data
  */
int CMsgProcess::DevLocationUp(uint64_t latitude,uint64_t longitude,uint32_t  height)
{
    _ConstructLocInfoPkt(latitude,longitude,height);
    _SendToPlt();

    return 1;

//    int len = -1;
//    int i;
//
//    WORD msgID = JTT_DEV_LOC_REP;
//    m_sendBuffer[0] = (BYTE) (msgID>>8);
//    m_sendBuffer[1] = (BYTE) (msgID);
//
//    m_sendBuffer[2] = (BYTE) (serialID>>8);
//    m_sendBuffer[3] = (BYTE) (serialID);
//
//    m_sendBuffer[4] = (BYTE) (location.alarm_flag >> 24);
//    m_sendBuffer[5] = (BYTE) (location.alarm_flag >> 16);
//    m_sendBuffer[6] = (BYTE) (location.alarm_flag  >> 8);
//    m_sendBuffer[7] = (BYTE) (location.alarm_flag );
//
//    m_sendBuffer[8] = (BYTE) (location.status >> 24);
//    m_sendBuffer[9] = (BYTE) (location.status >> 16);
//    m_sendBuffer[10] = (BYTE) (location.status >> 8);
//    m_sendBuffer[11] = (BYTE) (location.status );
//
//    m_sendBuffer[12] = (BYTE) (location.latitude >> 24);
//    m_sendBuffer[13] = (BYTE) (location.latitude >> 16);
//    m_sendBuffer[14] = (BYTE) (location.latitude >> 8);
//    m_sendBuffer[15] = (BYTE) (location.latitude );
//
//    m_sendBuffer[16] = (BYTE) (location.longitude>> 24);
//    m_sendBuffer[17] = (BYTE) (location.longitude  >> 16);
//    m_sendBuffer[18] = (BYTE) location.longitude >> 8;
//    m_sendBuffer[19] = (BYTE) (location.longitude );
//
//    m_sendBuffer[20] = (BYTE) (location.high >> 8);
//    m_sendBuffer[21] = (BYTE) (location.high );
//
//    m_sendBuffer[22] = (BYTE) (location.speed >> 8);
//    m_sendBuffer[23] = (BYTE) (location.speed );
//
//    m_sendBuffer[24] = (BYTE) (location.direction >> 8);
//    m_sendBuffer[25] = (BYTE) (location.direction );
//
//    for(i=0;i<6;i++)
//        m_sendBuffer[26+i] = location.time[i];
//
//    len = 32;
//    return len;
}

void CMsgProcess::_ConstructPktHeader(WORD nCmd, WORD nBodyLen)
{
    JT808Protocol* pPktHeader = (JT808Protocol*)m_szReqOneBuffer;
    m_reqPtrOffset = 0;

    // 消息ID 2
    pPktHeader->SetCmd(nCmd);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    pPktHeader->SetMsgBodyLength(nBodyLen);
    pPktHeader->DisableEncrypt();
    pPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;
    // 手机号 6
    Str2BCD((char*)pPktHeader->SimNo,CConfigFileReader::GetInstance()->GetConfigName("sim_no"));
    endswap(&(pPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pPktHeader->SetSeqNo(this->GenerateSeqNo());
    m_reqPtrOffset += 2;
    CDSMLog::Trace("+++BodyLen[%d] from header",pPktHeader->GetMsgBodyLength());
}

void CMsgProcess::_ConstructLocInfoPkt(uint64_t latitude,uint64_t longitude,uint32_t  height)
{
//    JT808Protocol* pLocUpPktHeader = (JT808Protocol*)m_szReqOneBuffer;
//    m_reqPtrOffset = 0;
//
//    // 消息ID 2
//    pLocUpPktHeader->SetCmd(JTT_DEV_AUTH);
//    m_reqPtrOffset += 2;
//    // 消息体属性 2
//
//    pLocUpPktHeader->DisableEncrypt();
//    pLocUpPktHeader->DisableMultiPacket();
//    m_reqPtrOffset += 2;
//    // 手机号 6
//    Str2BCD((char*)pLocUpPktHeader->SimNo,"013311877881");
//    m_reqPtrOffset += 6;
//    // 消息流水号 2
//    pLocUpPktHeader->SetSeqNo(this->GenerateSeqNo());
//    m_reqPtrOffset += 2;

    _ConstructPktHeader(JTT_DEV_LOC_REP,sizeof(JTT808Body_PositionUP));
    int nBodyLen = 0;
    JTT808Body_PositionUP* pLocUpPkt = (JTT808Body_PositionUP*)(m_szReqOneBuffer + JTT808MsgHead::HEADERSIZE);
    pLocUpPkt->EnableFatigueFlag();
//	pLocUpPkt->SetLatitude(45890000);
//	pLocUpPkt->SetLongitude(23480000);
//	pLocUpPkt->SetHigh(100);

    pLocUpPkt->SetLatitude(latitude);
    pLocUpPkt->SetLongitude(longitude);
    pLocUpPkt->SetHigh(height);

	pLocUpPkt->SetSpeed(80);
	pLocUpPkt->SetDirection(0);
    std::time_t t = std::time(NULL);

    char mbstr[18] = {0};
    if (std::strftime(mbstr, 18, "%y-%m-%d-%H-%M-%S", std::localtime(&t))) {
        Str2BCD((char*)pLocUpPkt->time,mbstr);
        endswap(&(pLocUpPkt->time));
    }
    m_reqPtrOffset += sizeof(JTT808Body_PositionUP);
    // 消息尾
    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;
    CDSMLog::Trace("+++BodyLen[%d] from sizeof",sizeof(JTT808Body_PositionUP));
    CDSMLog::Trace("m_reqPtrOffset=%d",m_reqPtrOffset);
    CDSMLog::dsm_dump((unsigned char*)m_szReqOneBuffer,m_reqPtrOffset);

    return;
}

/**
 * 构造终端监权报文
 */
void CMsgProcess::_ConstructDevAuthPkt(const char* authentication_code)
{
    JT808Protocol *pAuthPktHeader = (JT808Protocol*)m_szReqOneBuffer;
    m_reqPtrOffset = 0;
    // 消息ID 2
    pAuthPktHeader->SetCmd(JTT_DEV_AUTH);
    m_reqPtrOffset += 2;
    // 消息体属性 2

    pAuthPktHeader->DisableEncrypt();
    pAuthPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;
    // 手机号 6
    Str2BCD((char*)pAuthPktHeader->SimNo,CConfigFileReader::GetInstance()->GetConfigName("sim_no"));
    endswap(&(pAuthPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pAuthPktHeader->SetSeqNo(this->GenerateSeqNo());
    m_reqPtrOffset += 2;

    int nBodyLen = 0;
    memcpy(m_szReqOneBuffer+m_reqPtrOffset,authentication_code,strlen(authentication_code));
    nBodyLen += strlen(authentication_code);
    pAuthPktHeader->SetMsgBodyLength(nBodyLen);
    m_reqPtrOffset += nBodyLen;

    // 消息尾
    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;

    CDSMLog::Trace("m_reqPtrOffset=%d",m_reqPtrOffset);
    CDSMLog::dsm_dump((unsigned char*)m_szReqOneBuffer,m_reqPtrOffset);

    return;
}

/**
 *  构造终端保活报文
 *
 */
void CMsgProcess::_ConstructDevHeartBeatPkt()
{
    JT808Protocol *pHertBeatPktHeader = (JT808Protocol*)m_szReqOneBuffer;
    m_reqPtrOffset = 0;
    // 消息ID 2
    pHertBeatPktHeader->SetCmd(JTT_DEV_HEARTBEAT);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    pHertBeatPktHeader->SetMsgBodyLength(0);
    pHertBeatPktHeader->DisableEncrypt();
    pHertBeatPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;
    // 手机号 6
    Str2BCD((char*)pHertBeatPktHeader->SimNo,CConfigFileReader::GetInstance()->GetConfigName("sim_no"));
    endswap(&(pHertBeatPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pHertBeatPktHeader->SetSeqNo(this->GenerateSeqNo());
    m_reqPtrOffset += 2;
    // 消息尾
    *((BYTE*)pHertBeatPktHeader + m_reqPtrOffset + pHertBeatPktHeader->GetMsgBodyLength()) = MakeCheckProtocol((BYTE*)pHertBeatPktHeader,pHertBeatPktHeader->HEADERSIZE);  // 校验码
    m_reqPtrOffset++;

    CDSMLog::Trace("m_reqPtrOffset=%d",m_reqPtrOffset);
    CDSMLog::dsm_dump((unsigned char*)m_szReqOneBuffer,m_reqPtrOffset);

    return;
}


/**
 *  构造终端注册报文
 *
 */
void CMsgProcess::_ConstructDevRegisterPkt()
{
    m_reqPtrOffset = 0;
    JTT808MsgHead *pRegPktHeader = (JTT808MsgHead*)m_szReqOneBuffer;

    // 消息ID 2
    pRegPktHeader->SetCmd(JTT_DEV_REGISTER);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    //pRegPkt->SetMsgBodyLength(0);
    pRegPktHeader->DisableEncrypt();
    pRegPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;

    // 手机号 6
    Str2BCD((char*)pRegPktHeader->SimNo,CConfigFileReader::GetInstance()->GetConfigName("sim_no"));
    endswap(&(pRegPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pRegPktHeader->SetSeqNo(this->GenerateSeqNo());//请求流水号
    m_reqPtrOffset += 2;

    size_t nBodyLen = 0;
    JTT808RegPktUP* pRegisterPkt = (JTT808RegPktUP*)(m_szReqOneBuffer + JTT808MsgHead::HEADERSIZE);
    pRegisterPkt->SetProviceId(11); nBodyLen += 2;
    pRegisterPkt->SetCityId(105);  nBodyLen += 2;
    //终端标识：YCAC2110322<br>车牌号：京NWD888<br>车牌颜色：白色<br>所属车队：北京亿车安
    //char szManuId[5] = "audi";
	char* szManuId = CConfigFileReader::GetInstance()->GetConfigName("manufactur_id");
    pRegisterPkt->SetManuId((BYTE*)szManuId); nBodyLen += 5;

    //char szModuleId[20] = "13811088446";
	char* szModuleId = CConfigFileReader::GetInstance()->GetConfigName("dev_model");
    pRegisterPkt->SetDevModelId((BYTE*)szModuleId); nBodyLen += 20;	
	
    //char szDeviceId[7] = "device";
	char* szDeviceId = CConfigFileReader::GetInstance()->GetConfigName("dev_id");
    pRegisterPkt->SetDevId((BYTE*)szDeviceId); nBodyLen += 7;

	char* szColor = CConfigFileReader::GetInstance()->GetConfigName("license_color");
    pRegisterPkt->SetLicenseColor(atoi(szColor)); nBodyLen += 1;// 1-蓝色 2-黄色 3-黑色 4-白色 9-其它
		
    //char szFlag[16] = "京NWD888";
    // for(int i = 0; i < strlen(szFlag); i++)
	// 	m_szReqOneBuffer[m_reqPtrOffset+nBodyLen++] = szFlag[i];
    char* szFlag = CConfigFileReader::GetInstance()->GetConfigName("license_number");
	for(int i = 0; i < strlen(szFlag); i++)
		m_szReqOneBuffer[m_reqPtrOffset+nBodyLen++] = szFlag[i];
	
    pRegPktHeader->SetMsgBodyLength(nBodyLen);
    m_reqPtrOffset += nBodyLen;

    WORD nLenTemp = pRegPktHeader->GetMsgBodyLength();

    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;

    CDSMLog::Trace("m_reqPtrOffset=%d",m_reqPtrOffset);
    CDSMLog::dsm_dump((unsigned char*)m_szReqOneBuffer,m_reqPtrOffset);
}

/**
  * @brief  数据发送
  * @param  len 消息数据的长度
  * @retval
    *@arg 0 成功
    *@arg <0 失败
  */
int CMsgProcess::_SendToPlt()
{
	ReqBufferTransfer(); // 转义报文
    //ClientSend((const BYTE*)m_szReqRawOneBuffer,m_reqRawPtr);
    return m_reqPtrOffset;;
}

//void CMsgProcess::SetLocation(uint64_t latitude,uint64_t longitude,uint32_t  height)
//{
//    m_latitude = latitude;
//    m_longitude = longitude;
//    m_heigh = height;
//}