/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-7-9
 *
 * @file   CtUuid.c
 *
 * @remark
 *
 */

#include "CtUuid.h"
#include "ct_memory.h"
#include "ct_log.h"

#ifdef _WIN32
#else
  #ifdef __MAC_OSX__
  #include <uuid/uuid.h>
  #include <ctype.h>
  #else
  #include <fcntl.h>
#endif
#endif

/*-----------------------------------------------------------------------------
 *
 * Private API declare
 *
 *-----------------------------------------------------------------------------*/
#define TAG     "CtUuid"

typedef struct _uuid_time {
    uint32_t    time_low;
    uint16_t    time_mid;
    uint16_t    time_hi_and_version;
    uint16_t    clock_seq;
    uint8_t     node[6];
} uuid_time;

static CtRet ct_uuid_generate(uuid_u * uuid);
static CtRet ct_uuid_generate_from_string(uuid_u * uuid, const char *str, uint32_t len);

static void ct_uuid_pack(uuid_time *u_time, uuid_u *uuid);
static void ct_uuid_unpack(uuid_u *uuid, uuid_time *u_time);
static const char *fmt_lower = "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x";
static const char *fmt_upper = "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X";
static int ct_uuid_to_string(uuid_u *uuid, char *out, size_t len, const char *fmt);

// for Linux or Android
#if (defined __LINUX__) || (defined __ANDROID__)

#define UUIDD_OP_GETPID             0
#define UUIDD_OP_GET_MAXOP          1
#define UUIDD_OP_TIME_UUID          2
#define UUIDD_OP_RANDOM_UUID        3
#define UUIDD_OP_BULK_TIME_UUID     4
#define UUIDD_OP_BULK_RANDOM_UUID   5
#define UUIDD_MAX_OP                UUIDD_OP_BULK_RANDOM_UUID

#define THREAD_LOCAL static

static void linux_uuid_generate(uuid_u *uuid);
static void linux_get_random_bytes(void *buf, int nbytes);
static int linux_get_random_fd(void);

static void linux_uuid_generate_random(uuid_u * uuid);
static void linux_uuid_generate_random_ex(uuid_u * uuid);

static void linux_uuid_generate_time(uuid_u *uuid);
static void linux_uuid_generate_time_ex(uuid_u *uuid, int *num);

static int linux_get_clock(uint32_t *clock_high, uint32_t *clock_low, uint16_t *ret_clock_seq, int *num);

#endif // __LINUX__ || __ANDROID__

/*-----------------------------------------------------------------------------
 *
 * Public API
 *
 *-----------------------------------------------------------------------------*/
CtUuid * CtUuid_New(void)
{
    CtUuid *thiz = NULL;

    do
    {
        CtRet ret = CT_RET_OK;

        thiz = (CtUuid *)ct_malloc(sizeof(CtUuid));
        if (thiz == NULL)
        {
            break;
        }

        ret = CtUuid_Construct(thiz);
        if (RET_FAILED(ret))
        {
            CtUuid_Delete(thiz);
            thiz = NULL;
            break;
        }

    } while (0);

    return thiz;
}

CtRet CtUuid_Construct(CtUuid *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    memset(thiz, 0, sizeof(CtUuid));

    return CT_RET_OK;
}

CtRet CtUuid_Dispose(CtUuid *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    return CT_RET_OK;
}

void CtUuid_Delete(CtUuid *thiz)
{
    RETURN_IF_FAIL(thiz);

    CtUuid_Dispose(thiz);
    ct_free(thiz);
}

CtRet CtUuid_GenerateRandom(CtUuid *thiz)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);

    return ct_uuid_generate(&thiz->uuid);
}

CtRet CtUuid_ParseFromString(CtUuid *thiz, const char *string)
{
    CtRet ret = CT_RET_OK;

    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);
    RETURN_VAL_IF_FAIL(string, CT_RET_E_ARG_NULL);

    ret = ct_uuid_generate_from_string(&thiz->uuid, string, strlen(string));
    if (RET_FAILED(ret))
    {
        LOG_D(TAG, "[%s] %s", string, ct_ret_to_str(ret));
    }

    return ret;
}

CtRet CtUuid_ParseFromBuffer(CtUuid *thiz, const char *buffer, uint32_t len)
{
    RETURN_VAL_IF_FAIL(thiz, CT_RET_E_ARG_NULL);
    RETURN_VAL_IF_FAIL(buffer, CT_RET_E_ARG_NULL);

    return ct_uuid_generate_from_string(&thiz->uuid, buffer, len);
}

