CC = gcc
LD = ld
CFLAGS = -m32 -ffreestanding -fno-pie -fno-stack-protector

OBJS = kernel.o keyboard.o shell.o

all: kernel.bin

kernel.bin: $(OBJS)
	$(LD) -m elf_i386 -T linker.ld -o kernel.bin $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o kernel.bin

run: kernel.bin
	qemu-system-i386 -kernel kernel.bin -vga std