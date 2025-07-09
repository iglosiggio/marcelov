#pragma once

#include <stdint.h>

// https://wiki.osdev.org/PL050_PS/2_Controller
// PL050(KMI) bases under Integrator/CP compatible board.
typedef struct {
	uint32_t cr;   // control register (rw)
	uint32_t stat; // status register (r)
	uint32_t data; // data register (rw)
	uint32_t clk;  // clock divisor register (rw)
	uint32_t ir;   // interrupt control register (r)
} pl050_registers;

struct struct_PL050_CONTROL {
	// The force KMI clock LOW bit field is used to force the PrimeCell KMI
	// clock pad LOW regardless of the state of the KMI FSM.
	uint32_t FKMIC;
	// The force KMI data LOW bit field is used to force the PrimeCell KMI
	// data pad LOW regardless of the state of the KMI finite state machine
	// (FSM).
	uint32_t FKMID;
	// The enable PrimeCell KMI bit field is used to enable the KMI.
	uint32_t KmiEn;
	// Enable transmitter interrupt. This bit field is used to enable the
	// PrimeCell KMI transmitter interrupt.
	uint32_t KMITXINTREn;
	// Enable receiver interrupt. This bit field is used to enable the
	// PrimeCell KMI receiver interrupt.
	uint32_t KMIRXINTREn;
	// 0 = PS2/AT mode (default), 1 = No line control bit mode.
	uint32_t KMITYPE;
};
constexpr struct struct_PL050_CONTROL PL050_CONTROL = { 1, 2, 4, 8, 16, 32 };

struct struct_PL050_STATUS {
	// This bit reflects the status of the KMIDATAIN line after
	// synchronizing.
	uint32_t KMID;
	// This bit reflects the status of the KMICLKIN line after
	// synchronizing and sampling.
	uint32_t KMIC;
	// This bit reflects the parity bit for the last received data byte
	// (odd parity).
	uint32_t RXPARITY;
	// This bit indicates that the PrimeCell KMI is currently receiving
	// data. 0 = idle, 1 = receiving data.
	uint32_t RXBUSY;
	// This bit indicates that the receiver register is full and ready to
	// be read. 0 = receive register empty, 1 = receive register full,
	// ready to be read.
	uint32_t RXFULL;
	// This bit indicates that the PrimeCell KMI is currently sending data.
	// 0 = idle, 1 = currently sending data.
	uint32_t TXBUSY;
	// This bit indicates that the transmit register is empty and ready to
	// transmit. 0 = transmit register full, 1 = transmit register empty,
	// ready to be written.
	uint32_t TXEMPTY;
};
constexpr struct struct_PL050_STATUS PL050_STATUS = { 1, 2, 4, 8, 16, 32, 64 };

static volatile pl050_registers* const keyboard = (void*) 0x102000;
static volatile pl050_registers* const mouse = (void*) 0x103000;

void kmi_init(volatile pl050_registers* device);
void kmi_send(volatile pl050_registers* device, uint8_t command);
void kmi_enable_mouse();
void kmi_enable_keyboard();
