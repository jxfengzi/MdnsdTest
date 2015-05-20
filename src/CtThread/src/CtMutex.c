/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtMutex.c
 *
 * @remark
 *
 */

#include "CtMutex.h"
#include "ct_memory.h"
#include "ct_log.h"

#define TAG     "CtMutex"

CtMutex * CtMutex_New(void)
{
    CtMutex *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtMutex *)ct_malloc(sizeof(CtMutex));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtMutex_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtMutex_Delete(thiz);
            thiz = NULL;
            break;
        }
    }
    while (0);

    return thiz;
}

CtRet CtMutex_Construct(CtMutex *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    memset(thiz, 0, sizeof(CtMutex));

#ifdef _WIN32
    InitializeCriticalSection(&thiz->mutex);
#else
    if (pthread_mutex_init(&thiz->mutex, NULL) != 0)
    {
        LOG_E(TAG, "CtMutex_Construct: pthread_mutex_init failed");
        return CT_RET_E_INTERNAL;
    }
#endif

    return CT_RET_OK;
}

CtRet CtMutex_Dispose(CtMutex *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

#ifdef _WIN32
    DeleteCriticalSection(&thiz->mutex);
#else
    pthread_mutex_destroy(&thiz->mutex);
#endif

    return CT_RET_OK;
}

void CtMutex_Delete(CtMutex *thiz)
{
    RETURN_IF_FAIL(thiz);

    CtMutex_Dispose(thiz);
    ct_free(thiz);
}

bool CtMutex_Lock(CtMutex *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, false);

#ifdef _WIN32
    EnterCriticalSection(&thiz->mutex);
#else
    if (pthread_mutex_lock(&thiz->mutex) != 0)
    {
        LOG_W(TAG, "CtMutex_Lock: pthread_mutex_lock failed");
        return false;
    }
#endif

    return true;
}

bool CtMutex_Unlock(CtMutex *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, false);

#ifdef _WIN32
    LeaveCriticalSection(&thiz->mutex);
#else
    if (pthread_mutex_unlock(&thiz->mutex) != 0)
    {
        LOG_W(TAG, "CtMutex_Lock: pthread_mutex_unlock failed");
        return false;
    }
#endif

    return true;
}