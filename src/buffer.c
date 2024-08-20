#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "buffer.h"
#include "settings.h"
#include "util.h"
#include "vector.h"

// A basic text buffer implemented using a jagged 2d array of lines.
// This will be improved in the future (all simple operations plus changing the mode can be made O(log n) amortized)
struct buffer
{
	struct vector lines;
	// See buffer.h for an explanation of these fields.
	size_t row, real_col, display_col, index;
};
static inline struct vector *buffer_get_line(struct buffer *buf, size_t row)
{
	return vector_subscript(&buf->lines, row);
}
static inline struct vector const *buffer_get_line_const(struct buffer const *buf, size_t row)
{
	return vector_subscript_const(&buf->lines, row);
}

static inline size_t char_display_width(size_t position, char c)
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
static inline void buffer_update_display_column_index(struct buffer *buf)
{
	buf->display_col = 0;
	buf->index = 0;
	struct vector const *line = vector_subscript_const(&buf->lines, buf->row);
	for (; buf->index < line->size; buf->index++)
	{
		size_t cur_width = char_display_width(buf->display_col, *(char const *)vector_subscript_const(line, buf->index));
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
static inline void buffer_reset_column(struct buffer *buf)
{
	buf->display_col = 0;
	struct vector const *line = buffer_get_line_const(buf, buf->row);
	for (size_t i = 0; i < buf->index; i++)
	{
		buf->display_col += char_display_width(buf->display_col, *(char const *)vector_subscript_const(line, buf->index));
	}
	buf->real_col = buf->real_col;
}

size_t buffer_get_height(struct buffer const *buf)
{
	return buf->lines.size;
}
size_t buffer_get_display_width(struct buffer const *buf, size_t row)
{
	size_t width = 0;
	if (row < buffer_get_height(buf))
	{
		struct vector const *line = buffer_get_line_const(buf, row);
		for (size_t i = 0; i < line->size; i++)
			width += char_display_width(width, *(char const *)vector_subscript_const(line, i));
	}
	return width;
}
	
size_t buffer_get_length(struct buffer const *buf, size_t row)
{
	if (row >= buffer_get_height(buf))
	{
		return 0;
	}
	return buffer_get_line_const(buf, row)->size;
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
		vector_vacate(&buf->lines, buf->row + 1, 1);
		struct vector *line = buffer_get_line(buf, buf->row);
		vector_split(line + 1, line, buf->index);

		buf->row++;
		buf->index = 0;
	}
	else
	{
		struct vector *line = buffer_get_line(buf, buf->row);
		vector_insert(line, buf->index, &ch, 1);
		
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
		vector_erase(buffer_get_line(buf, buf->row), buf->index, 1);
	}
	else if (buf->row < buffer_get_height(buf) - 1)
	{
		struct vector *line = buffer_get_line(buf, buf->row);
		vector_push_array(line, line + 1);
		vector_free(line + 1);
		vector_erase(&buf->lines, (buf->row + 1) * sizeof(struct vector), sizeof(struct vector));
	}
}

struct buffer *buffer_mk_empty()
{
	struct buffer *buf = malloc(sizeof(struct buffer));

	vector_mk(&buf->lines, sizeof(struct vector), 1);
	vector_mk(buffer_get_line(buf, 0), sizeof(char), 0);
	
	buf->row = 0;
	buf->real_col = 0;
	buf->display_col = 0;
	buf->index = 0;
	
	return buf;
}
void buffer_free(struct buffer *buf)
{
	vector_free(&buf->lines);
}

