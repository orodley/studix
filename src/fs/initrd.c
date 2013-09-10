// Functions relating to the initial ramdisk

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "kmalloc.h"
#include "initrd.h"

uintptr_t    initrd_start;
File_header *file_headers;
FS_node     *initrd_root;
FS_node     *initrd_dev;
FS_node     *root_nodes;
size_t       num_root_nodes;

static uint32_t initrd_read(FS_node *node, size_t offset,
		size_t size, char *buf)
{
	File_header header = file_headers[node->inode];

	// Can't start reading past the end of the file
	if (offset > header.length)
		return 0;

	// If we would read past the end of the file, just read up to it instead
	if (offset + size > header.length)
		size = header.length - offset;

	memcpy(buf, (uint8_t*)(header.start + offset), size);
	return size;
}

Dir_entry    dir_entry;

static Dir_entry *initrd_read_dir(FS_node *node, size_t index)
{
	if (node == initrd_root && index == 0) {
		strcpy(dir_entry.name, "dev");
		dir_entry.inode = 0;
		return &dir_entry;
	}

	// The actual index is one less, as 0 is occupied by dev
	index--;

	if (index >= num_root_nodes)
		return 0;

	strcpy(dir_entry.name, root_nodes[index].name);
	dir_entry.inode = root_nodes[index].inode;
	return &dir_entry;
}

static FS_node *initrd_find_dir(FS_node *node, char *name)
{
	if (node == initrd_root && !strcmp(name, "dev"))
		return initrd_dev;

	for (size_t i = 0; i < num_root_nodes; i++)
		if (!strcmp(name, root_nodes[i].name))
			return &root_nodes[i];

	return NULL;
}

FS_node *init_initrd(uintptr_t location)
{
	initrd_start = location;
	file_headers = (File_header*)(location + 1);

	// Set up the root directory
	initrd_root  = (FS_node*)kmalloc(sizeof(FS_node));
	strcpy(initrd_root->name, "initrd");
	initrd_root->permissions = 0;
	initrd_root->uid         = 0;
	initrd_root->gid         = 0;
	initrd_root->inode       = 0;
	initrd_root->length      = 0;
	initrd_root->type        = DIR_NODE;
	initrd_root->read        = NULL;
	initrd_root->write       = NULL;
	initrd_root->open        = NULL;
	initrd_root->close       = NULL;
	initrd_root->read_dir    = initrd_read_dir;
	initrd_root->find_dir    = initrd_find_dir;
	initrd_root->node_ptr    = NULL;
	initrd_root->impl        = 0;

	// Set up /dev
	initrd_dev = (FS_node*)kmalloc(sizeof(FS_node));
	strcpy(initrd_root->name, "dev");
	initrd_dev->permissions = 0;
	initrd_dev->uid         = 0;
	initrd_dev->gid         = 0;
	initrd_dev->inode       = 0;
	initrd_dev->length      = 0;
	initrd_dev->type        = DIR_NODE;
	initrd_dev->read        = NULL;
	initrd_dev->write       = NULL;
	initrd_dev->open        = NULL;
	initrd_dev->close       = NULL;
	initrd_dev->read_dir    = initrd_read_dir;
	initrd_dev->find_dir    = initrd_find_dir;
	initrd_dev->node_ptr    = NULL;
	initrd_dev->impl        = 0;

	size_t num_files = *(size_t*)initrd_start;
	root_nodes = (FS_node*)kmalloc(sizeof(FS_node) * num_files);
	num_root_nodes = num_files;

	for (size_t i = 0; i < num_files; i++) {
		// Change the start pointer to be relative to start of memory, rather
		// than the start of the initrd
		file_headers[i].start += location;

		// Create new node for this file
		strcpy(root_nodes[i].name, file_headers[i].name);
		root_nodes[i].permissions = 0;
		root_nodes[i].uid         = 0;
		root_nodes[i].gid         = 0;
		root_nodes[i].length      = file_headers[i].length;
		root_nodes[i].inode       = i;
		root_nodes[i].type        = FILE_NODE;
		root_nodes[i].read        = initrd_read;
		root_nodes[i].write       = NULL;
		root_nodes[i].open        = NULL;
		root_nodes[i].close       = NULL;
		root_nodes[i].read_dir    = NULL;
		root_nodes[i].find_dir    = NULL;
		root_nodes[i].impl        = 0;
	}

	return initrd_root;
}
