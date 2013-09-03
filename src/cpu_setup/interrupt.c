// Interrupt stuff

#include <stdint.h>
#include "dev.h"
#include "idt.h"
#include "interrupt.h"
#include "term.h"

static const char *interrupt_names[] =
{
	"Division by zero",
	"Debug exception",
	"Non-maskable interrupt",
	"Breakpoint exception",
	"Overflow",
	"Out of bounds exception",
	"Invalid opcode",
	"No coprocessor",
	"Double fault",
	"Coprocessor segment overrun",
	"Bad TSS",
	"Segment not present",
	"Stack fault",
	"General Protection Fault",
	"Page fault",
	"Unknown interrupt exception",
	"Coprocessor fault",
	"Alignment check exception",
	"Machine check exception"
};

static Handler handlers[NUM_IDT_ENTRIES];

void irq_handler(Registers regs)
{
	if (regs.int_no >= IRQ8) {
		// This interrupt involved the slave, send reset to it
		outb(0xA0, 0x20);
	}

	// Reset master
	outb(0x20, 0x20);

	Handler handler = handlers[regs.int_no];
	if (handler)
		handler(regs);
}

void isr_handler(Registers regs)
{
	Handler handler = handlers[regs.int_no];
	if (handler)
		handler(regs);
	else
		term_printf("Unhandled interrupt: %s\n", interrupt_names[regs.int_no]);
}

void register_interrupt_handler(uint8_t i, Handler handler)
{
	handlers[i] = handler;
}
