#include <stdio.h>
#include <string.h>

#include "dsmapp_sms_custom_jtt.h"
#include "dsmapp_srvinteraction_jtt.h"
#include "dsmapp_srv_jtt.h"

#define	DSMAPP_JTT_BUFF_MAX	(384)

_GLIBCXX_BEGIN_EXTERN_C

static kal_uint8 g_s8TmpBuf[DSMAPP_JTT_BUFF_MAX];
static kal_uint8 g_s8TxBuf[DSMAPP_JTT_BUFF_MAX];


// �����ն��������ͼ��
// ���� 0:�ɹ� -1:ʧ��
static kal_int32 dsm_srv_set_para_heartbeat(kal_uint8 *in, kal_uint32 in_len)
{
	kal_int8 ret = -1;
	kal_uint8 idx = 0;
	kal_uint32 heartbeat = 0; // unit: s

	if (!in || in_len == 0) return -1;

	while (idx < in_len)
	{
		heartbeat = heartbeat * 10 + (in[idx++] - '0');
	}

	// �����ն��������ͼ��
	ret = dsmapp_srv_heart_set(heartbeat);

	return ret;
}

// ���÷�����IP
// ���� 0:�ɹ� -1:ʧ��
static kal_int32 dsm_srv_set_para_srv_ip(kal_uint8 *in, kal_uint32 in_len)
{
	kal_uint8 srv_ip[16] = { 0 };

	if (!in || in_len == 0) return -1;

	kal_mem_cpy(srv_ip, in, in_len);
	dsmapp_srv_address_set((kal_char*)srv_ip, 0);

	return 0;
}

// ���÷�����TCP�˿�
// ���� 0:�ɹ� -1:ʧ��
static kal_int32 dsm_srv_set_para_srv_tcp_port(kal_uint8 *in, kal_uint32 in_len)
{
	kal_uint8 idx = 0;
	kal_uint32 srv_port = 0;

	if (!in || in_len == 0) return -1;

	while (idx < in_len)
	{
		srv_port = srv_port * 10 + (in[idx++] - '0');
	}

	dsmapp_srv_address_set(NULL, srv_port);

	return 0;
}

// ���ö�λ�����ϴ�����
// ����: 0-�ɹ�
// �ο�dsm_srv_cmd_handle_down_period
static kal_int32 dsm_srv_set_para_loc_prop(kal_uint8 *in, kal_uint32 in_len)
{
	kal_int8 ret = -1;
	kal_uint8 idx = 0;
	kal_uint32 loc_prop = 0; // unit: s

	if (!in || in_len == 0) return -1;

	while (idx < in_len)
	{
		loc_prop = loc_prop * 10 + (in[idx++] - '0');
	}

	ret = dsm_pos_period_set(loc_prop);

	return ret;
}

static dsm_set_para_handle_struct set_para_func[] =
{
	{ DSMAPP_SRV_JTT_PARA_HEARTBEAT, dsm_srv_set_para_heartbeat }, // �ն��������ͼ��
	{ DSMAPP_SRV_JTT_PARA_SRV_IP, dsm_srv_set_para_srv_ip }, // ������IP
	{ DSMAPP_SRV_JTT_PARA_SRV_TCP_PORT, dsm_srv_set_para_srv_tcp_port }, // ������TCP�˿�
	{ DSMAPP_SRV_JTT_PARA_LOC_PROP, dsm_srv_set_para_loc_prop }, // ��λ�����ϴ�����

	{ DSMAPP_SRV_JTT_PARA_INVALID, NULL }
};

// �����ն˲���
// ��ʽ������ID,����ֵ,����ID,����ֵ...
// ����: 0-�ɹ�
static kal_int32 dsm_srv_cmd_handle_down_set_para(DSM_SRV_JTT_CMD_TYPE cmd, kal_uint8 *in, kal_uint32 in_len, kal_uint8 *out)
{
	kal_uint8 *dat_l = in;
	kal_uint8  * p = NULL;
	kal_uint32 tmp;

	kal_uint32 para_id = 0; // ����ID
	kal_uint8 para_len = 0; // ��������
	kal_uint8 *para = NULL; // ����ֵ

	dsm_set_para_handle_struct *func_l = NULL;
	kal_uint32 idx = 0;
	kal_int32 ret = -1;
	const char *next_char_tag =  ",";

	while ((dat_l - in) < in_len)
	{
		/*����ID*/
		para_id = 0;
		tmp = *dat_l++ - '0';
		para_id |= (tmp << 12);
		tmp = *dat_l++ - '0';
		para_id |= (tmp << 8);
		tmp = *dat_l++ - '0';
		para_id |= (tmp << 4);
		tmp = *dat_l++ - '0';
		para_id |= tmp;
	
		if (*dat_l++ != ',') return -1;
	
		/*����ֵ*/
		para = dat_l++;

		/*����ֵ����*/
		//p = strstr(dat_l, ",");  //next ,
		p = (kal_uint8*)strstr((char*)dat_l, ","/*next_char_tag*/);  //next ','
		if (!p)
		{
			para_len = in_len - (para - in);
		}
		else
		{
			para_len = p - para;
			
		}

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
	}

	return ret; // �������þ��ɹ�
}

