#include "aarch64.h"
#include "util.h"
#include "io.h"

/*
 * Memory types are defined in Memory Attribute Indirection Register.
 *  - nGnRnE Device non-Gathering, non-Reordering, No Early write acknowledgement
 *  - nGnRE Unused Device non-Gathering, non-Reordering, Early write acknowledgement
 *  - GRE Unused Device Gathering, Reordering, Early write acknowledgement
 *  - NORMAL_NC Normal Memory, Inner/Outer non-cacheable
 *  - NORMAL Normal Memory, Inner/Outer Write-back non-transient, Write-allocate, Read-allocate
 * Note: These should match with contents of MAIR_EL1 register!
 */
enum mair_types
{
    DEVICE_nGnRnE = 0,
    DEVICE_nGnRE = 1,
    DEVICE_GRE = 2,
    NORMAL_NC = 3,
    NORMAL = 4
};

#define ALIGN_BSS(n) \
    ALIGN(n)         \
    SECTION(".bss.aligned")

word_t kernelPGD[BIT(9)] ALIGN_BSS(BIT(12));
// 32 more page table should boot
word_t kernelPT[BIT(9 + 5)] ALIGN_BSS(BIT(12));

/**
 * generate the top level page table entry 
*/
word_t pgde_new(word_t paddr)
{
    paddr = paddr & 0xFFFFF000;
    paddr |= 0x0741;
    return paddr;
}

word_t pte_new(word_t paddr, enum mair_types mtype)
{
    paddr = paddr & 0xFFFFF000;
    paddr |= 0x0743;
    paddr |= mtype << 2;
    return paddr;
}

static inline word_t get_PT_addr(word_t pt_idx)
{
    word_t ret = (word_t)kernelPT;
    ret += pt_idx << 9;
    return ret;
}

static inline word_t get_PT_index(word_t pt_idx, word_t entry)
{
    // this has same effect as define, will optimsed out to constant by compiler
    return (pt_idx << 9) + entry;
}

void dumpMap(word_t vaddr, word_t padddr)
{

    print("vaddr: ");
    putSizeTHex(vaddr);
    print(" -> paddr: ");
    putSizeTHex(padddr);
    print("\n");
}

void map_kerenlPGD(word_t vaddr, word_t padddr)
{
    // dumpMap((vaddr >> 23) & MASK(9), padddr);
    kernelPGD[(vaddr >> 23) & MASK(9)] = pgde_new(padddr);
}

/**
    Map to actual block to prove concept 
*/
void map_kernelPT(word_t vaddr, word_t padddr, word_t pt_idx, enum mair_types mtype)
{
    // dumpMap(get_PT_index(pt_idx, (vaddr >> 14) & MASK(9)), padddr);
    kernelPT[get_PT_index(pt_idx, (vaddr >> 14) & MASK(9))] = pte_new(padddr, mtype);
}

static inline void page_table_boot()
{
    // for uart devices
    map_kerenlPGD(0x09000000, get_PT_addr(0));
    map_kernelPT(0x09000000, 0x09000000, 0, DEVICE_nGnRnE);
    // for kernel codes
    map_kerenlPGD(0x40000000, get_PT_addr(1));
    map_kernelPT(0x40000000, 0x40000000, 1, NORMAL);
}

void vspace_boot()
{

    /*
     *  set up mair table
     *  0x00 = 0b00000000    DEVICE_nGnRnE
     *  0x04 = 0b00000100    DEVICE_nGnRE
     *  0x0c = 0b00001100    DEVICE_GRE
     *  0x44 = 0b01000100    NORMAL_NC
     *  0xff = 0b11111111    NORMAL
    */
    MSR("MAIR_EL1", 0xff440c0400);
    page_table_boot();

    putSizeTHex(kernelPGD);
    print("\n");
    putSizeTHex(kernelPT);

    // enable mmu
    // 4GB space 4KB granularity
    enable_mmu_el1((word_t)kernelPGD, (word_t)kernelPGD, 0x3520);
    while (1)
    {
    }
}