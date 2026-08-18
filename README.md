# AI-Quantum Core OS (ACE-OS)

A low-level, x86 32-bit bare-metal operating system kernel built from scratch in C and Assembly. **AI-Quantum Core OS** is designed as a modular kernel architecture prioritizing deterministic hardware control, low-overhead interrupt processing, and interactive execution—serving as a baseline environment for future lightweight AI runtime abstractions and quantum algorithm simulation tools.

---

## Current Kernel Capabilities

* **Multiboot Standard Compliance:** Bootstrapped via GRUB/Multiboot specification for bare-metal and emulated execution.
* **Memory Segmentation & Gate Abstraction:** Custom Global Descriptor Table (**GDT**) and Interrupt Descriptor Table (**IDT**) configured to handle hardware traps and software gates.
* **Interrupt Management (PIC 8259):** Remapped dual PIC vectors (`0x20–0x2F`) with customized hardware bitmasking to isolate device IRQs safely.
* **System Clock & PIT Driver:** IRQ0 hardware timer running at 100 Hz providing precise system ticks and uptime tracking.
* **PS/2 Keyboard Driver:** Scancode decoding engine featuring input command buffering, backspace character erasing, and real-time keystroke processing.
* **VGA Text Mode Driver:** Direct hardware text buffer manipulation (`0xB8000`) with auto-scrolling, color attributes, and register-level cursor positioning (`0x3D4`/`0x3D5`).
* **Interactive CLI Shell:** Command execution environment featuring character parsing, error handling, and core utilities (`help`, `info`, `clear`, `uptime`).

---

## Technical Stack & Toolchain

| Component | Specification |
| :--- | :--- |
| **Target Architecture** | x86 (32-bit `i686`) |
| **Cross-Toolchain** | Custom `i686-elf-gcc`, `i686-elf-as`, GNU Binutils |
| **Build Management** | GNU `Makefile` |
| **Emulation Platform** | QEMU (`qemu-system-i386`) |
| **Primary Languages** | C99 (Freestanding), x86 Assembly (NASM/GAS) |

---

## Project Vision & Architecture Roadmap

### Phase 1: Memory Management Foundations
* **Physical Memory Manager (PMM):** Parse Multiboot memory maps to implement a 4KB page frame bitmap allocator.
* **Virtual Memory Manager (VMM):** Configure 2-level x86 paging and map a higher-half kernel architecture (`0xC0000000`).
* **Heap Allocation:** Implement dynamic kernel heap management (`kmalloc` / `kfree`).

### Phase 2: Tasking & Storage Layer
* **Multitasking Scheduler:** Implement context switching and preemptive round-robin thread scheduling.
* **Userland Separation:** Establish Ring 0 (Kernel) vs. Ring 3 (User Space) boundaries with system call gates (`int 0x80`).
* **Virtual File System (VFS):** Abstract storage with an initial ramdisk (`initrd`) or standard file system format.

### Phase 3: Extended Compute & AI/Quantum Runtimes
* **Kernel Space AI Runtime:** Build a lightweight, bare-metal fixed-point tensor math engine without standard C library dependencies.
* **Quantum Simulator Abstraction:** Implement a CLI-accessible quantum state-vector simulation engine for algorithm prototyping at the OS level.

---

## Build & Run Instructions

```bash
# 1. Export cross-compiler path
export PATH="$HOME/opt/cross/bin:$PATH"

# 2. Compile kernel and launch in QEMU
make run

# 3. Clean object files and binaries
make clean
