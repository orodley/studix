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
	term_putsn(str);
	func();
	term_puts(" done");
}

void timer_notify(void (*func)(), char *str)
{
	// TODO: Add long printing so this actually works...
	term_printf("[%lms] ", uptime());
	notify(func, str);
}

void traverse_fs(FS_node *root)
{
	size_t i = 0;
	Dir_entry *dir_entry;
	while ((dir_entry = read_dir_node(root, i)) != NULL) {
		FS_node *node = find_dir_node(root, dir_entry->name);

		if (node->type == FILE_NODE) {
			char buf[256];
			size_t c = read_fs_node(node, 0, 256, buf);

			term_printf("Found file: %s\n Contents = '", node->name);
			for (size_t j = 0; j < c; j++)
				term_putchar(buf[j]);
			term_puts("'");
		}

		i++;
	}
}

void kernel_main(Multiboot_info *multiboot)
{
	init_term();
	term_puts(NAME " booting...");

	notify(init_gdt,   "Initializing GDT...");
	notify(init_idt,   "Initializing IDT...");
	notify(init_timer, "Initializing PIT..."); // Now we can use timer_notify()
	__asm__ volatile ("sti");	// Enable interrupts

	ASSERT(multiboot->module_count > 0);
	uintptr_t initrd_addr = *(uintptr_t*)multiboot->modules_addr;
	uintptr_t initrd_end  = *(uintptr_t*)(multiboot->modules_addr + 4);
	// Make sure the placment allocator doesn't overwrite the initial ramdisk
	extern uintptr_t placement_addr;
	placement_addr = initrd_end;

	timer_notify(init_paging, "Initializing page table...");

	term_putsn("Loading initial ramdisk...");
	FS_node *root = init_initrd(initrd_addr);
	term_puts(" done");

	traverse_fs(root);

	timer_notify(init_ps2,    "Initializing PS/2 controller...");


	// Allocate some memory, just for fun
	uintptr_t a = kmalloc(8);
	uintptr_t b = kmalloc(8);
	term_printf("a: %p, b = %p", a, b);

	kfree((void*)b);
	kfree((void*)a);
	uintptr_t c = kmalloc(12);
	term_printf(", c: %p\n", c);
}
