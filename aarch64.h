/* -*- mode: C; coding:utf-8 -*- */
/**********************************************************************/
/*  OS kernel sample                                                  */
/*  Copyright 2014 Takeharu KATO                                      */
/*                                                                    */
/*  AArch64 definitions                                               */
/*                                                                    */
/**********************************************************************/
#if !defined(_AARCH64_H)
#define _AARCH64_H

#include "types.h"
/*
 * Reference: ARM® Architecture Reference Manual ARMv8, for ARMv8-A architecture profile
 */
#include <stdint.h>
/* CurrentEL, Current Exception Level */
#define CURRENT_EL_MASK 0x3
#define CURRENT_EL_SHIFT 2

#define BM(base, count, val) (((val) & ((1UL << (count)) - 1)) << (base))

/* DAIF, Interrupt Mask Bits */
#define DAIF_DBG_BIT (1 << 3) /* Debug mask bit */
#define DAIF_ABT_BIT (1 << 2) /* Asynchronous abort mask bit */
#define DAIF_IRQ_BIT (1 << 1) /* IRQ mask bit */
#define DAIF_FIQ_BIT (1 << 0) /* FIQ mask bit */

/*
 * Interrupt flags
 */
#define AARCH64_DAIF_FIQ (1) /* FIQ */
#define AARCH64_DAIF_IRQ (2) /* IRQ */

/* Timer */
#define CNTV_CTL_ENABLE (1 << 0)  /* Enables the timer */
#define CNTV_CTL_IMASK (1 << 1)   /* Timer interrupt mask bit */
#define CNTV_CTL_ISTATUS (1 << 2) /* The status of the timer interrupt. This bit is read-only */

/* Assembly helper */
#define asm __asm__
#define MRS(val, reg) asm volatile("mrs %x0," reg \
                                   : "=r"(val))
#define MSR(reg, v)                                \
    do                                             \
    {                                              \
        word_t _v = v;                             \
        asm volatile("msr " reg ",%x0" ::"r"(_v)); \
    } while (0)
#define SYSTEM_WRITE_WORD(reg, v) MSR(reg, v)
#define SYSTEM_READ_WORD(reg, v) MRS(reg, v)

/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * SPDX-License-Identifier: GPL-2.0-only
 */
#define MASK(n) (BIT(n) - UL_CONST(1))
#define MASK_SHIFT(n, left) (MASK(n) << UL_CONST(left))
#define IS_ALIGNED(n, b) (!((n)&MASK(b)))
#define ROUND_DOWN(n, b) (((n) >> (b)) << (b))
#define ROUND_UP(n, b) (((((n)-UL_CONST(1)) >> (b)) + UL_CONST(1)) << (b))
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define PASTE(a, b) a##b
#define _STRINGIFY(a) #a
#define STRINGIFY(a) _STRINGIFY(a)
#define NULL ((void *)0)
#define BIT(n) (1ul << (n))
#define UL_CONST(x) PASTE(x, ul)

/* Wait For Interrupt */
#define wfi() asm volatile("wfi" \
                           :     \
                           :     \
                           : "memory")

/* CurrentEL, Current Exception Level
	EL, bits [3:2]
		Current exception level. Possible values of this field are:
		00 EL0
		01 EL1
		10 EL2
		11 EL3
*/
static inline uint32_t raw_read_current_el(void)
{
    uint32_t current_el;

    __asm__ __volatile__("mrs %0, CurrentEL\n\t"
                         : "=r"(current_el)
                         :
                         : "memory");
    return current_el;
}

static inline uint32_t get_current_el(void)
{
    uint32_t current_el = raw_read_current_el();
    return ((current_el >> CURRENT_EL_SHIFT) & CURRENT_EL_MASK);
}

/* DAIF, Interrupt Mask Bits
	Allows access to the interrupt mask bits.

	D, bit [9]: Debug exceptions.
	A, bit [8]: SError (System Error) mask bit.
	I, bit [7]: IRQ mask bit.
	F, bit [6]: FIQ mask bit.
	value: 
		0 Exception not masked.
		1 Exception masked.
*/
static inline uint32_t raw_read_daif(void)
{
    uint32_t daif;

    __asm__ __volatile__("mrs %0, DAIF\n\t"
                         : "=r"(daif)
                         :
                         : "memory");
    return daif;
}

