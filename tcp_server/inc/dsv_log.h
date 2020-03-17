/*
 * dsv_log.h
 *
 *  Created on: Jul 24, 2018
 *      Author: desay-sv
 */

#ifndef DSV_LOG_H_
#define DSV_LOG_H_

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <string.h>
#include <sys/time.h>


typedef enum Log_Level_t
{
	VL_ERR	=	0,
	VL_WARN	= 	1,
	VL_INFO	=	2,
	VL_DEBUG=	3,
	VL_ALL	=	4
}Log_Level;

#define STRING_MAX 1024
#define LOG_FILE_NAME "YDLog.log"
extern Log_Level mLogLevel;
extern bool mIsToFile;//全局变量
extern bool mIsPrintf;
//extern std::ofstream mOfs;
#define YLOG_SET(isPrintf,isToFile,logLevel) Log_Level mLogLevel = logLevel; bool mIsToFile = isToFile; bool mIsPrintf = isPrintf;
#define USER_SPRINT_BASE(outstr, format, args...)  sprintf(outstr, format, ##args)
#define USER_SPRINT_0(outstr, format, args... )\
{\
	timeval timeVal;\
	gettimeofday(&timeVal,NULL);\
	time_t timep;\
    time( &timep );\
    struct tm *pTM = gmtime( &timep );\
    USER_SPRINT_BASE(outstr,"[%4d-%02d-%02d %02d:%02d:%02d:%06ld]" format,\
    				 pTM->tm_year+1900, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, pTM->tm_sec,timeVal.tv_usec,##args);\
}
#define USER_SPRINT(outstr, tag, format, args... )  USER_SPRINT_0(outstr, "[%s]" format, tag, ##args)
#define YLOG(logLevel, tag, format, args... )\
{\
	if(logLevel<=mLogLevel){\
		char tempStr[STRING_MAX];\
		USER_SPRINT(tempStr,tag,format,##args);\
		if(mIsPrintf == true){\
			printf("%s",tempStr);\
		}\
		if(mIsToFile == true){\
			std::ofstream mOfs;\
			mOfs.open(LOG_FILE_NAME, std::ofstream::app);\
			mOfs.write(tempStr, strlen(tempStr));\
			mOfs.close();\
		}\
	}\
}


#endif /* DSV_LOG_H_ */
