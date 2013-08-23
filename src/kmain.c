#include "vga.h"

void kernel_main()
{
	init_term();
	term_puts(NAME " booting...");
}
