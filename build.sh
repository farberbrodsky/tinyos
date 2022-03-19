#!/bin/bash
export PATH="/home/misha/opt/cross/bin:$PATH"
export CC=/home/misha/opt/cross/bin/i686-elf-gcc
export DESTDIR="$(pwd)/usr"
CLEAN_PROJECTS="libc kernel"
HEADER_PROJECTS="libc kernel"
INSTALL_PROJECTS="libc"
FINAL_PROJECTS="kernel"

for p in $CLEAN_PROJECTS; do
    pushd $p
    make clean
    popd
done
rm -rf $DESTDIR/include
for p in $HEADER_PROJECTS; do
    pushd $p
    make install-headers
    popd
done
for p in $INSTALL_PROJECTS; do
    pushd $p
    make install
    popd
done
for p in $FINAL_PROJECTS; do
    pushd $p
    make
    popd
done
