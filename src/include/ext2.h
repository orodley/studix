#include <stdint.h>

// Structure of an ext2 superblock
// NOTE: The values in the superblock header are stored little-endian, which
// happens to work fine for us since we're on x86, but this won't work as is
// on big-endian architectures!
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

typedef struct BGD
{
	uint32_t block_bitmap_id;
	uint32_t inode_bitmap_id;
	uint32_t inode_table_id;
	uint16_t free_blocks;
	uint16_t free_inodes;
	uint16_t used_dirs;
	uint16_t pad;
	uint8_t  bg_reserved[12];
} __attribute__ ((packed)) BGD;

void init_fs();
