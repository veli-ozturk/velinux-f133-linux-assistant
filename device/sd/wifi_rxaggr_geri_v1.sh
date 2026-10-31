#!/bin/sh
# wifi_rxaggr_geri_v1 (15.10.2026, Claude): ssv6x5x surucusunu VARSAYILAN yapilandirmayla (RX toplama acik) yeniden yukler.
# wifi_rxaggr_kapat_v1 denemesi kotulestirdi (paket basina ayirma: sikistirma beklemesi 1.3/sn -> 7/sn, gidis-donus 400-700 ms).
# Neden: toplama acikken surucu her SDIO alimi icin ~10 KB (kmalloc-16k, order-2/3 levha) GFP_KERNEL tampon ayiriyor;
# bellek parcali oldugundan sikistirma/geri kazanma beklemesi -> ikinci ekranda 300-500 ms takilma (paket kaybi yok,
# cihaz isleme temiz, PC gidis-donus gunlugu). Kapali: MAX_FRAME_SIZE_DMG 4 KB (+rsvd, kmalloc-8k, order-1).
# /lib/firmware'e dokunmaz: yapilandirma kopyasi /tmp'de, insmod tu_stacfgpath ile. Kalici degil (yeniden acilista eski).
# Kullanim (arka planda, Wi-Fi ~20 sn duser):  sh wifi_rxaggr_kapat_v1.sh </dev/null >/dev/null 2>&1 &
L=/tmp/wifi-rxaggr-geri.log
exec >>$L 2>&1
T() { echo "$(date +%T) $*"; }
S() { grep -E "^(compact_stall|allocstall_normal) " /proc/vmstat | tr '\n' ' '; }	# slabinfo yok
T "basla; once: $(S)"
killall udhcpc wpa_supplicant 2>/dev/null
ip link set wlan0 down
sleep 1
rmmod ssv6x5x; T "rmmod rc=$?"
sleep 1
insmod /lib/wifi/ssv6006.ko; T "insmod rc=$?"
for i in 1 2 3 4 5 6 7 8 9 10; do [ -e /sys/class/net/wlan0 ] && break; sleep 1; done
ip link set wlan0 up || { T "wlan0 yok"; exit 1; }
mkdir -p /var/run
wpa_supplicant -B -D nl80211 -i wlan0 -c /data/wifi/wpa.conf
udhcpc -b -i wlan0 -s /usr/share/udhcpc/default.script -p /var/run/udhcpc.wlan0.pid >/dev/null 2>&1
for i in $(seq 1 30); do ip -4 addr show wlan0 | grep -q inet && break; sleep 1; done
T "adres: $(ip -4 addr show wlan0 | grep -o 'inet [0-9.]*')"
sleep 30
T "30 sn sonra: $(S)"
