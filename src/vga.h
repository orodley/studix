#include <stdint.h>
#include <stddef.h>

/* Hardware text mode color constants. */
enum VGAColor
{
	COLOR_BLACK         =  0,
	COLOR_BLUE          =  1,
	COLOR_GREEN         =  2,
	COLOR_CYAN          =  3,
	COLOR_RED           =  4,
	COLOR_MAGENTA       =  5,
	COLOR_BROWN         =  6,
	COLOR_LIGHT_GREY    =  7,
	COLOR_DARK_GREY     =  8,
	COLOR_LIGHT_BLUE    =  9,
	COLOR_LIGHT_GREEN   = 10,
	COLOR_LIGHT_CYAN    = 11,
	COLOR_LIGHT_RED     = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN   = 14,
	COLOR_WHITE         = 15,
};

void term_putentryat(char c, uint8_t color, size_t x, size_t y);
void term_writestring(const char *data);
void term_putchar(char c);
void term_setcolor(uint8_t color);
void init_term();
uint8_t make_color(enum VGAColor fg, enum VGAColor bg);
