#if 1
#include <stdio.h>
#include <stdlib.h>
#include  <stdbool.h>
#include <iostream>
#include <signal.h>
#include <time.h>




//#include "helper.h"
#include "hpsocket/HPSocket4C.h"



//#include "timer.h"
#include "dsmapp_srv_jtt.h"
#include "dsmapp_srvinteraction_jtt.h"






extern kal_uint16 dsm_tcp_write(kal_int8 handle_fd, kal_uint8 *dat_in, kal_uint16 in_len);


#define IGNORE_ALARM_WHEN_LOC_INVALID // ��λʧ��ʱ�����Ա��������ϴ�λ��
#define USE_IMEI_AS_CELL_NUM // ʹ��IMEI��12λ��Ϊ�ն˺�
//#define USE_JTT808_SERVER_1_SIM_DEVICE // ����ʱʹ��ƽ̨1ģ���ն˺�:12000187148

#define	DSMAPP_JTT_BUFF_MAX	(384)
#define CELL_NUMBER_LEN (DEVICE_NUM_LEN) // �ֻ��ų���
#define	FRAME_FLAG_SYMBOL	0x7E // ��ʶλ

#define	PHONE_NO_LEN		15

static kal_int32 g_s32FirstConnect = 1;
static kal_int32 g_s32PowerOff = 0;

static kal_uint8 g_s8TmpBuf[DSMAPP_JTT_BUFF_MAX];
//static kal_uint8 g_s8RxBuf[DSMAPP_JTT_BUFF_MAX];
static kal_uint8 g_s8TxBuf[DSMAPP_JTT_BUFF_MAX];

static kal_int8 g_s8Numfamily[PHONE_NO_LEN];

nvram_ef_dsmapp_jtt_config_t jtt_config;

static kal_uint8 shutdown_is_active = 0; // ϵͳ���ڹػ�
static kal_uint16 serial_no_up = 0; // ������Ϣ��ˮ��
static kal_uint16 serial_no_down = 0; // ������Ϣ��ˮ��
static kal_uint16 serial_no_loc_query_lastest = 0; // ���µ� λ����Ϣ��ѯ ��ˮ��
static dsm_login_status login_status;

dsm_srv_cb dsm_srv_send_handle_callback = NULL;

static nvram_ef_dsmapp_pos_mode_t dsm_pos_mode;

/*
	0x00 - ��ͨ�ϴ�
	0x0C - �ػ�
	0x0D - ��������ʼֵ��
	0x10 - �����ϴ���λ����Ϣ��ѯ��
	0x11 - ����͸��
	����0x80 - ��������
	0x80 - SOS����
	0x90 - �͵籨��
*/
static kal_uint8 pos_info_type = 0x0D;	/*��ʼֵ��ʾ����*/


/********************
��������
********************/
static void srvinteraction_location_request(kal_int8 type);
kal_int32 dsm_srv_register_jtt(dsm_srv_cb reg_cb);
static kal_int32 dsm_srv_deregister_jtt(dsm_srv_cb dereg_cb);
static kal_int32 dsm_srv_authorize_jtt(dsm_srv_cb auth_cb);
void dsm_srv_register_cb(void );
void dsm_srv_deregister_cb(void );
void dsm_srv_authorize_cb(void );
void dsmapp_srvinteraction_poweroff(void );
static void dsmapp_srvinteraction_reset(void);

extern kal_uint8 * release_verno(void);
extern kal_uint8 * gnss_verno(void) ;

kal_uint8 dsm_pos_info_type_set(kal_uint8 alarm)
{
	// �ػ� �� sos ״̬���ȼ��ߣ�������2��״̬��ʱ������״̬����
	if ((alarm != 0x00) && (alarm != 0x0C) && ((pos_info_type >= 0x80) || (pos_info_type == 0x0C))) return 0;/*sos important*//*shutdown power important*/

	pos_info_type = alarm;
	dsmapp_trace("%s(%x)", __FUNCTION__, alarm);

	return 1;
}

kal_uint8 dsm_pos_info_type_get(void)
{
	dsmapp_trace("%s(%x)", __FUNCTION__, pos_info_type);
	return pos_info_type;
}

// ���ö�λģʽ
kal_uint8 dsm_pos_mode_set(kal_uint8 mode)
{
	dsmapp_trace("%s(%d)", __FUNCTION__, mode);
#if (DEBUG_IN_VS == 0)
	if(mode == 1)
	{
		dsm_location_mode_change(LOCATION_EVT_RMC, LOCATION_MODE_NORMAL_PERIOD);
		dsm_location_period_change_sec(dsm_pos_mode.custom_period);
	}
	else
	{
		dsm_location_mode_change(LOCATION_EVT_RMC, LOCATION_MODE_ONE_TIME);
	}

	dsm_pos_mode.dev_mode = mode;
	dsm_srv_config_nv_write(1);
	
#endif
	return 0;
}

// ���ö�λ����
kal_uint8 dsm_pos_period_set(kal_uint32 period_sec)
{
	if (dsm_pos_mode.dev_mode == 1) // 1-��׼ģʽ
	{
		if(dsm_pos_mode.custom_period == period_sec) // same
		{
			dsmapp_trace("%s: %d same, return", __FUNCTION__, period_sec);
		}
		else
		{
			dsm_pos_mode.custom_period = period_sec;
			dsm_location_period_change_sec(period_sec);
			dsmapp_trace("%s: %d", __FUNCTION__, period_sec);
		}

		dsm_srv_config_nv_write(1);
	}

	return 0;
}

// �����ն��������ͼ��
// ���� 0:�ɹ� -1:ʧ��
static kal_int32 dsm_srv_set_para_heartbeat(kal_uint8 *in, kal_uint32 in_len)
{
	kal_int8 ret = -1;
	kal_uint32 heartbeat = 0; // unit: s

	if (!in || in_len != 4) return -1;

	heartbeat |= in[0];
	heartbeat <<= 8;
	heartbeat |= in[1];
	heartbeat <<= 8;
	heartbeat |= in[2];
	heartbeat <<= 8;
	heartbeat |= in[3];

	// �����ն��������ͼ��
	ret = dsmapp_srv_heart_set(heartbeat);

	return ret;
}

// ���ü����绰����
// ����: 0-�ɹ�
static kal_int32 dsm_srv_set_para_monitor_num(kal_uint8 *in, kal_uint32 in_len)
{
	kal_char *num = (kal_char *)g_s8Numfamily;

	if (!in || in_len == 0) return -1;

	kal_mem_set(num, 0, sizeof(g_s8Numfamily));
	kal_mem_cpy(num, in, in_len);

#if defined(__WHDSM_CALL_SUPPORT__)
	dsmapp_call_save_number(DSM_RELATION_MGR, num);
#elif defined(__WHDSM_ADMIN_SUPPORT__)
	dsmapp_admin_number_save(num);
#endif

	return 0;
}

// ���ö�λ�����ϴ���ʽ
// ����: 0-�ɹ�
// �ο�dsm_srv_cmd_handle_down_pos_interval_min
static kal_int32 dsm_srv_set_para_loc_type(kal_uint8 *in, kal_uint32 in_len)
{
	kal_uint8 loc_type = 0;

	if (!in || in_len != 1) return -1;

	loc_type = in[0];
	if (loc_type == 0 || loc_type == 1) // 0-ʡ��ģʽ 1-��׼ģʽ
	{
		dsm_pos_mode_set(loc_type);
	}

	return 0;
}

// ���ö�λ�����ϴ�����
// ����: 0-�ɹ�
// �ο�dsm_srv_cmd_handle_down_period
static kal_int32 dsm_srv_set_para_loc_prop(kal_uint8 *in, kal_uint32 in_len)
{
	kal_uint32 loc_prop = 0; // unit: s

	if (!in || in_len != 4) return -1;

	loc_prop |= in[0];
	loc_prop <<= 8;
	loc_prop |= in[1];
	loc_prop <<= 8;
	loc_prop |= in[2];
	loc_prop <<= 8;
	loc_prop |= in[3];

	dsm_pos_period_set(loc_prop);

	return 0;
}

// ���÷�����IP
// ���� 0:�ɹ�
static kal_int32 dsm_srv_set_para_srv_ip(kal_uint8 *in, kal_uint32 in_len)
{
	kal_uint8 srv_ip[16] = { 0 };

	if (!in || in_len == 0) return -1;

	kal_mem_cpy(srv_ip, in, in_len);
	dsmapp_srv_address_set((kal_char*)srv_ip, 0);

	return 0;
}

// ���÷�����TCP�˿�
// ���� 0:�ɹ�
static kal_int32 dsm_srv_set_para_srv_tcp_port(kal_uint8 *in, kal_uint32 in_len)
{
	kal_uint8 idx = 0;
	kal_uint32 srv_port = 0;

	if (!in || in_len == 0) return -1;

	while (idx < in_len)
	{
		srv_port = (srv_port << 8) | (in[idx++]);
	}

	dsmapp_srv_address_set(NULL, srv_port);

	return 0;
}



static dsm_set_para_handle_struct set_para_func[] =
{
	{ DSMAPP_SRV_JTT_PARA_HEARTBEAT, dsm_srv_set_para_heartbeat }, // �ն��������ͼ��
	{ DSMAPP_SRV_JTT_PARA_MONITOR_NUM, dsm_srv_set_para_monitor_num }, // �����绰����
	{ DSMAPP_SRV_JTT_PARA_LOC_TYPE, dsm_srv_set_para_loc_type }, // ��λ�����ϴ���ʽ
	{ DSMAPP_SRV_JTT_PARA_LOC_PROP, dsm_srv_set_para_loc_prop }, // ��λ�����ϴ�����

	{ DSMAPP_SRV_JTT_PARA_SRV_IP, dsm_srv_set_para_srv_ip }, // ������IP
	{ DSMAPP_SRV_JTT_PARA_SRV_TCP_PORT, dsm_srv_set_para_srv_tcp_port }, // ������TCP�˿�

	{ DSMAPP_SRV_JTT_PARA_INVALID, NULL }
};


// ��ѯ�ն��������ͼ��
// ��ѯ������浽out����ʽ������ID+��������+����ֵ��
// ���ز�ѯ������ȣ�-1��ʾʧ��
kal_int32 dsm_srv_get_para_heartbeat(kal_uint8 *out)
{
	kal_uint8 idx = 0;
	kal_uint32 heart = dsmapp_srv_heart_get();

	if (!out) return -1;

	/*����ID*/
	out[idx++] = (DSMAPP_SRV_JTT_PARA_HEARTBEAT >> 24) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_HEARTBEAT >> 16) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_HEARTBEAT >> 8) & 0xFF;
	out[idx++] = DSMAPP_SRV_JTT_PARA_HEARTBEAT & 0xFF;

	/*��������*/
	out[idx++] = 4; // DWORD

	/*����ֵ*/
	out[idx++] = (heart >> 24) & 0xFF;
	out[idx++] = (heart >> 16) & 0xFF;
	out[idx++] = (heart >> 8) & 0xFF;
	out[idx++] = heart & 0xFF;

	return idx; // 9
}

// ��ѯ�����绰����
// ��ѯ������浽out����ʽ������ID+��������+����ֵ��
// ���ز�ѯ������ȣ�-1��ʾʧ��
kal_int32 dsm_srv_get_para_monitor_num(kal_uint8 *out)
{
	kal_uint8 idx = 0;
	kal_char *num = (kal_char*)g_s8Numfamily;

	if (!out) return -1;

	/*����ID*/
	out[0] = (DSMAPP_SRV_JTT_PARA_MONITOR_NUM >> 24) & 0xFF;
	out[1] = (DSMAPP_SRV_JTT_PARA_MONITOR_NUM >> 16) & 0xFF;
	out[2] = (DSMAPP_SRV_JTT_PARA_MONITOR_NUM >> 8) & 0xFF;
	out[3] = DSMAPP_SRV_JTT_PARA_MONITOR_NUM & 0xFF;

	/*��������*/
	out[4] = PHONE_NO_LEN;

	/*����ֵ*/
	for (idx = 0; idx < PHONE_NO_LEN; idx++)
	{
		out[5 + idx] = num[idx];
	}

	return (idx+5); // 20
}

