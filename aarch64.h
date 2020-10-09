#pragma once

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

#define BIT(n) (1ul << n)
#define MASK(n) (BIT(n) - 1)
#define MASK_LEFT(n, l) (MASK(n) << l)

#define MRS(val, reg) asm volatile("mrs %x0," reg \
                                   : "=r"(val))
#define MSR(reg, v)                                \
    do                                             \
    {                                              \
        word_t _v = v;                             \
        asm volatile("msr " reg ",%x0" ::"r"(_v)); \
    } while (0)

#define ALIGN_BSS(n) \
    ALIGN(n)         \
    SECTION(".bss.aligned")
