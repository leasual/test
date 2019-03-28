//
// Created by public on 19-3-19.
//

#ifndef DSM_JTT808_HP_SOCKET_HELPER_H
#define DSM_JTT808_HP_SOCKET_HELPER_H

#include "hpsocket/HPSocket4C.h"
#include "hpsocket/HPTypeDef.h"

//namespace HPSocketHelper {

    En_HP_HandleResult __HP_CALL OnConnect(HP_Client pSender, HP_CONNID dwConnID);
    En_HP_HandleResult __HP_CALL OnReceive(HP_Client pSender, HP_CONNID dwConnID, int iLength);
    En_HP_HandleResult __HP_CALL OnSend(HP_Client pSender, HP_CONNID dwConnID, const BYTE* pData, int iLength);
    En_HP_HandleResult __HP_CALL OnClose(HP_Client pSender, HP_CONNID dwConnID, En_HP_SocketOperation enOperation, int iErrorCode);
    int StartTcpClient(const char *svr_ip, unsigned short int port);
    int StopTcpClient(void);
    bool CreateHPSocketObjects();
    void DestroyHPSocketObjects();
    int  ClientSend(const BYTE* buff, size_t nBuffLen);

    void OnCmdStart(void);
    void OnCmdStop(void);
    void OnCmdStatus(void);
    void OnCmdSend(void);
//};

#endif //DSM_JTT808_HP_SOCKET_HELPER_H
