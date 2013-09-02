// PS/2 controller driver

#include <stddef.h>
#include "interrupt.h"
#include "irq.h"
#include "term.h"
#include "dev.h"

static const uint16_t PS2_DATA = 0x60;
static const uint16_t PS2_CMD  = 0x64;
static const uint16_t PS2_STAT = 0x64;

#define LSHIFT_DOWN 0x2A
#define LSHIFT_UP   0xAA
#define RSHIFT_DOWN 0x36
#define RSHIFT_UP   0xB6
#define ENTER_DOWN  0x1C
#define ENTER_UP    0x9C

static const size_t CASE_DIFF = 32;

static int shift_down = 0;

int pressedp(uint8_t scancode)
{
	return scancode <= 0x58;
}

static char const SCANCODE_SET1[] =
	"\0\0001234567890-=\b\tqwertyuiop[]\0\0asdfghjkl;'`\0\\zxcvbnm,./\0\0\0 ";

char get_char(uint8_t scancode)
{
	if (scancode >= 0x81)
		return SCANCODE_SET1[scancode - 0x80];
	else
		return SCANCODE_SET1[scancode];
}

/* Starting from SYMBOL_START, these are the characters to display for
 * each symbol if shift is currently pushed. \0 indicates an invalid
 * value, e.g. an uppercase symbol or letter */
static char const UPPERCASE_SYMBOLS[] =
	"\"\0\0\0\0<_>?)!@#$%^&*(\0:\0+\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\
	 \0\0\0\0\0\0\0\0\0\0\0{|}\0\0~";
static const char SYMBOL_START = '\'';

char capitalize(char c)
{
	if (c >= 'a' && c <= 'z')
		return c - CASE_DIFF;
	else
		return UPPERCASE_SYMBOLS[c - SYMBOL_START];
}

static void ps2_handler(Registers regs)
{
	uint8_t s = inb(PS2_DATA); // Read entered scancode

	switch (s) {
	case LSHIFT_DOWN:
	case RSHIFT_DOWN:
		shift_down = 1;
		return;
	case LSHIFT_UP:
	case RSHIFT_UP:
		shift_down = 0;
		return;
	case ENTER_DOWN:
		term_putchar('\n');
		return;
	}

	char c = get_char(s);
	if (shift_down)
		c = capitalize(c);

	if (pressedp(s)) {
		term_putchar(c);
		update_cursor();
	}
}

void init_ps2()
{
	register_irq_handler(IRQ1, ps2_handler);
}
