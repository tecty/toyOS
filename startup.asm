.extern stack_top
.global _start
_start:
	
	// reset control registers
	// we are on el1 
	LDR    X1, =0x30C50838
	MSR    SCTLR_EL1, X1
	
	ldr x30, =stack_top
	mov sp, x30
	bl main
hang:
	b hang

.equ _psci_system_off, 0x84000008
.global system_off
system_off:
	ldr x0, =_psci_system_off
    hvc #0

