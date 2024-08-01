#include <stdbool.h>
#include <stdint.h>

#include "qemu.h"
#include "utils.h"

#define QEMU_FW_CFG_BASE_ADDR 0x10100000
#define QEMU_FW_CFG_DATA_REGISTER_ADDR     (QEMU_FW_CFG_BASE_ADDR +  0)
#define QEMU_FW_CFG_REGISTER_SELECTOR_ADDR (QEMU_FW_CFG_BASE_ADDR +  8)
#define QEMU_FW_CFG_DMA_ADDRESS_ADDR       (QEMU_FW_CFG_BASE_ADDR + 16)

struct fw_cfg {
	volatile uint16_t* register_selector;
	volatile uint64_t* data_register;
	volatile void** dma_address;
};

static
const struct fw_cfg fw_cfg = {
	.register_selector = (void*) QEMU_FW_CFG_REGISTER_SELECTOR_ADDR,
	.data_register = (void*) QEMU_FW_CFG_DATA_REGISTER_ADDR,
	.dma_address = (void*) QEMU_FW_CFG_DMA_ADDRESS_ADDR
};

void fw_cfg_read_signature(char qemu[4], char qemu_cfg[8]) {
	uint64_t data;
	*fw_cfg.register_selector = FW_CFG_SIGNATURE;

	data = *(volatile uint32_t*) fw_cfg.data_register;
	qemu[0] = (data >> (0 * 8)) & 0xFF;
	qemu[1] = (data >> (1 * 8)) & 0xFF;
	qemu[2] = (data >> (2 * 8)) & 0xFF;
	qemu[3] = (data >> (3 * 8)) & 0xFF;

	data = *(volatile uint64_t*) fw_cfg.dma_address;
	qemu_cfg[0] = (data >> (0 * 8)) & 0xFF;
	qemu_cfg[1] = (data >> (1 * 8)) & 0xFF;
	qemu_cfg[2] = (data >> (2 * 8)) & 0xFF;
	qemu_cfg[3] = (data >> (3 * 8)) & 0xFF;
	qemu_cfg[4] = (data >> (4 * 8)) & 0xFF;
	qemu_cfg[5] = (data >> (5 * 8)) & 0xFF;
	qemu_cfg[6] = (data >> (6 * 8)) & 0xFF;
	qemu_cfg[7] = (data >> (7 * 8)) & 0xFF;
}

#define FW_CFG_DMA_ERROR   1
#define FW_CFG_DMA_READ    2
#define FW_CFG_DMA_SKIP    4
#define FW_CFG_DMA_SELECT  8
#define FW_CFG_DMA_WRITE  16

struct fw_cfg_dma_access {
	uint16_t selector;
	volatile uint16_t control;
	uint32_t size;
	uint64_t address;
};

static
bool fw_cfg_dma_transfer(uint16_t selector, uint16_t control, uint32_t size, void* address) {
	struct fw_cfg_dma_access access = (struct fw_cfg_dma_access) {
		.selector = bswap2(selector),
		.control = bswap2(control),
		.size = bswap4(size),
		.address = bswap8((uint64_t) address)
	};
	volatile uint32_t* dma_addr_hi = (void*) fw_cfg.dma_address;
	volatile uint32_t* dma_addr_low = 4 + (void*) fw_cfg.dma_address;
	uint64_t access_low = (uint64_t) &access & 0xFFFFFFFF;
	uint64_t access_hi  = (uint64_t) &access >> 32;
	*dma_addr_hi  = bswap4(access_hi);
	*dma_addr_low = bswap4(access_low);

	do {
		uint16_t control = bswap2(access.control);
		if (control == 0) {
			return false;
		}
		if ((control & FW_CFG_DMA_ERROR) == 1) {
			return true;
		}
	} while (1);
}

bool fw_cfg_dma_read_from(uint16_t selector_from, void* to_addr, uint32_t size) {
	return fw_cfg_dma_transfer(selector_from, FW_CFG_DMA_SELECT | FW_CFG_DMA_READ, size, to_addr);
}

bool fw_cfg_dma_write_to(uint16_t selector_to, void* from_addr, uint32_t size) {
	return fw_cfg_dma_transfer(selector_to, FW_CFG_DMA_SELECT | FW_CFG_DMA_WRITE, size, from_addr);
}

bool fw_cfg_dma_read(void* to_addr, uint32_t size) {
	return fw_cfg_dma_transfer(0, FW_CFG_DMA_READ, size, to_addr);
}

bool fw_cfg_dma_write(void* from_addr, uint32_t size) {
	return fw_cfg_dma_transfer(0, FW_CFG_DMA_WRITE, size, from_addr);
}
