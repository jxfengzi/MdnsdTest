/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   ct_socket.c
 *
 * @remark
 *
 */

#include "ct_socket.h"
#include "ct_log.h"
#include "ct_memory.h"
#include <string.h>


#ifdef _WIN32
#include <ws2tcpip.h>
#include <windows.h>
#include <Iphlpapi.h>   // 用于枚举所有网卡IP
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif // _WIN32

#define TAG "ct_socket"

static CtRet udp_join_multicast_group(int fd, const char *group, uint16_t port);
static CtRet udp_join_multicast_group_with_all_ip(int fd, const char *group);
static CtRet udp_join_multicast_group_with_ip(int fd, const char *group, unsigned long ip);
static CtRet udp_leave_multicast_group(int fd);

CtRet ct_socket_init(void)
{
    CtRet ret = CT_RET_OK;
    static bool is_init = false;

    do
    {
    #ifdef _WIN32
        WORD wVersionRequested;
        WSADATA wsaData;
    #endif

        if (is_init)
        {
            ret = CT_RET_OK;
            break;
        }

    #ifdef _WIN32
        wVersionRequested = MAKEWORD(2, 0);
        if (WSAStartup(wVersionRequested, &wsaData) != 0)
        {
            LOG_W(TAG, "WSAStartup failed");
            ret = CT_RET_E_INTERNAL;
            break;
        }
    #else
        // Ignore SIGPIPE signal, so if browser cancels the request, it won't kill the whole process.
        (void)signal(SIGPIPE, SIG_IGN);
    #endif

        ret = CT_RET_OK;
        is_init = true;

    } while (0);

    return ret;
}

