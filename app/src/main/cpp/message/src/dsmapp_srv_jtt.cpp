#if 1

#include <VS_kal_general_types.h>
#include "dsmapp_srv_jtt.h"
#include "dsmapp_srvinteraction_jtt.h"

#define DSMAPP_SRV_BUFF_MAX_RX 256
#define DSMAPP_SRV_BUFF_MAX_TX 448




int need_register = 0;


#define DSMAPP_SRV_DEBUG_TXRX 1
#if DSMAPP_SRV_DEBUG_TXRX
static kal_uint8 edp_dbg_buf[DSMAPP_SRV_BUFF_MAX_TX * 3];
#endif

#define HTTP_DEBUG 1
#if HTTP_DEBUG
#define http_trace(...)  dsmapp_trace(__VA_ARGS__)
#else
#define http_trace(...)
#endif

#define FUNC_ENTER http_trace("%s enter.\r\n",__FUNCTION__)
#define FUNC_LEAVE http_trace("%s leave.\r\n",__FUNCTION__)

#define DSMAPP_SRV_RECON_TIMEOUT	(2000)	/*unit:ms*/
#define DSMAPP_SRV_CONN_TIMEOUT	(90000)	/*unit:ms*/
#define MODE_FLIGHT_TO_NORMAL_INTERVAL	(30000)	/*unit:ms*/
#define DSMAPP_SRV_RECON_COUNT_MAX	5	//10

kal_uint8 OnenetEdpRxBuf[DSMAPP_SRV_BUFF_MAX_RX] = { 0 };
//static kal_uint8 OnenetEdpTxBuf[DSMAPP_SRV_BUFF_MAX_TX] = { 0 };

static kal_int32 dsmapp_srv_hdl = -1;
static kal_uint8 dsmapp_srv_recon_cnt = 0;
static kal_uint8 dsmapp_srv_con_state = 0;//0,1,2
static kal_uint8 dsmapp_srv_con_login = 0;//0,1,2
static kal_uint8 dsmapp_srv_send_err = 0;
static kal_uint8 dsmapp_srv_con_req_cnt = 0;

extern nvram_ef_dsmapp_jtt_config_t jtt_config;

void dsmapp_srv_reconnect(void );
void dsmapp_srv_disconnect(void );
void dsmapp_srv_heart(void *);

//extern void mmi_flight_mode_switch_for_mx(U8 select_mode);

void dsmapp_srv_address_set(kal_char * ip, kal_uint16 port)
{	
	if (ip)
	{
		kal_mem_cpy(&jtt_config.srv_ip[0], ip, 16);
	}
	
	if (port != 0)
	{
		jtt_config.srv_port = port;
	}
	
	dsmapp_trace("%s: %s %d", __FUNCTION__, jtt_config.srv_ip, jtt_config.srv_port);

	dsm_srv_auth_code_clear(); //
	dsmapp_srv_reconnect(); //
	dsm_srv_config_nv_write(2);
	return;
}

void dsmapp_srv_address_get(kal_char * ip, kal_uint16 * port)
{
	kal_mem_cpy(ip, &jtt_config.srv_ip[0], 16);
	*port = jtt_config.srv_port;
	dsmapp_trace("%s: %s %d", __FUNCTION__, ip, *port);
}

static kal_bool dsmapp_srv_call_connected(void)
{
#if (DEBUG_IN_VS == 0)
	if(srv_ucm_query_call_count(SRV_UCM_CALL_STATE_ALL, SRV_UCM_CALL_TYPE_ALL, NULL) > 0)
	{
		return KAL_TRUE;
	}
#endif

	if(dsmapp_srv_hdl)
		return KAL_TRUE;
	
	return KAL_FALSE;
}