// ��ѯ��λ�����ϴ���ʽ
// ��ѯ������浽out����ʽ������ID+��������+����ֵ��
// ���ز�ѯ������ȣ�-1��ʾʧ��
kal_int32 dsm_srv_get_para_loc_type(kal_uint8 *out)
{
	kal_uint8 idx = 0;
	kal_uint8 loc_type = dsm_pos_mode.dev_mode;
	
	if (loc_type != 0)
	{
		loc_type = 1; // ֻ֧��0��ʡ�磬���ϴ�����1����׼����ʱ�����ϴ�������
	}

	if (!out) return -1;

	/*����ID*/
	out[idx++] = (DSMAPP_SRV_JTT_PARA_LOC_TYPE >> 24) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_LOC_TYPE >> 16) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_LOC_TYPE >> 8) & 0xFF;
	out[idx++] = DSMAPP_SRV_JTT_PARA_LOC_TYPE & 0xFF;

	/*��������*/
	out[idx++] = 1; // BYTE

	/*����ֵ*/
	out[idx++] = loc_type;

	return idx; // 6
}

// ��ѯ��λ�����ϴ�����
// ��ѯ������浽out����ʽ������ID+��������+����ֵ��
// ���ز�ѯ������ȣ�-1��ʾʧ��
kal_int32 dsm_srv_get_para_loc_prop(kal_uint8 *out)
{
	kal_uint8 idx = 0;
	kal_uint32 loc_prop = dsm_pos_mode.custom_period; // unit: s

	if (!out) return -1;

	/*����ID*/
	out[idx++] = (DSMAPP_SRV_JTT_PARA_LOC_PROP >> 24) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_LOC_PROP >> 16) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_LOC_PROP >> 8) & 0xFF;
	out[idx++] = DSMAPP_SRV_JTT_PARA_LOC_PROP & 0xFF;

	/*��������*/
	out[idx++] = 4; // DWORD

	/*����ֵ*/
	out[idx++] = (loc_prop >> 24) & 0xFF;
	out[idx++] = (loc_prop >> 16) & 0xFF;
	out[idx++] = (loc_prop >> 8) & 0xFF;
	out[idx++] = loc_prop & 0xFF;

	return idx; // 9
}

// ��ѯ����
// ��ѯ������浽out����ʽ������ID+��������+����ֵ��
// ���ز�ѯ������ȣ�-1��ʾʧ��
kal_int32 dsm_srv_get_para_battery(kal_uint8 *out)
{
	kal_uint8 idx = 0;
	kal_uint8 bat = dsmapp_battery_get_voltage_percent();

	if (!out) return -1;

	/*����ID*/
	out[idx++] = (DSMAPP_SRV_JTT_PARA_BATTERY >> 24) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_BATTERY >> 16) & 0xFF;
	out[idx++] = (DSMAPP_SRV_JTT_PARA_BATTERY >> 8) & 0xFF;
	out[idx++] = DSMAPP_SRV_JTT_PARA_BATTERY & 0xFF;

	/*��������*/
	out[idx++] = 1; // BYTE

	/*����ֵ*/
	out[idx++] = bat;

	return idx; // 6
}

static dsm_get_para_handle_struct get_para_func[] =
{
	{ DSMAPP_SRV_JTT_PARA_HEARTBEAT, dsm_srv_get_para_heartbeat }, // �ն��������ͼ��
	{ DSMAPP_SRV_JTT_PARA_MONITOR_NUM, dsm_srv_get_para_monitor_num }, // �����绰����
	{ DSMAPP_SRV_JTT_PARA_LOC_TYPE, dsm_srv_get_para_loc_type }, // ��λ�����ϴ���ʽ
	{ DSMAPP_SRV_JTT_PARA_LOC_PROP, dsm_srv_get_para_loc_prop }, // ��λ�����ϴ�����

	/*�Զ���*/
	{ DSMAPP_SRV_JTT_PARA_BATTERY, dsm_srv_get_para_battery }, // ����

	{ DSMAPP_SRV_JTT_PARA_INVALID, NULL }
};

