// Types for the initial ramdisk filesystem

#include <stddef.h>
#include <stdint.h>
#include "fs.h"

#define MAX_FILES 64

typedef struct File_header
{
	char   name[MAX_NAME_LENGTH];
	size_t start;
	size_t length;
} File_header;

FS_node *init_initrd(uintptr_t location);
