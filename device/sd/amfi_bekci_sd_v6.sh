#!/bin/sh
# v6 (24.10.2026 Claude, scripts/628, denetim-2026-10-24-v3 A10/A19): amfi-bekci-v4 (H4; bad_xor tabani, mcu-servis-v7)
#   varsa o; yoksa ya da SHA'si tutmazsa amfi-bekci-v3 (H3). Gunluk arka planda 30 sn'de bir YERINDE kirpilir (amfi
#   >> ile yaziyor; v5 yalniz yeniden baslatmada kirpiyordu): 64 KB'i asarsa son 16 KB .previous'a, gunluk sifirlanir
#   (mcu_servis_sd_v5 gibi). Kirpma dongusu bu betik olunce 30 sn icinde biter (sinyal davranisi v5 ile ayni).
# v5 (23.10.2026 Claude, scripts/609, denetim v2 R1): v4 + SHA her baslatmadan once (v4 yalniz basta denetliyordu:
#   SD'de degisen ikili denetimsiz yeniden baslardi; amfi-bekci MCU'ya kalici ayar yazabilir). Tutmazsa calistirilmaz,
#   5 sn sonra yeniden denetlenir (gecici SD okuma hatasi sesi kalici kapatmasin; inceleme 24.10).
# v4 (23.10.2026 Claude, scripts/595, denetim-2026-10-23 bulgu 4): v3 + amfi-bekci-v3 cikarsa yeniden baslatilir (mcu-servis
#   yeniden baslayinca saglik denetimi 6 ile cikiyordu; v3'te kimse baslatmiyordu, ses yeniden baslatmaya kadar kapali kaliyordu).
#   60 sn'den kisa yasadiysa 30 sn, yoksa 5 sn sonra (MCU'ya cerceve firtinasi olmasin). Gunluk >>, 64 KB'ta son 16 KB kalir.
#   FEL/flash icin uyutma: bu betik de .fel-bekle adina cevrilmeli (v3 ve v2 gibi).
# v3 (22.10.2026 Claude, madde 116): amfi-bekci-v3 (BT muzigi -> A7 0b 00 kaynak, BT IS -> AT#MM ad). Gerisi v2 ile ayni.
# v2: AUX is initialized while muted, before the first short sound.
D=/mnt/sd/velinux
H4=46cbca709bf3cb6d86bd5ff44cccdf82a0b7e598bdf08a6bf4a23c2417d05dca
H3=cff0518ed76c0329d3e85aa3293de82a83279d47c8d370d03ce791c5ef458373
L=/tmp/amfi-bekci-v1.log
( while kill -0 $$ 2>/dev/null; do	# v6: $$ = bu betik (alt kabukta da)
	if [ -f $L ] && [ "$(wc -c < $L)" -gt 65536 ]; then tail -c 16384 $L > $L.previous.new && mv $L.previous.new $L.previous && : > $L; fi
	sleep 30
  done ) </dev/null >/dev/null 2>&1 &
dene() {	# v6: dene <surum> <sha>: calistirilabilir ve SHA tutuyorsa B; degilse N'ye eklenir
	B=$D/amfi-bekci-$1
	if ! test -x "$B"; then N="$N $1 yok;"; return 1; fi
	test "$(sha256sum "$B" | cut -d' ' -f1)" = "$2" && return 0
	N="$N $1 sha uyusmuyor;"
	return 1
}
while :; do
	N=
	if ! { dene v4 "$H4" || dene v3 "$H3"; }; then	# calistirilmaz; gecici SD hatasi kalici olmasin
		[ "$u" = "$N" ] || echo "$(date '+%T') amfi_bekci_sd_v6:$N calistirilmadi; 5 sn'de bir denetlenir" >> $L
		u=$N
		sleep 5
		continue
	fi
	u=
	t0=$(cut -d. -f1 /proc/uptime)
	"$B" --a3-kalici-dogrulandi </dev/null >>$L 2>&1 &
	echo $! > /tmp/amfi-bekci-v1.pid
	wait $!
	rc=$?
	s=$(( $(cut -d. -f1 /proc/uptime) - t0 ))
	b=5; [ $s -lt 60 ] && b=30
	echo "$(date '+%T') ${B##*/} cikti (rc $rc, $s sn), $b sn sonra yeniden" >> $L
	sleep $b
done