// �ն�ͨ��Ӧ��
// in: ��������Ϣ�����ݣ�Ӧ����ˮ�š�Ӧ��ID�������
static kal_int32 dsm_srv_cmd_handle_up_ack(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	if (dat_l == NULL) return len_l;

	if (in&&in_len) // �������������
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	} 
	else
	{
		return len_l;
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// �ն�����
static kal_int32 dsm_srv_cmd_handle_up_heartbeat(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		/*��Ϣ��Ϊ��*/
		len_l = 0;
		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// �ն�ע��
// ʾ����7E 0100002B0000000000000000 002A006F31323334354D585431363038000000000000000000000000004D58543136303801415A31323334 40 7E
static kal_int32 dsm_srv_cmd_handle_up_register(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint16 province = 42;
	kal_uint16 city = 111;
	kal_uint8 manufac[] = "12345";
	kal_uint8 dev_type[] = "MXT1608";
	//kal_uint8 dev_id[] = "MXT1608";
	//kal_uint8 dev_id[] = "WD0001";
	//kal_uint8 dev_id[] = "1104058";
	kal_uint8 dev_id[] = "1104060";
	kal_uint8 license_color = 1;
	kal_char license[] = "AZ1234";

	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;
	kal_uint32 i = 0, len = 0;

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		len_l = 0;

		/*province*/
		dat_l[len_l++] = (province >> 8) & 0xFF;
		dat_l[len_l++] = province & 0xFF;

		/*city*/
		dat_l[len_l++] = (city >> 8) & 0xFF;
		dat_l[len_l++] = city & 0xFF;

		/*manufac*/
		snprintf((char*)&dat_l[len_l], 5, "%s\0", manufac);
		len_l += 5;

		/*dev_type*/
		len = strlen((const char*)dev_type);
		//snprintf(&dat_l[len_l], 20, "%s\0", dev_type);
		snprintf((char*)&dat_l[len_l], 20, "%s\n", dev_type);
		for (i = len; i < 20; i++)
			dat_l[len_l + i] = 0; // ������0
		len_l += 20;

		/*dev_id*/
		len = strlen((const char*)dev_id);
		snprintf((char *)&dat_l[len_l], 7, "%s\0", dev_id);
		for (i = len; i < 7; i++)
			dat_l[len_l + i] = 0; // ������0
		len_l += 7;

		/*license_color*/
		dat_l[len_l++] = license_color;

		
		/*license*/
		len = strlen(license);
		snprintf((char*)&dat_l[len_l], len, "%s\0", license);
		len_l += len;

		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// �ն�ע��
static kal_int32 dsm_srv_cmd_handle_up_deregister(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		/*��Ϣ��Ϊ��*/
		len_l = 0;
		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// �ն˼�Ȩ
static kal_int32 dsm_srv_cmd_handle_up_authorize(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		info("dsm_srv_cmd_handle_up_authorize in_len %d\n", in_len);
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{

		info("dsm_srv_cmd_handle_up_authorize jtt_config.auth_code strlen: %d\n", strlen((const char*)jtt_config.auth_code));

		
		/*��Ȩ��*/
		memcpy(dat_l, jtt_config.auth_code, AUTH_CODE_LEN-1);
		//len_l = AUTH_CODE_LEN-1;//strlen((const char*)jtt_config.auth_code);
		len_l = strlen((const char*)jtt_config.auth_code) ;

		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// ��ѯ�ն˲���Ӧ��
static kal_int32 dsm_srv_cmd_handle_up_query_para_ack(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	if (dat_l == NULL) return len_l;

	// ������ǰ׼��������
	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// ��ѯ�ն�����Ӧ��
static kal_int32 dsm_srv_cmd_handle_up_query_prop_ack(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;
	kal_uint8 idx = 0;
	kal_uint8 i;

	kal_uint8 verno[100] = { 0 }; // �ն˹̼��汾��
	kal_uint8 verno_len = 0; // �ն˹̼��汾�ų���

	kal_uint8 hd_verno_len = 0; // �ն�Ӳ���汾�ų��ȣ�Ĭ��0

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
#if defined(__WHDSM_BT_SUPPORT__)
		// 6261�汾�� GNSS�̼��汾�� �����汾��
		snprintf(verno, 100, "%s|%s|%s", release_verno(), gnss_verno(), bt_verno());
#else
		// 6261�汾�� GNSS�̼��汾��
		snprintf((char *)verno, 100, "%s|%s", release_verno(), gnss_verno());
#endif
		verno_len = strlen((const char*)verno);

		for (i = 0; i < 2; i++)
		{
			dat_l[idx++] = 0; // �ն�����
		}

		for (i = 0; i < 5; i++)
		{
			dat_l[idx++] = 0; // ������ID
		}

		for (i = 0; i < 20; i++)
		{
			dat_l[idx++] = 0; // �ն��ͺ�
		}

		for (i = 0; i < 7; i++)
		{
			dat_l[idx++] = 0; // �ն�ID
		}

		for (i = 0; i < 10; i++)
		{
			dat_l[idx++] = 0; // �ն�SIM��ICCID
		}

		dat_l[idx++] = hd_verno_len; // �ն�Ӳ���汾�ų���
		for (i = 0; i < hd_verno_len; i++)
		{
			dat_l[idx++] = 0; // �ն�Ӳ���汾��
		}

		dat_l[idx++] = verno_len; // �ն˹̼��汾�ų���
		kal_mem_cpy(&dat_l[idx], verno, verno_len); // �ն˹̼��汾��
		idx += verno_len;

		dat_l[idx++] = 3; // GNSSģ�����ԣ�֧��GPS��������λ��
		dat_l[idx++] = 1; // ͨ��ģ�����ԣ�֧��GPRSͨ�ţ�

		len_l = idx;
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// λ����Ϣ�㱨
static kal_int32 dsm_srv_cmd_handle_up_loc_report(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint32 alarm = 0;
	kal_uint32 status = 0;
	kal_uint32 lat = 0;
	kal_uint32 lon = 0;
	kal_uint16 alt = 0;
	kal_uint16 speed = 0; // unit: 1/10km/h
	kal_uint16 course = 0;
	
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	kal_uint8 pos_info_type;
	ST_DSM_LOCATION_INFO *pos_info;
	MYTIME stCurrentTime;

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		len_l = 0;

		/*λ����Ϣ�㱨start*/
		/*������ʶλ*/
		pos_info_type = dsm_pos_info_type_get();
		if (pos_info_type == 0x80) // 0x80 - SOS����
		{
			alarm = 0x01<<0;
		}
		else if (pos_info_type == 0x90) // 0x80 - �͵籨��
		{
			alarm = 0x01<<7;
		}
		dsm_pos_info_type_set(0x00); // �ָ���ͨ�ϴ�
		dsmapp_trace("alarm=%x", alarm);

		/*��λ�������*/
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
		pos_info = dsm_location_get_latest_position();
		if ((pos_info->valid) && (pos_info->type == LOCATION_TYPE_GPS))
		{
			status |= (0x01 << 0); // ACCĬ��Ϊ1
			status |= (0x01 << 1); // ��λ
			if(pos_info->info.gps.N_S == 'S')
				status |= (0x01 << 2); // ��γ
			if(pos_info->info.gps.E_W == 'W')
				status |= (0x01 << 3); // ����
			status |= (0x01 << 18); // ʹ��GPS��λ
			status |= (0x01 << 19); // ʹ�ñ�����λ
			
			lat = 0.5+(dsm_base_coordinates2double(&(pos_info->info.gps.lat[0])) * 1000000);
			lon = 0.5+(dsm_base_coordinates2double(&(pos_info->info.gps.lon[0])) * 1000000);
			alt = pos_info->info.gps.alt;
			speed = pos_info->info.gps.speed * 10;
			course = pos_info->info.gps.course;
		}
#else
		lat = 30000000;
		lon = 114000000;
#endif
		dsmapp_trace("status=%x", status); // ��λʱһ��Ϊ0xC002 δ��λΪ0
		dsmapp_trace("lat=%d,lon=%d,alt=%d,speed=%d,course=%d", lat, lon, alt, speed, course);

		/*alarm*/
		dat_l[len_l++] = (alarm >> 24) & 0xFF;
		dat_l[len_l++] = (alarm >> 16) & 0xFF;
		dat_l[len_l++] = (alarm >> 8) & 0xFF;
		dat_l[len_l++] = alarm & 0xFF;

		/*status*/
		dat_l[len_l++] = (status >> 24) & 0xFF;
		dat_l[len_l++] = (status >> 16) & 0xFF;
		dat_l[len_l++] = (status >> 8) & 0xFF;
		dat_l[len_l++] = status & 0xFF;

		/*lat*/
		dat_l[len_l++] = (lat >> 24) & 0xFF;
		dat_l[len_l++] = (lat >> 16) & 0xFF;
		dat_l[len_l++] = (lat >> 8) & 0xFF;
		dat_l[len_l++] = lat & 0xFF;

		/*lon*/
		dat_l[len_l++] = (lon >> 24) & 0xFF;
		dat_l[len_l++] = (lon >> 16) & 0xFF;
		dat_l[len_l++] = (lon >> 8) & 0xFF;
		dat_l[len_l++] = lon & 0xFF;

		/*alt*/
		dat_l[len_l++] = (alt >> 8) & 0xFF;
		dat_l[len_l++] = alt & 0xFF;

		/*speed*/
		dat_l[len_l++] = (speed >> 8) & 0xFF;
		dat_l[len_l++] = speed & 0xFF;

		/*course*/
		dat_l[len_l++] = (course >> 8) & 0xFF;
		dat_l[len_l++] = course & 0xFF;

		/*time - BCD code*/
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
		DTGetRTCTime(&stCurrentTime);
#else
		stCurrentTime.nYear = 2017;
		stCurrentTime.nMonth = 4;
		stCurrentTime.nDay = 1;
		stCurrentTime.nHour = 17;
		stCurrentTime.nMin = 15;
		stCurrentTime.nSec = 33;
#endif
		stCurrentTime.nYear -= 2000;
		dat_l[len_l++] = ((stCurrentTime.nYear / 10) << 4) + (stCurrentTime.nYear % 10);
		dat_l[len_l++] = ((stCurrentTime.nMonth / 10) << 4) + (stCurrentTime.nMonth % 10);
		dat_l[len_l++] = ((stCurrentTime.nDay / 10) << 4) + (stCurrentTime.nDay % 10);
		dat_l[len_l++] = ((stCurrentTime.nHour / 10) << 4) + (stCurrentTime.nHour % 10);
		dat_l[len_l++] = ((stCurrentTime.nMin / 10) << 4) + (stCurrentTime.nMin % 10);
		dat_l[len_l++] = ((stCurrentTime.nSec / 10) << 4) + (stCurrentTime.nSec % 10);
		//len_l += 6;

		/*λ����Ϣ�㱨end*/

		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}



static kal_int32 dsm_srv_cmd_handle_up_loc_report_extern(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint32 alarm = 0;
	kal_uint32 status = 0;
	kal_uint32 lat = 0;
	kal_uint32 lon = 0;
	kal_uint16 alt = 0;
	kal_uint16 speed = 0; // unit: 1/10km/h
	kal_uint16 course = 0;
	
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	kal_uint8 pos_info_type;
	ST_DSM_LOCATION_INFO *pos_info;
	MYTIME stCurrentTime;

	if (dat_l == NULL) return len_l;

	
	if (1) 
	{
		len_l = 0;

		/*λ����Ϣ�㱨start*/
		/*������ʶλ*/
		pos_info_type = dsm_pos_info_type_get();
		if (pos_info_type == 0x80) // 0x80 - SOS����
		{
			alarm = 0x01<<0;
		}
		else if (pos_info_type == 0x90) // 0x80 - �͵籨��
		{
			alarm = 0x01<<7;
		}
		dsm_pos_info_type_set(0x00); // �ָ���ͨ�ϴ�
		dsmapp_trace("alarm=%x", alarm);

		/*��λ�������*/
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
		pos_info = dsm_location_get_latest_position();
		if ((pos_info->valid) && (pos_info->type == LOCATION_TYPE_GPS))
		{
			status |= (0x01 << 0); // ACCĬ��Ϊ1
			status |= (0x01 << 1); // ��λ
			if(pos_info->info.gps.N_S == 'S')
				status |= (0x01 << 2); // ��γ
			if(pos_info->info.gps.E_W == 'W')
				status |= (0x01 << 3); // ����
			status |= (0x01 << 18); // ʹ��GPS��λ
			status |= (0x01 << 19); // ʹ�ñ�����λ
			
			lat = 0.5+(dsm_base_coordinates2double(&(pos_info->info.gps.lat[0])) * 1000000);
			lon = 0.5+(dsm_base_coordinates2double(&(pos_info->info.gps.lon[0])) * 1000000);
			alt = pos_info->info.gps.alt;
			speed = pos_info->info.gps.speed * 10;
			course = pos_info->info.gps.course;
		}
#else
		lat = 30000000;
		lon = 114000000;
#endif
		dsmapp_trace("status=%x", status); // ��λʱһ��Ϊ0xC002 δ��λΪ0
		dsmapp_trace("lat=%d,lon=%d,alt=%d,speed=%d,course=%d", lat, lon, alt, speed, course);

		/*alarm*/
		dat_l[len_l++] = (alarm >> 24) & 0xFF;
		dat_l[len_l++] = (alarm >> 16) & 0xFF;
		dat_l[len_l++] = (alarm >> 8) & 0xFF;
		dat_l[len_l++] = alarm & 0xFF;

		/*status*/
		dat_l[len_l++] = (status >> 24) & 0xFF;
		dat_l[len_l++] = (status >> 16) & 0xFF;
		dat_l[len_l++] = (status >> 8) & 0xFF;
		dat_l[len_l++] = status & 0xFF;

		/*lat*/
		dat_l[len_l++] = (lat >> 24) & 0xFF;
		dat_l[len_l++] = (lat >> 16) & 0xFF;
		dat_l[len_l++] = (lat >> 8) & 0xFF;
		dat_l[len_l++] = lat & 0xFF;

		/*lon*/
		dat_l[len_l++] = (lon >> 24) & 0xFF;
		dat_l[len_l++] = (lon >> 16) & 0xFF;
		dat_l[len_l++] = (lon >> 8) & 0xFF;
		dat_l[len_l++] = lon & 0xFF;

		/*alt*/
		dat_l[len_l++] = (alt >> 8) & 0xFF;
		dat_l[len_l++] = alt & 0xFF;

		/*speed*/
		dat_l[len_l++] = (speed >> 8) & 0xFF;
		dat_l[len_l++] = speed & 0xFF;

		/*course*/
		dat_l[len_l++] = (course >> 8) & 0xFF;
		dat_l[len_l++] = course & 0xFF;

		/*time - BCD code*/
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
		DTGetRTCTime(&stCurrentTime);
#else
		stCurrentTime.nYear = 2017;
		stCurrentTime.nMonth = 4;
		stCurrentTime.nDay = 1;
		stCurrentTime.nHour = 17;
		stCurrentTime.nMin = 15;
		stCurrentTime.nSec = 33;
#endif
		stCurrentTime.nYear -= 2000;
		dat_l[len_l++] = ((stCurrentTime.nYear / 10) << 4) + (stCurrentTime.nYear % 10);
		dat_l[len_l++] = ((stCurrentTime.nMonth / 10) << 4) + (stCurrentTime.nMonth % 10);
		dat_l[len_l++] = ((stCurrentTime.nDay / 10) << 4) + (stCurrentTime.nDay % 10);
		dat_l[len_l++] = ((stCurrentTime.nHour / 10) << 4) + (stCurrentTime.nHour % 10);
		dat_l[len_l++] = ((stCurrentTime.nMin / 10) << 4) + (stCurrentTime.nMin % 10);
		dat_l[len_l++] = ((stCurrentTime.nSec / 10) << 4) + (stCurrentTime.nSec % 10);
		//len_l += 6;

		/*λ����Ϣ�㱨end*/


		if (in&&in_len)
		{
			kal_mem_cpy(&dat_l[len_l], in, in_len);
			len_l += in_len;

			info("padding %d bytes data \n", in_len);
		}

		

		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}


// λ����Ϣ��ѯӦ��
static kal_int32 dsm_srv_cmd_handle_up_loc_query_ack(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint32 alarm = 0;
	kal_uint32 status = 0;
	kal_uint32 lat = 0;
	kal_uint32 lon = 0;
	kal_uint16 alt = 0;
	kal_uint16 speed = 0; // unit: 1/10km/h
	kal_uint16 course = 0;

	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;

	kal_uint8 pos_info_type;
	ST_DSM_LOCATION_INFO *pos_info;
	MYTIME stCurrentTime;

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		len_l = 0;

		/*Ӧ����ˮ��*/
		dat_l[len_l++] = (serial_no_loc_query_lastest >> 8) & 0xFF;
		dat_l[len_l++] = serial_no_loc_query_lastest & 0xFF;

		/*λ����Ϣ�㱨start*/
		/*������ʶλ*/
		pos_info_type = dsm_pos_info_type_get();
		if (pos_info_type == 0x80) // 0x80 - SOS����
		{
			alarm = 0x01<<0;
		}
		else if (pos_info_type == 0x90) // 0x80 - �͵籨��
		{
			alarm = 0x01<<7;
		}
		dsm_pos_info_type_set(0x00); // �ָ���ͨ�ϴ�
		dsmapp_trace("alarm=%x", alarm);

		/*��λ�������*/
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
		pos_info = dsm_location_get_latest_position();
		if ((pos_info->valid) && (pos_info->type == LOCATION_TYPE_GPS))
		{
			status |= (0x01 << 0); // ACCĬ��Ϊ1
			status |= (0x01 << 1); // ��λ
			if(pos_info->info.gps.N_S == 'S')
				status |= (0x01 << 2); // ��γ
			if(pos_info->info.gps.E_W == 'W')
				status |= (0x01 << 3); // ����
			status |= (0x01 << 18); // ʹ��GPS��λ
			status |= (0x01 << 19); // ʹ�ñ�����λ
			
			lat = 0.5+(dsm_base_coordinates2double(&(pos_info->info.gps.lat[0])) * 1000000);
			lon = 0.5+(dsm_base_coordinates2double(&(pos_info->info.gps.lon[0])) * 1000000);
			alt = pos_info->info.gps.alt;
			speed = pos_info->info.gps.speed * 10;
			course = pos_info->info.gps.course;
		}
#else
		lat = 30000000;
		lon = 114000000;
#endif
		dsmapp_trace("status=%x", status); // ��λʱһ��Ϊ0xC002 δ��λΪ0
		dsmapp_trace("lat=%d,lon=%d,alt=%d,speed=%d,course=%d", lat, lon, alt, speed, course);

		/*alarm*/
		dat_l[len_l++] = (alarm >> 24) & 0xFF;
		dat_l[len_l++] = (alarm >> 16) & 0xFF;
		dat_l[len_l++] = (alarm >> 8) & 0xFF;
		dat_l[len_l++] = alarm & 0xFF;

		/*status*/
		dat_l[len_l++] = (status >> 24) & 0xFF;
		dat_l[len_l++] = (status >> 16) & 0xFF;
		dat_l[len_l++] = (status >> 8) & 0xFF;
		dat_l[len_l++] = status & 0xFF;

		/*lat*/
		dat_l[len_l++] = (lat >> 24) & 0xFF;
		dat_l[len_l++] = (lat >> 16) & 0xFF;
		dat_l[len_l++] = (lat >> 8) & 0xFF;
		dat_l[len_l++] = lat & 0xFF;

		/*lon*/
		dat_l[len_l++] = (lon >> 24) & 0xFF;
		dat_l[len_l++] = (lon >> 16) & 0xFF;
		dat_l[len_l++] = (lon >> 8) & 0xFF;
		dat_l[len_l++] = lon & 0xFF;

		/*alt*/
		dat_l[len_l++] = (alt >> 8) & 0xFF;
		dat_l[len_l++] = alt & 0xFF;

		/*speed*/
		dat_l[len_l++] = (speed >> 8) & 0xFF;
		dat_l[len_l++] = speed & 0xFF;

		/*course*/
		dat_l[len_l++] = (course >> 8) & 0xFF;
		dat_l[len_l++] = course & 0xFF;

		/*time - BCD code*/
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
		DTGetRTCTime(&stCurrentTime);
#else
		stCurrentTime.nYear = 2017;
		stCurrentTime.nMonth = 4;
		stCurrentTime.nDay = 1;
		stCurrentTime.nHour = 17;
		stCurrentTime.nMin = 15;
		stCurrentTime.nSec = 33;
#endif
		stCurrentTime.nYear -= 2000;
		dat_l[len_l++] = ((stCurrentTime.nYear / 10) << 4) + (stCurrentTime.nYear % 10);
		dat_l[len_l++] = ((stCurrentTime.nMonth / 10) << 4) + (stCurrentTime.nMonth % 10);
		dat_l[len_l++] = ((stCurrentTime.nDay / 10) << 4) + (stCurrentTime.nDay % 10);
		dat_l[len_l++] = ((stCurrentTime.nHour / 10) << 4) + (stCurrentTime.nHour % 10);
		dat_l[len_l++] = ((stCurrentTime.nMin / 10) << 4) + (stCurrentTime.nMin % 10);
		dat_l[len_l++] = ((stCurrentTime.nSec / 10) << 4) + (stCurrentTime.nSec % 10);
		//len_l += 6;
		
		/*λ����Ϣ�㱨end*/

		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// �¼�����
static kal_int32 dsm_srv_cmd_handle_up_event_report(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;
	SRV_EVENT_ID event;

	if (dat_l == NULL) return len_l;

	if (in&&in_len) // �������������
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		return len_l;
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// ������Ϣ
kal_uint8 BATTERY_SENDUP_JTT = 0; // JTT808 �ϴ��ĵ����ٷ�ֵ
static kal_int32 dsm_srv_cmd_handle_up_battery(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;
	BATTERY_SENDUP_JTT = dsmapp_battery_get_voltage_percent();

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		len_l = 0;

		/*battery*/
		dat_l[len_l++] = BATTERY_SENDUP_JTT;
		
		dat_l[len_l] = '\0';
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}

// ��Ƭ����
extern void get_iccid_value(kal_uint8 *iccid, kal_uint8 source);

unsigned char last_correct_autho_num[AUTH_CODE_LEN] = {0 };
extern kal_uint8 *release_verno(void);
static kal_int32 dsm_srv_cmd_handle_up_card_login(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 len_l = -1;
	kal_uint8 *dat_l = out;
	kal_uint8 iccid[11] = {0};
	kal_uint8 verno_len = 0;
	kal_uint8 * verno = NULL;

	dsmapp_trace("%s: enter", __FUNCTION__);
	get_iccid_value(&iccid[0], 0);
	verno = (kal_uint8*)release_verno();
	verno_len = strlen((const char*)verno);

	if (dat_l == NULL) return len_l;

	if (in&&in_len)
	{
		kal_mem_cpy(dat_l, in, in_len);
		len_l = in_len;
	}
	else
	{
		len_l = 0;

		/*iccid BCD��*/
		while(len_l < 10)
		{
			dsmapp_trace("iccid[%d] = %d", len_l, iccid[len_l]);
			dat_l[len_l] = (iccid[len_l] << 4) | (iccid[len_l] >> 4); // 0xab -> 0xba
			len_l++;
		}

		/*verno_len*/
		dat_l[len_l++] = verno_len;
		dsmapp_trace("verno_len = %d", verno_len);

		/*verno*/
		int ret = snprintf((char*)&dat_l[len_l], verno_len, "%s", verno);//not same with snprintf in MTK!
		len_l += verno_len;
	}

	dsmapp_trace("%s: len_l=%d(%x)", __FUNCTION__, len_l, len_l);

	return len_l;
}


// ƽ̨ͨ��Ӧ��
// ����ֵ: 0-�ɹ�/ȷ�� 1-ʧ�� 2-��Ϣ���� 3-��֧�� 4-��������ȷ��
static kal_int32 dsm_srv_cmd_handle_down_ack(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint8 *dat_l = in;
	kal_uint8 tmp;

	kal_uint16 ack_flow = 0; // Ӧ����ˮ��
	kal_uint16 ack_id = 0; // Ӧ��ID
	kal_int32 ret = -1; // ���

	if ((dat_l == NULL) || (in_len != 5)) return -1; // ƽ̨ͨ��Ӧ�� ��Ϣ�峤��in_lenӦΪ5
	dsmapp_trace("%s:cmd(%x) in_len(%d)", __FUNCTION__, cmd, in_len);

	/*Ӧ����ˮ��*/
	tmp = *dat_l++;
	ack_flow += tmp << 8;
	tmp = *dat_l++;
	ack_flow += tmp;

	/*Ӧ��ID*/
	tmp = *dat_l++;
	ack_id += tmp << 8;
	tmp = *dat_l++;
	ack_id += tmp;

	/*���: 0-�ɹ�/ȷ�� 1-ʧ�� 2-��Ϣ���� 3-��֧�� 4-��������ȷ��*/
	tmp = *dat_l++;
	ret = tmp;

	dsmapp_trace("content: ack_flow=%d ack_id=%x ret=%d", ack_flow, ack_id, ret);

	dsmapp_trace("login_status: status=%d flow=%d", login_status.status, login_status.flow);
	if (ack_flow == login_status.flow)
	{
		StopTimer(DSMAPP_TIMER_JTT_SERVER);
		StopTimer(DSMAPP_TIMER_JTT_SERVER_REGISTER);
		StopTimer(DSMAPP_TIMER_JTT_SERVER_DEREGISTER);
		StopTimer(DSMAPP_TIMER_JTT_SERVER_AUTHORIZE);
		if ((login_status.status == 2) && (ret == 0))
		{
			// ע���ɹ� ��ʼע��
			dsm_srv_register_jtt(dsm_srv_register_cb);
		}
		else if ((login_status.status == 1) && (ret == 0))
		{
			// ��Ȩ�ɹ�
			login_status.status = 4;

			printf("backup last correct auth num\n");

			memcpy(jtt_config.auth_code, last_correct_autho_num, AUTH_CODE_LEN);

			succ("\n==================Authorize Successfully !============================\n\n\n\n ");

			
			dsmapp_trace("first loc report-->");
			dsm_srv_loc_report_jtt(); // ��½ƽ̨���ϴ�λ��

			
		}
		else if ((login_status.status == 1) && (ret != 0))
		{
			// ��Ȩʧ��
			login_status.status = 0;

			dsmapp_trace("auth failed");
			
			err("\n==================Authorize Failed !============================\n\n\n\n ");
			dsm_srv_auth_code_clear(); // ��ռ�Ȩ��
		}
	}
	
	return ret;
}

// �ն�ע��Ӧ��
// ����ֵ: 0-�ɹ� 1-�����ѱ�ע�� 2-���ݿ����޸ó��� 3-�ն��ѱ�ע�� 4-���ݿ����޸��ն�
static kal_int32 dsm_srv_cmd_handle_down_register_ack(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint8 *dat_l = in;
	kal_uint8 tmp;

	kal_uint16 ack_flow = 0; // Ӧ����ˮ��
	kal_int32 ret = -1; // ���
	kal_uint8 *auth = jtt_config.auth_code; // ��Ȩ��

	if ((dat_l == NULL) || (in_len == 0)) return -1;
	dsmapp_trace("%s:cmd(%x) in_len(%d)", __FUNCTION__, cmd, in_len);

	/*Ӧ����ˮ��*/
	tmp = *dat_l++;
	ack_flow += tmp << 8;
	tmp = *dat_l++;
	ack_flow += tmp;

	/*���: 0-�ɹ� 1-�����ѱ�ע�� 2-���ݿ����޸ó��� 3-�ն��ѱ�ע�� 4-���ݿ����޸��ն�*/
	tmp = *dat_l++;
	ret = tmp;

	dsmapp_trace("content: ack_flow=%d ret=%d", ack_flow, ret);

	if (ret == 0) // �ɹ�
	{
		/*��Ȩ��*/
		jtt_config.auth_code_len = in_len - 3; // ��Ȩ��ʵ�ʳ���
		kal_mem_cpy(auth, dat_l, jtt_config.auth_code_len);

		dsm_srv_config_nv_write(2); // ���浽NVRAM

		// debug log
		warn("auth_code(len=%d):\n", jtt_config.auth_code_len);
		tmp = 0;
		while(tmp < jtt_config.auth_code_len)
		{
			warn("%x ", auth[tmp++]);
		}

		warn("\n");

		info("\n============================== Register Follow End (successfully) =============================\n\n\n");

		StopTimer(DSMAPP_TIMER_JTT_SERVER);

		StopTimer(DSMAPP_TIMER_JTT_SERVER);
		StopTimer(DSMAPP_TIMER_JTT_SERVER_REGISTER);


		

		info("\n\n\n============================== Authrize Follow Begin =============================\n");
		dsm_srv_authorize_jtt(dsm_srv_authorize_cb); // ��ʼ��Ȩ
	}

	return ret;
}

// �����ն˲���
// ����: 0-�ɹ�
static kal_int32 dsm_srv_cmd_handle_down_set_para(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint8 *dat_l = in;
	kal_uint32 tmp;

	kal_uint8 para_num = 0; // ��������
	kal_uint8 para_idx = 0;

	kal_uint32 para_id = 0; // ����ID
	kal_uint8 para_len = 0; // ��������
	kal_uint8 *para = NULL; // ����ֵ

	dsm_set_para_handle_struct *func_l = NULL;
	kal_uint32 idx = 0;
	kal_int32 ret = -1;

	para_num = *dat_l++;
	while (para_idx < para_num)
	{
		/*����ID*/
		para_id = 0;
		tmp = *dat_l++;
		para_id |= (tmp << 12);
		tmp = *dat_l++;
		para_id |= (tmp << 8);
		tmp = *dat_l++;
		para_id |= (tmp << 4);
		tmp = *dat_l++;
		para_id |= tmp;

		/*��������*/
		tmp = *dat_l++;
		para_len = tmp;

		/*����ֵ*/
		para = dat_l;
		dat_l += para_len;

		/*���Ҳ���ID��Ӧ���ú���*/
		idx = 0;
		while (set_para_func[idx].para_id != DSMAPP_SRV_JTT_PARA_INVALID)
		{
			if (set_para_func[idx].para_id == para_id)
			{
				func_l = &set_para_func[idx];
				break;
			}
			idx++;
		}
		if (!func_l)
		{
			dsmapp_trace("--->>dsm_srv_cmd_handle_down_set_para(para_id[%04x] undef!!!!)\n", para_id);
		}
		else
		{
			dsmapp_trace("--->>dsm_srv_cmd_handle_down_set_para([%d]=%04x,%04x)\n", idx, func_l->para_id, para_id);
		}

		/*ִ�в���ID��Ӧ���ú���*/
		if ((func_l) && (func_l->handle))
		{
			ret = func_l->handle(para, para_len);
		}

		if (ret != 0) // ĳһ��ʧ���������˳�
		{
			return ret;
		}

		para_idx++; 
	}

	return ret; // �������þ��ɹ�
}

// ��ѯ�ն˲���
// ��ѯ������ղ������б��浽out
static kal_int32 dsm_srv_cmd_handle_down_query_para(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	/*��Ϣ��Ϊ��*/
	kal_int32 ret = 0;
	kal_uint32 idx = 0;
	dsm_get_para_handle_struct *func_l = NULL;
	kal_uint8 *buf_tmp = NULL; // ���������ݣ���Ϣ��
	kal_uint8 para_num = 0; // ��������
	kal_uint16 out_len = 0; // ��Ϣ�峤��
	
	if (!out) return -1;

	buf_tmp = out + 2; // Ԥ��2�ֽڱ���Ӧ����ˮ��
	out_len++; // Ԥ��buf_tmp[0]�����������

	/*����ִ��ȫ������ID��Ӧ��ѯ����*/
	idx = 0;
	while (get_para_func[idx].para_id != DSMAPP_SRV_JTT_PARA_INVALID)
	{
		func_l = &get_para_func[idx];

		/*ִ�в���ID��Ӧ���ú���*/
		if ((func_l) && (func_l->handle) && (out_len < DSMAPP_JTT_BUFF_MAX))
		{
			ret = func_l->handle(&buf_tmp[out_len]);
			if (ret > 0)
			{
				out_len += ret;
				para_num++;
			}
		}
		idx++;
	}

	buf_tmp[0] = para_num;

	return out_len;
}

// �ն˿��ƣ�3�ػ���4��λ��
// ����ֵ: 0 �ɹ� -1 ʧ��
static kal_int32 dsm_srv_cmd_handle_down_control(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 ret = -1;
	kal_uint32 idx = 0;
	kal_uint8 control;

	if (!in || in_len == 0) return -1;
	if (!out) return -1;

	control = in[idx++];
	if (control == 3) // 3�ػ�����������ն�ͨ��Ӧ��
	{
		ret = 0;
		dsmapp_srvinteraction_poweroff();
	}
	else if (control == 4) // 4��λ
	{
		ret = 0;
		dsmapp_srvinteraction_reset();
	}
	else // ���������ݲ�֧��
	{
	}

	return ret;
}

// ��ѯָ���ն˲���
// ����ID 0xF000~0xFFFFΪ�Զ��壨��ƽ̨��ϣ��ݲ�ʵ�֣�
static kal_int32 dsm_srv_cmd_handle_down_query_spec_para(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 ret = 0;
	kal_uint32 idx = 0;
	kal_uint32 cmd_l = 0;
	kal_uint8 i,j;
	dsm_get_para_handle_struct *func_l = NULL;
	kal_uint8 *buf_tmp = NULL; // ���������ݣ���Ϣ��
	kal_uint8 para_num = 0; // ��ѯ��������
	kal_uint16 out_len = 0; // ��Ϣ�峤��
	kal_uint8 para_num_ret = 0; // ��ѯ�����������

	if (!in || in_len == 0) return -1;
	if (!out) return -1;

	buf_tmp = out + 2; // Ԥ��2�ֽڱ���Ӧ����ˮ��
	out_len++; // Ԥ��buf_tmp[0]�����������

	para_num = in[idx++];
	for (i = 0; i < para_num; i++)
	{
		cmd_l = (in[idx++] << 24);
		cmd_l += (in[idx++] << 16);
		cmd_l += (in[idx++] << 8);
		cmd_l += in[idx++];

		/*���Ҳ���ID��Ӧ��ѯ����*/
		j = 0;
		while (get_para_func[j].para_id != DSMAPP_SRV_JTT_PARA_INVALID)
		{
			if (get_para_func[j].para_id == cmd_l)
			{
				func_l = &get_para_func[j];
				break;
			}
			j++;
		}

		/*ִ�в���ID��Ӧ��ѯ����*/
		if ((func_l) && (func_l->handle))
		{
			ret = func_l->handle(&buf_tmp[out_len]);
			if (ret > 0)
			{
				out_len += ret;
				para_num_ret++;
			}
		}
	}

	buf_tmp[0] = para_num_ret;

	return out_len;
}

// ��ѯ�ն�����
static kal_int32 dsm_srv_cmd_handle_down_query_prop(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	/*��Ϣ��Ϊ��*/

	return 0;
}

// λ����Ϣ��ѯ
static kal_int32 dsm_srv_cmd_handle_down_loc_query(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	/*��Ϣ��Ϊ��*/

	dsm_pos_info_type_set(0x10); // λ����Ϣ��ѯ
	srvinteraction_location_request(0);

	return 0;
}

// TODO: �¼�����
static kal_int32 dsm_srv_cmd_handle_down_event_set(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	return 0;
}

// TODO: �绰�ز�
static kal_int32 dsm_srv_cmd_handle_down_call_monitor(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	return 0;
}

// TODO: ���õ绰��
static kal_int32 dsm_srv_cmd_handle_down_phonebook(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	return 0;
}

static dsm_cmd_handle_struct cmd_func[] =
{
	{ DSMAPP_SRV_JTT_CMD_UP_ACK, dsm_srv_cmd_handle_up_ack }, // �ն�ͨ��Ӧ��
	{ DSMAPP_SRV_JTT_CMD_UP_HEARTBEAT, dsm_srv_cmd_handle_up_heartbeat }, // �ն�����
	{ DSMAPP_SRV_JTT_CMD_UP_REGISTER, dsm_srv_cmd_handle_up_register }, // �ն�ע��
	{ DSMAPP_SRV_JTT_CMD_UP_DEREGISTER, dsm_srv_cmd_handle_up_deregister }, // �ն�ע��
	{ DSMAPP_SRV_JTT_CMD_UP_AUTHORIZE, dsm_srv_cmd_handle_up_authorize }, // �ն˼�Ȩ
	{ DSMAPP_SRV_JTT_CMD_UP_QUERY_PARA_ACK, dsm_srv_cmd_handle_up_query_para_ack }, // ��ѯ�ն˲���Ӧ��
	{ DSMAPP_SRV_JTT_CMD_UP_QUERY_PROP_ACK, dsm_srv_cmd_handle_up_query_prop_ack }, // ��ѯ�ն�����Ӧ��
	{ DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT, dsm_srv_cmd_handle_up_loc_report }, // λ����Ϣ�㱨
	{ DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT_EXTERN, dsm_srv_cmd_handle_up_loc_report_extern}, // λ����Ϣ�㱨
	{ DSMAPP_SRV_JTT_CMD_UP_LOC_QUERY_ACK, dsm_srv_cmd_handle_up_loc_query_ack }, // λ����Ϣ��ѯӦ��
	{ DSMAPP_SRV_JTT_CMD_UP_EVENT_REPORT, dsm_srv_cmd_handle_up_event_report }, // �¼�����

	{ DSMAPP_SRV_JTT_CMD_UP_BATTERY, dsm_srv_cmd_handle_up_battery}, // ������Ϣ
	{ DSMAPP_SRV_JTT_CMD_UP_CARD_LOGIN, dsm_srv_cmd_handle_up_card_login}, // ��Ƭ����

	{ DSMAPP_SRV_JTT_CMD_DOWN_ACK, dsm_srv_cmd_handle_down_ack }, // ƽ̨ͨ��Ӧ��
	{ DSMAPP_SRV_JTT_CMD_DOWN_REGISTER_ACK, dsm_srv_cmd_handle_down_register_ack }, // �ն�ע��Ӧ��
	{ DSMAPP_SRV_JTT_CMD_DOWN_SET_PARA, dsm_srv_cmd_handle_down_set_para }, // �����ն˲���
	{ DSMAPP_SRV_JTT_CMD_DOWN_QUERY_PARA, dsm_srv_cmd_handle_down_query_para }, // ��ѯ�ն˲���
	{ DSMAPP_SRV_JTT_CMD_DOWN_CONTROL, dsm_srv_cmd_handle_down_control }, // �ն˿���
	{ DSMAPP_SRV_JTT_CMD_DOWN_QUERY_SPEC_PARA, dsm_srv_cmd_handle_down_query_spec_para }, // ��ѯָ���ն˲���
	{ DSMAPP_SRV_JTT_CMD_DOWN_QUERY_PROP, dsm_srv_cmd_handle_down_query_prop }, // ��ѯ�ն�����
	{ DSMAPP_SRV_JTT_CMD_DOWN_LOC_QUERY, dsm_srv_cmd_handle_down_loc_query }, // λ����Ϣ��ѯ
	{ DSMAPP_SRV_JTT_CMD_DOWN_EVENT_SET, dsm_srv_cmd_handle_down_event_set }, // �¼�����
#ifdef __WHDSM_CALL_SUPPORT__
	{ DSMAPP_SRV_JTT_CMD_DOWN_CALL_MONITOR, dsm_srv_cmd_handle_down_call_monitor }, // �绰�ز�
	{ DSMAPP_SRV_JTT_CMD_DOWN_PHONEBOOK, dsm_srv_cmd_handle_down_phonebook }, // ���õ绰��
#endif
#ifdef __WHDSM_JTT_FENCE_SUPPORT__
#endif
	{ DSMAPP_SRV_JTT_CMD_INVALID, NULL }
};





// ת�崦�����ش���󳤶�
static kal_int32 dsm_srv_data_escape(kal_uint8 *dat, kal_uint32 in_len)
{
	kal_uint32 idx, buf_idx;
	kal_uint8 buf[DSMAPP_JTT_BUFF_MAX];
	kal_uint8 temp;

	buf_idx = 0;
	for (idx = 0; idx < in_len; idx++)
	{
		temp = dat[idx];
		if (temp == 0x7e)
		{
			buf[buf_idx++] = 0x7d;
			buf[buf_idx++] = 0x02;
		}
		else if (temp == 0x7d)
		{
			buf[buf_idx++] = 0x7d;
			buf[buf_idx++] = 0x01;
		}
		else
		{
			buf[buf_idx++] = temp;
		}
	}

	kal_mem_cpy(dat, buf, buf_idx);
	return buf_idx;
}

// ������Ϣ ���ݴ���
// ��ʽ: ��ʶλ+��Ϣͷ+��Ϣ��+У����+��ʶλ
// ����: ��Ϣ��װ����>�������У���롪��>ת��
static kal_int32 dsm_srv_data_package(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_int32 ret = 0;
	kal_uint32 idx = 0;
	kal_uint8 *buf_l = out;
	kal_uint16 cmd_l = cmd;
	kal_uint8 cs = 0; // У����
	kal_uint16 buf_len = 0; // ��Ϣͷ+��Ϣ��+У���볤��
	//unsigned char backup_cell_num[] = {0x01, 0x00, 0x00, 0x00, 0x00,0x01};

	if ((in == NULL) || (out == NULL)) return -1;

	/*��ʶλ*/
	buf_l[0] = FRAME_FLAG_SYMBOL;

	/*��Ϣͷ-START*/
	/*��ϢID*/
	buf_l[1] = (cmd_l >> 8);
	buf_l[2] = cmd_l & 0xFF;

	/*��Ϣ�����ԣ����ȣ�*/
	buf_l[3] = (in_len >> 8);
	buf_l[4] = in_len & 0xFF;

	/*�ն��ֻ���*/
	for (idx = 0; idx < 6; idx++)
	{
		buf_l[5 + idx] = jtt_config.cell_num[idx]; // ��λ��ǰ
		//buf_l[5 + idx] = backup_cell_num[idx];
	}
	
	/*��Ϣ��ˮ�ţ���0��ʼѭ���ۼӣ�*/
	buf_l[11] = (serial_no_up >> 8);
	buf_l[12] = serial_no_up & 0xFF;
	login_status.flow = serial_no_up; // ����ؼ���Ϣ��ˮ��
	serial_no_up++; // ����

	/*��Ϣ����װ��-��*/
	/*��Ϣͷ-END*/

	/*��Ϣ��*/
	if (in_len != 0)
	{
		kal_mem_cpy(&buf_l[13], in, in_len);
	}

	/*У����*/
	cs = buf_l[1];
	for (idx = 2; idx < (13 + in_len); idx++)
	{
		cs ^= buf_l[idx];
	}
	buf_l[13 + in_len] = cs;

	/*ת��*/
	buf_len = 13 + in_len;
	buf_len = dsm_srv_data_escape(&buf_l[1], buf_len);

	/*��ʶλ*/
	buf_l[buf_len + 1] = FRAME_FLAG_SYMBOL;

	ret = buf_len + 2; // ��Ϣ�ܳ���

	dsmapp_trace("--->>dsm_srv_data_package(cmd=%04x,msg_len=%d,len=%d,cs=0x%x,next_flow_no=%d)\n", cmd_l, buf_len, ret, cs, serial_no_up);
	return ret;
}

// �ն˷�����Ϣ��ƽ̨
static kal_int32 dsm_srv_send_handle(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *para, kal_uint32 para_len)
{
//	static kal_uint32 send_len_bak = 0;
	dsm_cmd_handle_struct *func_l = NULL;
	kal_uint8 *buf_tmp = g_s8TmpBuf;
	kal_uint8 *buf_tx = g_s8TxBuf;
	kal_uint32 idx = 0;
	kal_int32 ret = 0;

	if (shutdown_is_active) return -1;

	// get cmd-function pointer
	while (cmd_func[idx].cmd != DSMAPP_SRV_JTT_CMD_INVALID)
	{
		if (cmd_func[idx].cmd == cmd)
		{
			func_l = &cmd_func[idx];
			break;
		}
		idx++;
	}

	// no corresponding cmd-function
	if (!func_l)
	{
		dsmapp_trace("--->>dsm_srv_send_handle(cmd[%04x] undef!!!!)\n", cmd);

		return -1;
	}

	dsmapp_trace("--->>dsm_srv_send_handle([%d]=%04x,%04x)\n", idx, func_l->cmd, cmd);

	//	if(KAL_TRUE == dsm_srv_call_connected()) return -1;

	if (func_l->handle)
	{
		kal_mem_set(buf_tmp, 0, sizeof(g_s8TmpBuf));
		// execute cmd-function handler
		ret = func_l->handle(cmd, para, para_len, buf_tmp);

		if (ret >= 0)
		{
			kal_mem_set(buf_tx, 0, sizeof(g_s8TxBuf));
			ret = dsm_srv_data_package(cmd, buf_tmp, ret, buf_tx); // ��װ��������Ϣ
//			send_len_bak = ret;
			if (/*(g_s32FirstConnect == 0)&&*/(ret > 0))
			{
				// upload to server to fixed by frex
				dsmapp_srv_send(buf_tx, ret, dsm_srv_send_handle_callback);
				dsm_srv_send_handle_callback = NULL;
			}
#if defined(__WHDSM_LOG_SRV_SUPPORT__)
			{
				kal_char *head = NULL;
				kal_char *pbuf = NULL;
				kal_uint32 idx = 0;

				head = dsm_log_srv_write_buf();
				if (head)
				{
					pbuf = head;
					kal_sprintf(pbuf, "dsm_srv_send_handle[%d][%04x]:", ret, cmd);
					pbuf += strlen(pbuf);

					while(idx < ret)
					{
						kal_sprintf(pbuf, "%02x ", buf_tx[idx]);
						pbuf += 3;
						idx++;
					}
					
					dsm_log_srv_write(head, strlen(head), KAL_TRUE);
					for (idx = 0; idx < strlen(head);)
					{
						dsmapp_trace("%s\r\n", (head + idx));
						idx += 127;
					}
				}
			}
#endif
		}
	}

	return ret;
}


// ��ת�崦�����ش���󳤶�
static kal_int32 dsm_srv_data_de_escape(kal_uint8 *dat, kal_uint32 in_len)
{
	kal_uint32 idx, buf_idx;
	kal_uint8 buf[DSMAPP_JTT_BUFF_MAX];
	kal_uint8 temp1, temp2;

	buf_idx = 0;
	for (idx = 0; idx < in_len;)
	{
		temp1 = dat[idx];
		temp2 = 0;
		if (idx < in_len - 1)
		{
			temp2 = dat[idx + 1];
		}
		
		if (temp1 == 0x7d && temp2 == 0x02)
		{
			buf[buf_idx++] = 0x7e;
			idx += 2;
		}
		else if (temp1 == 0x7d && temp2 == 0x01)
		{
			buf[buf_idx++] = 0x7d;
			idx += 2;
		}
		else
		{
			buf[buf_idx++] = temp1;
			idx += 1;
		}
	}

	kal_mem_cpy(dat, buf, buf_idx);
	return buf_idx;
}


// ������Ϣ ���ݴ���
// ��ʽ: ��ʶλ+��Ϣͷ+��Ϣ��+У����+��ʶλ
// ����: ��ת�塪��>��֤У���롪��>������Ϣ
// ������Ϣ�峤��
static kal_int32 dsm_srv_data_parse(kal_uint8 src, kal_uint8 *in, kal_uint32 in_len, kal_uint32 *cmd, kal_uint16 *flow, kal_uint8 *msg)
{
	kal_uint8 *buf = in;
	kal_int32 buf_len = -1; // ��ת��󳤶�
	kal_uint8 cs = 0; // У����
	kal_uint32 idx = 0;
	kal_uint16 msg_len = -1; // ��Ϣ�峤��

	if (src != 0) return -1; // srcΪ���ò�����Ĭ��Ϊ0
	if (!cmd || !flow || !msg) return -1; // ������

	/*����ʶλ*/
	if ((buf[0] != 0x7e) || (buf[in_len-1] != 0x7e)) return -1;

	/*��ת��*/
	buf_len = dsm_srv_data_de_escape(&buf[1], in_len - 2); // ��ȥ��ʶλ�ĳ���
	buf_len += 2; // ��ת����ܳ���

	/*��֤У����*/
	cs = buf[1];
	for (idx = 2; idx < buf_len - 2; idx++)
	{
		cs ^= buf[idx];
	}
	if (buf[buf_len - 2] != cs) return -1;

	/*��ϢID*/
	*cmd = (buf[1] << 8) | buf[2];

	/*��Ϣ�峤��*/
	msg_len = (buf[3] << 8) | buf[4];

	/*��Ϣ��ˮ��*/
	*flow = (buf[11] << 8) | buf[12];

	if ((msg_len & 0x2000) == 0) // ���ְ�
	{
		kal_mem_cpy(msg, &buf[13], msg_len); // ��Ϣ������
	}
	else // �ְ�
	{
	}

	return msg_len;
}



// �ն˽���ƽ̨��Ϣ
kal_int32 dsm_srv_receive_handle_jtt(kal_uint8 src, kal_uint8 *in, kal_int32 in_len)
{	
	kal_int32 ret = 0;
	kal_uint32 cmd = 0; // ��ϢID
	kal_uint16 flow = 0; // ��Ϣ��ˮ��
	kal_uint8 *msg = g_s8TmpBuf; // ������Ϣ������
	kal_uint8 *out = g_s8TxBuf; // �������
	dsm_cmd_handle_struct *func_l = NULL;
	kal_uint32 idx = 0;

	if ((in == NULL) || (in_len < 1)) return -1;

	kal_mem_set(msg, 0, sizeof(g_s8TmpBuf));
	ret = dsm_srv_data_parse(src, in, in_len, &cmd, &flow, msg);

	/*����CMD��Ӧ������*/
	while (cmd_func[idx].cmd != DSMAPP_SRV_JTT_CMD_INVALID)
	{
		if (cmd_func[idx].cmd == cmd)
		{
			func_l = &cmd_func[idx];
			break;
		}
		idx++;
	}
	if (!func_l)
	{
		dsmapp_trace("--->>dsm_srv_receive_handle(cmd[%04x] undef!!!!)\n", cmd);
	}
	else
	{
		dsmapp_trace("--->>dsm_srv_receive_handle([%d]=%04x,%04x)\n", idx, func_l->cmd, cmd);
	}

	kal_mem_set(out, 0, sizeof(g_s8TxBuf)); // ����

	/*ִ��CMD��Ӧ������*/
	if ((func_l) && (func_l->handle))
	{
		ret = func_l->handle((DSM_SRV_JTT_CMD_TYPE)cmd, msg, ret, out); // ע�����
	}

	/*�������Ӧ��*/
	if (cmd == DSMAPP_SRV_JTT_CMD_DOWN_SET_PARA // �����ն˲����������ն�ͨ��Ӧ��
		|| cmd == DSMAPP_SRV_JTT_CMD_DOWN_CONTROL) // �ն˿��ƣ������ն�ͨ��Ӧ��
	{
		if(ret != 0) ret = 1; // 0�ɹ� 1ʧ��
		
		out[0] = flow >> 8; // Ӧ����ˮ��
		out[1] = flow & 0xFF;
		out[2] = cmd >> 8; // Ӧ��ID
		out[3] = cmd & 0xFF;
		out[4] = ret; // ִ�н��

		ret = dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_ACK, out, 5);
	}
	else if (cmd == DSMAPP_SRV_JTT_CMD_DOWN_LOC_QUERY) // λ����Ϣ��ѯ������λ����Ϣ��ѯӦ��
	{
#if 0 // �ɶ�λ��Ӧ����srvinteraction_location_cbִ��Ӧ������
		kal_uint8 cmd_ack[2] = { 0 };

		cmd_ack[0] = flow >> 8; // Ӧ����ˮ��
		cmd_ack[1] = flow & 0xFF;

		ret = dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_LOC_QUERY_ACK, cmd_ack, 2);		
#else
		serial_no_loc_query_lastest = flow; // Ӧ����ˮ�ţ�ֻ�������һ����
#endif
	}
	else if (cmd == DSMAPP_SRV_JTT_CMD_DOWN_QUERY_SPEC_PARA
		|| cmd == DSMAPP_SRV_JTT_CMD_DOWN_QUERY_PARA) // ��ѯ�ն˲��������в�ѯ�ն˲���Ӧ��
	{
		out[0] = flow >> 8; // Ӧ����ˮ��
		out[1] = flow & 0xFF;

		ret = dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_QUERY_PARA_ACK, out, ret + 2);
	}
	else if (cmd == DSMAPP_SRV_JTT_CMD_DOWN_QUERY_PROP) // ��ѯ�ն����ԣ����в�ѯ�ն�����Ӧ��
	{
		ret = dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_QUERY_PROP_ACK, NULL, 0);
	}

	return ret;
}

//�ն�ע��
 kal_int32 dsm_srv_register_jtt(dsm_srv_cb reg_cb)
{
	dsmapp_trace("%s: enter", __FUNCTION__);

	// ������ˮ��
	serial_no_up = 0;
	serial_no_down = 0;
	serial_no_loc_query_lastest = 0;
	
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_REGISTER, NULL, 0);
	login_status.status = 3;
	StartTimer(DSMAPP_TIMER_JTT_SERVER_REGISTER, 100*1000, reg_cb); // 30s��ʱ
	return 0;
}

//�ն�ע��
static kal_int32 dsm_srv_deregister_jtt(dsm_srv_cb dereg_cb)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_DEREGISTER, NULL, 0);
	login_status.status = 2;
	StartTimer(DSMAPP_TIMER_JTT_SERVER_DEREGISTER, 30*1000, dereg_cb); // 30s��ʱ
	return 0;
}

//�ն˼�Ȩ
static kal_int32 dsm_srv_authorize_jtt(dsm_srv_cb auth_cb)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_AUTHORIZE, NULL, 0);
	login_status.status = 1;
	StartTimer(DSMAPP_TIMER_JTT_SERVER_AUTHORIZE, 90*1000, auth_cb); // 30s��ʱ
	return 0;
}

//�ն�����
kal_int32 dsm_srv_heartbeat_jtt(void)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_HEARTBEAT, NULL, 0);
	return 0;
}

