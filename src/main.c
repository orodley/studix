#include <stdint.h>
#include "kmalloc.h"
#include "assert.h"
#include "gdt.h"
#include "idt.h"
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
	term_printf("[%d] ", (int)uptime());
	notify(func, str);
}

void kernel_main()
{
	init_term();
	term_puts(NAME " booting...");

	notify(init_gdt,   "Initializing GDT...");
	notify(init_idt,   "Initializing IDT...");
	notify(init_timer, "Initializing PIT...");

	// Now we can use timer_notify()!
	timer_notify(init_ps2,    "Initializing PS/2 controller...");
	timer_notify(init_paging, "Initializing page table...");

	__asm__ volatile ("sti");	// Enable interrupts

	term_printf("term_printf is %d%% p%cre %s\n", 100, 'u', "awesome");

	// Allocate some memory, just for fun
	uintptr_t a = kmalloc(8);
	uintptr_t b = kmalloc(8);
	term_printf("a: %p, b = %p", a, b);

	kfree((void*)b);
	kfree((void*)a);
	uintptr_t c = kmalloc(12);
	term_printf(", c: %p\n", c);
}