static inline void raw_write_daif(uint32_t daif)
{
    __asm__ __volatile__("msr DAIF, %0\n\t"
                         :
                         : "r"(daif)
                         : "memory");
}

static inline void enable_debug_exceptions(void)
{
    __asm__ __volatile__("msr DAIFClr, %0\n\t"
                         :
                         : "i"(DAIF_DBG_BIT)
                         : "memory");
}

static inline void enable_serror_exceptions(void)
{
    __asm__ __volatile__("msr DAIFClr, %0\n\t"
                         :
                         : "i"(DAIF_ABT_BIT)
                         : "memory");
}

static inline void enable_irq(void)
{
    __asm__ __volatile__("msr DAIFClr, %0\n\t"
                         :
                         : "i"(DAIF_IRQ_BIT)
                         : "memory");
}

static inline void enable_fiq(void)
{
    __asm__ __volatile__("msr DAIFClr, %0\n\t"
                         :
                         : "i"(DAIF_FIQ_BIT)
                         : "memory");
}

static inline void disable_debug_exceptions(void)
{
    __asm__ __volatile__("msr DAIFSet, %0\n\t"
                         :
                         : "i"(DAIF_DBG_BIT)
                         : "memory");
}

static inline void disable_serror_exceptions(void)
{
    __asm__ __volatile__("msr DAIFSet, %0\n\t"
                         :
                         : "i"(DAIF_ABT_BIT)
                         : "memory");
}

static inline void disable_irq(void)
{
    __asm__ __volatile__("msr DAIFSet, %0\n\t"
                         :
                         : "i"(DAIF_IRQ_BIT)
                         : "memory");
}

static inline void disable_fiq(void)
{
    __asm__ __volatile__("msr DAIFSet, %0\n\t"
                         :
                         : "i"(DAIF_FIQ_BIT)
                         : "memory");
}

/* SPSR_EL1, Saved Program Status Register (EL1)
	Holds the saved processor state when an exception is taken to EL1.
*/
static inline uint32_t raw_read_spsr_el1(void)
{
    uint32_t spsr_el1;

    __asm__ __volatile__("mrs %0, SPSR_EL1\n\t"
                         : "=r"(spsr_el1)
                         :
                         : "memory");
    return spsr_el1;
}

static inline void raw_write_spsr_el1(uint32_t spsr_el1)
{
    __asm__ __volatile__("msr SPSR_EL1, %0\n\t"
                         :
                         : "r"(spsr_el1)
                         : "memory");
}

/* 
ISR_EL1, Interrupt Status Register
	Shows whether an IRQ, FIQ, or SError interrupt is pending.
*/
static inline uint32_t raw_read_isr_el1(void)
{
    uint32_t isr_el1;

    __asm__ __volatile__("mrs %0, ISR_EL1\n\t"
                         : "=r"(isr_el1)
                         :
                         : "memory");
    return isr_el1;
}

/* 
RVBAR_EL1, Reset Vector Base Address Register (if EL2 and EL3 not implemented)
	If EL1 is the highest exception level implemented, contains the 
	IMPLEMENTATION DEFINED address that execution starts from after reset when
	executing in AArch64 state.
*/
static inline uint64_t raw_read_rvbar_el1(void)
{
    uint64_t rvbar_el1;

    __asm__ __volatile__("mrs %0, RVBAR_EL1\n\t"
                         : "=r"(rvbar_el1)
                         :
                         : "memory");
    return rvbar_el1;
}

static inline void raw_write_rvbar_el1(uint64_t rvbar_el1)
{
    __asm__ __volatile__("msr RVBAR_EL1, %0\n\t"
                         :
                         : "r"(rvbar_el1)
                         : "memory");
}

/* VBAR_EL1, Vector Base Address Register (EL1)
	Holds the exception base address for any exception that is taken to EL1.
*/
static inline uint64_t raw_read_vbar_el1(void)
{
    uint64_t vbar_el1;

    __asm__ __volatile__("mrs %0, VBAR_EL1\n\t"
                         : "=r"(vbar_el1)
                         :
                         : "memory");
    return vbar_el1;
}

static inline void raw_write_vbar_el1(uint64_t vbar_el1)
{
    __asm__ __volatile__("msr VBAR_EL1, %0\n\t"
                         :
                         : "r"(vbar_el1)
                         : "memory");
}

