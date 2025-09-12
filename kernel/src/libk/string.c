#include <goofy-os/slab.h>
#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t count) {
	uint8_t *tmp = dest;
	while (count--)
		*tmp++ = *(uint8_t *)src++;

	return dest;
}

void *memset(void *dest, int c, size_t count) {
	uint8_t *b_ptr = dest;
	uint64_t *q_ptr = dest;

	uint64_t pattern;
	pattern = (uint8_t)c;
	pattern |= pattern << 8;
	pattern |= pattern << 16;
	pattern |= pattern << 32;

	while (count >= 8) {
		*q_ptr++ = pattern;
		count -= 8;
	}

	while (count--)
		*b_ptr++ = (uint8_t)c;

	return dest;
}

void *memmove(void *dstptr, const void *srcptr, size_t size) {
	unsigned char *dst = (unsigned char *)dstptr;
	const unsigned char *src = (const unsigned char *)srcptr;
	if (dst < src) {
		for (size_t i = 0; i < size; i++)
			dst[i] = src[i];
	} else {
		for (size_t i = size; i != 0; i--)
			dst[i - 1] = src[i - 1];
	}
	return dstptr;
}

int strncmp(char *str1, char *str2, int n) {
	for (int i = 0; i < n; i++) {
		unsigned char c1 = str1[i];
		unsigned char c2 = str2[i];
		if (c1 != c2)
			return c1 - c2;

		if (c1 == '\0')
			return 0;
	}
	return 0;
}

int strlen(char *str) {
	int i;
	for (i = 0; str[i]; i++)
		;
	return i;
}

int strcmp(const char *str1, const char *str2) {
	while (*str1 && *str1 == *str2) {
		str1++;
		str2++;
	}
	return (unsigned char)*str1 - (unsigned char)*str2;
}

int memcmp(char *str1, char *str2, int n) {
	for (int i = 0; i < n; i++) {
		if (str1[i] < str2[i])
			return -1;
		if (str1[i] > str2[i])
			return 1;
	}
	return 0;
}

int strcpy(char *str1, char *str2) {
	int len = strlen(str2);
	memcpy(str1, str2, len);
	str1[len] = '\0';
	return len;
}

char *strdup(char *str) {
	char *new = kmalloc(strlen(str));
	strcpy(new, str);
	return new;
}
