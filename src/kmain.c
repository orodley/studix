#include "vga.h"
#include "gdt.h"

void kernel_main()
{
	init_term();
	term_puts(NAME " booting...");

	term_puts("Initializing GDT...");
	init_gdt();
}
