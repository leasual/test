#include <stdio.h>
#include <stdlib.h>
#include  <stdbool.h>
#include <iostream>
#include <signal.h>
#include <time.h>



#include "hpsocket/HPSocket4C.h"
#include "dsmapp_srv_jtt.h"
#include "others.h"
#include "dsm_transmit_jtt.h"
#include <android/log.h>

//#define LOG_TAG "OpenCV-NDK-Native"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#define LOGE printf
HP_TcpPullClientListener s_listener;
HP_TcpPullClient s_client;

struct TPkgInfo{
	bool is_header;
	int length;	
} s_pkgInfo;




kal_uint16 dsm_tcp_write(kal_int8 handle_fd, kal_uint8 *dat_in, kal_uint16 in_len)
{
	
	 if(HP_Client_Send(s_client, dat_in, in_len)){	 
		
 		succ("dsm_tcp_write has been sent %d bytes following data to server:\n", in_len);

		dsm_log(dat_in, in_len);

		return in_len;
	 }
	 else {
     		err("Send failed(dsm_tcp_write) %d, %d %s\n", HP_Client_GetConnectionID(s_client), SYS_GetLastError(), SYS_GetLastErrorStr());

		return 0;
    }

}

kal_uint16 dsm_tcp_read(kal_int8 handle_fd, kal_uint8 *dat_in, kal_uint16 in_len)
{
	
	return 0;
}


kal_uint16 dsm_tcp_connect(const char *ip_addr, int ip_port, int apn/*, sock_event_func func*/){

	kal_uint16 conn_fd = -1;

	
	int service_state = HP_Client_GetState(s_client);
	info("service_state %d\n", service_state);
	if ( service_state == 0 || service_state == 1 ){
		printf("tcp server has been connected\n");
		 if(HP_Client_Stop(s_client))
		 	printf("close connect first\n");	
	}
	
	info("connect Serverip %s port %d\n", ip_addr, ip_port);
	if(HP_Client_Start(s_client, ip_addr, ip_port, 0)) {
           
			conn_fd = (kal_uint16)HP_Client_GetConnectionID(s_client);

			 succ("Start ServerIP %s Port %d (connectfd %d) successfully !\n", ip_addr, ip_port, conn_fd);
			
			dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_CONNECTED);
			
	 }
     else  
			err("start ServerIP %s port %d failed errno: %d , error desc %s\n", ip_addr, ip_port, HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));



	 return conn_fd;	

}






En_HP_HandleResult __HP_CALL OnConnect(HP_Client pSender, HP_CONNID dwConnID)
{
        TCHAR szAddress[50];
        int iAddressLen = sizeof(szAddress) / sizeof(TCHAR);
        USHORT usPort;

        HP_Client_GetRemoteHost(pSender, szAddress, &iAddressLen, &usPort);

        return HR_OK;
}



extern kal_uint8 OnenetEdpRxBuf[256] ;



En_HP_HandleResult __HP_CALL OnReceive(HP_Client pSender, HP_CONNID dwConnID, int iLength)
{
	struct TPkgInfo * pInfo = (struct TPkgInfo*)HP_Client_GetExtra(pSender);
	if (pInfo == NULL ){
		printf("Null packet Error\n");
		return HR_OK;
	}

	char *buffer = (char *)malloc(iLength);
	memset(buffer, 0, iLength);

	printf("enter OnReceive(Len=%d)\n", iLength);

	En_HP_FetchResult result = HP_TcpPullClient_Fetch(pSender, (BYTE*)buffer, (int)iLength);

	if(result == FR_OK)
	{
		printf("HP_TcpPullClient_Fetch %x bytes successfully.\n", iLength);
		dsm_log((unsigned char*)buffer, iLength);
	}

	memcpy(OnenetEdpRxBuf, buffer, iLength);

	dsmapp_srv_recv_handle(OnenetEdpRxBuf, iLength);

	
	if (buffer){
		free(buffer);
	}

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

        return HR_OK;
}


