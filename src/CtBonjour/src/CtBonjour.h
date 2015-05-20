/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtBonjour.h
 *
 * @remark
 *
 */

#ifndef __CT_BONJOUR_H__
#define __CT_BONJOUR_H__

#include "ct_common.h"


CT_BEGIN_DECLS

struct _CtBonjour;
typedef struct _CtBonjour CtBonjour;

CtBonjour * CtBonjour_New(void);
void CtBonjour_Delete(CtBonjour *thiz);

CtRet CtBonjour_DiscoverService(CtBonjour *thiz, const char *type, const char *domain, void *ctx);
CtRet CtBonjour_StopServiceDiscovery(CtBonjour *thiz);


CT_END_DECLS

#endif /* __CT_BONJOUR_H__ */
