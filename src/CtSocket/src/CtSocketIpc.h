/*
* Copyright (C) 2013-2015
*
* @author jxfengzi@gmail.com
* @date   2013-11-19
*
* @file   CtSocketIpc.h
*
* @remark
*
*/

#ifndef __CT_SOCKET_IPC_H__
#define __CT_SOCKET_IPC_H__

#include "ct_common.h"

CT_BEGIN_DECLS


#define MAX_IPC_MSG_LEN 1024

typedef enum _IpcMsgType
{
    IPC_MSG_STOP = 0,
    IPC_MSG_RESELECT = 1,
    IPC_MSG_USER_DEFINED = 2,
} IpcMsgType;

typedef struct _IpcMsg {
    IpcMsgType  type;
    char        msg[MAX_IPC_MSG_LEN];
} IpcMsg;

typedef struct _CtSocketIpc
{
    int         socket_fd;
    uint16_t    socket_port;
} CtSocketIpc;

CtSocketIpc * CtSocketIpc_New(void);
CtRet CtSocketIpc_Construct(CtSocketIpc *thiz);
CtRet CtSocketIpc_Dispose(CtSocketIpc *thiz);
void CtSocketIpc_Delete(CtSocketIpc *thiz);

int CtSocketIpc_GetFd(CtSocketIpc *thiz);
CtRet CtSocketIpc_Send(CtSocketIpc *thiz, IpcMsg * ipc_msg);
CtRet CtSocketIpc_Recv(CtSocketIpc *thiz, IpcMsg * ipc_msg);
CtRet CtSocketIpc_SendStopMsg(CtSocketIpc *thiz);


CT_END_DECLS

#endif /* __CT_SOCKET_IPC_H__ */