CtRet ct_socket_set_nonblock(int fd)
{
    CtRet ret = CT_RET_OK;

#ifdef _WIN32
    do 
    {
        unsigned long ul = 1;
        if (ioctlsocket(fd, FIONBIO, &ul) == SOCKET_ERROR)
        {
            LOG_W(TAG, "ioctlsocket failed: %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }

        ret = CT_RET_OK;

    } while (0);
#else
    do 
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0)
        {
            LOG_W(TAG, "ioctlsocket failed: %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }

        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        ret = CT_RET_OK;

    } while (0);
#endif

    return ret;
}

CtRet ct_socket_set_block(int fd)
{
    CtRet ret = CT_RET_OK;

#ifdef _WIN32
    do 
    {
        unsigned long ul = 0;
        if (ioctlsocket(fd, FIONBIO, &ul) == SOCKET_ERROR)
        {
            LOG_W(TAG, "ioctlsocket failed: %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }

        ret = CT_RET_OK;

    } while (0);
#else
    do 
    {
        int flags = fcntl(fd, F_GETFL, 0);
        if (flags < 0)
        {
            LOG_W(TAG, "fcntl failed: %s", strerror(errno));
            ret = CT_RET_E_INTERNAL;
            break;
        }

        flags &= ~O_NONBLOCK;
        fcntl(fd, F_SETFL, flags);

        ret = CT_RET_OK;

    } while (0);
#endif

    return ret;
}

uint32_t ct_socket_get_ipv4_bytes(int fd)
{
    uint32_t ip = 0;

    do
    {
        struct sockaddr_in sin;
        socklen_t len = (socklen_t) sizeof(sin);

        if (getsockname(fd, (struct sockaddr *)&sin, &len) == 0)
        {
            // ip = sin.sin_addr.S_un.S_addr;
            LOG_W(TAG, "ct_socket_get_ipv4_bytes NOT implemented");
            break;
        }

        LOG_W(TAG, "getsockname failed: %s", strerror(errno));
    }
    while (0);

    return ip;
}

CtRet ct_socket_get_ip(int fd, char ip[], uint32_t ip_len)
{
    CtRet ret = CT_RET_OK;

    do
    {
        struct sockaddr_in sin;
        socklen_t len = (socklen_t) sizeof(sin);

        if (getsockname(fd, (struct sockaddr *)&sin, &len) == 0)
        {
            const char *addr = inet_ntoa(sin.sin_addr);
            strncpy(ip, addr, ip_len);
            ret = CT_RET_OK;
            break;
        }

        LOG_W(TAG, "getsockname failed: %s", strerror(errno));
        ret = CT_RET_E_INTERNAL;

    } while (0);
    
    return ret;
}

uint16_t ct_socket_get_port(int fd)
{
    uint16_t port = 0;
    struct sockaddr_in sin;
    socklen_t len = (socklen_t) sizeof(sin);

    if (getsockname(fd, (struct sockaddr *)&sin, &len) == 0)
    {
        port = ntohs(sin.sin_port);
    }

    return port;
}

bool ct_socket_has_error(int fd)
{
    bool ret = false;

    do
    {
    #ifdef _WIN32
        DWORD error = 0;
    #else
        char error = 0;
    #endif

        socklen_t len = sizeof(error);
        if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&error, &len) < 0)
        {
            ret = true;
            break;
        }

        if (error)
        {
            ret = true;
            break;
        }

        ret = false;
    }
    while (0);

    return ret;
}

CtRet ct_socket_waiting_for_read(int fd, uint32_t timeout)
{
    CtRet result = CT_RET_OK;

    do
    {
        int ret = 0;
        int max_fd = 0;
        fd_set read_set;
        struct timeval tv;

        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);

        /*
        * NOTE
        *   max_fd is not used on Windows,
        *   but on linux/unix it MUST Greater than socket_no.
        */
        max_fd = fd + 1;

        if (timeout > 0)
        {
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = timeout % 1000;
            ret = select(max_fd, &read_set, NULL, NULL, &tv);
        }
        else
        {
            ret = select(max_fd, &read_set, NULL, NULL, NULL);
        }

        if (ret < 0)
        {
            LOG_D(TAG, "select failed: %s", strerror(errno));
            result = CT_RET_E_SELECT;
            break;
        }

        if (ret == 0)
        {
            result = CT_RET_E_TIMEOUT;
            break;
        }

        if (ct_socket_has_error(fd))
        {
            result = CT_RET_E_SOCKET_READ;
            break;
        }

        if (FD_ISSET(fd, &read_set))
        {
            result = CT_RET_OK;
            break;
        }
    } while (false);

    return result;
}

CtRet ct_socket_waiting_for_write(int fd, uint32_t timeout)
{
    CtRet result = CT_RET_OK;

    do
    {
        int ret = 0;
        int max_fd = 0;
        fd_set write_set;
        struct timeval tv;

        FD_ZERO(&write_set);
        FD_SET(fd, &write_set);

        /*
        * NOTE
        *   max_fd is not used on Windows,
        *   but on linux/unix it MUST Greater than socket_no.
        */
        max_fd = fd + 1;

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = timeout % 1000;

        if (timeout > 0)
        {
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = timeout % 1000;
            ret = select(max_fd, NULL, &write_set, NULL, &tv);
        }
        else
        {
            ret = select(max_fd, NULL, &write_set, NULL, NULL);
        }

        if (ret < 0)
        {
            LOG_D(TAG, "select failed: %s", strerror(errno));
            result = CT_RET_E_SELECT;
            break;
        }

        if (ret == 0)
        {
            result = CT_RET_E_TIMEOUT;
            break;
        }

        if (ct_socket_has_error(fd))
        {
            result = CT_RET_E_SOCKET_WRITE;
            break;
        }

        if (FD_ISSET(fd, &write_set))
        {
            result = CT_RET_OK;
            break;
        }
    } while (false);

    return result;
}

CtRet ct_tcp_open(int *fd, bool block)
{
    *fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (*fd < 0)
    {
        return CT_RET_E_SOCKET_FD;
    }

    if (block)
    {
        return CT_RET_OK;
    }

    return ct_socket_set_nonblock(*fd);
}

CtRet ct_tcp_close(int fd)
{
#ifdef _WIN32
    closesocket(fd);
#else
    shutdown(fd, SHUT_RDWR);
    close(fd);
#endif

    return CT_RET_OK;
}

CtRet ct_tcp_listen(int fd, uint32_t port, uint32_t max_conns)
{
    int ret = 0;
    struct sockaddr_in  self_addr;

    memset(&self_addr, 0, sizeof(self_addr));
    self_addr.sin_family = AF_INET;
    self_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    self_addr.sin_port = htons(port);

    ret = bind(fd, (struct sockaddr *)&self_addr, sizeof(self_addr));

#ifdef _WIN32
    if (ret == SOCKET_ERROR)
    {
        return CT_RET_E_SOCKET_BIND;
    }
#else
    if (ret < 0)
    {
        return CT_RET_E_SOCKET_FD;
    }
#endif

    ret = listen(fd, max_conns);

#ifdef _WIN32
    if (ret == SOCKET_ERROR)
    {
        return CT_RET_E_SOCKET_LISTEN;
    }
#else
    if (ret < 0)
    {
        return CT_RET_E_SOCKET_LISTEN;
    }
#endif

    return CT_RET_OK;
}

int ct_tcp_accept(int fd, char *ip, uint32_t ip_len, uint16_t *port)
{
    int new_fd = 0;
    struct sockaddr_in addr;
    socklen_t len = (socklen_t) sizeof(addr);

    memset(&addr, 0, sizeof(addr));
    new_fd = accept(fd, (struct sockaddr *)&addr, &len);
    if (new_fd > 0)
    {
        const char *remote_ip = inet_ntoa(addr.sin_addr);
        strncpy(ip, remote_ip, ip_len);
        ip[ip_len - 1] = 0; 

        *port = ntohs(addr.sin_port);
    }

    return new_fd;
}

CtRet ct_tcp_async_connect(int fd, const char *ip, uint16_t port)
{
    CtRet result = CT_RET_OK;

    RETURN_VAL_IF_FAIL(ip, CT_RET_E_ARG_NULL);

    do
    {
        int ret = 0;
        struct sockaddr_in server_addr;

        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = inet_addr(ip);
        server_addr.sin_port = htons(port);

        if (fd == 0)
        {
            result = CT_RET_E_SOCKET_FD;
            break;
        }

        /* connecting ... */
        ret = connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (ret == 0)
        {
            result = CT_RET_OK;
            break;
        }
        else if (ret < 0)
        {
#ifdef _WIN32
            DWORD e = GetLastError();
            if (e == WSAEWOULDBLOCK)
            {
                /* 10035 = A non-blocking socket operation could not be completed immediately */
                result = CT_RET_PENDING;
                break;
            }
            else if (e == WSAEISCONN)
            {
                LOG_W(TAG, "connection is established!");
                result = CT_RET_E_SOCKET_CONNECTED;
                break;
            }
            else
            {
                result = CT_RET_E_SOCKET_DISCONNECTED;
                break;
            }
            break;
#else
            /* UNP v1, p385, connection is build but not completed. */
            if (errno != EINPROGRESS)
            {
                LOG_W(TAG, "connect failed: %s", strerror(errno));
                result =  CT_RET_E_SOCKET_DISCONNECTED;
                break;
            }
#endif
        }
    }
    while (false);

    return result;
}

CtRet ct_tcp_waiting_for_connected(int fd, uint32_t timeout)
{
    CtRet result = CT_RET_OK;

    do
    {
        int max_fd = 0;
        fd_set read_set;
        fd_set write_set;
        struct timeval tv;

        FD_ZERO(&read_set);
        FD_SET(fd, &read_set);

        FD_ZERO(&write_set);
        FD_SET(fd, &write_set);

        /*
        * NOTE
        *   max_fd is not used on Windows,
        *   but on linux/unix it MUST Greater than socket_no.
        */
        max_fd = fd + 1;

        tv.tv_sec = timeout / 1000;
        tv.tv_usec = timeout % 1000;

        int ret = select(max_fd, &read_set, &write_set, NULL, &tv);
        if (ret < 0)
        {
            LOG_D(TAG, "select failed: %s", strerror(errno));
            result = CT_RET_E_SELECT;
            break;
        }

        if (ret == 0)
        {
            result = CT_RET_E_TIMEOUT;
            break;
        }

        // [1] socket is writeable if connection estabelished.
        // [2] socket is readable and writeable if connection occur error.
        if (FD_ISSET(fd, &read_set) || FD_ISSET(fd, &write_set))
        {
            if (ct_socket_has_error(fd))
            {
                result = CT_RET_E_SOCKET_DISCONNECTED;
                break;
            }
        }

        result = CT_RET_OK;
    }
    while (false);

    return result;
}

CtRet ct_tcp_waiting_for_read(int fd, uint32_t timeout)
{
    return ct_socket_waiting_for_read(fd, timeout);
}

CtRet ct_tcp_waiting_for_write(int fd, uint32_t timeout)
{
    return ct_socket_waiting_for_write(fd, timeout);
}

int ct_tcp_read(int fd, char *buf, uint32_t len)
{
    int received = 0;

    while (len > 0)
    {
        int n = recv(fd, buf + received, len, 0);

    #if (SOCKET_DEBUG)
        LOG_D(TAG, "recv: %d", n);
    #endif

        if (n == 0)
        {
            break;
        }

#if 0
        // block
#ifdef _WIN32
        if (n == SOCKET_ERROR)
        {
            DWORD e = GetLastError();
            LOG_D(TAG, "GetLastError: %d", e);

            if (e == WSAEWOULDBLOCK)
            {
                ct_usleep(100);
                continue;
            }
            else
            {
                break;
            }
        }
#else
        if (n == -1)
        {
            if (errno == EAGAIN)
            {
                /* more data */
                LOG_D(TAG, "recv: EAGAIN");
                ct_usleep(100);
                continue;
            }
            else
            {
                break;
            }
        }
#endif
#endif

#ifdef _WIN32
        if (n == SOCKET_ERROR)
        {
            break;
        }
#else
        if (n == -1)
        {
            break;
        }
#endif

        received += n;
        len -= n;
    }

    return received;
}

int ct_tcp_write(int fd, const char *buf, uint32_t len)
{
    int sent = 0;

    while (len > 0)
    {
        int n = send(fd, buf + sent, len, 0);
        if (n == 0)
        {
            break;
        }

    #if (SOCKET_DEBUG)
        LOG_D(TAG, "send: %d", n);
    #endif

#ifdef _WIN32
        if (n == SOCKET_ERROR)
        {
            DWORD e = GetLastError();
            LOG_D(TAG, "GetLastError: %d", e);

            if (e == WSAEWOULDBLOCK)
            {
                ct_usleep(100);
                continue;
            }
            else
            {
                break;
            }
        }
#else
        if (n == -1)
        {
            if (errno == EAGAIN)
            {
                /* more data */
                continue;
            }
            else
            {
                break;
            }
        }
#endif

        sent += n;
        len -= n;
    }

    return sent;
}

CtRet ct_udp_unicast_open(int *fd, uint16_t port, bool block)
{
    int ret = 0;
    int net_timeout = 1000;
    struct sockaddr_in receiver_addr;
    int addr_len = sizeof(receiver_addr);

    *fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (*fd < 0)
    {
        LOG_E(TAG, "socket failed");
        return CT_RET_E_SOCKET_FD;
    }

#ifdef _WIN32
    ret = setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, (char *)&net_timeout, sizeof(int));
    if (ret == SOCKET_ERROR)
    {
        DWORD error = GetLastError();
        LOG_E(TAG, "setsockopt: %d\n", error);
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }
#else
    ret = setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, (char *)&net_timeout, sizeof(int));
    if (ret < 0)
    {
        LOG_E(TAG, "setsockopt: %s\n", strerror(errno));
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }
#endif

    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    receiver_addr.sin_port = htons(port);

    ret = bind(*fd, (struct sockaddr *)&receiver_addr, sizeof(receiver_addr));

