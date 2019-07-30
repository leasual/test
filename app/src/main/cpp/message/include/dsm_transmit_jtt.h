#ifndef DSM_TRANSMIT_JTT_HDR__
#define DSM_TRANSMIT_JTT_HDR__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iostream>
#include <signal.h>
#include <time.h>
//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/function.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
#include "VS_kal_general_types.h"
#include "hpsocket/HPSocket4C.h"

struct TPkgHeader 
{
	DWORD seq;
	int body_len;
};

struct TPkgBody 
{
	char name[30];
	short age;
	char desc[1];
};


#define YOU_BANG_SERVER_IP "106.14.186.44"
#define YOU_BANG_SERVER_PORT  7000

//void dsmapp_srv_recv_handle(kal_uint8 *dat_in, kal_uint16 in_len);
kal_uint16 dsm_tcp_write(kal_int8 handle_fd, kal_uint8 *dat_in, kal_uint16 in_len);
kal_uint16 dsm_tcp_read(kal_int8 handle_fd, kal_uint8 *dat_in, kal_uint16 in_len);
kal_uint16 dsm_tcp_connect(const char *ip_addr, int ip_port, int apn);
En_HP_HandleResult __HP_CALL OnConnect(HP_Client pSender, HP_CONNID dwConnID);
En_HP_HandleResult __HP_CALL OnReceive(HP_Client pSender, HP_CONNID dwConnID, int iLength);
En_HP_HandleResult __HP_CALL OnSend(HP_Client pSender, HP_CONNID dwConnID, const BYTE* pData, int iLength);
En_HP_HandleResult __HP_CALL OnClose(HP_Client pSender, HP_CONNID dwConnID, En_HP_SocketOperation enOperation, int iErrorCode);
void OnCmdStart(void);
void OnCmdStop(void);
void OnCmdStatus(void);
void OnCmdSend(void);
bool CreateHPSocketObjects();
void DestroyHPSocketObjects();

int  StartTcpClient(const char *svr_ip, unsigned short int port);
int  StopTcpClient(void);
int HandleTcpClientEvent(int warn_type, int file_type, const char *path);
kal_int32 dsm_srv_loc_report_jtt_extern(int warn_type, int file_type, const char *buf);

#endif

