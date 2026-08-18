CC = i686-elf-gcc
AS = i686-elf-as
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra

all: myos.bin

boot.o: boot.s
	$(AS) boot.s -o boot.o

gdt_flush.o: gdt_flush.s
	$(AS) gdt_flush.s -o gdt_flush.o

idt_flush.o: idt_flush.s
	$(AS) idt_flush.s -o idt_flush.o

interrupt_stubs.o: interrupt_stubs.s
	$(AS) interrupt_stubs.s -o interrupt_stubs.o

gdt.o: gdt.c
	$(CC) -c gdt.c -o gdt.o $(CFLAGS)

idt.o: idt.c
	$(CC) -c idt.c -o idt.o $(CFLAGS)

keyboard.o: keyboard.c
	$(CC) -c keyboard.c -o keyboard.o $(CFLAGS)

timer.o: timer.c
	$(CC) -c timer.c -o timer.o $(CFLAGS)

kernel.o: kernel.c
	$(CC) -c kernel.c -o kernel.o $(CFLAGS)

myos.bin: boot.o gdt_flush.o gdt.o idt_flush.o idt.o interrupt_stubs.o keyboard.o timer.o kernel.o linker.ld
	$(CC) -T linker.ld -o myos.bin -ffreestanding -O2 -nostdlib boot.o gdt_flush.o gdt.o idt_flush.o idt.o interrupt_stubs.o keyboard.o timer.o kernel.o -lgcc

run: myos.bin
	qemu-system-i386 -kernel myos.bin -no-reboot -d cpu_reset

clean:
	rm -f *.o myos.bin