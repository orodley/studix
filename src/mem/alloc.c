// Simple memory allocator
// Based on kheap.c from JamesM's kernel development tutorial:
//  http://www.jamesmolloy.co.uk/tutorial_html/7.-The%20Heap.html

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "alloc.h"
#include "assert.h"
#include "kmalloc.h"
#include "page.h"
#include "term.h"

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

void print_block(Header *header, Footer *footer)
{
	term_printf("%X  <= %X =>  %X  <= %X =>  %X  <= %X =>  %X\n",
		header, sizeof(Header), (uintptr_t)header + sizeof(Header),
		header->size - sizeof(Header) - sizeof(Footer),
		footer, sizeof(Footer), (uintptr_t)header + header->size);
	term_printf("Footer says header at %p, end at %p\n",
		footer->header, (uintptr_t)footer + sizeof(Footer));
}

extern Page_dir *kernel_dir;

static int32_t find_smallest_hole(size_t size, bool align, Heap *heap)
{
	for (size_t i = 0; i < heap->index.size; i++) {
		Header *header = (Header*)lookup_ordered_array(i, &heap->index);

		if (align) {
			uintptr_t loc    = (uintptr_t)header;
			size_t    offset = 0;
			if (((loc + sizeof(Header)) & 0xFFFFF000) != 0)
				offset = PAGE_SIZE - (loc + sizeof(Header)) % PAGE_SIZE;

			if (header->size - offset >= size)
				return i;
		} else if (header->size >= size)
			return i;
	}

	return -1;
}

static int8_t header_comparer(void *a, void *b)
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

	if ((start & 0xFFFFF000) != 0) {
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
	insert_ordered_array((void*)hole, &heap->index);

	return heap;
}

