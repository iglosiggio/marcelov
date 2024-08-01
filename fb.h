#pragma once

#include <stdbool.h>
#include <stdint.h>

bool fb_init(void* address, uint32_t width, uint32_t height);
void fb_clear(uint8_t r, uint8_t g, uint8_t b);
void fb_print(const char* str, uint32_t start_x, uint32_t start_y);

typedef struct fb_rgb_pixel {
	uint8_t B, G, R, X;
} rgb_t;

struct fb_config {
	struct fb_rgb_pixel* canvas;
	uint32_t width;
	uint32_t height;
};

extern struct fb_config fb;
