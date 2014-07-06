// Virtual filesystem stuff
// TODO: Lots of the types here should be changed, e.g.: uint32_t -> ssize_t

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_NAME_LENGTH 128 // Maximum length of a filename

typedef enum Node_type
{
	FILE_NODE       = 1,
	DIR_NODE,
	CHAR_DEV_NODE,
	BLOCK_DEV_NODE,
	PIPE_NODE,
	SYMLINK_NODE,
	MOUNTPOINT_NODE = 8 // We want to OR with DIR_NODE sometimes
} Node_type;

struct FS_node;
struct Dir_entry;

// Standard file function types
typedef uint32_t (*Read) (struct FS_node*, size_t, size_t, char*);
typedef uint32_t (*Write)(struct FS_node*, size_t, size_t, char*);
typedef void     (*Open) (struct FS_node*);
typedef void     (*Close)(struct FS_node*);

typedef struct Dir_entry *(*Read_dir)(struct FS_node*, size_t);
typedef struct FS_node   *(*Find_dir)(struct FS_node*, char*);

typedef struct FS_node
{
	char            name[MAX_NAME_LENGTH];
	uint32_t        length;
	uint32_t        inode;
	uint32_t        permissions;
	uint32_t        uid;
	uint32_t        gid;
	Node_type       type;

	// Used in symlinks and mountpoints
	struct FS_node *node_ptr;

	// FS implementation specific stuff
	void            *impl;

	// Standard file functions
	Read            read;
	Write           write;
	Open            open;
	Close           close;
	Read_dir        read_dir;
	Find_dir        find_dir;
} FS_node;

typedef struct Dir_entry
{
	char     name[MAX_NAME_LENGTH];
	uint32_t inode;
} Dir_entry;

extern FS_node *fs_root; // The root of the filesystem

// Wrapper functions for calling file functions on an FS_node
uint32_t    read_fs_node(FS_node *node, size_t offset, size_t size, char *buf);
uint32_t   write_fs_node(FS_node *node, size_t offset, size_t size, char *buf);
void        open_fs_node(FS_node *node, bool read, bool write);
void       close_fs_node(FS_node *node);
Dir_entry *read_dir_node(FS_node *node, size_t index);
FS_node   *find_dir_node(FS_node *node, char *name);
