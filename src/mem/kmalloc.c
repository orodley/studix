// Memory allocator
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// We need to allocate some memory before we even have virtual
// memory set up, so we use a dumb placement allocator at first

extern uint32_t end;
uintptr_t placement_addr = (uintptr_t)&end;

static uintptr_t kmalloc_aux(size_t size, bool align,
		uint32_t *phys)
{
	// If we need a page-aligned address and its not aligned...
	if (align && (placement_addr & 0xFFFFF000)) {
		// ...align it
		placement_addr &= 0xFFFFF000;
		placement_addr += 0x1000;
	}

	if (phys)
		*phys = placement_addr;

	uintptr_t temp = placement_addr;
	placement_addr += size;
	return temp;
}

uintptr_t kmalloc_a(size_t size)
{
	return kmalloc_aux(size, true, NULL);
}

uintptr_t kmalloc_p(size_t size,  uint32_t *phys)
{
	return kmalloc_aux(size, false, phys);
}

uintptr_t kmalloc_ap(size_t size, uint32_t *phys)
{
	return kmalloc_aux(size, true, phys);
}

uintptr_t kmalloc(size_t size)
{
	return kmalloc_aux(size, false, NULL);
}
