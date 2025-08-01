#include <stddef.h>
#include <stdint.h>

void *memcpy(void *dest, const void *src, size_t count) {
	uint8_t *tmp = dest;
	while (count--)
		*tmp++ = *(uint8_t *)src++;

	return dest;
}

void *memset(void *dest, int c, size_t count) {
	uint8_t *tmp = dest;
	while (count--)
		*tmp++ = (uint8_t)c;

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
