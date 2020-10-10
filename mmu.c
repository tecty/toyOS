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

#include "gpio.h" // get MMIO_BASE
#include "uart.h"
#include "types.h"
#include "aarch64.h"

#define PAGESIZE 4096

#define L0_START 30
#define L1_START 21
#define L2_START 12
// granularity
#define PT_PAGE 0b11  // 4k granule
#define PT_BLOCK 0b01 // 2M granule
// accessibility
#define PT_KERNEL (0 << 6) // privileged, supervisor EL1 access only
#define PT_USER (1 << 6)   // unprivileged, EL0 access allowed
#define PT_RW (0 << 7)     // read-write
#define PT_RO (1 << 7)     // read-only
#define PT_AF (1 << 10)    // accessed flag
#define PT_NX (1UL << 54)  // no execute
// shareability
#define PT_OSH (2 << 8) // outter shareable
#define PT_ISH (3 << 8) // inner shareable
// defined in MAIR register
#define PT_MEM (0 << 2) // normal memory
#define PT_DEV (1 << 2) // device MMIO
#define PT_NC (2 << 2)  // non-cachable

#define TTBR_CNP 1
#define N_PAGE_TABLE BIT(9)

// get addresses from linker
extern char __data_start[1];
// extern volatile unsigned char _end;

typedef word_t pte_t;
pte_t pageTables[N_PAGE_TABLE][BIT(9)] ALIGN_BSS(BIT(12));
word_t pageTable_brk = 0;

word_t *alloc_pt()
{
    if (pageTable_brk == N_PAGE_TABLE)
    {
        return 0;
    }
    return (word_t *)&pageTables[pageTable_brk++];
}

static inline void enable_mmu(word_t ttbr0_el1, word_t ttbr1_el1, word_t tcr_el1)
{
    word_t tmp;
    MRS(tmp, "id_aa64mmfr0_el1");
    if (tmp & (0xF << 28) || (tmp & 0xF) < 1)
    {
        uart_puts("ERROR: 4k granule or 36 bit address space not supported\n");
        return;
    }

    // first, set Memory Attributes array, indexed by PT_MEM, PT_DEV, PT_NC in our example
    tmp = (0xFF << 0) | // AttrIdx=0: normal, IWBWA, OWBWA, NTR
          (0x04 << 8) | // AttrIdx=1: device, nGnRE (must be OSH too)
          (0x44 << 16); // AttrIdx=2: non cacheable
    MSR("mair_el1", tmp);

    // tell the MMU where our translation tables are. TTBR_CNP bit not documented, but required
    MSR("tcr_el1", tcr_el1);
    // lower half, user space
    MSR("ttbr0_el1", ttbr0_el1);
    // upper half, kernel space
    MSR("ttbr1_el1", ttbr1_el1);
    asm volatile("dsb ish\n\t"
                 "isb\n\t");
    // finally, toggle some bits in system control register to enable page translation
    MRS(tmp, "sctlr_el1");
    tmp |= 0xC00800;     // set mandatory reserved bits
    tmp &= ~((1 << 25) | // clear EE, little endian translation tables
             (1 << 24) | // clear E0E
             (1 << 19) | // clear WXN
             (1 << 12) | // clear I, no instruction cache
             (1 << 4) |  // clear SA0
             (1 << 3) |  // clear SA
             (1 << 2) |  // clear C, no cache at all
             (1 << 1));  // clear A, no aligment check
    tmp |= (1 << 0);     // set M, enable MMU

    MSR("sctlr_el1", tmp);
}

word_t *kernelPGD_Hi;
word_t *kernelPGD_Lo;

/**
 * Set up page translation tables and enable virtual memory
 */
