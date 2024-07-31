#include <stdint.h>
#include "sbi.h"

void zero_bss() {
	extern uint8_t kernel_bss_start, kernel_bss_end;

	uint8_t* c = &kernel_bss_start;
	while (c != &kernel_bss_end) {
		*c++ = 0;
	}
}

long print(const char* str) {
	long len = 0;
	while (*str) {
		long res = sbi_console_putchar(*str++);
		if (res != 0) {
			return res;
		}
		len++;
	}
	return len;
}

long read_until(char delimiter, char* dst, long dst_size, long* written_size) {
	long written = 0;
	while (written < dst_size) {
		long c = sbi_console_getchar();
		if (c < 0) {
			return c;
		}
		dst[written++] = c;
		if (written_size != 0x0) {
			*written_size += 1;
		}
	}
}

int b;
int c = 123;
const int a_start;
const int b_start = 1;
int main() {
	zero_bss();

	struct sbiret spec_version = sbi_get_spec_version();
	struct sbiret impl_id = sbi_get_impl_id();
	struct sbiret impl_ver = sbi_get_impl_version();
	struct sbiret mvendorid = sbi_get_mvendorid();
	struct sbiret marchid = sbi_get_marchid();
	struct sbiret mimpid = sbi_get_mimpid();

	struct sbiret dbcn_support = sbi_probe_extension(0x4442434E);

	print("Hola mundo!!\n");
	print("¿Cómo te llamás?\n");
	char nombre[64] = {};
	int res = read_until('\n', nombre, sizeof(nombre) - 1, 0x0);
	if (res < 0) {
		print("No pude leer de la consola :(\n");
		sbi_shutdown();
	} else {
		print("Hola ");
		print(nombre);
		print("!!\n");
	}


	int a = a_start;
	b = b_start;
	while (1) {
		c = a + b;
		a = b;
		b = c;
	}
	return 0;
}
