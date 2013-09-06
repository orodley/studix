#include <stdint.h>
#include <stddef.h>

uintptr_t kmalloc(   size_t size);
uintptr_t kmalloc_a( size_t size);
uintptr_t kmalloc_p( size_t size, uint32_t *phys);
uintptr_t kmalloc_ap(size_t size, uint32_t *phys);

void kfree(void *ptr);
