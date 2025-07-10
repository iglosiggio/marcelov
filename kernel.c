#include <stdint.h>

#include "encoding.h"
#include "fb.h"
#include "interrupts.h"
#include "keyboard.h"
#include "kmi.h"
#include "qemu.h"
#include "sbi.h"
#include "utils.h"

void handle_keyboard(void) {
	// QEMU makes data immediately available
	uint8_t data = keyboard->data;

	if (data == 0xFA) {
		print("Received ACK from keyboard\n");
		return;
	}

	keyboard_process_scancode(data);
	//print("Received "); print_sdec(data); print(" from the keyboard\n");
}

enum {
	MOUSE_WAIT_BYTE0,
	MOUSE_WAIT_BYTE1,
	MOUSE_WAIT_BYTE2,
} mouse_state = MOUSE_WAIT_BYTE0;
uint8_t mouse_byte0;
uint8_t mouse_byte1;
uint8_t mouse_byte2;
int32_t mouse_x;
int32_t mouse_y;
bool mouse_left;
bool mouse_mid;
bool mouse_right;

void mouse_process_event(void) {
	bool left  = !!(mouse_byte0 & 1);
	bool right = !!(mouse_byte0 & 2);
	bool mid   = !!(mouse_byte0 & 4);

	int32_t x_offset_base = !!(mouse_byte0 & 16) ? 0x100 : 0;
	int32_t x_offset = ((int32_t) mouse_byte1) - x_offset_base;

	int32_t y_offset_base = !!(mouse_byte0 & 32) ? 0x100 : 0;
	int32_t y_offset = ((int32_t) mouse_byte2) - y_offset_base;

	// FIXME: Parece que tengo mal el signo en los ejes??
	mouse_x -= x_offset;
	if (mouse_x < 0)  mouse_x = 0;
	if (fb.width <= mouse_x) mouse_x = fb.width - 1;

	mouse_y += y_offset;
	if (mouse_y < 0)  mouse_y = 0;
	if (fb.height <= mouse_y) mouse_y = fb.height - 1;

	if (mouse_left  != left)  print(left  ? "Left button down\n"   : "Left button up\n");
	if (mouse_mid   != mid)   print(mid   ? "Middle button down\n" : "Middle button up\n");
	if (mouse_right != right) print(right ? "Right button down\n"  : "Right button up\n");
	mouse_left = left;
	mouse_right = right;
	mouse_mid = mid;

	if (mouse_left) {
		int i = mouse_y * fb.width + mouse_x;
		fb.canvas[i] = (rgb_t) { mouse_x, mouse_y, 0 };
	}
	//print_sdec(mouse_x); print(" "); print_sdec(mouse_y); print("\n");

	// Log mouse events
	//print_hex(mouse_byte0); print(" "); print_hex(mouse_byte1); print(" "); print_hex(mouse_byte2); print(" ");
	//if (left)  print("L"); else print(" ");
	//if (mid)   print("M"); else print(" ");
	//if (right) print("R"); else print(" ");
	//print("\n  X: ");
	//print_sdec(x_offset);
	//print("\n  Y: ");
	//print_sdec(y_offset);
	//print("\n");
}

void handle_mouse(void) {
	// QEMU makes data immediately available
	uint8_t data = mouse->data;
	if (mouse_state == MOUSE_WAIT_BYTE0 && data == 0xFA) {
		print("Received ACK from mouse\n");
		return;
	}

	if (mouse_state == MOUSE_WAIT_BYTE0) {
		mouse_byte0 = data;
		mouse_state = MOUSE_WAIT_BYTE1;
	} else if (mouse_state == MOUSE_WAIT_BYTE1) {
		mouse_byte1 = data;
		mouse_state = MOUSE_WAIT_BYTE2;
	} else if (mouse_state == MOUSE_WAIT_BYTE2) {
		mouse_byte2 = data;
		mouse_state = MOUSE_WAIT_BYTE0;
		mouse_process_event();
	}
}

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

void scrollback_print_line(const char* s) {
	while (*s) {
		scrollback_putchar(*s++);
	}
	scrollback_new_line();
}

