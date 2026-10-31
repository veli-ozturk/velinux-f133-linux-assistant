#!/bin/sh
# yeniden_baslat_sd_v1 (23.10.2026, Claude; scripts/582): MCU kalp sorgusuna (B2) HIZALI yeniden baslatma.
#   Uyanik MCU, B2 ~9-10 sn cevapsiz kalinca karti sifirliyor (docs bolum 136); yeni acilista mcu-servis autorun_v42 ile
#   ~5. sn'de basliyor. reboot son B2 cevabindan hemen sonra yapilirsa cevapsiz bosluk en kisa olur. /tmp/bt-durum.json
#   last_mcu_heartbeat (B2 ~2.987 sn aralikla; dosya ~1 sn'de bir yazilir, gecikmesi faz hesabini bozmaz) + aralik ile bir
#   sonraki B2'nin 0.35 sn sonrasi beklenir. mcu-servis calismiyorsa / kalp yoksa / 10 sn'den eskiyse hemen reboot.
#   [--wifi-atla]: /data/.wifi-deneme OLUSTURULUR (deney: yeni acilista wifi-baslat Wi-Fi'yi atlar); yoksa silinir.
#   Test kancalari: YB_DURUM (json), YB_UPTIME (uptime dosyasi), YB_DATA (/data yerine), YB_KURU=1 (reboot yok, yazdirir).
J=${YB_DURUM:-/tmp/bt-durum.json}
U=${YB_UPTIME:-/proc/uptime}
DA=${YB_DATA:-/data}
if [ "$1" = --wifi-atla ]; then echo atla > $DA/.wifi-deneme; else rm -f $DA/.wifi-deneme; fi
sync
L=$(tr ',' '\n' < "$J" 2>/dev/null | sed -n 's/.*"last_mcu_heartbeat":\([0-9.]*\).*/\1/p')
R=$(tr ',' '\n' < "$J" 2>/dev/null | grep -c '"running":true')
read u x < "$U"
bekle=$(awk -v L="${L:-0}" -v u="$u" -v r="${R:-0}" 'BEGIN {
	if (r < 1 || L <= 0 || u - L > 10) { print "0"; exit }
	t = L; while (t < u + 0.2) t += 2.987
	printf "%.2f", t + 0.35 - u }')
S="[yeniden] son B2 ${L:-yok}, uptime $u -> $bekle sn sonra reboot -f"
if [ -n "$YB_KURU" ]; then echo "$S"; else echo "$S" > /dev/kmsg; fi
[ "$bekle" != 0 ] && sleep "$bekle"
if [ -n "$YB_KURU" ]; then echo "KURU: reboot -f"; else reboot -f; fi
