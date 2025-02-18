AS=riscv64-unknown-elf-as
CC=riscv64-unknown-elf-gcc
LD=riscv64-unknown-elf-ld

CFLAGS=-g -O0 -mcmodel=medany -ffreestanding
LDFLAGS=-nostartfiles -nodefaultlibs -nostdlib -Tlinker.ld

all: kernel

clean:
	rm -f kernel *.o
	$(MAKE) -C fonts clean
	$(MAKE) -C utils clean

run: kernel
	qemu-system-riscv64 -device ramfb -global virtio-mmio.force-legacy=false -device virtio-keyboard-device -device virtio-mouse-device --machine virt -m 128m -serial stdio -gdb tcp::1234 -kernel kernel

kernel: start.o kernel.o sbi.o qemu.o fb.o virtio.o
	$(LD) $(LDFLAGS) $^ -o $@

fb.o: fb.c fonts/cream12.inc

fonts/cream12.inc: utils
	$(MAKE) -C fonts cream12.inc

utils:
	$(MAKE) -C utils all

.PHONY: all clean run utils
