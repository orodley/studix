// Page table stuff

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "alloc.h"
#include "interrupt.h"
#include "kmalloc.h"
#include "page.h"
#include "panic.h"
#include "term.h"

Page_dir *curr_dir   = NULL;
Page_dir *kernel_dir = NULL;

uint32_t *frames;
uint32_t num_frames;

extern uintptr_t placement_addr;

#define BIT_INDEX(a)  (a / (8 * 4))
#define BIT_OFFSET(a) (a % (8 * 4))

static void set_frame(uint32_t frame_addr)
{
	uint32_t frame  = frame_addr / PAGE_SIZE;
	uint32_t index  = BIT_INDEX( frame);
	uint32_t offset = BIT_OFFSET(frame);

	frames[index] |= 1 << offset;
}

// Clear a bit in the bitset
static void clear_frame(uint32_t frame_addr)
{
	uint32_t frame = frame_addr / PAGE_SIZE;
	uint32_t index  = BIT_INDEX( frame);
	uint32_t offset = BIT_OFFSET(frame);

	frames[index] &= ~(1 << offset);
}

// Find the first free frame
static uint32_t first_frame()
{
	uint32_t i = 0;
	for (; i < BIT_INDEX(num_frames); i++) {
		if (frames[i] == 0xFFFFFFFF) {
			continue;
		}

		for (uint32_t j = 0; j < 32; j++) {
			if (!(frames[i] & (1 << j)))
				return i * 8 * 4 + j;
		}
	}

	return i;
}

// Allocate a new frame
void alloc_frame(Page_entry *page, bool kernel, bool writeable)
{
	if (page->frame) {
		return; // Frame is already allocated
	} else {
		uint32_t index = first_frame();

		if (index == (uint32_t)-1)
			PANIC("No free frames!");

		set_frame(index * PAGE_SIZE);
		page->present = 1;
		page->rw      = writeable ? 1 : 0;
		page->user    = kernel    ? 0 : 1;
		page->frame   = index;
	}
}

// Free an allocated frame
void free_frame(Page_entry *page)
{
	uint32_t frame = page->frame;

	if (!frame) {
		return; // Page doesn't even have a frame allocated
	} else {
		clear_frame(frame);
		page->frame = 0;
	}
}

Page_entry *get_page(uint32_t addr, bool make_table, Page_dir *dir)
{
	addr /= PAGE_SIZE;

	uint32_t index = addr / 1024;
	if (dir->tables[index]) { // Table is already assigned
		return &dir->tables[index]->pages[addr % 1024];
	} else if (make_table) {
		uint32_t phys_addr;
		dir->tables[index] =
			(Page_table*)kmalloc_ap(sizeof(Page_table), &phys_addr);
		memset(dir->tables[index], 0, PAGE_SIZE);
		dir->tables_physical[index] = phys_addr | 7; // Present, RW, user

		return &dir->tables[index]->pages[addr % 1024];
	}
	else {
		return NULL;
	}
}

void switch_page_dir(Page_dir *dir)
{
	curr_dir = dir;

	__asm__ volatile ("mov %0, %%cr3" :: "r" (&dir->tables_physical));

	uint32_t cr0;

	__asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
	cr0 |= 0x80000000; // Enable paging
	__asm__ volatile ("mov %0, %%cr0" :: "r" (cr0));
}

void page_fault_handler(Registers regs)
{
	// The faulting address is stored in the CR2 register.
	uintptr_t fault_addr;
	__asm__ volatile("mov %%cr2, %0" : "=r" (fault_addr));
	uint8_t   err = regs.err;

	// The error code gives us details of what happened.
	bool present   = !(err & 0x01); // Page not present
	bool read      =   err & 0x02;  // Write operation?
	bool user      =   err & 0x04;  // Processor was in user-mode?
	bool overwrite =   err & 0x08;  // Overwritten CPU-reserved bits?
	bool fetch     =   err & 0x10;  // Caused by an instruction fetch?

	// Output an error message.
	term_printf("Page fault at %p\n", fault_addr);
	term_printf(" %s\n caused by a %s access in %s mode\n",
		present ? "protection fault"     : "page was not found",
		read    ? "read"                 : "write",
		user    ? "user"                 : "kernel");
	term_printf(" reserved bits were %soverwritten\n",
		overwrite ? "" : "not ");
	term_printf(" %s caused by instruction fetch\n",
		fetch ? "was" : "not");

	__asm__ volatile ("xchg %bx, %bx"); // Magic breakpoint!
	PANIC("Page fault!");
}

Heap *kheap = NULL;

void init_paging()
{
	// Assume physical memory is 64MB, for now
	uint32_t phys_mem_size = 0x4000000;

	num_frames = phys_mem_size / PAGE_SIZE;
	frames     = (uint32_t*)kmalloc(BIT_INDEX(num_frames));
	memset(frames, 0, BIT_INDEX(num_frames));

	// Create page directory
	kernel_dir = (Page_dir*)kmalloc_a(sizeof(Page_dir));
	memset(kernel_dir, 0, sizeof(Page_dir));
	curr_dir   = kernel_dir;

	// Map some kernel heap pages
	for (size_t i = HEAP_START; i < HEAP_START + HEAP_INIT_SIZE;
			i += PAGE_SIZE)
		get_page(i, true, kernel_dir);

	// Allocate an extra page for the kernel heap
	for (size_t i = 0; i < placement_addr + PAGE_SIZE; i += PAGE_SIZE)
		alloc_frame(get_page(i, true, kernel_dir), false, false);

	// Allocate those pages from earlier
	for (size_t i = HEAP_START; i <HEAP_START + HEAP_INIT_SIZE;
			i += PAGE_SIZE)
		alloc_frame(get_page(i, true, kernel_dir), false, false);

	register_interrupt_handler(14, page_fault_handler);

	switch_page_dir(kernel_dir);

	kheap = create_heap(HEAP_START, HEAP_START + HEAP_INIT_SIZE,
			HEAP_START + HEAP_MAX_SIZE, false, false);
}
