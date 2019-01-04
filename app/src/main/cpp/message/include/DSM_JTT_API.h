#ifndef DSM_JTT808_API_HDR__
#define DSM_JTT808_API_HDR__

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
 * input 2,  the type of the the waring file
 * input 3,  the path name of the waring file 
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

int DSM_JTT808_Event_Callback(int warn_type, int file_type, const char *path);







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


int DSM_JTT808_Start(const char *server, unsigned short int port);






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


int DSM_JTT808_Stop(unsigned char force_stop);


#endif /* DSM_JTT808_API_HDR__ */
