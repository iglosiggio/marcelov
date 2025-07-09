#include "encoding.h"
#include "interrupts.h"
#include "utils.h"

#define MCAUSE_INT   0x8000000000000000
#define MCAUSE_CAUSE 0x7FFFFFFFFFFFFFFF

[[gnu::interrupt("supervisor")]]
[[gnu::aligned(4)]]
void handle_trap(void);

#define EXTERNAL_TABLE_SIZE 32
handler_fn external_handler[EXTERNAL_TABLE_SIZE];

void handle_external_irq();

#define ISR_TABLE_SIZE 32
handler_fn isr_handler[ISR_TABLE_SIZE] = {
	[EXTERNAL_IRQ] = handle_external_irq,
};

#define EXCEPTION_TABLE_SIZE 32
handler_fn exception_handler[EXCEPTION_TABLE_SIZE];

static volatile void* const PLIC_BASE = (void*) 0xC000000;

// https://cdn2.hubspot.net/hubfs/3020607/An%20Introduction%20to%20the%20RISC-V%20Architecture.pdf
// https://five-embeddev.com/riscv-priv-isa-manual/Priv-v1.12/supervisor.html#supervisor-trap-vector-base-address-register-stvec
void interrupts_enable() {
	write_csr(stvec, (uintptr_t) &handle_trap);
	write_csr(sstatus, read_csr(sstatus) | SSTATUS_SIE);
	write_csr(sie, 1 << EXTERNAL_IRQ /* SEIE */);

	const uint32_t supervisor_context = 1;
	volatile uint32_t* priority_threshold = PLIC_BASE + 0x200000 + 0x1000 * supervisor_context;
	*priority_threshold = 0;
}

bool interrupts_external_query(uint32_t context, uint32_t interrupt) {
	uint32_t mask = 1 << interrupt;
	volatile uint32_t* enabled_register = PLIC_BASE + 0x2000 + 0x80 * context + interrupt / 32;
	return (*enabled_register & mask) == mask;
}

void interrupts_external_enable(uint32_t context, uint32_t interrupt, handler_fn handler) {
	if (EXTERNAL_TABLE_SIZE <= interrupt) return; // Out of bounds!
	uint32_t mask = 1 << interrupt;
	volatile uint32_t* priority = PLIC_BASE + 0x4 * interrupt;
	volatile uint32_t* enabled_register = PLIC_BASE + 0x2000 + 0x80 * context + interrupt / 32;
	*enabled_register = *enabled_register | mask;
	*priority = 1;
	external_handler[interrupt] = handler;
}

void interrupts_external_disable(uint32_t context, uint32_t interrupt) {
	if (EXTERNAL_TABLE_SIZE <= interrupt) return; // Out of bounds!
	uint32_t mask = 1 << interrupt;
	volatile uint32_t* priority = PLIC_BASE + 0x4 * interrupt;
	volatile uint32_t* enabled_register = PLIC_BASE + 0x2000 + 0x80 * context + interrupt / 32;
	*enabled_register = *enabled_register & ~mask;
	*priority = 0;
	external_handler[interrupt] = 0x0;
}

uint32_t interrupts_claim(uint32_t context) {
	volatile uint32_t* claim_register = PLIC_BASE + 0x200000 + 0x1000 * context + 0x4;
	return *claim_register;
}

void interrupts_complete(uint32_t context, uint32_t claim) {
	volatile uint32_t* claim_register = PLIC_BASE + 0x200000 + 0x1000 * context + 0x4;
	*claim_register = claim;
}

void handle_external_irq() {
	uint32_t claim = interrupts_claim(SUPERVISOR_CONTEXT);
	if (claim < EXTERNAL_TABLE_SIZE && external_handler[claim]) {
		external_handler[claim]();

		//FIXME: Trace
		//print("Finished handling ");
		//print_hex(claim);
		//print("\n");
	} else {
		print("Ignoring unknown interrupt ");
		print_hex(claim);
		print("\n");
	}
	interrupts_complete(SUPERVISOR_CONTEXT, claim);
}

void handle_trap() {
	uint64_t scause = read_csr(scause);
	uint64_t id = scause & MCAUSE_CAUSE;
	if (scause & MCAUSE_INT) {
		if (id < ISR_TABLE_SIZE && isr_handler[id]) {
			isr_handler[id]();
		}
	} else {
		if (id < EXCEPTION_TABLE_SIZE && exception_handler[id]) {
			exception_handler[id]();
		}
		print("Finished handling exception ");
		print_hex(id);
		print("\n");
	}
}
