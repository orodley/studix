// Interrupt Descriptor Table setup

#include <stddef.h>
#include <stdint.h>
#include "idt.h"

typedef struct IDT_entry
{
	uint16_t base_low;  // 16 LSB of handler address
	uint16_t selector;  // Segment selector
	uint8_t  zero;      // Always 0
	uint8_t  flags;
	uint16_t base_high; // 16 MSB of handler address
} __attribute__((packed)) IDT_entry;

typedef struct IDT_ptr
{
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) IDT_ptr;

// extern declarations for isr{0..31}
#define EXISR(n) extern void isr##n()

EXISR(0);  EXISR(1);  EXISR(2);  EXISR(3);  EXISR(4);  EXISR(5);  EXISR(6);
EXISR(7);  EXISR(8);  EXISR(9);  EXISR(10); EXISR(11); EXISR(12); EXISR(13);
EXISR(14); EXISR(15); EXISR(16); EXISR(17); EXISR(18); EXISR(19); EXISR(20);
EXISR(21); EXISR(22); EXISR(23); EXISR(24); EXISR(25); EXISR(26); EXISR(27);
EXISR(28); EXISR(29); EXISR(30); EXISR(31);

extern void flush_idt(uint32_t);

#define NUM_IDT_ENTRIES 256

IDT_entry idt_entries[NUM_IDT_ENTRIES];
IDT_ptr   idt;

// TODO: Move to string.h in lib/ dir
void *memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; i++)
		((uint8_t*)s)[i] = c;

	return s;
}

static void set_idt_entry(uint8_t i, uint32_t base, uint16_t selector,
		uint8_t flags)
{
	idt_entries[i].base_low  = base & 0xFFFF;
	idt_entries[i].base_high = base >> 16;

	idt_entries[i].selector  = selector;
	idt_entries[i].zero      = 0;
	// Must be changed when user-mode added
	idt_entries[i].flags     = flags;
}

// Default IDT entry setter
#define SET_IDT(n) set_idt_entry(n, (uint32_t)isr##n, 0x08, 0x8E)

void init_idt()
{
	idt.limit = (sizeof(IDT_entry) * NUM_IDT_ENTRIES) - 1;
	idt.base  = (uint32_t)&idt_entries;

	memset(&idt_entries, 0, sizeof(IDT_entry) * NUM_IDT_ENTRIES);

	// Set IDT entries 0-31
	SET_IDT(0);  SET_IDT(1);  SET_IDT(2);  SET_IDT(3);  SET_IDT(4);
	SET_IDT(5);  SET_IDT(6);  SET_IDT(7);  SET_IDT(8);  SET_IDT(9);
	SET_IDT(10); SET_IDT(11); SET_IDT(12); SET_IDT(13); SET_IDT(14);
	SET_IDT(15); SET_IDT(16); SET_IDT(17); SET_IDT(18); SET_IDT(19);
	SET_IDT(20); SET_IDT(21); SET_IDT(22); SET_IDT(23); SET_IDT(24);
	SET_IDT(25); SET_IDT(26); SET_IDT(27); SET_IDT(28); SET_IDT(29);
	SET_IDT(30); SET_IDT(31);

	flush_idt((uint32_t)&idt);
}
