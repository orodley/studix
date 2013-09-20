#include <stdint.h>
#include <stddef.h>

void *kmalloc(   size_t size);
void *kmalloc_a( size_t size);
void *kmalloc_p( size_t size, uint32_t *phys);
void *kmalloc_ap(size_t size, uint32_t *phys);

void kfree(void *ptr);
