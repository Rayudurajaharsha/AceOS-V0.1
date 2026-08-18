#include "idt.h"

extern void irq0_stub(void);
extern void idt_flush(uint32_t);

struct idt_entry idt[256];
struct idt_ptr   idtp;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/* Remap PIC vectors 0x00-0x0F away from CPU exceptions to 0x20-0x2F */
static void pic_remap(void) {
    uint8_t a1 = inb(0x21);
    uint8_t a2 = inb(0xA1);

    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); /* Master PIC offset: 32 */
    outb(0xA1, 0x28); /* Slave PIC offset: 40 */
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    outb(0x21, a1);
    outb(0xA1, a2);
}

void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low  = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].sel       = sel;
    idt[num].always0   = 0;
    idt[num].flags     = flags;
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base  = (uint32_t)&idt;
    

    /* Zero out all interrupt descriptors */
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }
    
    /* Vector 32 is IRQ0 (Timer) */
    idt_set_gate(32, (uint32_t)irq0_stub, 0x08, 0x8E);

    pic_remap();
    idt_flush((uint32_t)&idtp);
}