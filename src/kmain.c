#include "gdt.h"
#include "idt.h"
#include "vga.h"

void kernel_main()
{
	init_term();
	term_puts(NAME " booting...");

	term_puts("Initializing GDT...");
	init_gdt();
	term_puts("Initializing IDT...");
	init_idt();

	// Test out ISR functionality
	__asm__ volatile ("int $3");
}
