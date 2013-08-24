// ISR handler

#include <stdint.h>
#include "vga.h"

// Represents the registers we push onto the stack in isr_common (isrs.s)
typedef struct Registers
{
	uint32_t ds, edi, esi, ebp, esp, ebx, edx, ecx, eax,
		int_no, err, eip, cs, eflags, useresp, ss;
} Registers;

void isr_handler(Registers regs)
{
	term_puts("Recieved interrupt");
	// Frob regs to avoid compiler warning; we'll actually use them later
	regs.ds = regs.ds; 
}