static dsm_cmd_handle_struct cmd_func[] =
{
	{ DSMAPP_SRV_JTT_CMD_DOWN_SET_PARA, dsm_srv_cmd_handle_down_set_para }, // �����ն˲���

	{ DSMAPP_SRV_JTT_CMD_INVALID, NULL }
};

// ������Ϣ ���ݴ���
// ��ʽ: ��ϢID,��Ϣ��ˮ��,����ID,��Ϣ��
// ������Ϣ�峤��
static kal_int32 dsm_srv_data_parse(kal_uint8 src, kal_uint8 *in, kal_uint32 in_len, kal_uint32 *cmd, kal_uint16 *flow, kal_uint8 *msg)
{
	kal_uint8 *buf = in;
	kal_uint32 idx = 0;
	kal_uint8 tmp = 0;
	kal_uint8 *p = NULL;
	kal_uint16 msg_len = -1; // ��Ϣ�峤��

	if (src != 1) return -1; // Ĭ��Ϊ7BIT��ʽ src = (SRV_SMS_DCS_7BIT == dcs)?1:2
	if (!cmd || !flow || !msg) return -1; // ������

	/*��ϢID ʮ������*/
	*cmd = 0;
	tmp = buf[idx++] - '0';
	*cmd += tmp << 12;
	tmp = buf[idx++] - '0';
	*cmd += tmp << 8;
	tmp = buf[idx++] - '0';
	*cmd += tmp << 4;
	tmp = buf[idx++] - '0';
	*cmd += tmp;

	p = (kal_uint8 *)strstr((char *)buf, ","); // first ','
	if (!p)	return -1;
	
	/*��Ϣ��ˮ�� ʮ����*/
	buf = p + 1;
	idx = 0;
	*flow = 0;
	while (buf[idx] != ',')
	{
		*flow = (*flow * 10) + (buf[idx++] - '0');
	}

	p = (kal_uint8 *)strstr((char *)buf, ","); // next ','
	if (!p)	return -1;

	/*��Ϣ�� �ַ���*/
	msg_len = in_len - (p + 1 - in);
	kal_mem_cpy(msg, p + 1, msg_len);

	return msg_len;
}

kal_int32 mx_sms_custom_jtt_handle(kal_uint8 src, kal_uint8 *in, kal_int32 in_len)
{
	kal_int32 ret = -1;
	kal_uint32 cmd = 0; // ��ϢID
	kal_uint16 flow = 0; // ��Ϣ��ˮ��
	kal_uint8 *msg = g_s8TmpBuf; // ������Ϣ������
	kal_uint8 *out = g_s8TxBuf; // �������
	dsm_cmd_handle_struct *func_l = NULL;
	kal_uint32 idx = 0;

	if ((in == NULL) || (in_len < 1)) return -1;

	kal_mem_set(msg, 0, sizeof(g_s8TmpBuf));

	/*������������ �õ�CMD ��ˮ�� ��Ϣ��*/
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

	kal_mem_set(out, 0, sizeof(g_s8TxBuf)); // ����

	/*ִ��CMD��Ӧ������*/
	if ((func_l) && (func_l->handle))
	{
		ret = func_l->handle((DSM_SRV_JTT_CMD_TYPE)cmd, msg, ret, out); // ע�����
	}

	/*�������Ӧ��*/
	if (cmd == DSMAPP_SRV_JTT_CMD_DOWN_SET_PARA) // �����ն�ͨ��Ӧ��
	{
		if(ret != 0) ret = 1; // 0�ɹ� 1ʧ��
		
		out[0] = flow >> 8; // Ӧ����ˮ��
		out[1] = flow & 0xFF;
		out[2] = cmd >> 8; // Ӧ��ID
		out[3] = cmd & 0xFF;
		out[4] = ret; // ִ�н��

		dsm_srv_ack_jtt(out, 5);
	}

	return 0;
}

_GLIBCXX_END_EXTERN_C
