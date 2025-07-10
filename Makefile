AS=riscv64-unknown-elf-as
CC=riscv64-unknown-elf-gcc
LD=riscv64-unknown-elf-ld

CFLAGS = -std=gnu2x -g -O0 -mcmodel=medany -ffreestanding
LDFLAGS = -nostdlib -Tlinker.ld

QEMU = qemu-system-riscv64

GDB = riscv64-elf-gdb

FONT=cream12

all: kernel

clean:
	rm -f kernel *.o
	$(MAKE) -C fonts clean
	$(MAKE) -C utils clean

run: kernel
	$(QEMU) -device ramfb --machine virt -m 128m -serial stdio -gdb tcp::1234 -kernel kernel #-S

attach:
	$(GDB) kernel -ex "target remote localhost:1234"

kernel: start.o kernel.o sbi.o qemu.o fb.o virtio.o kmi.o interrupts.o keyboard.o
	$(LD) $(LDFLAGS) $^ -o $@

fb.o: fb.c fonts/$(FONT).inc

fonts/$(FONT).inc: utils
	$(MAKE) -C fonts $(FONT).inc

utils:
	$(MAKE) -C utils all

.PHONY: all clean run utils