#ifdef _WIN32
    if (ret == SOCKET_ERROR)
    {
        DWORD error = GetLastError();
        LOG_E(TAG, "setsockopt: %d\n", error);
        return CT_RET_E_SOCKET_BIND;
    }
#else
    if (ret < 0)
    {
        LOG_E(TAG, "setsockopt: %s\n", strerror(errno));
        return CT_RET_E_SOCKET_BIND;
    }
#endif

    if (!block)
    {
        return ct_socket_set_nonblock(*fd);
    }

    return CT_RET_OK;
}

CtRet ct_udp_multicast_open(int *fd, const char *group, uint16_t port, bool block)
{
    CtRet ret = CT_RET_OK;

    do
    {
        *fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (*fd < 0)
        {
            LOG_E(TAG, "socket failed");
            ret = CT_RET_E_SOCKET_FD;

#ifdef _WIN32
            DWORD e = GetLastError();
            LOG_D(TAG, "socket, e = %d", e);
#else
            LOG_D(TAG, "socket: %s", strerror(errno));
#endif
            break;
        }

        ret = udp_join_multicast_group(*fd, group, port);
        if (RET_FAILED(ret))
        {
            break;
        }

        if (!block)
        {
            ret = ct_socket_set_nonblock(*fd);
            if (RET_FAILED(ret))
            {
                break;
            }
        }
    }
    while (0);

    return ret;
}

