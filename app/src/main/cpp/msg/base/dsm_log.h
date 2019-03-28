//
// Created by public on 19-3-18.
//

#ifndef DSM_JTT808_LOG_H
#define DSM_JTT808_LOG_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mutex>
#include <thread>
#include "log4z.h"

#define BLACK           0
#define RED             1
#define GREEN           2
#define YELLOW          3
#define BLUE            4
#define MAGENTA         5
#define CYAN            6
#define WHITE           7

/* Color log print  */
#define RESET           0
#define BRIGHT          1
#define DIM             2
#define UNDERLINE       4
#define BLINK           5
#define REVERSE         7
#define HIDDEN          8


// ===================================
//void textcolor(int attr, int fg, int bg)
//{
//    char command[13];
//
//    /* Command is the control command to the terminal */
//    sprintf(command, "%c[%d;%d;%dm", 0x1B, attr, fg + 30, bg + 40);
//    printf("%s", command);
//}

#define BUG_LEN 1024
#include "singleton.h"
#include "../../Util.h"


//! base macro.
#define FORMAT_PARAM(format,szBuffer)									\
	do{															\
    	va_list args;											\
        va_start(args, format);									\
        vsnprintf(szBuffer, sizeof(szBuffer), format, args);	\
        va_end(args);											\
	} while (0)

class CDSMLog : public Singleton<CDSMLog>
{
public:
    CDSMLog(){};
    ~CDSMLog(){};
    static void my_vprint(char* fmt, va_list va_args)
    {
        char buffer[BUG_LEN] = {0};
        vsnprintf(buffer, BUG_LEN-1, fmt, va_args);
        printf("%s", buffer);
    }

    ///Color print log function
    static void _print_color(int attr, int color, const char* fmt,...)
    {
        char buffer[BUG_LEN] = {0};
        va_list marker;
        va_start(marker, fmt);

        // \u80cc\u666f\u8272\u4e3a0\u65f6\uff0c\u4e0d\u5f71\u54cd\u540e\u9762\u7684\u4fe1\u606f\uff0c\u5176\u5b83\u503c\u4f1a\u5f71\u54cd
        snprintf(buffer, BUG_LEN-1, "\x1b[%d;%dm%s\x1b[0m", attr, color+30, fmt);
        my_vprint(buffer, marker);  // \u4e00\u5b9a\u8981\u8fd9\u4e2a\u51fd\u6570\u624d\u80fd\u4f7f\u7528\u53ef\u53d8\u53c2\u6570
        va_end(marker);
    }

    static void dsm_dump(unsigned char *buf_log, unsigned int len)
    {
        int i;
        //printf("\nBuffer(%d) Data:\n", len);
        char buff[8] = {0};
        std::string strHex;
        for (i = 0; i < len; i++)
        {
            sprintf(buff,"%02x ",buf_log[i]);
            strHex.append(buff);

            //printf("%02x ", buf_log[i]);
            //if ((i+1) % 16 == 0)
            //printf("\n");
        }
        Trace(strHex.c_str());
        //printf("\n");
        //printf("# %sClient Stop Fail --> %s (%d) [%d]"), (LPCTSTR)SafeString(lpszName), lpszDesc, code, ::GetLastError());
    }

    void InitialiseLog4z(std::string szLogPath)
    {
        std::call_once(m_once_flag,[this,szLogPath](){
									   zsummer::log4z::ILog4zManager::getInstance()->config(szLogPath.c_str());
									   zsummer::log4z::ILog4zManager::getInstance()->start();
								   });
    }

    static void Trace(const char* format, ...)
    {
		char szBuffer[LOG4Z_LOG_BUF_SIZE];
//		FORMAT_PARAM(format,szBuffer);
//		LOGE(("Trace %s ", szBuffer);
		LOG_TRACE(zsummer::log4z::ILog4zManager::getInstance()->findLogger("moniter"),
				  szBuffer);
    }
	
    static void Debug(const char* format, ...)
	{
		char szBuffer[LOG4Z_LOG_BUF_SIZE];
		FORMAT_PARAM(format,szBuffer);
		LOG_DEBUG(zsummer::log4z::ILog4zManager::getInstance()->findLogger("moniter"),
				  szBuffer);
	}
	
    static void Info(const char* format, ...)
	{
		char szBuffer[LOG4Z_LOG_BUF_SIZE];
		FORMAT_PARAM(format,szBuffer);
		LOG_INFO(zsummer::log4z::ILog4zManager::getInstance()->findLogger("moniter"),
				 szBuffer);
	}
	
    static void Warn(const char* format, ...)
	{
		char szBuffer[LOG4Z_LOG_BUF_SIZE];
		FORMAT_PARAM(format,szBuffer);
		LOGE(" %s ",szBuffer);
		LOG_WARN(zsummer::log4z::ILog4zManager::getInstance()->findLogger("moniter"),
				 szBuffer);
	}
	
    static void Error(const char* format, ...)
	{
		char szBuffer[LOG4Z_LOG_BUF_SIZE];
//		FORMAT_PARAM(format,szBuffer);
		LOGE("Error %s ",szBuffer);
//		LOG_ERROR(zsummer::log4z::ILog4zManager::getInstance()->findLogger("moniter"),
//				  szBuffer);
	}
	
    static void Fatal(const char* format, ...)
	{
		char szBuffer[LOG4Z_LOG_BUF_SIZE];
		FORMAT_PARAM(format,szBuffer);
		LOG_FATAL(zsummer::log4z::ILog4zManager::getInstance()->findLogger("moniter"),
				  szBuffer);
	}
	
private:
	char m_szBuffer[LOG4Z_LOG_BUF_SIZE];
    std::once_flag  m_once_flag;
	LoggerId m_moniter_logId;
};

//#define print_color(attr, color, fmt,...) CDSMLog::_print_color(attr, color, fmt, ##__VA_ARGS__)
//#define succ(fmt, ...) CDSMLog::_print_color(BRIGHT, BLUE, fmt, ##__VA_ARGS__)
//#define warn(fmt, ...) CDSMLog::_print_color(BRIGHT, YELLOW, fmt, ##__VA_ARGS__)
//#define err(fmt, ...) CDSMLog::_print_color(BRIGHT, RED, fmt, ##__VA_ARGS__)
//#define info(fmt, ...) CDSMLog::_print_color(BRIGHT, GREEN, fmt, ##__VA_ARGS__)
//#define alarm(fmt, ...) CDSMLog::_print_color(BRIGHT, CYAN, fmt, ##__VA_ARGS__)
//#define dsmapp_trace(...) printf(__VA_ARGS__);printf("\n")
//#define FUNC_ENTER dsmapp_trace("%s enter.\r\n",__FUNCTION__)
//#define FUNC_LEAVE dsmapp_trace("%s leave.\r\n",__FUNCTION__)

#endif //DSM_JTT808_LOG_H
