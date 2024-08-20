#include <string.h>

#include "util.h"
#include "vector.h"

void vector_mk(struct vector *dest, size_t elem, size_t size)
{
	assert(elem > 0);
	dest->data = size ? malloc(elem * size) : NULL;
	dest->elem = elem;
	dest->size = size;
	dest->capacity = elem * size;
}
void vector_free(struct vector *arr)
{
	free(arr->data);
}
void vector_clear(struct vector *arr)
{
	free(arr->data);
	arr->data = NULL;
	arr->size = 0;
	arr->capacity = 0;
}
void vector_recapacitate(struct vector *arr)
{
	size_t bytes = arr->elem * arr->size;
	if (bytes > arr->capacity)
	{
		arr->capacity = size_max(bytes, 2 * arr->capacity);
	}
	else if (bytes * 4 < arr->capacity)
	{
		arr->capacity = bytes;
	}
	else
	{
		return;
	}
	// FIX add error handling for insufficient memory
	if (arr->capacity == 0)
	{
		free(arr->data);
		arr->data = NULL;
	}
	else
	{
		arr->data = realloc(arr->data, arr->capacity);
	}
}
void *vector_subscript(struct vector *arr, size_t index)
{
	return arr->data + arr->elem * index;
}
void const *vector_subscript_const(struct vector const *arr, size_t index)
{
	return arr->data + arr->elem * index;
}
void vector_push(struct vector *arr, void const *data, size_t size)
{
	size_t original_size = arr->size;
	size_t bytes = arr->elem * size;
	size_t original_bytes = arr->elem * original_size;

	arr->size += size;
	vector_recapacitate(arr);
	
	memmove(arr->data + original_bytes, data, bytes);
}
void vector_push_array(struct vector *arr, struct vector const *src)
{
	assert(arr->elem == src->elem);

	vector_push(arr, src->data, src->size);
}
void vector_vacate(struct vector *arr, size_t index, size_t size)
{
	assert(index <= arr->size);

	size_t original_size = arr->size;
	size_t bytes = arr->elem * size;
	size_t index_bytes = arr->elem * index;
	size_t original_bytes = arr->elem * original_size;

	arr->size += size;

	vector_recapacitate(arr);
	
	memmove(arr->data + index_bytes + bytes, arr->data + index_bytes, original_bytes - index_bytes);
}
void vector_insert(struct vector *arr, size_t index, void const *data, size_t size)
{
	size_t bytes = arr->elem * size;
	size_t index_bytes = arr->elem * index;

	vector_vacate(arr, index, size);
	
	memmove(arr->data + index_bytes, data, bytes);
}
void vector_insert_array(struct vector *arr, size_t index, struct vector const *src)
{
	assert(arr->elem == src->elem);

	vector_insert(arr, index, src->data, src->size);
}
void vector_pop(struct vector *arr, size_t size)
{
	assert(size <= arr->size);

	arr->size -= size;

	vector_recapacitate(arr);
}
void vector_erase(struct vector *arr, size_t index, size_t size)
{
	assert(index + size <= arr->size);

	arr->size -= size;
	memmove(arr->data + index, arr->data + index + size, arr->size - index);
	vector_recapacitate(arr);
}
void vector_split(struct vector *dest, struct vector *arr, size_t index)
{
	assert(index <= arr->size);

	size_t index_bytes = arr->elem * index;
	size_t bytes = arr->elem * arr->size;

	vector_mk(dest, arr->elem, arr->size - index);

	memcpy(dest->data, arr->data + index_bytes, bytes - index_bytes);

	vector_pop(arr, arr->size - index);
}

