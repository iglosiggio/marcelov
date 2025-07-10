#include <stdint.h>
#include <stdbool.h>

#include "keyboard.h"
#include "fb.h"
#include "utils.h"

constexpr uint64_t TOGGLE_SHIFT_IDX = 1;
constexpr uint64_t MOVE_DOWN_IDX = 2;
constexpr uint64_t MOVE_UP_IDX = 3;
constexpr uint64_t MOVE_LEFT_IDX = 4;
constexpr uint64_t MOVE_RIGHT_IDX = 5;
constexpr uint64_t NEW_LINE_IDX = 6;
constexpr uint64_t DELETE_LAST_IDX = 7;

// `true` -> the scrollbuffer changed
// `false` -> the scrollbuffer remains the same

static bool unrecognized_scancode(uint8_t scancode);
static bool toggle_shift(uint8_t);
static bool move_down(uint8_t);
static bool move_up(uint8_t);
static bool move_left(uint8_t);
static bool move_right(uint8_t);
static bool new_line(uint8_t);
static bool delete_last(uint8_t);

// Este es un array de punteros a funciones
typedef bool (*scancode_handler) (uint8_t scancode);
static const scancode_handler special_scancodes[] = {
	unrecognized_scancode,
	toggle_shift,
	move_down,
	move_up,
	move_left,
	move_right,
	new_line,
	delete_last
};

struct scancode_info {
	// Si es distinto de '\0' su valor es la letra a imprimir de no estar
	// shift activado.
	uint8_t main_value;
	// Si `main_value` es distinto de '\0' su valor es la letra a imprimir
	// de estar shift activado.
	//
	// Sino su valor es un índice en la tabla `special_scancodes` que
	// determina el código a ejecutar para esta tecla.
	uint8_t special_value;
};

