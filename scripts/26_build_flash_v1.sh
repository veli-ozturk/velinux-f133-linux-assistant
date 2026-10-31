#!/bin/bash
# SPI NOR duzeni (boot0 DOKUNULMAZ, toc1 @0x10000). Cikti: linux-build/flash/
#   opensbi : Tina opensbi_sun20iw1p1.bin (degismez, +0x2C=0x41FC0000)  -> 0x41FC0000
#   u-boot  : kopru (+4 "uboot", +0x2C=0x42000000, kod @0x800: a1=DTB, jr 0x40000000) + DTB @+0x1000 -> 0x42000000
#   melis   : kernel Image (initramfs GOMULU DEGIL, 11_kernel_config_v3.sh), +0x2C=0x40000000 -> 0x40000000
#   rtos    : 4 KB baslik (+0x2C=0x42200000) + initramfs.cpio.gz -> 0x42200000 (initrd 0x42201000)
# boot0 sirasi: opensbi varsa once ona atlar (a1=uboot_base). Tina SBI sabit 0x42000000'a gider.
# Kernel < 0x40800000 olmali (boot0 heap 0x40800000-0x41800000).
set -e
P=~/f133/proj; O=~/f133/out; F=$P/flash; T=$O/flash-tmp
mkdir -p $F $T
SBI=$P/tina-sbi/opensbi_sun20iw1p1.bin
DTB0=$P/tina-sbi/board-tina-sbi.dtb
RD_ADDR=0x42200000; RD_START=0x42201000

# 1) initramfs.cpio.gz (20_build_initramfs_v4.sh ciktisi)
gzip -9 -n -c $O/initramfs.cpio > $T/initramfs.cpio.gz
RDLEN=$(stat -c %s $T/initramfs.cpio.gz)
RD_END=$(printf '0x%x' $((RD_START + RDLEN)))

# 2) DTB + chosen/linux,initrd-*
cp $DTB0 $T/board.dtb
fdtput -t x $T/board.dtb /chosen linux,initrd-start $RD_START
fdtput -t x $T/board.dtb /chosen linux,initrd-end $RD_END
echo "chosen: initrd $(fdtget -t x $T/board.dtb /chosen linux,initrd-start) .. $(fdtget -t x $T/board.dtb /chosen linux,initrd-end)"
echo "bootargs: $(fdtget $T/board.dtb /chosen bootargs)"

# 3) kopru (u-boot ogesi)
cat > $T/bridge.S <<'S'
.option norvc
.globl _start
_start:
	j code
	.ascii "uboot"
	.org 0x2C
	.word 0x42000000
	.org 0x800
code:
	li a1, 0x42001000
	li t0, 0x40000000
	jr t0
	.org 0x1000
S
riscv64-linux-gnu-as -march=rv64imac -o $T/bridge.o $T/bridge.S
riscv64-linux-gnu-ld -Ttext=0x42000000 -o $T/bridge.elf $T/bridge.o
riscv64-linux-gnu-objcopy -O binary $T/bridge.elf $T/bridge.bin
test $(stat -c %s $T/bridge.bin) -eq 4096
cat $T/bridge.bin $T/board.dtb > $F/item-uboot.bin

# 4) kernel (melis ogesi): +0x2C'ye 0x40000000 (Image basliginda res2, sifir olmali)
IMG=$O/Image
test "$(xxd -s 0x28 -l 8 -p $IMG)" = "0000000000000000" || { echo "!!! Image +0x28 bos degil"; exit 1; }
cp $IMG $F/item-melis.bin
printf '\x00\x00\x00\x40' | dd of=$F/item-melis.bin bs=1 seek=44 conv=notrunc status=none
KLEN=$(stat -c %s $F/item-melis.bin)
[ $((0x40000000 + KLEN)) -lt $((0x40800000)) ] || { echo "!!! kernel $KLEN bayt, 0x40800000'i asiyor"; exit 1; }

# 5) initrd (rtos ogesi)
python3 -c "import struct,sys; h=bytearray(4096); struct.pack_into('<I',h,0x2C,$RD_ADDR); h[0:8]=b'F133INRD'; sys.stdout.buffer.write(bytes(h))" > $F/item-rtos.bin
cat $T/initramfs.cpio.gz >> $F/item-rtos.bin

# 6) opensbi
cp $SBI $F/item-opensbi.bin
test "$(xxd -s 0x2C -l 4 -p $F/item-opensbi.bin)" = "0000fc41"

# 7) toc1
python3 $P/scripts/toc1_build_v1.py $F/toc1.bin opensbi=$F/item-opensbi.bin u-boot=$F/item-uboot.bin \
	melis=$F/item-melis.bin rtos=$F/item-rtos.bin
printf 'kernel 0x40000000..0x%x, initrd 0x%x..%s (%d B)\n' $((0x40000000 + KLEN)) $RD_START $RD_END $RDLEN
cp $T/board.dtb $F/board-flash.dtb
sha256sum $F/*.bin | sed "s|$F/||"
ls -l $F
