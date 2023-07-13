#pragma once
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define ERROR_LEVEL     0x01
#define INFO_LEVEL      0x02
#define DEBUG_LEVEL     0x03

/****************** NOTICE ******************/
/********************************************/
//        Please set the "LOG_LEVEL"
/********************************************/
/********************************************/
/* multimedia - audio */
#ifndef AUDIO_LOG_LEVEL
#define AUDIO_LOG_LEVEL   INFO_LEVEL
#endif

#if AUDIO_LOG_LEVEL >= DEBUG_LEVEL
#define ALOG_D(message, ...)      LOG_DEBUG(message, ##__VA_ARGS__)
#else
#define ALOG_D(message, ...)
#endif

#if AUDIO_LOG_LEVEL >= INFO_LEVEL
#define ALOG_I(message, ...)      LOG_INFO(message, ##__VA_ARGS__)
#else
#define ALOG_I(message, ...)
#endif

#if AUDIO_LOG_LEVEL >= ERROR_LEVEL
#define ALOG_E(message, ...)      LOG_ERROR(message, ##__VA_ARGS__)
#else
#define ALOG_E(message, ...)
#endif

/* multimedia - video */
#ifndef VIDEO_LOG_LEVEL
#define VIDEO_LOG_LEVEL   INFO_LEVEL
#endif

#if VIDEO_LOG_LEVEL >= DEBUG_LEVEL
#define VLOG_D(message, ...)      LOG_DEBUG(message, ##__VA_ARGS__)
#else
#define VLOG_D(message, ...)
#endif

#if VIDEO_LOG_LEVEL >= INFO_LEVEL
#define VLOG_I(message, ...)      LOG_INFO(message, ##__VA_ARGS__)
#else
#define VLOG_I(message, ...)
#endif

#if VIDEO_LOG_LEVEL >= ERROR_LEVEL
#define VLOG_E(message, ...)      LOG_ERROR(message, ##__VA_ARGS__)
#else
#define VLOG_E(message, ...)
#endif

/* UI */
#ifndef UI_LOG_LEVEL
#define UI_LOG_LEVEL   DEBUG_LEVEL
#endif

#if UI_LOG_LEVEL >= DEBUG_LEVEL
#define ULOG_D(message, ...)      LOG_DEBUG(message, ##__VA_ARGS__)
#else
#define ULOG_D(message, ...)
#endif

#if UI_LOG_LEVEL >= INFO_LEVEL
#define ULOG_I(message, ...)      LOG_INFO(message, ##__VA_ARGS__)
#else
#define ULOG_I(message, ...)
#endif

#if UI_LOG_LEVEL >= ERROR_LEVEL
#define ULOG_E(message, ...)      LOG_ERROR(message, ##__VA_ARGS__)
#else
#define ULOG_E(message, ...)
#endif

/* net */
#ifndef NET_LOG_LEVEL
#define NET_LOG_LEVEL   INFO_LEVEL
#endif

#if NET_LOG_LEVEL >= DEBUG_LEVEL
#define NLOG_D(message, ...)      LOG_DEBUG(message, ##__VA_ARGS__)
#else
#define NLOG_D(message, ...)
#endif

#if NET_LOG_LEVEL >= INFO_LEVEL
#define NLOG_I(message, ...)      LOG_INFO(message, ##__VA_ARGS__)
#else
#define NLOG_I(message, ...)
#endif

#if NET_LOG_LEVEL >= ERROR_LEVEL
#define NLOG_E(message, ...)      LOG_ERROR(message, ##__VA_ARGS__)
#else
#define NLOG_E(message, ...)
#endif

/* common */
#ifndef LOG_LEVEL
#define LOG_LEVEL   DEBUG_LEVEL
#endif

#if LOG_LEVEL >= DEBUG_LEVEL
#define LOG_D(message, ...)      LOG_DEBUG(message, ##__VA_ARGS__)
#else
#define LOG_D(message, ...)
#endif

#if LOG_LEVEL >= INFO_LEVEL
#define LOG_I(message, ...)      LOG_INFO(message, ##__VA_ARGS__)
#else
#define LOG_I(message, ...)
#endif

#if LOG_LEVEL >= ERROR_LEVEL
#define LOG_E(message, ...)      LOG_ERROR(message, ##__VA_ARGS__)
#else
#define LOG_E(message, ...)
#endif


static void LOG_DEBUG(const char* format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf("[DBG]");
	printf(buffer);
	va_end(args);
}

static void LOG_INFO(const char* format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf("[INF]");
	printf(buffer);
	va_end(args);
}

static void LOG_ERROR(const char* format, ...)
{
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf("[ERR]");
	printf(buffer);
	va_end(args);
}


#define LOG_AV_ENABLED
#define LOG_UI_ENABLED
#define LOG_NT_ENABLED

//#define _FILE strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
//#define PRINTFUNCTION(format, ...)      fprintf(stderr, format, __VA_ARGS__)

//#define LOG_ARGS(LOG_TAG_)   LOG_TAG_, _FILE, __FUNCTION__, __LINE__
//#define NEWLINE     "\n"
//#define LOG_DEBUG(message, args...)     PRINTFUNCTION(LOG_FMT message NEWLINE, LOG_ARGS(DEBUG_TAG), ## args)

/*
static void LOG_AV(const char* format, ...)
{
#ifdef	LOG_AV_ENABLED
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf("[DBG]");
	printf(buffer);
	va_end(args);
#endif
}

static void LOG_UI(const char* format, ...)
{
#ifdef	LOG_UI_ENABLED
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf("[INF]");
	printf(buffer);
	va_end(args);
#endif
}

static void LOG_NT(const char* format, ...)
{
#ifdef	LOG_NT_ENABLED
	char buffer[256];
	va_list args;
	va_start(args, format);
	vsprintf(buffer, format, args);
	printf("[ERR]");
	printf(buffer);
	va_end(args);
#endif
}
*/