void mmu_init()
{
    word_t data_page = (word_t)__data_start << L2_START;
    word_t r, b;

    /* create MMU translation tables at _end */
    kernelPGD_Hi = alloc_pt();
    kernelPGD_Lo = alloc_pt();

    word_t *l1_lo = alloc_pt();
    word_t *l2_lo = alloc_pt();

    kernelPGD_Lo[0] = (word_t)l1_lo | // physical address
                      PT_PAGE |       // it has the "Present" flag, which must be set, and we have area in it mapped by pages
                      PT_AF |         // accessed flag. Without this we're going to have a Data Abort exception
                      PT_USER |       // non-privileged
                      PT_ISH |        // inner shareable
                      PT_MEM;         // normal memory

    l1_lo[0] = (word_t)l2_lo | // physical address
               PT_PAGE |       // we have area in it mapped by pages
               PT_AF |         // accessed flag
               PT_USER |       // non-privileged
               PT_ISH |        // inner shareable
               PT_MEM;         // normal memory

    // identity L2 2M blocks
    b = MMIO_BASE >> L1_START;
    // skip 0th, as we're about to map it by L3
    for (r = 1; r < 512; r++)
        l1_lo[r] = (unsigned long)((r << 21)) |                  // physical address
                   PT_BLOCK |                                    // map 2M block
                   PT_AF |                                       // accessed flag
                   PT_NX |                                       // no execute
                   PT_USER |                                     // non-privileged
                   (r >= b ? PT_OSH | PT_DEV : PT_ISH | PT_MEM); // different attributes for device memory

    // identity L3
    for (r = 0; r < 512; r++)
        l2_lo[r] = (unsigned long)(r << L2_START) |                        // physical address
                   PT_PAGE |                                               // map 4k
                   PT_AF |                                                 // accessed flag
                   PT_USER |                                               // non-privileged
                   PT_ISH |                                                // inner shareable
                   ((r < 0x80 || r >= data_page) ? PT_RW | PT_NX : PT_RO); // different for code and data

    // TTBR1, kernel L1
    word_t *hi_l2 = alloc_pt();
    kernelPGD_Hi[MASK(9)] = (word_t)(hi_l2) | // physical address
                            PT_PAGE |         // we have area in it mapped by pages
                            PT_AF |           // accessed flag
                            PT_KERNEL |       // privileged
                            PT_ISH |          // inner shareable
                            PT_MEM;           // normal memory
    // TODO: add a map kernel device helper
    word_t *hi_l3 = alloc_pt();
    hi_l2[MASK(9)] = (word_t)hi_l3 |      // physical address
                     PT_PAGE |            // we have area in it mapped by pages
                     PT_AF |              // accessed flag
                     PT_KERNEL |          // privileged
                     PT_ISH |             // inner shareable
                     PT_MEM;              // normal memory
    hi_l3[0] = (MMIO_BASE + 0x00201000) | // physical address
               PT_PAGE |                  // map 4k
               PT_AF |                    // accessed flag
               PT_NX |                    // no execute
               PT_KERNEL |                // privileged
               PT_OSH |                   // outter shareable
               PT_DEV;                    // device memory

    /* okay, now we have to set system registers to enable MMU */
    word_t tcr_el1 = (0b00LL << 37) |  // TBI=0, no tagging
                     (0b001LL << 32) | // IPS=autodetected
                     (0b10LL << 30) |  // TG1=4k
                     (0b11LL << 28) |  // SH1=3 inner
                     (0b01LL << 26) |  // ORGN1=1 write back
                     (0b01LL << 24) |  // IRGN1=1 write back
                     (0b0LL << 23) |   // EPD1 enable higher half
                     (25LL << 16) |    // T1SZ=25, 3 levels (512G)
                     (0b00LL << 14) |  // TG0=4k
                     (0b11LL << 12) |  // SH0=3 inner
                     (0b01LL << 10) |  // ORGN0=1 write back
                     (0b01LL << 8) |   // IRGN0=1 write back
                     (0b0LL << 7) |    // EPD0 enable lower half
                     (25LL << 0);      // T0SZ=25, 3 levels (512G)

    // enable_mmu((word_t)kernelPGD_Lo | TTBR_CNP, (unsigned long)&_end + TTBR_CNP + PAGESIZE, tcr_el1);
    enable_mmu((word_t)kernelPGD_Lo | TTBR_CNP, (word_t)kernelPGD_Hi | TTBR_CNP, tcr_el1);
}
