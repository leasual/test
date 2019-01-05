
#if 1
#ifndef __DSMAPP_SRVINTERACTION_JTT_H__
#define __DSMAPP_SRVINTERACTION_JTT_H__

#include "others.h"

typedef enum DSM_SRV_CMD_TYPE_
{
	DSMAPP_SRV_JTT_CMD_UP_ACK = 0x0001, // �ն�ͨ��Ӧ��
	DSMAPP_SRV_JTT_CMD_UP_HEARTBEAT = 0x0002, // �ն�����
	DSMAPP_SRV_JTT_CMD_UP_REGISTER = 0x0100, // �ն�ע��
	DSMAPP_SRV_JTT_CMD_UP_DEREGISTER = 0x0003, // �ն�ע��
	DSMAPP_SRV_JTT_CMD_UP_AUTHORIZE = 0x0102, // �ն˼�Ȩ
	DSMAPP_SRV_JTT_CMD_UP_QUERY_PARA_ACK = 0x0104, // ��ѯ�ն˲���Ӧ��
	DSMAPP_SRV_JTT_CMD_UP_QUERY_PROP_ACK = 0x0107, // ��ѯ�ն�����Ӧ��
	DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT = 0x0200, // λ����Ϣ�㱨
	DSMAPP_SRV_JTT_CMD_UP_LOC_QUERY_ACK = 0x0201, // λ����Ϣ��ѯӦ��
	DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT_EXTERN = 0x0202,
	
	DSMAPP_SRV_JTT_CMD_UP_EVENT_REPORT = 0x0301, // �¼�����
	
	DSMAPP_SRV_JTT_CMD_UP_BATTERY = 0x0F01, // ������Ϣ
	DSMAPP_SRV_JTT_CMD_UP_CARD_LOGIN = 0x0F02, // ��Ƭ����

	DSMAPP_SRV_JTT_CMD_DOWN_ACK = 0x8001, // ƽ̨ͨ��Ӧ��
	DSMAPP_SRV_JTT_CMD_DOWN_REGISTER_ACK = 0x8100, // �ն�ע��Ӧ��
	DSMAPP_SRV_JTT_CMD_DOWN_SET_PARA = 0x8103, // �����ն˲���
	DSMAPP_SRV_JTT_CMD_DOWN_QUERY_PARA = 0x8104, // ��ѯ�ն˲���
	DSMAPP_SRV_JTT_CMD_DOWN_CONTROL = 0x8105, // �ն˿���
	DSMAPP_SRV_JTT_CMD_DOWN_QUERY_SPEC_PARA = 0x8106, // ��ѯָ���ն˲���
	DSMAPP_SRV_JTT_CMD_DOWN_QUERY_PROP = 0x8107, // ��ѯ�ն�����
	DSMAPP_SRV_JTT_CMD_DOWN_LOC_QUERY = 0x8201, // λ����Ϣ��ѯ
	DSMAPP_SRV_JTT_CMD_DOWN_EVENT_SET = 0x8301, // �¼�����
#ifdef __WHMX_CALL_SUPPORT__
	DSMAPP_SRV_JTT_CMD_DOWN_CALL_MONITOR = 0x8400, // �绰�ز�
	DSMAPP_SRV_JTT_CMD_DOWN_PHONEBOOK = 0x8401, // ���õ绰��
#endif
#ifdef __WHMX_JTT_FENCE_SUPPORT__
#endif

	DSMAPP_SRV_JTT_CMD_INVALID = 0xFFFF
}DSM_SRV_JTT_CMD_TYPE;

typedef struct
{
	DSM_SRV_JTT_CMD_TYPE cmd;
	kal_int32(*handle)(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out);
}dsm_cmd_handle_struct;


