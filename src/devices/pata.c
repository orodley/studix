// Parallel ATA (aka IDE) driver

#include <stdbool.h>
#include <stdint.h>
#include "assert.h"
#include "dev.h"
#include "term.h"
#include "pata.h"

#define LBA_BITS 28

// Port bases and offsets
#define PRIMARY_BASE   0x1F0
#define SECONDARY_BASE 0x170
#define DATA               0
#define ERROR              1
#define SECTOR_COUNT       2
#define LBA_LOW            3
#define LBA_MID            4
#define LBA_HIGH           5
#define DRIVE_SELECT       6
#define COM_STAT           7

#define PRI_CONTROL    0x3F6
#define SEC_CONTROL    0x376


// Commands
#define SEL_MASTER   0xA0
#define SEL_SLAVE    0xB0
#define IDENTIFY     0xEC
#define READ_SECTORS 0x20


// Status byte flags
#define ERR (1 << 0)
#define DRQ (1 << 3)
#define SRV (1 << 4)
#define DF  (1 << 5)
#define RDY (1 << 6)
#define BSY (1 << 7)

// Interesting indices into the data returned by IDENTIFY
#define MAX_28LBA_SECTORS 60


// Current drive state information

// We can only use one drive at a time for the moment, and these vars tell us
// if the one we want is on the primary or secondary bus, and whether we want
// the master or slave
static uint16_t sel_base_port       = 0;
static uint8_t  sel_master_or_slave = 0;

static uint32_t max_sector;


static uint8_t read_stat(uint16_t base)
{
	// We have to allow for a 400ns delay, so read it 5 times and discard
	// the first four values
	inb(base + COM_STAT);
	inb(base + COM_STAT);
	inb(base + COM_STAT);
	inb(base + COM_STAT);

	return inb(base + COM_STAT);
}

static void check_drive(uint16_t base, uint8_t master_or_slave)
{
	if (sel_base_port != 0) // Check if we've already found a drive
		return;

	outb(base + DRIVE_SELECT, master_or_slave); // select the drive

	// Zero out these 4 ports
	outb(base + SECTOR_COUNT, 0);
	outb(base + LBA_LOW,      0);
	outb(base + LBA_MID,      0);
	outb(base + LBA_HIGH,     0);

	outb(base + COM_STAT,  IDENTIFY); // Send IDENTIFY command
	uint8_t stat = read_stat(base);

	if (stat == 0) // The drive does not exist
		return;

	// It exists! Retrieve some status info

	while ((stat & BSY) != 0) // Poll status port until BSY clears
		stat = read_stat(base);

	// TODO: check for ATAPI at this point

	while ((stat & DRQ) == 0 && (stat & ERR) == 0)
		stat = read_stat(base);

	if ((stat & ERR) != 0) // Something's wrong. Maybe it's ATAPI or SATA
		return;

	// The drive is ready to send us 256 (16-bit) words of data
	uint16_t drive_data[256];
	for (size_t i = 0; i < 256; i++)
		drive_data[i] = inw(base + DATA);

	max_sector = drive_data[MAX_28LBA_SECTORS] |
		drive_data[MAX_28LBA_SECTORS + 1] << 16;

	// This drive seems to work, so let's select it
	sel_base_port       = base;
	sel_master_or_slave = master_or_slave;
}

void init_ata()
{
	// First, check for a floating bus (no drives attached)
	uint8_t pri_status = inb(PRIMARY_BASE   + COM_STAT);
	uint8_t sec_status = inb(SECONDARY_BASE + COM_STAT);
	bool    primary_floating   = false;
	bool    secondary_floating = false;
	if (pri_status == 0xFF)
		primary_floating = true;
	if (sec_status == 0xFF)
		secondary_floating = true;

	// Both buses are floating
	if (primary_floating && secondary_floating) {
		term_puts("No drives attached! What's going on?");
		return;
	}

	// Non-0xFF values are not definitive; we need to do some more checks
	check_drive(PRIMARY_BASE,   SEL_MASTER);
	check_drive(PRIMARY_BASE,   SEL_SLAVE);
	check_drive(SECONDARY_BASE, SEL_MASTER);
	check_drive(SECONDARY_BASE, SEL_SLAVE);

	if (sel_base_port == 0) // We didn't find a (PATA) drive
		term_puts("No drives attached! What's going on?");
	else {
		term_printf("Found a drive!\nSelected drive is the %s on the %s bus\n",
				sel_master_or_slave == SEL_MASTER ? "master"  : "slave",
				sel_base_port == PRIMARY_BASE     ? "primary" : "secondary");
		term_printf("Max LBA value is %d\n", max_sector);
	}
}

static void poll()
{
	uint8_t stat;

	do
		stat = read_stat(sel_base_port);
	while ((stat & BSY) != 0); // TODO: Error detection
}

// Read sector_count sectors into a buffer, using 28 bit absolute LBA.
// Buffer must be at least sector_count * SECTOR_SIZE bytes long.
void read_abs_sectors(uint32_t lba, uint8_t sector_count, uint16_t buf[])
{
	// Sanity check; the address shouldn't be more than LBA_BITS bits long
	ASSERT(lba >> LBA_BITS == 0);

	// First, send a drive select OR'd with the 4 MSB of the address, with bit
	// 6 set, to indicate this is LBA
	outb(sel_base_port + DRIVE_SELECT,
			(lba >> (LBA_BITS - 4)) | sel_master_or_slave | 1 << 6);
	outb(sel_base_port + SECTOR_COUNT, sector_count); // Send the sector count

	// Now send the 24 LSB of the LBA, in 3 1-byte chunks
	outb(sel_base_port + LBA_LOW,   lba        & 0xFF);
	outb(sel_base_port + LBA_MID,  (lba >> 8)  & 0xFF);
	outb(sel_base_port + LBA_HIGH, (lba >> 16) & 0xFF);

	// Issue the READ_SECTORS command
	outb(sel_base_port + COM_STAT, READ_SECTORS);

	size_t i = 0;
	for (; sector_count > 0; sector_count--) {
		poll();

		__asm__ volatile ("rep insw" :: "c"(SECTOR_SIZE / 2),      // Count
		                                "d"(sel_base_port + DATA), // Port #
									    "D"(buf + i));             // Buffer
		i += SECTOR_SIZE / 2;
	}
}
