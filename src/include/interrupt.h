#include <stdint.h>

// Represents the registers we push onto the stack
// in isr_common and irq_common (isrs.s)
typedef struct Registers
{
	uint32_t ds, edi, esi, ebp, esp, ebx, edx, ecx, eax,
		int_no, err, eip, cs, eflags, useresp, ss;
} Registers;
