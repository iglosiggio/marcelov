#pragma once

#include <stdint.h>

void keyboard_process_scancode(uint8_t scancode);
void scrollback_putchar(char c);
void scrollback_new_line(void);
void scrollback_draw();
