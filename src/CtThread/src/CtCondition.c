/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtCondition.c
 *
 * @remark
 *
 */

#include "CtCondition.h"
#include "ct_memory.h"
#include "ct_log.h"

#define TAG     "CtCondition"

CtCondition * CtCondition_New(void)
{
    CtCondition *thiz = NULL;
    
    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtCondition *)ct_malloc(sizeof(CtCondition));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtCondition_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtCondition_Delete(thiz);
            thiz = NULL;
            break;
        }
    }
    while (0);

    return thiz;
}

CtRet CtCondition_Construct(CtCondition *thiz)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    do
    {
        memset(thiz, 0, sizeof(CtCondition));

        thiz->is_ready = false;

        ret = CtMutex_Construct(&thiz->mutex);
        if (RET_FAILED(ret))
        {
            break;
        }

#ifdef _WIN32
        thiz->job = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (thiz->job == NULL)
        {
            LOG_E(TAG, "CtCondition_Construct: CreateEvent failed");
            ret = CT_RET_E_INTERNAL;
            break;
        }
#else
        if (pthread_cond_init(&thiz->job, NULL) != 0)
        {
            LOG_E(TAG, "CtCondition_Construct: pthread_cond_init failed");
            ret = CT_RET_E_INTERNAL;
            break;
        }
#endif

        ret = CT_RET_OK;
    }
    while (false);

    return ret;
}

CtRet CtCondition_Dispose(CtCondition *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    CtMutex_Dispose(&thiz->mutex);

#ifdef _WIN32
    if (thiz->job)
    {
        CloseHandle(thiz->job);
    }
#else
    pthread_cond_destroy(&thiz->job);
#endif

    return CT_RET_OK;
}

void CtCondition_Delete(CtCondition *thiz)
{
    RETURN_IF_FAIL(thiz);

    CtCondition_NotifyAll(thiz);
    CtCondition_Dispose(thiz);
    ct_free(thiz);
}

bool CtCondition_Wait(CtCondition *thiz)
{
    bool result = false;

    RETURN_VAL_IF_FAIL(thiz, false);

#ifdef _WIN32
    switch (WaitForSingleObject(thiz->job, INFINITE))
    {
    case WAIT_OBJECT_0:
        result = true;
        break;

        /* 指定的对象是一个互斥对象，该对象没有被拥有该对象的线程在线程结束前释放。
        互斥对象的所有权被同意授予调用该函数的线程。互斥对象被设置成为无信号状态。*/
    case WAIT_ABANDONED:
        result = false;
        break;

    case WAIT_TIMEOUT:
        result = false;
        break;

    default:
        result = false;
        break;
    }
#else
    result = true;

    CtMutex_Lock(&thiz->mutex);
    thiz->is_ready = false;

    while (thiz->is_ready == false)
    {
        pthread_cond_wait(&thiz->job, &(thiz->mutex.mutex));
    }

    CtMutex_Unlock(&thiz->mutex);
#endif

    return result;
}

bool CtCondition_NotifyOne(CtCondition *thiz)
{
    bool ret = true;

    RETURN_VAL_IF_FAIL(thiz, false);

#ifdef _WIN32
    SetEvent(thiz->job);
#else

    CtMutex_Lock(&thiz->mutex);

    do
    {
        thiz->is_ready = true;

        if (pthread_cond_signal(&thiz->job) != 0)
        {
            ret = false;
            break;
        }

        ret = true;
    }
    while (0);

    CtMutex_Unlock(&thiz->mutex);
#endif

    return ret;
}

bool CtCondition_NotifyAll(CtCondition *thiz)
{
    bool ret = true;

    RETURN_VAL_IF_FAIL(thiz, false);

#ifdef _WIN32
    SetEvent(thiz->job);
#else

    CtMutex_Lock(&thiz->mutex);

    do
    {
        thiz->is_ready = true;

        if (pthread_cond_broadcast(&thiz->job) != 0)
        {
            ret = false;
            break;
        }

        ret = true;
    }
    while (0);

    CtMutex_Unlock(&thiz->mutex);
#endif

    return ret;
}
