// PS/2 controller driver

#include "interrupt.h"
#include "irq.h"
#include "term_printf.h"
#include "dev.h"

static const uint16_t PS2_DATA = 0x60;
static const uint16_t PS2_CMD  = 0x64;
static const uint16_t PS2_STAT = 0x64;

int pressedp(uint8_t scancode)
{
	return scancode <= 0x58;
}

static char const scancode_set1[] =
	"\0\0001234567890-=\b\tqwertyuiop[]\0\0asdfghjkl;'`\0\\zxcvbnm,./\0\0\0 ";

char get_char(uint8_t scancode)
{
	if (scancode >= 0x81)
		return scancode_set1[scancode - 0x80];
	else
		return scancode_set1[scancode];
}

static void ps2_handler(Registers regs)
{
	uint8_t s = inb(PS2_DATA); // Read entered char
	term_printf("Scan code: %p (%c %s)\n", (int)s, get_char(s),
			pressedp(s) ? "pressed" : "released");
}

void init_ps2()
{
	register_irq_handler(IRQ1, ps2_handler);
}
