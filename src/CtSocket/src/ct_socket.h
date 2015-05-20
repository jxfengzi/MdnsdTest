/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   ct_socket.h
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 */

#ifndef __CT_SOCKET_H__
#define __CT_SOCKET_H__

#include "ct_common.h"

#ifdef _WIN32
#include <ws2tcpip.h>
#include <windows.h>
#else /* Linux */
#include <stdint.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#endif /* _WIN32 */

CT_BEGIN_DECLS


#define CT_SOCKET_DEBUG           0

CtRet ct_socket_init(void);
CtRet ct_socket_set_nonblock(int socket_fd);
CtRet ct_socket_set_block(int socket_fd);
uint32_t ct_socket_get_ipv4_bytes(int socket_fd);
CtRet ct_socket_get_ip(int socket_fd, char ip[], uint32_t ip_len);
uint16_t ct_socket_get_port(int socket_fd);
bool ct_socket_has_error(int socket_fd);
CtRet ct_socket_waiting_for_read(int fd, uint32_t timeout);
CtRet ct_socket_waiting_for_write(int fd, uint32_t timeout);

CtRet ct_tcp_open(int *fd, bool block);
CtRet ct_tcp_close(int fd);
CtRet ct_tcp_listen(int fd, uint32_t port, uint32_t max_conns);
int ct_tcp_accept(int fd, char *ip, uint32_t ip_len, uint16_t *port);
CtRet ct_tcp_async_connect(int fd, const char *ip, uint16_t port);
CtRet ct_tcp_waiting_for_connected(int fd, uint32_t timeout);
CtRet ct_tcp_waiting_for_read(int fd, uint32_t timeout);
CtRet ct_tcp_waiting_for_write(int fd, uint32_t timeout);
int ct_tcp_read(int fd, char *buf, uint32_t len);
int ct_tcp_write(int fd, const char *buf, uint32_t len);

CtRet ct_udp_unicast_open(int *fd, uint16_t port, bool block);
CtRet ct_udp_multicast_open(int *fd, const char *group, uint16_t port, bool block);
CtRet ct_udp_unicast_close(int fd);
CtRet ct_udp_multicast_close(int fd);
CtRet ct_udp_waiting_for_read(int fd, uint32_t timeout);
CtRet ct_udp_waiting_for_write(int fd, uint32_t timeout);
int ct_udp_read(int fd, char *buf, uint32_t buf_len, char *ip, uint32_t ip_len, uint16_t *port);
int ct_udp_write(int fd, const char *ip, uint16_t port, const char *buf, uint32_t len);


CT_END_DECLS

#endif /* __CT_SOCKET_H__ */
