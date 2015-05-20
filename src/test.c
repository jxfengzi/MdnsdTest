#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dns_sd.h> 


#define DIM(a) (sizeof(a)/sizeof(a[0]))


typedef struct _Bonjour
{
    DNSServiceRef sharedRef;
} Bonjour;

static Bonjour gBonjour;

static 
void DNSSD_API address_reply (DNSServiceRef sdRef,
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

static
void DNSSD_API resolve_reply (DNSServiceRef sdRef,
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
            &(bonjour->sharedRef),
            kDNSServiceFlagsShareConnection,
            interfaceIndex,
            kDNSServiceProtocol_IPv4,
            hosttarget,
            address_reply,
            ctx);
}

static 
void DNSSD_API browse_reply (DNSServiceRef sdref, 
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

        DNSServiceResolve (&(bonjour->sharedRef),
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
int discoveryService(const char *serviceType, const char *domainName, void *ctx)
{
    Bonjour *bonjour = (Bonjour *)ctx;
    DNSServiceErrorType err;
    uint32_t interfaceIndex = kDNSServiceInterfaceIndexAny;
    DNSServiceFlags flag = kDNSServiceFlagsShareConnection;

    printf("DNSServiceCreateConnection start\n");

    do{
        err = DNSServiceCreateConnection (&bonjour->sharedRef);
        if (err != kDNSServiceErr_NoError)
        {
            printf("DNSServiceCreateConnection failed: %d\n", err);
            break;
        }

        err = DNSServiceBrowse (&bonjour->sharedRef, 
                flag, 
                interfaceIndex, 
                serviceType, 
                domainName, 
                browse_reply, 
                ctx);
        if (err != kDNSServiceErr_NoError)
        {
            printf("DNSServiceBrowse failed: %d\n", err);
            break;
        }
    } while (0);

    printf("DNSServiceCreateConnection ok\n");

    return 0;
}

static int g_loop = 0;

static void cmd_help(void)
{
    fprintf(stdout, "\n------------ help --------------\n");
    fprintf(stdout, "h       --  show help information\n");
    fprintf(stdout, "x       --  exit\n");
}

static void cmd_exit(void)
{
    g_loop = 0;
}

typedef void (*cb_command)(void);

struct _cmd_exec {
    const char * name;
    cb_command exec;
};

struct _cmd_exec cmd_exec[] = {
        { "h", cmd_help },
        { "x", cmd_exit },
};

static
void command(const char *buf)
{
    int i   = 0;
    int len = DIM(cmd_exec);

    for (i=0; i < len; i++)
    {
        if (strcmp(cmd_exec[i].name, buf) == 0)
        {
            cmd_exec[i].exec();
            break;
        }
    }
}

static
void cmd_post_select(fd_set *p_read_set, fd_set *p_write_set, fd_set *p_error_set)
{
    char buf[2048];
    size_t nbytes;
    ssize_t bytes_read;
    int fd = 0;

    nbytes = sizeof(buf);
    memset(buf, 0, nbytes);

    if (FD_ISSET(fd, p_read_set))
    {
        //bytes_read = read(fd, (void*)buf, nbytes);
        bytes_read = read(fd, (void*)buf, nbytes);

        /* remove '\n' */
        if (strlen(buf) > 0) 
        {
            buf[strlen(buf) - 1] = 0;
        }

        command(buf);
    }
}

static 
void cmd_pre_select(int *p_max_soc, fd_set *p_read_set, fd_set *p_write_set, fd_set *p_error_set)
{
    int soc = 0;

    FD_SET(soc, p_error_set);
    FD_SET(soc, p_read_set);

    if (*p_max_soc < soc)
        *p_max_soc = soc;
}

/**
 * @brief Linuxƽ̨�����У��ȴ��û���������ָ�ִ��
 */
void cmd_loop()
{
    fd_set  read_set;
    fd_set  write_set;
    fd_set  error_set;
    int     max_soc = 0;
    int     ret = 0;

    g_loop = 1;

    while (g_loop)
    {
        ret = write(1, "> ", 2);

        FD_ZERO(&read_set);
        FD_ZERO(&write_set);
        FD_ZERO(&error_set);

        cmd_pre_select(&max_soc, &read_set, &write_set, &error_set);

        select(max_soc + 1, &read_set, &write_set, &error_set, NULL);

        cmd_post_select(&read_set, &write_set, &error_set);
    }

    fprintf(stdout, "\n");

    printf("-------------- cmd_run -----------------\n");
}

int main()
{
    discoveryService("_airplay._tcp", "local.", &gBonjour);

    cmd_loop();

    return 0;
}
