//
// Created by public on 19-3-19.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "base/msg_def.h"
#include "msg_process.h"
#include "api/dsm_jtt808_api.h"


CMsgProcess::CMsgProcess():m_pCurrentUpLoading(NULL)
{
    memset(m_respBuffer,0,sizeof(m_respBuffer));
    memset(m_sendBuffer,0,sizeof(m_sendBuffer));
	memset(m_szAuthCode,0,sizeof(m_szAuthCode));
}


CMsgProcess::~CMsgProcess()
{

}

//
// 处理注册回应的消息
//
int CMsgProcess::_ProcessRegisterResp(BYTE *pRegResp, int nLen)
{
	//81 00 00 05 01 78 87 11 33 01 00 00
	//e1 3d 00 31 30 84
	if (NULL == pRegResp || nLen < 0) {
        UT_ERROR("Parameter error!");
        return -1;
    }

    JTT808MsgHead* pMsgHeader = (JTT808MsgHead*)pRegResp;
    if (nLen <= pMsgHeader->HEADERSIZE) {
        // 非法包
        UT_ERROR("Package is less than header!");
        return -1;
    }

	BYTE* pTemp = pRegResp + pMsgHeader->HEADERSIZE;
	size_t nBodyLen = pMsgHeader->GetMsgBodyLength();
    //UT_TRACE("The response msgLen[%x]",nBodyLen);
    bool bRegResult = false;  // 注册结果
    pTemp += 2; // 跳过应答流水号
	switch(*pTemp) // 解析注册回应结果
	{
		case REGISTER_OK:   //!<  成功
			UT_INFO("注册返回: 注册成功");
            bRegResult = true;
			break;

		case REGISTER_CAR_REGISTERED:    //!< 车辆已被注册
			UT_INFO("注册返回: 车辆已被注册");
			break;
					
		case REGISTER_HAVE_NO_CAR:       //!< 数据库中无该车辆
			UT_INFO("注册返回: 数据库中无该车辆");
			break;
					
		case REGISTER_DEV_REGISTERED:   //!< 终端已被注册
			UT_INFO("注册返回: 终端已被注册");
			break;
					
		case REGISTER_HAVE_NO_DEV:	    //!< 数据库中无该终端
			UT_INFO("注册返回: 数据库中无该终端");
			break;

        default:
		    UT_ERROR("注册返回: Unknown response code[%d]",*pTemp);
			break;
	}
    pTemp++; // 跳过应答的结果

    if (bRegResult)  { // 注册成功
        memcpy(m_szAuthCode,pTemp,nBodyLen-3);
        UT_DUMP(m_szAuthCode,strlen(m_szAuthCode));
        CConfigFileReader::GetInstance()->SetConfigValue("auth_code",(const char*)m_szAuthCode); //保存到配置文件
        return REGISTER_OK;
    }

	return -1;
}


// 处理平台发送的9208消息
int CMsgProcess::_ProcessPltAccessoryReq(CClientConn *pClientConn, BYTE *pAccessoryReq, int nLen)
{
    if (NULL == pClientConn || NULL == pAccessoryReq || nLen < 0) {
        UT_ERROR("Parameter error!");
        return -1;
    }

    JTT808MsgHead* pMsgHeader = (JTT808MsgHead*)pAccessoryReq;
    if (nLen <= pMsgHeader->HEADERSIZE) {
        // 非法包
        UT_ERROR("Package is less than header!");
        return -1;
    }

    BYTE* pBody = pAccessoryReq + pMsgHeader->HEADERSIZE;
    int nSeq = pMsgHeader->SeqNo;
    endswap(&nSeq);

    JTT808_SU_Accessory_up objAccessUp;
    objAccessUp.Format(pBody);
 
    DevCommResp(pClientConn,nSeq,JTT_SU_PLT_ACCESSORY_UP,0); // 往平台发通用应答消息

    int nClientFd = CClientConnManager::GetInstance()->Connect(pClientConn->GetSimNo(),pClientConn->GetDevModel(),
            objAccessUp.strIp,objAccessUp.nTcpPort);
    UT_INFO("Start Connect accessory server ip[%s] port[%d] clientFd[%d]...",objAccessUp.strIp.c_str(),objAccessUp.nTcpPort,nClientFd);
    // 给附件服务器发指令
    CClientConn* pClient = CClientConnManager::GetInstance()->GetClientConnByFd(nClientFd);
    if (NULL != pClient)
        DevAlarmAccessoryUp(pClient,objAccessUp.alarmFlag, objAccessUp.alarmNo);
    else
        UT_FATAL("Send msg[0X1210] failed!");

    return  1;
}

/**
 *0x1210
 * @param pAlarmFlag  报警标识号
 * @param pAlarmNo 报警编号
 * @return
 */