// Lista sacada de https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html
static const struct scancode_info scancode_defs[256] = {
	[0x01] = { '\0', '\0' }, // Esc

	[0x02] = { '1', '!' },
	[0x03] = { '2', '@' },
	[0x04] = { '3', '#' },
	[0x05] = { '4', '$' },
	[0x06] = { '5', '%' },
	[0x07] = { '6', '^' },
	[0x08] = { '7', '&' },
	[0x09] = { '8', '*' },
	[0x0a] = { '9', '(' },
	[0x0b] = { '0', ')' },
	[0x0c] = { '-', '_' },
	[0x0d] = { '=', '+' },
	[0x0e] = { '\0', DELETE_LAST_IDX }, // Backspace

	[0x0f] = { '\0', '\0' }, // Tab
	[0x10] = { 'q', 'Q' },
	[0x11] = { 'w', 'W' },
	[0x12] = { 'e', 'E' },
	[0x13] = { 'r', 'R' },
	[0x14] = { 't', 'T' },
	[0x15] = { 'y', 'Y' },
	[0x16] = { 'u', 'U' },
	[0x17] = { 'i', 'I' },
	[0x18] = { 'o', 'O' },
	[0x19] = { 'p', 'P' },
	[0x1a] = { '[', '{' },
	[0x1b] = { ']', '}' },

	[0x1c] = { '\0', NEW_LINE_IDX }, // Enter

	[0x1d] = { '\0', '\0' }, // LCtrl

	[0x1e] = { 'a', 'A' },
	[0x1f] = { 's', 'S' },
	[0x20] = { 'd', 'D' },
	[0x21] = { 'f', 'F' },
	[0x22] = { 'g', 'G' },
	[0x23] = { 'h', 'H' },
	[0x24] = { 'j', 'J' },
	[0x25] = { 'k', 'K' },
	[0x26] = { 'l', 'L' },
	[0x27] = { ';', ':' },
	[0x28] = { '\'', '"' },

	[0x29] = { '`', '~' },

	[0x2a] = { '\0', TOGGLE_SHIFT_IDX }, // LShift
	[0x80 | 0x2a] = { '\0', TOGGLE_SHIFT_IDX }, // LShift (UP)

	[0x2b] = { '\\', '|' }, // on a 102-key keyboard

	[0x2c] = { 'z', 'Z' },
	[0x2d] = { 'x', 'X' },
	[0x2e] = { 'c', 'C' },
	[0x2f] = { 'v', 'V' },
	[0x30] = { 'b', 'B' },
	[0x31] = { 'n', 'N' },
	[0x32] = { 'm', 'M' },
	[0x33] = { ',', '<' },
	[0x34] = { '.', '>' },
	[0x35] = { '/', '?' },
	[0x36] = { '\0', '\0' }, // RShift

	[0x37] = { '\0', '\0' }, // Keypad-* or */PrtScn on a 83/84-key keyboard

	[0x38] = { '\0', '\0' }, // LAlt
	[0x39] = { ' ', ' ' }, // Space bar

	[0x3a] = { '\0', TOGGLE_SHIFT_IDX }, // CapsLock

	// IDEA: ¿Como usarian los Fx para poder cambiar de color?
	[0x3b] = { '\0', '\0' }, // F1
	[0x3c] = { '\0', '\0' }, // F2
	[0x3d] = { '\0', '\0' }, // F3
	[0x3e] = { '\0', '\0' }, // F4
	[0x3f] = { '\0', '\0' }, // F5
	[0x40] = { '\0', '\0' }, // F6
	[0x41] = { '\0', '\0' }, // F7
	[0x42] = { '\0', '\0' }, // F8
	[0x43] = { '\0', '\0' }, // F9
	[0x44] = { '\0', '\0' }, // F10

	[0x45] = { '\0', '\0' }, // NumLock

	[0x46] = { '\0', '\0' }, // ScrollLock

	[0x47] = { '\0', '\0' }, // Keypad-7/Home
	[0x48] = { '\0', MOVE_UP_IDX }, // Keypad-8/Up
	[0x49] = { '\0', '\0' }, // Keypad-9/PgUp

	[0x4a] = { '\0', '\0' }, // Keypad--

	[0x4b] = { '\0', MOVE_LEFT_IDX }, // Keypad-4/Left
	[0x4c] = { '\0', '\0' }, // Keypad-5
	[0x4d] = { '\0', MOVE_RIGHT_IDX }, // Keypad-6/Right
	[0x4e] = { '\0', '\0' }, // Keypad-+

	[0x4f] = { '\0', '\0' }, // Keypad-1/End
	[0x50] = { '\0', MOVE_DOWN_IDX }, // Keypad-2/Down
	[0x51] = { '\0', '\0' }, // Keypad-3/PgDn

	[0x52] = { '\0', '\0' }, // Keypad-0/Ins
	[0x53] = { '\0', '\0' }, // Keypad-./Del

	[0x54] = { '\0', '\0' }, // Alt-SysRq on a 84+ key keyboard

	[0x57] = { '\0', '\0' }, // F11 on a 101+ key keyboard
	[0x58] = { '\0', '\0' } // F12 on a 101+ key keyboard
};

constexpr uint32_t MAX_LINE_LEN = 256;
constexpr uint32_t MAX_LINES_REMEMBERED = 1024;
char scrollbuffer[MAX_LINES_REMEMBERED][MAX_LINE_LEN + 1];

constexpr uint32_t margin_left= 15;
constexpr uint32_t margin_right = 15;
constexpr uint32_t margin_top = 15;
constexpr uint32_t margin_bottom = 15;

static bool scrollbuffer_scrolls_on_new_line = false;
static uint32_t scrollbuffer_top_line = 0;
static uint32_t scrollbuffer_start = 0;
static uint32_t curr_line = 0;
static uint32_t curr_column = 0;
static bool is_shift_pressed = false;

static
void scrollbuffer_scroll_down(void) {
	if ((scrollbuffer_top_line + 1) % MAX_LINES_REMEMBERED == scrollbuffer_start) return;
	scrollbuffer_top_line = (scrollbuffer_top_line + 1) % MAX_LINES_REMEMBERED;
}

