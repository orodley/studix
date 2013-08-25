// ISR handler

#include <stdint.h>
#include "term_printf.h"

// Represents the registers we push onto the stack in isr_common (isrs.s)
typedef struct Registers
{
	uint32_t ds, edi, esi, ebp, esp, ebx, edx, ecx, eax,
		int_no, err, eip, cs, eflags, useresp, ss;
} Registers;

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

void isr_handler(Registers regs)
{
	term_printf("Unhandled interrupt: %s\n", interrupt_names[regs.int_no]);
}
