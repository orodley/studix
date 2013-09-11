// Interface for ordered arrays

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool (*Comparer)(void*, void*);

typedef struct Ordered_array
{
	void   **array;
	size_t   size;
	size_t   max_size;
	Comparer comparer;
} Ordered_array;

bool std_comparer(void*, void*);

Ordered_array create_ordered_array(size_t max_size, Comparer comparer);
Ordered_array place_ordered_array(void *addr, size_t max_size,
		Comparer comparer);

void  delete_ordered_array(Ordered_array);
void  ordered_array_insert(Ordered_array *array, void *item);
void *ordered_array_lookup(Ordered_array *array, size_t i);
void  ordered_array_remove(Ordered_array *array, size_t i);
