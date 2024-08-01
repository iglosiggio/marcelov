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

typedef uint8_t char_bitmap[8];
static
char_bitmap font[256] = {
	['a'] = {
		0b00000000,
		0b00011110,
		0b00000001,
		0b00000001,
		0b00011111,
		0b00100001,
		0b00100001,
		0b00011110
	},
	['b'] = {
		0b00000000,
		0b00100000,
		0b00100000,
		0b00111110,
		0b00100001,
		0b00100001,
		0b00100001,
		0b00011110
	},
	['c'] = {
		0b00000000,
		0b00011110,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00100000,
		0b00011110
	},
	['d'] = {
		0b00000000,
		0b00000001,
		0b00000001,
		0b00011111,
		0b00100001,
		0b00100001,
		0b00100001,
		0b00011110
	},
	['e'] = {
		0b00000000,
		0b00011110,
		0b00100001,
		0b00100001,
		0b00111111,
		0b00100000,
		0b00100000,
		0b00011110
	},
	['f'] = {
		0b00000000,
		0b00001110,
		0b00010001,
		0b00010000,
		0b00111110,
		0b00010000,
		0b00010000,
		0b00010000
	},
};

static
void fb_print_char(char c, uint32_t start_x, uint32_t start_y) {
	rgb_t px = { .R = 0, .G = 0, .B = 0 };
	for (uint32_t y = 0; y < 8; y++) {
		for (uint32_t x = 0; x < 8; x++) {
			if (fb.width <= start_x + x) {
				break;
			}
			if (font[c][y] & (0x80 >> x)) {
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
			y += 10;
			x = start_x;
		} else {
			fb_print_char(c, x, y);
			x += 8;
		}
		str++;
	}
}
