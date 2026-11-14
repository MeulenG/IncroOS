#ifndef __INCRO_H__
#define __INCRO_H__

#include <stdint.h>

// Compiler attributes
#define IncroInterrupt              __attribute__((interrupt))
#define IncroPacked                 __attribute__((packed))
#define IncroNoReturn               __attribute__((noreturn))
#define IncroUnused                 __attribute__((unused))

// Status codes
typedef int IncroStatus;
#define INCRO_RET_OK                0
#define INCRO_RET_FAIL              -1
#define INCRO_RET_NOT_FOUND         -2
#define INCRO_RET_NOT_SUPPORTED     -3
#define INCRO_RET_NO_MEMORY         -4
#define INCRO_RET_INVALID           -5
#define INCRO_RET_NOT_READY         -6
#define INCRO_RET_ALREADY_INIT      -7
#define INCRO_RET_NOT_INIT          -8

// CPU control
#define INCRO_DISABLE_INTERRUPTS    __asm__ volatile("cli")
#define INCRO_ENABLE_INTERRUPTS     __asm__ volatile("sti")
#define INCRO_HALT                  __asm__ volatile("hlt")
#define KERNEL_PANIC()              do { __asm__ volatile("cli"); for(;;) { __asm__ volatile("hlt"); } } while(0)

// Boot services
#define INCRO_SERVICE_CRITICAL      INCRO_TRUE
#define INCRO_SERVICE_OPTIONAL      INCRO_FALSE

// Utility
#define INCRO_ARRAY_SIZE(x)         (sizeof(x) / sizeof((x)[0]))
#define INCRO_KB(x)                 ((x) * 1024ULL)
#define INCRO_MB(x)                 ((x) * 1024ULL * 1024ULL)
#define INCRO_GB(x)                 ((x) * 1024ULL * 1024ULL * 1024ULL)

#endif
