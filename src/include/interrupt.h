#include <stdint.h>

// IRQ numbers
#define IRQ0 32
#define IRQ1 33
#define IRQ2 34
#define IRQ3 35
#define IRQ4 36
#define IRQ5 37
#define IRQ6 38
#define IRQ7 39
#define IRQ8 40
#define IRQ9 41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

// Represents the registers we push onto the stack
// in isr_common and irq_common (isrs.s)
typedef struct Registers
{
	uint32_t ds, edi, esi, ebp, esp, ebx, edx, ecx, eax,
		int_no, err, eip, cs, eflags, useresp, ss;
} Registers;

// Interrupt handler function type
typedef void (*Handler)(Registers);
void register_interrupt_handler(uint8_t i, Handler handler);