// λ����Ϣ�㱨(������)
kal_int32 dsm_srv_loc_report_jtt(void)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT, NULL, 0);
	return 0;
}






kal_int32 dsm_srv_loc_report_jtt_extern(int warn_type, int file_type, const char *buf)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	unsigned int size = 2 +  1 + 1 + strlen(buf) + 1;
	char *send_buf = (char *)malloc( size ); 
	if (send_buf == NULL) {
		err("cann't rememory the report issue\n");
		return 0;
	}

	memset(send_buf, 0, size);

	int i = 0, j = 0;
	send_buf[i++] = 0xf0;
	send_buf[i++] = size;
	send_buf[i++] = (char)warn_type;
	send_buf[i++] = (char)file_type;
	for ( j = 0;j < strlen(buf)+1; j++)
		send_buf[i++] = buf[j];
	
	
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT_EXTERN,(unsigned char*) send_buf, size);

	if (send_buf != NULL){
		free( send_buf );
		send_buf = NULL;
	}
	return 0;
}





// �ն�ͨ��Ӧ��
kal_int32 dsm_srv_ack_jtt(kal_uint8 *para, kal_uint32 para_len)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_ACK, para, para_len);
	return 0;
}


// ���͵�����Ϣ��Ϣ
kal_int32 dsm_srv_send_battery_jtt(kal_bool poweroff)
{
	kal_uint8 bat = 0;
	dsmapp_trace("%s: enter", __FUNCTION__);
	if(poweroff)
	{
		dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_BATTERY, &bat, 1);
	}
	else
	{
		dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_BATTERY, NULL, 0);
	}
	return 0;
}

