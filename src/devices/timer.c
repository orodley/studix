// Programmable Interrupt Timer functions

#include <stdint.h>
#include "../cpu_setup/interrupt.h"
#include "../cpu_setup/irq.h"
#include "../term/term_printf.h"
#include "dev.h"

static const int     BASE_FREQUENCY = 1193180;
static const uint8_t TIMER_CHAN0    = 0x40;
static const uint8_t TIMER_CMD      = 0x43;

static uint32_t tick = 0;

static void timer_callback(Registers regs)
{
	tick++;
	term_printf("Tick: %d\n", tick);
}

void init_timer(uint32_t frequency)
{
	register_irq_handler(IRQ0, timer_callback);

	// Divisor must be small enough to fit into 16 bits!
	uint16_t divisor = BASE_FREQUENCY / frequency;

	// Send a command byte to set up the timer
	outb(TIMER_CMD, 0x36);

	// Send the divisor byte-wise
	outb(TIMER_CHAN0, divisor & 0xFF);
	outb(TIMER_CHAN0, divisor >> 8);
}
