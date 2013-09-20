// Memory allocator
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "alloc.h"
#include "term.h"
#include "page.h"

// We need to allocate some memory before we even have virtual
// memory set up, so we use a dumb placement allocator at first

extern uint32_t  end;
extern Heap     *kheap;
extern Page_dir *kernel_dir;

uintptr_t placement_addr = (uintptr_t)&end;

static void *kmalloc_aux(size_t size, bool align,
		uint32_t *phys)
{
	if (kheap != NULL) {
		void *addr = alloc(kheap, size, align);

		if (phys) {
			Page_entry *page = get_page((uintptr_t)addr, false, kernel_dir);
			*phys = page->frame * PAGE_SIZE + ((uintptr_t)addr & 0xFFF);
		}

		return addr;
	}
	// If we need a page-aligned address and its not aligned...
	if (align)
		placement_addr = align_up(placement_addr);

	if (phys)
		*phys = placement_addr;

	uintptr_t temp = placement_addr;
	placement_addr += size;

	return (void*)temp;
}

void *kmalloc(size_t size)
{
	return kmalloc_aux(size, false, NULL);
}

void *kmalloc_a(size_t size)
{
	return kmalloc_aux(size, true, NULL);
}

void *kmalloc_p(size_t size,  uint32_t *phys)
{
	return kmalloc_aux(size, false, phys);
}

void *kmalloc_ap(size_t size, uint32_t *phys)
{
	return kmalloc_aux(size, true, phys);
}

void kfree(void *ptr)
{
	free(kheap, ptr);
}
