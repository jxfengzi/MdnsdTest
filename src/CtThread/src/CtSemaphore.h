/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtSemaphore.h
 *
 * @remark
 *
 */

#ifndef __CT_SEMAPHORE_H__
#define __CT_SEMAPHORE_H__

#include "ct_common.h"

CT_BEGIN_DECLS


#if (defined __LINUX__) || (defined __ANDROID__)
#include <semaphore.h>  
#endif

#ifdef __MAC_OSX__
#include <semaphore.h>  
#include <fcntl.h>
#include <limits.h>
#endif

#ifdef _WIN32
#define MAX_SEM_SIZE    128
typedef HANDLE          ct_sem_t;
#endif /* _WIN32 */

#if (defined __LINUX__) || (defined __ANDROID__)
typedef sem_t           ct_sem_t;
#endif

#ifdef __MAC_OSX__
typedef struct _ct_sem_t
{
    char                name[PATH_MAX + 1];
    sem_t *             sem;
} ct_sem_t;
#endif

typedef struct _CtSemaphore
{
    ct_sem_t           sem;
} CtSemaphore;

CtSemaphore * CtSemaphore_New(void);
CtRet CtSemaphore_Construct(CtSemaphore *thiz);
CtRet CtSemaphore_Dispose(CtSemaphore *thiz);
void CtSemaphore_Delete(CtSemaphore *thiz);

bool CtSemaphore_Wait(CtSemaphore *thiz);
bool CtSemaphore_Post(CtSemaphore *thiz);


CT_END_DECLS

#endif /* __CT_SEMAPHORE_H__ */
