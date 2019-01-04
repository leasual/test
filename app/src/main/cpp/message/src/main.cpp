#include <iostream>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//#include <Config.h>
//#include "common.h"
#include "dsm_transmit_jtt.h"




int main(int argc, char* const argv[]) 
{ 
	
	char* lpszLine	= NULL;
	SIZE_T nSize	= 0;


	CreateHPSocketObjects(); 

	printf("enter mainloop\n");
	printf("> ");

	while(1)
	{
    		SSIZE_T nRead = getline(&lpszLine, &nSize, stdin);

        	if(nRead == EOF)
        	{
        		printf("input line: %s\n", lpszLine);
		}

        	printf("> ");

		if(nSize >= 4096)
		{
			if (lpszLine)
				free(lpszLine);

			lpszLine = NULL;
			nSize    = 0;
        	}

		if(strncmp(lpszLine, "start", 5) == 0) 
		{
			printf("start cmd\n");
			OnCmdStart();
		}
		else if(strncmp(lpszLine, "send", 4) == 0) 
		{
			OnCmdSend();
		}
		else if(strncmp(lpszLine, "stop", 4) == 0) 
		{
			OnCmdStop();
		}
		else
			;
	}
	if (lpszLine)
		free(lpszLine);

    DestroyHPSocketObjects(); 
 
    return 0;//EXIT_CODE_OK; 
}

#if 0

//MSGReceive * g_msgReceive;
//
//void intHandler(int sig) {
//    g_msgReceive->stop();
//}

//#include <boost/interprocess/ipc/message_queue.hpp>
/*
int main() {
//    using namespace boost::interprocess;
//    try{
//        message_queue mq(open_only,"123");
//        unsigned  int priority;
//        message_queue::size_type  recv_size;
//        int buf[512]={0};
//
////        std::cout<<"recv size : "<<recv_size<<std::endl;
//        int num;
//        mq.try_receive(&num,sizeof(num),recv_size,priority);
//        std::cout<<num<<std::endl;
//        mq.try_receive(&num,sizeof(num),recv_size,priority);
//        std::cout<<num<<std::endl;
//        for(int i=0;i<20;i++){
//            bool isRecv = mq.try_receive(&num,sizeof(num),recv_size,priority);
//            std::cout<<num<<",";
//            if( i%20 == 0);
//                std::cout<<std::endl;
//        }
//    }
//    catch (interprocess_exception& ex){
//        message_queue::remove("123");
//        std::cout<<ex.what()<<std::endl;
//    }
//    message_queue::remove("123");
//    MSGReceive msgReceive;
//    g_msgReceive = &msgReceive;

//    msgReceive.start();

//    signal(SIGINT, intHandler);
    MSGHandle msgHandle;
    msgHandle.startHandle();
    return 0;
}
*/

#endif