int HandleTcpClientEvent(int warn_type, int file_type, const char *path) {
	return dsm_srv_loc_report_jtt_extern(warn_type, file_type, path);
}

int  StartTcpClient(const char *svr_ip, unsigned short int port)
{
	info("StartTcpClient server ip addr %s port %d ...\n", svr_ip, port);
	

	kal_uint16 conn_fd = 0;
    if(HP_Client_Start(s_client, svr_ip, port, 0)){
		succ("Start serverip ....ok\n");
        LOGE(" message ---   success  start !!");
		/* set heart break time, default 20 second now */
		dsmapp_srv_heart_set(20);

		/*  connected session maintain --> ( one client connection ==  one  connect fd )  */
	  	conn_fd = (kal_uint16)HP_Client_GetConnectionID(s_client);

		/* notice the event that DSM client has been connected  */
		dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_CONNECTED);

		return 1;
    }
	else {
		err("start server ip failed errno: %d , error desc %s\n", HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));

		return -1;
	}

}




int StopTcpClient(void){
	if(HP_Client_Stop(s_client)){
		  
		  kal_uint16 conn_fd = 0;
		  conn_fd = (kal_uint16)HP_Client_GetConnectionID(s_client);
		  
		  warn("Shutdown the connect socketFD %d !\n", conn_fd);
		  dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_PIPE_CLOSED);

		  return 1;
	  }
	  else {
		  err("Stop Tcp Client error :%d %s \n", HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));

		  return -1;

	  }

}



//TPkgInfo s_pkgInfo;

void OnCmdStart(void)
{
	printf("OnCmdStart iiii....\n");
	

	kal_uint16 conn_fd = 0;
       if(HP_Client_Start(s_client, "106.14.186.44", 7000, 0)){
     //    if(HP_Client_Start(s_client, "112.64.117.214", 20001, 0)){
      // if(HP_Client_Start(s_client, "127.0.0.1", 1234, 0))

	  	succ("Start serverip ....ok\n"); 

	  dsmapp_srv_heart_set(20);

	  	conn_fd = (kal_uint16)HP_Client_GetConnectionID(s_client);

		dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_CONNECTED);
    }
	else
		err("start server ip failed errno: %d , error desc %s\n", HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));
}

void OnCmdStop(void)
{
        if(HP_Client_Stop(s_client)){
			
			kal_uint16 conn_fd = 0;
			conn_fd = (kal_uint16)HP_Client_GetConnectionID(s_client);
			warn("Shutdowning the connect socket fd %d..........\n", conn_fd);
			dsmapp_srv_ind(conn_fd, DSM_TCP_EVT_PIPE_CLOSED);
        }
        else
               ; //LogClientStopFail(HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));
}

void OnCmdStatus(void)
{
      //  pParser->PrintStatus(HP_Client_GetState(s_client));
}

