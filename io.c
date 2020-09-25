#include "io.h"

volatile unsigned int *const UART0_DR = (unsigned int *)0x09000000;
static inline void uart(const char c)
{
    *UART0_DR = (unsigned int)c;
}

void print(const char *s)
{
    while (*s != '\0')
        uart(*s++);
}

void putSizeT(size_t i)
{
    if (!i)
        return;
    size_t curr = i % 10;
    putSizeT(i / 10);
    uart(curr + '0');
}

void putSizeTHex(size_t i)
{
    if (!i)
        return;
    size_t curr = i % 16;
    putSizeTHex(i / 16);
    if (curr < 10)
    {
        uart(curr + '0');
    }
    else
    {
        uart(curr + 'A' - 10);
    }
}

void putPtr(void *i)
{
    print("0x");
    putSizeTHex((size_t)i);
}