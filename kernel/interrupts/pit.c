#include "interrupts.h"

// PIT I/O ports
#define PIT_CHANNEL0    0x40
#define PIT_CHANNEL1    0x41
#define PIT_CHANNEL2    0x42
#define PIT_COMMAND     0x43

// PIT frequency (input clock frequency)
#define PIT_FREQUENCY   1193182

// I/O port operations
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// External function to enable IRQ0
extern void pic_enable_irq(uint8_t irq);

// Initialize PIT with specified frequency
void pit_init(uint32_t frequency) {
    // Calculate divisor
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Ensure divisor is within valid range
    if (divisor > 65535) divisor = 65535;
    if (divisor < 1) divisor = 1;

    // Command byte:
    // Bits 6-7: Channel (00 = channel 0)
    // Bits 4-5: Access mode (11 = lobyte/hibyte)
    // Bits 1-3: Operating mode (011 = square wave generator)
    // Bit 0: BCD mode (0 = binary)
    uint8_t command = 0x36;  // 00110110b
    
    outb(PIT_COMMAND, command);
    
    // Send divisor (low byte then high byte)
    outb(PIT_CHANNEL0, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0, (uint8_t)((divisor >> 8) & 0xFF));
    
    // Enable IRQ0 (timer interrupt)
    pic_enable_irq(0);
}
