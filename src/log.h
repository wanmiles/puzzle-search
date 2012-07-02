/**
 * Copyright (c) 2010-2012, Ken Anderson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

/**
 * Log macros and functions.
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

