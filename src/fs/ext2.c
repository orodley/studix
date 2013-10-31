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
static const uint16_t INODE_SIZE      = 128;
static const uint32_t ROOT_INODE      = 2;

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

static void read_inode(Ext2_inode *inode, uint32_t inode_num)
{
	inode_num--; // inode numbers start at 1, change to 0-based index
	size_t block_group = inode_num / superblock.inodes_per_group;

	// Look up the starting block in the BGDT
	BGD *bgd = &bgdt[block_group];
	uint32_t i_table_block = bgd->inode_table_addr;

	// Now figure out how many blocks the inode we want is offset by
	size_t index           = inode_num % superblock.inodes_per_group;
	size_t block_offset    = (index * INODE_SIZE) / block_size;
	size_t offset_in_block = (index * INODE_SIZE) % block_size;
	size_t block           = i_table_block + block_offset;

	// Read however many sectors we need to, then copy
	size_t num_sectors = sizeof(Ext2_inode) / SECTOR_SIZE + 1;
	uint16_t buf[num_sectors * SECTOR_SIZE / 2];
	read_abs_sectors(block * block_size / SECTOR_SIZE, num_sectors, buf);
	memcpy(inode, &buf[offset_in_block / 2], sizeof(Ext2_inode));
}


static void load_superblock()
{
	uint16_t buf[SUPERBLOCK_LENGTH / 2];

	// We can't just copy into superblock directly, as it isn't long enough
	read_abs_sectors(SUPERBLOCK_LBA, SUPERBLOCK_SECTORS, buf);
	memcpy(&superblock, buf, sizeof(Ext2_superblock));

	block_size = 1024 << superblock.log2_block_size;
	num_groups = superblock.total_blocks / superblock.blocks_per_group;

	// Print some interesting stuff to check it loaded correctly
	ASSERT(superblock.signature == EXT2_SIGNATURE);
	term_printf(" total inodes    = 0x%X\n", superblock.total_inodes);
	term_printf(" total blocks    = 0x%X\n", superblock.total_blocks);
	term_printf(" block size      = %b\n",   block_size);
	term_printf(" num blocks      = %d\n",   superblock.total_blocks);
	term_printf(" blocks/group    = %d\n",   superblock.blocks_per_group);
	term_printf(" inodes/group    = %d\n",   superblock.inodes_per_group);
	term_printf(" num groups      = %d\n",   num_groups);
}

static void load_bgdt()
{
	size_t bgdt_sectors = (sizeof(BGD) * num_groups) / SECTOR_SIZE + 1;
	size_t bgdt_block = (SUPERBLOCK_OFFSET + SUPERBLOCK_LENGTH) / block_size + 1;
	size_t bgdt_lba = bgdt_block * block_size / SECTOR_SIZE;

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

	// Read the root inode, just for fun
	Ext2_inode root_inode;
	read_inode(&root_inode, ROOT_INODE);
	term_printf(" / creation time = %d\n",   root_inode.creation_time);
	term_printf(" / uid           = %d\n",   root_inode.uid);
	term_printf(" / type & perms  = 0x%X\n", root_inode.type_and_permissions);
}