// ���Ϳ�Ƭ������Ϣ
kal_int32 dsm_srv_send_card_login_jtt(void)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_CARD_LOGIN, NULL, 0);
	return 0;
}


//��ȡ��Ȩ��
//������Ч��Ȩ�볤�ȣ�����0��ʾ��ȡʧ��
static kal_uint8 dsm_srv_get_auth_code(kal_uint8 * code)
{
	int i=0;
	char last_correct_athor_work = 0;
	dsmapp_trace("%s: enter", __FUNCTION__);

	last_correct_athor_work = 0;

	for(i=0; i<AUTH_CODE_LEN; i++){
		info("%x ", last_correct_autho_num[i]);
		if (last_correct_autho_num[i]!= 0)
			last_correct_athor_work = 1;
	}

	if(last_correct_athor_work == 0){
		jtt_config.auth_code_len = 0;

	}
	if ((jtt_config.auth_code_len != 0) && (code != NULL))
	{
		if (last_correct_athor_work){
			kal_mem_set(code, 0, AUTH_CODE_LEN);
		kal_mem_cpy(code, last_correct_autho_num, AUTH_CODE_LEN);
		kal_mem_cpy(jtt_config.auth_code, last_correct_autho_num, AUTH_CODE_LEN);
				jtt_config.auth_code_len = AUTH_CODE_LEN;

				info("last correct auth work, use the last correct auth num !\n");
		}
		else {
		kal_mem_set(code, 0, AUTH_CODE_LEN);
		kal_mem_cpy(code, jtt_config.auth_code, jtt_config.auth_code_len);

			}
	}

	info("dsm_srv_get_auth_code Autho Num: %d\n", jtt_config.auth_code_len);
	dsm_log(jtt_config.auth_code, jtt_config.auth_code_len);

	return jtt_config.auth_code_len;
}

