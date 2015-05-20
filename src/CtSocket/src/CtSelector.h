/*
* Copyright (C) 2013-2015
*
* @author jxfengzi@gmail.com
* @date   2013-11-19
*
* @file   CtSelector.h
*
* @remark
*		set tabstop=4
*		set shiftwidth=4
*		set expandtab
*/

#ifndef __CT_SELECTOR_H__
#define __CT_SELECTOR_H__

#include "ct_common.h"

CT_BEGIN_DECLS


typedef struct _CtSelector
{
    uint32_t                ref;
    int                     max_fd;
    fd_set                  read_set;
    fd_set                  write_set;
} CtSelector;

typedef enum _CtSelectorOperation
{
    SELECTOR_OP_READ = 0,
    SELECTOR_OP_WRITE = 1,
} CtSelectionOperation;

typedef enum _CtSelectorRet
{
    SELECTOR_RET_OK = 0,
    SELECTOR_RET_ERROR = 1,
    SELECTOR_RET_TIMEOUT = 2,
} CtSelectorRet;

CtSelector * CtSelector_New(void);
CtRet CtSelector_Construct(CtSelector *thiz);
CtRet CtSelector_Dispose(CtSelector *thiz);
void CtSelector_Delete(CtSelector *thiz);

void CtSelector_Reset(CtSelector *thiz);
void CtSelector_Register(CtSelector *thiz, int fd, CtSelectionOperation op);
CtSelectorRet CtSelector_RunOnce(CtSelector *thiz, uint32_t ms);
bool CtSelector_IsReadable(CtSelector *thiz, int fd);
bool CtSelector_IsWriteable(CtSelector *thiz, int fd);


CT_END_DECLS

#endif /* __CT_SELECTOR_H__ */
