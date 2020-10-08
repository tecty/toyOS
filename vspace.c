#include "aarch64.h"
#include "util.h"
#include "io.h"

#define PAGE_TABLE_ENTRIES BIT(9)
#define BLOCK_ENTRY 0b01
#define TABLE_ENTRY 0b11
#define N_PAGE_TABLE BIT(9)
#define MAIR_INDEX(index) (index << 2)
#define PT_PADDR_MASK ((-1) << 12)

#define l0_PT_START 39
#define l1_PT_START 30
#define l2_PT_START 21
#define l3_PT_START 12



// have to fake an array to get the pointer value
extern char kernel_start[1];
extern char kernel_end[1];
extern char stack_top[1];
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

word_t pageTables[N_PAGE_TABLE][BIT(9)] ALIGN_BSS(BIT(12));
word_t pageTable_brk = 0;
word_t *kernelPGD;

// void dumpMap(const char *pre, word_t index, word_t vaddr, word_t padddr, word_t value)
// {
//     print(pre);
//     print(" - index: ");
//     putSizeTHex(index);
//     print("\tvaddr: ");
//     putSizeTHex(vaddr);
//     print(" -> paddr: ");
//     putSizeTHex(padddr);
//     print(" = ");
//     putSizeTHex(value);
//     print("\n");
// }

static inline word_t mask_get_index(word_t right, word_t addr)
{
    return (addr >> right) & MASK(9);
}

word_t *alloc_pt()
{
    if (pageTable_brk == N_PAGE_TABLE)
    {
        return NULL;
    }
    return (word_t *)&pageTables[pageTable_brk++];
}

static inline word_t pte_new(word_t *pt_paddr, enum mair_types mtype, word_t entry_type)
{
    entry_type = ((word_t)pt_paddr) | MAIR_INDEX(mtype) | (entry_type & 0b11);
    print("==> PTE_NEW ");
    putSizeTHex(entry_type);
    ENDL;

    return entry_type;
}

/* The region has identical va and pa */
void map_kernel_identical(word_t *l2_pt)
{
    size_t entry_index = 0;
    // print("helloword\n");

    for (size_t idmap_addr = (word_t)kernel_start; idmap_addr <= (word_t)kernel_end; idmap_addr += (1 << l2_PT_START))
    {
        if (entry_index >= BIT(9))
        {
            print("kerenl is too big to fit in 1 level 1 PT\n");
            return;
        }
        // DUMP_PTR((void *)mask_get_index(l2_PT_START, idmap_addr));
        word_t target_addr = (word_t)kernel_start;
        target_addr += entry_index << l2_PT_START;
        l2_pt[mask_get_index(l2_PT_START, idmap_addr)] = pte_new((word_t *)target_addr, NORMAL, BLOCK_ENTRY);
        entry_index++;
    }
}

static inline void page_table_boot()
{
    kernelPGD = alloc_pt();
    word_t *l1_page_table = alloc_pt();
    kernelPGD[mask_get_index(l0_PT_START, (word_t)kernel_start)] = pte_new(l1_page_table, NORMAL, TABLE_ENTRY);
    word_t *l2_page_table = alloc_pt();
    l1_page_table[mask_get_index(l1_PT_START, (word_t)kernel_start)] = pte_new(l2_page_table, NORMAL, TABLE_ENTRY);
    map_kernel_identical(l2_page_table);
    print("\n==> page_table_inited\n");
    DUMP_PTR(kernelPGD);
    DUMP_PTR(l1_page_table);
    DUMP_PTR(l2_page_table);
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
    // page_table_boot();
    // putSizeTHex(get_PT_addr(1));
    // print("\n");
    // putSizeTHex(kernelPGD);
    // print("\n");
    page_table_boot();

    // enable mmu
    // 4GB space 4KB granularity
    enable_mmu_el1((word_t)kernelPGD, (word_t)kernelPGD, 0x3520);
    while (1)
    {
        /* code */
    }
}