// ����豸��(�ն��ֻ���)�Ƿ���Ч��ȫ0Ϊ��Ч������Ϊ��Ч
// ��Ч����KAL_TRUE
static kal_bool dsm_srv_check_cell_num(void)
{
	kal_uint8 idx;
	dsmapp_trace("%s: enter", __FUNCTION__);
	for(idx = 0; idx < DEVICE_NUM_LEN; idx++)
	{
		if(jtt_config.cell_num[idx] != 0)
		{
			dsmapp_trace("cell_num valid");
			return KAL_TRUE;
		}
	}
	
	dsmapp_trace("cell_num invalid");
	return KAL_FALSE;
}

kal_int32 dsmapp_srvinteraction_upload_location_info(ST_DSM_LOCATION_INFO *pLocateInfo)
{
	kal_int32 s32Ret = 0;
	kal_uint8 pos_info_type = 0;

	if(pLocateInfo == NULL)
	{
		pLocateInfo = dsm_location_get_latest_position();
	}

	pos_info_type = dsm_pos_info_type_get();
	
	// �ϱ�λ��
#ifdef IGNORE_ALARM_WHEN_LOC_INVALID
	dsmapp_trace("IGNORE_ALARM_WHEN_LOC_INVALID");
	if((pLocateInfo->valid) && (pLocateInfo->type == LOCATION_TYPE_GPS)) // GPS��Ч
#else
	if((pLocateInfo->valid && (pLocateInfo->type == LOCATION_TYPE_GPS)) // GPS��Ч
	|| (pos_info_type != 0x00)) // ������ͨ�ϱ�ʱ
#endif
	{
		if (pos_info_type == 0x10) // �����ϴ���λ����Ϣ��ѯ��
		{
			dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_LOC_QUERY_ACK, NULL, 0);
		}
		else
		{
			dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT, NULL, 0);
		}
	}
	else
	{
		dsmapp_trace("NO report: location type %d, pos_info_type %x", pLocateInfo->type, pos_info_type);
#ifdef IGNORE_ALARM_WHEN_LOC_INVALID
		dsm_pos_info_type_set(0x00); // �ָ���ͨ�ϴ�
#endif
	}