static void expand(size_t new_size, Heap *heap)
{
	// Sanity check
    ASSERT(heap->start_addr + new_size > heap->end_addr);

	if ((new_size & 0xFFFFF000) != 0) {
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

static size_t contract(size_t new_size, Heap *heap)
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

void *alloc(size_t size, bool align, Heap *heap)
{
	// Account for header/footer
	size_t total_block_size = size + sizeof(Header) + sizeof(Footer);

	int32_t i = find_smallest_hole(total_block_size, align, heap);

	if (i == -1) { // Uh oh, there's no hole big enough!
		term_puts("Couldn't find big enough hole");
		uintptr_t old_end_addr = heap->end_addr;
		uintptr_t old_length   = old_end_addr - heap->start_addr;

		// Expand the heap!
		expand(old_length + total_block_size, heap);
		uintptr_t new_length = heap->end_addr - heap->start_addr;

		// Find the last header
		int32_t   header_index = -1;
		uintptr_t value        =  0;
		for (size_t i = 0; i < heap->index.size; i++) {
			uintptr_t tmp = (uintptr_t)lookup_ordered_array(i, &heap->index);

			if (tmp > value) {
				value = tmp;
				header_index++;
			}
		}

		if (header_index != -1) {
			// The last header needs adjusting
			Header *header = lookup_ordered_array(header_index, &heap->index);
			header->size  += new_length - old_length;

			make_footer((uintptr_t)header + header->size - sizeof(Footer),
				header);
		} else {
			Header *header =
				make_header(old_end_addr, new_length - old_length, true);

			make_footer(old_end_addr + header->size - sizeof(Footer), header);
			insert_ordered_array((void*)header, &heap->index);
		}

		// Now we have enough space. Try again.
		return alloc(size, align, heap);
	}

	Header *orig_hole_header = (Header*)lookup_ordered_array(i, &heap->index);
	uintptr_t orig_hole_pos  = (uintptr_t)orig_hole_header;
	size_t    orig_hole_size = orig_hole_header->size;

	// Should we split the hole in two?
	// Is the original hole size - requested size less than the
	// overhead for a new hole?
	if (orig_hole_size - total_block_size < sizeof(Header) + sizeof(Footer)) {
		size += orig_hole_size - total_block_size;
		total_block_size = orig_hole_size;
	}

	// If we need to page-align it, then do it, and create a new hole in
	// the gap
	if (align && ((orig_hole_pos + sizeof(Header)) & 0xFFFFF000) != 0) {
		uintptr_t new_hole_loc  = orig_hole_pos + PAGE_SIZE -
			(orig_hole_pos & 0xFFF) - sizeof(Header);
		Header *new_hole_header =
			make_header(orig_hole_pos, new_hole_loc - orig_hole_pos, true);

		make_footer(new_hole_loc - sizeof(Footer), new_hole_header);

		orig_hole_pos           = new_hole_loc;
		orig_hole_size         -= new_hole_header->size;
	} else {
		// We only need to remove the hole from the index if we're using it
		// as the location for our new block. In the branch above we create
		// a new, free, hole at the location we're removing here
		remove_ordered_array(i, &heap->index);
	}

	// Overwrite the old header
	Header *block_header = make_header(orig_hole_pos, total_block_size, false);

	// Set up the new footer
	// We're potentially creating a new footer here, if we ended up using less
	// space than was in the original hole
	make_footer(orig_hole_pos + total_block_size - sizeof(Footer),
		block_header);

	// If necessary, split the block in two
	if (orig_hole_size > total_block_size) {
		// The new hole sits later in memory than the first half, as we've
		// potentially need our new block to be page-aligned.
		// So we now create a new header and footer for this new block
		Header *new_hole_header = make_header(orig_hole_pos + total_block_size,
				orig_hole_size - total_block_size, true);

		Footer *new_hole_footer  = (Footer*)((uintptr_t)new_hole_header +
			new_hole_header->size - sizeof(Footer));

		if ((uintptr_t)new_hole_footer + sizeof(Footer) < heap->end_addr) {
			new_hole_footer->magic  = HEAP_MAGIC;
			new_hole_footer->header = new_hole_header;
		}

		insert_ordered_array((void*)new_hole_header, &heap->index);
	}

	// Phew!
	return (void*)((uintptr_t)block_header + sizeof(Header));
}

void free(void *ptr, Heap *heap)
{
	if (ptr == NULL)
		return;

	// Find the header and footer for the pointer
	Header *header = (Header*)((uintptr_t)ptr - sizeof(Header));
	Footer *footer = (Footer*)((uintptr_t)header + header->size - sizeof(Footer));

	// Sanity checks
	// Check the footer has a correct pointer to the header
	ASSERT(footer->header == header);
	// Check the magic numbers are present
	ASSERT(header->magic == HEAP_MAGIC);
	ASSERT(footer->magic == HEAP_MAGIC);

	header->is_hole = true;

	// Should we add the header for this pointer to the index?
	bool add_header = true;

	// Unify left
	// Check if we're immediately next to another hole on the left
	Footer *potential_footer = (Footer*)((uintptr_t)header - sizeof(Footer));
	if (potential_footer->magic == HEAP_MAGIC &&
			potential_footer->header->is_hole) {
		// Merge the two holes
		size_t curr_size = header->size;
		header           = potential_footer->header;
		footer->header   = header;
		header->size    += curr_size;

		add_header = false; // It's already in the index
	}

	// Unify right
	// Check if we're immediately next to another hole on the right
	Header *potential_header = (Header*)((uintptr_t)footer + sizeof(Footer));
	if (potential_header->magic == HEAP_MAGIC &&
			potential_header->is_hole) {
		// Merge the two holes
		header->size += potential_header->size;
		footer = (Footer*)((uintptr_t)potential_header +
			potential_header->size - sizeof(Footer));

		// Remove the header from the index
		size_t i;
		for (i = 0; i < heap->index.size; i++)
			if (lookup_ordered_array(i, &heap->index) == potential_header)
				break;

		// Make sure we actually found it
		ASSERT(i < heap->index.size);

		remove_ordered_array(i, &heap->index);
	}

	// We can contract the heap if this hole is at the end
	if ((uintptr_t)footer + sizeof(Footer) == heap->end_addr) {
		size_t old_length = heap->end_addr - heap->start_addr;
		size_t new_length = contract(((uintptr_t)header - heap->start_addr),
				heap);
		if (header->size > old_length - new_length) {
			// We'll still exist, so resize us
			header->size -= old_length - new_length;
			footer        = make_footer((uintptr_t)header + header->size -
					sizeof(Footer), header);
		} else {
			// We're not going to be around anymore
			size_t i;
			for (i = 0; i < heap->index.size; i++)
				if (lookup_ordered_array(i, &heap->index) == header)
					break;

			if (i < heap->index.size)
				remove_ordered_array(i, &heap->index);
		}
	}

	if (add_header)
		insert_ordered_array(header, &heap->index);
}
