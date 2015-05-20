/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-7-9
 *
 * @file   ct_log.h
 *
 * @remark
 *
 */

#ifndef __CT_LOG_H__
#define __CT_LOG_H__


#ifdef __ANDROID__
    #include <android/log.h>
    #define LOG_OPEN(file)
    #define LOG_CLOSE()
    #define LOG_D(tag, format, ...) __android_log_print(ANDROID_LOG_DEBUG, tag, format, ##__VA_ARGS__)
    #define LOG_E(tag, format, ...) __android_log_print(ANDROID_LOG_ERROR, tag, format, ##__VA_ARGS__)
    #define LOG_I(tag, format, ...) __android_log_print(ANDROID_LOG_INFO, tag, format, ##__VA_ARGS__)
    #define LOG_V(tag, format, ...) __android_log_print(ANDROID_LOG_VERBOSE, tag, format, ##__VA_ARGS__)
    #define LOG_W(tag, format, ...) __android_log_print(ANDROID_LOG_WARN, tag, format, ##__VA_ARGS__)
#else
    #include "ct_log_print.h"
    #define LOG_OPEN(file)  __ct_log_open(file)
    #define LOG_CLOSE() __ct_log_close()

    #ifdef CT_DEBUG
        #define LOG_D(tag, format, ...) __ct_log_print(CT_LOG_DEBUG, tag, format, ##__VA_ARGS__)
    #else
        #define LOG_D(tag, format, ...)
    #endif /* CT_DEBUG */

    #define LOG_E(tag, format, ...) __ct_log_print(CT_LOG_ERROR, tag, format, ##__VA_ARGS__)
    #define LOG_I(tag, format, ...) __ct_log_print(CT_LOG_INFO, tag, format, ##__VA_ARGS__)
    #define LOG_V(tag, format, ...) __ct_log_print(CT_LOG_VERBOSE, tag, format, ##__VA_ARGS__)
    #define LOG_W(tag, format, ...) __ct_log_print(CT_LOG_WARN, tag, format, ##__VA_ARGS__)
#endif /* __ANDROID__ */

#ifdef CT_DEBUG
    #define LOG_TIME_BEGIN(tag, func) uint64_t usec_##func = ct_getusec()

    #ifdef __MAC_OSX__
        #define LOG_TIME_END(tag, func) LOG_D(tag, "%s, consuming: %lldms", #func, (ct_getusec() - usec_##func) / 1000)
    #else
        #define LOG_TIME_END(tag, func) LOG_D(tag, "%s, consuming: %lums", #func, (ct_getusec() - usec_##func) / 1000)
    #endif
#else
    #define LOG_TIME_BEGIN(tag, func)
    #define LOG_TIME_END(tag, func)
#endif

#endif /* __CT_LOG_H__ */
