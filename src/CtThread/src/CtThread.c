/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtThread.c
 *
 * @remark
 *
 */

#include "CtThread.h"
#include "ct_memory.h"
#include "ct_log.h"

#define TAG                 "CtThread"
#define THREAD_UNNAMED      "unnamed"

static void * thread_run(void *param);

CtThread * CtThread_New(void)
{
    CtThread *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtThread *)ct_malloc(sizeof(CtThread));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtThread_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtThread_Delete(thiz);
            thiz = NULL;
            break;
        }
    }
    while (0);

    return thiz;
}

CtRet CtThread_Construct(CtThread *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    memset(thiz, 0, sizeof(CtThread));

    return CT_RET_OK;
}

CtRet CtThread_Initialize(CtThread *thiz, ThreadLoop loop, void *param, const char *name)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    if (name != NULL)
    {
        strncpy(thiz->name, name, THREAD_NAME_LEN);
    }
    else
    {
        strncpy(thiz->name, THREAD_UNNAMED, THREAD_NAME_LEN);
    }

    thiz->loop = loop;
    thiz->thread_param = param;
    thiz->status = ThreadInit;

#ifdef _WIN32
    thiz->thread_handler = NULL;
    thiz->thread_id = 0;
#else /* Linux */
    thiz->thread_handler = 0;
    thiz->thread_id = 0;
#endif /* _WIN32 */

    return CT_RET_OK;
}

CtRet CtThread_Dispose(CtThread *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    CtThread_Join(thiz);

    memset(thiz->name, 0, THREAD_NAME_LEN);

    thiz->loop = NULL;
    thiz->thread_param = NULL;
    thiz->status = ThreadInit;

#ifdef _WIN32
    thiz->thread_handler = NULL;
    thiz->thread_id = 0;
#else /* Linux */
    thiz->thread_handler = 0;
    thiz->thread_id = 0;
#endif /* _WIN32 */

    return CT_RET_OK;
}

void CtThread_Delete(CtThread *thiz)
{
    CtThread_Dispose(thiz);
    ct_free(thiz);
}

const char * CtThread_GetName(CtThread *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, NULL);

    return thiz->name;
}

bool CtThread_Start(CtThread *thiz)
{
    int ret = 0;

    RETURN_VAL_IF_FAIL(thiz, false);

    LOG_D(TAG, "CtThread_Start: %s", thiz->name);

    if (thiz->status == ThreadRunning)
    {
        return false;
    }

    if (thiz->loop == NULL)
    {
        return false;
    }

#ifdef _WIN32
    thiz->thread_handler = CreateThread(NULL,
        0,
        (LPTHREAD_START_ROUTINE)thread_run,
        (LPVOID)thiz,
        0,
        &thiz->thread_id);

#else
    typedef void* (*LPTHREAD_START_ROUTINE)(void *);
    ret = pthread_create(&thiz->thread_id,
        NULL,
        (LPTHREAD_START_ROUTINE)thread_run,
        (void*)thiz);

    thiz->thread_handler = (int)thiz->thread_id;
#endif

    thiz->status = ThreadRunning;

    return true;
}

bool CtThread_Join(CtThread *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, false);

    if (thiz->status == ThreadRunning)
    {
        LOG_D(TAG, "CtThread_Join: %s", thiz->name);

#ifdef _WIN32
        if (thiz->thread_handler != INVALID_HANDLE_VALUE)
        {
            WaitForSingleObject(thiz->thread_handler, INFINITE);
            CloseHandle(thiz->thread_handler);
            thiz->thread_handler = INVALID_HANDLE_VALUE;
        }
#else
        void *status = NULL;

        if (thiz->thread_handler != 0)
        {
            pthread_join(thiz->thread_id, &status);
            thiz->thread_handler = 0;
        }
#endif

        thiz->status = ThreadStop;
    }

    return true;
}

static void * thread_run(void *param)
{
    CtThread *thiz = (CtThread *)param;
    thiz->loop(thiz->thread_param);

    return NULL;
}
