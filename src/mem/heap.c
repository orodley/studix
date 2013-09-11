// Heap data structure used in the memory allocator
// Based on kheap.c from JamesM's kernel development tutorial:
//  http://www.jamesmolloy.co.uk/tutorial_html/7.-The%20Heap.html

#include <stdint.h>
#include <stddef.h>
#include "assert.h"
#include "kmalloc.h"
#include "heap.h"
#include "page.h"
#include "term.h"

extern Page_dir *kernel_dir;

void print_block(Header *header, Footer *footer)
{
	term_printf("%X  <= %X =>  %X  <= %X =>  %X  <= %X =>  %X\n",
		header, sizeof(Header), (uintptr_t)header + sizeof(Header),
		header->size - sizeof(Header) - sizeof(Footer),
		footer, sizeof(Footer), (uintptr_t)header + header->size);
	term_printf("Footer says header at %p, end at %p\n",
		footer->header, (uintptr_t)footer + sizeof(Footer));
}

int32_t find_smallest_hole(Heap *heap, size_t size, bool align)
{
	for (size_t i = 0; i < heap->index.size; i++) {
		Header *header = (Header*)ordered_array_lookup(&heap->index, i);

		if (align) {
			uintptr_t loc    = (uintptr_t)header;
			size_t    offset = 0;
			if (((loc + sizeof(Header)) & 0xFFF) != 0)
				offset = PAGE_SIZE - (loc + sizeof(Header)) % PAGE_SIZE;

			if (header->size - offset >= size)
				return i;
		} else if (header->size >= size)
			return i;
	}

	return -1;
}

int8_t header_comparer(void *a, void *b)
{
	return ((Header*)a)->size < ((Header*)b)->size ? 1 : 0;
}

Header *make_header(uintptr_t loc, size_t size, bool is_hole)
{
	Header *header  = (Header*)loc;
	header->size    = size;
	header->magic   = HEAP_MAGIC;
	header->is_hole = is_hole;

	return header;
}

Footer *make_footer(uintptr_t loc, Header *assoc_header)
{
	Footer *footer = (Footer*)loc;
	footer->header = assoc_header;
	footer->magic  = HEAP_MAGIC;

	return footer;
}

Heap *create_heap(uintptr_t start, uintptr_t end, uintptr_t max,
		bool supervisor, bool readonly)
{
	Heap *heap  = (Heap*)kmalloc(sizeof(Heap));

	// Make sure start and end are page-aligned
	ASSERT(start % 0x1000 == 0);
	ASSERT(end   % 0x1000 == 0);

	heap->index = place_ordered_array((void*)start, HEAP_INDEX_SIZE,
			header_comparer);
	start      += sizeof(void*) * HEAP_INDEX_SIZE;

	if ((start & 0xFFF) != 0) {
		start &= 0xFFFFF000;
		start += PAGE_SIZE;
	}

	heap->start_addr = start;
	heap->end_addr   = end;
	heap->max_addr   = max;
	heap->supervisor = supervisor;
	heap->readonly   = readonly;

	// At the start, the whole thing is a hole
	Header *hole = make_header(start, end - start, true);
	ordered_array_insert(&heap->index, (void*)hole);

	return heap;
}

void expand(Heap *heap, size_t new_size)
{
	// Sanity check
	ASSERT(heap->start_addr + new_size > heap->end_addr);

	if ((new_size & 0xFFF) != 0) {
		new_size &= 0xFFFFF000;
		new_size += PAGE_SIZE;
	}

	// We don't want to expand over the max address
	ASSERT(heap->start_addr + new_size <= heap->max_addr);

	size_t old_size = heap->end_addr - heap->start_addr;

	for (size_t i = old_size; i < new_size; i += PAGE_SIZE)
		alloc_frame(get_page(heap->start_addr + i, 1, kernel_dir),
				heap->supervisor, heap->readonly);

	heap->end_addr = heap->start_addr + new_size;
}

size_t contract(Heap *heap, size_t new_size)
{
	// Sanity check.
	ASSERT(heap->start_addr + new_size < heap->end_addr);

	if ((new_size & PAGE_SIZE) != 0) {
		new_size &= PAGE_SIZE;
		new_size += PAGE_SIZE;
	}
	if (new_size < HEAP_MIN_SIZE)
		new_size = HEAP_MIN_SIZE;

	size_t old_size = heap->end_addr - heap->start_addr;
	for (size_t i = old_size - PAGE_SIZE; i > new_size; i -= PAGE_SIZE)
		free_frame(get_page(heap->start_addr + i, 0, kernel_dir));

	heap->end_addr = heap->start_addr + new_size;
	return new_size;
}

Footer *assoc_footer(Header *header)
{
	return (Footer*)((uintptr_t)header + header->size - sizeof(Footer));
}

Header *header_for(uintptr_t ptr)
{
	return (Header*)(ptr - sizeof(Header));
}

int32_t find_header_index(Heap *heap, Header *header)
{
	for (size_t i = 0; i < heap->index.size; i++)
		if (ordered_array_lookup(&heap->index, i) == header)
			return i;

	return -1;
}
