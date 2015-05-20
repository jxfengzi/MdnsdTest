/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtThread.h
 *
 * @remark
 *
 */

#ifndef __CT_THREAD_H__
#define __CT_THREAD_H__

#include "ct_common.h"

#ifdef _WIN32
#else
#include <pthread.h> 
#endif

CT_BEGIN_DECLS


#ifdef _WIN32
typedef void *              ThreadHandler;
typedef unsigned long       ThreadId;
#else /* Linux */
typedef int                 ThreadHandler;
typedef pthread_t           ThreadId;
#endif /* _WIN32 */

#define THREAD_NAME_LEN     256

typedef enum _CtThreadStatus
{
    ThreadInit = 0,
    ThreadRunning = 1,
    ThreadStop = 2,
} CtThreadStatus;

typedef void (*ThreadLoop)(void *param);

typedef struct _CtThread
{
    char                    name[THREAD_NAME_LEN];
    CtThreadStatus          status;
    ThreadLoop              loop;
    ThreadHandler           thread_handler;
    ThreadId                thread_id;
    void *                  thread_param;
} CtThread;

CtThread * CtThread_New(void);
CtRet CtThread_Construct(CtThread *thiz);
CtRet CtThread_Initialize(CtThread *thiz, ThreadLoop loop, void *param, const char *name);
CtRet CtThread_Dispose(CtThread *thiz);
void CtThread_Delete(CtThread *thiz);

const char * CtThread_GetName(CtThread *thiz);
bool CtThread_Start(CtThread *thiz);
bool CtThread_Join(CtThread *thiz);


CT_END_DECLS

#endif /* __CT_THREAD_H__ */
