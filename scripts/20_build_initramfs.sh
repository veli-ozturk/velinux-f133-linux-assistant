#!/bin/bash
# initramfs: busybox (statik) + fbtest + evtest + init -> ~/f133/out/initramfs.cpio
set -e
P=~/f133/proj; O=~/f133/out; BB=~/f133/busybox; R=$O/rootfs
CC=riscv64-linux-gnu-gcc
if [ ! -x $BB/busybox ]; then
	make -C $BB ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- defconfig >/dev/null
	sed -i 's/# CONFIG_STATIC is not set/CONFIG_STATIC=y/; s/CONFIG_TC=y/# CONFIG_TC is not set/' $BB/.config
	make -C $BB ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- oldconfig </dev/null >/dev/null 2>&1 || true
	make -C $BB ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- -j$(nproc) 2>&1 | grep -E "error|LINK" || true
fi
file $BB/busybox
rm -rf $R; mkdir -p $R/{bin,sbin,usr/bin,usr/sbin,proc,sys,dev,tmp,etc,mnt,root,lib/firmware}
make -C $BB ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu- CONFIG_PREFIX=$R install >/dev/null
$CC -static -O2 -s -o $R/usr/bin/fbtest $P/initramfs/src/fbtest.c
$CC -static -O2 -s -o $R/usr/bin/evtest $P/initramfs/src/evtest.c
sh -n $P/initramfs/rootfs/init || { echo "HATA: init sozdizimi"; exit 1; }
cp ~/f133/mod/plicdump.ko $R/lib/ 2>/dev/null || true   # PLIC tani modulu (modules/plicdump)
mkdir -p $R/lib/wifi && cp ~/f133/out/wifi/ssv6x5x.ko ~/f133/out/wifi/ssv6x5x-wifi.cfg $R/lib/wifi/ 2>/dev/null || true   # SV6256P (21_build_wifi.sh)
cp $P/initramfs/rootfs/init $R/init; sed -i 's/\r$//' $R/init; chmod +x $R/init
echo "root::0:0:root:/root:/bin/sh" > $R/etc/passwd
[ "$F133_DEBUG_RESET" = 1 ] && touch $R/etc/f133-debug-reset   # yalniz F133_DEBUG_RESET=1 ile: 35 sn sonra reset (dmesg RAM'den okunur)
# Aygit dugumleri (devtmpfs oncesi konsol icin)
(cd $R && fakeroot -- sh -c 'mknod -m 600 dev/console c 5 1; mknod -m 666 dev/null c 1 3; find . | cpio -o -H newc' > $O/initramfs.cpio 2>/dev/null) \
 || (cd $R && find . | cpio -o -H newc > $O/initramfs.cpio 2>/dev/null)
ls -l $O/initramfs.cpio; du -sh $R