int CMsgProcess::DevAlarmAccessoryUp(CClientConn* pClient,BYTE* pAlarmFlag, BYTE* pAlarmNo)
{
    m_reqPtrOffset = 0;
    JTT808MsgHead *pPktHeader = (JTT808MsgHead*)m_szReqOneBuffer;

    // 消息ID 2
    pPktHeader->SetCmd(JTT_SU_DEV_ACCESSORY_UP);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    //pRegPkt->SetMsgBodyLength(0);
    pPktHeader->DisableEncrypt();
    pPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;

    // 手机号 6
    Str2BCD((char*)pPktHeader->SimNo,(char*)pClient->GetSimNo());
    endswap(&(pPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pPktHeader->SetSeqNo(this->GenerateSeqNo());//请求流水号
    m_reqPtrOffset += 2;

    int nBodyLen = 0;
    JTT808_SU_DEV_ACCESSORY_UP* pDevAccessoryUp = (JTT808_SU_DEV_ACCESSORY_UP*)(m_szReqOneBuffer + JTT808MsgHead::HEADERSIZE);
    std::vector<AlarmAccessory>  vAccessories;
    if (!CClientConnManager::GetInstance()->GetAlarmAccessory((char *) pAlarmFlag, vAccessories))
        return -1;

    char* szDeviceId = CConfigFileReader::GetInstance()->GetConfigName("dev_id");
    memcpy(pDevAccessoryUp->dev_ID,szDeviceId,sizeof(pDevAccessoryUp->dev_ID));
    memcpy(pDevAccessoryUp->alarmFlag,pAlarmFlag,sizeof(pDevAccessoryUp->alarmFlag));
    memcpy(pDevAccessoryUp->alarmNo,pAlarmNo,sizeof(pDevAccessoryUp->alarmNo));
    pDevAccessoryUp->accessoryNum = vAccessories.size();
    pDevAccessoryUp->accessoryType = 0;
    nBodyLen += LEN_DEV_ACCESSORY_UP;

    JTT808_SU_ACCESSORY* pAccessory = (JTT808_SU_ACCESSORY*)(m_szReqOneBuffer + JTT808MsgHead::HEADERSIZE + nBodyLen);

    //<文件类型>_<通道号>_<报警类型>_<序号>_<报警编号>.<后缀名>
    std::string strFileName,strRealName;
    std::vector<AlarmAccessory>::iterator pIter = vAccessories.begin();
    for (; pIter != vAccessories.end() ; ++pIter) {
        strRealName = (char*)pIter->stFileName;
        strFileName = pIter->stPltFileName;
        strFileName.append((char*)pDevAccessoryUp->alarmNo);

        if (pIter->stFileType == euPIC)
            strFileName.append(".jpg");
        else if(pIter->stFileType == euVideo)
            strFileName.append(".avi");
        else
            strFileName.append(".jpg");
        pAccessory->fileNameLen = strFileName.length();
        nBodyLen += 1;

        UT_TRACE("FileName[%s],length[%lu]",strFileName.c_str(),strFileName.length());
        memcpy(m_szReqOneBuffer + JTT808MsgHead::HEADERSIZE + nBodyLen,strFileName.c_str(),strFileName.length());
        nBodyLen += strFileName.length();

        CFile file;
        file.Open(pIter->stFileName, O_RDONLY);
        size_t size = 0;
        if (!file.GetSize(size))
            UT_FATAL("Get file[%s] size error",pIter->stFileName);
        DWORD fileSize = size;
        endswap(&fileSize);
        memcpy(m_szReqOneBuffer + JTT808MsgHead::HEADERSIZE + nBodyLen,&fileSize,sizeof(pAccessory->fileSize));
        nBodyLen += sizeof(pAccessory->fileSize);

        FileInfo objFileInfo(strRealName,euPIC,size);
        strcpy(objFileInfo.m_szFileName,strFileName.c_str());
        pClient->UpdateUpFileInfo(strFileName,objFileInfo);
    }

    m_reqPtrOffset += nBodyLen;
    pPktHeader->SetMsgBodyLength(nBodyLen);

    // 消息尾
    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;
    //UT_TRACE("+++BodyLen[%d] from sizeof",nBodyLen);
    UT_TRACE("m_reqPtrOffset=%d",m_reqPtrOffset);
    UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);

    _SendToPlt(pClient);

    CClientConnManager::GetInstance()->DelAlarmFlag((char*)pDevAccessoryUp->alarmFlag);
    return 1;
}


/**
 * 往平台传输文件
 * @param pClientConn
 * @param pFileInfo   需要上传的文件类型
 * @return
 */
int CMsgProcess::DevFileUpload(CClientConn* pClientConn,FileInfo* pFileInfo)
{
    CFile file;
    CFileMapping fmap;
    bool bResult = CUtil::GetInstance()->ReadFile(pFileInfo->m_strOrigFileName.c_str(),file,fmap);
    if (!bResult){
		UT_FATAL("Read file[%s] failed!",pFileInfo->m_strOrigFileName.c_str());
		return 1;
	}

    int nLeftSend = (int)fmap.Size();
    DWORD nSendBytes =  0;
    DWORD nOffset = 0;

    UT_TRACE("===Start send file[%s]===",pFileInfo->m_szFileName);
    while(nLeftSend>0) {
        nSendBytes = (nLeftSend <= FILE_SIZE_PER_PACK ? nLeftSend : FILE_SIZE_PER_PACK);
        m_reqPtrOffset = 0;
        m_szReqOneBuffer[0] = 0x30;
        m_szReqOneBuffer[1] = 0x31;
        m_szReqOneBuffer[2] = 0x63;
        m_szReqOneBuffer[3] = 0x64;
        m_reqPtrOffset += sizeof(DWORD);

        //UT_TRACE("File[%s],length[%lu]",pFileInfo->m_szFileName,strlen(pFileInfo->m_szFileName));
        BYTE    szFileName[50] = {0};
        strcpy((char*)szFileName,pFileInfo->m_szFileName);
        memcpy(m_szReqOneBuffer + m_reqPtrOffset,szFileName, 50);
        m_reqPtrOffset += 50; // 文件名称的长度

        DWORD  nTempOffSet = nOffset;
        endswap(&nTempOffSet);
        memcpy(m_szReqOneBuffer + m_reqPtrOffset,&nTempOffSet,sizeof(DWORD));  // 数据偏移量
        m_reqPtrOffset += sizeof(DWORD);

        DWORD nTempSendBytes = nSendBytes;
        endswap(&nTempSendBytes);
        memcpy(m_szReqOneBuffer + m_reqPtrOffset,&nTempSendBytes,sizeof(DWORD)); // 数据长度
        m_reqPtrOffset += sizeof(DWORD);
        UT_TRACE("--->Send File");
        UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);

        memcpy(m_szReqOneBuffer + m_reqPtrOffset,(BYTE*)fmap + nOffset,nSendBytes);
        m_reqPtrOffset += nSendBytes;
        nOffset += nSendBytes;  // 更新偏移量
        nLeftSend -= nSendBytes;  // 计算还剩下多少个字节
        pClientConn->ClientSend((const BYTE*)m_szReqOneBuffer, m_reqPtrOffset); // 不需要转义,直接发送原始数据包
    }

    // 平台已经正确回应了终端需要发送的文件信息
    // 这里给平台发送文件信息
    m_reqPtrOffset = 0;
    JTT808MsgHead *pPktHeader = (JTT808MsgHead*)m_szReqOneBuffer;

    // 消息ID 2
    pPktHeader->SetCmd(JTT_SU_FILE_UP_COMPLETE);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    //pRegPkt->SetMsgBodyLength(0);
    pPktHeader->DisableEncrypt();
    pPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;

    // 手机号 6
    Str2BCD((char*)pPktHeader->SimNo,(char*)pClientConn->GetSimNo());
    endswap(&(pPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pPktHeader->SetSeqNo(this->GenerateSeqNo());//请求流水号
    m_reqPtrOffset += 2;

    int nBodyLen = 0;
    BYTE nFileNameLen = (BYTE)strlen(pFileInfo->m_szFileName);
    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,&nFileNameLen,1);
    nBodyLen += 1;

    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,pFileInfo->m_szFileName,nFileNameLen);
    nBodyLen += strlen(pFileInfo->m_szFileName);

    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,&(pFileInfo->m_euType),1);
    nBodyLen += 1;

    DWORD dwFileSize = pFileInfo->m_dwSize;
    endswap(&dwFileSize);
    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,&dwFileSize,4);
    nBodyLen += 4;

    m_reqPtrOffset += nBodyLen;
    pPktHeader->SetMsgBodyLength(nBodyLen);
    // 消息尾
    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;

    UT_TRACE("$$$Send File Complete$$$");
    UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);
	return _SendToPlt(pClientConn);
}

