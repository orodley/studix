// Paging stuff

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Page_entry
{
	unsigned int present  : 1;  // Page present in memory?
	unsigned int rw       : 1;  // Read-only (0) or read/write (1)?
	unsigned int user     : 1;  // Kernel or user access level?
	unsigned int accessed : 1;  // Has it been accessed since last refresh?
	unsigned int dirty    : 1;  // Has it been written to since last refresh?
	unsigned int unused   : 7;  // Reserved + unused available bits
	unsigned int frame    : 20; // Physical frame address, right shifted by 12
} Page_entry;

typedef struct Page_table
{
	Page_entry pages[1024];
} Page_table;

typedef struct Page_dir
{
	// Array of page table pointers
	Page_table *tables[1024];

	// Physical addresses of tables above
	uint32_t tables_physical[1024];

	// Physical address of tables_physical;
	uint32_t physical_addr;
} Page_dir;

bool aligned(uintptr_t ptr);
Page_entry *get_page(uint32_t addr, bool make_table, Page_dir *dir);
void alloc_frame(Page_entry *page, bool kernel, bool writeable);
void free_frame(Page_entry *page);
void init_paging();

// Pages are 4KiB
#define PAGE_SIZE 0x1000
