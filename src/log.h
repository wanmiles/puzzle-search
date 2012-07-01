/**
 * Copyright Ken Anderson, 2012
 */
/**
 * Log macros / functions.
 */

#ifndef LOG_H
#define LOG_H

#include <time.h>
#include <stdio.h>

/////////////////////////////////
// MACROS ///////////////////////
/////////////////////////////////

enum LogLevel {VERBOSE=0,DEBUG,NORMAL,WARN,ERROR};
LogLevel g_logLevel = NORMAL;

#define _LOG(level,...) if(level>=g_logLevel){printf(__VA_ARGS__); }
#define LOG_ERROR(...) _LOG(ERROR,__VA_ARGS__)
#define LOG_WARN(...) _LOG(WARN,__VA_ARGS__)
#define LOG(...) _LOG(NORMAL,__VA_ARGS__)
#define LOG_DEBUG(...) _LOG(DEBUG,__VA_ARGS__)
#define LOG_VERBOSE(...) _LOG(VERBOSE,__VA_ARGS__)

/////////////////////////////////
// HELPER FUNCTIONS /////////////
/////////////////////////////////

void printTime(LogLevel level)
{
  time_t t;
  time(&t);
  _LOG(level,"%s",ctime(&t));
}


#endif

