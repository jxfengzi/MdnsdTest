/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-7-9
 *
 * @file   ct_ret.h
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 *
 * @description
 *
 *  Values are 64 bit values layed out as follows:
 *  
 *   7 65 43210  76543210  76543210  76543210 | 76543210  76543210  76543210  76543210
 *  +-+--+------------------------------------+----------------------------------------+
 *  |S|EL|             Reserved               |                  Code                  |
 *  +-+--+------------------------------------+----------------------------------------+
 *
 *  S - Severity - indicates success/fail
 *      0 - Success
 *      1 - Fail
 *
 *  EL - Error level
 *      00 - General Error
 *      01 - Serious Error
 *      10 - Fatal Error
 *
 * 1. return value
 *      return CT_RET_OK;
 *      return CT_RET_E_SOCKET_FD;
 *      return CT_RET_E_SOCKET_WRITE;
 *
 * 2. get return status
 *      CtRet ret = ct_tcp_async_connect(...);
 *      if (RET_SUCCEEDED(ret))
 *      {
 *          if (ret == CT_RET_SOCKET_CONNECTED)
 *          {
 *              ...
 *          }
 *
 *          else if (ret == CT_RET_SOCKET_CONNECTING)
 *          {
 *              ...
 *          }
 *      }
 *      else
 *      {
 *        ...
 *      }
 *
 *      if (RET_FAILED(ret))
 *      {
 *          if (ret == CT_RET_SOCKET_CONNECT_FAILED)
 *          {
 *              ...
 *          }
 *      }
 *
 * 3. get return code
 *      uint32_t code = CT_RET_GET_CODE(ret);
 *      LOG_D(TAG, "%s", ct_ret_to_str(ret));
 */

#ifndef __CT_RET_H__
#define __CT_RET_H__

#include "ct_typedef.h"

CT_BEGIN_DECLS


typedef int64_t CtRet;

const char * ct_ret_to_str(CtRet ret);

#define RET_SUCCEEDED(r)                    (((CtRet)(r)) >= 0)
#define RET_FAILED(r)                       (((CtRet)(r)) < 0)

/* Severity Value */
#define SV_OK                               (0)
#define SV_ERR                              (1)

/* Error Level */
#define EL_NULL                             (0)
#define EL_GENERAL                          (1)
#define EL_SERIOUS                          (2)
#define EL_FATAL                            (3)

/* Code */
#define CODE_UNDEFINED                      -1
#define CODE_OK                             0
#define CODE_PENDING                        1
#define CODE_INTERNAL                       2
#define CODE_OUT_OF_MEMORY                  3
#define CODE_ARG_NULL                       4
#define CODE_ARG_INVALID                    5
#define CODE_NOT_IMPLEMENTED                6
#define CODE_NEW                            7
#define CODE_CONSTRUCT                      8
#define CODE_TIMEOUT                        9
#define CODE_NOT_FOUND                      10
#define CODE_STARTED                        11
#define CODE_STOPPED                        12
#define CODE_POSITION_INVALID               13
#define CODE_SELECT                         14
#define CODE_SOCKET_FD                      15
#define CODE_SOCKET_BIND                    16
#define CODE_SOCKETSETSOCKOPT               17
#define CODE_SOCKET_LISTEN                  18
#define CODE_SOCKET_READ                    19
#define CODE_SOCKET_WRITE                   20
#define CODE_SOCKET_CONNECTING              21
#define CODE_SOCKET_CONNECTED               22
#define CODE_SOCKET_DISCONNECTED            23
#define CODE_SOCKET_ASCEPT                  24
#define CODE_UUID_INVALID                   25
#define CODE_HTTP_MSG_INVALID               26
#define CODE_HTTP_TYPE_INVALID              27
#define CODE_HTTP_STATUS                    28
#define CODE_ITEM_EXIST                     29
#define CODE_URL_INVALID                    30
#define CODE_URL_GET_FAILED                 31
#define CODE_XML_INVALID                    32
#define CODE_UPNP_DEVICE_NOT_FOUND          33
#define CODE_UPNP_SERVICE_NOT_FOUND         34
#define CODE_UPNP_ACTION_NOT_FOUND          35
#define CODE_UPNP_ARGUMENT_NOT_FOUND        36
#define CODE_UPNP_INVOKE_FAILED             37
#define CODE_UPNP_SUBSCRIBE_FAILED          38
#define CODE_UPNP_UNSUBSCRIBE_FAILED        39

/* Return the error level */
#define ERR_LEVEL(r)                        (((uint64_t)(r) & 0x7FFFFFFFFFFFFFFF) >> 61)

/* Return the code */
#define CT_RET_CODE(r)                      ((r) & 0xFFFFFFFF)

/* Create an CtRet value from component pieces */
#define MAKE_RET(s, el, code)               (CtRet)(((uint64_t)(s) << 63) | ((uint64_t)(el) << 61) | ((uint64_t)(code)))

/* return value */
#define CT_RET_OK                           MAKE_RET(SV_OK, EL_NULL, CODE_OK)
#define CT_RET_PENDING                      MAKE_RET(SV_OK, EL_NULL, CODE_PENDING)

