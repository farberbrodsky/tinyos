#!/bin/bash
set -e
. ./build.sh
 
mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub
 
cp kernel/tinyos.bin isodir/boot/tinyos.bin
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "tinyos" {
	multiboot /boot/tinyos.bin
}
EOF
grub-mkrescue -o tinyos.iso isodir
