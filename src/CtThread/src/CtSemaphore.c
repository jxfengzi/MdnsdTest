/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtSemaphore.c
 *
 * @remark
 *
 */

#include "CtSemaphore.h"
#include "ct_memory.h"
#include "ct_log.h"

#define TAG     "CtSemaphore"

#ifdef _WIN32
#else
bool ctx_gen_ipc_name(const char *name, char *full_name, uint32_t len)
{
    const char * dir = NULL;
    const char * slash = NULL;

    dir = getenv("PX_IPC_NAME");
    if (dir == NULL)
    {
    #ifdef POSIX_IPC_PREFIX
        dir = POSIX_IPC_PREFIX;
    #else
        dir = "/tmp/";
    #endif
    }

    /* dir must end in a slash */
    slash = (dir[strlen(dir) - 1] == '/') ? "" : "/";

    if (name == NULL)
    {
        uint64_t usec = ct_getusec();
    #ifdef __MAC_OSX__
        ct_snprintf(full_name, len, "%s%s%llu", dir, slash, usec);
    #else
        ct_snprintf(full_name, len, "%s%s%lu", dir, slash, usec);
    #endif
    }
    else
    {
        ct_snprintf(full_name, len, "%s%s%s", dir, slash, name);
    }

    return true;
}
#endif

CtSemaphore * CtSemaphore_New(void)
{
    CtSemaphore *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtSemaphore *)ct_malloc(sizeof(CtSemaphore));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtSemaphore_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtSemaphore_Delete(thiz);
            thiz = NULL;
            break;
        }
    }
    while (0);

    return thiz;
}

CtRet CtSemaphore_Construct(CtSemaphore *thiz)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    do
    {
        memset(thiz, 0, sizeof(CtSemaphore));

#ifdef _WIN32
        thiz->sem = CreateSemaphore(NULL, 0, MAX_SEM_SIZE, NULL);
        if (thiz->sem == NULL)
        {
            LOG_E(TAG, "CtSemaphore_Initialize: CreateSemaphore failed");
            ret = CT_RET_E_INTERNAL;
            break;
        }
#endif

#if (defined __LINUX__) || (defined __ANDROID__)
        if (sem_init(&thiz->sem, 0, 0) != 0)
        {
            LOG_E(TAG, "CtSemaphore_Initialize: sem_init %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }
#endif

#ifdef __MAC_OSX__
        ctx_gen_ipc_name(NULL, thiz->sem.name, PATH_MAX);
        thiz->sem.sem = sem_open(thiz->sem.name, O_CREAT, 0600, 0);
        if (thiz->sem.sem == SEM_FAILED)
        {
            LOG_E(TAG, "CtSemaphore_Initialize: sem_open %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }
#endif

        ret = CT_RET_OK;
    }
    while (false);

    return ret;
}

CtRet CtSemaphore_Dispose(CtSemaphore *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

#ifdef _WIN32
    if (thiz->sem)
    {
        CloseHandle(thiz->sem);
    }
#endif

#if (defined __LINUX__) || (defined __ANDROID__)
    sem_destroy(&thiz->sem);
#endif

#ifdef __MAC_OSX__
    if (thiz->sem.sem)
    {
        sem_close(thiz->sem.sem);
        sem_unlink(thiz->sem.name);
    }
#endif

    return CT_RET_OK;
}

void CtSemaphore_Delete(CtSemaphore *thiz)
{
    RETURN_IF_FAIL(thiz);

    CtSemaphore_Dispose(thiz);
    ct_free(thiz);
}

bool CtSemaphore_Wait(CtSemaphore *thiz)
{
    bool result = false;

    RETURN_VAL_IF_FAIL(thiz, false);

#ifdef _WIN32
    switch (WaitForSingleObject(thiz->sem, INFINITE))
    {
    case WAIT_OBJECT_0:
        result = true;
        break;

        /* 指定的对象是一个互斥对象，该对象没有被拥有该对象的线程在线程结束前释放。
        互斥对象的所有权被同意授予调用该函数的线程。互斥对象被设置成为无信号状态。*/
    case WAIT_ABANDONED:
        break;

    case WAIT_TIMEOUT:
        break;

    default:
        break;
    }
#endif

#if (defined __LINUX__) || (defined __ANDROID__)
    if (sem_wait(&thiz->sem) == 0)
    {
        result = true;
    }

#endif

#ifdef __MAC_OSX__
    if (sem_wait(thiz->sem.sem) == 0)
    {
        result = true;
    }
#endif

    return result;
}

bool CtSemaphore_Post(CtSemaphore *thiz)
{
    bool result = false;

    RETURN_VAL_IF_FAIL(thiz, false);

#ifdef _WIN32
    if (ReleaseSemaphore(thiz->sem, 1, NULL) == TRUE)
    {
        result = true;
    }
#endif

#if (defined __LINUX__) || (defined __ANDROID__)
    if (sem_post(&thiz->sem) == 0)
    {
        result = true;
    }
#endif

#ifdef __MAC_OSX__
    if (sem_post(thiz->sem.sem) == 0)
    {
        result = true;
    }
#endif

    return result;
}