/**
 * 0x1211  文件信息上传
 * @param pClientConn
 * @param pResp
 * @param nLen
 * @return
 */
int CMsgProcess::_ProcessPltAccessoryUpResp(CClientConn* pClientConn)
{
    // 平台已经正确回应了终端需要发送的文件信息
    // 这里给平台发送文件信息
    std::map<std::string,FileInfo> files = pClientConn->GetUpFileInfo();
    std::map<std::string,FileInfo>::iterator pIter = files.begin();
    for (;pIter != files.end(); pIter++) {
        if (pIter->second.m_fileStatus == euInit) {
            break;
        }
    }
    if (pIter == files.end()) {
        // 到这里,没有文件需要传输了
        pClientConn->ClearUpFileInfo();
        // 到这里,此次上传附件的任务已经完成,删除和附件服务器之间的Socket连接
        CClientConnManager::GetInstance()->DeleteClientConnect(pClientConn->GetClientHandle());
        return -1;
    }
    pIter->second.m_fileStatus = euUploading; // 更新文件上传的状态
    m_pCurrentUpLoading = &pIter->second;   // 当前需要上传的文件

    m_reqPtrOffset = 0;
    JTT808MsgHead *pPktHeader = (JTT808MsgHead*)m_szReqOneBuffer;

    // 消息ID 2
    pPktHeader->SetCmd(JTT_SU_DEV_FILE_UP);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    //pRegPkt->SetMsgBodyLength(0);
    pPktHeader->DisableEncrypt();
    pPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;

    // 手机号 6
    Str2BCD((char*)pPktHeader->SimNo,(char*)pClientConn->GetSimNo());
    endswap(&(pPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pPktHeader->SetSeqNo(this->GenerateSeqNo());//请求流水号
    m_reqPtrOffset += 2;

    int nBodyLen = 0;
    BYTE nFileNameLen = (BYTE)strlen(pIter->second.m_szFileName);
    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,&nFileNameLen,1);
    nBodyLen += 1;

    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,pIter->second.m_szFileName,nFileNameLen);
    nBodyLen += strlen(pIter->second.m_szFileName);

    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,&(pIter->second.m_euType),1);
    nBodyLen += 1;

    DWORD dwFileSize = pIter->second.m_dwSize;
    endswap(&dwFileSize);
    memcpy(m_szReqOneBuffer + m_reqPtrOffset + nBodyLen,&dwFileSize,4);
    nBodyLen += 4;

    m_reqPtrOffset += nBodyLen;
    pPktHeader->SetMsgBodyLength(nBodyLen);
    // 消息尾
    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;
    //UT_TRACE("+++BodyLen[%d] from sizeof",nBodyLen);
    UT_TRACE("m_reqPtrOffset=%d",m_reqPtrOffset);
    UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);
    //pClientConn->UpdateUpFileStatus(pIter->second.m_szFileName,euUploading);
    return _SendToPlt(pClientConn);
}

