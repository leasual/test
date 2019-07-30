#include "others.h"
//#include <iostream>
//#include <atimer.hpp>
#include <android/log.h>

#define LOG_TAG "OpenCV-NDK-Native"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

double mx_base_coordinates2double(kal_uint8 *coordinates)
{
	double result = 0;
	char str[14];

	result += (double)coordinates[1];
	result += (double)coordinates[2] * 0.01;
	result += (double)coordinates[3] * 0.0001;
	result += (double)coordinates[4] * 0.000001;
	result /= 60;
	result += (double)coordinates[0];

	memset(str, 0, sizeof(str));
	sprintf(str, "%13f", result);
	DSM_BASE_PRINT("%s(%d %d.%02d%02d%02d): %s", __FUNCTION__, coordinates[0], coordinates[1], coordinates[2], coordinates[3], coordinates[4], str);
	return result;
}



char * mxapp_get_imei(void)
{
	return (char*)imei;
}



void dsm_log(unsigned char *buf_log, unsigned int len)
{
	int i; 

	//printf("\nBuffer(%d) Data:\n", len);
	for (i = 0; i < len; i++)
	{
		printf("%02x ", buf_log[i]);
		//if ((i+1) % 16 == 0)
			//printf("\n");
	}
	printf("\n");
//printf("# %sClient Stop Fail --> %s (%d) [%d]"), (LPCTSTR)SafeString(lpszName), lpszDesc, code, ::GetLastError());


}

kal_uint8 * release_verno(void)
{
	return (kal_uint8 *)release;
}

kal_uint8 * gnss_verno(void)
{
	return (kal_uint8 *) gnss;
}

kal_uint8 dsmapp_battery_get_voltage_percent(void)
{
	return 42;
}

void get_iccid_value(kal_uint8 *iccid_p, kal_uint8 source)
{
	memcpy(iccid_p, iccid, sizeof(iccid));
}


//extern boost::asio::io_service io;


//extern DeadlineOpsManager deadlineOPSManager;


#if 0
//std::map<E_TIMER_TYPE, ATimer<>> tvec;
ATimer<> *recnn_timer = NULL;

ATimer<> *inter_timer= NULL;

ATimer<> *cmd_timer= NULL;


ATimer<> *discnn_timer= NULL;


ATimer<> *srv_timer= NULL;



ATimer<> *reg_timer= NULL;


ATimer<> *auth_timer= NULL;

ATimer<> *dereg_timer= NULL;

ATimer<> *uploc_timer= NULL;





void testx(void){

}

