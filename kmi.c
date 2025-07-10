//#include <stdbool.h>
#include "kmi.h"

void kmi_send(volatile pl050_registers* device, uint8_t command) {
	device->data = command;
	while (device->stat & PL050_STATUS.TXBUSY);
	// assert(registers->data == 0xFA);
}

void kmi_send_with_data(volatile pl050_registers* device, uint8_t command, uint8_t data) {
	device->data = command;
	while (device->stat & PL050_STATUS.TXBUSY);
	device->data = data;
	while (device->stat & PL050_STATUS.TXBUSY);
}

void kmi_enable_mouse() {
	mouse->cr = PL050_CONTROL.KmiEn | PL050_CONTROL.KMIRXINTREn;
	kmi_send(mouse, 0xF4);
}

void kmi_enable_keyboard() {
	keyboard->cr = PL050_CONTROL.KmiEn | PL050_CONTROL.KMIRXINTREn;
	kmi_send(keyboard, 0xF4);
	kmi_send_with_data(keyboard, 0xF0, 1);
}