CtRet ct_udp_unicast_close(int fd)
{
#ifdef _WIN32
    closesocket(fd);
#else
    shutdown(fd, SHUT_RDWR);
    close(fd);
#endif

    return CT_RET_OK;
}

CtRet ct_udp_multicast_close(int fd)
{
    udp_leave_multicast_group(fd);

#ifdef _WIN32
    closesocket(fd);
#else
    shutdown(fd, SHUT_RDWR);
    close(fd);
#endif

    return CT_RET_OK;
}

CtRet ct_udp_waiting_for_read(int fd, uint32_t timeout)
{
    return ct_socket_waiting_for_read(fd, timeout);
}

CtRet ct_udp_waiting_for_write(int fd, uint32_t timeout)
{
    return ct_socket_waiting_for_write(fd, timeout);
}

int ct_udp_read(int fd, char *buf, uint32_t buf_len, char *ip, uint32_t ip_len, uint16_t *port)
{
    struct sockaddr_in sender_addr;
    socklen_t addr_len = (socklen_t) sizeof(sender_addr);
    int received = 0;
    const char *from_ip = NULL;

    RETURN_VAL_IF_FAIL(buf, 0);
    RETURN_VAL_IF_FAIL(ip, 0);

    memset(buf, 0, buf_len);

    received = recvfrom(fd, buf, buf_len, 0, (struct sockaddr *)&sender_addr, &addr_len);
    if (received < 0)
    {
#ifdef _WIN32
        DWORD e = GetLastError();
        LOG_D(TAG, "recvfrom < 0, e = %d", e);
#else
        LOG_D(TAG, "recvfrom: %s", strerror(errno));
#endif

        return received;
    }
    else if (received == 0)
    {
        return 0;
    }

    from_ip = inet_ntoa(sender_addr.sin_addr);
    strncpy(ip, from_ip, ip_len);

    *port = ntohs(sender_addr.sin_port);

    return received;
}