/* CNTV_CTL_EL0, Counter-timer Virtual Timer Control register
	Control register for the virtual timer.

	ISTATUS, bit [2]:	The status of the timer interrupt.
	IMASK, bit [1]:		Timer interrupt mask bit.
	ENABLE, bit [0]:	Enables the timer.
*/
static inline uint32_t raw_read_cntv_ctl(void)
{
    uint32_t cntv_ctl;

    __asm__ __volatile__("mrs %0, CNTV_CTL_EL0\n\t"
                         : "=r"(cntv_ctl)
                         :
                         : "memory");
    return cntv_ctl;
}

static inline void disable_cntv(void)
{
    uint32_t cntv_ctl;

    cntv_ctl = raw_read_cntv_ctl();
    cntv_ctl &= ~CNTV_CTL_ENABLE;
    __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t"
                         :
                         : "r"(cntv_ctl)
                         : "memory");
}

static inline void enable_cntv(void)
{
    uint32_t cntv_ctl;

    cntv_ctl = raw_read_cntv_ctl();
    cntv_ctl |= CNTV_CTL_ENABLE;
    __asm__ __volatile__("msr CNTV_CTL_EL0, %0\n\t"
                         :
                         : "r"(cntv_ctl)
                         : "memory");
}

/*
CNTFRQ_EL0, Counter-timer Frequency register
	Holds the clock frequency of the system counter.
*/
static inline uint32_t raw_read_cntfrq_el0(void)
{
    uint32_t cntfrq_el0;

    __asm__ __volatile__("mrs %0, CNTFRQ_EL0\n\t"
                         : "=r"(cntfrq_el0)
                         :
                         : "memory");
    return cntfrq_el0;
}

static inline void raw_write_cntfrq_el0(uint32_t cntfrq_el0)
{
    __asm__ __volatile__("msr CNTFRQ_EL0, %0\n\t"
                         :
                         : "r"(cntfrq_el0)
                         : "memory");
}

/* CNTVCT_EL0, Counter-timer Virtual Count register
	Holds the 64-bit virtual count value.
*/
static inline uint64_t raw_read_cntvct_el0(void)
{
    uint64_t cntvct_el0;

    __asm__ __volatile__("mrs %0, CNTVCT_EL0\n\t"
                         : "=r"(cntvct_el0)
                         :
                         : "memory");
    return cntvct_el0;
}

/* CNTV_CVAL_EL0, Counter-timer Virtual Timer CompareValue register
	Holds the compare value for the virtual timer.
*/
static inline uint64_t raw_read_cntv_cval_el0(void)
{
    uint64_t cntv_cval_el0;

    __asm__ __volatile__("mrs %0, CNTV_CVAL_EL0\n\t"
                         : "=r"(cntv_cval_el0)
                         :
                         : "memory");
    return cntv_cval_el0;
}

static inline void raw_write_cntv_cval_el0(uint64_t cntv_cval_el0)
{
    __asm__ __volatile__("msr CNTV_CVAL_EL0, %0\n\t"
                         :
                         : "r"(cntv_cval_el0)
                         : "memory");
}

static inline void isb()
{
    __asm__ __volatile__("isb");
}

static inline void set_ttbr0_el0(uint64_t ttbr0_el0)
{
    __asm__ __volatile__("msr TTBR0_EL0, %0\n\t"
                         :
                         : "r"(ttbr0_el0)
                         : "memory");
}

/**
 *  Enble el1 mmu by specify three registers 
 *  Need to check (ID_AA64MMFR0_EL1) in hardware, the page size is defined by hardware
*/
static inline void enable_mmu_el1(uint64_t ttbr0_el1, uint64_t ttbr1_el1, uint64_t tcr_el1)
{
    asm volatile(
        "tlbi vmalle1is\n\t"
        "isb\n\t"
        "dsb sy\n\t");
    MSR("TTBR0_EL1", ttbr0_el1);
    MSR("TTBR1_EL1", ttbr1_el1);
    MSR("TCR_EL1", tcr_el1);
    isb();
    word_t sctlr_tmp;
    MRS(sctlr_tmp, "SCTLR_EL1");
    MSR("SCTLR_EL1", sctlr_tmp | 1);
    isb();
}

#endif /*  _AARCH64_H   */
