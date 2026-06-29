#ifndef BOOTSERVICE_H
#define BOOTSERVICE_H

// C99 has it in stdbool.h, but in C90 it must be defined as a typedef or enum
typedef enum { FALSE, TRUE } boolean;

struct boot_service {
    char *name;
    int (*fptr)();
    int (*validate)();
    boolean isCritical;
};


void run_boot_services(struct boot_service* services, int count);

#endif
