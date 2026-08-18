#ifndef IDT_H
#define IDT_H

#include <stdint.h>

struct idt_entry {
    uint16_t base_low;  /* Lower 16 bits of handler function address */
    uint16_t sel;       /* Kernel Code Segment Selector (0x08) */
    uint8_t  always0;   /* Must always be 0 */
    uint8_t  flags;     /* Access flags (Type, Ring level, Present bit) */
    uint16_t base_high; /* Upper 16 bits of handler function address */
} __attribute__((packed));

struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

void idt_init(void);
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);

#endif