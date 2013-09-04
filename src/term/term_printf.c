// Print formatted output to the VGA terminal

#include <stdarg.h>
#include "term.h"

static void term_print_dec(int x)
{
	int divisor = 1;

	for (; divisor <= x; divisor *= 10)
		;

	divisor /= 10;

	for (; divisor > 0; divisor /= 10)
		term_putchar(((x / divisor) % 10) + '0');
}

static const size_t CASE_DIFF = 32;

static void term_print_hex(unsigned int x, int upper)
{
	int shift = 28;
	for (; ((x >> shift) & 0xF) == 0 && shift > 0; shift -= 4)
		;

	for (; shift >= 0; shift -= 4) {
		uint8_t digit = (x >> shift) & 0xF;

		char c;
		if (digit < 0xA)
			c = '0' + digit;
		else
			c = 'A' + (digit - 0xA) + (upper ? 0 : CASE_DIFF);

		term_putchar(c);
	}
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
			case 'x':
				term_print_hex(va_arg(args, unsigned int), 0);
				break;
			case 'X':
				term_print_hex(va_arg(args, unsigned int), 1);
				break;
			case 'p':
				term_putsn("0x");
				term_print_hex(va_arg(args, unsigned int), 0);
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
