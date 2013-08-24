#include "gdt.h"
#include "idt.h"
#include "vga.h"
#include "term_printf.h"

void kernel_main()
{
	init_term();
	term_puts(NAME " booting...");

	term_puts("Initializing GDT...");
	init_gdt();
	term_puts("Initializing IDT...");
	init_idt();

	term_printf("%d%% p%cre %s", 100, 'u', "awesome");
}
