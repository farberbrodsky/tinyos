#!/bin/bash
set -e
. ./iso.sh

echo "in gdb: target remote localhost:1234"
qemu-system-i386 -s -S -cdrom tinyos.iso
