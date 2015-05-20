/*
 * Copyright (C) 2013-2015
 *
 * @author jxfengzi@gmail.com
 * @date   2013-7-9
 *
 * @file   ct_api.h
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 */

#ifndef __CT_API_H__
#define __CT_API_H__


#ifdef _Mct_VER
#   if (defined airtunes_client_shared_EXPORTS) || (defined dlna_shared_EXPORTS)
#       define DLLEXPORT _declspec(dllexport)
#   else
#       define DLLEXPORT _declspec(dllimport)
#   endif
#   define DLLLOCAL
#else
#   ifdef __ANDROID__
#       define DLLEXPORT
#       define DLLLOCAL
#   else
#       define DLLEXPORT __attribute__ ((visibility("default")))
#       define DLLLOCAL __attribute__ ((visibility("hidden")))
#   endif /* __ANDROID__ */
#endif /* _Mct_VER */


#ifdef CT_STATIC
#   define CT_API
#else
#   define CT_API DLLEXPORT
#endif /* CT_STATIC */


#endif /* __CT_API_H__ */