void dsmapp_srv_recv_handle(kal_uint8 *dat_in, kal_uint16 in_len)
{
	kal_int32 ret = -1;
	kal_uint8 cmd_t = 0;
	kal_uint8 *p_head = NULL;
	kal_uint8 *p_tail = NULL;
	kal_uint16 msg_len = 0;
	kal_uint16 i = 0;

	FUNC_ENTER;

#if DSMAPP_SRV_DEBUG_TXRX
	{
		kal_uint8 *log_buf = edp_dbg_buf, *log_buf_h = edp_dbg_buf;
		kal_uint32 i;

		kal_mem_set(log_buf, 0, sizeof(edp_dbg_buf));
		for (i = 0; i < ((in_len < DSMAPP_SRV_BUFF_MAX_TX) ? in_len : DSMAPP_SRV_BUFF_MAX_TX); i++)
		{
#if (DEBUG_IN_VS == 1)
			kal_sprintf((char*)log_buf, "%02x ", dat_in[i]);
#else
			kal_sprintf((char*)log_buf, "%x ", dat_in[i]);
#endif
			log_buf += strlen((const char*)log_buf);
		}
		for (i = 0; i < strlen((const char*)log_buf_h);)
		{
			dsmapp_trace("%s\r\n", (log_buf_h + i));
			i += 127;
		}
	}
#endif

	if (dat_in)
	{
		while (i < in_len)
		{
			/*���ұ�ʶλ*/
			while ((i < in_len) && ((p_head == NULL) || (p_tail == NULL)))
			{
				if (dat_in[i] == 0x7e)
				{
					if (p_head == NULL)
					{
						p_head = dat_in + i;
					}
					else if (p_tail == NULL)
					{
						p_tail = dat_in + i;
						msg_len = p_tail - p_head + 1;
					}
				}
				i++;
			}

			printf("msg offset, len %d\n", msg_len);
		
		//	dsmapp_trace("msg: offset=%d, len=%d", (p_head - dat_in), msg_len);
			if (p_head && msg_len)
			{
				ret = dsm_srv_receive_handle_jtt(0, p_head, msg_len);
			}
			p_head = NULL;
			p_tail = NULL;
			msg_len = 0;
		}
	}

//	dsmapp_srv_heart();

	dsmapp_trace("%s leave (ret=%d)[%x](len=%d)\r\n", __FUNCTION__, ret, cmd_t, /*dat_in ? (*dat_in) : 0, */in_len);
}

void dsmapp_srv_ind(kal_int32 hdl, kal_uint32 evt)
{
	kal_int32 s32Ret = 0;
	kal_int32 len_real = 0;
#if defined(__WHDSM_LOG_SRV_SUPPORT__)
	kal_char *head = NULL;
	kal_char *pbuf = NULL;
#endif

	dsmapp_trace("%s (%d)(%d)\r\n", __FUNCTION__, hdl, evt);

#if defined(__WHDSM_LOG_SRV_SUPPORT__)
	{
		kal_char *ind_info[7] = { "unknow", "connected", "write", "read", "broken", "host not found", "pipe closed" };

		head = dsm_log_srv_write_buf();
		if (head)
		{
			pbuf = head;
			kal_sprintf(pbuf, "%s handle = (%d,%d), event = %d.%s", __FUNCTION__, dsmapp_srv_hdl, hdl, evt, ind_info[(evt > 6) ? 0 : evt]);
			//			dsm_log_srv_write(head,strlen(head),KAL_TRUE);
		}
	}
#endif

	printf("hdl %d dsmapp_srv_hdl %d\n", hdl, dsmapp_srv_hdl);

	if (hdl != dsmapp_srv_hdl)
	{
#if defined(__WHDSM_LOG_SRV_SUPPORT__)
		if (head)
		{
			dsm_log_srv_write(head, strlen(head), KAL_TRUE);
			dsmapp_trace("%s", head);
		}
#endif

		succ("repalce server handle %d with new handle %d\n", dsmapp_srv_hdl, hdl);
		dsmapp_srv_hdl = hdl;
		//return;
	}
#if defined(__WHDSM_LOG_SRV_SUPPORT__)
	else
	{
		if (evt != DSM_TCP_EVT_CAN_READ)
		{
			if (head)
			{
				dsm_log_srv_write(head, strlen(head), KAL_TRUE);
				dsmapp_trace("%s", head);
			}
		}
	}
#endif

	switch (evt)
	{
	case DSM_TCP_EVT_CONNECTED:
		StopTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE);
		dsmapp_srv_recon_cnt = 0;
		dsmapp_srv_con_state = 2;
		printf("connected dsmapp_srv_con_state : %d \n",dsmapp_srv_con_state);
		dsmapp_srvinteraction_first_location(); // ��AGPS
		if (need_register)
			dsmapp_srvinteraction_connect(0);
		else
			dsmapp_srv_heart(NULL); // ��������

			
			/*
	#if (SRV_NO_REGISTER == 1) // ��ע���Ȩ
		dsmapp_srv_heart(NULL); // ��������
	#else
		dsmapp_srvinteraction_connect(0);
	#endif
	*/
		break;

	case DSM_TCP_EVT_CAN_WRITE:
		break;

	case DSM_TCP_EVT_CAN_READ:
		kal_mem_set(OnenetEdpRxBuf, 0, sizeof(OnenetEdpRxBuf));
		do
		{
			s32Ret = dsm_tcp_read(hdl, OnenetEdpRxBuf, sizeof(OnenetEdpRxBuf));
			len_real += s32Ret;
		} while (s32Ret == DSMAPP_SRV_BUFF_MAX_RX);
		if (len_real > DSMAPP_SRV_BUFF_MAX_RX)
		{
#if defined(__WHDSM_LOG_SRV_SUPPORT__)
			if (head)
			{
				pbuf = head + strlen(head);
				kal_sprintf(pbuf, " %d", len_real);
				dsm_log_srv_write(head, strlen(head), KAL_TRUE);
			}
#endif
			break;
		}

#if defined(__WHDSM_LOG_SRV_SUPPORT__)
		if (head)
		{
			pbuf = head + strlen(head);
			kal_sprintf(pbuf, " %d", s32Ret);
			if (s32Ret > 0)
			{
				kal_uint32 i;

				pbuf = head + strlen(head);
				for (i = 0; i<((s32Ret>DSMAPP_SRV_BUFF_MAX_RX) ? DSMAPP_SRV_BUFF_MAX_RX : s32Ret); i++)
				{
					kal_sprintf(pbuf, " %02x", OnenetEdpRxBuf[i]);
					pbuf += 3;
				}
			}
			dsm_log_srv_write(head, strlen(head), KAL_TRUE);
			dsmapp_trace("%s", head);
		}
#endif
		if (s32Ret > 0)
		{
			dsmapp_srv_recv_handle(OnenetEdpRxBuf, s32Ret);
		}
		break;

	case DSM_TCP_EVT_PIPE_BROKEN:
	case DSM_TCP_EVT_HOST_NOT_FOUND:
	case DSM_TCP_EVT_PIPE_CLOSED:
		dsmapp_srv_recon_cnt++;
		//clear auth code first
		dsm_srv_auth_code_clear();
		dsmapp_trace("dsmapp_srv_recon_cnt=%d", dsmapp_srv_recon_cnt);
		if (dsmapp_srv_recon_cnt >= DSMAPP_SRV_RECON_COUNT_MAX)
		{
			dsmapp_srv_recon_cnt = 0;
			dsmapp_srv_disconnect();
			/*****/
			if (KAL_FALSE == dsmapp_srv_call_connected())
			{
				mmi_flight_mode_switch_for_mx(1);/*normal--->flight*/
				StopTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE);
				StartTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE, MODE_FLIGHT_TO_NORMAL_INTERVAL, dsmapp_srv_reconnect);
			}
			else
			{
				dsmapp_srv_reconnect();
			}
			/*****/
		}
		else
		{
			dsmapp_srv_reconnect();
		}
		break;

	default:
		dsmapp_srv_reconnect();
		break;
	}
}




