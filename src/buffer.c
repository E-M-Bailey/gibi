#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "buffer.h"
#include "settings.h"

inline size_t size_max(size_t a, size_t b)
{
	return a >= b ? a : b;
}
inline size_t size_min(size_t a, size_t b)
{
	return a <= b ? a : b;
}

// A basic text buffer implemented using a jagged 2d array of lines.
// This will be improved in the future (all simple operations plus changing the mode can be made O(log n) amortized)
struct dynamic_array
{
	void *data;
	size_t elem, size, capacity;
};

// WARN dest should not be allocated, or else it will be leaked!
inline void array_mk(struct dynamic_array *dest, size_t elem, size_t size)
{
	assert(elem > 0);
	dest->data = size ? malloc(elem * size) : NULL;
	dest->elem = elem;
	dest->size = size;
	dest->capacity = elem * size;
}
inline void array_free(struct dynamic_array *arr)
{
	free(arr->data);
}
inline void array_clear(struct dynamic_array *arr)
{
	free(arr->data);
	arr->data = NULL;
	arr->size = 0;
	arr->capacity = 0;
}
// reallocate unless capacity in [bytes, 4*bytes]
inline void array_recapacitate(struct dynamic_array *arr)
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
inline void *array_subscript(struct dynamic_array *arr, size_t index)
{
	return arr->data + arr->elem * index;
}
inline void const *array_subscript_const(struct dynamic_array const *arr, size_t index)
{
	return arr->data + arr->elem * index;
}
// Below, size is measured in multiples of arr->elem.
inline void array_push(struct dynamic_array *arr, void const *data, size_t size)
{
	size_t original_size = arr->size;
	size_t bytes = arr->elem * size;
	size_t original_bytes = arr->elem * original_size;

	arr->size += size;
	array_recapacitate(arr);
	
	memmove(arr->data + original_bytes, data, bytes);
}
inline void array_push_array(struct dynamic_array *arr, struct dynamic_array const *src)
{
	assert(arr->elem == src->elem);

	array_push(arr, src->data, src->size);
}
inline void array_vacate(struct dynamic_array *arr, size_t index, size_t size)
{
	assert(index <= arr->size);

	size_t original_size = arr->size;
	size_t bytes = arr->elem * size;
	size_t index_bytes = arr->elem * index;
	size_t original_bytes = arr->elem * original_size;

	arr->size += size;

	array_recapacitate(arr);
	
	memmove(arr->data + index_bytes + bytes, arr->data + index_bytes, original_bytes - index_bytes);
}
inline void array_insert(struct dynamic_array *arr, size_t index, void const *data, size_t size)
{
	size_t bytes = arr->elem * size;
	size_t index_bytes = arr->elem * index;

	array_vacate(arr, index, size);
	
	memmove(arr->data + index_bytes, data, bytes);
}
inline void array_insert_array(struct dynamic_array *arr, size_t index, struct dynamic_array const *src)
{
	assert(arr->elem == src->elem);

	array_insert(arr, index, src->data, src->size);
}
inline void array_pop(struct dynamic_array *arr, size_t size)
{
	assert(size <= arr->size);

	arr->size -= size;

	array_recapacitate(arr);
}
inline void array_erase(struct dynamic_array *arr, size_t index, size_t size)
{
	assert(index + size <= arr->size);

	arr->size -= size;
	memmove(arr->data + index, arr->data + index + size, arr->size - index);
	array_recapacitate(arr);
}
// Returns a new array of the elements with indices [index, size), which have been removed from arr.
// WARN dest should not be allocated, or else it will be leaked!
inline void array_split(struct dynamic_array *dest, struct dynamic_array *arr, size_t index)
{
	assert(index <= arr->size);

	size_t index_bytes = arr->elem * index;
	size_t bytes = arr->elem * arr->size;

	array_mk(dest, arr->elem, arr->size - index);

	memcpy(dest->data, arr->data + index_bytes, bytes - index_bytes);

	array_pop(arr, arr->size - index);
}

struct buffer
{
	struct dynamic_array lines;
	// See buffer.h for an explanation of these fields.
	size_t row, real_col, display_col, index;
};
// FIX
inline struct dynamic_array *buffer_get_line(struct buffer *buf, size_t row)
{
	return array_subscript(&buf->lines, row);
}
inline struct dynamic_array const *buffer_get_line_const(struct buffer const *buf, size_t row)
{
	return array_subscript_const(&buf->lines, row);
}

inline size_t char_display_width(size_t position, char c)
{
	if (c == '\t')
	{
		// next multiple of tab_width
		return tab_width - (position % tab_width);
	}
	else if (c == '\n')
	{
		return 0;
	}
	else if (isprint(c))
	{
		return 1;
	}
	else
	{
		return 2;
	}
}
// Recalculate display column and index from row and real column.
inline void buffer_update_display_column_index(struct buffer *buf)
{
	buf->display_col = 0;
	buf->index = 0;
	struct dynamic_array const *line = array_subscript_const(&buf->lines, buf->row);
	for (; buf->index < line->size; buf->index++)
	{
		size_t cur_width = char_display_width(buf->display_col, *(char const *)array_subscript_const(line, buf->index));
		size_t next = buf->display_col + cur_width;
		bool done = next >= buf->real_col;
		// If display_col reaches real_col, go to whichever of the previous and current column is closer, breaking ties rightward.
		if (done && cur_width > buf->real_col - buf->display_col)
		{
			break;
		}
		buf->display_col = next;
		buf->index++;
		if (done)
		{
			break;
		}
	}
}
// Recaulculate display and real column from row and index
inline void buffer_reset_column(struct buffer *buf)
{
	buf->display_col = 0;
	struct dynamic_array const *line = buffer_get_line_const(buf, buf->row);
	for (size_t i = 0; i < buf->index; i++)
	{
		buf->display_col += char_display_width(buf->display_col, *(char const *)array_subscript_const(line, buf->index));
	}
	buf->real_col = buf->real_col;
}

