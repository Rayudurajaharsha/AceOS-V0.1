#include <stdbool.h>
#include <stdint.h>
#include "pmm.h"

#define PMM_MAX_MEMORY_BYTES 0x100000000ULL
#define PMM_MAX_FRAMES       (PMM_MAX_MEMORY_BYTES / PMM_PAGE_SIZE)
#define PMM_BITMAP_WORDS     (PMM_MAX_FRAMES / 32u)

static uint32_t frame_bitmap[PMM_BITMAP_WORDS];
static uint32_t free_frames;

extern uint8_t kernel_start;
extern uint8_t kernel_end;

static bool frame_is_used(uint32_t frame) {
    return (frame_bitmap[frame / 32u] & (1u << (frame % 32u))) != 0;
}

static void frame_mark_used(uint32_t frame) {
    if (frame >= PMM_MAX_FRAMES || frame_is_used(frame)) {
        return;
    }

    frame_bitmap[frame / 32u] |= 1u << (frame % 32u);
    free_frames--;
}

static void frame_mark_free(uint32_t frame) {
    if (frame >= PMM_MAX_FRAMES || !frame_is_used(frame)) {
        return;
    }

    frame_bitmap[frame / 32u] &= ~(1u << (frame % 32u));
    free_frames++;
}

static void pmm_release_region(uint64_t base, uint64_t length) {
    uint64_t end = base + length;

    if (end < base || base >= PMM_MAX_MEMORY_BYTES) {
        return;
    }

    if (end > PMM_MAX_MEMORY_BYTES) {
        end = PMM_MAX_MEMORY_BYTES;
    }

    uint64_t start = (base + PMM_PAGE_SIZE - 1u) &
                     ~(uint64_t)(PMM_PAGE_SIZE - 1u);
    end &= ~(uint64_t)(PMM_PAGE_SIZE - 1u);

    for (uint64_t address = start; address < end; address += PMM_PAGE_SIZE) {
        frame_mark_free((uint32_t)(address / PMM_PAGE_SIZE));
    }
}

static void pmm_reserve_region(uint64_t base, uint64_t length) {
    uint64_t end = base + length;

    if (end < base || base >= PMM_MAX_MEMORY_BYTES) {
        return;
    }

    if (end > PMM_MAX_MEMORY_BYTES) {
        end = PMM_MAX_MEMORY_BYTES;
    }

    uint64_t start = base & ~(uint64_t)(PMM_PAGE_SIZE - 1u);
    end = (end + PMM_PAGE_SIZE - 1u) &
          ~(uint64_t)(PMM_PAGE_SIZE - 1u);

    if (end > PMM_MAX_MEMORY_BYTES) {
        end = PMM_MAX_MEMORY_BYTES;
    }

    for (uint64_t address = start; address < end; address += PMM_PAGE_SIZE) {
        frame_mark_used((uint32_t)(address / PMM_PAGE_SIZE));
    }
}

void pmm_init(const multiboot_info_t* multiboot_info) {
    for (uint32_t i = 0; i < PMM_BITMAP_WORDS; i++) {
        frame_bitmap[i] = 0xFFFFFFFFu;
    }

    free_frames = 0;

    uint32_t current = multiboot_info->mmap_addr;
    uint32_t end = current + multiboot_info->mmap_length;

    while (current < end) {
        const multiboot_mmap_entry_t* entry =
            (const multiboot_mmap_entry_t*)(uintptr_t)current;

        if (entry->size < 20u) {
            break;
        }

        if (entry->type == 1u) {
            pmm_release_region(entry->addr, entry->len);
        }

        uint32_t next = current + entry->size + sizeof(entry->size);

        if (next <= current) {
            break;
        }

        current = next;
    }

    /* Never allocate BIOS, low memory, or the VGA framebuffer. */
    pmm_reserve_region(0, 0x100000u);

    /* Reserve every physical page occupied by AceOS, including the bitmap. */
    pmm_reserve_region(
        (uint32_t)(uintptr_t)&kernel_start,
        (uint32_t)((uintptr_t)&kernel_end - (uintptr_t)&kernel_start)
    );

    /* Keep the Multiboot data available and untouched. */
    pmm_reserve_region(
        (uint32_t)(uintptr_t)multiboot_info,
        PMM_PAGE_SIZE
    );

    pmm_reserve_region(
        multiboot_info->mmap_addr,
        multiboot_info->mmap_length
    );
}

uint32_t pmm_alloc_page(void) {
    for (uint32_t word = 0; word < PMM_BITMAP_WORDS; word++) {
        if (frame_bitmap[word] == 0xFFFFFFFFu) {
            continue;
        }

        for (uint32_t bit = 0; bit < 32u; bit++) {
            uint32_t frame = word * 32u + bit;

            if (!frame_is_used(frame)) {
                frame_mark_used(frame);
                return frame * PMM_PAGE_SIZE;
            }
        }
    }

    return 0;
}

void pmm_free_page(uint32_t physical_address) {
    if (physical_address == 0 ||
        (physical_address % PMM_PAGE_SIZE) != 0) {
        return;
    }

    frame_mark_free(physical_address / PMM_PAGE_SIZE);
}

uint32_t pmm_free_page_count(void) {
    return free_frames;
}