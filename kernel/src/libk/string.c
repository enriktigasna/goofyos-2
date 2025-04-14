#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t count)
{    
        uint8_t *tmp = dest;
        while (count--)
                *tmp++ = *(uint8_t*)src++;

        return dest;
}

void *memset(void *dest, int c, size_t count)
{    
        uint8_t *tmp = dest;
        while(count--)
                *tmp++ = (uint8_t)c;

        return dest;
}
