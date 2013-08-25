// IRQ handler

#include "../devices/dev.h"
#include "idt.h"
#include "interrupt.h"
#include "irq.h"

static Handler handlers[NUM_IDT_ENTRIES];

void irq_handler(Registers regs)
{
	if (regs.int_no >= IRQ8) {
		// This interrupt involved the slave, send reset to it
		outb(0xA0, 0x20);
	}

	// Reset master
	outb(0x20, 0x20);

	Handler sel_handler = handlers[regs.int_no];
	if (sel_handler) {
		sel_handler(regs);
	}
}

void register_irq_handler(uint8_t i, Handler handler)
{
	handlers[i] = handler;
}
