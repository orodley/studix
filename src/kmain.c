#include "gdt.h"
#include "idt.h"
#include "term_printf.h"
#include "timer.h"
#include "vga.h"

void notify(void (*func)(), char *str)
{
	term_putsn(str);
	func();
	term_puts(" done");
}

void kernel_main()
{
	init_term();
	term_puts(NAME " booting...");

	notify(init_gdt, "Initializing GDT...");
	notify(init_idt, "Initializing IDT...");

	term_putsn("Initializing PIT...");
	init_timer(50);
	term_puts(" done");

	term_printf("term_printf is %d%% p%cre %s\n", 100, 'u', "awesome");

	__asm__ volatile ("int $2");
}