#if DSM_LBS_UPLOAD_FILTER
	if(pLocateInfo->valid)
	{
		if(pLocateInfo->type == LOCATION_TYPE_LBS)
		{
			dsm_location_lbs_check_repeat(1, &(pLocateInfo->info.lbs)); // set
		}
		else
		{
			dsm_location_lbs_check_repeat(1, NULL); // clear
		}
	}
#endif

	return s32Ret;
}

static kal_int8 srvinteraction_location_cb(ST_DSM_LOCATION_INFO *pParams)
{
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
	kal_int8 s8Ret = 0;
	dsmapp_trace("%s: enter", __FUNCTION__);

	dsmapp_srvinteraction_upload_location_info(pParams);

	if (g_s32PowerOff==1)
	{
		g_s32PowerOff = 2;/*!=0,,��ֹ���·���͵�ػ���λ����*/

		dsmapp_trace("srvinteraction poweroff");

		StopTimer(DSMAPP_TIMER_NET_UPLOAD);
		StartTimer(DSMAPP_TIMER_NET_UPLOAD, (10 * 1000), dsmapp_srvinteraction_poweroff);
	}

	return s8Ret;
#else
	dsmapp_srvinteraction_upload_location_info(pParams);
#endif
}


static void srvinteraction_location_request(kal_int8 type)
{
#if (DEBUG_IN_VS == 0) // ԭOneNet���룬�ɱ������˴�Ϊ����
	kal_prompt_trace(MOD_MMI, "srvinteraction_location_request (type=%d) --------------> \r\n", type);

	switch (type)
	{
	case 0:	// GNSS + LBS
		dsm_location_request(srvinteraction_location_cb);
		break;
	case 1: // LBS only
		//		dsm_location_request_LBS_only(srvinteraction_location_cb);
		break;
	case 2:
		//		dsm_location_request(srvinteraction_bootup_location_cb);
		break;
	case 3: // sos-1
		dsm_location_request_NO_GNSS(srvinteraction_location_cb);
		break;
	case 4: // sos-2
		dsm_location_request_ignore_movement(srvinteraction_location_cb);
		break;
	}
#else
	srvinteraction_location_cb(NULL);
#endif
}

// ע����ʱ������
void dsm_srv_deregister_cb(void )
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	
	info("\n============================== Deregister Follow End =============================\n\n\n");
}


// ע�ᳬʱ������
 void dsm_srv_register_cb(void )
{
	info("%s: enter", __FUNCTION__);
}

// ��Ȩ��ʱ������
void dsm_srv_authorize_cb(void )
{
	
	err("\n======= Authorize Follow End (Authorize Follow Failed : Timer timeout Callback)===========\n\n\n");
	dsmapp_trace("%s: enter", __FUNCTION__);
	
	
	info("\n\n\n============================== Deregister Follow Begin =============================\n");
	dsm_srv_deregister_jtt(&dsm_srv_deregister_cb);
}

// �����Ȩ��
void dsm_srv_auth_code_clear(void)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	err("not clear auth num now \n");
	return ;
	kal_mem_set(jtt_config.auth_code, 0, AUTH_CODE_LEN);
	jtt_config.auth_code_len = 0;
	dsm_srv_config_nv_write(2);
}
static nvram_ef_dsmapp_jtt_config_t auth_code_backup ;

kal_int32 dsm_srv_config_nv_write(kal_uint8 item)
{
	kal_int16 error=0;
	kal_int32 ret=0;
#if (DEBUG_IN_VS == 0)
	if(item == 1) // ���涨λģʽ
	{
	    ret = WriteRecord(
	            NVRAM_EF_DSMAPP_POS_MODE_LID,
	            1,
	            (kal_uint8*)&dsm_pos_mode,
	            NVRAM_EF_DSMAPP_POS_MODE_SIZE,
	            &error);
	}
	else if(item == 2) // �����豸����(SIM�š���Ȩ�볤�ȡ���Ȩ��)
	{
	    ret = WriteRecord(
	        NVRAM_EF_DSMAPP_JTT_CONFIG_LID,
	        1,
	        (kal_uint8*)&jtt_config,
	        NVRAM_EF_DSMAPP_JTT_CONFIG_SIZE,
	        &error);
	}
#endif


	printf("has been backup the auth code \n");
	
	memcpy(&auth_code_backup, &jtt_config, sizeof(nvram_ef_dsmapp_jtt_config_t));

	dsm_log(auth_code_backup.auth_code, AUTH_CODE_LEN);
	
	dsmapp_trace("%s(%d): ret=%d", __FUNCTION__, item, ret);
	return ret;
}


//static unsigned char defaul_cell_num[CELL_NUMBER_LEN] = {0x01,0x00,0x00,0x00,0x00,0x01 };

//static unsigned char defaul_cell_num[CELL_NUMBER_LEN] = {0x01,0x00,0x00,0x00,0x00,0x02 };

static unsigned char defaul_cell_num[CELL_NUMBER_LEN] = {0x01,0x00,0x00,0x00,0x00,0x03 };


//static unsigned char defaul_cell_num[CELL_NUMBER_LEN] = {0x09,0x00,0x00,0x00,0x00,0x74 };

kal_int32 dsm_srv_config_nv_read(void)
{
	static kal_uint8 dsm_nv_read = 1;
	kal_int16 error = 0;
	kal_int32 ret = -1;
	kal_uint8 idx;
	// unsigned char buff[AUTH_CODE_LEN] = {0x47,0x4e,0x53,0x53,0x5f,0x30,0x39,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x37,0x34,0x5f,0x31,0x31,0x30,0x34,0x30,0x35,0x38};/*{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x31,0x30,0x34,0x30,0x35,0x38,0x02,0xBE,0xA9,0x42,0x31,0x32,0x33,0x34,
	//0x35,0x36,0x37};*/

	

	if(dsm_nv_read)
	{
		dsm_nv_read = 0;
		dsmapp_trace("%s: enter", __FUNCTION__);
#if (DEBUG_IN_VS == 0)
		// ��ȡ�豸����(SIM�š���Ȩ�볤�ȡ���Ȩ��)
		memset(&jtt_config, 0, sizeof(jtt_config));
		ret = ReadRecord(
				NVRAM_EF_DSMAPP_JTT_CONFIG_LID,
				1,
				(kal_uint8*)&jtt_config,
				NVRAM_EF_DSMAPP_JTT_CONFIG_SIZE,
				&error);

#ifdef USE_IMEI_AS_CELL_NUM
	{
		char *imei_p;
		kal_uint8 idx, i_imei;
		
		dsmapp_trace("USE_IMEI_AS_CELL_NUM");
		
		// ��ȡIMEI����BCD�뱣�浽device_num_st.cell_num
		imei_p = dsmapp_get_imei();

#ifdef USE_JTT808_SERVER_1_SIM_DEVICE
		imei_p = "000012000187148";
#endif
		for (idx = 0; idx < DEVICE_NUM_LEN; idx++)
		{
			i_imei = 3 + idx * 2; // ȡIMEI��12λ
			jtt_config.cell_num[idx] = ((imei_p[i_imei] - '0') << 4) + (imei_p[i_imei + 1] - '0'); // BCD��
		}
	}
#endif

		info("cell_num:\n");
		memcpy(jtt_config.cell_num, defaul_cell_num, CELL_NUMBER_LEN);
		for(idx = 0; idx < DEVICE_NUM_LEN; idx++)
		{
			info("%x", jtt_config.cell_num[idx]);
		}
		
		info("\nauth_code(%d):\n", jtt_config.auth_code_len);
		for(idx = 0; idx < jtt_config.auth_code_len; idx++)
		{
			//jtt_config.auth_code[idx] = buff[idx];
			info("%x", jtt_config.auth_code[idx]);
		}

		info("\n");

#if 0
		// ������������������ַ
		dsmapp_srv_heart_set(jtt_config.heart);
		dsmapp_srv_address_set(&jtt_config.srv_ip[0], jtt_config.srv_port);
#endif

		// ��ȡ��λģʽ
        memset(&dsm_pos_mode , 0 , sizeof(nvram_ef_dsmapp_pos_mode_t));
        ret = ReadRecord(
                NVRAM_EF_DSMAPP_POS_MODE_LID,
                1,
                (kal_uint8*)&dsm_pos_mode,
                NVRAM_EF_DSMAPP_POS_MODE_SIZE,
                &error);
		dsmapp_trace("dev_mode = %d (0-low 1-normal 2-continue 3-auto)", dsm_pos_mode.dev_mode);
		dsmapp_trace("custom period = %ds", dsm_pos_mode.custom_period);

		// ���ö�λģʽ������
		dsm_pos_mode_set(dsm_pos_mode.dev_mode);

#endif

		//memcpy(auth_code_backup.auth_code, buff, AUTH_CODE_LEN);
		//memcpy(jtt_config.auth_code, buff/*auth_code_backup.auth_code*/, AUTH_CODE_LEN);
		/*090000000074 */

		//unsigned char backup_cell_num[] = {0x09, 0x00, 0x00, 0x00, 0x00,0x74};


		info("cell_num:\n");
		memcpy(jtt_config.cell_num, defaul_cell_num, CELL_NUMBER_LEN);
				for(idx = 0; idx < DEVICE_NUM_LEN; idx++)
				{
					info("%x", jtt_config.cell_num[idx]);
				}
		
			
				


		char last_correct_auth_work = 0;

		for(idx = 0; idx < AUTH_CODE_LEN; idx++)
		{
			if(last_correct_autho_num[idx] != 0){
				info("last correct authorize code is not NULL\n");
				last_correct_auth_work = 1;
			}

		}



		if (last_correct_auth_work){
			info("copy auth num from last correct auth_num array !!!!!!\n");
			memcpy(jtt_config.auth_code, last_correct_autho_num, AUTH_CODE_LEN);
		}
		else 
			err("\nlast correct authorize code  is NULL, so we need stat Register Follow to fetch Authorize code.\n");
		

		

		info("cell_num:\n");
		for(idx = 0; idx < DEVICE_NUM_LEN; idx++)
		{
			info("%x", jtt_config.cell_num[idx]);
		}

		
		dsmapp_trace("\nauth_code(%d):\n", jtt_config.auth_code_len);
		dsm_log(jtt_config.auth_code, AUTH_CODE_LEN);
	 
	}

	return ret;
}

