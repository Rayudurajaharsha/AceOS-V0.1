.global idt_flush
.type idt_flush, @function

idt_flush:
    mov 4(%esp), %eax   /* Load pointer to idt_ptr from stack */
    lidt (%eax)         /* Load Interrupt Descriptor Table Register */
    ret