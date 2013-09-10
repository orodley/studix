// String functions

#include <stddef.h>
#include <stdint.h>

void *memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; i++)
		((uint8_t*)s)[i] = c;

	return s;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	uint8_t *dest8 = (uint8_t*)dest;
	uint8_t *src8  = (uint8_t*)src;

	for (size_t i = 0; i < n; i++)
		dest8[i] = src8[i];

	return dest;
}

char *strcpy(char *dest, const char *src)
{
	size_t i;
	for (i = 0; src[i] != '\0'; i++)
		dest[i] = src[i];

	dest[++i] = '\0';
	return dest;
}

int strcmp(const char *s1, const char *s2)
{
	for (size_t i = 0;; i++) {
		if (s1[i] < s2[i])
			return -1;
		if (s1[i] > s2[i])
			return  1;
		if (s1[i] == '\0')
			return 0;
	}
}

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len++] != '\0')
		;

	return len;
}
