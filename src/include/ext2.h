#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// First, structures and constants corresponding to stuff in the ext2 spec
#define ROOT_INODE 2

// NOTE: The values in the superblock header are stored little-endian and read
// in from the drive in 16-bit units. This happens to work fine for us since
// we're on x86, but this won't work as is on big-endian architectures!
typedef struct Ext2_superblock
{
	uint32_t total_inodes;
	uint32_t total_blocks;
	uint32_t su_res_blocks;   // How many blocks are reserved for the superuser
	uint32_t free_blocks;
	uint32_t free_inodes;
	uint32_t superblock_block_num;
	uint32_t log2_block_size; // (1024 << log2_block_size) = block    size
	uint32_t log2_frag_size;  // (1024 << log2_frag_size)  = fragment size
	uint32_t blocks_per_group;
	uint32_t frags_per_group;
	uint32_t inodes_per_group;
	uint32_t last_mount_time;
	uint32_t last_write_time;
	uint16_t mounts_since_fsck;
	uint16_t max_mounts_since_fsck;
	uint16_t signature;       // Should always be (EXT2_SIGNATURE)
	uint16_t state;
	uint16_t error_action;
	uint16_t minor_version;
	uint32_t last_fsck_time;
	uint32_t max_time_since_fsck;
	uint32_t creator_OS_id;
	uint32_t major_version;
	uint16_t res_block_uid;
	uint16_t res_block_gid;
} __attribute__ ((packed)) Ext2_superblock;

// Block group descriptor
typedef struct BGD
{
	uint32_t block_bitmap_addr;
	uint32_t inode_bitmap_addr;
	uint32_t inode_table_addr;
	uint16_t free_blocks;
	uint16_t free_inodes;
	uint16_t used_dirs;
	uint16_t pad;
	uint8_t  bg_reserved[12];
} __attribute__ ((packed)) BGD;

typedef struct Ext2_inode
{
	// Bitwise OR of the two groups of #define's below this struct decl
	uint16_t type_and_permissions;
	// User ID of the file's owner
	uint16_t uid;
	// Size of the corresponding file in bytes
	uint32_t size;

	// All the fields below which refer to a time use standard POSIX time
	uint32_t last_access_time;
	uint32_t creation_time;
	uint32_t last_modification_time;
	uint32_t deletion_time;

	// The group ID of the group that has access to the file
	uint16_t gid;
	// How many hard links point to this file. Once it reaches zero, the
	// blocks occupied by this file are marked as unallocated
	uint16_t link_count;
	// How many *sectors* are taken up by this file - not blocks
	uint32_t sectors_used;
	// Some bitwise OR combination of the flags listed below
	uint32_t flags;
	// Currently unused
	uint32_t os_specific_val1;
	// Direct block pointers; pointers to blocks that contain this file's data
	uint32_t dbp[12];
	// Indirect block pointer; pointer to a block containing DBPs
	uint32_t ibp;
	// Doubly indirect block pointer; pointer to a block containing IBPs
	uint32_t dibp;
	// Trebly indirect block pointer; pointer to a block containing DIBPs
	uint32_t tibp;
	// File version, used by NFS
	uint32_t gen_number;

	// As we're only doing version 0 (for now), these two values are reserved
	uint32_t reserved1;
	uint32_t reserved2;

	// Location of the file fragment; probably won't be used
	uint32_t fragment_addr;
	// Currently unused
	uint8_t  os_specific_val2[12];
} __attribute__ ((packed)) Ext2_inode;

// Type of file; occupies the top hex digit of type_and_permissions
// TODO: Unify this with the vfs stuff in vfs.h somehow?
#define S_IFIFO  0x1000
#define S_IFCHR  0x2000
#define S_IFDIR  0x4000
#define S_IFBLK  0x6000
#define S_IFREG  0x8000
#define S_IFLNK  0xA000
#define S_IFSOCK 0xC000

// File permissions: occupies the other 3 hex digits of type_and_permissions
// This should possibly be moved to <sys/stat.h> once it exists, or perhaps
// kept separate as it is a FS-dependent thing that happens to be the same in
// this case, and may be different for some future FS
#define S_ISUID  04000
#define S_ISGID  02000
#define S_ISTICK 01000
#define S_IRUSR  00400
#define S_IWUSR  00200
#define S_IXUSR  00100
#define S_IRGRP  00040
#define S_IWGRP  00020
#define S_IXGRP  00010
#define S_IROTH  00004
#define S_IWOTH  00002
#define S_IXOTH  00001

// Flags; Ext2_inode.flags is a bitwise OR combination of these
#define SECURE_DELETE    0x00001
#define UNDELETE         0x00002
#define COMPRESSED       0x00004
#define SYNCRONOUS       0x00008
#define IMMUTABLE        0x00010
#define APPEND_ONLY      0x00020
#define DUMP_IGNORE      0x00040
#define NO_UPDATE_ACCESS 0x00080
// ...
// Various other flags are defined that I don't care about for now. I'll bother
// adding them if I ever support them

// Directory entry; a directory file's data is composed of these
typedef struct Ext2_dirent
{
	uint32_t  inode_num;
	uint16_t  total_len;
	uint8_t   name_len;
	uint8_t   type_indicator;
	// The filesystem doesn't actually contain a pointer of course, but as it
	// is variable length this is best we can do
	uint8_t  *name;
} __attribute__ ((packed)) Ext2_dirent;


// Implementation-specific stuff

// Represents an open file handle
// TODO: support reading from indirect (and doubly-/trebly- so) blocks. At the
// moment we only support files < 12 * block_size bytes; somewhere between
// 12KiB and 49KiB
typedef struct Ext2_file
{
	// Inode of the file that's open
	Ext2_inode inode;
	// Current position in the whole file, in bytes
	size_t     pos;
	// Index of the block we're currently in
	uint8_t    block_index;
	// Buffer containing the data in the current block
	uint8_t   *buf;
	// Position in the current block
	size_t     curr_block_pos;
} Ext2_file;

void ext2_init_fs();
void ext2_open_inode(uint32_t inode_num, Ext2_file *file);
size_t ext2_read(Ext2_file *file, uint8_t *buf, size_t count);
bool ext2_next_dirent(Ext2_file *file, Ext2_dirent *dir);
uint32_t ext2_find_in_dir(uint32_t dir_inode, const char *name);
uint32_t ext2_look_up_path(char *path);
