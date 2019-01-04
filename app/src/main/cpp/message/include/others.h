#ifndef __OTHERS_H__
#define __OTHERS_H__

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>



//#include <android/log.h>

//#define LOG_TAG "OpenCV-NDK-Native"
//#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

//#include "timer.h"


//#include <mutex>
//#include <condition_variable>
//#include <atomic>




#include "VS_kal_general_types.h"

#include <iostream>
#include <signal.h>
//#include <Config.h>
//#include "common.h"


#include <iostream>
#include <time.h>
//#include <boost/asio.hpp>
//#include <boost/bind.hpp>
//#include <boost/function.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>




/* Color log print  */
#define RESET           0
#define BRIGHT          1
#define DIM             2
#define UNDERLINE       4
#define BLINK           5
#define REVERSE         7
#define HIDDEN          8

#define BLACK           0
#define RED             1
#define GREEN           2
#define YELLOW          3
#define BLUE            4
#define MAGENTA         5
#define CYAN            6
#define WHITE           7




void textcolor(int attr, int fg, int bg);
void my_vprint(char* fmt, va_list va_args);
void _print_color(int attr, int color, const char* fmt,...);


#define print_color(attr, color, fmt,...) _print_color(attr, color, fmt, ##__VA_ARGS__)
#define succ(fmt, ...) _print_color(BRIGHT, BLUE, fmt, ##__VA_ARGS__)
#define warn(fmt, ...) _print_color(BRIGHT, YELLOW, fmt, ##__VA_ARGS__)
#define err(fmt, ...) _print_color(BRIGHT, RED, fmt, ##__VA_ARGS__)
#define info(fmt, ...) _print_color(BRIGHT, GREEN, fmt, ##__VA_ARGS__)
#define alarm(fmt, ...) _print_color(BRIGHT, CYAN, fmt, ##__VA_ARGS__)




 
using namespace std;
//namespace ba = boost::asio;



 
typedef void (*callback_t)(void );



typedef enum {
	DSMAPP_TIMER_NULL = 0,
	DSMAPP_TIMER_RECONNECT_SERVICE,
	DSMAPP_TIMER_INTERACTION_SERVICE,
	DSMAPP_TIMER_CMD_TIMEOUT_SERVICE,
	DSMAPP_TIMER_DISCONNECT_SERVICE,
	DSMAPP_TIMER_JTT_SERVER,
	DSMAPP_TIMER_JTT_SERVER_REGISTER,	
	DSMAPP_TIMER_JTT_SERVER_AUTHORIZE,
	DSMAPP_TIMER_JTT_SERVER_DEREGISTER,
	
	DSMAPP_TIMER_NET_UPLOAD,
	DSMAPP_TIMER_END,
} E_TIMER_TYPE ;


#define print_timer_type(t) do { switch(t){  	\
		case DSMAPP_TIMER_NULL: 		\
			info("DSMAPP_TIMER_NULL");		\
			break;						\
		case DSMAPP_TIMER_RECONNECT_SERVICE:	\
			info( "DSMAPP_TIMER_RECONNECT_SERVICE");	\
			break;						\
		case DSMAPP_TIMER_INTERACTION_SERVICE:	\
			info("DSMAPP_TIMER_INTERACTION_SERVICE\n");	\
			break;					\
		case DSMAPP_TIMER_DISCONNECT_SERVICE:	\
			info ("DSMAPP_TIMER_DISCONNECT_SERVICE\n");	\
			break;					\
		case DSMAPP_TIMER_JTT_SERVER:	\
			info("DSMAPP_TIMER_JTT_SERVER\n");	\
			break;						\
		case DSMAPP_TIMER_JTT_SERVER_REGISTER:	\
			info("DSMAPP_TIMER_JTT_SERVER_REGISTER\n");	\
			break;							\
		case DSMAPP_TIMER_JTT_SERVER_AUTHORIZE:	\
			info("DSMAPP_TIMER_JTT_SERVER_AUTHORIZE\n");	\
			break;							\
		case DSMAPP_TIMER_JTT_SERVER_DEREGISTER:	\
			info("DSMAPP_TIMER_JTT_SERVER_DEREGISTER\n");	\
			break;								\
		case DSMAPP_TIMER_NET_UPLOAD:		\
			info("DSMAPP_TIMER_NET_UPLOAD\n");	\
			break;					\
		default	:				\
			info("wrong timer type \n");	\
			break;						\
	}	\
}	while(0)


		void StartTimer(E_TIMER_TYPE type,int ms_timeout,callback_t cbp);
		void StopTimer(E_TIMER_TYPE type );


		