void dsmapp_srv_connect(void )
{
	kal_int32 hdl = dsmapp_srv_hdl;

	FUNC_ENTER;


	if (hdl != -1)
	{
		dsmapp_trace("hdl != -1, return");
		info("hdl != -1, return !\n");
		return;
	}

	/*****/
	mmi_flight_mode_switch_for_mx(0);/*flight--->normal*/
	/*****/

	dsmapp_srv_con_state = 0;
	dsmapp_srv_con_login = 0;

	dsm_srv_config_nv_read(); // read NV (srv ip and port included)

	if (0 /*KAL_TRUE == dsmapp_srv_call_connected()*/)
	{
		hdl = -1;
	}
	else
	{
#if (SRV_USE_NVRAM == 1)
		dsmapp_trace("tcp_connect:%s(%d)\r\n", jtt_config.srv_ip, jtt_config.srv_port);
		hdl = dsm_tcp_connect(jtt_config.srv_ip, jtt_config.srv_port, dsm_net_get_apn(0)/*, dsmapp_srv_ind*/);
#else
		//dsmapp_trace("tcp_connect:%s(%d)\r\n", DSMAPP_SRV_ADDR_IP, DSMAPP_SRV_ADDR_PORT);
		//hdl = dsm_tcp_connect(DSMAPP_SRV_ADDR_IP, DSMAPP_SRV_ADDR_PORT, dsm_net_get_apn(0)/*, dsmapp_srv_ind*/);

		info("tcp_connect:%s(%d)\r\n", jtt_config.srv_ip, jtt_config.srv_port);
		hdl = dsm_tcp_connect(jtt_config.srv_ip, jtt_config.srv_port, dsm_net_get_apn(0)/*, dsmapp_srv_ind*/);
#endif
	}

	info("hdl %d\n", hdl);
	if (hdl < 0)
	{
		dsmapp_srv_reconnect();
	}
	else
	{
		dsmapp_srv_hdl = hdl;
		dsmapp_srv_con_state = 1;

		StopTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE);
		StartTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE, DSMAPP_SRV_CONN_TIMEOUT, dsmapp_srv_reconnect);
	}

	FUNC_LEAVE;
}

