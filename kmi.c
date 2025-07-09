//#include <stdbool.h>
#include "kmi.h"

void kmi_send(volatile pl050_registers* device, uint8_t command) {
	device->data = command;
	while (device->stat & PL050_STATUS.TXBUSY);
	// assert(registers->data == 0xFA);
}

void kmi_enable_mouse() {
	mouse->cr = PL050_CONTROL.KmiEn | PL050_CONTROL.KMIRXINTREn;
	kmi_send(mouse, 0xF4);
}

void kmi_enable_keyboard() {
	keyboard->cr = PL050_CONTROL.KmiEn | PL050_CONTROL.KMIRXINTREn;
	kmi_send(keyboard, 0xF4);
}
