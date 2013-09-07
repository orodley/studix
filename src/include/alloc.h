#include <stddef.h>
#include "heap.h"

void *alloc(Heap *heap, size_t size, bool page_align);
void free(Heap *heap, void *ptr);
