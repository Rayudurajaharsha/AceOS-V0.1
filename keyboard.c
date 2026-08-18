#include "keyboard.h"
#include "idt.h"
#include <stdint.h>

#define KEYBOARD_DATA_PORT 0x60
#define PIC1_COMMAND_PORT  0x20
#define PIC1_DATA_PORT     0x21

extern void terminal_putchar(char c);
extern void irq1_stub(void);

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static const char scancode_ascii[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
     0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
     0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
   '*',   0, ' '
};

/* Buffer variables */
#define CMD_BUFFER_SIZE 256
static char cmd_buffer[CMD_BUFFER_SIZE];
static int cmd_index = 0;

/* External functions from kernel.c */
extern void execute_command(char* input);
extern void terminal_writestring(const char* data);

void keyboard_handler(void) {
    uint8_t scancode = inb(KEYBOARD_DATA_PORT);

    if (!(scancode & 0x80)) {
        if (scancode < sizeof(scancode_ascii)) {
            char c = scancode_ascii[scancode];
            
            if (c == '\b') { /* Backspace */
                if (cmd_index > 0) {
                    cmd_index--;
                    terminal_putchar('\b');
                }
            } else if (c == '\n') { /* Enter key */
                cmd_buffer[cmd_index] = '\0'; /* Terminate the string */
                terminal_putchar('\n');
                
                execute_command(cmd_buffer); /* Pass it to the kernel! */
                
                cmd_index = 0; /* Reset buffer for next command */
                terminal_writestring("> ");
            } else if (c != 0) { /* Standard keys */
                if (cmd_index < CMD_BUFFER_SIZE - 1) {
                    cmd_buffer[cmd_index++] = c;
                    terminal_putchar(c);
                }
            }
        }
    }

    outb(PIC1_COMMAND_PORT, 0x20);
}

void keyboard_init(void) {
    /* Register irq1_stub at vector 33 (Master PIC base 0x20 + IRQ1) */
    idt_set_gate(33, (uint32_t)irq1_stub, 0x08, 0x8E);

    /* 
     * Mask PIC Interrupt Lines:
     * 0xFD = 1111 1101 in binary.
     * Bit 0 (IRQ0 / Timer)    = 1 (Disabled)
     * Bit 1 (IRQ1 / Keyboard) = 0 (Enabled)
     */
    outb(0x21, 0xFD); /* Master PIC */
    outb(0xA1, 0xFF); /* Slave PIC (Disable all IRQ8-15) */
}