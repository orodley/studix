// Parallel ATA (aka IDE) driver

#include <stdbool.h>
#include <stdint.h>
#include "dev.h"
#include "term.h"

static const uint16_t PRIMARY_BASE   = 0x1F0;
static const uint16_t SECONDARY_BASE = 0x170;
static const uint16_t DATA           =     0;
static const uint16_t ERROR          =     1;
static const uint16_t SECTOR_COUNT   =     2;
static const uint16_t SECTOR_NUM     =     3;
static const uint16_t CYL_LOW        =     4;
static const uint16_t CYL_HIGH       =     5;
static const uint16_t DRIVE_SELECT   =     6;
static const uint16_t COM_STAT       =     7;

static const uint16_t PRI_CONTROL    = 0x3F6;
static const uint16_t SEC_CONTROL    = 0x376;

// Current state information
static bool primary_has_drives;
static bool secondary_has_drives;

void init_ata()
{
	// First, check for a floating bus (no drives attached)
	uint8_t pri_status = inb(PRIMARY_BASE   + COM_STAT);
	uint8_t sec_status = inb(SECONDARY_BASE + COM_STAT);
	if (pri_status == 0xFF)
		primary_has_drives   = false;
	if (sec_status == 0xFF)
		secondary_has_drives = false;

	if (!primary_has_drives && !secondary_has_drives)
		term_puts("No drives attached! What's going on?");
}
