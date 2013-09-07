// Simple memory allocator
// Based on kheap.c from JamesM's kernel development tutorial:
//  http://www.jamesmolloy.co.uk/tutorial_html/7.-The%20Heap.html

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "assert.h"
#include "heap.h"
#include "page.h"

static void make_space(Heap *heap, size_t needed_size)
{
	uintptr_t old_end_addr = heap->end_addr;
	uintptr_t old_length   = old_end_addr - heap->start_addr;

	// Expand the heap!
	expand(heap, old_length + needed_size);
	uintptr_t new_length = heap->end_addr - heap->start_addr;

	// Find the last header
	int32_t   header_index = -1;
	uintptr_t value        =  0;
	for (size_t i = 0; i < heap->index.size; i++) {
		uintptr_t tmp = (uintptr_t)ordered_array_lookup(&heap->index, i);

		if (tmp > value) {
			value = tmp;
			header_index++;
		}
	}

	if (header_index != -1) {
		// The last header needs adjusting
		Header *header = ordered_array_lookup(&heap->index, header_index);
		header->size  += new_length - old_length;

		make_footer((uintptr_t)header + header->size - sizeof(Footer),
				header);
	} else {
		Header *header =
			make_header(old_end_addr, new_length - old_length, true);

		make_footer(old_end_addr + header->size - sizeof(Footer), header);
		ordered_array_insert(&heap->index, (void*)header);
	}
}

void *alloc(size_t size, bool align, Heap *heap)
{
	// Account for header/footer
	size_t total_block_size = size + sizeof(Header) + sizeof(Footer);
	int32_t i = find_smallest_hole(total_block_size, align, heap);

	if (i == -1) { // Uh oh, there's no hole big enough!
		make_space(heap, total_block_size);

		// Now we have enough space. Try again.
		return alloc(size, align, heap);
	}

	Header *orig_hole_header = (Header*)ordered_array_lookup(&heap->index, i);
	uintptr_t orig_hole_pos  = (uintptr_t)orig_hole_header;
	size_t    orig_hole_size = orig_hole_header->size;

	// Don't split the hole if the other half wouldn't be big enough
	if (orig_hole_size - total_block_size < sizeof(Header) + sizeof(Footer)) {
		size += orig_hole_size - total_block_size;
		total_block_size = orig_hole_size;
	}

	// If we need to page-align it, then do, and create a new hole in the gap
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
		ordered_array_remove(&heap->index, i);
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
		// potentially already page-aligned our block
		// So we now create a new header and footer for this new block
		Header *new_hole_header = make_header(orig_hole_pos + total_block_size,
				orig_hole_size - total_block_size, true);

		Footer *new_hole_footer  = (Footer*)((uintptr_t)new_hole_header +
			new_hole_header->size - sizeof(Footer));

		if ((uintptr_t)new_hole_footer + sizeof(Footer) < heap->end_addr) {
			new_hole_footer->magic  = HEAP_MAGIC;
			new_hole_footer->header = new_hole_header;
		}

		ordered_array_insert(&heap->index, (void*)new_hole_header);
	}

	// Phew!
	return (void*)((uintptr_t)block_header + sizeof(Header));
}

Header *unify_left(Header *header, Footer *footer)
{
	// Check if we're immediately next to another footer on the right
	Footer *potential_footer = (Footer*)((uintptr_t)header - sizeof(Header));
	if (potential_footer->magic == HEAP_MAGIC &&
			potential_footer->header->is_hole) {
		size_t curr_size = header->size;
		header           = potential_footer->header;
		footer->header   = header;
		header->size    += curr_size;
	}

	return header;
}

Footer *unify_right(Header *header, Footer *footer, Heap *heap)
{
	// Check if we're immediately next to another header on the right
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
			if (ordered_array_lookup(&heap->index, i) == potential_header)
				break;

		// Make sure we actually found it
		ASSERT(i < heap->index.size);

		ordered_array_remove(&heap->index, i);
	}

	return footer;
}

void free(void *ptr, Heap *heap)
{
	if (ptr == NULL)
		return;

	// Find the header and footer for the pointer
	Header *orig_header = (Header*)((uintptr_t)ptr - sizeof(Header));
	Header *header      = orig_header;
	Footer *footer      = (Footer*)((uintptr_t)header + header->size -
			sizeof(Footer));

	// Sanity checks
	// Check the footer has a correct pointer to the header
	ASSERT(footer->header == header);
	// Check the magic numbers are present
	ASSERT(header->magic == HEAP_MAGIC);
	ASSERT(footer->magic == HEAP_MAGIC);

	header->is_hole = true;
	header = unify_left( header, footer);
	footer = unify_right(header, footer, heap);

	// We can contract the heap if this hole is at the end
	if ((uintptr_t)footer + sizeof(Footer) == heap->end_addr) {
		size_t old_length = heap->end_addr - heap->start_addr;
		size_t new_length = contract(heap,
				(uintptr_t)header - heap->start_addr);
		if (header->size > old_length - new_length) {
			// We'll still exist, so resize us
			header->size -= old_length - new_length;
			footer        = make_footer((uintptr_t)header + header->size -
					sizeof(Footer), header);
		} else {
			// We're not going to be around anymore
			size_t i;
			for (i = 0; i < heap->index.size; i++)
				if (ordered_array_lookup(&heap->index, i) == header)
					break;

			if (i < heap->index.size)
				ordered_array_remove(&heap->index, i);
		}
	}

	// If our header is still the same as at the start, we need to add it
	// to the free hole index
	if (header == orig_header)
		ordered_array_insert(&heap->index, header);
}
