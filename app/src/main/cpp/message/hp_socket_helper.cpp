//
// Created by public on 19-3-19.
//
#include <stdlib.h>
#include <stdio.h>

#include "hp_socket_helper.h"
#include "base/type_def.h"
#include "base/dsm_log.h"
#include "client_conn.h"
#include "client_conn_manager.h"

struct TPkgInfo{
    bool is_header;
    int length;
};

//HP_TcpPullClientListener s_listener;
//HP_TcpPullClient s_client;
TPkgInfo s_pkgInfo;

//namespace  HPSocketHelper {

    En_HP_HandleResult __HP_CALL OnConnect(HP_Client pSender, HP_CONNID dwConnID)
    {
        TCHAR szAddress[50];
        int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
        USHORT usPort;

        HP_Client_GetRemoteHost(pSender, szAddress, &iAddressLen, &usPort);

        return HR_OK;
    }

    En_HP_HandleResult __HP_CALL OnReceive(HP_Client pSender, HP_CONNID dwConnID, int iLength)
    {
        struct TPkgInfo * pInfo = (struct TPkgInfo*)HP_Client_GetExtra(pSender);
        if (pInfo == NULL ){
            printf("Null packet Error\n");
            return HR_OK;
        }

        char buffer[1024*4] = {0};
        //char *buffer = (char *)malloc(iLength);
        memset(buffer, 0, iLength);
        UT_TRACE("Received buffer which length[%d]", iLength);
        En_HP_FetchResult result = HP_TcpPullClient_Fetch(pSender, (BYTE*)buffer, (int)iLength);
        if(result == FR_OK)
        {
            UT_TRACE("HP_TcpPullClient_Fetch %d bytes successfully.\n", iLength);
            UT_DUMP(buffer, iLength);
        }

        CClientConnManager::GetInstance()->ReceivePkt(dwConnID,(BYTE*)buffer,iLength);

        //CClientConn::GetInstance()->ProcessMsg((BYTE*)buffer,iLength);
        //memcpy(OnenetEdpRxBuf, buffer, iLength);
        //dsmapp_srv_recv_handle(OnenetEdpRxBuf, iLength);

//        if (buffer){
//            free(buffer);
//        }

        return HR_OK;
    }

    En_HP_HandleResult __HP_CALL OnSend(HP_Client pSender, HP_CONNID dwConnID, const BYTE* pData, int iLength)
    {
        //PostOnSend(dwConnID, pData, iLength);
        return HR_OK;
    }

    En_HP_HandleResult __HP_CALL OnClose(HP_Client pSender, HP_CONNID dwConnID, En_HP_SocketOperation enOperation, int iErrorCode)
    {
        // iErrorCode == SE_OK ? PostOnClose(dwConnID) :
        //  PostOnError(dwConnID, enOperation, iErrorCode);
        CClientConnManager::GetInstance()->UpdateConnStatus(dwConnID,NET_DISCONNECTED);


        return HR_OK;
    }

    int StartTcpClient(HP_TcpPullClient& refClient,const char *svr_ip, unsigned short int port)
    {
        UT_TRACE("StartTcpClient server ip addr %s port %d ", svr_ip, port);
        ut_uint16 conn_fd = 0;
        if(HP_Client_Start(refClient, svr_ip, port, 0)){
            UT_TRACE("Start serverip ....ok.");
            /*  connected session maintain --> ( one client connection ==  one  connect fd )  */
            conn_fd = (WORD)HP_Client_GetConnectionID(refClient);
			UT_TRACE("Connect successful,peer handle[%d]",conn_fd);
            /* notice the event that DSM client has been connected  */
            //dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_CONNECTED);
            return conn_fd;
        }
        else {
			UT_ERROR("Start server ip failed errno: %d , error desc %s\n",
                     HP_Client_GetLastError(refClient), HP_Client_GetLastErrorDesc(refClient));
            return -1;
        }
    }

    int StopTcpClient(HP_TcpPullClient& refClient)
    {
        if(HP_Client_Stop(refClient)){

            ut_uint16 conn_fd = 0;
            conn_fd = (ut_uint16)HP_Client_GetConnectionID(refClient);

            UT_TRACE("Shutdown the connect socketFD %d !\n", conn_fd);
            //dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_PIPE_CLOSED);

            return 1;
        }
        else {
            UT_ERROR("Stop Tcp Client error :%d %s \n", HP_Client_GetLastError(refClient), HP_Client_GetLastErrorDesc(refClient));

            return -1;
        }
    }

    bool CreateHPSocketObjects(HP_TcpPullClientListener& refListener,HP_TcpPullClient& refClient)
    {
        refListener      = Create_HP_TcpPullClientListener();
        refClient        = Create_HP_TcpPullClient(refListener);
        HP_Set_FN_Client_OnConnect(refListener, OnConnect);
        HP_Set_FN_Client_OnSend(refListener, OnSend);
        HP_Set_FN_Client_OnPullReceive(refListener, OnReceive);
        HP_Set_FN_Client_OnClose(refListener, OnClose);

        HP_Client_SetExtra(refClient, &s_pkgInfo);
        HP_TcpClient_SetKeepAliveTime(refClient, 0 );

        return true;
    }


    void DestroyHPSocketObjects(HP_TcpPullClientListener& refListener,HP_TcpPullClient& refClient)
    {
        Destroy_HP_TcpPullClient(refClient);
        Destroy_HP_TcpPullClientListener(refListener);
    }

