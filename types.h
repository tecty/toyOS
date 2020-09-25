#if !defined(TYPES_H)
#define TYPES_H

#define MRS(reg, v) asm volatile("mrs %x0," reg \
                                 : "=r"(v))
#define MSR(reg, v)                                \
    do                                             \
    {                                              \
        word_t _v = v;                             \
        asm volatile("msr " reg ",%x0" ::"r"(_v)); \
    } while (0)
#define SYSTEM_WRITE_WORD(reg, v) MSR(reg, v)
#define SYSTEM_READ_WORD(reg, v) MRS(reg, v)
/**
 * data type alias by arm document 
 https://developer.arm.com/documentation/den0024/a/porting-to-a64/data-types
*/

typedef unsigned long long uint64_t;
typedef uint64_t word_t;
typedef uint64_t size_t;

typedef int int32_t;
typedef long long int64_t;

#endif // TYPES_H
