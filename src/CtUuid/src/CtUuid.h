/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-7-9
 *
 * @file   CtUuid.h
 *
 * @remark
 *
 */

#ifndef __CT_UUID_H__
#define __CT_UUID_H__

#include "ct_common.h"

CT_BEGIN_DECLS

#define CT_UUID_LEN    (36 + 2)

/**************************************************************************
 *
 * uuid_t for Windows
 *
 **************************************************************************/
#ifdef _WIN32
/* uuid_t is defined in guiddef.h */
#endif /* _WIN32 */

/**************************************************************************
 *
 * uuid_t for OSX
 *
 **************************************************************************/
#ifdef __MAC_OSX__
/* uuit_t is defined in /usr/include/unistd.h */
#endif /* __MAC_OSX__ */

/**************************************************************************
 *
 * uuid_t for Linux or Android
 *
 **************************************************************************/
#if (defined __LINUX__) || (defined __ANDROID__)
typedef struct _uuid_t
{
    uint32_t    data1;          // 4
    uint16_t    data2;          // 2
    uint16_t    data3;          // 2
    uint8_t     data4[8];       // 8
} uuid_t;
#endif /* __LINUX__ || __ANDROID__ */

typedef union
{
    uuid_t      u;
    uint8_t     byte[16];
} uuid_u;

typedef struct _CtUuid
{
    char        string[CT_UUID_LEN];
    uuid_u      uuid;
} CtUuid;

CtUuid * CtUuid_New(void);
CtRet CtUuid_Construct(CtUuid *thiz);
CtRet CtUuid_Dispose(CtUuid *thiz);
void CtUuid_Delete(CtUuid *thiz);

CtRet CtUuid_GenerateRandom(CtUuid *thiz);
CtRet CtUuid_ParseFromString(CtUuid *thiz, const char *string);
CtRet CtUuid_ParseFromBuffer(CtUuid *thiz, const char *buffer, uint32_t len);
//uint32_t CtUuid_ToString(CtUuid * thiz, bool upper, char *str, uint32_t len);
const char * CtUuid_ToString(CtUuid * thiz, bool upper);

void CtUuid_Clear(CtUuid *thiz);
bool CtUuid_Equal(CtUuid *thiz, CtUuid *other);
void CtUuid_Copy(CtUuid *dst, CtUuid *src);
bool CtUuid_IsNull(CtUuid *thiz);


CT_END_DECLS

#endif /* __CT_UUID_H__ */
