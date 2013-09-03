// Programmable Interval Timer functions

#include <stdint.h>
#include "interrupt.h"
#include "term.h"
#include "dev.h"

static const uint8_t TIMER_CHAN0      = 0x40;
static const uint8_t TIMER_CMD        = 0x43;

static const int BASE_FREQUENCY    = 1193180;
static const int MILLISECOND_FREQ  = 1000;

static long milli_uptime = 0;

static void timer_handler(Registers regs)
{
	milli_uptime++;
}

long uptime()
{
	return milli_uptime;
}

void init_timer()
{
	register_interrupt_handler(IRQ0, timer_handler);

	// Divisor must be small enough to fit into 16 bits!
	uint16_t divisor = BASE_FREQUENCY / MILLISECOND_FREQ;

	// Send a command byte to set up the timer
	outb(TIMER_CMD, 0x36);

	// Send the divisor byte-wise
	outb(TIMER_CHAN0, divisor & 0xFF);
	outb(TIMER_CHAN0, divisor >> 8);
}
