// Interrupt Descriptor Table setup

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "../devices/dev.h"
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
EXISR(28); EXISR(29); EXISR(30); EXISR(31); EXISR(32); EXISR(33); EXISR(34);
EXISR(35); EXISR(36); EXISR(37); EXISR(38); EXISR(39); EXISR(40); EXISR(41);
EXISR(42); EXISR(43); EXISR(44); EXISR(45); EXISR(46); EXISR(47);

extern void flush_idt(uint32_t);

IDT_entry idt_entries[NUM_IDT_ENTRIES];
IDT_ptr   idt;

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

// Remap the IRQ table
void remap_irq()
{
	outb(0x20, 0x11); // Start the init sequence for PIC 1...
	outb(0xA0, 0x11); // ...and PIC 2
	outb(0x21, 0x20); // Offset PIC 1 by 32, as the CPU uses the first 32 ISRs
	outb(0xA1, 0x28); // Offset PIC 2 by another 8 (8 pins on the PIC)
	outb(0x21, 0x04); // Let PIC 1 know where PIC 2 (the slave) is
	outb(0xA1, 0x02); // Let PIC 2 know its cascade identity
	outb(0x21, 0x01); // Put PIC 1 into 8086 mode
	outb(0xA1, 0x01); // Put PIC 2 into 8086 mode
	outb(0x21, 0x0);  // Restore saved masks (assuming zero)
	outb(0xA1, 0x0);
}

void init_idt()
{
	remap_irq();

	idt.limit = (sizeof(IDT_entry) * NUM_IDT_ENTRIES) - 1;
	idt.base  = (uint32_t)&idt_entries;

	memset(&idt_entries, 0, sizeof(IDT_entry) * NUM_IDT_ENTRIES);

	// Set IDT entries 0-47
	SET_IDT(0);  SET_IDT(1);  SET_IDT(2);  SET_IDT(3);  SET_IDT(4);
	SET_IDT(5);  SET_IDT(6);  SET_IDT(7);  SET_IDT(8);  SET_IDT(9);
	SET_IDT(10); SET_IDT(11); SET_IDT(12); SET_IDT(13); SET_IDT(14);
	SET_IDT(15); SET_IDT(16); SET_IDT(17); SET_IDT(18); SET_IDT(19);
	SET_IDT(20); SET_IDT(21); SET_IDT(22); SET_IDT(23); SET_IDT(24);
	SET_IDT(25); SET_IDT(26); SET_IDT(27); SET_IDT(28); SET_IDT(29);
	SET_IDT(30); SET_IDT(31); SET_IDT(32); SET_IDT(33); SET_IDT(34);
	SET_IDT(35); SET_IDT(36); SET_IDT(37); SET_IDT(38); SET_IDT(39);
	SET_IDT(40); SET_IDT(41); SET_IDT(42); SET_IDT(43); SET_IDT(44);
	SET_IDT(45); SET_IDT(46); SET_IDT(47);

	flush_idt((uint32_t)&idt);
}
