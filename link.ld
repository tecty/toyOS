ENTRY(_start)
SECTIONS
{
    . = 0x00000000; 
    
    . = 0x40000000;
    .startup . : { startup.o(.text) }
    arm_vector_table = .;
    .vectors : {
        trap.o(.vectors)
    }
    . = ALIGN(8);
    .text : { 
        *(.text) 
    }
    data_start = . ;
    .data : { *(.data) }
    data_end = . ;
    bss_start = .; 
    .bss : { *(.bss COMMON) }
    bss_end = .; 
    
    . = ALIGN(8);
    . = . + 0x1000; /* 4kB of stack memory */
    stack_top = .;
}
