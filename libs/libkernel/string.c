#include "string.h"

extern size_t strlen(const char* str);


int strncmp(const char* str1, const char* str2) {
    // Compare two strings character by character
    // In C style
    // The way C compares strings is by comparing the ASCII values of the characters in the strings
    // Until the non-matching character is found or the NULL terminator is reached
    while (*(int*)str1 && (*(int*)str1 == *(int*)str2)) {
        str1++;
        str2++;
        if (*str1 == '\0' && *str2 == '\0') {
            return 0; // Strings are equal
        }
        else if (*str1 == '\0') {
            return -1; // str1 is shorter than str2
        }
        else if (*str2 == '\0') {
            return 1; // str2 is shorter than str1
        }
    }
    return (unsigned char)(*str1) - (unsigned char)(*str2);
}
