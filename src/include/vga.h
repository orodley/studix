#include <stdint.h>
#include <stddef.h>

/* Hardware text mode color constants. */
enum VGAColor
{
	BLACK         =  0,
	BLUE          =  1,
	GREEN         =  2,
	CYAN          =  3,
	RED           =  4,
	MAGENTA       =  5,
	BROWN         =  6,
	LIGHT_GREY    =  7,
	DARK_GREY     =  8,
	LIGHT_BLUE    =  9,
	LIGHT_GREEN   = 10,
	LIGHT_CYAN    = 11,
	LIGHT_RED     = 12,
	LIGHT_MAGENTA = 13,
	LIGHT_BROWN   = 14,
	WHITE         = 15,
};

void term_putchar(char c);
void term_puts(const char *data);
void term_putsn(const char *data);
void term_set_color(uint8_t color);
void init_term();
uint8_t make_color(enum VGAColor fg, enum VGAColor bg);
