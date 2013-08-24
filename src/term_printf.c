// Print formatted output to the VGA terminal

#include "vga.h"
#include <stdarg.h>

void term_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    for (int i = 0; fmt[i] != '\0'; i++) {
	char c = fmt[i];

	if (c != '%') {
	    term_putchar(c);
	} else {
	    char fmt_type = fmt[++i];
	    switch (fmt_type) {
	    case '%':
		term_putchar('%');
	    default:
		break;
	    }
	}
    }

    va_end(args);
}
