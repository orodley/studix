#include "vga.h"

void kernel_main()
{
	init_term();
	term_writestring(
			"Hello, kernel World!\n"
			"Newlines working!\n"
			"XXXXXX seems to work\rReturn\n"
			"XXXX\b\b\b\bBackspace works\n"
			"\b|");
}
