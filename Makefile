NAME     = studix
NAME    := $(NAME)-$(shell git describe)
CC       = i586-elf-gcc
LD       = $(CC)
AS       = i586-elf-as
CFLAGS   = -c -std=c99 -ffreestanding -Isrc/lib -Isrc/include -Wall -Wextra \
           -pedantic -Werror -Wno-unused-parameter -DNAME=\"$(NAME)\"
LDFLAGS  = -ffreestanding -nostdlib -lgcc
EMU      = qemu-system-i386
VPATH    = src
ASM_OBJS = $(patsubst %.s, %.o, $(shell find src -name '*.s'))
C_OBJS   = $(patsubst %.c, %.o, $(shell find src -name '*.c'))
OBJS     = $(C_OBJS) $(ASM_OBJS)

# C compiler used for compiling tools that run on the host during build
HOST_CC     = gcc
HOST_CFLAGS = -std=c99 -Isrc/include -Wall -Wextra -pedantic -Werror

.PHONY: all run clean bochs

all: $(NAME).bin initrd.img

run: $(NAME).bin initrd.img
	$(EMU) -boot order=d -kernel $(NAME).bin -initrd initrd.img -hda disk.img

bochs: $(NAME).iso
	bochs

$(NAME).iso: $(NAME).bin tools/make_config_files.sh initrd.img
	@echo
	@echo Generating disk image...
	cp $(NAME).bin isodir/boot/
	cp initrd.img  isodir/boot/
	tools/make_config_files.sh $(NAME)
	grub-mkrescue -o $@ isodir

initrd.img: tools/make_initrd
	@echo Generating initial ramdisk...
	mkdir -p isodir/boot/grub
	tools/make_initrd initrd initrd.img

tools/make_initrd: tools/make_initrd.c
	$(HOST_CC) -o $@ $(HOST_CFLAGS) $<

$(NAME).bin: linker.ld $(OBJS)
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)


clean:
	rm -f `find . -type f -name '*.o'`
	rm -f *.bin *.iso
	rm -f bochsrc
	rm -f tools/make_initrd
	rm -rf isodir
