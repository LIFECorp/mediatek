/*
* ANT Stack
*
* Copyright 2009 Dynastream Innovations
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/******************************************************************************\
*
* FILE NAME: ANT_log.h
*
* BRIEF:
* This file defines logging functions
*
*
\******************************************************************************/

#ifndef __ANT_LOG_H
#define __ANT_LOG_H

#include <unistd.h>
#include "ant_types.h"

#define LEVEL_NONE      0
#define LEVEL_ERROR     1
#define LEVEL_WARNING   2
#define LEVEL_INFO      3
#define LEVEL_DEBUG     4
#define LEVEL_VERBOSE   5

#define STREAM_STDOUT   16
#define STREAM_LOGCAT   17

/* Define what stream output should go to depending on platform */
#if defined(__ANDROID__) || defined(ANDROID)
   #define ANT_OUTPUT_STREAM STREAM_LOGCAT
#elif defined(__linux__) || defined(__linux) || defined(linux)
   #define ANT_OUTPUT_STREAM STREAM_STDOUT
#endif

/* If no debug level defined, set default as none */
#if !defined(ANT_DEBUG)
  // #define ANT_DEBUG LEVEL_NONE
   #define ANT_DEBUG LEVEL_VERBOSE
#endif

/* Define to show function entry and exit */
#define ANT_STACK_TRACE

/* Define to show message in byte form */
#define ANT_LOG_SERIAL

/* Define to write serial log to file instead of logcat */
//#define ANT_LOG_SERIAL_FILE "/data/system/antseriallog.txt"
#undef LOG_TAG
#define LOG_TAG "antradio"

#if ANT_DEBUG == LEVEL_NONE
   #undef ANT_STACK_TRACE
   #undef ANT_LOG_SERIAL
#endif

#if ANT_DEBUG >= LEVEL_ERROR
   #define OUTPUT_LEVEL_ERROR(...)     OUTPUT_ERROR(__VA_ARGS__)
#else
   #define OUTPUT_LEVEL_ERROR(...)     ((void)0)
#endif
#if ANT_DEBUG >= LEVEL_WARNING
   #define OUTPUT_LEVEL_WARNING(...)   OUTPUT_WARNING(__VA_ARGS__)
#else
   #define OUTPUT_LEVEL_WARNING(...)   ((void)0)
#endif
#if ANT_DEBUG >= LEVEL_INFO
   #define OUTPUT_LEVEL_INFO(...)      OUTPUT_INFO(__VA_ARGS__)
#else
   #define OUTPUT_LEVEL_INFO(...)      ((void)0)
#endif
#if ANT_DEBUG >= LEVEL_DEBUG
   #define OUTPUT_LEVEL_DEBUG(...)     OUTPUT_DEBUG(__VA_ARGS__)
#else
   #define OUTPUT_LEVEL_DEBUG(...)     ((void)0)
#endif
#if ANT_DEBUG >= LEVEL_VERBOSE
   #define OUTPUT_LEVEL_VERBOSE(...)   OUTPUT_VERBOSE(__VA_ARGS__)
#else
   #define OUTPUT_LEVEL_VERBOSE(...)   ((void)0)
#endif

#if ANT_OUTPUT_STREAM == STREAM_STDOUT
   #include <stdio.h>
   #define OUTPUT_VERBOSE(fmt, ...)    fprintf(stdout, LOG_TAG "<V>: " fmt "\n", ##__VA_ARGS__)
   #define OUTPUT_DEBUG(fmt, ...)      fprintf(stdout, LOG_TAG "<D>: " fmt "\n", ##__VA_ARGS__)
   #define OUTPUT_INFO(fmt, ...)       fprintf(stdout, LOG_TAG "<I>: " fmt "\n", ##__VA_ARGS__)
   #define OUTPUT_WARNING(fmt, ...)    fprintf(stdout, LOG_TAG "<W>: " fmt "\n", ##__VA_ARGS__)
   #define OUTPUT_ERROR(fmt, ...)      fprintf(stdout, LOG_TAG "<E>: " fmt "\n", ##__VA_ARGS__)
