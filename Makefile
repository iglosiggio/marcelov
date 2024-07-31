AS=riscv64-unknown-elf-as
CC=riscv64-unknown-elf-gcc
LD=riscv64-unknown-elf-ld

CFLAGS=-g -O1 -mcmodel=medany -ffreestanding
LDFLAGS=-nostartfiles -nodefaultlibs -nostdlib -Tlinker.ld

kernel: start.o kernel.o sbi.o
	$(LD) $(LDFLAGS) $^ -o $@

run: kernel
	qemu-system-riscv64 --machine virt -m 128m -serial stdio -gdb tcp::1234 -kernel kernel

clean:
	rm -f kernel *.o

.PHONY: run clean
