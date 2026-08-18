#include <stdint.h>
#include "io.h"

/* I/O port communication functions from your keyboard/VGA setup */
extern void outb(uint16_t port, uint8_t data);
extern uint8_t inb(uint16_t port);
extern void terminal_writestring(const char* data);

/* PIT Ports */
#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40
#define PIC1_COMMAND_PORT 0x20

volatile uint32_t timer_ticks = 0;

/* This function will run 100 times every second */
void timer_handler(void) {
    timer_ticks++;

    /* Update top-right corner of screen every 100 ticks (1 second) */
    if (timer_ticks % 100 == 0) {
        volatile uint16_t* vga = (volatile uint16_t*)0xB8000;
        uint32_t seconds = timer_ticks / 100;
        
        /* Display 'T: [seconds]' at row 0, column 72 */
        vga[72] = (0x0F << 8) | 'T';
        vga[73] = (0x0F << 8) | ':';
        vga[74] = (0x0F << 8) | ('0' + (seconds % 10));
    }

    outb(PIC1_COMMAND_PORT, 0x20);
}

void timer_init(uint32_t frequency) {
    /* The hardware clock runs at 1193180 Hz */
    uint32_t divisor = 1193180 / frequency;

    /* Send the command byte (0x36) to set repeating mode */
    outb(PIT_COMMAND_PORT, 0x36);

    /* Send the frequency divisor (low byte, then high byte) */
    outb(PIT_CHANNEL0_PORT, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_PORT, (uint8_t)((divisor >> 8) & 0xFF));
}
