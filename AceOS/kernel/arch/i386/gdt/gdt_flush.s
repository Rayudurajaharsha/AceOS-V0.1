.global gdt_flush
.type gdt_flush, @function

gdt_flush:
    mov 4(%esp), %eax   /* Fetch GDT pointer from stack */
    lgdt (%eax)         /* Load GDT register */

    mov $0x10, %ax      /* 0x10 points to Kernel Data Segment */
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    ljmp $0x08, $flush  /* 0x08 points to Kernel Code Segment; far jump flushes CS */

flush:
    ret