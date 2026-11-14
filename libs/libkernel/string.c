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
