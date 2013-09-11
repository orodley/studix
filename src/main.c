#include <stdint.h>
#include "assert.h"
#include "gdt.h"
#include "idt.h"
#include "initrd.h"
#include "kmalloc.h"
#include "multiboot.h"
#include "term.h"
#include "timer.h"
#include "page.h"
#include "ps2.h"

void notify(void (*func)(), char *str)
{
	term_puts(str);
	func();
}

void print_time()
{
	term_printf("[%lms] ", uptime());
}

void timer_notify(void (*func)(), char *str)
{
	print_time();
	notify(func, str);
}

size_t file_count(FS_node *root)
{
	size_t i = 0, count = 0;
	Dir_entry *dir_entry;
	while ((dir_entry = read_dir_node(root, i)) != NULL) {
		FS_node *node = find_dir_node(root, dir_entry->name);

		if (node->type == FILE_NODE)
			count++;

		i++;
	}

	return count;
}

void kernel_main(Multiboot_info *multiboot)
{
	init_term();
	term_puts(NAME " booting");

	notify(init_gdt,   "Initializing GDT");
	notify(init_idt,   "Initializing IDT");
	notify(init_timer, "Initializing PIT"); // Now we can use timer_notify()
	__asm__ volatile ("sti");	// Enable interrupts

	ASSERT(multiboot->module_count > 0);
	uintptr_t initrd_addr = *(uintptr_t*)multiboot->modules_addr;
	uintptr_t initrd_end  = *(uintptr_t*)(multiboot->modules_addr + 4);
	// Make sure the placment allocator doesn't overwrite the initial ramdisk
	extern uintptr_t placement_addr;
	placement_addr = initrd_end;

	timer_notify(init_paging, "Initializing page table");

	print_time();
	term_putsn("Loading initial ramdisk");
	FS_node *root = init_initrd(initrd_addr);
	term_printf(". Found %u file(s)\n", file_count(root));

	timer_notify(init_ps2, "Initializing PS/2 controller");

	// Allocate some memory, just for fun
	uintptr_t a = kmalloc(8);
	uintptr_t b = kmalloc(8);
	kfree((void*)b);
	kfree((void*)a);
	uintptr_t c = kmalloc(12);

	ASSERT(a == c); // a & b should have been merged
}
