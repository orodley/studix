// Paging stuff

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Page_entry
{
	uint32_t present  : 1;  // Page present in memory?
	uint32_t rw       : 1;  // Read-only (0) or read/write (1)?
	uint32_t user     : 1;  // Kernel or user access level?
	uint32_t accessed : 1;  // Has the page been accessed since last refresh?
	uint32_t dirty    : 1;  // Has the page been written to since last refresh?
	uint32_t unused   : 7;  // Reserved + unused available bits
	uint32_t frame    : 20; // Physical frame address, right shifted by 12
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

Page_entry *get_page(uint32_t addr, bool make_table, Page_dir *dir);
void alloc_frame(Page_entry *page, bool kernel, bool writeable);
void free_frame(Page_entry *page);
void init_paging();

// Pages are 4KiB
#define PAGE_SIZE 0x1000
