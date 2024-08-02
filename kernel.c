#include <stdint.h>

#include "fb.h"
#include "qemu.h"
#include "sbi.h"
#include "utils.h"

void zero_bss() {
	extern uint8_t kernel_bss_start, kernel_bss_end;

	uint8_t* c = &kernel_bss_start;
	while (c != &kernel_bss_end) {
		*c++ = 0;
	}
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

	char qemu[5] = {};
	char qemu_cfg[9] = {};
	fw_cfg_read_signature(qemu, qemu_cfg);
	if (str_eq(qemu, "QEMU") && str_eq(qemu_cfg, "QEMU CFG")) {
		print("Hola QEMU!\n");
	}

	{
		uint32_t count;
		struct {
			uint32_t size;
			uint16_t select;
			uint16_t reserved;
			char name[56];
		} file;

		if (fw_cfg_dma_read_from(FW_CFG_FILE_DIR, &count, sizeof(count))) {
			print("No pude leer el count :(\n");
			sbi_shutdown();
		}
		count = bswap4(count);

		print("Listando los dispositivos de fw_cfg:\n");
		for (int i = 0; i < count; i++) {
			if (fw_cfg_dma_read(&file, sizeof(file))) {
				print("No pude leer el file :(\n");
				sbi_shutdown();
			}
			print("  - ");
			print(file.name);
			print(" (size = ");
			print_hex(bswap4(file.size));
			print(", select = ");
			print_hex(bswap2(file.select));
			print(")\n");
		}
	}

	if (fb_init((void*) 0x80200000, 640, 480)) {
		print("Hubo un problema inicializando la pantalla!\n");
	}

	fb_clear(170, 69, 69);
	fb_print("Hola ~~Organizacion del Computador 2~~!\nHola Arquitectura y Organizacion del Computador!", 40, 40);
	fb_print_charmap(100, 100);

	while (1) asm volatile("");
	return 0;
}
