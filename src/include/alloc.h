#include <stddef.h>
#include "heap.h"

void *alloc(size_t size, bool page_align, Heap *heap);
void free(void *ptr, Heap *heap);
