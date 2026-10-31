#!/bin/bash
# sun20iw1p1_defconfig + F133 panel ayarlari. Cikti: ~/f133/build/.config
set -e
K=~/f133/linux; B=~/f133/build
M="make -C $K O=$B ARCH=riscv CROSS_COMPILE=riscv64-linux-gnu-"
$M sun20iw1p1_defconfig
C="$K/scripts/config --file $B/.config"
# T-Head vektor 0.7 standart araç zinciriyle derlenmez
$C -d VECTOR
# DVFS yok (CPU PWM regulatoru tanimsiz)
$C -d CPU_FREQ
# initramfs (kaynak 20_build_initramfs.sh ile uretilir)
$C -e BLK_DEV_INITRD --set-str INITRAMFS_SOURCE "$HOME/f133/out/initramfs.cpio" -e RD_GZIP
$C -e INITRAMFS_COMPRESSION_GZIP
# Ekran + konsol
$C -e DISP2_SUNXI -e LCD_SUPPORT_HE0801A068 -e FRAMEBUFFER_CONSOLE -e FONTS -e FONT_8x16 -e VT -e VT_CONSOLE
$C -e LOGO -e LOGO_LINUX_CLUT224
# Dokunmatik
$C -e INPUT_EVDEV -e INPUT_TOUCHSCREEN -e TOUCHSCREEN_GOODIX -e I2C_SUNXI -e I2C_CHARDEV
# USB aygit: seri konsol (g_serial, ACM). Windows usbser ile surucusuz COM portu olur.
# (g_cdc'deki ECM agi Windows'ta surucusuz calismaz; ag sonra RNDIS ile eklenecek)
$C -e USB -e USB_GADGET -e USB_SUNXI_UDC0 -e USB_SUNXI_USB -e USB_SUNXI_HCI -e USB_SUNXI_HCD -e USB_SUNXI_USB_MANAGER
$C -d USB_CDC_COMPOSITE -e USB_G_SERIAL -e U_SERIAL_CONSOLE -d USB_CONFIGFS
# Tina'nin cpuidle'i Tina OpenSBI uzantisi bekleyebilir; kapali
$C -d CPU_IDLE
# Wi-Fi (ssv6x5x kernel disi modul)
$C -e WIRELESS -e CFG80211 -e MAC80211 -e RFKILL -e MODULES -e MODULE_UNLOAD -e FW_LOADER
# SD kart (okuma) + dosya sistemleri
$C -e MMC -e MMC_SUNXI -e VFAT_FS -e MSDOS_FS -e NLS_CODEPAGE_437 -e NLS_ISO8859_1 -e NLS_CODEPAGE_857
# SPI NOR surucusu adim 1'de derlenmez
$C -d MTD
# Watchdog: OpenSBI hata ayiklama surumu sistem watchdogunu kullaniyor, kernel karismasin
$C -d WATCHDOG
# Hata ayiklama
$C -e PRINTK_TIME -e MAGIC_SYSRQ -e IKCONFIG -e IKCONFIG_PROC
$M olddefconfig
echo "--- secilen anahtarlar ---"
grep -E "^CONFIG_(VECTOR|CPU_FREQ|BLK_DEV_INITRD|INITRAMFS_SOURCE|DISP2_SUNXI|LCD_SUPPORT_HE0801A068|FRAMEBUFFER_CONSOLE|TOUCHSCREEN_GOODIX|USB_SUNXI_UDC0|USB_G_SERIAL|CPU_IDLE|USB_SUNXI_USB_MANAGER|CFG80211|MAC80211|MMC_SUNXI|MTD|VFAT_FS)=" $B/.config || true
grep -E "^# CONFIG_(VECTOR|CPU_FREQ|MTD|USB_G_SERIAL|CPU_IDLE|TOUCHSCREEN_GOODIX|USB_SUNXI_UDC0|USB_SUNXI_USB_MANAGER) is not set" $B/.config || true
