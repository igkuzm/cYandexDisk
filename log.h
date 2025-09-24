/**
 * File              : log.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 19.03.2023
 * Last Modified Date: 25.10.2024
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

/**
 * log.h
 * Copyright (c) 2023 Igor V. Sementsov <ig.kuzm@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Debugging in Android/iOS and other platforms
 */

#ifndef k_lib_log_h
#define k_lib_log_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static char __buf[BUFSIZ];

static char *STR(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsprintf(__buf, fmt, args);
	va_end(args);
	return __buf;
}

static char * STR_ERR(const char *fmt, ...) {
	char str[BUFSIZ];
	va_list args;
	va_start(args, fmt);
	vsprintf(str, fmt, args);
	va_end(args);
	sprintf(__buf, "E/%s: _%s: %d: %s", __FILE__, __func__, __LINE__, str);
	return __buf;
}

static char * STR_LOG(const char *fmt, ...) {
	char str[BUFSIZ];
	va_list args;
	va_start(args, fmt);
	vsprintf(str, fmt, args);
	va_end(args);
	sprintf(__buf, "E/%s: _%s: %s", __FILE__, __func__, str);
	return __buf;
}

#ifdef __ANDROID__
	#include <android/log.h>
	#define LOG(fmt, ...) __android_log_print(ANDROID_LOG_INFO,  __FILE__, "_%s: " fmt, __func__, __VA_ARGS__)
	#define ERR(fmt, ...) __android_log_print(ANDROID_LOG_ERROR, __FILE__, "_%s: %d: " fmt, __func__, __LINE__, __VA_ARGS__) 	
#elif defined __APPLE__
	#include <CoreFoundation/CoreFoundation.h>
	void NSLog(CFStringRef format, ...);
	#define LOG(fmt, ...) NSLog(CFSTR(fmt), ##__VA_ARGS__)
	#define ERR(fmt, ...) LOG("E/_%s: %d: %s", __func__, __LINE__, STR(fmt, __VA_ARGS__)) 
#else

static void LOG(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsprintf(__buf, fmt, args);
	va_end(args);
	fprintf(stderr, "%s: _%s: %s\n",   __FILE__, __func__, __buf);
}

static void LOG(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsprintf(__buf, fmt, args);
	va_end(args);
	char _e[BUFSIZ]; 
	sprintf(_e, "E/%s: _%s: %d: %s", __FILE__, __func__, __LINE__, __buf);
	perror(_e);
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef k_lib_log_h */