static
void scrollbuffer_scroll_up(void) {
	if (scrollbuffer_top_line == scrollbuffer_start) return;
	scrollbuffer_top_line = (scrollbuffer_top_line + MAX_LINES_REMEMBERED - 1) % MAX_LINES_REMEMBERED;
}

static
void add_new_line(void) {
	curr_line = (curr_line + 1) % MAX_LINES_REMEMBERED;
	curr_column = 0;
	if (curr_line == scrollbuffer_start) {
		scrollbuffer_start = (scrollbuffer_start + 1) % MAX_LINES_REMEMBERED;
	}
	if (curr_line == scrollbuffer_top_line) {
		scrollbuffer_top_line = (scrollbuffer_top_line + 1) % MAX_LINES_REMEMBERED;
	} else if (scrollbuffer_scrolls_on_new_line) {
		scrollbuffer_scroll_down();
	}

	for (int i = 0; i < MAX_LINE_LEN; i++) {
		scrollbuffer[curr_line][i] = 0;
	}
}

static
void putchar(char c) {
	if (c == 0) return;
	if (MAX_LINE_LEN <= curr_column) add_new_line();
	uint32_t line_width = margin_left + fb_measure_line_width(scrollbuffer[curr_line], curr_column) + fb_measure_char(c) + margin_right;
	if (fb.width <= line_width) add_new_line();
	scrollbuffer[curr_line][curr_column++] = c;
}

static
bool unrecognized_scancode(uint8_t scancode) {
	return false;
}

static
bool toggle_shift(uint8_t scancode) {
	is_shift_pressed = !is_shift_pressed;
	return false;
}

static
bool move_down(uint8_t scancode) {
	scrollbuffer_scroll_down();
	scrollbuffer_scrolls_on_new_line = false;
	return true;
}

static
bool move_up(uint8_t scancode) {
	scrollbuffer_scroll_up();
	scrollbuffer_scrolls_on_new_line = false;
	return true;
}

static
bool move_left(uint8_t scancode) {
	return false;
}

static
bool move_right(uint8_t scancode) {
	return false;
}

static
bool new_line(uint8_t scancode) {
	add_new_line();
	return true;
}

static
bool delete_last(uint8_t scancode) {
	if (curr_column == 0) return false;
	scrollbuffer[curr_line][--curr_column] = 0;
	return true;
}

static
void draw_scrollbuffer(void) {
	fb_clear(170, 69, 69);

	uint32_t next_y;
	uint32_t y = margin_top;
	uint32_t line = scrollbuffer_top_line;
	uint32_t last_line = -1;

	while (1) {
		uint32_t x = margin_left;
		uint32_t next_y = y + fb_measure_line_height(scrollbuffer[line], MAX_LINE_LEN);

		// If there's no vertical space available let's stop drawing here
		if (fb.height <= next_y + margin_bottom) {
			break;
		}

		// Draw the current line
		for (int i = 0; i < MAX_LINE_LEN && x + margin_right < fb.width; i++) {
			char c = scrollbuffer[line][i];
			if (c == 0) break;

			fb_print_char(c, x, y);
			x += fb_measure_char(c);
		}
		last_line = line; // Mark the last line drawn

		// Advance
		y += fb_measure_line_height(scrollbuffer[line], MAX_LINE_LEN);
		line = (line + 1) % MAX_LINES_REMEMBERED;

		// Check if the next line is valid
		if (line == scrollbuffer_start) {
			break;
		}
	}

	// If the last line drawn is the current scrollbuffer line then re-enable the sticky bit
	if (last_line == curr_line) {
		scrollbuffer_scrolls_on_new_line = true;
	}
}

void keyboard_process_scancode(uint8_t scancode) {
	struct scancode_info info = scancode_defs[scancode];
	bool should_redraw = true;

	if (info.main_value == '\0') {
		should_redraw = special_scancodes[info.special_value](scancode);
	} else {
		putchar(is_shift_pressed ? info.special_value : info.main_value);
	}

	if (should_redraw) {
		draw_scrollbuffer();
	}
}
