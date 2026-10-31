#!/bin/sh
# RAM G2D+zram proof v1. No flash erase, no SD panel override, no MCU TX.
#  1) USB seri konsol (COM) her durumda en once   2) /data (SPI NOR "data", JFFS2) kalici ayarlar
#  3) kayitli Wi-Fi (wifi-baslat, guc korumali)   4) ekranda panel uygulamasi (/usr/bin/panel), cokerse yeniden
export PATH=/bin:/sbin:/usr/bin:/usr/sbin
export TZ='<+03>-3'
export ENV=/etc/profile

mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t devtmpfs devtmpfs /dev 2>/dev/null
mkdir -p /dev/pts /tmp /run /var/run /mnt/sd /data
mount -t devpts devpts /dev/pts
mount -t tmpfs tmpfs /tmp
# VeLinux KOLE acilis ekrani: once logo, ekrana hicbir Linux yazisi dusmesin (cekirdek: quiet loglevel=1;
# init ciktisi /tmp/init.log'a). acilis tty1'i grafik kipine alir, panel gelene kadar logo kalir.
exec > /tmp/init.log 2>&1 < /dev/null
# acilis uses old portrait geometry; omitted in this RAM proof.
mount -t debugfs debugfs /sys/kernel/debug 2>/dev/null
echo /sbin/mdev > /proc/sys/kernel/hotplug
mdev -s
hostname velinux

msg() { echo "[init] $*" > /dev/kmsg; }

# HATA AYIKLAMA: /etc/f133-debug-reset varsa 35 sn sonra watchdog reseti (dmesg RAM'den okunur).
if [ -e /etc/f133-debug-reset ]; then
	( sleep 35; echo "[init] debug reset" > /dev/kmsg
	  devmem 0x020500B4 32 0x16aa0001; devmem 0x020500B8 32 0x16aa0001; devmem 0x020500B0 32 0x14af ) &
fi
msg "VeLinux KOLE acildi:$(uname -r), $(grep MemTotal /proc/meminfo | tr -s ' ')"

# 1) USB aygit: usb manager -> aygit modu, configfs ACM (Windows'ta COM portu), kabuk
for r in /sys/devices/platform/soc*/usbc0/otg_role /sys/devices/platform/soc*/*usbc0*/otg_role; do
	[ -e "$r" ] && echo usb_device > "$r"
done
mkdir -p /sys/kernel/config
mount -t configfs none /sys/kernel/config 2>/dev/null
G=/sys/kernel/config/usb_gadget/f133
if [ -d /sys/kernel/config/usb_gadget ]; then
	mkdir -p $G/strings/0x409 $G/configs/c.1/strings/0x409 $G/functions/acm.GS0
	echo 0x1d6b > $G/idVendor; echo 0x0104 > $G/idProduct
	echo VeLinux > $G/strings/0x409/manufacturer; echo "VeLinux KÖLE" > $G/strings/0x409/product
	echo 0001 > $G/strings/0x409/serialnumber
	echo "ACM" > $G/configs/c.1/strings/0x409/configuration
	echo 250 > $G/configs/c.1/MaxPower
	ln -s $G/functions/acm.GS0 $G/configs/c.1/ 2>/dev/null
	echo "$(ls /sys/class/udc | head -1)" > $G/UDC && msg "USB ACM bagli"
fi
( while true; do [ -e /dev/ttyGS0 ] && setsid getty -n -l /bin/sh 115200 ttyGS0; sleep 1; done ) &

# Mount existing data only. A RAM proof must never format/erase flash data.
D=$(grep '"data"' /proc/mtd | cut -d: -f1)
if [ -n "$D" ]; then
    mount -t jffs2 /dev/mtdblock${D#mtd} /data && msg "/data mounted, no formatting" || msg "/data mount failed"
fi

# SD kart = kalici depolama (FAT32, okuma-yazma, /mnt/sd). Kart algilamasi surucu yuklendikten sonra
# ~1 sn surebildigi icin arka planda en fazla 5 sn beklenir. velinux/autorun.sh (ya da eski f133/autorun.sh) calistirilir.
( i=0; while [ $i -lt 10 ] && [ ! -b /dev/mmcblk0 ]; do sleep 0.5; i=$((i+1)); done
  for p in /dev/mmcblk0p1 /dev/mmcblk0; do
	[ -b $p ] && mount -o rw,noatime,flush,umask=000 -t vfat $p /mnt/sd 2>/dev/null && \
		{ msg "SD $p -> /mnt/sd ($(df -h /mnt/sd | awk 'NR==2{print $4}') bos)"; break; }
  done
  [ -b /dev/mmcblk0 ] && dd if=/dev/mmcblk0 bs=512 skip=16 count=1 2>/dev/null | grep -q eGON && \
	msg "UYARI: SD'de FEL anahtari (eGON @8 KB) var: guc verilince cihaz FEL'e duser"
  mountpoint -q /mnt/sd && { msg "RAM autorun (SD panel unchanged)"; /bin/autorun-g2d-zram-v1 &
  } ) &

# 3) Wi-Fi (kayitli ag varsa)
[ -x /bin/wifi-baslat ] && /bin/wifi-baslat &

# RAM proof: no MCU TX. Passive observer starts from RAM autorun.

# Donma bekcisi (11.10.2026, 09:56 kilitlenmesinden sonra): zamanlayici blogundaki watchdog 16 sn'ye kurulur,
# 4 sn'de bir beslenir. Cekirdek/kullanici alani donarsa sistem reset atar (SD FEL karti takiliysa FEL'e duser,
# DRAM korunur -> __log_buf xfel ile okunabilir). /data/wdt-kapali varsa calismaz.
if [ ! -e /data/wdt-kapali ]; then
	devmem 0x020500B4 32 0x16aa0001; devmem 0x020500B8 32 0x16aa00B1; devmem 0x020500B0 32 0x14af
	nice -n -10 sh -c 'while true; do devmem 0x020500B0 32 0x14af; sleep 4; done' &
	msg "watchdog 16 sn acik"
fi
/bin/zram-baslat-v1 &

# Saglik gunlugu (RAM): dakikada bir bellek/yuk; donma sonrasi FEL'den okunabilsin diye kmsg'e de yazilir
( while true; do sleep 60
	S="saglik: $(cut -d' ' -f1-3 /proc/loadavg) bos=$(awk '/MemAvailable/{print $2}' /proc/meminfo)kB"
	echo "$S" > /dev/kmsg; done ) &

# 4) Ekran: panel uygulamasi (yoksa ekranda kabuk)
if [ -x /usr/bin/panel ]; then
	echo 0 > /sys/class/graphics/fbcon/cursor_blink 2>/dev/null
	( while true; do panel < /dev/null > /tmp/panel.log 2>&1; R=$?
	  msg "panel cikti ($R), yeniden: $(tail -n 3 /tmp/panel.log | tr '\n' '|')"
	  mv /tmp/panel.log /tmp/panel.log.1; sleep 2; done ) &
else
	( while true; do setsid sh -c 'exec sh </dev/tty1 >/dev/tty1 2>&1'; sleep 1; done ) &
fi

while true; do sleep 3600; done
