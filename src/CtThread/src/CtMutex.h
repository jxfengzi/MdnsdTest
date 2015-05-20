/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtMutex.h
 *
 * @remark
 *    set tabstop=4
 *    set shiftwidth=4
 *    set expandtab
 */

#ifndef __CT_MUTEX_H__
#define __CT_MUTEX_H__

#ifdef _WIN32
#else
#include <pthread.h>
#endif

#include "ct_common.h"

CT_BEGIN_DECLS

#ifdef _WIN32
    typedef CRITICAL_SECTION    ct_mutex_t;
#else /* Linux */
    typedef pthread_mutex_t     ct_mutex_t;
#endif /* _WIN32 */

typedef struct _CtMutex
{
    ct_mutex_t    mutex;
} CtMutex;

CtMutex * CtMutex_New(void);
CtRet CtMutex_Construct(CtMutex *thiz);
CtRet CtMutex_Dispose(CtMutex *thiz);
void CtMutex_Delete(CtMutex *thiz);

bool CtMutex_Lock(CtMutex *thiz);
bool CtMutex_Unlock(CtMutex *thiz);


CT_END_DECLS

#endif /* __CT_MUTEX_H__ */
