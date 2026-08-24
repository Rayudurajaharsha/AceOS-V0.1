#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"

extern void timer_init(uint32_t frequency);

/* --- VGA Setup & Constants --- */
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEMORY ((uint16_t*) 0xB8000)

enum vga_color {
    VGA_COLOR_BLACK = 0,
    VGA_COLOR_BLUE = 1,
    VGA_COLOR_GREEN = 2,
    VGA_COLOR_CYAN = 3,
    VGA_COLOR_RED = 4,
    VGA_COLOR_MAGENTA = 5,
    VGA_COLOR_BROWN = 6,
    VGA_COLOR_LIGHT_GREY = 7,
    VGA_COLOR_DARK_GREY = 8,
    VGA_COLOR_LIGHT_BLUE = 9,
    VGA_COLOR_LIGHT_GREEN = 10,
    VGA_COLOR_LIGHT_CYAN = 11,
    VGA_COLOR_LIGHT_RED = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_LIGHT_BROWN = 14,
    VGA_COLOR_WHITE = 15,
};

/* Terminal State Variables */
static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

/* --- STEP 1: Low-Level Assembly Port I/O --- */
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

/* --- STEP 2: Hardware Cursor Management --- */
static void update_cursor(int x, int y) {
    uint16_t pos = y * VGA_WIDTH + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

/* Helper function to pair character and color byte */
static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

/* --- STEP 3: Terminal Initialization --- */
void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = VGA_COLOR_LIGHT_GREEN | (VGA_COLOR_BLACK << 4);
    terminal_buffer = VGA_MEMORY;

    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
    update_cursor(0, 0);
}

/* --- STEP 4: Scrolling Mechanism --- */
static void terminal_scroll(void) {
    /* Copy every row up by one position */
    for (size_t y = 0; y < VGA_HEIGHT - 1; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            terminal_buffer[y * VGA_WIDTH + x] = terminal_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }

    /* Clear the bottom row with empty spaces */
    for (size_t x = 0; x < VGA_WIDTH; x++) {
        terminal_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }

    terminal_row = VGA_HEIGHT - 1;
}

/* --- Custom String Functions --- */
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

/* --- STEP 5: Printing & Character Handling --- */
void terminal_putchar(char c) {
    if (c == '\b') { /* Handle Backspace */
        if (terminal_column > 0) {
            terminal_column--;
        } else if (terminal_row > 0) {
            terminal_row--;
            terminal_column = VGA_WIDTH - 1;
        }
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(' ', terminal_color);
    } else if (c == '\n') { /* Handle Newline */
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_scroll();
        }
    } else { /* Standard Character */
        const size_t index = terminal_row * VGA_WIDTH + terminal_column;
        terminal_buffer[index] = vga_entry(c, terminal_color);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                terminal_scroll();
            }
        }
    }
    update_cursor(terminal_column, terminal_row);
}

void terminal_writestring(const char* data) {
    for (size_t i = 0; data[i] != '\0'; i++) {
        terminal_putchar(data[i]);
    }
}

/* --- Command Execution Engine --- */
void execute_command(char* input) {
    if (input[0] == '\0') {
        return; /* Do nothing on empty enter */
    }

    if (strcmp(input, "help") == 0) {
        terminal_writestring("Available commands:\n");
        terminal_writestring("  help  - Show this message\n");
        terminal_writestring("  clear - Clear the screen\n");
        terminal_writestring("  info  - Show system information\n");
    } 
    else if (strcmp(input, "clear") == 0) {
        terminal_initialize(); /* Reusing init to clear screen */
    } 
    else if (strcmp(input, "info") == 0) {
        terminal_writestring("AI-Quantum Core OS v0.1\n");
        terminal_writestring("Kernel Architecture: x86 32-bit (i686)\n");
    } 
    else {
        terminal_writestring("Unknown command: ");
        terminal_writestring(input);
        terminal_writestring("\n");
    }
}

/* --- STEP 6: Kernel Entry Point --- */
void kernel_main(void) {
    gdt_init();
    idt_init();
    keyboard_init();
    
    /* Initialize the timer at 100 Hz */
    timer_init(100);

    /* Unmask BOTH Keyboard (IRQ1) and Timer (IRQ0) */
    outb(0x21, 0xFC); 

    /* Enable Hardware Interrupts */
    asm volatile("sti");

    terminal_initialize();
    terminal_writestring("AI-Quantum OS Kernel Online.\n");
    terminal_writestring("System Clock Initialized (100Hz).\n");
    terminal_writestring("Type 'help' to see available commands.\n\n> ");

    while (1) {
        asm volatile("hlt");
    }
}