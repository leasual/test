#include <iostream>
#include <signal.h>
#include "others.h"
#include "dsm_transmit_jtt.h"

/* API module first start flag */
static char first_start = 1;
extern  nvram_ef_dsmapp_jtt_config_t jtt_config;
extern int need_register;

/**
 * Driver Status Monitor System's communicate module API
 * 
 * parameters desc:  
 * input 1  type which means the DSM's warining event type
 *		0, nowarning 
 *		1, Distraction Warning
 *		2, Fatigue Warning 
 *		3, Smoke Warning
 *		4, Call Warning
 *		5, Chat Warning
 * input 2,  the waring file type 
 *              1, picture 
 *              2, movie
 * input 3,  the waring picture or movie's path
 *
 * output the result of the uploadte waring event
 *         success  1 
 *	   do nothing  0
 *         failed   < 0
 *		-1,  start server error
 *		-2,  connect server error
 *		-3,  upload event error
 *		-4,  other... 
 *
 * Author: Frex tangzt@gmail.com
 * Datetime: 2018/12/28
 *
 */

int DSM_JTT808_Event_Callback(int warn_type, int file_type, const char *path) {

	info("DSM_JTT808_event_callback enter\n");

	/*  take care:
         *    if firstly call callback func, 
	 *    we need to create socket resource,
         * 
         *  Also, you can call DSM_JTT808_Start 
	 *  to pre-initilize this part whenever you want
	 */	
	if ( first_start ) {

 		CreateHPSocketObjects();

		if (StartTcpClient(YOU_BANG_SERVER_IP, YOU_BANG_SERVER_PORT) == 1) {
			first_start = 0;

			jtt_config.srv_port = YOU_BANG_SERVER_PORT;
			strcpy(jtt_config.srv_ip, YOU_BANG_SERVER_IP);
		}
		else {
			DestroyHPSocketObjects();

			err("Staring server %s port %d error\n", YOU_BANG_SERVER_IP, YOU_BANG_SERVER_PORT);
			return -1;
		}
	}

	HandleTcpClientEvent(warn_type, file_type, path);		   	
	

	info("DSM_JTT808_event_callback leave\n");

	return 0;
}








/**
 * Driver Status Monitor System's communicate module API
 * 
 * parameters desc:  
 * input 1  svr_ip, server ip address 
 * input 2, port  , server port  
 *
 * output the result of connect server  
 *         success  1 
 *         failed   < 0
 *		-1,  start server error
 *
 * Author: Frex tangzt@gmail.com
 * Datetime: 2018/12/28
 *
 */


int DSM_JTT808_Start(const char *server, unsigned short int port, int start_register_follow = 0) {

	info("DSM_JTT808_Start enter__ serverip %s, port %d need start register follow %d\n",server,port,start_register_follow);

	first_start = 1;

	need_register = start_register_follow;
	/* create the socket resuorce for DSM JTT808 transmitter API */
	if ( first_start ) {
		CreateHPSocketObjects();

		if (StartTcpClient(server, port) == 1) {
			first_start = 0;

			jtt_config.srv_port = port;
			strcpy(jtt_config.srv_ip, server);

			succ("Staring server %s port %d error\n", server, port);
			return 1;
		}
		else {
			DestroyHPSocketObjects();

			err("Staring server %s port %d error\n", server, port);
			return -1;
		}
	}

	info("DSM_JTT808_Start leave\n");

	return 0;
}

/**
 * Driver Status Monitor System's communicate module API
 * 
 * parameters desc:  
 * input  force stop the JTT808 module 
 * output the result of connect server  
 *         success  1 
 *         failed   -1 
 *
 * Author: Frex tangzt@gmail.com
 * Datetime: 2018/12/28
 *
 */


int DSM_JTT808_Stop(unsigned char force_stop) {

	info("DSM_JTT808_Stop enter\n");
	
	/* descroy resuorce for DSM JTT808 transmitter API */
	if ( first_start == 0 || force_stop > 0 ) {
		StopTcpClient();	
		DestroyHPSocketObjects();

		succ("Shutdown DSM JTT808 socket resource now\n");
		first_start = 1;
		return 1;
	}

	info("DSM_JTT808_Stop leave\n");
	return 0;

}