int ct_udp_write(int fd, const char *ip, uint16_t port, const char *buf, uint32_t len)
{
    struct sockaddr_in receiver_addr;
    int addr_len = sizeof(receiver_addr);

    RETURN_VAL_IF_FAIL(ip, 0);
    RETURN_VAL_IF_FAIL(buf, 0);

    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = inet_addr(ip);
    receiver_addr.sin_port = htons(port);

    return sendto(fd, buf, len, 0, (struct sockaddr *)&receiver_addr, addr_len);
}

#ifdef _WIN32
static CtRet udp_join_multicast_group(int fd, const char *group, uint16_t port)
{
    struct sockaddr_in addr;
    int rc = NO_ERROR;
    char loop = 1;

    rc = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &loop, sizeof(loop));
    if (rc == SOCKET_ERROR)
    {
        DWORD e = GetLastError();
        LOG_D(TAG, "setsockopt: %d", e);
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    rc = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc == SOCKET_ERROR)
    {
        DWORD e = GetLastError();
        LOG_D(TAG, "bind: %d", e);
        return CT_RET_E_SOCKET_BIND;
    }

    loop = 1;
    rc = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    if (rc == SOCKET_ERROR)
    {
        DWORD e = GetLastError();
        LOG_D(TAG, "setsockopt: %d", e);
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }

    return udp_join_multicast_group_with_all_ip(fd, group);
}
#else // Linux | unix
static CtRet udp_join_multicast_group(int fd, const char *group, uint16_t port)
{
    struct sockaddr_in addr;
    int ret = 0;
    int loop = 0;

    loop = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &loop, sizeof(loop));
    if (ret < 0)
    {
        LOG_D(TAG, "setsockopt: %s", strerror(errno));
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    ret = bind(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        LOG_D(TAG, "bind: %s", strerror(errno));
        return CT_RET_E_SOCKET_BIND;
    }

    loop = 1;
    ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    if (ret < 0)
    {
        LOG_D(TAG, "setsockopt: %s", strerror(errno));
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }

    return udp_join_multicast_group_with_all_ip(fd, group);
}
#endif // _WIN32