void StartTimer(E_TIMER_TYPE type, int timeout_ms,  callback_t cbp)
{

	succ("\n\n!!!!!!!!!!!!! start timer %d.....\n", timeout_ms);
	print_timer_type(type);

	switch(type){
		case DSMAPP_TIMER_RECONNECT_SERVICE:
			if (recnn_timer!= NULL){
				recnn_timer->stop();
				delete recnn_timer;
				recnn_timer = NULL;
			}
			if (recnn_timer == NULL)
				recnn_timer = new (ATimer<>);
			if (recnn_timer != NULL){
				recnn_timer->bind(cbp);
				recnn_timer->start(timeout_ms);
			}
			break;
		case DSMAPP_TIMER_INTERACTION_SERVICE:
			
			if (inter_timer!= NULL){
				inter_timer->stop();
				delete inter_timer;
				inter_timer = NULL;
			}
			if (inter_timer == NULL)
				inter_timer = new (ATimer<>);
			if (inter_timer != NULL){
				inter_timer->bind(cbp);
				inter_timer->start(timeout_ms);
			}
			
			break;

		case DSMAPP_TIMER_CMD_TIMEOUT_SERVICE:			
			
			//cmd_timer.stop();
			//cmd_timer.bind(cbp);
			//cmd_timer.start(timeout_ms);

			if (cmd_timer!= NULL){
				cmd_timer->stop();
				delete cmd_timer; 
				cmd_timer = NULL;
			}
			if (cmd_timer == NULL)
				cmd_timer = new (ATimer<>);
			if (cmd_timer != NULL){
				cmd_timer->bind(cbp);
				cmd_timer->start(timeout_ms);
			}
			
			break;

		case DSMAPP_TIMER_DISCONNECT_SERVICE:			
			
			//discnn_timer.stop();
			//discnn_timer.bind(cbp);
			//discnn_timer.start(timeout_ms);

			if (discnn_timer!= NULL){
				discnn_timer->stop();
				delete discnn_timer;
				discnn_timer = NULL;
			}
			if (discnn_timer == NULL)
				discnn_timer = new (ATimer<>);
			if (discnn_timer != NULL){
				discnn_timer->bind(cbp);
				discnn_timer->start(timeout_ms);
			}
			
			break;

		case DSMAPP_TIMER_JTT_SERVER:
			
			//srv_timer.stop();
			//srv_timer.bind(cbp);
			//srv_timer.start(timeout_ms);

			
			if (srv_timer!= NULL){
				srv_timer->stop();
				delete srv_timer;
				srv_timer = NULL;
			}
			if (srv_timer == NULL)
				srv_timer = new (ATimer<>);
			if (srv_timer != NULL){
				srv_timer->bind(cbp);
				srv_timer->start(timeout_ms);
			}

			break;

		case DSMAPP_TIMER_JTT_SERVER_REGISTER:
			//reg_timer.stop();
			//reg_timer.bind(cbp);
			//reg_timer.start(timeout_ms);


			if (reg_timer!= NULL){
				reg_timer->stop();
				delete reg_timer;
				reg_timer = NULL;
			}
			if (reg_timer == NULL)
				reg_timer = new (ATimer<>);
			if (reg_timer != NULL){
				reg_timer->bind(cbp);
				reg_timer->start(timeout_ms);
			}

			break;

		case DSMAPP_TIMER_JTT_SERVER_AUTHORIZE:
			//auth_timer.stop();
			//auth_timer.bind(cbp);
			//auth_timer.start(timeout_ms);

			if (auth_timer!= NULL){
				auth_timer->stop();
				delete auth_timer;
				auth_timer = NULL;
			}
			if (auth_timer == NULL)
				auth_timer = new (ATimer<>);
			if (auth_timer != NULL){
				auth_timer->bind(cbp);
				auth_timer->start(timeout_ms);
			}

			break;

		case DSMAPP_TIMER_JTT_SERVER_DEREGISTER:
			//dereg_timer.stop();
			//dereg_timer.bind(cbp);
			//dereg_timer.start(timeout_ms);


			if (dereg_timer!= NULL){
				dereg_timer->stop();
				delete dereg_timer;
				dereg_timer = NULL;
			}
			if (dereg_timer == NULL)
				dereg_timer = new (ATimer<>);
			if (dereg_timer != NULL){
				dereg_timer->bind(cbp);
				dereg_timer->start(timeout_ms);
			}

			break;

		case DSMAPP_TIMER_NET_UPLOAD:
			//uploc_timer.stop();
			//uploc_timer.bind(cbp);
			//uploc_timer.start(timeout_ms);


			if (uploc_timer!= NULL){
				uploc_timer->stop();
				delete uploc_timer;
				uploc_timer = NULL;
			}
			if (uploc_timer == NULL)
				uploc_timer = new (ATimer<>);
			if (uploc_timer != NULL){
				uploc_timer->bind(cbp);
				uploc_timer->start(timeout_ms);
			}

			break;

		default:
			err("error timer\n");
			break;
	}
			




void StopTimer(E_TIMER_TYPE type)
{

	info("StopTimer ...\n");
	print_timer_type(type);
	
	switch(type){
		case DSMAPP_TIMER_RECONNECT_SERVICE:
			if (recnn_timer != NULL){
				recnn_timer->stop();
				delete recnn_timer;
				recnn_timer = NULL;
			}

			break;
		case DSMAPP_TIMER_INTERACTION_SERVICE:
			if (inter_timer != NULL){
				inter_timer->stop();
				delete inter_timer;
				inter_timer = NULL;
			}

			break;

		case DSMAPP_TIMER_CMD_TIMEOUT_SERVICE:			

			if (cmd_timer != NULL){
				cmd_timer->stop();
				delete cmd_timer;
				cmd_timer = NULL;
			}

			break;

		case DSMAPP_TIMER_DISCONNECT_SERVICE:			
			if (discnn_timer != NULL) {
				discnn_timer->stop();
				delete discnn_timer;
				discnn_timer = NULL;
			}

			break;

		case DSMAPP_TIMER_JTT_SERVER:
			if (srv_timer != NULL){
				srv_timer->stop();
				delete srv_timer;
				srv_timer = NULL;
			}


			break;

		case DSMAPP_TIMER_JTT_SERVER_REGISTER:
			if(reg_timer != NULL){
				reg_timer->stop();
				delete reg_timer;
				reg_timer = NULL;
			}


			break;

		case DSMAPP_TIMER_JTT_SERVER_AUTHORIZE:

			if (auth_timer != NULL){
				auth_timer->stop();
				delete auth_timer;
				auth_timer = NULL;
			}


			break;

		case DSMAPP_TIMER_JTT_SERVER_DEREGISTER:

			if (dereg_timer != NULL){
				dereg_timer->stop();
				delete dereg_timer;
				dereg_timer = NULL;

			}


			break;

		case DSMAPP_TIMER_NET_UPLOAD:
			if (uploc_timer != NULL ){
				uploc_timer->stop();
				delete uploc_timer;
				uploc_timer = NULL;
			}
	

			break;

		default:
			err("error timer\n");
			break;
	}


}




//StartTimer(DSMAPP_TIMER_CMD_TIMEOUT_SERVICE, MODE_FLIGHT_TO_NORMAL_INTERVAL, dsmapp_srv_reconnect);

#ifdef TIMER_SUP

void StartTimer(E_TIMER_TYPE type, int ms_timeout,  callback_t cbp)
{
	
	info(" StartTimer : %d \n", type);

	deadlineOPSManager.start_timer(type, ms_timeout, cbp);
}



void StopTimer(E_TIMER_TYPE type)
{
	alarm("stop timer %d\n", type);
	deadlineOPSManager.stop_timer(type);
}

#endif

#endif



///Color print log function

// ===================================
void textcolor(int attr, int fg, int bg)
{
    char command[13];

    /* Command is the control command to the terminal */
    sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
    printf("%s", command);
}

void color_test(void)
{
    textcolor(BRIGHT, RED, GREEN);
    printf("hello %d\n", 250);
    textcolor(RESET, WHITE, BLACK);
}

//==================================
#define BUG_LEN 1024
void my_vprint(char* fmt, va_list va_args)
{
        char buffer[BUG_LEN] = {0};
        vsnprintf(buffer, BUG_LEN-1, fmt, va_args);
        printf("%s", buffer);
		LOGE(" %s", buffer);
}

// \u4e0d\u4f7f\u7528\u80cc\u666f\u8272
void _print_color(int attr, int color, const char* fmt,...)
{
    char buffer[BUG_LEN] = {0};
    va_list marker;
    va_start(marker, fmt);

    // \u80cc\u666f\u8272\u4e3a0\u65f6\uff0c\u4e0d\u5f71\u54cd\u540e\u9762\u7684\u4fe1\u606f\uff0c\u5176\u5b83\u503c\u4f1a\u5f71\u54cd
    snprintf(buffer, BUG_LEN-1, "\x1b[%d;%dm  %s\x1b[0m", attr, color+30, fmt);
    my_vprint(buffer, marker);  // \u4e00\u5b9a\u8981\u8fd9\u4e2a\u51fd\u6570\u624d\u80fd\u4f7f\u7528\u53ef\u53d8\u53c2\u6570
    va_end(marker);
}


