#include <stdint.h>

void zero_bss() {
	extern uint8_t kernel_bss_start, kernel_bss_end;

	uint8_t* c = &kernel_bss_start;
	while (c != &kernel_bss_end) {
		*c++ = 0;
	}
}

int b;
int c = 123;
const int a_start;
const int b_start = 1;
int main() {
	zero_bss();

	int a = a_start;
	b = b_start;
	while (1) {
		c = a + b;
		a = b;
		b = c;
	}
	return 0;
}
