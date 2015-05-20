/*
* Copyright (C) 2013-2015
*
* @author jxfengzi@gmail.com
* @date   2013-11-19
*
* @file   CtSelector.c
*
* @remark
*		set tabstop=4
*		set shiftwidth=4
*		set expandtab
*/

#include "CtSelector.h"
#include "ct_memory.h"
#include "ct_log.h"

#define TAG     "CtSelector"

CtSelector * CtSelector_New(void)
{
    CtSelector *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtSelector *)ct_malloc(sizeof(CtSelector));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtSelector_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtSelector_Delete(thiz);
            thiz = NULL;
            break;
        }
    } while (0);

    return thiz;
}

CtRet CtSelector_Construct(CtSelector *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    memset(thiz, 0, sizeof(CtSelector));

    return CT_RET_OK;
}

CtRet CtSelector_Dispose(CtSelector *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    return CT_RET_OK;
}

void CtSelector_Delete(CtSelector *thiz)
{
    RETURN_IF_FAIL(thiz);

    CtSelector_Dispose(thiz);
    ct_free(thiz);
}

void CtSelector_Reset(CtSelector *thiz)
{
    RETURN_IF_FAIL(thiz);

    thiz->max_fd = 0;
    FD_ZERO(&thiz->read_set);
    FD_ZERO(&thiz->write_set);
}

void CtSelector_Register(CtSelector *thiz, int fd, CtSelectionOperation op)
{
    RETURN_IF_FAIL(thiz);

    if (op == SELECTOR_OP_READ)
    {
        FD_SET(fd, &thiz->read_set);
    }
    else
    {
        FD_SET(fd, &thiz->write_set);
    }

    /**
    * NOTE
    *   max_fd is not used on Windows,
    *   but on linux/unix it MUST Greater than socket_fd.
    */
    if (thiz->max_fd <= fd)
    {
        thiz->max_fd = fd + 1;
    }
}

CtSelectorRet CtSelector_RunOnce(CtSelector *thiz, uint32_t ms)
{
    CtSelectorRet result = SELECTOR_RET_OK;
    int ret = 0;

    RETURN_VAL_IF_FAIL(thiz, SELECTOR_RET_ERROR);

    if (ms == 0)
    {
        ret = select(thiz->max_fd, &thiz->read_set, &thiz->write_set, NULL, NULL);
    }
    else
    {
        struct timeval tv;
        tv.tv_sec = ms / 1000;
        tv.tv_usec = ms % 1000;

        ret = select(thiz->max_fd, &thiz->read_set, &thiz->write_set, NULL, &tv);
    }

    if (ret == 0)
    {
        result = SELECTOR_RET_TIMEOUT;
    }
    else if (ret < 0)
    {
        LOG_D(TAG, "select failed");
        result = SELECTOR_RET_ERROR;
    }
    else
    {
        result = SELECTOR_RET_OK;
    }

    return result;
}

bool CtSelector_IsReadable(CtSelector *thiz, int fd)
{
    RETURN_VAL_IF_FAIL(thiz, false);

    return FD_ISSET(fd, &thiz->read_set);
}

bool CtSelector_IsWriteable(CtSelector *thiz, int fd)
{
    RETURN_VAL_IF_FAIL(thiz, false);

    return FD_ISSET(fd, &thiz->write_set);
}
