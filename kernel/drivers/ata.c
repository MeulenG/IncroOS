#include "ata.h"


int ata_initialize(void) {
    // Is there a device present on the primary channel?
    // Read 0x1F7 and check for 0x00 and 0xFF
    uint8_t status = inb(0x1F7);
    if (status == 0x00 || status == 0xFF) {
        vesa_terminal_writestring("No device present on primary channel.\n", VESA_COLOR_RED);
        return -1;
    }
    else {
        vesa_terminal_writestring("Device detected on primary channel.\n", VESA_COLOR_GREEN);
        // Send IDENTIFY command (0xEC) to port 0x1F7
        outb(0x1F7, 0xEC);
        // Wait for the device to be ready
        ata_poll();
        // Read and discard
        for (int i = 0; i < 256; i++) {
            inw(0x1F0);
        }
    }
    return 0; 
}

int ata_poll(void) {
    // Polling for the drive to be ready
    int flag = 0;
    while (1) {
        uint8_t status = inb(0x1F7);
        if (!(status & 0x80) && (status & 0x08)) {
            if (status & 0x01 || status & 0x20) {
                vesa_terminal_writestring("Drive error detected.\n", VESA_COLOR_RED);
                flag = -1;
                break;
            } else {
                vesa_terminal_writestring("Drive is ready.\n", VESA_COLOR_GREEN);
            }
            // Drive is ready
            flag = 0;
            break;
        }
    }
    return flag;
}

void ata_read_sector(uint8_t drive, uint32_t lba, uint8_t sector_count, uint8_t* buffer) {
    // Send 0xE0 for master drive, 0xF0 for slave drive
    outb(0x1F6, (drive == 0 ? 0xE0 : 0xF0) | ((lba >> 24) & 0x0F));
    // Send a null byte
    outb(0x1F1, 0x00);
    // Send sector count to port 0x1F2
    outb(0x1F2, sector_count);
    // Send LBA low, mid, high bytes to ports 0x1F3
    // LBA low
    outb(0x1F3, (uint8_t)(lba & 0xFF));
    // LBA mid
    outb(0x1F4, (uint8_t)((lba >> 8) & 0xFF));
    // LBA high
    outb(0x1F5, (uint8_t)((lba >> 16) & 0xFF));
    // Send the read command (0x20) to port 0x1F7
    outb(0x1F7, 0x20);
    for (uint8_t i = 0; i < sector_count; i++) {
        if (ata_poll() != 0) {
            vesa_terminal_writestring("Error during sector read.\n", VESA_COLOR_RED);
            return;
        }
        for (uint16_t j = 0; j < 256; j++) {
            uint16_t data = inw(0x1F0);
            buffer[i * 512 + j * 2] = (uint8_t)(data & 0xFF); // Low byte
            buffer[i * 512 + j * 2 + 1] = (uint8_t)((data >> 8) & 0xFF); // High byte
        }
    }
}

void ata_write_sector(uint8_t drive, uint32_t lba, uint8_t sector_count, const uint8_t* buffer) {
    
}