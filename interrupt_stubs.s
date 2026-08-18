.global irq1_stub
.extern keyboard_handler

irq1_stub:
    pusha                   /* Push EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI */
    call keyboard_handler   /* Call C keyboard handler function */
    popa                    /* Restore register state */
    iret                    /* Interrupt return */


.global irq0_stub
.extern timer_handler

irq0_stub:
    pusha           # Save all general-purpose registers
    cld             # Clear direction flag (C calling convention)
    call timer_handler
    popa            # Restore registers
    iret            # Return from interrupt