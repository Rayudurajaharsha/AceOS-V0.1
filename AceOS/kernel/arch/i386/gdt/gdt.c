#include "gdt.h"

extern void gdt_flush(uint32_t);

struct gdt_entry gdt[5];
struct gdt_ptr   gp;

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * 5) - 1;
    gp.base  = (uint32_t)&gdt;

    /* 0x00: Null segment */
    gdt_set_gate(0, 0, 0, 0, 0);
    /* 0x08: Kernel Code segment (Ring 0, Executable, Read/Write) */
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    /* 0x10: Kernel Data segment (Ring 0, Read/Write) */
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    /* 0x18: User Code segment (Ring 3, Executable, Read/Write) */
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    /* 0x20: User Data segment (Ring 3, Read/Write) */
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_flush((uint32_t)&gp);
}