void OnCmdSend(void)
{
	//kal_uint8 buff[] = { 0x7E, 0x81, 0x03, 0x00, 0x13, 0x01, 0x20, 0x00, 0x18, 0x71, 0x48, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x01, 0x3C, 0x00, 0x00, 0x09, 0x05, 0x04, 0x00, 0x00, 0x01, 0x0F, 0xAC, 0x7E };//����������0x0001��Ϊ316s��DWORD�������ö�λ�����0x0095��Ϊ271s��DWORD��
	const unsigned char buff[] = {0x7E, 0x01,0x00,0x00,0x2F,0x09,0x00,0x00,0x00,0x00,0x74,0x00,0x15,0x00,0x00,0x00,0x00,0x37,0x30,0x31,0x30,0x37,
	0x48,0x42,0x2D,0x44,0x56,0x30,0x36,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x31,0x30,0x34,0x30,0x35,0x38,0x02,0xBE,0xA9,0x42,0x31,0x32,0x33,0x34,
	0x35,0x36,0x37,0x1A,0x7E};

	const char *svr_ip = "106.14.186.44";
	int srv_port = 7000;


	if(HP_Client_Start(s_client,svr_ip , srv_port, 0)){
      // if(HP_Client_Start(s_client, "127.0.0.1", 1234, 0))
      printf("start serverip %s(port %d)...ok\n", svr_ip, srv_port); //;;LogClientStart("127.0.0.1", 8080);
        LOGE(" message ---   success  start !!");
	}
	else
	{
		
		printf("start server ip failed errno: %d , error desc %s\n", HP_Client_GetLastError(s_client), HP_Client_GetLastErrorDesc(s_client));
        LOGE(" message ---  faild  start !!");
		return;

	}
	printf("OnCmdSend : %d\n", sizeof(buff));
	dsm_log((unsigned char*)buff,sizeof (buff ));

    if(HP_Client_Send(s_client, buff, (int)sizeof(buff))){
        LOGE(" message --- send %d",(int)sizeof(buff));

        printf("send ok %d\n", sizeof(buff));//               LogSend(HP_Client_GetConnectionID(s_client), pParser->m_strMessage);
    }
    else{
        LOGE(" message ---   faild  send !!");

        printf("send failed %d, %d %s\n", HP_Client_GetConnectionID(s_client), SYS_GetLastError(), SYS_GetLastErrorStr());
    }
}




bool CreateHPSocketObjects()
{
        // ��������������
        s_listener      = Create_HP_TcpPullClientListener();
        // ���� Socket ����
        s_client        = Create_HP_TcpPullClient(s_listener);

        // ���� Socket �������ص�����
        HP_Set_FN_Client_OnConnect(s_listener, OnConnect);
        HP_Set_FN_Client_OnSend(s_listener, OnSend);
        HP_Set_FN_Client_OnPullReceive(s_listener, OnReceive);
        HP_Set_FN_Client_OnClose(s_listener, OnClose);

	    HP_Client_SetExtra(s_client, &s_pkgInfo);
        HP_TcpClient_SetKeepAliveTime(s_client, 0 );

	return true;
}


void DestroyHPSocketObjects()
{
        // ���� Socket ����
        Destroy_HP_TcpPullClient(s_client);
        // ���ټ���������
        Destroy_HP_TcpPullClientListener(s_listener);
}






#define PRINTLN printf






#if 0
int main(void)
{
	HP_TcpPullClientListener s_listener;
	HP_TcpPullClient s_client;

	// 1. Create listener object
	//s_listener	= ::Create_HP_TcpPullAgentListener();
	s_listener	= Create_HP_TcpPullClientListener();
	// 2. Create component object (and binding with listener object)
	s_client	= Create_HP_TcpPullClient(s_listener);
	
	/* Set listener callbacks */
	HP_Set_FN_Client_OnConnect(s_listener, OnConnect);
	HP_Set_FN_Agent_OnSend(s_listener, OnSend);
	HP_Set_FN_Agent_OnPullReceive(s_listener, OnReceive);
	HP_Set_FN_Agent_OnClose(s_listener, OnClose);
	//HP_Set_FN_Agent_OnShutdown(s_listener, OnShutdown);
	
	// 3. Start component object
	if(HP_Agent_HasStarted(s_client))
		exit(1);
	
	// 4. Connect to dest host
	HP_Agent_Connect(s_client, "remote.host.1", 8080, NULL);
//	HP_Agent_Connect(s_agent, "remote.host.2", REMOTE_PORT_2, nullptr);
//	HP_Agent_Connect(s_agent, "remote.host.3", REMOTE_PORT_3, nullptr);
	
	/* wait for exit */
	// ... ... 
	
	// 6. (optional) Stop component object
	HP_Agent_Stop(s_client);

	// 7. Destroy component object
	Destroy_HP_TcpPullAgent(s_client);
	// 8. Destroy listener object
	Destroy_HP_TcpPullClientListener(s_listener);
	
	return 0;
}
#endif