const char * CtUuid_ToString(CtUuid * thiz, bool upper)
{
    RETURN_VAL_IF_FAIL(thiz, NULL);

    memset(thiz->string, 0, CT_UUID_LEN);
    ct_uuid_to_string(&thiz->uuid, thiz->string, CT_UUID_LEN, upper ? fmt_upper : fmt_lower);

    return thiz->string;
}

void CtUuid_Clear(CtUuid *thiz)
{
    RETURN_IF_FAIL(thiz);

    memset(thiz->uuid.byte, 0, 16);
}

bool CtUuid_Equal(CtUuid *thiz, CtUuid *other)
{
    uuid_time u1;
    uuid_time u2;

    RETURN_VAL_IF_FAIL(thiz, false);
    RETURN_VAL_IF_FAIL(other, false);

    ct_uuid_unpack(&thiz->uuid, &u1);
    ct_uuid_unpack(&other->uuid, &u2);

    if (u1.time_low == u2.time_low
            && u1.time_mid == u2.time_mid
            && u1.time_hi_and_version == u2.time_hi_and_version
            && u1.clock_seq == u2.clock_seq)
    {
        int ret = memcmp(u1.node, u2.node, 6);
        if (ret == 0)
        {
            return true;
        }
    }

    return false;
}

void CtUuid_Copy(CtUuid *dst, CtUuid *src)
{
    int i = 0;
    uint8_t * d = dst->uuid.byte;
    uint8_t * s = src->uuid.byte;

    RETURN_IF_FAIL(dst);
    RETURN_IF_FAIL(src);

    for (i = 0; i < 16; i++)
    {
        *d++ = *s++;
    }
}

bool CtUuid_IsNull(CtUuid *thiz)
{
    int i = 0;

    RETURN_VAL_IF_FAIL(thiz, false);

    for (i = 0; i < 16; ++i)
    {
        if (thiz->uuid.byte[i] != 0)
        {
            return false;
        }
    }

    return true;
}

/*-----------------------------------------------------------------------------
 *
 * Private API
 *
 *-----------------------------------------------------------------------------*/
static CtRet ct_uuid_generate(uuid_u * uuid)
{
#ifdef _WIN32
    RPC_STATUS status = UuidCreate(&uuid->u);
    if (status == RPC_S_OK)
    {
        return CT_RET_OK;
    }

    return CT_RET_E_INTERNAL;
#endif // _WIN32

#if (defined __LINUX__) || (defined __ANDROID__)
    linux_uuid_generate(uuid);
    return CT_RET_OK;
#endif // __LINUX__ || __ANDROID__

#ifdef __MAC_OSX__
    uuid_generate(uuid->u);
    return CT_RET_OK;
#endif /* __MAC_OSX__ */
}

static CtRet ct_uuid_generate_from_string(uuid_u * uuid, const char *str, uint32_t len)
{
    int i = 0;
    char buf[3];
    const char * cp = NULL;
    uuid_time u_time;

    if (len != 36)
    {
        return CT_RET_E_UUID_INVALID;
    }

    for (i=0, cp = str; i <= 36; i++,cp++) 
    {
        if ((i == 8) || (i == 13) || (i == 18) || (i == 23)) 
        {
            if (*cp == '-')
            {
                continue;
            }
            else
            {
                return CT_RET_E_UUID_INVALID;
            }
        }

        if (i == 36)
        {
            if (*cp == 0)
            {
                continue;
            }
        }

        if (!isxdigit((int)(*cp)))
        {
            return CT_RET_E_UUID_INVALID;
        }
    }

    u_time.time_low = strtoul(str, NULL, 16);
    u_time.time_mid = (uint16_t) strtoul(str + 9, NULL, 16);
    u_time.time_hi_and_version = (uint16_t) strtoul(str + 14, NULL, 16);
    u_time.clock_seq = (uint16_t) strtoul(str + 19, NULL, 16);
    cp = str + 24;
    buf[2] = 0;

    for (i=0; i < 6; i++) 
    {
        buf[0] = *cp++;
        buf[1] = *cp++;
        u_time.node[i] = (uint8_t) strtoul(buf, NULL, 16);
    }

    ct_uuid_pack(&u_time, uuid);
    return CT_RET_OK;
}

static void ct_uuid_pack(uuid_time *u_time, uuid_u *uuid)
{
    uint32_t tmp = 0;
    unsigned char * out = uuid->byte;

    tmp = u_time->time_low;
    out[3] = (unsigned char) tmp;
    tmp >>= 8;
    out[2] = (unsigned char) tmp;
    tmp >>= 8;
    out[1] = (unsigned char) tmp;
    tmp >>= 8;
    out[0] = (unsigned char) tmp;

    tmp = u_time->time_mid;
    out[5] = (unsigned char) tmp;
    tmp >>= 8;
    out[4] = (unsigned char) tmp;

    tmp = u_time->time_hi_and_version;
    out[7] = (unsigned char) tmp;
    tmp >>= 8;
    out[6] = (unsigned char) tmp;

    tmp = u_time->clock_seq;
    out[9] = (unsigned char) tmp;
    tmp >>= 8;
    out[8] = (unsigned char) tmp;

    memcpy(out+10, u_time->node, 6);
}