/**
 * 向平台传输文件
 * @param pClientConn
 * @param pResp
 * @param nLen
 * @return
 */
int CMsgProcess::_ProcessPltUpFileOk(CClientConn* pClientConn,BYTE* pResp, int nLen)
{
    if (NULL == pClientConn || NULL == pResp || nLen < 0) {
        UT_ERROR("Parameter error!");
        return -1;
    }

    JTT808MsgHead* pMsgHeader = (JTT808MsgHead*)pResp;
    if (nLen <= pMsgHeader->HEADERSIZE) {
        // 非法包
        UT_ERROR("Package is less than header!");
        return -1;
    }

    BYTE* pBody = pResp + pMsgHeader->HEADERSIZE;
    int nSeq = pMsgHeader->SeqNo;
    endswap(&nSeq);

    PltFileUpCompleteAck objUpCompleteAck;
    objUpCompleteAck.Format(pBody);

    FileInfo* pFileInfo = pClientConn->GetFileItem((char*)objUpCompleteAck.szFileName);
	if (objUpCompleteAck.btUpResult == 0) {
	    //当前文件上传完成,如果有多个文件接着传下一个文件
        //m_pCurrentUpLoading->m_fileStatus = euLoadSucc; // 更新状态
        //pClientConn->UpdateUpFileStatus((char*)objUpCompleteAck.fileName,euLoadSucc);
        if (NULL != pFileInfo) {
            UT_INFO("Send file[%s] success!",pFileInfo->m_strOrigFileName.c_str());
        }
        pClientConn->DelFileItem((char*)objUpCompleteAck.szFileName);
        _ProcessPltAccessoryUpResp(pClientConn);  //检查还有没有文件需要传输
	} else {
        if (NULL != pFileInfo) {
            UT_ERROR("Send file[%s] failed!",pFileInfo->m_strOrigFileName.c_str());
        }
	}
    return 1;
}


///  处理平台通用应答的返回结果
/// \param pGeneralResp
/// \param nLen
/// \return
int CMsgProcess::_ProcessGeneralResp(CClientConn* pClientConn,BYTE *pGeneralResp, int nLen)
{
    //跳过指令头,解析对应应答ID的返回结果
    //80 01 00 05 01 78 87 11 33 01 00 0f
    // 6a dc 00 02 00 e2
    if (NULL == pClientConn || NULL == pGeneralResp || nLen < 0) {
        UT_ERROR("Parameter error!");
        return -1;
    }

    JTT808MsgHead* pMsgHeader = (JTT808MsgHead*)pGeneralResp;
    if (nLen <= pMsgHeader->HEADERSIZE) {
        // 非法包
        UT_ERROR("Package is less than header!");
        return -1;
    }

    //80 01 00 05 01 78 87 11 33 01 00 10
    // 57 af 00 02 00 b3
    BYTE* pTemp = pGeneralResp + pMsgHeader->HEADERSIZE;
    //UT_TRACE("The response msgLen[%x]",pMsgHeader->GetMsgBodyLength());

    pTemp += 2; // 跳过应答流水号
    WORD cmd = (*(pTemp) <<8) + *(pTemp + 1);
    pTemp += 2; // 跳过应答的消息ID

    switch(*pTemp)
    {
        case RET_OK: //0
            UT_INFO("Response result[%s] for MsgID[%x]","成功",cmd);
            if (cmd == JTT_DEV_AUTH) {
                // 收到注册的正确回应
                pClientConn->UpdateConnStatus(NET_AUTHENTICATED); // 更新状态
            }

            if (cmd == JTT_DEV_LOC_REP) {
                pClientConn->ResetLocation();
            }

            if (cmd == JTT_SU_DEV_ACCESSORY_UP) {
                // 收到报警附件信息应答
                UT_TRACE("Accessory server[%u] response msg[%d]",pClientConn->GetClientHandle(),JTT_SU_DEV_ACCESSORY_UP);
                // 启动一个线程来发送附件文件
                _ProcessPltAccessoryUpResp(pClientConn);
            }

            if (cmd == JTT_SU_DEV_FILE_UP) {
                //平台的1211应答,开始发送文件
                if (m_pCurrentUpLoading->m_fileStatus != euUploading)
                    return 1;
                DevFileUpload(pClientConn,m_pCurrentUpLoading);
            }
            break;

        case RET_FAILED:
            UT_INFO("Response result[%s] for MsgID[%x]","失败",cmd);
            break;

        case RET_MESSAGE_ERROR:
            UT_INFO("Response result[%s] for MsgID[%x]","消息有误",cmd);
            break;

        case RET_NOT_SUPPORT:
            UT_INFO("Response result[%s] for MsgID[%x]","不支持",cmd);
            break;

        case RET_ALARM:
            UT_INFO("Response result[%s] for MsgID[%x]","报警处理确认",cmd);
            break;

        case RET_PARAMETER_ERROR:
            UT_INFO("Response result[%s] for MsgID[%x]","参数错误",cmd);
            break;

        default:
            UT_ERROR("Unknown response result[%x] for MsgId[%x]",*pTemp,cmd);
            break;
    }

    return 1;
}