static kal_uint8 imei[] = "123456789012345";
static kal_uint8 release[] = "MXT1608S-V100C001B006s";
static kal_uint8 gnss[] = "MXT1608S-V300C001B003E6902";
static kal_uint8 iccid[] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90 };



#define __WHDSM_MXT1608S__
#define __WHDSM_SOS_2ROUND__

#define DEBUG_IN_VS 1

#define kal_mem_cpy(...) memcpy(__VA_ARGS__)
#define kal_mem_set(...) memset(__VA_ARGS__)

#define kal_sprintf(...) sprintf(__VA_ARGS__)
//##define snprintf _snprintf

#define dsmapp_trace(...) printf(__VA_ARGS__);printf("\n")

#define dsm_location_get_latest_position(...) NULL
#define DTGetRTCTime(...)
#define DSM_BASE_PRINT(...)

//#define  TIMER_SUP

#define StopTimer(...)
#define StartTimer(...)
#ifndef TIMER_SUP


//void StartTimer(E_TIMER_TYPE type, int timeout_ms,  callback_t cbp);
//void StopTimer(E_TIMER_TYPE type);




#endif




#if 0

#else


typedef void (*sock_event_func)(kal_int8 hdl, kal_uint32 evt)  ;

#define dsm_net_get_apn(...) 0
//#define dsm_tcp_connect(...) 1
kal_uint16 dsm_tcp_write(kal_int8 handle_fd, kal_uint8 *dat_in, kal_uint16 in_len);
kal_uint16 dsm_tcp_read(kal_int8 handle_fd, kal_uint8 *dat_in, kal_uint16 in_len);

kal_uint16 dsm_tcp_connect(const char *ip_addr, int ip_port, int apn/*, sock_event_func *func */);



//#define dsm_tcp_read(...) 1
#define dsm_tcp_close(...)
//#define dsm_tcp_write(...) 1
#define mmi_flight_mode_switch_for_mx(...)
#define dsm_srv_call_connected(...) 0
#define dsm_location_check_and_start_agps(...)
#define dsmapp_sys_shutdown(...)



#endif

void dsm_log(unsigned char *buf_log, unsigned int len);


//#define dsm_location_period_change(...)
#define dsm_location_period_change_sec(...)

#define DSM_APP_CALL_NUM_MAX_LEN     15

/*
#define DSMAPP_TIMER_RECONNECT_SERVICE 1
#define DSMAPP_TIMER_INTERACTION_SERVICE 2
#define DSMAPP_TIMER_CMD_TIMEOUT_SERVICE 3
#define DSMAPP_TIMER_DISCONNECT_SERVICE 4
*/

#define DEVICE_NUM_LEN 6 // 6λBCD�� JTT808Э��涨�ն��ֻ������12λ ǰ�油0
#define AUTH_CODE_LEN 26 // ��Ȩ�볤�� Ҫ��֤���Ϊ0
typedef struct
{
	kal_uint32 heart; // unit: s
	kal_char srv_ip[16];
	kal_uint16 srv_port;
	kal_uint8 auth_code_len;
	kal_uint8 auth_code[AUTH_CODE_LEN];
	kal_uint8 cell_num[DEVICE_NUM_LEN];
} nvram_ef_dsmapp_jtt_config_t;


