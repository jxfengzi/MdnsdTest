/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-11-19
 *
 * @file   CtBonjour.c
 *
 * @remark
 *
 */

#include "CtBonjour.h"

static 
CtRet CtThread_Construct(CtThread *thiz);

static
CtRet CtThread_Dispose(CtThread *thiz);

static
bool event_poll(DNSServiceRef ref, double timeoutInSeconds, DNSServiceErrorType *err);

static
void event_loop(void *param);

static 
void browse_reply (DNSServiceRef sdref, 
                   const DNSServiceFlags flags, 
                   uint32_t ifIndex, 
                   DNSServiceErrorType errorCode,
                   const char *replyName, 
                   const char *replyType, 
                   const char *replyDomain, 
                   void *ctx);

static
void resolve_reply (DNSServiceRef sdRef,
                    DNSServiceFlags flags, 
                    uint32_t interfaceIndex, 
                    DNSServiceErrorType errorCode,
                    const char *fullresolvename, 
                    const char *hosttarget, 
                    uint16_t opaqueport,            ///< port 
                    uint16_t txtLen,
                    const unsigned char *txtRecord, 
                    void *ctx);

static 
void address_reply (DNSServiceRef sdRef,
                    DNSServiceFlags flags, 
                    uint32_t interfaceIndex, 
                    DNSServiceErrorType errorCode, 
                    const char *hostname, 
                    const struct sockaddr *address, 
                    uint32_t ttl, 
                    void *ctx);

struct _CtBonjour
{
    CtThread *thread;
    DNSServiceRef dnsSvcRef;
    bool serviceDiscoveryStarted;
};

CtBonjour * CtBonjour_New(void)
{
    CtBonjour *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtBonjour *)ct_malloc(sizeof(CtBonjour));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtBonjour_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtBonjour_Delete(thiz);
            thiz = NULL;
            break;
        }
    }
    while (0);

    return thiz;
}

static
CtRet CtBonjour_Construct(CtBonjour *thiz)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    do
    {
        memset(thiz, 0, sizeof(CtBonjour));
        thiz->serviceDiscoveryStarted = false;

        thiz->thread = CtThread_New();
        if (thiz->thread == null)
        {
            LOG_W(TAG, "CtThread_New() failed");
            ret = CT_RET_NEW;
            break;
        }

 
    }
    while (0);

    return ret;
}

static
CtRet CtBonjour_Dispose(CtBonjour *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    CtBonjour_Stop(thiz);
    CtThread_Delete(thiz->thread);

    return CT_RET_OK;
}

void CtBonjour_Delete(CtBonjour *thiz)
{
    CtBonjour_Dispose(thiz);
    ct_free(thiz);
}

CtRet CtBonjour_DiscoverService(CtBonjour *thiz, const char *type, const char *domain, void *ctx)
{
    CtRet ret = CT_RET_OK;
    DNSServiceErrorType err;
    uint32_t interfaceIndex = kDNSServiceInterfaceIndexAny;
    DNSServiceFlags flag = kDNSServiceFlagsShareConnection;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);
    RETURN_VAL_IF_FAIL(type, CT_RET_E_ARG_NULL);
    RETURN_VAL_IF_FAIL(domain, CT_RET_E_ARG_NULL);

    do{
        if (thiz->serviceDiscoveryStarted)
        {
            LOG_W("serviceDiscoveryStarted");
            ret = CT_RET_E_STARTED;
            break;
        }

        err = DNSServiceCreateConnection (&(thiz->dnsSvcRef));
        if (err != kDNSServiceErr_NoError)
        {
            LOG_W("DNSServiceCreateConnection failed: %d", err);
            ret = CT_RET_E_FAILED;
            break;
        }

        err = DNSServiceBrowse (&(thiz->dnsSvcRef),
                flag, 
                interfaceIndex, 
                serviceType, 
                domainName, 
                browse_reply, 
                ctx);
        if (err != kDNSServiceErr_NoError)
        {
            LOG_W("DNSServiceBrowse failed: %d", err);
            ret = CT_RET_E_FAILED;

            DNSServiceRefDeallocate(thiz->dnsSvcRef);
            thiz->dnsSvcRef = NULL;
            break;
        }

        ret = CtThread_Initialize(&thiz->thread, event_loop, thiz, "mdns");
        if (RET_FAILED(ret))
        {
            LOG_W(TAG, "CtThread_Initialize failed");
            ret = CT_RET_FAILED;
            break;
        }

        CtThread_Start(&thiz->thread);
        thiz->serviceDiscoveryStarted = true;

        ret = CT_RET_OK;
    } while (0);

    return ret;
}

