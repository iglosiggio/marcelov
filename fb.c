#include <stdbool.h>
#include <stdint.h>

#include "qemu.h"
#include "utils.h"

#include "fb.h"

/* See https://github.com/qemu/qemu/blob/master/include/standard-headers/drm/drm_fourcc.h#L192 */
#define FB_PIXFMT_XRGB8888 0x34325258

struct fb_config fb;

bool fb_init(void* address, uint32_t width, uint32_t height) {
	uint16_t fb_selector;
	uint32_t count;
	struct {
		uint32_t size;
		uint16_t select;
		uint16_t reserved;
		char name[56];
	} file;

	if (fw_cfg_dma_read_from(FW_CFG_FILE_DIR, &count, sizeof(count))) {
		return false;
	}
	count = bswap4(count);
	for (int i = 0; i < count; i++) {
		if (fw_cfg_dma_read(&file, sizeof(file))) {
			continue;
		}
		if (str_eq("etc/ramfb", file.name)) {
			fb_selector = bswap2(file.select);
			break;
		}
	}

	struct attr_packed {
		uint64_t addr;
		uint32_t fourcc;
		uint32_t flags;
		uint32_t width;
		uint32_t height;
		uint32_t stride;
	} fb_config = {
		.addr = bswap8((uint64_t) address),
		.fourcc = bswap4(FB_PIXFMT_XRGB8888),
		.flags = bswap4(0),
		.width = bswap4(width),
		.height = bswap4(height),
		.stride = bswap4(width * 4)
	};

	if (fw_cfg_dma_write_to(fb_selector, &fb_config, sizeof(fb_config))) {
		return true;
	}

	fb = (struct fb_config) {
		.canvas = address,
		.width = width,
		.height = height
	};

	return false;
}

void fb_clear(uint8_t r, uint8_t g, uint8_t b) {
	rgb_t px = { .R = r, .G = g, .B = b };
	for (int i = 0; i < fb.width * fb.height; i++) {
		fb.canvas[i] = px;
	}
}

//#include "fonts/monaco.inc"
//#define font monaco
//#define FIRST_CHAR 32
//#define CHAR_WIDTH(c) 8
//#define CHAR_HEIGHT 16
//#define CHAR_SIZE 16
//#define LAST_CHAR (FIRST_CHAR + sizeof(font) / CHAR_SIZE)

#include "fonts/cream12.inc"
#define font cream12
#define FIRST_CHAR 32
#define CHAR_WIDTH(c) cream12_width[c]
#define CHAR_HEIGHT 16
#define CHAR_SIZE 32
#define LAST_CHAR (FIRST_CHAR + sizeof(font) / CHAR_SIZE)


static
void fb_print_char(char c, uint32_t start_x, uint32_t start_y) {
	rgb_t px = { .R = 0, .G = 0, .B = 0 };

	if (c < FIRST_CHAR || LAST_CHAR <= c) {
		c = '?';
	}

	for (uint32_t y = 0; y < CHAR_HEIGHT; y++) {
		for (uint32_t x = 0; x < CHAR_WIDTH(c); x++) {
			// Índice al principio de la definición de este char.
			int c_start = (c - FIRST_CHAR) * CHAR_SIZE;
			// Para chars que ocupan más de 8 px de ancho.
			// Offset del principio del char al bloque de 8px a usar.
			int c_offset = CHAR_HEIGHT * (x / 8);
			// Cuál offset x dentro de bloque de 8px actual.
			int c_x = x % 8;
			if (fb.width <= start_x + x) {
				break;
			}
			if (font[c_start + c_offset + y] & (0x80 >> c_x)) {
				fb.canvas[(start_y + y) * fb.width + (start_x + x)] = px;
			}
		}
		if (fb.height <= start_y + y) {
			break;
		}
	}
}

void fb_print(const char* str, uint32_t start_x, uint32_t start_y) {
	uint32_t x = start_x;
	uint32_t y = start_y;
	while (*str != 0) {
		char c = *str;
		if (c == '\n') {
			y += CHAR_HEIGHT + 2;
			x = start_x;
		} else {
			fb_print_char(c, x, y);
			x += CHAR_WIDTH(c);
		}
		str++;
	}
}

void fb_print_charmap(uint32_t start_x, uint32_t start_y) {
	uint32_t x = start_x;
	uint32_t y = start_y;
	for (int i = 0; i < 256; i++) {
		if (i && i % 32 == 0) {
			y += CHAR_HEIGHT + 2;
			x = start_x;
		}
		fb_print_char(i, x, y);
		x += i < FIRST_CHAR || LAST_CHAR <= i
			? CHAR_WIDTH('?')
			: CHAR_WIDTH(i);
	}
}