#ifdef _WIN32
static CtRet udp_join_multicast_group_with_all_ip(int fd, const char *group)
{
    IP_ADAPTER_INFO * pNextAdapter = NULL;
    IP_ADAPTER_INFO * ipAdaptersInfo = NULL;
    ULONG infolen = sizeof(IP_ADAPTER_INFO);

    ipAdaptersInfo = (IP_ADAPTER_INFO *)ct_malloc(infolen);

    if (GetAdaptersInfo(ipAdaptersInfo, &infolen) == ERROR_BUFFER_OVERFLOW)
    {
        ct_free(ipAdaptersInfo);
        ipAdaptersInfo = (IP_ADAPTER_INFO *)ct_malloc(infolen);
    }

    if (GetAdaptersInfo(ipAdaptersInfo, &infolen))
    {
        ct_free(ipAdaptersInfo);
        return CT_RET_E_INTERNAL;
    }

    for (pNextAdapter = ipAdaptersInfo; pNextAdapter; pNextAdapter = pNextAdapter->Next)
    {
        IP_ADDR_STRING *pNextIp = NULL;
        for (pNextIp = &(pNextAdapter->IpAddressList); pNextIp; pNextIp = pNextIp->Next)
        {
            unsigned long ip = inet_addr(pNextIp->IpAddress.String);
            if (ip == 0)
            {
                break;
            }

            LOG_D(TAG, "join MultiGroup ip: %s", pNextIp->IpAddress.String);

            udp_join_multicast_group_with_ip(fd, group, ip);
        }
    }

    ct_free(ipAdaptersInfo);

    return CT_RET_OK;
}
#else // Linux | Unix
static CtRet udp_join_multicast_group_with_all_ip(int fd, const char *group)
{
    int socket_fd = 0;
    int interface_count = 0;
    int i = 0;
    int ret = 0;

    struct ifreq buf[8];
    struct ifconf ifc;

    socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        LOG_E(TAG, "socket: %s", strerror(errno));
        return CT_RET_E_SOCKET_FD;
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_buf = (caddr_t)buf;

    ret = ioctl(socket_fd, SIOCGIFCONF, (char *)&ifc);
    if (ret != 0)
    {
        LOG_E(TAG, "ioctl: %s", strerror(errno));
        return CT_RET_E_INTERNAL;
    }

    interface_count = ifc.ifc_len / sizeof(struct ifreq);

    for (i = 0; i < interface_count; ++i)
    {
        struct sockaddr_in *addr = NULL;
        unsigned long ip = 0;

        ret = ioctl(socket_fd, SIOCGIFFLAGS, (char *)&buf[i]);
        if (ret != 0)
        {
            continue;
        }

        if (buf[i].ifr_flags & IFF_LOOPBACK)
        {
            continue;
        }

        ret = ioctl(socket_fd, SIOCGIFADDR, (char *)&buf[i]);
        if (ret != 0)
        {
            continue;
        }

        addr = (struct sockaddr_in *)&buf[i].ifr_addr;
        ip = addr->sin_addr.s_addr;

        udp_join_multicast_group_with_ip(fd, group, ip);
    }

    close(socket_fd);

    return CT_RET_OK;
}
#endif // _WIN32

#ifdef _WIN32
static CtRet udp_join_multicast_group_with_ip(int fd, const char *group, unsigned long ip)
{
    int ret = 0;
    struct ip_mreq ipMreqV4;

    // Setup the v4 option values and ip_mreq structure
    memset(&ipMreqV4, 0, sizeof(struct ip_mreq));
    ipMreqV4.imr_multiaddr.s_addr = inet_addr(group);
    //ipMreqV4.imr_interface.s_addr = htonl(ip);
    ipMreqV4.imr_interface.s_addr = ip;

    // Join the group
    ret = setsockopt(fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&ipMreqV4, sizeof(ipMreqV4));
    if (ret == SOCKET_ERROR)
    {
        // !!! bug， 10065 = A socket operation was attempted to an unreachable host.
        DWORD e = GetLastError();
        LOG_E(TAG, "setsockopt: %d", e);
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }

    return CT_RET_OK;
}
#else // Linux | Unix
static CtRet udp_join_multicast_group_with_ip(int fd, const char *group, unsigned long ip)
{
    int ret = setsockopt(fd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&ip, sizeof(ip));
    if (ret < 0)
    {
        LOG_E(TAG, "setsockopt: %s", strerror(errno));
        return CT_RET_E_SOCKET_SETSOCKOPT;
    }

    return CT_RET_OK;
}
#endif // _WIN32

static CtRet udp_leave_multicast_group(int fd)
{
    LOG_W(TAG, "udp_leave_multicast_group: not implements");

#if 0
    int ret = 0;
    struct ip_mreq mreq;
    ret = setsockopt(fd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq));
#endif

    return CT_RET_OK;
}
