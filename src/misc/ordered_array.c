#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "assert.h"
#include "kmalloc.h"
#include "ordered_array.h"
#include "term.h"

bool std_comparer(void *a, void *b)
{
	return (a < b);
}

Ordered_array create_ordered_array(size_t max_size, Comparer comparer)
{
	size_t size = max_size * sizeof(void*);
	return place_ordered_array((void*)kmalloc(size), max_size, comparer);
}

Ordered_array place_ordered_array(void *addr, size_t max_size,
		Comparer comparer)
{
	Ordered_array result;

	result.array = addr;
	memset(addr, 0, max_size * (sizeof(void*)));

	result.size     = 0;
	result.max_size = max_size;
	result.comparer = comparer;

	return result;
}

void ordered_array_insert(Ordered_array *array, void *item)
{
	ASSERT(array->comparer);

	size_t i = 0;
	while (i < array->size && array->comparer(array->array[i], item))
		i++;

	if (i == array->size) {
		array->array[array->size++] = item; // Insert at end
	} else {
		// Shift everything along
		void *tmp = array->array[i];
		array->array[i] = item;

		while (i < array->size) {
			i++;
			void *tmp2 = array->array[i];
			array->array[i] = tmp;
			tmp = tmp2;
		}

		array->size++;
	}
}

void *ordered_array_lookup(Ordered_array *array, size_t i)
{
	ASSERT(i < array->size);
	return array->array[i];
}

void ordered_array_remove(Ordered_array *array, size_t i)
{
	for (; i < array->size; i++)
		array->array[i] = array->array[i + 1];

	array->size++;
}
