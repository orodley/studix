// Print formatted output to the VGA terminal

#include <stdarg.h>
#include "vga.h"

static void term_print_dec(int x)
{
	int divisor = 1;

	for (; divisor <= x; divisor *= 10)
		;

	divisor /= 10;

	for (; divisor > 0; divisor /= 10)
		term_putchar(((x / divisor) % 10) + '0');
}

void term_printf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	for (int i = 0; fmt[i] != '\0'; i++) {
		if (fmt[i] != '%') {
			term_putchar(fmt[i]);
		} else {
			char fmt_type = fmt[++i];
			switch (fmt_type) {
			case '%':
				term_putchar('%');
				break;
			case 'd':
				term_print_dec(va_arg(args, int));
				break;
			case 's':
				term_putsn(va_arg(args, char*));
				break;
			case 'c':
				term_putchar(va_arg(args, int));
				break;
			default:
				break;
			}
		}
	}

	va_end(args);
}
