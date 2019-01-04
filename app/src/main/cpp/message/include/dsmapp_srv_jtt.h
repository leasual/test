
#if 1
#ifndef __DSMAPP_SRV_JTT_H__
#define __DSMAPP_SRV_JTT_H__


#include "others.h"

#define SRV_USE_NVRAM 0 // ʹ��NV����
#define SRV_NO_REGISTER 0 // ��ע���Ȩ

#if (SRV_USE_NVRAM == 0)
#define JTT808_SERVER_1 1 // ƽ̨
#define JTT808_SERVER_1_DEBUG 2 // ƽ̨
#define JTT808_SERVER_2 3 // ƽ̨2
#define JTT808_SERVER_3 4 // ƽ̨2

#define MX_JTT808_SRV JTT808_SERVER_3

#if (MX_JTT808_SRV == JTT808_SERVER_1)
#define	DSMAPP_SRV_ADDR_IP		"221.204.237.94"
#define	DSMAPP_SRV_ADDR_PORT		9988

#elif (MX_JTT808_SRV == JTT808_SERVER_1_DEBUG)
#define	DSMAPP_SRV_ADDR_IP		"112.74.87.95"
#define	DSMAPP_SRV_ADDR_PORT		6968

#elif (MX_JTT808_SRV == JTT808_SERVER_2)
#define	DSMAPP_SRV_ADDR_IP		"139.196.164.147"
#define	DSMAPP_SRV_ADDR_PORT		9090

#elif (MX_JTT808_SRV == JTT808_SERVER_3)
//#define	DSMAPP_SRV_ADDR_IP		"47.101.52.88"
//#define	DSMAPP_SRV_ADDR_PORT		20048
//#define    DSMAPP_SRV_ADDR_IP        "112.64.117.214"
//#define    DSMAPP_SRV_ADDR_PORT        20001
#define    DSMAPP_SRV_ADDR_IP        "47.101.52.88"
#define    DSMAPP_SRV_ADDR_PORT        20048
#endif
#endif

#define DSMAPP_RX_BUFSIZE        1024

typedef void(*mx_srv_cb)(void);

void dsmapp_srv_connect(void);

kal_int32 dsmapp_srv_send(kal_uint8 *dat_in, kal_uint16 in_len, mx_srv_cb cb);

kal_int8 dsmapp_srv_heart_set(kal_uint16 heart_s);

kal_uint32 dsmapp_srv_heart_get(void);

void dsmapp_srv_address_set(kal_char *ip, kal_uint16 port);

void dsmapp_srv_address_get(kal_char *ip, kal_uint16 *port);

void dsmapp_srv_ind(kal_int32 hdl, kal_uint32 evt);

void dsmapp_srv_recv_handle(kal_uint8 *dat_in, kal_uint16 in_len);


kal_int32 dsm_srv_loc_report_jtt_extern(int warn_type, int file_type, const char *buf);



#endif
#endif
