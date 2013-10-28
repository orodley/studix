/* ext2 filesystem implementation
 *
 * Helpful resources used include:
 *   http://nongnu.org/ext2-doc/ext2.html
 *   http://wiki.osdev.org/Ext2
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "assert.h"
#include "ext2.h"
#include "kmalloc.h"
#include "pata.h"
#include "term.h"

static const uint16_t EXT2_SIGNATURE  = 0xEF53;

static const size_t SUPERBLOCK_OFFSET = 1024;
static const size_t SUPERBLOCK_LENGTH = 1024;

// Can't be consts, as the initializers contain consts
#define             SUPERBLOCK_LBA      (SUPERBLOCK_OFFSET / SECTOR_SIZE)
#define             SUPERBLOCK_SECTORS  (SUPERBLOCK_LENGTH / SECTOR_SIZE)

static Ext2_superblock         superblock;
static BGD *bgdt;


// Some figures we need to calculate once we've read the superblock
static size_t block_size;
static size_t num_groups;

void load_superblock()
{
	uint16_t buf[SUPERBLOCK_LENGTH / 2];

	// We can't just copy into superblock directly, as it isn't long enough
	read_abs_sectors(SUPERBLOCK_LBA, SUPERBLOCK_SECTORS, buf);
	memcpy(&superblock, buf, sizeof(Ext2_superblock));

	block_size = 1024 << superblock.log2_block_size;
	num_groups = superblock.total_blocks / superblock.blocks_per_group;

	// Print some interesting stuff to check it loaded correctly
	ASSERT(superblock.signature == EXT2_SIGNATURE);
	term_printf(" total inodes = 0x%X\n", superblock.total_inodes);
	term_printf(" total blocks = 0x%X\n", superblock.total_blocks);
	term_printf(" block size   = %b\n", block_size);
	term_printf(" num blocks   = %d\n", superblock.total_blocks);
	term_printf(" blocks/group = %d\n", superblock.blocks_per_group);
	term_printf(" num groups   = %d\n", num_groups);
	term_printf(" BGDT size    = %b\n", sizeof(BGD) * num_groups);
}

void load_bgdt()
{
	size_t bgdt_sectors = (sizeof(BGD) * num_groups) / SECTOR_SIZE + 1;
	size_t bgdt_lba = (SUPERBLOCK_OFFSET + SUPERBLOCK_LENGTH) / block_size + 1;

	uint16_t buf[bgdt_sectors * SECTOR_SIZE / 2];
	read_abs_sectors(bgdt_lba, bgdt_sectors, buf);

	size_t bgdt_size = sizeof(BGD) * num_groups;
	bgdt = kmalloc(bgdt_size);
	memcpy(bgdt, buf, bgdt_size);
}


void init_fs()
{
	load_superblock();
	load_bgdt();
}
