#!/bin/sh

if [ $# -ne 1 ]; then
	exit 1
fi

cat > isodir/boot/grub/grub.cfg <<EOF
set default=0
set timeout=0

menuentry "$1" {
	multiboot /boot/$1.bin
	module    /boot/initrd.img
}
EOF

cat > bochsrc <<EOF
ata0: enabled=1, ioaddr1=0x1f0, ioaddr2=0x3f0, irq=14
ata0-master: type=cdrom, path="$1.iso", status=inserted
display_library: sdl
romimage: file=/usr/share/bochs/BIOS-bochs-latest
vgaromimage: file=/usr/share/vgabios/vgabios.bin
boot: cdrom
magic_break: enabled=1
EOF
