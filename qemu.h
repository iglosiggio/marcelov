#pragma once

#include <stdbool.h>
#include <stdint.h>

#define FW_CFG_SIGNATURE  0x0000
#define FW_CFG_ID         0x0001
#define FW_CFG_FILE_DIR   0x0019
#define FW_CFG_FILE_FIRST 0x0020

void fw_cfg_read_signature(char data_register[8], char dma_address[8]);
bool fw_cfg_dma_read_from(uint16_t selector_from, void* to_addr, uint32_t size);
bool fw_cfg_dma_write_to(uint16_t selector_to, void* from_addr, uint32_t size);
bool fw_cfg_dma_read(void* to_addr, uint32_t size);
bool fw_cfg_dma_write(void* from_addr, uint32_t size);
