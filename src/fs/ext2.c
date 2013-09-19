// ext2 filesystem implementation

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "assert.h"
#include "ext2.h"
#include "pata.h"
#include "term.h"

static const size_t SUPERBLOCK_OFFSET = 1024;
static const size_t SUPERBLOCK_LENGTH = 1024;

static const uint16_t EXT2_SIGNATURE = 0xEF53;

static Ext2_superblock superblock;

void load_superblock()
{
	uint32_t lba          = SUPERBLOCK_OFFSET / SECTOR_SIZE;
	size_t   sector_count = SUPERBLOCK_LENGTH / SECTOR_SIZE;

	uint16_t buf[SUPERBLOCK_LENGTH / 2];

	// We can't just copy into superblock directly, as it isn't long enough
	read_abs_sectors(lba, sector_count, buf);
	memcpy(&superblock, buf, sizeof(Ext2_superblock));

	// Print some interesting stuff to check it loaded correctly
	ASSERT(superblock.signature == EXT2_SIGNATURE);
	term_printf(" total inodes = 0x%X\n", superblock.total_inodes);
	term_printf(" total blocks = 0x%X\n", superblock.total_blocks);
	term_printf(" block size   = 0x%x\n", 1024 << superblock.log2_block_size);
}


void init_fs()
{
	load_superblock();
}