size_t buffer_get_height(struct buffer const *buf)
{
	return buf->lines.size / sizeof(struct dynamic_array);
}
size_t buffer_get_display_width(struct buffer const *buf, size_t row)
{
	size_t width = 0;
	if (row < buffer_get_height(buf))
	{
		struct dynamic_array const *line = buffer_get_line_const(buf, row);
		for (size_t i = 0; i < line->size; i++)
			width += char_display_width(width, *(char const *)array_subscript_const(line, i));
	}
	return width;
}
	
size_t buffer_get_length(struct buffer const *buf, size_t row)
{
	if (row >= buffer_get_height(buf))
	{
		return 0;
	}
	struct dynamic_array const *lines = buf->lines.data;
	return lines[row].size;
}

size_t buffer_get_row(struct buffer const *buf)
{
	return buf->row;
}
size_t buffer_get_real_col(struct buffer const *buf)
{
	return buf->real_col;
}
size_t buffer_get_display_col(struct buffer const *buf)
{
	return buf->display_col;
}
size_t buffer_get_cursor_index(struct buffer const *buf)
{
	return buf->index;
}

void buffer_set_row(struct buffer *buf, size_t row)
{
	row = size_min(buffer_get_height(buf) - 1, row);
	if (row != buf->row)
	{
		buf->row = row;
		buffer_update_display_column_index(buf);
	}
}
void buffer_set_cursor_real_position(struct buffer *buf, size_t col)
{
	buf->real_col = col;
	buffer_update_display_column_index(buf);
}
	
	
void buffer_move_cursor_up(struct buffer *buf)
{
	if (buf->row > 0)
	{
		buffer_set_row(buf, buf->row - 1);
	}
}
void buffer_move_cursor_down(struct buffer *buf)
{
	buffer_set_row(buf, buf->row + 1);
}
void buffer_move_cursor_left(struct buffer *buf)
{
	if (buf->index > 0)
	{
		buf->index--;
	}
	else if (buf->row > 0)
	{
		buf->row--;
		buf->index = buffer_get_length(buf, buf->row);
	}
	else
	{
		return;
	}
	buffer_reset_column(buf);
}
void buffer_move_cursor_right(struct buffer *buf)
{
	if (buf->index < buffer_get_length(buf, buf->row))
	{
		buf->index++;
	}
	else if (buf->row < buffer_get_height(buf) - 1)
	{
		buf->row++;
		buf->index = 0;
	}
	else
	{
		return;
	}
	buffer_reset_column(buf);
}

void buffer_replace_after_advance(struct buffer *buf, char ch)
{
	buffer_delete_after(buf);
	buffer_insert_before(buf, ch);
}
void buffer_insert_before(struct buffer *buf, char ch)
{
	if (ch == '\n')
	{
		array_vacate(&buf->lines, buf->row + 1, 1);
		struct dynamic_array *line = buffer_get_line(buf, buf->row);
		array_split(line + 1, line, buf->index);

		buf->row++;
		buf->index = 0;
	}
	else
	{
		struct dynamic_array *line = buffer_get_line(buf, buf->row);
		array_insert(line, buf->index, &ch, 1);
		
		buf->index++;
	}
	buffer_reset_column(buf);
}
void buffer_delete_before(struct buffer *buf)
{
	if (buf->index > 0 || buf->row > 0)
	{
		buffer_move_cursor_left(buf);
		buffer_delete_after(buf);
	}
}
void buffer_delete_after(struct buffer *buf)
{
	if (buf->index < buffer_get_length(buf, buf->row))
	{
		array_erase(buffer_get_line(buf, buf->row), buf->index, 1);
	}
	else if (buf->row < buffer_get_height(buf) - 1)
	{
		struct dynamic_array *line = buffer_get_line(buf, buf->row);
		array_push_array(line, line + 1);
		array_free(line + 1);
		array_erase(&buf->lines, (buf->row + 1) * sizeof(struct dynamic_array), sizeof(struct dynamic_array));
	}
}

struct buffer *buffer_mk_empty()
{
	struct buffer *buf = malloc(sizeof(struct buffer));

	array_mk(&buf->lines, sizeof(struct dynamic_array), 1);
	array_mk(buffer_get_line(buf, 0), sizeof(char), 0);
	
	buf->row = 0;
	buf->real_col = 0;
	buf->display_col = 0;
	buf->index = 0;
	
	return buf;
}
void buffer_free(struct buffer *buf)
{
	array_free(&buf->lines);
}


