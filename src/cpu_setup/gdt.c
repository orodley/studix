// Global Descriptor Table setup

#include <stddef.h>
#include <stdint.h>
#include "gdt.h"

// In this file, "limit" refers to the higest address accessible, and
// "base" refers to the base address, the lowest address accessible

typedef struct GDT_entry
{
	uint16_t limit_low; // 16 LSB of the limit
	uint16_t base_low;  // 16 LSB of the base
	uint8_t  base_mid;  // Next 8 bits of the base
	uint8_t  access;    // Access flags
	uint8_t  gran;      // Granularity
	uint8_t  base_high; // 8 MSB of the base
} __attribute__((packed)) GDT_entry;

typedef struct GDT_ptr
{
	uint16_t limit; // 16 MSB of all limits
	uint32_t base;  // Address of the first GDT_entry
} __attribute__((packed)) GDT_ptr;

extern void flush_gdt(uint32_t gdt);

#define NUM_GDT_ENTRIES 5

static GDT_entry gdt_entries[NUM_GDT_ENTRIES];
static GDT_ptr   gdt;

static void set_gdt_entry(size_t i, uint32_t base, uint32_t limit,
		uint8_t access, uint8_t gran)
{
	gdt_entries[i].base_low  =  base & 0xFFFF;
	gdt_entries[i].base_mid  = (base >> 16) & 0xFF;
	gdt_entries[i].base_high =  base >> 24;

	gdt_entries[i].limit_low = limit & 0xFFFF;

	gdt_entries[i].gran      = (limit >> 16) & 0xF;
	gdt_entries[i].gran     |= gran & 0xF0;

	gdt_entries[i].access    = access;
}

void init_gdt()
{
	gdt.limit = (sizeof(GDT_entry) * NUM_GDT_ENTRIES) - 1;
	gdt.base  = (uintptr_t)&gdt_entries;

	set_gdt_entry(0, 0, 0,          0,    0);    // Null segment
	set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // Code segment
	set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // Data segment
	set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // User mode code segment
	set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // User mode data segment

	flush_gdt((uintptr_t)&gdt);
}
