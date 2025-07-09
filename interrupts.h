#pragma once

#include <stdint.h>
#include <stdbool.h>

void interrupts_enable();
bool interrupts_external_query(uint32_t context, uint32_t interrupt);
void interrupts_external_set(uint32_t context, uint32_t interrupt, bool value);
uint32_t interrupts_claim(uint32_t context, uint32_t interrupt);
void interrupts_complete(uint32_t context, uint32_t interrupt, uint32_t claim);
