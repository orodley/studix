void init_ata();

// TODO: remove this once a higher level interface is available
void read_abs_sectors(uint32_t lba, uint8_t sector_count, uint16_t buf[]);
