// Programmable Interval Timer functions

#include <stdint.h>
#include "interrupt.h"
#include "term.h"
#include "dev.h"

#define TIMER_CHAN0 0x40
#define TIMER_CMD   0x43

#define BASE_FREQUENCY   1193180
#define MILLISECOND_FREQ 1000

static unsigned long milli_uptime = 0;

static void timer_handler(Registers regs)
{
	milli_uptime++;
}

unsigned long uptime()
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
