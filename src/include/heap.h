#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "ordered_array.h"

typedef struct Header
{
	uint32_t magic;   // Magic number, for ID'ing
	bool     is_hole; // Is this a hole?
	size_t   size;    // Size of allocated memory, including header & footer
} Header;

typedef struct Footer
{
	uint32_t magic;   // Same magic number as above
	Header  *header;  // Pointer to block header
} Footer;

void print_block(Header *header, Footer *footer);

typedef struct Heap
{
	Ordered_array index;      // Index of block headers
	uintptr_t     start_addr; // Start of allocated space
	uintptr_t     end_addr;   // End of allocated space
	uintptr_t     max_addr;   // Maximum value of end_addr
	bool          supervisor; // Should extra pages be supervisor-only?
	bool          readonly;   // Should extra pages be read-only
} Heap;

Heap   *create_heap(uintptr_t start, uintptr_t end, uintptr_t max,
	bool supervisor, bool readonly);
int32_t find_smallest_hole(Heap *heap, size_t size, bool align);

void   expand(  Heap *heap, size_t new_size);
size_t contract(Heap *heap, size_t new_size);

int8_t  header_comparer(void *a, void *b);
Header *make_header(uintptr_t loc, size_t size, bool is_hole);
Footer *make_footer(uintptr_t loc, Header *assoc_header);

#define HEAP_START      0xC0000000
#define HEAP_INIT_SIZE    0x100000
#define HEAP_MIN_SIZE      0x70000
#define HEAP_MAX_SIZE    0xFFFF000
#define HEAP_INDEX_SIZE    0x20000
#define HEAP_MAGIC      0xBEEFACED