void CMsgProcess::ProcessMsg(CClientConn* pClientConn,BYTE* data, size_t nLen)
{
    if (NULL == pClientConn || NULL == data || nLen <= 0) {
        UT_ERROR("Invalidate parameter!");
        return;
    }

    if (!this->ReceiveData(data,nLen)) {
        UT_ERROR("Recive data failed!");
        return;
    }

    //81 00 00 05 01 78 87 11 33 01 00 00 5f df 00 31 30 d8
    WORD cmd;
    while(GetOnePacket(&cmd))
    {
        switch(cmd) {
            case JTT_PLAT_REGISTER_ACK:  // 平台注册回应
				//81 00 00 05 01 78 87 11 33 01 00 00 e1 3d 00 31 30 84
                UT_TRACE("==>JTT_PLAT_REGISTER_ACK");
                if (_ProcessRegisterResp((BYTE *)m_szRecvOnePkt, m_nRecvPktOffset) == REGISTER_OK) {
                    pClientConn->UpdateConnStatus(NET_REGISTED); // 更新状态
                }
                //
                break;

            case JTT_PLAT_GENERAL_ACK:  // 平台通用应答
                UT_TRACE("==>JTT_PLAT_GENERAL_ACK");
                _ProcessGeneralResp(pClientConn,(BYTE *) m_szRecvOnePkt, m_nRecvPktOffset);
                break;

            case JTT_PLT_GET_PARA: // 查询终端参数
                UT_TRACE("==>JTT_PLT_GET_PARA");
                break;

            case JTT_SU_PLT_ACCESSORY_UP:  // 平台请求报警附件上传指令
                UT_TRACE("==>JTT_SU_PLT_ACCESSORIES_UP");
                _ProcessPltAccessoryReq(pClientConn,(BYTE*)m_szRecvOnePkt, m_nRecvPktOffset);
                break;

            case JTT_SU_PLT_FILE_UP_OK:
                UT_TRACE("==>JTT_SU_PLT_FILE_UP_OK");
                _ProcessPltUpFileOk(pClientConn,(BYTE*)m_szRecvOnePkt, m_nRecvPktOffset);
                break;

            default:
                UT_ERROR("Unknown cmd[%2x]",cmd);
                break;
        }
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
int CMsgProcess::DevCommResp(CClientConn* pClientConn,WORD serialID,WORD answerID,BYTE result)
{
    _ConstructPktHeader(JTT_DEV_GENERAL_ACK,LEN_DEV_GEN_ACK,(char*)pClientConn->GetSimNo());

    DevGeneralAck* objGenAck = (DevGeneralAck*)(m_szReqOneBuffer + JT808Protocol::HEADERSIZE);
    objGenAck->SetSeqNo(serialID);
    objGenAck->SetAckId(answerID);
    objGenAck->nResult = result;
    m_reqPtrOffset += LEN_DEV_GEN_ACK;

    // 校验码
    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);
    m_reqPtrOffset++;

    return _SendToPlt(pClientConn);
}


/**
  * @brief  终端鉴权
  * @param  authentication_code 鉴权码
  * @param  length 鉴权码长度
  * @retval length of data
  */
int CMsgProcess::DevAuthentication(CClientConn* pClientConn)
{
    char* szAuthCode = CConfigFileReader::GetInstance()->GetConfigName("auth_code");
    if (szAuthCode == NULL)
        szAuthCode = (char*)m_szAuthCode;
    endswap(szAuthCode);
    this->_ConstructDevAuthPkt(szAuthCode,(char*)pClientConn->GetSimNo());
    this->_SendToPlt(pClientConn);

    return 1;
}

/**
  * @brief  终端心跳
  * @param  None
  * @retval length of data
  */
int CMsgProcess::DevHeartBeat(CClientConn* pClientConn)
{
    UT_TRACE("Start DevHeartBeat");
    _ConstructDevHeartBeatPkt((char*)pClientConn->GetSimNo());
    _SendToPlt(pClientConn);

    return 1;
}


int CMsgProcess::DevRegister(CClientConn* pClientConn)
{
    this->_ConstructDevRegisterPkt((char*)pClientConn->GetDevModel(),(char*)pClientConn->GetSimNo());
    this->_SendToPlt(pClientConn);

	return 1;
}

/**
  * @brief  终端注销
  * @param  None
  * @retval length of data
  */
int CMsgProcess::DevUnregister(CClientConn* pClientConn)
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
int CMsgProcess::DevGetParameterResp(WORD serialID,BYTE argNum,STR_PARAMETER *dev_arg)
{
    int len = -1;
    BYTE i,k;
    WORD j;

    WORD msgID = JTT_PLT_GET_PARA_RESP;
    m_sendBuffer[0] = (BYTE) (msgID>>8);
    m_sendBuffer[1] = (BYTE) (msgID);

    m_sendBuffer[2] = (BYTE) serialID>>8;
    m_sendBuffer[3] = (BYTE) (serialID);

    m_sendBuffer[4] = argNum;

    k = 0;
    for(i=0;i<argNum;i++)
    {
        for(j=0;j<dev_arg->data[i].count;j++)
        {
            m_sendBuffer[5+k] = dev_arg->data[i].idList[j];
        }
        k += dev_arg->data[i].count;
    }

    len = k+5;
    return len;
}


/**
  * @brief  查询终端属性应答
  * @param  attr 终端属性
  * @retval length of data
  */
int CMsgProcess::DevGetAttriResp(STR_DEV_ATTR attr)
{
    int len = -1;
    int i,j;

    WORD msgID = JTT_PLT_GET_DEV_ATTRI_RESP;
    m_sendBuffer[0] = (BYTE) (msgID>>8);
    m_sendBuffer[1] = (BYTE) (msgID);

    m_sendBuffer[2] = (BYTE)(attr.dev_type>>8);
    m_sendBuffer[3] = (BYTE) (attr.dev_type);

    for(i=0;i<5;i++)
        m_sendBuffer[4+i] = attr.manufacturer_ID[i];
    for(i=0;i<20;i++)
        m_sendBuffer[9+i] = attr.dev_model[i];
    for(i=0;i<7;i++)
        m_sendBuffer[29+i] = attr.dev_ID[i];
    for(i=0;i<10;i++)
        m_sendBuffer[44+i] = attr.ICCID[i];

    m_sendBuffer[54] = attr.dev_version_len;
    for(i=0;i<attr.dev_version_len;i++)
        m_sendBuffer[55+i] = attr.dev_version[i];
    j = attr.dev_version_len;

    m_sendBuffer[55+j] = attr.firmware_version_len;
    for(i=0;i<attr.firmware_version_len;i++)
        m_sendBuffer[56+j] = attr.firmware_version[i];
    j += attr.firmware_version_len;

    m_sendBuffer[56+j] = attr.gnns;
    m_sendBuffer[57+j] = attr.communication_module;

    len = 57+j+1;
    return len;
}

/**
  * @brief  位置信息汇报
  * @retval length of data
  */
int CMsgProcess::DevLocationUp(CClientConn* pClientConn)
{
    // 从消息队列中取位置报警信息
    DevLocInfo* pDevLoc = nullptr;
    if (CClientConnManager::GetInstance()->m_queueDevLoc.PopFront(&pDevLoc)) {
        _ConstructLocInfoPkt(pDevLoc,(char*)pClientConn->GetSimNo());
        _SendToPlt(pClientConn);
    }
    return 1;
}

void CMsgProcess::_ConstructPktHeader(WORD nCmd, WORD nBodyLen,char* szSimNo)
{
    JT808Protocol* pPktHeader = (JT808Protocol*)m_szReqOneBuffer;
    m_reqPtrOffset = 0;

    // 消息ID 2
    pPktHeader->SetCmd(nCmd);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    pPktHeader->SetMsgBodyLength(nBodyLen);
    pPktHeader->DisableEncrypt();
    //pPktHeader->EnableEncrypt();
    pPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;
    // 手机号 6
    Str2BCD((char*)pPktHeader->SimNo,szSimNo);
    endswap(&(pPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pPktHeader->SetSeqNo(this->GenerateSeqNo());
    m_reqPtrOffset += 2;
}

void CMsgProcess::_ConstructLocInfoPkt(DevLocInfo* pDevLocInfo,char* szSimNo)
{
    if (NULL == pDevLocInfo) {
        UT_FATAL("Pointer to pDevLocInfo error!");
        return;
    }
    static DWORD dwAlarmId = 0;
    JT808Protocol* pPktHeader = (JT808Protocol*)m_szReqOneBuffer;
    m_reqPtrOffset = 0;

    // 消息ID 2
    pPktHeader->SetCmd(JTT_DEV_LOC_REP);
    m_reqPtrOffset += 2;
    // 消息体属性 2
    //pPktHeader->SetMsgBodyLength(nBodyLen);
    pPktHeader->DisableEncrypt();
    pPktHeader->DisableMultiPacket();
    m_reqPtrOffset += 2;
    // 手机号 6
    Str2BCD((char*)pPktHeader->SimNo,szSimNo);
    endswap(&(pPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pPktHeader->SetSeqNo(this->GenerateSeqNo());
    m_reqPtrOffset += 2;

    //_ConstructPktHeader(JTT_DEV_LOC_REP,sizeof(JTT808Body_PositionUP));
    int nBodyLen = 0;
    JTT808Body_PositionUP* pLocUpPkt = (JTT808Body_PositionUP*)(m_szReqOneBuffer + JTT808MsgHead::HEADERSIZE);
    pLocUpPkt->ResetAlarmFlag();
    //pLocUpPkt->EnableFatigueFlag();
    pLocUpPkt->ResetStatus();
    pLocUpPkt->SetLatitude(pDevLocInfo->stLatitude);
    pLocUpPkt->SetLongitude(pDevLocInfo->stLongitude);
    pLocUpPkt->SetHigh(pDevLocInfo->stHeight);
	pLocUpPkt->SetSpeed(pDevLocInfo->stSpeed);
	pLocUpPkt->SetDirection(0);
    std::time_t t = std::time(NULL);
    char mbstr[18] = {0};
    if (std::strftime(mbstr, 18, "%y%m%d%H%M%S", std::localtime(&t))) {
        Str2BCD((char*)pLocUpPkt->time,(char*)mbstr);
        endswap(&(pLocUpPkt->time));
    }
    m_reqPtrOffset += sizeof(JTT808Body_PositionUP);
    nBodyLen += sizeof(JTT808Body_PositionUP);

    if (pDevLocInfo->stHasAlarm) {
        // 构造位置附加信息
        JTT808Body_PositionUP_Extra* pPosExtra = (JTT808Body_PositionUP_Extra*)(m_szReqOneBuffer + m_reqPtrOffset);
        pPosExtra->extra_id = pDevLocInfo->stAlarmChannel;
        pPosExtra->extra_len = LEN_SU808_ALARM;  // 附加信息长度
        m_reqPtrOffset += sizeof(JTT808Body_PositionUP_Extra);
        nBodyLen += sizeof(JTT808Body_PositionUP_Extra);

        JTT808_SU_Body_DSM_Alarm* pDSMAlarm = (JTT808_SU_Body_DSM_Alarm*)(m_szReqOneBuffer + m_reqPtrOffset);
        //pDSMAlarm->SetAlarmId((dwAlarmId++)%MAXUINT32);
        pDSMAlarm->SetAlarmId((dwAlarmId++)%0XFFFFFFFF);
        pDSMAlarm->btStatus = 0;
        pDSMAlarm->btAlarmType = pDevLocInfo->stAlaryType;
        if (pDevLocInfo->stAlaryType == euFatigue)
            pLocUpPkt->EnableFatigueFlag();
        else
            pLocUpPkt->EnableDangerousFlag();

        pDSMAlarm->btAlarmGrade = 1;
        pDSMAlarm->btFatiqueDegree = 1;

        DWORD dwReserved = 0;
        memcpy(pDSMAlarm->btReserved,&dwReserved,sizeof(pDSMAlarm->btReserved));
        pDSMAlarm->btSpeed = pDevLocInfo->stSpeed;
        pDSMAlarm->SetHigh(pDevLocInfo->stHeight);
        pDSMAlarm->SetLatitude(pDevLocInfo->stLatitude);
        pDSMAlarm->SetLongitude(pDevLocInfo->stLongitude);
        if (std::strftime(mbstr, 18, "%y%m%d%H%M%S", std::localtime(&t))) {
            Str2BCD((char*)pDSMAlarm->time,mbstr);
            endswap(&(pDSMAlarm->time));
        }
        pDSMAlarm->status = 0;
        nBodyLen += sizeof(JTT808_SU_Body_DSM_Alarm);

        // 设置报警标志号
        int offset_alram_flag = 0;
        OFFSET(JTT808_SU_Body_DSM_Alarm,btAlarmFlag,offset_alram_flag);
        JTT808_SU_Body_DSM_Alarm_Flag* pDSMAlarmFlag = (JTT808_SU_Body_DSM_Alarm_Flag*)(m_szReqOneBuffer + m_reqPtrOffset + offset_alram_flag);
        char* szDeviceId = CConfigFileReader::GetInstance()->GetConfigName("dev_id");
        pDSMAlarmFlag->SetDevId((BYTE*)szDeviceId);
        if (std::strftime(mbstr, 18, "%y%m%d%H%M%S", std::localtime(&t))) {
            Str2BCD((char*)pDSMAlarmFlag->time,mbstr);
            endswap(&(pDSMAlarmFlag->time));
        }
        pDSMAlarmFlag->btSeqNo = 0;
        pDSMAlarmFlag->btAccessories = pDevLocInfo->stAccessories.size();  // 附件数量
        pDSMAlarmFlag->btReserved = 0;

        // 确定一下平台的文件名称
        int nAlarmIndex = 0;
        std::vector<AlarmAccessory>::iterator pIter = pDevLocInfo->stAccessories.begin();
        for (; pIter != pDevLocInfo->stAccessories.end() ; ++pIter) {
            std::string strPltFileName;
            // 文件类型
            if (pIter->stFileType == euPIC)
                strPltFileName.append("00_");
            else if (pIter->stFileType == euAudio)
                strPltFileName.append("01_");
            else if (pIter->stFileType == euVideo)
                strPltFileName.append("02_");
            else if (pIter->stFileType == euText)
                strPltFileName.append("03_");
            else
                strPltFileName.append("04_");

            // 通道号
            if (pDevLocInfo->stAlarmChannel == ADAS_ALARM_FLAG)
                strPltFileName.append("64_");
            else if (pDevLocInfo->stAlarmChannel == DSM_ALARM_FLAG)
                strPltFileName.append("65_");
            else
                strPltFileName.append("0_");

            // 报警类型
            if (pDevLocInfo->stAlarmChannel == DSM_ALARM_FLAG) {
                if(pDevLocInfo->stAlaryType == euFatigue)
                    strPltFileName.append("6501_");
                else if (pDevLocInfo->stAlaryType == euCall)
                    strPltFileName.append("6502_");
                else if (pDevLocInfo->stAlaryType == euSmoking)
                    strPltFileName.append("6503_");
                else if (pDevLocInfo->stAlaryType == euDistract)
                    strPltFileName.append("6504_");
                else
                    strPltFileName.append("6500_");
            }
            else if (pDevLocInfo->stAlarmChannel == ADAS_ALARM_FLAG) {
                strPltFileName.append("6401_");
            } else {
                strPltFileName.append("0000_");
            }

            // 序号
            char szIndex[8] = {0};
            itoa(nAlarmIndex, szIndex, 10);
            strPltFileName.append((char*)szIndex);
            strPltFileName.append("_");
            nAlarmIndex++;

            strcpy(pIter->stPltFileName,strPltFileName.c_str());
        }
        //std::string strAlarmFlag = pDSMAlarmFlag->FormatToString();
        //UT_TRACE("###alarm flag[%s]",strAlarmFlag.c_str());
        CClientConnManager::GetInstance()->UpdateAlarmFlag((char*)pDSMAlarmFlag,pDevLocInfo->stAccessories);
        m_reqPtrOffset += LEN_SU808_ALARM;  // 偏移累加
    }

    pPktHeader->SetMsgBodyLength(nBodyLen);
    // 消息尾
    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;
    UT_TRACE("m_reqPtrOffset=%d",m_reqPtrOffset);
    UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);

    return;
}

/**
 * 构造终端监权报文
 */
void CMsgProcess::_ConstructDevAuthPkt(const char* authentication_code,char* szSimNo)
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
    Str2BCD((char*)pAuthPktHeader->SimNo,szSimNo);
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

    UT_TRACE("m_reqPtrOffset=%d",m_reqPtrOffset);
    UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);

    return;
}

/**
 *  构造终端保活报文
 *
 */
void CMsgProcess::_ConstructDevHeartBeatPkt(char* szSimNo)
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
    Str2BCD((char*)pHertBeatPktHeader->SimNo,szSimNo);
    endswap(&(pHertBeatPktHeader->SimNo));
    m_reqPtrOffset += 6;
    // 消息流水号 2
    pHertBeatPktHeader->SetSeqNo(this->GenerateSeqNo());
    m_reqPtrOffset += 2;
    // 校验码
    *((BYTE*)pHertBeatPktHeader + m_reqPtrOffset + pHertBeatPktHeader->GetMsgBodyLength()) = MakeCheckProtocol((BYTE*)pHertBeatPktHeader,pHertBeatPktHeader->HEADERSIZE);  // 校验码
    m_reqPtrOffset++;

    UT_TRACE("m_reqPtrOffset=%d",m_reqPtrOffset);
    UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);

    return;
}


