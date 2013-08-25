// String functions

#include <stddef.h>
#include <stdint.h>

void *memset(void *s, int c, size_t n)
{
	for (size_t i = 0; i < n; i++)
		((uint8_t*)s)[i] = c;

	return s;
}

size_t strlen(const char *str)
{
	size_t len = 0;
	while (str[len++] != '\0')
		;

	return len;
}
