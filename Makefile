NAME    = simple-kernel
CC      = i586-elf-gcc
LD      = $(CC)
AS      = i586-elf-as
CFLAGS  = -c -std=c99 -ffreestanding -Wall -Wextra -Werror
LDFLAGS = -ffreestanding -nostdlib -lgcc
EMU     = qemu-system-i386

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

$(NAME).bin: boot.o kernel.o linker.ld vga.o
	$(LD) $(LDFLAGS) -T linker.ld -o $@ boot.o kernel.o vga.o

boot.o:   src/boot.s
	$(AS) -o $@ $<
vga.o:    src/vga.c
	$(CC) $(CFLAGS) -o $@ $<
kernel.o: src/kernel.c src/vga.h
	$(CC) $(CFLAGS) -o $@ $<


clean:
	rm -f *.o
	rm -f $(NAME).bin $(NAME).iso
	rm -rf isodir