/* connected status */
#define DSM_TCP_EVT_CONNECTED	1

/* write status */
#define DSM_TCP_EVT_CAN_WRITE	2

/* read status */
#define DSM_TCP_EVT_CAN_READ		3

/* broken status */
#define DSM_TCP_EVT_PIPE_BROKEN	4

/* not find host */
#define DSM_TCP_EVT_HOST_NOT_FOUND	 5

/* tcp pipe closed */
#define DSM_TCP_EVT_PIPE_CLOSED	     6


typedef struct
{
	kal_uint8 dev_mode; // 0:low;1:normal;2:continue (3:auto in 1608S)
#if defined(__WHDSM_MXT1608S__)
	kal_uint8 custom_period; // minute
	kal_uint8 auto_Tmin; // second
	kal_uint8 auto_Tgap; // second
#endif
} nvram_ef_dsmapp_pos_mode_t;


typedef enum
{
	LOCATION_TYPE_GPS,
	LOCATION_TYPE_LBS
}LOCATION_TYPE;

typedef struct DSM_BASE_TIME
{
	kal_uint8 hour;
	kal_uint8 minute;
	kal_uint8 second;
	kal_uint8 misecond;
}ST_DSM_BASE_TIME;

typedef struct DSM_BASE_DATE
{
	kal_uint8 year;
	kal_uint8 month;
	kal_uint8 day;
}ST_DSM_BASE_DATE;

typedef struct GPS_DRV_INFO
{
	ST_DSM_BASE_TIME time;
	//	kal_uint8 hour;
	//	kal_uint8 minute;
	//	kal_uint8 second;
	//	kal_uint8 misecond;
	kal_uint8 status;
	kal_uint8 lat[5];
	kal_uint8 N_S;
	kal_uint8 lon[5];
	kal_uint8 E_W;
	kal_uint16 speed;
	kal_uint16 course;
	ST_DSM_BASE_DATE date;
	//	kal_uint8 year;
	//	kal_uint8 month;
	//	kal_uint8 day;
	kal_uint8 postMode;
	kal_uint8 numSV;
	kal_uint8 quality;

	kal_uint8 gnss_type;
	kal_uint8 speed_grade;
	kal_uint16 speed_AVG;

	kal_uint8 CN0_AVG;
	kal_uint8 CN0_max;
	kal_uint8 CN0_cnt;
}ST_GPS_DRV_INFO;

typedef struct
{
	kal_uint16 arfcn;
	kal_uint8  bsic;
	kal_uint8  rxlev;
	kal_uint16 mcc;
	kal_uint16 mnc;
	kal_uint16 lac;
	kal_uint16 ci;
} dsm_nbr_info_t;

typedef struct
{
	kal_bool is_info_valid;
	kal_uint8 ta;
	dsm_nbr_info_t cur_cell_info;
	kal_uint8 nbr_cell_num;
	dsm_nbr_info_t nbr_cell_info[6];
} dsm_cell_info_t;

typedef struct DSM_LOCATION_INFO
{
	kal_bool			valid;
	LOCATION_TYPE 	type;
	union
	{
		ST_GPS_DRV_INFO gps;
		dsm_cell_info_t  	 lbs;
	}info;
}ST_DSM_LOCATION_INFO;

typedef struct MYTIME
{
	kal_int16 nYear;
	kal_int8 nMonth;
	kal_int8 nDay;
	kal_int8 nHour;
	kal_int8 nMin;
	kal_int8 nSec;
	kal_int8 DayIndex; /* 0=Sunday */
} MYTIME;










double dsm_base_coordinates2double(kal_uint8 *coordinates);

kal_uint8 dsmapp_battery_get_voltage_percent(void);


typedef struct
{
	kal_uint8 unused;
} *KAL_ADM_ID;

void get_iccid_value(kal_uint8 *iccid, kal_uint8 source);

#endif
