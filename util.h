/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */

#if !defined(UTIL_H)
#define UTIL_H

// KEYWORDS
#define PACKED __attribute__((packed))
#define NORETURN __attribute__((__noreturn__))
#define CONST __attribute__((__const__))
#define PURE __attribute__((__pure__))
#define ALIGN(n) __attribute__((__aligned__(n)))
#define FASTCALL __attribute__((fastcall))
#define NO_INLINE __attribute__((noinline))
#define FORCE_INLINE __attribute__((always_inline))
#define SECTION(sec) __attribute__((__section__(sec)))
#define UNUSED __attribute__((unused))
#define USED __attribute__((used))
#define FASTCALL __attribute__((fastcall))

#ifdef __GNUC__
/* Borrowed from linux/include/linux/compiler.h */
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (!!(x))
#define unlikely(x) (!!(x))
#endif

void memzero(void *s, unsigned long n);
void *memset(void *s, unsigned long c, unsigned long n);
void *memcpy(void *ptr_dst, const void *ptr_src, unsigned long n);
int strncmp(const char *s1, const char *s2, int n);
const long char_to_long(char c);
const long str_to_long(const char *str);

#endif // UTIL_H
