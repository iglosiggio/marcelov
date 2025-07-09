#include "encoding.h"
#include "interrupts.h"

#define MCAUSE_INT   0x8000000000000000
#define MCAUSE_CAUSE 0x7FFFFFFFFFFFFFFF

[[gnu::interrupt("supervisor")]]
[[gnu::aligned(4)]]
void handle_trap(void);

static volatile void* const PLIC_BASE = (void*) 0xC000000;

// https://cdn2.hubspot.net/hubfs/3020607/An%20Introduction%20to%20the%20RISC-V%20Architecture.pdf
// https://five-embeddev.com/riscv-priv-isa-manual/Priv-v1.12/supervisor.html#supervisor-trap-vector-base-address-register-stvec
void interrupts_enable() {
	write_csr(stvec, (uintptr_t) &handle_trap);
	write_csr(sstatus, read_csr(sstatus) | SSTATUS_SIE);
	//write_csr(sie, 0xFFFFFFFFFFFF0000);

	const uint32_t supervisor_context = 1;
	volatile uint32_t* priority_threshold = PLIC_BASE + 0x200000 + 0x1000 * supervisor_context;
	*priority_threshold = 0;
}

bool interrupts_external_query(uint32_t context, uint32_t interrupt) {
	uint32_t mask = 1 << interrupt;
	volatile uint32_t* enabled_register = PLIC_BASE + 0x2000 + 0x80 * context + interrupt / 32;
	return (*enabled_register & mask) == mask;
}

void interrupts_external_set(uint32_t context, uint32_t interrupt, bool value) {
	uint32_t mask = 1 << interrupt;
	uint32_t bit_to_set = value ? mask : 0;
	volatile uint32_t* priority = PLIC_BASE + 0x4 * interrupt;
	volatile uint32_t* enabled_register = PLIC_BASE + 0x2000 + 0x80 * context + interrupt / 32;
	*enabled_register = (*enabled_register & ~mask) | bit_to_set;
	*priority = 1;
}

uint32_t interrupts_claim(uint32_t context, uint32_t interrupt) {
	volatile uint32_t* claim_register = PLIC_BASE + 0x200000 + 0x1000 * context + 0x4;
	return *claim_register;
}

void interrupts_complete(uint32_t context, uint32_t interrupt, uint32_t claim) {
	volatile uint32_t* claim_register = PLIC_BASE + 0x200000 + 0x1000 * context + 0x4;
	*claim_register = claim;
}

typedef void (*handler_fn)(void);
#define ISR_TABLE_SIZE 32
handler_fn isr_handler[ISR_TABLE_SIZE];

#define EXCEPTION_TABLE_SIZE 32
handler_fn exception_handler[EXCEPTION_TABLE_SIZE];

#include "utils.h"
void handle_trap() {
	uint64_t scause = read_csr(scause);
	uint64_t id = scause & MCAUSE_CAUSE;
	if (scause & MCAUSE_INT) {
		print_hex(id);
		print("\n");
		if (id < ISR_TABLE_SIZE && isr_handler[id]) {
			isr_handler[id]();
		}
	} else {
		uint32_t claim = interrupts_claim(1, id);
		if (id < EXCEPTION_TABLE_SIZE && exception_handler[id]) {
			exception_handler[id]();
		}
		interrupts_complete(1, id, claim);
		print("Finished handling ");
		print_hex(claim);
		print(" for id ");
		print_hex(id);
		print("\n");
	}
}
