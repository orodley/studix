/* ext2 filesystem implementation
 *
 * Helpful resources used include:
 *   http://nongnu.org/ext2-doc/ext2.html
 *   http://wiki.osdev.org/Ext2
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "assert.h"
#include "ext2.h"
#include "kmalloc.h"
#include "pata.h"
#include "panic.h"
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


// Prototypes for static functions
static void     read_block(uint32_t block_num, void *buf);
static void     load_superblock();
static void     load_bgdt();
static void     read_inode(Ext2_inode *inode, uint32_t inode_num);
static void     open_inode(uint32_t inode_num, Ext2_file *file);
static bool     next_dirent(Ext2_file *file, Ext2_dirent *dir);
static uint32_t find_in_dir(uint32_t dir_inode, const char *name);
static uint32_t look_up_path(char *path);


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
	term_printf(" / size          = %d\n",   root_inode.size);

	// Enumerate the files in it
	term_puts(" / files:");

	Ext2_file file;
	open_inode(ROOT_INODE, &file);
	Ext2_dirent dirent;

	while (next_dirent(&file, &dirent)) {
		term_printf("  inode %d, name `%s'\n", dirent.inode_num, dirent.name);
	}

	kfree(file.buf);

	// Look for a file
	term_putsn(" looking for file `/bar/baz/quux'...");
	uint32_t inode = look_up_path("/bar/baz/quux");
	if (inode == 0)
		term_puts(" not found");
	else
		term_printf(" found: inode = %d\n", inode);
}

static void read_block(uint32_t block_num, void *buf)
{
	uint32_t lba   = block_num * block_size / SECTOR_SIZE;
	size_t sectors = block_size / SECTOR_SIZE;

	read_abs_sectors(lba, sectors, buf);
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
	uint32_t bgdt_lba = bgdt_block * block_size / SECTOR_SIZE;

	uint16_t buf[bgdt_sectors * SECTOR_SIZE / 2];
	read_abs_sectors(bgdt_lba, bgdt_sectors, buf);

	size_t bgdt_size = sizeof(BGD) * num_groups;
	bgdt = kmalloc(bgdt_size);
	memcpy(bgdt, buf, bgdt_size);
}

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

static void open_inode(uint32_t inode_num, Ext2_file *file)
{
	read_inode(&file->inode, inode_num);
	file->pos            = 0;
	file->block_index    = 0;
	file->buf            = kmalloc(block_size);
	file->curr_block_pos = 0;

	// Read in the first block immediately
	read_block(file->inode.dbp[0], file->buf);
}

static size_t ext2_read(Ext2_file *file, uint8_t *buf, size_t count)
{
	// Check if we would read past the end of the file
	if (file->pos + count > file->inode.size)
		count = file->inode.size - file->pos;

	size_t bytes_left   = count;

	while (bytes_left > 0) {
		size_t to_copy = bytes_left;

		// Check if this read will go beyond the current buffer
		bool new_block = file->curr_block_pos + to_copy >= block_size;
		if (new_block)
			to_copy = block_size - file->curr_block_pos;

		// Copy across from the buffer in the *file and advance the position
		memcpy(buf + (count - bytes_left),
				file->buf + file->curr_block_pos, to_copy);
		file->curr_block_pos += to_copy;
		file->pos            += to_copy;
		bytes_left           -= to_copy;

		// If we read to the end of the buffer then read the next block
		if (new_block) {
			file->curr_block_pos = 0;
			file->block_index++;
			if (file->block_index >= 12)
				PANIC("Indirect block pointers are currently unsupported");

			read_block(file->inode.dbp[file->block_index], file->buf);
		}
	}

	return count;
}

#define READ_SIZE (sizeof(Ext2_dirent) - sizeof(uint8_t*))

// Returns true if a new direntry was read, otherwise false, indicating that
// all of the entries have been read
static bool next_dirent(Ext2_file *file, Ext2_dirent *dir)
{
	uint8_t buf[READ_SIZE];
	if (ext2_read(file, buf, READ_SIZE) != READ_SIZE) // Not enough data left
		return false;

	memcpy(dir, buf, READ_SIZE);

	size_t   size = dir->name_len + 1;
	uint8_t *name = kmalloc(size);
	if (ext2_read(file, name, size - 1) != size - 1)
		return false;

	dir->name = name;
	dir->name[size - 1] = '\0';

	// Read up to the next entry
	size_t bytes_left = dir->total_len - (READ_SIZE + size - 1);
	if (bytes_left > 0) {
		uint8_t dummy[bytes_left];
		ext2_read(file, dummy, bytes_left);
	}

	return true;
}

// Returns true if the file was found
static uint32_t find_in_dir(uint32_t dir_inode, const char *name)
{
	uint32_t inode;
	Ext2_file dir;
	Ext2_dirent dirent;

	open_inode(dir_inode, &dir);
	while (next_dirent(&dir, &dirent)) {
		if (strcmp((char*)dirent.name, name) == 0) {
			inode = dirent.inode_num;
			goto cleanup;
		}
	}

	inode = 0;

cleanup:
	kfree(dir.buf);
	return inode; // inodes are 1-based, so 0 can be used as an error value
}

// Return the inode corrsponding to the absolute pathname in `path'
static uint32_t look_up_path(char *path)
{
	if (path[0] != '/') // Path must be absolute
		return 0;

	path++;
	uint32_t curr_dir_inode = ROOT_INODE;

	for (;;) {
		size_t j;
		for (j = 0; path[j] != '/' && path[j] != '\0'; j++)
			;

		if (path[j] == '\0')
			break;

		path[j] = '\0';
		curr_dir_inode = find_in_dir(curr_dir_inode, path);
		path[j] = '/';

		if (curr_dir_inode == 0)
			return 0;

		path += j + 1;
	}

	uint32_t inode = find_in_dir(curr_dir_inode, path);
	if (inode == 0)
		return 0;

	return inode;
}
