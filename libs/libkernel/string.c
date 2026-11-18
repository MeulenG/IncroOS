#include "string.h"

extern size_t strlen(const char* str);


int strcmp(const char* str1, const char* str2) {
    // Start by comparing the ascii values of the characters in both strings
    int i = 0;
    while (str1[i] != '\0' && str2[i] != '\0') {
        int ascii1 = (int)str1[i];
        int ascii2 = (int)str2[i];

        if (ascii1 != ascii2) {
            kprintf("Strings differ at index %d: %c vs %c\n", i, str1[i], str2[i]);
            return ascii1 - ascii2;
        }
        i++;
    }
    kprintf("Returning difference: %d\n", (unsigned char)str1[i] - (unsigned char)str2[i]);
    return (unsigned char)str1[i] - (unsigned char)str2[i];
}

// The mem functions use rep movsb/stosb, so -O2 cannot turn them back into calls to themselves
void* memcpy(void* dest, const void* src, size_t n) {
    void* ret = dest;
    __asm__ volatile ("rep movsb" : "+D"(dest), "+S"(src), "+c"(n) : : "memory");
    return ret;
}

void* memmove(void* dest, const void* src, size_t n) {
    if ((unsigned char*)dest <= (const unsigned char*)src || (const unsigned char*)src + n <= (unsigned char*)dest) {
        return memcpy(dest, src, n);
    }
    // dest overlaps the end of src, so copy backwards. Interrupts stay off while the
    // direction flag is set, handlers expect it clear.
    unsigned char* d = (unsigned char*)dest + n - 1;
    const unsigned char* s = (const unsigned char*)src + n - 1;
    __asm__ volatile ("pushfq\n\tcli\n\tstd\n\trep movsb\n\tcld\n\tpopfq"
                      : "+D"(d), "+S"(s), "+c"(n) : : "memory", "cc");
    return dest;
}

void* memset(void* dest, int value, size_t n) {
    void* ret = dest;
    __asm__ volatile ("rep stosb" : "+D"(dest), "+c"(n) : "a"(value) : "memory");
    return ret;
}
