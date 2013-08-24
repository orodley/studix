#include <stddef.h>
#include <stdint.h>
#include "vga.h"
#include "lib/string.h"
 
static const size_t VGA_WIDTH  = 80;
static const size_t VGA_HEIGHT = 25;
 
static size_t          curr_y      = 0;
static size_t          curr_x      = 0;
static uint16_t *const term_buffer = (uint16_t*)0xB8000;
static uint8_t         term_color;
 
uint8_t make_color(enum VGAColor fg, enum VGAColor bg)
{
	return fg | bg << 4;
}
 
uint16_t make_vga_entry(char c, uint8_t color)
{
	uint16_t c16     = c;
	uint16_t color16 = color;

	return c16 | color16 << 8;
}
 
static void term_put_entry(char c, uint8_t color, size_t x, size_t y)
{
	term_buffer[y * VGA_WIDTH + x] = make_vga_entry(c, color);
}
 
void init_term()
{
	term_color  = make_color(LIGHT_GREY, BLACK);

	for (size_t y = 0; y < VGA_HEIGHT; y++)
		for (size_t x = 0; x < VGA_WIDTH; x++)
			term_put_entry(' ', term_color, x, y);
}
 
void term_set_color(uint8_t color)
{
	term_color = color;
}
 
void term_putchar(char c)
{
	switch (c) {
	case '\n':
		curr_x = 0;
		curr_y++;
		return;
	case '\r':
		curr_x = 0;
		return;
	case '\b':
		if (curr_x == 0) {
			curr_x = VGA_WIDTH - 1;
			curr_y--;
		} else {
			curr_x--;
		}

		return;
	default:
		break;
	}

	term_put_entry(c, term_color, curr_x, curr_y);

	if (++curr_x == VGA_WIDTH) {
		curr_x = 0;

		if (++curr_y == VGA_HEIGHT)
			curr_y = 0;
	}
}
 
void term_putsn(const char *str)
{
    for (size_t i = 0; str[i] != '\0'; i++)
	term_putchar(str[i]);
}

void term_puts(const char *str)
{
    term_putsn(str);
    term_putchar('\n');
}
