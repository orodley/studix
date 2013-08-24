// ISR handler

#include <stdint.h>
#include "term_printf.h"

// Represents the registers we push onto the stack in isr_common (isrs.s)
typedef struct Registers
{
	uint32_t ds, edi, esi, ebp, esp, ebx, edx, ecx, eax,
		int_no, err, eip, cs, eflags, useresp, ss;
} Registers;

void isr_handler(Registers regs)
{
	term_printf("Unhandled interrupt: %d", regs.int_no);
}
