#pragma once

#include <stdbool.h>

#include "sbi.h"

#define attr_packed __attribute__((packed))

static
bool str_eq(const char* a, const char* b) {
	while (*a == *b) {
		if (*a == 0) {
			return true;
		}
		a++;
		b++;
	}
	return false;
}

static
uint64_t bswap8(uint64_t in) {
	return ((in >>  0) & 0xFF) << 56
	     | ((in >>  8) & 0xFF) << 48
	     | ((in >> 16) & 0xFF) << 40
	     | ((in >> 24) & 0xFF) << 32
	     | ((in >> 32) & 0xFF) << 24
	     | ((in >> 40) & 0xFF) << 16
	     | ((in >> 48) & 0xFF) <<  8
	     | ((in >> 56) & 0xFF) <<  0;
}

static
uint32_t bswap4(uint32_t in) {
	return ((in >>  0) & 0xFF) << 24
	     | ((in >>  8) & 0xFF) << 16
	     | ((in >> 16) & 0xFF) <<  8
	     | ((in >> 24) & 0xFF) <<  0;
}

static
uint16_t bswap2(uint16_t in) {
	return ((in >> 0) & 0xFF) << 8
	     | ((in >> 8) & 0xFF) << 0;
}

static
uint64_t print(const char* str) {
	uint64_t len = 0;
	while (*str) {
		uint64_t res = sbi_console_putchar(*str++);
		if (res != 0) {
			return res;
		}
		len++;
	}
	return len;
}

static
uint64_t print_hex(uint64_t v) {
	static const char* table = "0123456789ABCDEF";
	char buf[19] = { [17] = '0' };
	char* s = buf + 18;
	while (v != 0) {
		*--s = table[v & 0xF];
		v >>= 4;
	}
	*--s = 'x';
	*--s = '0';
	return print(s);
}
