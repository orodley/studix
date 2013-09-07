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

all: $(NAME).iso

run: $(NAME).iso
	$(EMU) -boot order=d -cdrom $(NAME).iso

bochs: $(NAME).iso
	bochs

tools/make_initrd: tools/make_initrd.c
	$(HOST_CC) -o $@ $(HOST_CFLAGS) $<

$(NAME).iso: $(NAME).bin tools/make_config_files.sh
	@echo
	@echo Generating disk image...
	mkdir -p isodir/boot/grub
	cp $(NAME).bin isodir/boot/
	tools/make_config_files.sh $(NAME)
	grub-mkrescue -o $@ isodir

$(NAME).bin: linker.ld $(OBJS)
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)


clean:
	rm -f `find . -type f -name '*.o'`
	rm -f *.bin *.iso
	rm -f bochsrc
	rm -f tools/make_init_rd
	rm -rf isodir