// �������Ȩ��ע������
void dsmapp_srvinteraction_connect(kal_int32 s32Level)
{
	kal_int32 ret = -1;
	kal_uint8 code[AUTH_CODE_LEN] = { 0 };

	dsmapp_trace("%s(JTT): enter", __FUNCTION__);

	dsm_srv_config_nv_read(); // ��ȡNV��(�豸�š���Ȩ�롢��λģʽ)

	if(!dsm_srv_check_cell_num()) // �ն��ֻ�����Ч
	{
		dsmapp_trace("%s(JTT): stop", __FUNCTION__);
		return;
	}

	ret = dsm_srv_get_auth_code(&code[0]); // ��ȡ��Ȩ��
	if (ret > 0)
	{
		
		info("\n\n\n============================== Authorize Follow Begin =============================\n");
		dsm_srv_authorize_jtt(dsm_srv_authorize_cb); // ��Ȩ
	}
	else
	{
		
		info("\n\n\n============================== Register Follow Begin =============================\n");
		dsm_srv_register_jtt(dsm_srv_register_cb); // ע��
	}

	dsmapp_trace("%s(JTT): leave", __FUNCTION__);
}

void srvinteraction_bootup_location_request(void)
{
	dsmapp_trace("%s(JTT): enter", __FUNCTION__);

	srvinteraction_location_request(0);
}

void dsmapp_srvinteraction_first_location(void)
{
	ST_DSM_LOCATION_INFO *pos_info=dsm_location_get_latest_position();

	if (pos_info == NULL){
		printf("location info cann't fetch\n");
		return;
	}

	if(g_s32FirstConnect == 0)
	{
		return; // not first connect
	}
	else
	{
		g_s32FirstConnect = 0;
	}

	dsmapp_trace("%s(JTT): enter", __FUNCTION__);
	
	if((pos_info->valid == KAL_TRUE))
	{
		dsmapp_trace("GPS already valid, no need LBS or AGPS");
	}
	else
	{
		dsm_location_check_and_start_agps(); // added
	}
}

void dsmapp_srvinteraction_sos(void)
{
	if(g_s32PowerOff) return;/*�ػ����̲���ӦSOS*/
	dsm_pos_info_type_set(0x80);
	srvinteraction_location_request(0);
}

// �ػ�
void dsmapp_srvinteraction_poweroff(void )
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsmapp_sys_shutdown();
}

// TODO: ��λ
static void dsmapp_srvinteraction_reset(void)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
}

// �����¼����� �����ڱ���͵���¼�
kal_int32 dsmapp_srvinteraction_send_event(SRV_EVENT_ID evt)
{
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_EVENT_REPORT, (kal_uint8*)&evt, 1);
	return 0;
}

// TODO: 5%�͵�(��λ)�ػ�
void dsmapp_srvinteraction_locate_and_poweroff(void)
{
	// 5%����ʱ����
	// ���滻Ϊ�������¼����� dsmapp_srvinteraction_send_event(DSMAPP_SRV_JTT_EVENT_NO_POWER)
}

// APPԶ��(��λ)�ػ�
void dsmapp_srvinteraction_locate_and_poweroff_remote(ST_DSM_LOCATION_INFO *loc)
{
	return;
}

kal_int32 dsmapp_srvinteraction_uploader_pos_mode(void)
{
	return 0;
}

kal_int32 dsmapp_srvinteraction_uploader_config(void)
{
	return 0;
}

kal_int32 dsmapp_srvinteraction_uploader_batt_info(void)
{
	return 0;
}

kal_int32 dsmapp_srvinteraction_send_battery_warning(void)
{
	// 20%(10%)����ʱ����
	// ���滻Ϊ�������¼����� dsmapp_srvinteraction_send_event(DSMAPP_SRV_JTT_EVENT_LOW_POWER)
	ST_DSM_LOCATION_INFO *pLocateInfo;
	dsmapp_trace("%s: enter", __FUNCTION__);
	dsm_pos_info_type_set(0x90);
	
#ifdef IGNORE_ALARM_WHEN_LOC_INVALID
	pLocateInfo = dsm_location_get_latest_position();
	
	dsmapp_trace("IGNORE_ALARM_WHEN_LOC_INVALID");
	if((pLocateInfo->valid) && (pLocateInfo->type == LOCATION_TYPE_GPS)) // GPS��Ч
#endif
	{
		dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_LOC_REPORT, NULL, 0);
	}
#ifdef IGNORE_ALARM_WHEN_LOC_INVALID
	else
	{
		dsmapp_trace("NO report: location type %d", pLocateInfo->type);
		dsm_pos_info_type_set(0x00); // �ָ���ͨ�ϴ�
	}
#endif
	return 0;
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


#if 0//(DEBUG_IN_VS == 1)
void main(void)
{
	kal_uint8 ret;
	kal_uint8 g_s8RxBuf[DSMAPP_JTT_BUFF_MAX] = { 0 };

	kal_uint8 buf_in9[] = { 0x7e, 0x81, 0x03, 0x00, 0x06, 0x71, 0x00, 0x90, 0x00, 0x09, 0x94, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x18, 0x04, 0x00, 0x00, 0x23, 0x9e, 0x58, 0x7e };
	ret = dsm_srv_receive_handle_jtt(0, buf_in9, sizeof(buf_in9));

	kal_uint8 buf_in8[] = { 0x7E, 0x81, 0x03, 0x00, 0x13, 0x01, 0x20, 0x00, 0x18, 0x71, 0x48, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x01, 0x3C, 0x00, 0x00, 0x09, 0x05, 0x04, 0x00, 0x00, 0x01, 0x0F, 0xAC, 0x7E };//����������0x0001��Ϊ316s��DWORD�������ö�λ�����0x0095��Ϊ271s��DWORD��
	ret = dsm_srv_receive_handle_jtt(0, buf_in8, sizeof(buf_in8));


	kal_uint8 buf_in7[] = { 0x7E, 0x81, 0x03, 0x00, 0x18, 0x01, 0x20, 0x00, 0x18, 0x71, 0x48, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x03, 0x09, 0x31, 0x32, 0x33, 0x2e, 0x31, 0x2e, 0x32, 0x2e, 0x33, 0x00, 0x00, 0x01, 0x08, 0x04, 0x00, 0x00, 0x11, 0xd7, 0x77, 0x7E };//����IPΪ123.1.2.3�����ö˿�Ϊ4567
	ret = dsm_srv_receive_handle_jtt(0, buf_in7, sizeof(buf_in7));

	dsmapp_srvinteraction_send_event(DSMAPP_SRV_JTT_EVENT_NO_POWER);

	kal_uint8 buf_in6[] = { 0x7E, 0x81, 0x06, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xF0, 0x01, 0x72, 0x7E };

	dsm_srv_receive_handle_jtt(0, buf_in6, sizeof(buf_in6)); // ��ѯָ���ն˲�����������

	kal_uint8 buf_in5[] = { 0x7E, 0x81, 0x07, 0x00, 0x00, 0x01, 0x20, 0x00, 0x18, 0x71, 0x48, 0x00, 0x02, 0x84, 0x7E };

	dsm_srv_receive_handle_jtt(0, buf_in5, sizeof(buf_in5)); // ��ѯ�ն�����:ok

	kal_uint8 buf_in4[] = { 0x7e, 0x81, 0x06, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x94, 0x00, 0x00, 0x00, 0x95, 0x8d, 0x7e };

	dsm_srv_receive_handle_jtt(0, buf_in4, sizeof(buf_in4)); // ��ѯָ���ն˲���:ok
	
	kal_uint8 buf_in3[] = { 0x7E, 0x81, 0x04, 0x00, 0x00, 0x01, 0x20, 0x00, 0x18, 0x71, 0x48, 0x00, 0x02, 0x87, 0x7E };

	dsm_srv_receive_handle_jtt(0, buf_in3, sizeof(buf_in3)); // ��ѯ�ն˲���:ok

	dsm_srv_cmd_handle_down_query_para(0,NULL,0,NULL);

	dsm_srv_deregister_jtt(dsm_srv_deregister_cb);

	dsm_srv_register_jtt(dsm_srv_register_cb);

	kal_uint8 buf_in2[] = { 0x7E, 0x81, 0x03, 0x00, 0x1A, 0x01, 0x20, 0x00, 0x18, 0x71, 0x48, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x04, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00, 0x48, 0x0B, 0x31, 0x33, 0x38, 0x30, 0x38, 0x36, 0x32, 0x38, 0x38, 0x36, 0x33, 0xD2, 0x7E };//�ڶ�������ID����
	ret = dsm_srv_receive_handle_jtt(0, buf_in2, sizeof(buf_in2));

	kal_uint8 buf_in[] = { 0x7E, 0x80, 0x01, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x25, 0x01, 0x00, 0x01, 0xB6, 0x7E }; // 0x8001 ƽ̨ͨ��Ӧ��
	ret = dsm_srv_receive_handle_jtt(0, buf_in, sizeof(buf_in));

	kal_uint8 buf_in1[] = { 0x7E, 0x81, 0x00, 0x00, 0x09, 0x02, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x25, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x15, 0xAD, 0x7E }; // 0x8100 �ն�ע��Ӧ��
	ret = dsm_srv_receive_handle_jtt(0, buf_in1, sizeof(buf_in1));



	kal_uint8 tmp0[] = { 0x30,0x7e,0x08,0x7d,0x55 };
	ret = dsm_srv_data_escape(tmp0, sizeof(tmp0));
	ret = dsm_srv_data_de_escape(tmp0, ret);

	dsm_srv_send_handle(DSMAPP_SRV_JTT_CMD_UP_REGISTER, NULL, 0);

	memset(g_s8TxBuf, 1, DSMAPP_JTT_BUFF_MAX);
	dsm_srv_cmd_handle_up_register(0, NULL, 0, g_s8TxBuf);

	kal_uint8 tmp[] = { 0x00, 0x25, 0x00, 0x02, 0x31, 0x32, 0x30, 0x30, 0x30, 0x31, 0x38, 0x37, 0x31, 0x34, 0x38 };
	memcpy(g_s8RxBuf, tmp, sizeof(tmp));
	dsm_srv_cmd_handle_down_register_ack(0, g_s8RxBuf, sizeof(tmp), NULL);

	dsm_srv_cmd_handle_up_authorize(0, NULL, 0, g_s8TxBuf);

	dsm_srv_cmd_handle_up_loc_report(0, NULL, 0, g_s8TxBuf);

	kal_uint8 tmp1[] = { 0x00, 0x02, 0x02, 0x00, 0x04 };
	memcpy(g_s8RxBuf, tmp1, sizeof(tmp));
	dsm_srv_cmd_handle_down_ack(0, g_s8RxBuf, sizeof(tmp), NULL);
}
#endif

#endif

