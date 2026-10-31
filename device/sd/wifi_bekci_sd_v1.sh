#!/bin/sh
# wifi_bekci_sd_v1 (24.10.2026, Claude; scripts/630, denetim-2026-10-24-v3 A2): wpa_supplicant ya da udhcpc olurse (OOM,
#   kendi cikisi) Wi-Fi'yi geri getirir; v47'de kimse yeniden baslatmiyordu, cihaz guc kesilene kadar agsiz kaliyordu.
#   - Yalniz bu acilista Wi-Fi bir kez IP aldiysa calisir (guc korumasinin bilerek atladigi acilisa dokunmaz).
#   - /data/.wifi-deneme ya da .wifi-gec-deneme varken ve wifi-kaydet calisirken bekler.
#   - wpa_supplicant iki denetimde ust uste yoksa: wifi-baslat -f (wpa olu oldugu icin killall yarisi yok).
#     Yalniz udhcpc iki denetimde yoksa: yalniz udhcpc baslatilir.
#   Test kancalari: WB_ARALIK (60), WB_C (wpa.conf), WB_M (/data), WB_PIDOF, WB_UDHCPC.
A=${WB_ARALIK:-60}
C=${WB_C:-/data/wifi/wpa.conf}
M=${WB_M:-/data}
P=${WB_PIDOF:-pidof}
U=${WB_UDHCPC:-udhcpc}
K() { echo "[wifi-bekci] $*" > /dev/kmsg 2>/dev/null || echo "[wifi-bekci] $*"; }
ip_var() { wpa_cli -i wlan0 status 2>/dev/null | grep -q '^ip_address='; }
gordu=0
yw=0
yu=0
while :; do
	sleep "$A"
	[ -f "$C" ] || continue
	if [ -e "$M/.wifi-deneme" ] || [ -e "$M/.wifi-gec-deneme" ] || $P wifi-kaydet >/dev/null; then
		yw=0
		yu=0
		continue
	fi
	if [ $gordu = 0 ]; then
		ip_var && gordu=1
		continue
	fi
	if $P wpa_supplicant >/dev/null; then yw=0; else yw=$((yw + 1)); fi
	if $P udhcpc >/dev/null; then yu=0; else yu=$((yu + 1)); fi
	if [ $yw -ge 2 ]; then
		K "wpa_supplicant yok, Wi-Fi yeniden baslatiliyor"
		killall udhcpc 2>/dev/null
		wifi-baslat -f
		yw=0
		yu=0
	elif [ $yu -ge 2 ]; then
		K "udhcpc yok, yeniden baslatiliyor"
		$U -b -i wlan0 -s /usr/share/udhcpc/default.script -p /var/run/udhcpc.wlan0.pid >/dev/null 2>&1
		yu=0
	fi
done
