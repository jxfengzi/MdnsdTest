/*
 * Copyright (C) 2013-2015
 *
 * @author coding.tom@gmail.com
 * @date   2013-7-9
 *
 * @file   ct_malloc.h
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 *
 */

#ifndef __CT_MALLOC_H__
#define __CT_MALLOC_H__

#include "ct_common.h"

CT_BEGIN_DECLS


void * ct_malloc(uint32_t size);
void * ct_realloc(void *p, uint32_t size);
void ct_free(void *p);


CT_END_DECLS

#endif /* __CT_MALLOC_H__ */