int b;
int c = 123;
const int a_start;
const int b_start = 1;
int main() {
	zero_bss();

	void test_enumerate();
	test_enumerate();

	struct sbiret spec_version = sbi_get_spec_version();
	struct sbiret impl_id = sbi_get_impl_id();
	struct sbiret impl_ver = sbi_get_impl_version();
	struct sbiret mvendorid = sbi_get_mvendorid();
	struct sbiret marchid = sbi_get_marchid();
	struct sbiret mimpid = sbi_get_mimpid();

	struct sbiret dbcn_support = sbi_probe_extension(0x4442434E);

	char qemu[8] = {};
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

	interrupts_external_enable(1, 12, handle_keyboard);
	interrupts_external_enable(1, 13, handle_mouse);

	kmi_enable_keyboard();
	kmi_enable_mouse();

	interrupts_enable();

	scrollback_print_line("Hola! Esto es una prueba :)");
	scrollback_print_line("");
	scrollback_print_line("Los docentes de orga2 son:");
	scrollback_print_line("  - agus");
	scrollback_print_line("  - bruno");
	scrollback_print_line("  - bruno");
	scrollback_print_line("  - ed");
	scrollback_print_line("  - eze");
	scrollback_print_line("  - fd");
	scrollback_print_line("  - furfi");
	scrollback_print_line("  - gaspi");
	scrollback_print_line("  - kevin");
	scrollback_print_line("  - maca");
	scrollback_print_line("  - marcos");
	scrollback_print_line("  - mega");
	scrollback_print_line("  - pache");
	scrollback_print_line("  - stef");
	scrollback_print_line("  - tincho");
	scrollback_print_line("  - tomas");
	scrollback_print_line(
		"We're no strangers to love"
		" / You know the rules and so do I (Do I)"
		" / A full commitment's what I'm thinking of"
		" / You wouldn't get this from any other guy"
		" / I just wanna tell you how I'm feeling"
		" / Gotta make you understand"
		" / Never gonna give you up"
		" / Never gonna let you down"
		" / Never gonna run around and desert you"
		" / Never gonna make you cry"
		" / Never gonna say goodbye"
		" / Never gonna tell a lie and hurt you"
		" / We've known each other for so long"
		" / Your heart's been aching, but you're too shy to say it (To say it)"
		" / Inside, we both know what's been going on (Going on)"
		" / We know the game, and we're gonna play it"
		" / And if you ask me how I'm feeling"
		" / Don't tell me you're too blind to see"
		" / Never gonna give you up"
		" / Never gonna let you down"
		" / Never gonna run around and desert you"
		" / Never gonna make you cry"
		" / Never gonna say goodbye"
		" / Never gonna tell a lie and hurt you"
		" / Never gonna give you up"
		" / Never gonna let you down"
		" / Never gonna run around and desert you"
		" / Never gonna make you cry"
		" / Never gonna say goodbye"
		" / Never gonna tell a lie and hurt you"
		" / Ooh (Give you up)"
		" / Ooh-ooh (Give you up)"
		" / Ooh-ooh"
		" / Never gonna give, never gonna give (Give you up)"
		" / Ooh-ooh"
		" / Never gonna give, never gonna give (Give you up)"
		" / We've known each other for so long"
		" / Your heart's been aching, but you're too shy to say it (To say it)"
		" / Inside, we both know what's been going on (Going on)"
		" / We know the game, and we're gonna play it"
		" / I just wanna tell you how I'm feeling"
		" / Gotta make you understand"
		" / Never gonna give you up"
		" / Never gonna let you down"
		" / Never gonna run around and desert you"
		" / Never gonna make you cry"
		" / Never gonna say goodbye"
		" / Never gonna tell a lie and hurt you"
		" / Never gonna give you up"
		" / Never gonna let you down"
		" / Never gonna run around and desert you"
		" / Never gonna make you cry"
		" / Never gonna say goodbye"
		" / Never gonna tell a lie and hurt you"
		" / Never gonna give you up"
		" / Never gonna let you down"
		" / Never gonna run around and desert you"
		" / Never gonna make you cry"
		" / Never gonna say goodbye"
		" / Never gonna tell a lie and hurt you"
	);
	scrollback_draw();

	while (1) asm volatile("");
	return 0;
}
