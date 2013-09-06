// Interface for ordered arrays
#include <stddef.h>
#include <stdint.h>

typedef int8_t (*Comparer)(void*, void*);

typedef struct Ordered_array
{
	void   **array;
	size_t   size;
	size_t   max_size;
	Comparer comparer;
} Ordered_array;

int8_t std_comparer(void*, void*);

Ordered_array create_ordered_array(size_t max_size, Comparer comparer);
Ordered_array place_ordered_array(void *addr, size_t max_size,
		Comparer comparer);

void  delete_ordered_array(Ordered_array);
void  insert_ordered_array(void *item, Ordered_array *array);
void *lookup_ordered_array(size_t i, Ordered_array *array);
void  remove_ordered_array(size_t i, Ordered_array *array);
