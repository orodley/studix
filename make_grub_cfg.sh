#!/bin/sh

if [ $# -ne 1 ]; then
	exit 1
fi

cat > isodir/boot/grub/grub.cfg <<EOF
set default=0
set timeout=0

menuentry "$1" {
	multiboot /boot/$1.bin
}
EOF
