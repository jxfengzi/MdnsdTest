/* * Copyright (C) 2013-2015
*
* @author jxfengzi@gmail.com
* @date   2013-11-19
*
* @file   CtSocketIpc.c
*
* @remark
*
*/

#include "CtSocketIpc.h"
#include "ct_memory.h"
#include "ct_log.h"
#include "ct_socket.h"

#define TAG             "CtSocketIpc"

#define LOCALHOST       "127.0.0.1"
#define MSG_STOP        "MSG_STOP"
#define MSG_RESELECT    "MSG_RESELECT" 

static int socket_read(int fd, char *buf, size_t buf_len, char *ip, uint32_t ip_len, uint16_t *port);
static int socket_write(int fd, const char *ip, uint16_t port, const char *buf, uint32_t len);

CtSocketIpc * CtSocketIpc_New(void)
{
    CtSocketIpc *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtSocketIpc *)ct_malloc(sizeof(CtSocketIpc));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtSocketIpc_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtSocketIpc_Delete(thiz);
            thiz = NULL;
            break;
        }
    }
    while (0);

    return thiz;
}

CtRet CtSocketIpc_Construct(CtSocketIpc *thiz)
{
    CtRet ret = CT_RET_OK;
    uint16_t port = 0;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    memset(thiz, 0, sizeof(CtSocketIpc));

    do
    {
        int net_timeout = 1000;
        struct sockaddr_in addr;
        int addr_len = sizeof(addr);

        thiz->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (thiz->socket_fd < 0)
        {
            LOG_W(TAG, "socket: %s", strerror(errno));
            ret = CT_RET_E_SOCKET_FD;
            break;
        }

        ret = ct_socket_set_nonblock(thiz->socket_fd);
        if (RET_FAILED(ret))
        {
            ret = CT_RET_E_INTERNAL;
            break;
        }

#ifdef _WIN32
        if (setsockopt(thiz->socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&net_timeout, sizeof(int)) == SOCKET_ERROR)
        {
            LOG_W(TAG, "setsockopt error: %d", GetLastError());
            ret = CT_RET_E_INTERNAL;
            break;
        }
#else
        if (setsockopt(thiz->socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&net_timeout, sizeof(int)) < 0)
        {
            LOG_W(TAG, "setsockopt error: %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }
#endif

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = htons(port);

#ifdef _WIN32
        if (bind(thiz->socket_fd, (struct sockaddr *)&addr, addr_len) == SOCKET_ERROR)
        {
            LOG_W(TAG, "bind error: %d", GetLastError());
            ret = CT_RET_E_INTERNAL;
            break;
        }
#else
        if (bind(thiz->socket_fd, (struct sockaddr *)&addr, addr_len) < 0)
        {
            LOG_W(TAG, "bind error: %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }
#endif

        if (port == 0)
        {
            thiz->socket_port = ct_socket_get_port(thiz->socket_fd);
        }
        else
        {
            thiz->socket_port = port;
        }

        LOG_D(TAG, "socket fd: %d, port: %d", thiz->socket_fd, thiz->socket_port);
        ret = CT_RET_OK;
    }
    while (0);

    return ret;
}

CtRet CtSocketIpc_Dispose(CtSocketIpc *thiz)
{
    do
    {
        if (thiz->socket_fd == 0)
        {
            break;
        }

#ifdef _WIN32
        closesocket(thiz->socket_fd);
#else
        shutdown(thiz->socket_fd, SHUT_RDWR);
        close(thiz->socket_fd);
#endif

        thiz->socket_fd = 0;
    } while (0);

    return CT_RET_OK;
}

void CtSocketIpc_Delete(CtSocketIpc *thiz)
{
    RETURN_IF_FAIL(thiz);

    CtSocketIpc_Dispose(thiz);
    ct_free(thiz);
}

int CtSocketIpc_GetFd(CtSocketIpc *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, 0);
    return thiz->socket_fd;
}

CtRet CtSocketIpc_Send(CtSocketIpc *thiz, IpcMsg * ipc_msg)
{
    CtRet ret = CT_RET_OK;
    const char * m = NULL;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);
    RETURN_VAL_IF_FAIL(ipc_msg, CT_RET_E_ARG_NULL);

    switch (ipc_msg->type)
    {
    case IPC_MSG_STOP:
        m = MSG_STOP;
        break;

    case IPC_MSG_RESELECT:
        m = MSG_RESELECT;
        break;

    case IPC_MSG_USER_DEFINED:
        m = ipc_msg->msg;
        break;

    default:
        break;
    }

    do
    {
        if (m == NULL)
        {
            ret = CT_RET_E_ARG_INVALID;
            break;
        }

        if (strlen(m) == 0)
        {
            ret = CT_RET_E_ARG_INVALID;
            break;
        }

        socket_write(thiz->socket_fd, LOCALHOST, thiz->socket_port, m, strlen(m));

        ret = CT_RET_OK;
    } while (0);

    return ret;
}

CtRet CtSocketIpc_Recv(CtSocketIpc *thiz, IpcMsg * ipc_msg)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    do
    {
        char ip[CT_IP_LEN] = { 0 };
        uint16_t port = 0;
        uint32_t len = 0;

        if (socket_read(thiz->socket_fd, ipc_msg->msg, MAX_IPC_MSG_LEN, ip, CT_IP_LEN, &port) <= 0)
        {
            LOG_W(TAG, "socket_read: %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }

        if (strncmp(ipc_msg->msg, MSG_STOP, strlen(MSG_STOP)) == 0)
        {
            ipc_msg->type = IPC_MSG_STOP;
        }
        else if (strncmp(ipc_msg->msg, MSG_RESELECT, strlen(MSG_RESELECT)) == 0)
        {
            ipc_msg->type = IPC_MSG_RESELECT;
        }

        ret = CT_RET_OK;
    } while (0);

    return ret;
}

CtRet CtSocketIpc_SendStopMsg(CtSocketIpc *thiz)
{
    IpcMsg m;
    memset(&m, 0, sizeof(IpcMsg));
    m.type = IPC_MSG_STOP;

    return CtSocketIpc_Send(thiz, &m);
}

static int socket_read(int fd, char *buf, size_t buf_len, char *ip, uint32_t ip_len, uint16_t *port)
{
    struct sockaddr_in sender_addr;
    socklen_t addr_len = (socklen_t) sizeof(sender_addr);
    int received = 0;
    const char *from_ip = NULL;

    RETURN_VAL_IF_FAIL(buf, 0);

    memset(buf, 0, buf_len);

    do
    {
        received = recvfrom(fd, buf, buf_len, 0, (struct sockaddr *)&sender_addr, &addr_len);

#ifdef _WIN32
        if (received < 0)
        {
            LOG_W(TAG, "recvfrom < 0, e = %d", GetLastError());
            break;
        }
#else
        if (received < 0)
        {
            LOG_W(TAG, "recvfrom < 0: %s", strerror(errno));
            break;
        }
#endif
        else if (received == 0)
        {
            break;
        }

        from_ip = inet_ntoa(sender_addr.sin_addr);
        strncpy(ip, from_ip, ip_len);

        *port = ntohs(sender_addr.sin_port);

    } while (0);

    return received;
}

static int socket_write(int fd, const char *ip, uint16_t port, const char *buf, uint32_t len)
{
    struct sockaddr_in addr;
    int addr_len = sizeof(addr);

    RETURN_VAL_IF_FAIL(ip, 0);
    RETURN_VAL_IF_FAIL(buf, 0);

    memset(&addr, 0, addr_len);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    return sendto(fd, buf, len, 0, (struct sockaddr *)&addr, addr_len);
}