void dsmapp_srv_disconnect(void )
{
	FUNC_ENTER;


	StopTimer(DSMAPP_TIMER_JTT_SERVER);	
	StopTimer(DSMAPP_TIMER_INTERACTION_SERVICE);
	StopTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE);
	StopTimer(DSMAPP_TIMER_DISCONNECT_SERVICE);
	StopTimer(DSMAPP_TIMER_RECONNECT_SERVICE);
	StopTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE);		
	StopTimer(DSMAPP_TIMER_JTT_SERVER_AUTHORIZE);
	StopTimer(DSMAPP_TIMER_JTT_SERVER_REGISTER);
	StopTimer(DSMAPP_TIMER_JTT_SERVER_DEREGISTER);
	StopTimer(DSMAPP_TIMER_NET_UPLOAD);
	if (dsmapp_srv_hdl > 0)
	{
		dsm_tcp_close(dsmapp_srv_hdl);
		dsmapp_srv_hdl = -1;
		dsmapp_srv_con_state = 0;
		dsmapp_srv_con_login = 0;
		dsmapp_srv_send_err = 0;
	}
	dsmapp_srv_con_req_cnt = 0;

	FUNC_LEAVE;
}

void dsmapp_srv_reconnect(void)
{
	FUNC_ENTER;

	//alarm("reconnect timer\n");


	dsmapp_srv_disconnect();
	StartTimer(DSMAPP_TIMER_RECONNECT_SERVICE, DSMAPP_SRV_RECON_TIMEOUT, dsmapp_srv_connect);

	FUNC_LEAVE;
}


kal_int32 dsmapp_srv_send(kal_uint8 *dat_in, kal_uint16 in_len, dsm_srv_cb cb)
{
	kal_int32 ret = -1;

	FUNC_ENTER;

	if (dsmapp_srv_con_state == 0)
	{
		dsmapp_srv_connect();
		//return ret;
	}

	warn("dsmapp_srv_con_state %d\n", dsmapp_srv_con_state);

	if ((dsmapp_srv_con_state == 2 || dsmapp_srv_con_state == 1) && (dat_in != NULL) && (in_len > 0))
	{
		ret = dsm_tcp_write(dsmapp_srv_hdl, dat_in, in_len);
		warn("dsmapp_srv_send (%d,%d)\r\n", ret, in_len);
		if (ret != in_len)	//(ret <= 0)
		{
			dsmapp_srv_reconnect();
		}
		else
		{
//#if ((DSMAPP_SRV_HEART_TIMEOUT == 0)&&(DSMAPP_SRV_DISCON_TIMEOUT > 0))
//			StopTimer(DSMAPP_TIMER_DISCONNECT_SERVICE);
//			StartTimer(DSMAPP_TIMER_DISCONNECT_SERVICE, DSMAPP_SRV_DISCON_TIMEOUT, dsmapp_srv_disconnect);
//#endif

#if DSMAPP_SRV_DEBUG_TXRX
			{
				kal_uint8 *log_buf = edp_dbg_buf, *log_buf_h = edp_dbg_buf;
				kal_uint32 i;

				kal_mem_set(log_buf, 0, sizeof(edp_dbg_buf));
				for (i = 0; i < ((in_len < DSMAPP_SRV_BUFF_MAX_TX) ? in_len : DSMAPP_SRV_BUFF_MAX_TX); i++)
				{
					kal_sprintf((char*)log_buf, "%02x ", dat_in[i]);
					log_buf += strlen((const char*)log_buf);
				}
				for (i = 0; i < strlen((const char*)log_buf_h);)
				{
					dsmapp_trace("%s\r\n", (log_buf_h + i));
					i += 127;
				}
			}
#endif
		}
		dsmapp_srv_send_err = (ret == in_len) ? 0 : 1;
	}

	FUNC_LEAVE;

	return ret;
}


#define DSMAPP_SRV_HEART_TIMEOUT_REDUNDANCE	(20)	/*unit:s*/
#define DSMAPP_SRV_HEART_SEND_ACK_TIMEOUT	((5+DSMAPP_SRV_HEART_TIMEOUT_REDUNDANCE)*1000) /*unit:ms*/

