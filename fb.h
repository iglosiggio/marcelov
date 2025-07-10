#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct fb_rgb_pixel {
	uint8_t B, G, R, X;
} rgb_t;

struct fb_config {
	struct fb_rgb_pixel* canvas;
	uint32_t width;
	uint32_t height;
};

extern struct fb_config fb;

bool fb_init(void* address, uint32_t width, uint32_t height);
void fb_clear(uint8_t r, uint8_t g, uint8_t b);
uint32_t fb_measure_line_width(const char* str, uint64_t size);
uint32_t fb_measure_line_height(const char* str, uint64_t size);
uint32_t fb_measure_char(char c);
void fb_print(const char* str, uint32_t start_x, uint32_t start_y);
void fb_print_char(char c, uint32_t start_x, uint32_t start_y);
void fb_print_charmap(uint32_t start_x, uint32_t start_y);
void fb_print_dec(uint32_t n, uint32_t start_x, uint32_t start_y);
void fb_fill_rect(rgb_t col, uint32_t x, uint32_t y, uint32_t width, uint32_t height);