#!/bin/bash
# Kernel Image + DTB derler, ciktilari ~/f133/out ve Windows'taki linux-build/out'a kopyalar.
set -e
K=~/f133/linux; B=~/f133/build; O=~/f133/out; W="${VELINUX_ROOT:-$HOME/velinux}/linux-build/out"
mkdir -p $O "$W"
[ -f $O/initramfs.cpio ] || { echo "initramfs yok, bos yer tutucu"; mkdir -p $O/empty-rd; (cd $O/empty-rd && mkdir -p dev && echo | cpio -o -H newc > $O/initramfs.cpio 2>/dev/null); }
time make -C $K O=$B ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc) Image 2>&1 | tee $O/kernel-build.log | grep -E "error|Error|warning: (implicit|unused)|CC |LD |OBJCOPY" --line-buffered | grep -v "^ *CC " ; true
test -f $B/arch/riscv/boot/Image || { echo "!!! DERLEME HATASI, son satirlar:"; grep -n -B2 -A5 "error" $O/kernel-build.log | tail -40; exit 1; }
cp $B/arch/riscv/boot/Image $B/System.map $O/
lz4 -l -12 -f -q $O/Image $O/Image.lz4
cp $O/Image $O/Image.lz4 $O/System.map "$W/"
ls -l $O/Image
echo "=== KERNEL HAZIR ==="
