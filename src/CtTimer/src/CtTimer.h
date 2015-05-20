/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtTimer.h
 *
 * @remark
 *
 */

#ifndef __CT_TIMER_H__
#define __CT_TIMER_H__

#include "ct_common.h"
#include "CtSocketIpc.h"
#include "CtThread.h"

CT_BEGIN_DECLS

struct _CtTimer;
typedef struct _CtTimer CtTimer;

typedef bool(*CtTimerListener)(CtTimer *thiz, void *ctx);

struct _CtTimer
{
    bool                    is_running;
    uint32_t                times;
    uint64_t                interval;
    uint64_t                running_interval;
    CtTimerListener         listener;
    void                  * listener_ctx;
    CtThread                thread;
    CtSocketIpc             ipc;
};

CtTimer * CtTimer_New(void);
CtRet CtTimer_Construct(CtTimer *thiz);
CtRet CtTimer_Initialize(CtTimer *thiz, uint64_t interval, uint32_t times);
CtRet CtTimer_Dispose(CtTimer *thiz);
void CtTimer_Delete(CtTimer *thiz);

CtRet CtTimer_Start(CtTimer *thiz, CtTimerListener listener, void *ctx);
CtRet CtTimer_Stop(CtTimer *thiz);


CT_END_DECLS

#endif /* __MI_TIMER_H__ */
