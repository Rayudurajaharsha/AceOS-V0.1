CC := i686-elf-gcc
AS := i686-elf-as

ROOT := AceOS
BUILD := build

.RECIPEPREFIX := >

CFLAGS := -std=gnu99 -ffreestanding -O2 -Wall -Wextra \
  -I$(ROOT)/boot \
  -I$(ROOT)/kernel/arch/i386/gdt \
  -I$(ROOT)/kernel/arch/i386/idt \
  -I$(ROOT)/kernel/arch/i386/io \
  -I$(ROOT)/kernel/drivers/keyboard \
  -I$(ROOT)/kernel/drivers/timer \
  -I$(ROOT)/kernel/drivers/vga \
  -I$(ROOT)/kernel/memory/pmm

C_SOURCES := \
  $(ROOT)/kernel/kernel.c \
  $(ROOT)/kernel/arch/i386/gdt/gdt.c \
  $(ROOT)/kernel/arch/i386/idt/idt.c \
  $(ROOT)/kernel/drivers/keyboard/keyboard.c \
  $(ROOT)/kernel/drivers/timer/timer.c \
  $(ROOT)/kernel/memory/pmm/pmm.c

ASM_SOURCES := \
  $(ROOT)/boot/boot.s \
  $(ROOT)/kernel/arch/i386/gdt/gdt_flush.s \
  $(ROOT)/kernel/arch/i386/idt/idt_flush.s \
  $(ROOT)/kernel/arch/i386/interrupt/interrupt_stubs.s

C_OBJECTS := $(C_SOURCES:$(ROOT)/%.c=$(BUILD)/%.o)
ASM_OBJECTS := $(ASM_SOURCES:$(ROOT)/%.s=$(BUILD)/%.o)
OBJECTS := $(C_OBJECTS) $(ASM_OBJECTS)

KERNEL := $(BUILD)/myos.bin

all: $(KERNEL)

$(BUILD)/%.o: $(ROOT)/%.c
>@mkdir -p $(dir $@)
>$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: $(ROOT)/%.s
>@mkdir -p $(dir $@)
>$(AS) $< -o $@

$(KERNEL): $(OBJECTS) $(ROOT)/boot/linker.ld
>$(CC) -T $(ROOT)/boot/linker.ld -o $@ \
>  -ffreestanding -O2 -nostdlib $(OBJECTS) -lgcc

run: $(KERNEL)
>qemu-system-i386 -kernel $(KERNEL) -no-reboot -display gtk

clean:
>rm -rf $(BUILD)

.PHONY: all run clean