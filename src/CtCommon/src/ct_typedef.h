/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-7-9
 *
 * @file   ct_typedef.h
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 */

#ifndef __CT_TYPEDEF_H__
#define __CT_TYPEDEF_H__

#ifdef _WIN32
#include <ws2tcpip.h>
#include <windows.h>
#include <time.h>
#else /* Linux */
#include <unistd.h>
#include <sys/time.h>
#endif /* _WIN32 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>

#ifdef __cplusplus
    #define CT_BEGIN_DECLS extern "C" {
    #define CT_END_DECLS }
#else
    #define CT_BEGIN_DECLS
    #define CT_END_DECLS
#endif

#ifndef STR_EQUAL
    #define STR_EQUAL(a, b) (strcmp(a, b) == 0)
#endif

#define DIM(a) (sizeof(a)/sizeof(a[0]))

#ifdef CT_DEBUG
    #ifdef _WIN32
        #include <crtdbg.h>
        #define RETURN_IF_FAIL(p) if(!(p)) \
                        {printf("%s:%d Warning: "#p" failed.\n", \
                        __FILE__, __LINE__); return;}
        #define RETURN_VAL_IF_FAIL(p, ret) if(!(p)) \
            	        {printf("%s:%d Warning: "#p" failed.\n", \
                        __FILE__, __LINE__); return (ret);}
    #else
        #define RETURN_IF_FAIL(p) if(!(p)) \
            	        {printf("%s:%d Warning: "#p" failed.\n", \
                        __func__, __LINE__); return;}
        #define RETURN_VAL_IF_FAIL(p, ret) if(!(p)) \
            	        {printf("%s:%d Warning: "#p" failed.\n", \
                        __func__, __LINE__); return (ret);}
    #endif /* _WIN32 */
#else
    #define RETURN_IF_FAIL(p)
    #define RETURN_VAL_IF_FAIL(p, ret)
#endif /* CT_DEBUG */

#ifdef _WIN32
    #define CT_INLINE              __inline
    #define ct_snprintf            _snprintf
    #define ct_strdup              _strdup
#else
    #define CT_INLINE              inline
    #define ct_snprintf            snprintf
    #define ct_strdup              strdup
#endif /* _WIN32 */


#endif /* __CT_TYPEDEF_H__ */
