// Called by the ASSERT macro
#include <stdbool.h>
#include "term.h"

void assert(const char *asserted_expr, const char *filename, const char *func,
		int line)
{
	term_printf("\nFailed assert at %s, %s:%d\n %s",
			filename, func, line, asserted_expr);

	// That's it, I'm done
	__asm__ volatile ("cli");
	for (;;)
		;
}
