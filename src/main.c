#include "gdt.h"
#include "idt.h"
#include "timer.h"
#include "ps2.h"
#include "term.h"

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
	timer_notify(init_ps2,   "Initializing PS/2 controller...");
	__asm__ volatile ("sti");	// Enable interrupts

	term_printf("term_printf is %d%% p%cre %s\n", 100, 'u', "awesome");
}
