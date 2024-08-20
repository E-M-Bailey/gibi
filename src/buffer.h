// Definitions related to buffers, which are the representations of open files in memory.
// Buffers also keep track of a cursor, since this allows optimizations for some 
// buffer data structures and is necessary for updating settings.
// They are also responsible for styling text, though this may be delegated in the future.

// For the cursor:
// The *real* column is an size_ternal number which tracks where the cursor "wants" to be in a row.
// The *displayed* column is a column number that tries to be as close as possible to the real column.
// Usually they will be equal, but if the real column is past the end of the line or in the middle of a tab, it must be adjusted.
// The displayed column determines the index of the character in a row which will be affected by operations.

#ifndef GIBI_BUFFER_H
#define GIBI_BUFFER_H

#include <stdlib.h>
#include <limits.h>

struct buffer;

// Returns the number of rows in buf (at least one if it has been initialized)
size_t buffer_get_height(struct buffer const *buf);
// Returns the display width of the given row of the buffer or 0 if the given row number is invalid.
size_t buffer_get_display_width(struct buffer const *buf, size_t row);
// Returns the number of characters of the given row of the buffer (excluding the newline) or 0 if the given row number is invalid.
size_t buffer_get_length(struct buffer const *buf, size_t row);

// Returns buf's cursor row
size_t buffer_get_row(struct buffer const *buf);
// Returns buf's cursor real column
size_t buffer_get_real_col(struct buffer const *buf);
// Returns buf's cursor display column
size_t buffer_get_display_col(struct buffer const *buf);
// Returns buf's cursor character index
size_t buffer_get_cursor_index(struct buffer const *buf);
// Returns the number of rows in buf (which must be at least 1)

// Sets buf's cursor row.
void buffer_set_cursor_row(struct buffer *buf, size_t row);
// Sets buf's cursor real column and adjusts the display column/index accordingly.
void buffer_set_cursor_real_position(struct buffer *buf, size_t real_col);
// Moves buf's cursor up one row if possible.
void buffer_move_cursor_up(struct buffer *buf);
// Moves buf's cursor down one row if possible.
void buffer_move_cursor_down(struct buffer *buf);
// Moves buf's cursor left one character (possibly to the previous row) if possible and resets its real column.
void buffer_move_cursor_left(struct buffer *buf);
// Moves buf's cursor right one character (possibly to the next row) if possible and resets its real column.
void buffer_move_cursor_right(struct buffer *buf);

// Replaces the character at the cursor position and advances the cursor.
void buffer_replace_after_advance(struct buffer *buf, char ch);
// Inserts a character before the cursor position.
void buffer_insert_before(struct buffer *buf, char ch);
// Deletes the character before the cursor position.
void buffer_delete_before(struct buffer *buf);
// Deletes the character after the cursor position.
void buffer_delete_after(struct buffer *buf);

// Create an empty buffer.
struct buffer *buffer_mk_empty();
// TODO add more constructors.
// Free the resources associated with a buffer.
void buffer_free(struct buffer *buf);

// Returns a capacity large enough for the previous method.
size_t capacity_per_row(struct buffer const *buf, size_t width);
// Writes the text to be displayed for positions [left, left + width) of the given row.
// This includes style escape codes, but not a trailing newline.
// Returns the number of characters written, or SIZE_MAX if there is not enough capacity (this should not happen barring a bug in other parts of the code).
size_t get_display_text(struct buffer const *buf, size_t capacity, size_t row, size_t left, size_t width);

#endif