#elif ANT_OUTPUT_STREAM == STREAM_LOGCAT
   #if (ANT_DEBUG >= LEVEL_VERBOSE) || (defined(ANT_LOG_SERIAL) && !defined(ANT_LOG_SERIAL_FILE))
      #undef NDEBUG /* Define verbose logging for logcat if required */
   #endif
   #include <cutils/log.h>
   #define OUTPUT_VERBOSE(...)         ALOGD(__VA_ARGS__)
   #define OUTPUT_DEBUG(...)           ALOGD(__VA_ARGS__)
   #define OUTPUT_INFO(...)            ALOGI(__VA_ARGS__)
   #define OUTPUT_WARNING(...)         ALOGW(__VA_ARGS__)
   #define OUTPUT_ERROR(...)           ALOGE(__VA_ARGS__)
#endif

#define ANT_WARN(f, ...)                  OUTPUT_WARNING("func is %s " "line = %d "f, __func__, __LINE__,  ##__VA_ARGS__)
#define ANT_ERROR(f, ...)                 OUTPUT_ERROR("func is %s ""line = %d "f, __func__, __LINE__, ##__VA_ARGS__)

#define ANT_DEBUG_V(f, ...)               OUTPUT_LEVEL_DEBUG("func is %s " "line = %d "f, __func__, __LINE__, ##__VA_ARGS__)
#define ANT_DEBUG_D(f, ...)               OUTPUT_LEVEL_DEBUG("func is %s " "line = %d "f, __func__, __LINE__,  ##__VA_ARGS__)
#define ANT_DEBUG_I(f, ...)               OUTPUT_LEVEL_INFO("func is %s " "line = %d "f, __func__, __LINE__,  ##__VA_ARGS__)
#define ANT_DEBUG_W(f, ...)               OUTPUT_LEVEL_WARNING("func is %s " "line = %d "f, __func__, __LINE__, f, ##__VA_ARGS__)
#define ANT_DEBUG_E(f, ...)               OUTPUT_LEVEL_ERROR("func is %s " "line = %d "f, __func__, __LINE__, ##__VA_ARGS__)

#if defined(ANT_STACK_TRACE)
   #define ANT_FUNC_START()            OUTPUT_DEBUG("->  FUNC start %s", __FUNCTION__)
   #define ANT_FUNC_END()              OUTPUT_DEBUG("<-  FUNC end   %s", __FUNCTION__)
#else
   #define ANT_FUNC_START()            ((void)0)
   #define ANT_FUNC_END()              ((void)0)
#endif

#if defined(ANT_LOG_SERIAL)
static inline void ANT_SERIAL(ANT_U8 *buf, ANT_U8 len, char dir)
{
   static const char hexToChar[] = {'0','1','2','3','4','5','6','7',
                                    '8','9','A','B','C','D','E','F'};
   int i;
   static char log[1024];
   char *ptr = log;

   *(ptr++) = dir;
   *(ptr++) = 'x';
   *(ptr++) = ' ';
   for (i = 0; i < len; i++) {
      *(ptr++) = '[';
      *(ptr++) = hexToChar[(buf[i] & 0xF0) >> 4];
      *(ptr++) = hexToChar[(buf[i] & 0x0F) >> 0];
      *(ptr++) = ']';
   }
#if defined(ANT_LOG_SERIAL_FILE)
   *(ptr++) = '\n';
   FILE *fd = NULL;
   fd = fopen(ANT_LOG_SERIAL_FILE, "a");
   if (NULL == fd) {
      ANT_WARN("Could not open %s for serial output. %s", ANT_LOG_SERIAL_FILE, strerror(errno));
   } else {
      fwrite(log, 1, (ptr - log), fd);
      if (fclose(fd)) {
         ANT_WARN("Could not close file for serial output. %s", strerror(errno));
      }
   }
#else
   *(ptr++) = '\0';
   OUTPUT_VERBOSE("%s", log);
#endif
}
#else
   #define ANT_SERIAL(...)             ((void)0)
#endif

#endif /* __ANT_LOG_H */