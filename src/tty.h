#ifndef GIBI_TTY_H
#define GIBI_TTY_H

// Set the terminal to raw mode. On exit, the program will restore terminal settings.
void tty_setup();

// Terminal commands. Note that these may require flushing stdout afterward to work.

// Cursor/text:
// Move the cursor
void tty_cursor_position(int row, int col);
// Clear the screen and moves the cursor to (0, 0).
void tty_clear();

// Convenience function to flush stdout.
void tty_flush();

#endif