//�ն˲���: ����ID
typedef enum _SRV_PARA_ID_
{
	DSMAPP_SRV_JTT_PARA_HEARTBEAT = 0x0001,
	DSMAPP_SRV_JTT_PARA_SRV_IP = 0x0013,
	DSMAPP_SRV_JTT_PARA_SRV_TCP_PORT = 0x0018,
	DSMAPP_SRV_JTT_PARA_MONITOR_NUM = 0x0048,
	DSMAPP_SRV_JTT_PARA_LOC_TYPE = 0x0094,
	DSMAPP_SRV_JTT_PARA_LOC_PROP = 0x0095,

	/*�Զ���*/
	DSMAPP_SRV_JTT_PARA_BATTERY = 0xF001,

	DSMAPP_SRV_JTT_PARA_INVALID = 0x0000
}SRV_PARA_ID;

//�����ն˲���: ����ID-������
typedef struct
{
	SRV_PARA_ID para_id;
	kal_int32(*handle)(kal_uint8 *in, kal_uint32 in_len);
}dsm_set_para_handle_struct;

//��ѯ�ն˲���: ����ID-������
typedef struct
{
	SRV_PARA_ID para_id;
	kal_int32(*handle)(kal_uint8 *out/*, kal_uint32 out_len*/);
}dsm_get_para_handle_struct;

// �ն���������
typedef struct
{
	kal_uint8 status; // ��ʼ:0 ��Ȩ��:1 ע����:2 ע����:3 ��Ȩ�ɹ�:4
	kal_uint16 flow;
}dsm_login_status;

typedef void(*dsm_srv_cb)(void );


// new: �¼�ID
typedef enum _SRV_EVENT_ID_
{
	DSMAPP_SRV_JTT_EVENT_LOW_POWER = 0xF1, // �͵�
	DSMAPP_SRV_JTT_EVENT_NO_POWER = 0xF2, // �޵�
	DSMAPP_SRV_JTT_EVENT_INVALID = 0xFF
}SRV_EVENT_ID;


/*�½ӿ�*/
kal_int32 dsm_srv_receive_handle_jtt(kal_uint8 src, kal_uint8 *in, kal_int32 in_len);
kal_int32 dsm_srv_heartbeat_jtt(void);
kal_int32 dsm_srv_loc_report_jtt(void);
kal_int32 dsm_srv_ack_jtt(kal_uint8 *para, kal_uint32 para_len);
kal_int32 dsm_srv_send_battery_jtt(kal_bool poweroff);
kal_int32 dsm_srv_send_card_login_jtt(void);
kal_int32 dsm_srv_config_nv_write(kal_uint8 item);
kal_int32 dsm_srv_config_nv_read(void);
void dsm_srv_auth_code_clear(void);
kal_int32 dsmapp_srvinteraction_send_event(SRV_EVENT_ID evt);


/*ԭ�ӿ�*/
void dsmapp_srvinteraction_connect(kal_int32 s32Level);
void srvinteraction_bootup_location_request(void);
void dsmapp_srvinteraction_first_location(void);

kal_int32 dsmapp_srvinteraction_uploader_pos_mode(void);
kal_int32 dsmapp_srvinteraction_uploader_config(void);
kal_int32 dsmapp_srvinteraction_uploader_batt_info(void);
kal_int32 dsmapp_srvinteraction_send_battery_warning(void); 

kal_uint8 dsm_pos_mode_set(kal_uint8 mode);
kal_uint8 dsm_pos_period_set(kal_uint32 period_min);

void dsmapp_srvinteraction_sos(void);

void dsmapp_srvinteraction_locate_and_poweroff(void); // 5%�͵�ػ�

kal_bool dsmapp_srvinteraction_if_connected(void); // 2016-6-22
kal_bool dsmapp_srvinteraction_is_connected(void);

kal_int32 dsm_srv_loc_report_jtt_extern(int warn_type, int file_type, const char *buf);


#if defined(__WHMX_CALL_SUPPORT__)
extern kal_uint8 dsm_srv_cmd_location_status_get(void);
extern void dsm_srv_cmd_location_status_set(kal_uint8 ret);
#endif


#endif
#endif