#define CT_RET_E_INTERNAL                   MAKE_RET(SV_ERR, EL_GENERAL, CODE_INTERNAL)
#define CT_RET_E_OUT_OF_MEMORY              MAKE_RET(SV_ERR, EL_GENERAL, CODE_OUT_OF_MEMORY)
#define CT_RET_E_ARG_NULL                   MAKE_RET(SV_ERR, EL_GENERAL, CODE_ARG_NULL)
#define CT_RET_E_ARG_INVALID                MAKE_RET(SV_ERR, EL_GENERAL, CODE_ARG_INVALID)
#define CT_RET_E_NOT_IMPLEMENTED            MAKE_RET(SV_ERR, EL_GENERAL, CODE_NOT_IMPLEMENTED)
#define CT_RET_E_NEW                        MAKE_RET(SV_ERR, EL_GENERAL, CODE_NEW)
#define CT_RET_E_CONSTRUCT                  MAKE_RET(SV_ERR, EL_GENERAL, CODE_CONSTRUCT)
#define CT_RET_E_TIMEOUT                    MAKE_RET(SV_ERR, EL_GENERAL, CODE_TIMEOUT)
#define CT_RET_E_NOT_FOUND                  MAKE_RET(SV_ERR, EL_GENERAL, CODE_NOT_FOUND)
#define CT_RET_E_STARTED                    MAKE_RET(SV_ERR, EL_GENERAL, CODE_STARTED)
#define CT_RET_E_STOPPED                    MAKE_RET(SV_ERR, EL_GENERAL, CODE_STOPPED)
#define CT_RET_E_POSITION_INVALID           MAKE_RET(SV_ERR, EL_GENERAL, CODE_POSITION_INVALID)
#define CT_RET_E_SELECT                     MAKE_RET(SV_ERR, EL_GENERAL, CODE_SELECT)
#define CT_RET_E_SOCKET_FD                  MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_FD)
#define CT_RET_E_SOCKET_BIND                MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_BIND)
#define CT_RET_E_SOCKET_SETSOCKOPT          MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKETSETSOCKOPT)
#define CT_RET_E_SOCKET_LISTEN              MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_LISTEN)
#define CT_RET_E_SOCKET_READ                MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_READ)
#define CT_RET_E_SOCKET_WRITE               MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_WRITE)
#define CT_RET_E_SOCKET_CONNECTING          MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_CONNECTING)
#define CT_RET_E_SOCKET_CONNECTED           MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_CONNECTED)
#define CT_RET_E_SOCKET_DISCONNECTED        MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_DISCONNECTED)
#define CT_RET_E_SOCKET_ASCEPT              MAKE_RET(SV_ERR, EL_GENERAL, CODE_SOCKET_ASCEPT)
#define CT_RET_E_UUID_INVALID               MAKE_RET(SV_ERR, EL_GENERAL, CODE_UUID_INVALID)
#define CT_RET_E_HTTP_MSG_INVALID           MAKE_RET(SV_ERR, EL_GENERAL, CODE_HTTP_MSG_INVALID)
#define CT_RET_E_HTTP_TYPE_INVALID          MAKE_RET(SV_ERR, EL_GENERAL, CODE_HTTP_TYPE_INVALID)
#define CT_RET_E_HTTP_STATUS                MAKE_RET(SV_ERR, EL_GENERAL, CODE_HTTP_STATUS)
#define CT_RET_E_ITEM_EXIST                 MAKE_RET(SV_ERR, EL_GENERAL, CODE_ITEM_EXIST)
#define CT_RET_E_URL_INVALID                MAKE_RET(SV_ERR, EL_GENERAL, CODE_URL_INVALID)
#define CT_RET_E_URL_GET_FAILED             MAKE_RET(SV_ERR, EL_GENERAL, CODE_URL_GET_FAILED)
#define CT_RET_E_XML_INVALID                MAKE_RET(SV_ERR, EL_GENERAL, CODE_XML_INVALID)
#define CT_RET_E_UPNP_DEVICE_NOT_FOUND      MAKE_RET(SV_ERR, EL_GENERAL, CODE_UPNP_DEVICE_NOT_FOUND)
#define CT_RET_E_UPNP_SERVICE_NOT_FOUND     MAKE_RET(SV_ERR, EL_GENERAL, CODE_UPNP_SERVICE_NOT_FOUND)
#define CT_RET_E_UPNP_ACTION_NOT_FOUND      MAKE_RET(SV_ERR, EL_GENERAL, CODE_UPNP_ACTION_NOT_FOUND)
#define CT_RET_E_UPNP_ARGUMENT_NOT_FOUND    MAKE_RET(SV_ERR, EL_GENERAL, CODE_UPNP_ARGUMENT_NOT_FOUND)
#define CT_RET_E_UPNP_INVOKE_FAILED         MAKE_RET(SV_ERR, EL_GENERAL, CODE_UPNP_INVOKE_FAILED)
#define CT_RET_E_UPNP_SUBSCRIBE_FAILED      MAKE_RET(SV_ERR, EL_GENERAL, CODE_UPNP_SUBSCRIBE_FAILED)
#define CT_RET_E_UPNP_UNSUBSCRIBE_FAILED    MAKE_RET(SV_ERR, EL_GENERAL, CODE_UPNP_UNSUBSCRIBE_FAILED)

CT_END_DECLS

#endif /* __CT_RET_H__ */