CtRet CtBonjour_StopServiceDiscovery(CtBonjour *thiz)
{
    CtRet ret = CT_RET_OK;

    do
    {
        if (! thiz->serviceDiscoveryStarted)
        {
            LOG_W("serviceDiscovery not Started");
            ret = CT_RET_E_STOPPED;
            break;
        }

        if (thiz->dnsSvcRef != NULL) 
        {
            DNSServiceRefDeallocate(thiz->dnsSvcRef);
            thiz->dnsSvcRef = NULL;
        }

        thiz->threadShouldExit = true;
        CtThread_Join(&thiz->thread);
    }
    while (0);

    return ret;
}

static
bool event_poll(DNSServiceRef ref, double timeoutInSeconds, DNSServiceErrorType *err)
{
    *err = kDNSServiceErr_NoError;

    fd_set readfds;
    FD_ZERO(&readfds);

    int fd = DNSServiceRefSockFD(*ref);
    int nfds = fd + 1;
    FD_SET(fd, &readfds);

    struct timeval tv;
    tv.tv_sec = long(floor(timeoutInSeconds));
    tv.tv_usec = long(1000000*(timeoutInSeconds - tv.tv_sec));

    int result = select(nfds, &readfds, NULL, NULL, &tv);
    if (result>0 && FD_ISSET(fd, &readfds))
    {
        err = DNSServiceProcessResult(*dnsServiceRef);
        return true;
    }

    return false;
}

static
void event_loop(void *param)
{
    CtBonjour *thiz = (CtBonjour *)param;

    while (! thiz->threadShouldExit)
    {
        DNSServiceErrorType err = kDNSServiceErr_NoError;
        if (event_poll(thiz->dnsSvcRef, thiz->timeoutInSeconds, &err))
        {
            if (err > 0)
            {
                thiz->threadShouldExit = true;
                break;
            }
        }
    }
}

static 
void browse_reply (DNSServiceRef sdref, 
                   const DNSServiceFlags flags, 
                   uint32_t ifIndex, 
                   DNSServiceErrorType errorCode,
                   const char *replyName, 
                   const char *replyType, 
                   const char *replyDomain, 
                   void *ctx)
{
    Bonjour *bonjour = (Bonjour *)ctx;

    printf("browse_reply\r\n");

    if (flags & kDNSServiceFlagsAdd) 
    {
        printf("ServiceFound: %s %s %s\n", replyName, replyType, replyDomain);

        DNSServiceResolve (&(bonjour->dnsSvcRef),
                           kDNSServiceFlagsShareConnection,
                           ifIndex,
                           replyName,
                           replyType,
                           replyDomain,
                           resolve_reply,
                           ctx);
    }
    else 
    {
        printf("ServiceLost: %s %s %s\n", replyName, replyType, replyDomain);
    }
}

static
void resolve_reply (DNSServiceRef sdRef,
                    DNSServiceFlags flags, 
                    uint32_t interfaceIndex, 
                    DNSServiceErrorType errorCode,
                    const char *fullresolvename, 
                    const char *hosttarget, 
                    uint16_t opaqueport,            ///< port 
                    uint16_t txtLen,
                    const unsigned char *txtRecord, 
                    void *ctx)
{
    Bonjour *bonjour = (Bonjour *)ctx;

    printf("resolve_reply\r\n");
    printf("fullresolvename: %s\n", fullresolvename);
    printf("hosttarget: %s\n", hosttarget);
    printf("port: %d\n", opaqueport);
    printf("txtlen: %d\n", txtLen);
    printf("txtRecord: %s\n", txtRecord);

    DNSServiceGetAddrInfo (
            &(bonjour->dnsSvcRef),
            kDNSServiceFlagsShareConnection,
            interfaceIndex,
            kDNSServiceProtocol_IPv4,
            hosttarget,
            address_reply,
            ctx);
}

static 
void address_reply (DNSServiceRef sdRef,
                    DNSServiceFlags flags, 
                    uint32_t interfaceIndex, 
                    DNSServiceErrorType errorCode, 
                    const char *hostname, 
                    const struct sockaddr *address, 
                    uint32_t ttl, 
                    void *ctx)
{
    printf("address_reply: hostname: %s address: ? ttl: %d\r\n", hostname, ttl);
    //printf("address_reply: hostname: %s address: ? %s ttl: %d\r\n", hostname, address, ttl);
}
