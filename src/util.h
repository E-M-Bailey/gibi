// A collection of small utility functions.

#ifndef GIBI_UTIL_H
#define GIBI_UTIL_H

#include <stdlib.h>

static inline size_t size_max(size_t a, size_t b)
{
	return a >= b ? a : b;
}
static inline size_t size_min(size_t a, size_t b)
{
	return a <= b ? a : b;
}

#endif

