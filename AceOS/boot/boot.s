/* Declare constants for the multiboot header. */
.set ALIGN,    1<<0             /* align loaded modules on page boundaries */
.set MEMINFO,  1<<1             /* provide memory map */
.set FLAGS,    ALIGN | MEMINFO  /* this is the Multiboot 'flag' field */
.set MAGIC,    0x1BADB002       /* 'magic number' lets bootloader find the header */
.set CHECKSUM, -(MAGIC + FLAGS) /* checksum of above, to prove we are multiboot */

/* 
Declare a header as in the Multiboot Standard. We put this into a special 
section so we can force the header to be in the first 8 KiB of the kernel file.
*/
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

/*
Allocate the initial stack.
The stack grows downwards on x86. We allocate 16 KiB of memory here.
*/
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

/*
The kernel entry point.
The bootloader will jump to this position once the kernel is loaded.
*/
.section .text
.global _start
.type _start, @function
_start:
    /* Set up the stack pointer */
    mov $stack_top, %esp

    /* Call the global function 'kernel_main' from your C code. */
    call kernel_main

    /* 
    If the system has nothing more to do, put the computer into an 
    infinite loop. To do that:
    1) Disable interrupts with cli (clear interrupt enable in eflags).
    2) Wait for the next interrupt to arrive with hlt (halt instruction).
    3) Jump to the hlt instruction if it ever wakes up due to a non-maskable interrupt.
    */
    cli
1:  hlt
    jmp 1b

/* Set the size of the _start symbol to the current location '.' minus its start. */
.size _start, . - _start