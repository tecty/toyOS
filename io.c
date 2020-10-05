#include "io.h"
// 0x40000000
// 0x09000000
volatile unsigned int *const UART0_DR = (unsigned int *)0x09000000;
const char mapHex[] = "0123456789abcdef";
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
    print("0x");
    for (int k = 15; k >= 0; k--)
    {
        if (k == 7)
        {
            uart('_');
        }
        size_t tmp = i;
        tmp = tmp >> (k * 4);
        tmp &= 0b1111;
        uart(mapHex[tmp]);
    }
}

void putPtr(void *i)
{
    print("0x");
    putSizeTHex((size_t)i);
}