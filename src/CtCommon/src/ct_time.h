/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   ct_time.h
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 */

#ifndef __CT_TIME_H__
#define __CT_TIME_H__

#include "ct_typedef.h"

#ifdef _WIN32
#include <time.h>
#else /* Linux */
#include <unistd.h>
#include <sys/time.h>
#endif /* _WIN32 */

CT_BEGIN_DECLS


#ifdef _WIN32
int gettimeofday(struct timeval *tv, void *tz);
#endif /* _WIN32 */

void ct_sleep(int second);
void ct_usleep(int usecond);
int ct_getstrtime(char buf[], int len);

static CT_INLINE uint64_t ct_getusec(void)
{
    struct timeval __tv__ = {0};
    return (gettimeofday(&__tv__, NULL) ? 0LL : 1000000LL * __tv__.tv_sec + __tv__.tv_usec); 
}


CT_END_DECLS

#endif /* __CT_TIME_H__ */