// �޸��ն��������ͼ������λΪ�루s��
kal_int8 dsmapp_srv_heart_set(kal_uint16 heart_s)
{
	if(jtt_config.heart == heart_s) // same
	{
		succ("%s: %d same, retur\nn", __FUNCTION__, heart_s);
		dsmapp_trace("%s: %d same, return", __FUNCTION__, heart_s);
	}
	else
	{
		succ("%s: %d same, return\n", __FUNCTION__, heart_s);
		jtt_config.heart = heart_s;
		dsmapp_trace("%s: %d", __FUNCTION__, heart_s);
	}

	dsm_srv_config_nv_write(2);
	return 0;
}

// ��ѯ�ն��������ͼ������λΪ�루s��
kal_uint32 dsmapp_srv_heart_get(void)
{
	return jtt_config.heart;
}

void dsmapp_srv_heart_callback(void )
{

	alarm("heart timer timeout !!!!!!!!!!!!!!!!! \n");
	//alarm("Timer srv heart .....\n");

	if(jtt_config.heart > 0)
	{
		FUNC_ENTER;

		if (KAL_FALSE == dsmapp_srv_call_connected())
		{
			StartTimer(DSMAPP_TIMER_INTERACTION_SERVICE, DSMAPP_SRV_HEART_SEND_ACK_TIMEOUT, dsmapp_srv_reconnect);
			dsmapp_trace("%s: StartTimer %dms", __FUNCTION__, DSMAPP_SRV_HEART_SEND_ACK_TIMEOUT);
			dsm_srv_heartbeat_jtt();
		}
		else
		{

			dsm_srv_heartbeat_jtt();
			StartTimer(DSMAPP_TIMER_INTERACTION_SERVICE, 25000, dsmapp_srv_heart_callback);

			
			
			dsmapp_trace("%s: StartTimer 5000ms", __FUNCTION__);
		}

		FUNC_LEAVE;
	}
}

void dsmapp_srv_heart(void *p)
{
	p  = p;
	if(jtt_config.heart > 0)
	{
		FUNC_ENTER;

		StopTimer(DSMAPP_TIMER_INTERACTION_SERVICE);
		err("handle srv %d\n", dsmapp_srv_hdl);
		if (dsmapp_srv_hdl <= 0){
			
		}
		if (dsmapp_srv_hdl > 0)
		{
			StartTimer(DSMAPP_TIMER_INTERACTION_SERVICE, jtt_config.heart * 1000, dsmapp_srv_heart_callback);
			dsmapp_trace("%s: StartTimer %dms", __FUNCTION__, jtt_config.heart * 1000);
		}

		FUNC_LEAVE;
	}
}

#endif

#if 0 /* (DEBUG_IN_VS == 1) */
void main(void)
{
	char test1[] = { 0x7e, 0x80, 0x01, 0x00, 0x05, 0x08, 0x80, 0x20, 0x40, 0x53, 0x47, 0x00, 0xde, 0x00, 0x98, 0x0f, 0x01, 0x00, 0x30 }; // 0.9
	char test2[] = { 0x7e, 0x80, 0x01, 0x00, 0x05, 0x08, 0x80, 0x20, 0x40, 0x53, 0x47, 0x00, 0xde, 0x00, 0x98, 0x0f, 0x01, 0x00, 0x30, 0x7e }; // 1
	char test3[] = { 0x7e, 0x80, 0x01, 0x00, 0x05, 0x08, 0x80, 0x20, 0x40, 0x53, 0x47, 0x00, 0xde, 0x00, 0x98, 0x0f, 0x01, 0x00, 0x30, 0x7e, 0x7e, 0x80 }; // 1.1
	char test4[] = { 0x7e, 0x80, 0x01, 0x00, 0x05, 0x08, 0x80, 0x20, 0x40, 0x53, 0x47, 0x00, 0xde, 0x00, 0x98, 0x0f, 0x01, 0x00, 0x30, 0x7e, 0x7e, 0x80, 0x01, 0x00, 0x05, 0x08, 0x80, 0x20, 0x40, 0x53, 0x47, 0x00, 0xdf, 0x00, 0x99, 0x02, 0x00, 0x00, 0x3c, 0x7e }; // 2
	
	dsmapp_trace("[test1]0.9", __FUNCTION__);
	dsmapp_srv_recv_handle(test1, sizeof(test1));

	dsmapp_trace("[test2]1", __FUNCTION__);
	dsmapp_srv_recv_handle(test2, sizeof(test2));

	dsmapp_trace("[test3]1.1", __FUNCTION__);
	dsmapp_srv_recv_handle(test3, sizeof(test3));

	dsmapp_trace("[test4]2", __FUNCTION__);
	dsmapp_srv_recv_handle(test4, sizeof(test4));


}
#endif
