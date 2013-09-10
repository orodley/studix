// Standard structure for info passed to the kernel by multiboot

typedef struct Multiboot_info
{
	unsigned int memp           :  1;
	unsigned int boot_device_p  :  1;
	unsigned int cmdlinep       :  1;
	unsigned int aoutp          :  1;
	unsigned int elfp           :  1;
	unsigned int mmapp          :  1;
	unsigned int drivesp        :  1;
	unsigned int config_table_p :  1;
	unsigned int                : 24;
	uint32_t     mem_lower;
	uint32_t     mem_upper;
	uint32_t     boot_device;
	uint32_t     cmdline;
	uint32_t     module_count;
	uint32_t     modules_addr;
	uint32_t     num;
	uint32_t     size;
	uint32_t     addr;
	uint32_t     shndx;
	uint32_t     mmap_length;
	uint32_t     mmap_addr;
	uint32_t     drives_length;
	uint32_t     drives_addr;
	uint32_t     config_table_addr;
	uint32_t     bootloader_name_addr;
	uint32_t     apm_table_addr;
	uint32_t     vbe_control_info;
	uint32_t     vbe_mode_info;
	uint32_t     vbe_mode;
	uint32_t     vbe_interface_seg;
	uint32_t     vbe_interface_off;
	uint32_t     vbe_interface_len;
} __attribute__((packed)) Multiboot_info;
