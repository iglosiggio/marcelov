#pragma once

#include <stdint.h>
#include <stdbool.h>

// Internal IRQ codes
// https://five-embeddev.com/riscv-priv-isa-manual/Priv-v1.12/supervisor.html#supervisor-interrupt-registers-sip-and-sie
#define SOFTWARE_IRQ 1
#define TIMER_IRQ 5
#define EXTERNAL_IRQ 9

// External IRQ codes
#define KEYBOARD_IRQ 12
#define MOUSE_IRQ 13

#define SUPERVISOR_CONTEXT 1

typedef void (*handler_fn)(void);

void interrupts_enable();
bool interrupts_external_query(uint32_t context, uint32_t interrupt);
void interrupts_external_enable(uint32_t context, uint32_t interrupt, handler_fn handler);
void interrupts_external_disable(uint32_t context, uint32_t interrupt);