/**
 *  构造终端注册报文
 *
 */
void CMsgProcess::_ConstructDevRegisterPkt(const char*szModuleId,char* szSimNo)
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
    Str2BCD((char*)pRegPktHeader->SimNo,szSimNo);
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
	//char* szModuleId = CConfigFileReader::GetInstance()->GetConfigName("dev_model");
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

    //WORD nLenTemp = pRegPktHeader->GetMsgBodyLength();

    *((BYTE*)m_szReqOneBuffer + m_reqPtrOffset) = MakeCheckProtocol((BYTE*)m_szReqOneBuffer,m_reqPtrOffset);  // 校验码
    m_reqPtrOffset++;

    UT_TRACE("m_reqPtrOffset=%d",m_reqPtrOffset);
    UT_DUMP(m_szReqOneBuffer,m_reqPtrOffset);
}

/**
  * @brief  数据发送
  * @param  len 消息数据的长度
  * @retval
    *@arg 0 成功
    *@arg <0 失败
  */
int CMsgProcess::_SendToPlt(CClientConn* pClientConn)
{
	ReqBufferTransfer(); // 转义报文
    //UT_DUMP(m_szReqRawOneBuffer,m_reqRawPtr);
	pClientConn->ClientSend((const BYTE*)m_szReqRawOneBuffer, m_reqRawPtr);
    //CDsmJTT808_API::GetInstance()->ClientSend_API((const BYTE*)m_szReqRawOneBuffer,m_reqRawPtr);
    return m_reqPtrOffset;;
}
