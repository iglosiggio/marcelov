#pragma once

struct sbiret {
	long error;
	long value;
};

#define SBI_SUCCESS                0  /* Completed successfully */
#define SBI_ERR_FAILED            -1  /* Failed */
#define SBI_ERR_NOT_SUPPORTED     -2  /* Not supported */
#define SBI_ERR_INVALID_PARAM     -3  /* Invalid parameter(s) */
#define SBI_ERR_DENIED            -4  /* Denied or not allowed */
#define SBI_ERR_INVALID_ADDRESS   -5  /* Invalid address(s) */
#define SBI_ERR_ALREADY_AVAILABLE -6  /* Already available */
#define SBI_ERR_ALREADY_STARTED   -7  /* Already started */
#define SBI_ERR_ALREADY_STOPPED   -8  /* Already stopped */
#define SBI_ERR_NO_SHMEM          -9  /* Shared memory not available */
#define SBI_ERR_INVALID_STATE     -10 /* Invalid state */
#define SBI_ERR_BAD_RANGE         -11 /* Bad (or invalid) range */

/* Base Extension */
struct sbiret sbi_get_spec_version(void);
struct sbiret sbi_get_impl_id(void);
struct sbiret sbi_get_impl_version(void);
struct sbiret sbi_probe_extension(long extension_id);
struct sbiret sbi_get_mvendorid(void);
struct sbiret sbi_get_marchid(void);
struct sbiret sbi_get_mimpid(void);

/* Legacy Extensions */
long sbi_set_timer(unsigned long stime_value);
long sbi_console_putchar(int ch);
long sbi_console_getchar(void);
long sbi_clear_ipi(void);
long sbi_send_ipi(const unsigned long *hart_mask);
long sbi_remote_fence_i(const unsigned long *hart_mask);
long sbi_remote_sfence_vma(
	const unsigned long *hart_mask,
	unsigned long start,
	unsigned long size
);
long sbi_remote_sfence_vma_asid(
	const unsigned long *hart_mask,
	unsigned long start,
	unsigned long size,
	unsigned long asid
);
void sbi_shutdown(void);

struct sbiret sbi_debug_console_write(
	unsigned long num_bytes,
	unsigned long base_addr_lo,
	unsigned long base_addr_hi
);
