#ifndef GIBI_VECTOR_H
#define GIBI_VECTOR_H

#include <assert.h>
#include <stdlib.h>

struct vector
{
	char *data;
	size_t elem, size, capacity;
};

// Create a new dynamic array.
// WARN dest should not be allocated, or else it will be leaked!
void vector_mk(struct vector *dest, size_t elem, size_t size);
// Free the resources associated with arr, putting it in a possibly invalid state.
void vector_free(struct vector *arr);
// Clear arr.
void vector_clear(struct vector *arr);
// reallocate unless capacity in [bytes, 4*bytes]
void vector_recapacitate(struct vector *arr);

// Get a pointer to the index-th element.
void *vector_subscript(struct vector *arr, size_t index);
void const *vector_subscript_const(struct vector const *arr, size_t index);

// Below, size is measured in multiples of arr->elem.
// Append data to the end of arr.
void vector_push(struct vector *arr, void const *data, size_t size);
void vector_push_array(struct vector *arr, struct vector const *src);
// Make space for inserting size elements at index.
void vector_vacate(struct vector *arr, size_t index, size_t size);
// Insert data at index.
void vector_insert(struct vector *arr, size_t index, void const *data, size_t size);
void vector_insert_array(struct vector *arr, size_t index, struct vector const *src);
// Erase size elements from the end
void vector_pop(struct vector *arr, size_t size);
// Erase size elements at index.
void vector_erase(struct vector *arr, size_t index, size_t size);
// Return a new array of the elements with indices [index, size), which have been removed from arr.
// WARN dest should not be allocated, or else it will be leaked!
void vector_split(struct vector *dest, struct vector *arr, size_t index);

#endif

