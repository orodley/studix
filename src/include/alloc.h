#include <stdbool.h>
#include <stddef.h>
#include "ordered_array.h"

typedef struct Heap
{
	Ordered_array index;      // Index of block headers
	uintptr_t     start_addr; // Start of allocated space
	uintptr_t     end_addr;   // End of allocated space
	uintptr_t     max_addr;   // Maximum value of end_addr
	bool          supervisor; // Should extra pages be supervisor-only?
	bool          readonly;   // Should extra pages be read-only
} Heap;

Heap *create_heap(uintptr_t start, uintptr_t end, uintptr_t max,
		bool supervisor, bool readonly);
void *alloc(size_t size, bool page_align, Heap *heap);
void free(void *ptr, Heap *heap);

#define HEAP_START      0xC0000000
#define HEAP_INIT_SIZE    0x100000
#define HEAP_MIN_SIZE      0x70000
#define HEAP_MAX_SIZE    0xFFFF000
#define HEAP_INDEX_SIZE    0x20000
#define HEAP_MAGIC      0xBEEFACED
