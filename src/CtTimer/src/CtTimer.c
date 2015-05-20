/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtTimer.c
 *
 * @remark
 *
 */

#include "CtTimer.h"
#include "ct_memory.h"
#include "ct_log.h"

#define TAG     "CtTimer"

typedef enum _TimerLoopRet
{
    TIMER_LOOP_RET_BREAK = 0,
    TIMER_LOOP_RET_CONTINUE = 1,
    TIMER_LOOP_RET_TIMEOUT = 2,
} TimerLoopRet;

static void timer_loop(void *param);
static TimerLoopRet timer_loop_once(CtTimer *thiz);
static void CtTimer_StopLoop(CtTimer *thiz);

CtTimer * CtTimer_New(void)
{
    CtTimer *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtTimer *)ct_malloc(sizeof(CtTimer));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtTimer_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtTimer_Delete(thiz);
            thiz = NULL;
            break;
        }
    }
    while (0);

    return thiz;
}

CtRet CtTimer_Construct(CtTimer *thiz)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    do
    {
        memset(thiz, 0, sizeof(CtTimer));

        thiz->is_running = false;
        thiz->times = 0;
        thiz->interval = 0;
        thiz->running_interval = 0;
        thiz->listener = NULL;
        thiz->listener_ctx = NULL;

        ret = CtSocketIpc_Construct(&thiz->ipc);
        if (RET_FAILED(ret))
        {
            LOG_W(TAG, "CtSocketIpc_Construct failed");
            break;
        }

        ret = CtThread_Construct(&thiz->thread);
        if (RET_FAILED(ret))
        {
            LOG_W(TAG, "CtThread_Construct failed");
            break;
        }

        ret = CT_RET_OK;
    }
    while (0);

    return ret;
}

CtRet CtTimer_Initialize(CtTimer *thiz, uint64_t interval, uint32_t times)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    thiz->times = times;
    thiz->interval = interval;
    thiz->running_interval = interval;

    return CT_RET_OK;
}

CtRet CtTimer_Dispose(CtTimer *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    CtTimer_StopLoop(thiz);
    CtSocketIpc_Dispose(&thiz->ipc);

    return CT_RET_OK;
}

void CtTimer_Delete(CtTimer *thiz)
{
    RETURN_IF_FAIL(thiz);

    CtTimer_Dispose(thiz);
    ct_free(thiz);
}

CtRet CtTimer_Start(CtTimer *thiz, CtTimerListener listener, void *ctx)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    thiz->listener = listener;
    thiz->listener_ctx = ctx;

    do
    {
        if (thiz->is_running)
        {
            ret = CT_RET_E_STARTED;
            break;
        }

        ret = CtThread_Initialize(&thiz->thread, timer_loop, thiz, "timer");
        if (RET_FAILED(ret))
        {
            LOG_W(TAG, "CtThread_Initialize failed");
            break;
        }

        CtThread_Start(&thiz->thread);
        thiz->is_running = true;
        ret = CT_RET_OK;
    }
    while (0);

    return ret;
}

CtRet CtTimer_Stop(CtTimer *thiz)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    do
    {
        if (!thiz->is_running)
        {
            ret = CT_RET_E_STOPPED;
            break;
        }

        thiz->is_running = false;
        CtTimer_StopLoop(thiz);
        CtThread_Join(&thiz->thread);

        ret = CT_RET_OK;
    }
    while (0);

    return ret;
}

static void timer_loop(void *param)
{
    uint32_t i = 0;
    CtTimer *thiz = (CtTimer *)param;
    TimerLoopRet ret = TIMER_LOOP_RET_BREAK;
    bool listener_ret = false;

    if (thiz->times > 0)
    {
        for (i = 0; i < thiz->times; ++i)
        {
            ret = timer_loop_once(thiz);
            if (ret == TIMER_LOOP_RET_BREAK)
            {
                break;
            }

            if (ret != TIMER_LOOP_RET_TIMEOUT)
            {
                continue;
            }

            if (!thiz->listener(thiz, thiz->listener_ctx))
            {
                break;
            }
        }
    }
    else
    {
        int64_t previous_select_delta = 0;
        int64_t max_select_delta = thiz->interval >> 1;

        /* tiems is 0, loop forever */
        while (1)
        {
            int64_t delta = 0;
            uint64_t usec_before = 0;
            uint64_t usec_after = 0;

            usec_before = ct_getusec();
            ret = timer_loop_once(thiz);
            usec_after = ct_getusec();
            if (ret == TIMER_LOOP_RET_BREAK)
            {
                break;
            }

            if (ret != TIMER_LOOP_RET_TIMEOUT)
            {
                continue;
            }

            delta = (usec_after - usec_before - thiz->running_interval);
            if (0 >= delta)
            {
                int64_t delta_adjustment = -delta;
                delta = previous_select_delta;
                if (previous_select_delta > delta_adjustment)
                {
                    previous_select_delta -= delta_adjustment;
                }
                else
                {
                    previous_select_delta = 0;
                }
            }
            else
            {
                previous_select_delta = delta;
            }

            thiz->running_interval = thiz->interval - (delta < max_select_delta ? delta : max_select_delta);

            usec_before = ct_getusec();
            listener_ret = thiz->listener(thiz, thiz->listener_ctx);
            usec_after = ct_getusec();
            if (!listener_ret)
            {
                break;
            }

            delta = usec_after - usec_before;
            thiz->running_interval = (uint64_t)delta < thiz->running_interval ? thiz->running_interval - delta : 0;
            if (thiz->running_interval < (thiz->interval >> 1))
            {
                thiz->running_interval = thiz->interval >> 1;
            }
        }
    }
}

static TimerLoopRet timer_loop_once(CtTimer *thiz)
{
    TimerLoopRet result = TIMER_LOOP_RET_BREAK;
    int max_fd = 0;
    fd_set read_set;
    struct timeval tv;

    FD_ZERO(&read_set);  
    FD_SET(thiz->ipc.socket_fd, &read_set);

    /*
    * NOTE
    *   max_fd is not used on Windows,
    *   but on linux/unix it MUST Greater than socket_fd.
    */
    if (max_fd <= thiz->ipc.socket_fd)
    {
        max_fd = thiz->ipc.socket_fd + 1;
    }

    tv.tv_sec = (long)(thiz->running_interval / 1000000LL);
    tv.tv_usec = thiz->running_interval % 1000000LL;

    do
    {
        int ret = select(max_fd, &read_set, NULL, NULL, &tv);
        if (ret < 0)
        {
            LOG_W(TAG, "select: %s", strerror(errno));
            result = TIMER_LOOP_RET_BREAK;
            break;
        }
        
        if (ret == 0)
        {
            result = TIMER_LOOP_RET_TIMEOUT;
            break;
        }

        if (FD_ISSET(thiz->ipc.socket_fd, &read_set))
        {
            CtRet r = CT_RET_OK;
            IpcMsg m;
            memset(&m, 0, sizeof(IpcMsg));
            
            r = CtSocketIpc_Recv(&thiz->ipc, &m);
            if (RET_SUCCEEDED(r))
            {
                if (m.type == IPC_MSG_STOP)
                {
                    result = TIMER_LOOP_RET_BREAK;
                    break;
                }
            }
        }

        result = TIMER_LOOP_RET_CONTINUE;
    }
    while (0);

    return result;
} 

static void CtTimer_StopLoop(CtTimer *thiz)
{
    IpcMsg m;
    memset(&m, 0, sizeof(IpcMsg));
    m.type = IPC_MSG_STOP;

    CtSocketIpc_Send(&thiz->ipc, &m);
}
