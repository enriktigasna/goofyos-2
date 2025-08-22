#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *dest, int c, size_t n);
void *memmove(void *dstptr, const void *srcptr, size_t size);
int strncmp(char *str1, char *str2, int n);
int strlen(char *str1);
int strcpy(char *str1, char *str2);