//    void OnCmdStart(void)
//    {
//        printf("OnCmdStart ....\n");
//        ut_uint16 conn_fd = 0;
//        if(HP_Client_Start(s_client, "106.14.186.44", 7000, 0)){
//            UT_TRACE("Start serverip ....ok\n");
//            //dsmapp_srv_heart_set(20);
//            conn_fd = (ut_uint16)HP_Client_GetConnectionID(s_client);
//        }
//        else
//            UT_ERROR("start server ip failed errno: %d , error desc %s\n", HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));
//    }
//
//    void OnCmdStop(void)
//    {
//        if(HP_Client_Stop(s_client)){
//
//            ut_uint16 conn_fd = 0;
//            conn_fd = (ut_uint16)HP_Client_GetConnectionID(s_client);
//            UT_TRACE("Shutdowning the connect socket fd %d..........\n", conn_fd);
//            //dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_PIPE_CLOSED);
//        }
//        else
//            ; //LogClientStopFail(HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));
//    }
//
//    void OnCmdStatus(void)
//    {
//        //  pParser->PrintStatus(HP_Client_GetState(s_client));
//    }

    int  ClientSend(HP_TcpPullClient& refClient,const BYTE* buff, size_t nBuffLen)
    {
        if (NULL == buff || nBuffLen <= 0) {
            UT_FATAL("Parameter buff or nBuffLen[%zu] is Error!",nBuffLen);
            return -1;
        }

        //UT_DUMP(buff,nBuffLen);
        if(HP_Client_Send(refClient, buff,nBuffLen)){
            UT_TRACE("send ok %lu\n", nBuffLen);
        }
        else{
            UT_ERROR("send failed %lu, %d %s\n", HP_Client_GetConnectionID(refClient), SYS_GetLastError(), SYS_GetLastErrorStr());
            return -1;
        }
        return 1;
    }

//    void OnCmdSend(void)
//    {
//        //kal_uint8 buff[] = { 0x7E, 0x81, 0x03, 0x00, 0x13, 0x01, 0x20, 0x00, 0x18, 0x71, 0x48, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x01, 0x3C, 0x00, 0x00, 0x09, 0x05, 0x04, 0x00, 0x00, 0x01, 0x0F, 0xAC, 0x7E };//����������0x0001��Ϊ316s��DWORD�������ö�λ�����0x0095��Ϊ271s��DWORD��
//        const unsigned char buff[] = {0x7E, 0x01,0x00,0x00,0x2F,0x09,0x00,0x00,0x00,0x00,0x74,0x00,0x15,0x00,0x00,0x00,0x00,0x37,0x30,0x31,0x30,0x37,
//                                      0x48,0x42,0x2D,0x44,0x56,0x30,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x31,0x30,0x34,0x30,0x35,0x38,0x02,0xBE,0xA9,0x42,0x31,0x32,0x33,0x34,
//                                      0x35,0x36,0x37,0x1A,0x7E};
//
//        const char *svr_ip = "47.101.52.88";
//        int srv_port = 20048;
//
//
//        if(HP_Client_Start(s_client,svr_ip , srv_port, 0)){
//            // if(HP_Client_Start(s_client, "127.0.0.1", 1234, 0))
//            printf("start serverip %s(port %d)...ok\n", svr_ip, srv_port); //;;LogClientStart("127.0.0.1", 8080);
//            printf(" message ---   success  start !!");
//        }
//        else
//        {
//
//            printf("start server ip failed errno: %d , error desc %s\n", HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));
//            printf(" message ---  faild  start !!");
//            return;
//
//        }
//        printf("OnCmdSend : %lu\n", sizeof(buff));
//        UT_DUMP(buff,sizeof (buff ));
//
//        if(HP_Client_Send(s_client, buff, (int)sizeof(buff))){
//            printf(" message --- send %d",(int)sizeof(buff));
//
//            printf("send ok %lu\n", sizeof(buff));//               LogSend(HP_Client_GetConnectionID(s_client), pParser->m_strMessage);
//        }
//        else{
//            printf(" message ---   faild  send !!");
//
//            printf("send failed %lu, %d %s\n", HP_Client_GetConnectionID(s_client), SYS_GetLastError(), SYS_GetLastErrorStr());
//        }
//    }

//}
