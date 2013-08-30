// PS/2 controller driver

#include "../cpu_setup/interrupt.h"
#include "../cpu_setup/irq.h"
#include "../term/term_printf.h"
#include "dev.h"

static const uint16_t PS2_DATA = 0x60;
static const uint16_t PS2_CMD  = 0x64;
static const uint16_t PS2_STAT = 0x64;

static void ps2_handler(Registers regs)
{
	uint8_t c = inb(PS2_DATA); // Read entered char
	term_printf("Scan code: %p\n", (int)c);
}

void init_ps2()
{
	register_irq_handler(IRQ1, ps2_handler);
}
