#include <stddef.h>
#include <stdint.h>
#include "vga.h"
 
static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;
 
static size_t    term_row    = 0;
static size_t    term_col    = 0;
static uint16_t *term_buffer = (uint16_t*) 0xB8000;
static uint8_t   term_color;
 
uint8_t make_color(enum VGAColor fg, enum VGAColor bg)
{
	return fg | bg << 4;
}
 
uint16_t make_vgaentry(char c, uint8_t color)
{
	uint16_t c16     = c;
	uint16_t color16 = color;

	return c16 | color16 << 8;
}
 
size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len++] != '\0')
		;

	return len;
}
 
void term_putentryat(char c, uint8_t color, size_t x, size_t y)
{
	term_buffer[y * VGA_WIDTH + x] = make_vgaentry(c, color);
}
 
void init_term()
{
	term_color  = make_color(COLOR_LIGHT_GREY, COLOR_BLACK);

	for (size_t y = 0; y < VGA_HEIGHT; y++)
		for (size_t x = 0; x < VGA_WIDTH; x++)
			term_putentryat(' ', term_color, x, y);
}
 
void term_setcolor(uint8_t color)
{
	term_color = color;
}
 
void term_putchar(char c)
{
	switch (c) {
		case '\n':
			term_col = 0;
			term_row++;
			return;
		case '\r':
			term_col = 0;
			return;
		case '\b':
			term_col--;
			return;
		default:
			break;
	}

	term_putentryat(c, term_color, term_col, term_row);

	if (++term_col == VGA_WIDTH) {
		term_col = 0;

		if (++term_row == VGA_HEIGHT)
			term_row = 0;
	}
}
 
void term_writestring(const char *str)
{
	for (size_t i = 0; i < strlen(str); i++)
		term_putchar(str[i]);
}
