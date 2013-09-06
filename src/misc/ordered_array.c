#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "assert.h"
#include "kmalloc.h"
#include "ordered_array.h"
#include "term.h"

int8_t std_comparer(void *a, void *b)
{
	return (a < b) ? 1 : 0;
}

Ordered_array create_ordered_array(size_t max_size, Comparer comparer)
{
	Ordered_array result;

	size_t size = max_size * sizeof(void*);
	result.array = (void*)kmalloc(size);
	memset(result.array, 0, size);

	result.size     = 0;
	result.max_size = max_size;
	result.comparer = comparer;

	return result;
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

void insert_ordered_array(void *item, Ordered_array *array)
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

void *lookup_ordered_array(size_t i, Ordered_array *array)
{
	ASSERT(i < array->size);
	return array->array[i];
}

void remove_ordered_array(size_t i, Ordered_array *array)
{
	for (; i < array->size; i++)
		array->array[i] = array->array[i + 1];

	array->size++;
}
