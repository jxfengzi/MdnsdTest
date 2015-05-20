/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-7-9
 *
 * @file   ct_log_print.h
 *
 * @remark
 *
 */

#ifndef __CT_LOG_PRINT_H__
#define __CT_LOG_PRINT_H__

#include <stdarg.h>
#include "ct_common.h"

CT_BEGIN_DECLS

int __ct_log_open(const char *log_file);

int __ct_log_close(void);

/*
 * mi log priority values, in ascending priority order.
 */
typedef enum _ct_log_priority {
    CT_LOG_UNKNOWN = 0,
    CT_LOG_VERBOSE,
    CT_LOG_DEBUG,
    CT_LOG_INFO,
    CT_LOG_WARN,
    CT_LOG_ERROR,
} ct_log_priority;

/*
 * Send a simple string to the log.
 */
int __ct_log_write(int prio, const char *tag, const char *text);

/*
 * Send a formatted string to the log, used like printf(fmt,...)
 */
#if defined(__GNUC__)
int __ct_log_print(int prio, const char *tag, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
#else
int __ct_log_print(int prio, const char *tag, const char *fmt, ...);
#endif

/*
 * A variant of __ct_log_print() that takes a va_list to list
 * additional parameters.
 */
int __ct_log_vprint(int prio, const char *tag, const char *fmt, va_list ap);


CT_END_DECLS

#endif /* __CT_LOG_PRINT_H__ */
