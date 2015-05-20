/*
 * Copyright (C) 2013-2015
 *
 * @author coding.tom@gmail.com
 * @date   2013-7-9
 *
 * @file   ct_malloc.c
 *
 * @remark
 *      set tabstop=4
 *      set shiftwidth=4
 *      set expandtab
 *
 */

#include "ct_memory.h"
#include <stdlib.h>
#include <stdio.h>

void * ct_malloc(uint32_t size)
{
    return malloc(size);
}

void * ct_realloc(void *p, uint32_t size)
{
    return realloc(p, size);
}

void ct_free(void *p)
{
    free(p);
}
