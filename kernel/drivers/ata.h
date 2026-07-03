#ifndef __ATA_H__
#define __ATA_H__

#include <stdint.h>
// Will need this for the 400ns delay
#include "../cpu/io.h"
#include "../output/vesa_terminal.h"

int ata_initialize(void);
int ata_poll(void);
void ata_read_sector(uint8_t drive, uint32_t lba, uint8_t sector_count, uint8_t* buffer);
void ata_write_sector(uint8_t drive, uint32_t lba, uint8_t sector_count, const uint8_t* buffer);


#endif
