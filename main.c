/*
 * Copyright (C) 2018 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include "uart.h"
#include "mmu.h"
#include "types.h"
#include "aarch64.h"

#define KERNEL_UART0_DR ((volatile unsigned int *)0xFFFFFFFFFFE00000)
#define KERNEL_UART0_FR ((volatile unsigned int *)0xFFFFFFFFFFE00018)

extern char __bss_start[1];
extern char __data_start[1];
extern char __bss_end[1];
extern char __vector_start[1];

void memset(void *begin, void *end, word_t val)
{
    for (word_t *p = (word_t *)begin; p != (word_t *)end; p++)
    {
        *p = val;
    }
}

void boot()
{
    // clear all the data section
    memset(__bss_start, __bss_end, 0);

    // set up serial console
    uart_init();
    // set the execption handler
    MSR("vbar_el1", (word_t)__vector_start);
    // set up paging
    mmu_init();
}

void main()
{
    boot();

    // test mapping
    uart_puts("LO\t");

    char *s = "HI\r\n";
    // test mapping
    while (*s)
    {
        /* wait until we can send */
        do
        {
            asm volatile("nop");
        } while (*KERNEL_UART0_FR & 0x20);
        /* write the character to the buffer */
        *KERNEL_UART0_DR = *s++;
    }

    // uart_puts("here am i ");

    volatile word_t *r = (word_t *)0xFFFFFF0000000000;
    *r = 100;

    // echo everything back
    while (1)
    {
        uart_send(uart_getc());
    }
}
