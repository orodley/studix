NAME     = simple-kernel
CC       = i586-elf-gcc
LD       = $(CC)
AS       = i586-elf-as
CFLAGS   = -c -std=c99 -ffreestanding -Wall -Wextra -Werror
LDFLAGS  = -ffreestanding -nostdlib -lgcc
EMU      = qemu-system-i386
SRC_DIR  = src
VPATH    = $(SRC_DIR)
ASM_OBJS = $(patsubst %.s, %.o, $(wildcard $(SRC_DIR)/*.s))
C_OBJS   = $(patsubst %.c, %.o, $(wildcard $(SRC_DIR)/*.c))
OBJS     = $(C_OBJS) $(ASM_OBJS)

.PHONY: all run clean
all: $(NAME).iso

run: $(NAME).iso
	$(EMU) -cdrom $(NAME).iso

$(NAME).iso: $(NAME).bin make_grub_cfg.sh
	@echo
	@echo Generating disk image...
	mkdir -p isodir/boot/grub
	cp $(NAME).bin isodir/boot/
	./make_grub_cfg.sh $(NAME)
	grub-mkrescue -o $@ isodir

$(NAME).bin: linker.ld $(OBJS)
	$(LD) $(LDFLAGS) -T linker.ld -o $@ $(OBJS)


clean:
	rm -f *.o $(SRC_DIR)/*.o
	rm -f $(NAME).bin $(NAME).iso
	rm -rf isodir
