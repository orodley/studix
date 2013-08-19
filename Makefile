NAME    = simple-kernel
CC      = i586-elf-gcc
LD      = $(CC)
AS      = i586-elf-as
CFLAGS  = -c -std=c99 -ffreestanding -Wall -Wextra -Werror
LDFLAGS = -ffreestanding -nostdlib -lgcc
EMU     = qemu-system-i386
VPATH   = src

.PHONY: all run clean
all: $(NAME).iso

run: $(NAME).iso
	$(EMU) -cdrom $(NAME).iso

$(NAME).iso: $(NAME).bin grub.cfg
	@echo
	@echo Generating disk image...
	mkdir -p isodir/boot/grub
	cp $(NAME).bin isodir/boot/
	cp grub.cfg isodir/boot/grub
	grub-mkrescue -o $@ isodir

$(NAME).bin: linker.ld boot.o vga.o kernel.o
	$(LD) $(LDFLAGS) -T linker.ld -o $@ boot.o vga.o kernel.o

boot.o:   boot.s
	$(AS) -o $@ $<
vga.o:    vga.c
	$(CC) $(CFLAGS) -o $@ $<
kernel.o: kernel.c vga.h
	$(CC) $(CFLAGS) -o $@ $<


clean:
	rm -f *.o
	rm -f $(NAME).bin $(NAME).iso
	rm -rf isodir
