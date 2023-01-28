/*
 * Copyright (c) 2005,2006 Petr Machata
 * All rights reserved.
 */

#ifndef _AL60L_UTIL_H_
#define _AL60L_UTIL_H_

#include <setjmp.h>

/// Strdup for purposes of -std=c99 compilation.
char * a_strdup (char const* str);

/// Fire longjmp (env, fail_signal) when ptr == NULL
void guard_ptr (jmp_buf env, int fail_signal, void * ptr);

/// Fire longjmp (evn, fail_signal) when errcode != 0
void guard_int (jmp_buf env, int fail_signal, int errcode);

/// If buf is non-NULL, just return it.  Otherwise return the result
/// of buf_creator call.
void * tmpbuild (void * buf, void* (*buf_creator)(void));


#endif//_AL60L_UTIL_H_
