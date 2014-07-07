// Integrating our Ext2 implementation into the VFS

#include <stddef.h>
#include <string.h>
#include "assert.h"
#include "ext2.h"
#include "kmalloc.h"
#include "vfs.h"
#include "panic.h"

static void ext2_vfs_open(FS_node *node)
{
	node->inode = ext2_look_up_path(node->name);

	if (node->inode != 0) {
		// TODO: We need some way of reporting errors rather than silently
		// failing if look_up_path fails
		Ext2_file *file = kmalloc(sizeof *file);
		ext2_open_inode(node->inode, file);

		// We use node->impl to store a pointer to our Ext2_file structure for
		// Ext2 FS_nodes
		node->impl = file;

		// TODO: stick more file metadata into node
	}
}

static void ext2_vfs_close(FS_node *node)
{
	// Free the previously allocated Ext_file structure
	kfree(node->impl);
}

static uint32_t ext2_vfs_read(FS_node *node, size_t offset, size_t size, char *buf)
{
	if (offset != ((Ext2_file *)node->impl)->pos) {
		PANIC("Seeking is currently unsupported for Ext2 files\n");
		return 0;
	}

	return (uint32_t)ext2_read(node->impl, (uint8_t *)buf, size);
}

static uint32_t ext2_vfs_write(FS_node *node, size_t offset, size_t size, char *buf)
{
	PANIC("Writing to Ext2 is currently unsupported\n");

	return 0;
}

// TODO: let the caller choose
static Dir_entry *ext2_vfs_read_dir(FS_node *node, size_t index)
{
	Ext2_dirent ext2_dir;

	if (ext2_next_dirent(node->impl, &ext2_dir)) {
		Dir_entry *dir = kmalloc(sizeof *dir);

		dir->inode = ext2_dir.inode_num;
		strcpy(dir->name, (char *)ext2_dir.name);

		return dir;
	} else {
		return NULL;
	}
}

// TODO: let the caller choose
// TODO: not sure how this is supposed to interact with opening the file
static FS_node *ext2_vfs_find_dir(FS_node *node, char *name)
{
	uint32_t inode = ext2_find_in_dir(node->inode, name);
	if (inode == 0) {
		return NULL;
	} else {
		FS_node *found = kmalloc(sizeof *found);
		found->inode = inode;

		return found;
	}
}

void ext2_mount(FS_node *mountpoint)
{
	// There shouldn't be anything already mounted
	ASSERT(mountpoint->node_ptr == NULL &&
			(mountpoint->type & MOUNTPOINT_NODE) == 0);
	// We can only mount at a directory
	ASSERT((mountpoint->type & DIR_NODE) != 0);

	FS_node *ext2_root = (FS_node*)kmalloc(sizeof(FS_node));
	ext2_root->name[0]     = '\0';
	ext2_root->permissions = 0;
	ext2_root->uid         = 0;
	ext2_root->gid         = 0;
	ext2_root->inode       = ROOT_INODE;
	ext2_root->length      = 0;
	ext2_root->type        = DIR_NODE;
	ext2_root->read        = ext2_vfs_read;
	ext2_root->write       = ext2_vfs_write;
	ext2_root->open        = ext2_vfs_open;
	ext2_root->close       = ext2_vfs_close;
	ext2_root->read_dir    = ext2_vfs_read_dir;
	ext2_root->find_dir    = ext2_vfs_find_dir;
	ext2_root->node_ptr    = NULL;
	ext2_root->impl        = NULL;

	mountpoint->type |= MOUNTPOINT_NODE;
	mountpoint->node_ptr = ext2_root;
}
