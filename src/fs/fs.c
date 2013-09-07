// Filesystem functions

#include <stddef.h>
#include "fs.h"

FS_node *fs_root = NULL;

uint32_t read_fs_node(FS_node *node, size_t offset, size_t size, char *buf)
{
	if (node->read != NULL)
		return node->read(node, offset, size, buf);
	else
		return 0;
}

uint32_t write_fs_node(FS_node *node, size_t offset, size_t size, char *buf)
{
	if (node->write != NULL)
		return node->write(node, offset, size, buf);
	else
		return 0;
}

void open_fs_node(FS_node *node, bool read, bool write)
{
	if (node->open != NULL)
		node->open(node);
}

void close_fs_node(FS_node *node)
{
	if (node->close != NULL)
		node->close(node);
}

Dir_entry *read_dir_node(FS_node *node, size_t index)
{
	if ((node->type & DIR_NODE) != 0 && node->read_dir != NULL)
		return node->read_dir(node, index);
	else
		return NULL;
}

FS_node *find_dir_node(FS_node *node, char *name)
{
	if ((node->type & DIR_NODE) != 0 && node->find_dir != NULL)
		return node->find_dir(node, name);
	else
		return NULL;
}