static void ct_uuid_unpack(uuid_u *uuid, uuid_time *u_time)
{
    uint32_t tmp = 0;
    const uint8_t * ptr = uuid->byte;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    tmp = (tmp << 8) | *ptr++;
    tmp = (tmp << 8) | *ptr++;
    u_time->time_low = tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    u_time->time_mid = tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    u_time->time_hi_and_version = tmp;

    tmp = *ptr++;
    tmp = (tmp << 8) | *ptr++;
    u_time->clock_seq = tmp;

    memcpy(u_time->node, ptr, 6);
}

static int ct_uuid_to_string(uuid_u *uuid, char *out, size_t len, const char *fmt)
{
    uuid_time u_time;

    ct_uuid_unpack(uuid, &u_time);

    return ct_snprintf(out,
              len,
              fmt,
              u_time.time_low,
              u_time.time_mid,
              u_time.time_hi_and_version,
              u_time.clock_seq >> 8,
              u_time.clock_seq & 0xFF,
              u_time.node[0],
              u_time.node[1],
              u_time.node[2],
              u_time.node[3],
              u_time.node[4],
              u_time.node[5]);
}

#if (defined __LINUX__) || (defined __ANDROID__)
static void linux_uuid_generate(uuid_u *uuid)
{
    if (linux_get_random_fd() >= 0)
    {
        linux_uuid_generate_random(uuid);
    }
    else
    {
        linux_uuid_generate_time(uuid);
    }
}

static int linux_get_random_fd(void)
{
    struct timeval	tv;
    static int	fd = -2;
    int		i;

    if (fd == -2) 
    {
        gettimeofday(&tv, 0);
        fd = open("/dev/urandom", O_RDONLY);
        if (fd == -1)
        {
            fd = open("/dev/random", O_RDONLY | O_NONBLOCK);
        }

        if (fd >= 0) 
        {
            i = fcntl(fd, F_GETFD);
            if (i >= 0)
            {
                fcntl(fd, F_SETFD, i | FD_CLOEXEC);
            }
        }

        srand((getpid() << 16) ^ getuid() ^ tv.tv_sec ^ tv.tv_usec);
    }

    /* Crank the random number generator a few times */
    gettimeofday(&tv, 0);
    for (i = (tv.tv_sec ^ tv.tv_usec) & 0x1F; i > 0; i--)
    {
        rand();
    }

    return fd;
}

static void linux_uuid_generate_random(uuid_u * uuid)
{
    /* No real reason to use the daemon for random uuid's -- yet */
    linux_uuid_generate_random_ex(uuid);
}

static void linux_uuid_generate_random_ex(uuid_u * uuid)
{
    uuid_u	u;
    uuid_time u_time;

    linux_get_random_bytes(&u, sizeof(uuid_u));
    ct_uuid_unpack(&u, &u_time);

    u_time.clock_seq = (u_time.clock_seq & 0x3FFF) | 0x8000;
    u_time.time_hi_and_version = (u_time.time_hi_and_version & 0x0FFF) | 0x4000;
    ct_uuid_pack(&u_time, uuid);
}

/*
 * Generate a series of random bytes.  Use /dev/urandom if possible,
 * and if not, use srandom/random.
 */
static void linux_get_random_bytes(void *buf, int nbytes)
{
    int i = 0;
    int n = nbytes;
    int fd = linux_get_random_fd();
    int lose_counter = 0;
    unsigned char *cp = (unsigned char *) buf;

    if (fd >= 0) 
    {
        while (n > 0) 
        {
            i = read(fd, cp, n);
            if (i <= 0) 
            {
                if (lose_counter++ > 16)
                    break;

                continue;
            }

            n -= i;
            cp += i;
            lose_counter = 0;
        }
    }

    /*
     * We do this all the time, but this is the only source of
     * randomness if /dev/random/urandom is out to lunch.
     */
    for (cp = buf, i = 0; i < nbytes; i++)
        *cp++ ^= (rand() >> 7) & 0xFF;

    return;
}

static void linux_uuid_generate_time(uuid_u *uuid)
{
    linux_uuid_generate_time_ex(uuid, 0);
}

