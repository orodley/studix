// AAAARRRGGHHH!!!!
#include "interrupt.h"
#include "term.h"

void panic(const char *message, const char *filename, const char *func, int line)
{
	term_printf("Kernel panic!\n%s, %s:%d: %s", filename, func, line, message);

	// That's it, I'm done
	disable_interrupts();

	for (;;)
		;
}
