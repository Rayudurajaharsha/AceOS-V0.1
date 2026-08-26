#ifndef ACEOS_PMM_H
#define ACEOS_PMM_H

#include <stdint.h>
#include "multiboot.h"

#define PMM_PAGE_SIZE 4096u

void pmm_init(const multiboot_info_t* multiboot_info);

uint32_t pmm_alloc_page(void);
void pmm_free_page(uint32_t physical_address);

uint32_t pmm_free_page_count(void);

#endif