static void linux_uuid_generate_time_ex(uuid_u *uuid, int *num)
{
    static unsigned char node_id[6];
    static int has_init = 0;
    uint32_t clock_mid;
    uuid_time u_time;

    if (! has_init) 
    {
        linux_get_random_bytes(node_id, 6);

        /*
         * Set multicast bit, to prevent conflicts
         * with IEEE 802 addresses obtained from
         * network cards
         */
        node_id[0] |= 0x01;

        has_init = 1;
    }

    linux_get_clock(&clock_mid, &u_time.time_low, &u_time.clock_seq, num);

    u_time.clock_seq |= 0x8000;
    u_time.time_mid = (uint16_t) clock_mid;
    u_time.time_hi_and_version = ((clock_mid >> 16) & 0x0FFF) | 0x1000;
    memcpy(u_time.node, node_id, 6);
    ct_uuid_pack(&u_time, uuid);
}

/* Assume that the gettimeofday() has microsecond granularity */
#define MAX_ADJUSTMENT 10
static int linux_get_clock(uint32_t *clock_high, uint32_t *clock_low, uint16_t *ret_clock_seq, int *num)
{
    THREAD_LOCAL int    adjustment = 0;
    THREAD_LOCAL struct timeval last = {0, 0};
    THREAD_LOCAL int    state_fd = -2;
    THREAD_LOCAL FILE   *state_f;
    THREAD_LOCAL uint16_t   clock_seq;
    struct timeval  tv;
    struct flock    fl;
    uint64_t        clock_reg;
    mode_t          save_umask;
    int             len;

    if (state_fd == -2) 
    {
        save_umask = umask(0);
        state_fd = open("/var/lib/libuuid/clock.txt", O_RDWR|O_CREAT, 0660);
        (void) umask(save_umask);
        state_f = fdopen(state_fd, "r+");
        if (!state_f) 
        {
            close(state_fd);
            state_fd = -1;
        }
    }

    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;
    if (state_fd >= 0) 
    {
        rewind(state_f);
        while (fcntl(state_fd, F_SETLKW, &fl) < 0) 
        {
            if ((errno == EAGAIN) || (errno == EINTR))
                continue;

            fclose(state_f);
            close(state_fd);
            state_fd = -1;
            break;
        }
    }

    if (state_fd >= 0) 
    {
        unsigned int cl;
        unsigned long tv1, tv2;
        int a;

        if (fscanf(state_f, "clock: %04x tv: %lu %lu adj: %d\n", &cl, &tv1, &tv2, &a) == 4) 
        {
            clock_seq = cl & 0x3FFF;
            last.tv_sec = tv1;
            last.tv_usec = tv2;
            adjustment = a;
        }
    }

    if ((last.tv_sec == 0) && (last.tv_usec == 0)) 
    {
        linux_get_random_bytes(&clock_seq, sizeof(clock_seq));
        clock_seq &= 0x3FFF;
        gettimeofday(&last, 0);
        last.tv_sec--;
    }

try_again:
    gettimeofday(&tv, 0);
    if ((tv.tv_sec < last.tv_sec) ||
            ((tv.tv_sec == last.tv_sec) &&
             (tv.tv_usec < last.tv_usec))) 
    {
        clock_seq = (clock_seq+1) & 0x3FFF;
        adjustment = 0;
        last = tv;
    }
    else if ((tv.tv_sec == last.tv_sec) &&
            (tv.tv_usec == last.tv_usec)) 
    {
        if (adjustment >= MAX_ADJUSTMENT)
            goto try_again;

        adjustment++;
    }
    else 
    {
        adjustment = 0;
        last = tv;
    }

    clock_reg = tv.tv_usec*10 + adjustment;
    clock_reg += ((uint64_t) tv.tv_sec)*10000000;
    clock_reg += (((uint64_t) 0x01B21DD2) << 32) + 0x13814000;

    if (num && (*num > 1)) 
    {
        adjustment += *num - 1;
        last.tv_usec += adjustment / 10;
        adjustment = adjustment % 10;
        last.tv_sec += last.tv_usec / 1000000;
        last.tv_usec = last.tv_usec % 1000000;
    }

    if (state_fd > 0) 
    {
        rewind(state_f);
        len = fprintf(state_f,
                "clock: %04x tv: %016lu %08lu adj: %08d\n",
                clock_seq, last.tv_sec, last.tv_usec, adjustment);
        fflush(state_f);

        if (ftruncate(state_fd, len) < 0) 
        {
            fprintf(state_f, "                   \n");
            fflush(state_f);
        }

        rewind(state_f);
        fl.l_type = F_UNLCK;
        fcntl(state_fd, F_SETLK, &fl);
    }

    *clock_high = clock_reg >> 32;
    *clock_low = clock_reg;
    *ret_clock_seq = clock_seq;
    return 0;
}

#endif /* __LINUX__ || __ANDROID__ */
