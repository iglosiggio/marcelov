#include "sbi.h"

static inline
long sbi_legacycall0(int sbi_extension_id) {
	register int eid asm("a7") = sbi_extension_id;
	register long value asm("a0");
	asm volatile ("ecall"
	    : "=r" (value)
	    : "r" (eid));
	return value;
}

static inline
long sbi_legacycall1(unsigned long arg0, int sbi_extension_id) {
	register int eid asm("a7") = sbi_extension_id;
	register long arg0_r asm("a0") = arg0;
	register long value asm("a0");
	asm volatile ("ecall"
	    : "=r" (value)
	    : "r" (arg0_r), "r" (eid));
	return value;
}

static inline
long sbi_legacycall3(unsigned long arg0, unsigned long arg1, unsigned long arg2, int sbi_extension_id) {
	register int eid asm("a7") = sbi_extension_id;
	register long arg0_r asm("a0") = arg0;
	register long arg1_r asm("a1") = arg1;
	register long arg2_r asm("a2") = arg2;
	register long value asm("a0");
	asm volatile ("ecall"
	    : "=r" (value)
	    : "r" (arg0_r), "r" (arg1_r), "r" (arg2_r), "r" (eid));
	return value;
}

static inline
long sbi_legacycall4(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, int sbi_extension_id) {
	register int eid asm("a7") = sbi_extension_id;
	register long arg0_r asm("a0") = arg0;
	register long arg1_r asm("a1") = arg1;
	register long arg2_r asm("a2") = arg2;
	register long arg3_r asm("a3") = arg3;
	register long value asm("a0");
	asm volatile ("ecall"
	    : "=r" (value)
	    : "r" (arg0_r), "r" (arg1_r), "r" (arg2_r), "r" (arg3_r), "r" (eid));
	return value;
}

static inline
struct sbiret sbi_call0(int sbi_extension_id, int sbi_function_id) {
	register int eid asm("a7") = sbi_extension_id;
	register int fid asm("a6") = sbi_function_id;
	register long error asm("a0");
	register long value asm("a1");
	asm volatile ("ecall"
	    : "=r" (error), "=r" (value)
	    : "r" (eid), "r" (fid));
	return (struct sbiret) { .error = error, .value = value };
}

static inline
struct sbiret sbi_call1(unsigned long arg0, int sbi_extension_id, int sbi_function_id) {
	register int eid asm("a7") = sbi_extension_id;
	register int fid asm("a6") = sbi_function_id;
	register unsigned long arg0_r asm("a0") = arg0;
	register long error asm("a0");
	register long value asm("a1");
	asm volatile ("ecall"
	    : "=r" (error), "=r" (value)
	    : "r" (arg0_r), "r" (eid), "r" (fid));
	return (struct sbiret) { .error = error, .value = value };
}

static inline
struct sbiret sbi_call2(unsigned long arg0, unsigned long arg1, int sbi_extension_id, int sbi_function_id) {
	register int eid asm("a7") = sbi_extension_id;
	register int fid asm("a6") = sbi_function_id;
	register unsigned long arg0_r asm("a0") = arg0;
	register unsigned long arg1_r asm("a1") = arg1;
	register long error asm("a0");
	register long value asm("a1");
	asm volatile ("ecall"
	    : "=r" (error), "=r" (value)
	    : "r" (arg0_r), "r" (arg1_r), "r" (eid), "r" (fid));
	return (struct sbiret) { .error = error, .value = value };
}

static inline
struct sbiret sbi_call3(unsigned long arg0, unsigned long arg1, unsigned long arg2, int sbi_extension_id, int sbi_function_id) {
	register int eid asm("a7") = sbi_extension_id;
	register int fid asm("a6") = sbi_function_id;
	register unsigned long arg0_r asm("a0") = arg0;
	register unsigned long arg1_r asm("a1") = arg1;
	register unsigned long arg2_r asm("a2") = arg2;
	register long error asm("a0");
	register long value asm("a1");
	asm volatile ("ecall"
	    : "=r" (error), "=r" (value)
	    : "r" (arg0_r), "r" (arg1_r), "r" (arg2_r), "r" (eid), "r" (fid));
	return (struct sbiret) { .error = error, .value = value };
}

/* Base Extension */
struct sbiret sbi_get_spec_version(void) {
	return sbi_call0(0x10, 0);
}

struct sbiret sbi_get_impl_id(void) {
	return sbi_call0(0x10, 1);
}

struct sbiret sbi_get_impl_version(void) {
	return sbi_call0(0x10, 2);
}

struct sbiret sbi_probe_extension(long extension_id) {
	return sbi_call1(extension_id, 0x10, 3);
}

struct sbiret sbi_get_mvendorid(void) {
	return sbi_call0(0x10, 4);
}

struct sbiret sbi_get_marchid(void) {
	return sbi_call0(0x10, 5);
}

struct sbiret sbi_get_mimpid(void) {
	return sbi_call0(0x10, 6);
}

/* Legacy Extensions */
long sbi_set_timer(unsigned long stime_value) {
	return sbi_legacycall1(stime_value, 0x0);
}

long sbi_console_putchar(int ch) {
	return sbi_legacycall1(ch, 0x1);
}

long sbi_console_getchar(void) {
	return sbi_legacycall0(0x2);
}

long sbi_clear_ipi(void) {
	return sbi_legacycall0(0x3);
}

long sbi_send_ipi(const unsigned long *hart_mask) {
	return sbi_legacycall1((unsigned long) hart_mask, 0x4);
}

long sbi_remote_fence_i(const unsigned long *hart_mask) {
	return sbi_legacycall1((unsigned long) hart_mask, 0x5);
}

long sbi_remote_sfence_vma(
	const unsigned long *hart_mask,
	unsigned long start,
	unsigned long size
) {
	return sbi_legacycall3((unsigned long) hart_mask, start, size, 0x6);
}

long sbi_remote_sfence_vma_asid(
	const unsigned long *hart_mask,
	unsigned long start,
	unsigned long size,
	unsigned long asid
) {
	return sbi_legacycall4((unsigned long) hart_mask, start, size, asid, 0x7);
}

void sbi_shutdown(void) {
	sbi_legacycall0(0x8);
}

struct sbiret sbi_debug_console_write(
	unsigned long num_bytes,
	unsigned long base_addr_lo,
	unsigned long base_addr_hi
) {
	sbi_call3(num_bytes, base_addr_lo, base_addr_hi, 0x4442434E, 0);
}
