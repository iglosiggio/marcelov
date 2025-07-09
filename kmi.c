//#include <stdbool.h>
#include "kmi.h"

void kmi_init(volatile pl050_registers* device) {
	device->cr = PL050_CONTROL.KmiEn | PL050_CONTROL.KMIRXINTREn;
}

void kmi_send(volatile pl050_registers* device, uint8_t command) {
	device->data = command;
	while (device->stat & PL050_STATUS.TXBUSY);
	// assert(registers->data == 0xFA);
}

void kmi_enable_mouse() {
	kmi_init(mouse);
	kmi_send(mouse, 0xF4);
}

void kmi_enable_keyboard() {
	kmi_init(keyboard);
	kmi_send(keyboard, 0xF4);
}
