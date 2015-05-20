/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtCondition.h
 *
 * @remark
 *
 */

#ifndef __CT_CONDITION_H__
#define __CT_CONDITION_H__

#include "ct_common.h"
#include "CtMutex.h"

CT_BEGIN_DECLS


#ifdef _WIN32
typedef HANDLE              ct_cond_t;
#else /* Linux */
typedef pthread_cond_t      ct_cond_t;
#endif /* _WIN32 */

typedef struct _CtCondition
{
    CtMutex      mutex;
    ct_cond_t    job;
    bool         is_ready;
} CtCondition;

CtCondition * CtCondition_New(void);
CtRet CtCondition_Construct(CtCondition *thiz);
CtRet CtCondition_Dispose(CtCondition *thiz);
void CtCondition_Delete(CtCondition *thiz);

bool CtCondition_Wait(CtCondition *thiz);
bool CtCondition_NotifyOne(CtCondition *thiz);
bool CtCondition_NotifyAll(CtCondition *thiz);


CT_END_DECLS

#endif /* __CT_CONDITION_H__ */
