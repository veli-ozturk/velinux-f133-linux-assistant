#!/bin/sh
# bellek_ayar_v6 (25.10.2026, Claude; scripts/658, §176): v5 + vm.watermark_boost_factor 0 (Wi-Fi order-3 basarisizliklari
#   boost ile kswapd/dogrudan geri kazanimi tetikleyip tailscaled'i takasa itiyordu; A/B major fault 5848 -> 311 / 60 sn).
# bellek_ayar_v5 (24.10.2026, Claude; scripts/630, denetim-2026-10-24-v3 A2): v4 + wpa_supplicant ve udhcpc -900
#   (OOM onlari secerse Wi-Fi -uzaktan erisimin tek yolu- guc kesilene kadar gelmiyordu).
# bellek_ayar_v4 (23.10.2026, Claude; Sira 18 madde 133, scripts/571): v3 + iw ve wpa_cli SD'ye ($D/araclar), RAM'deki
#   ikili yerine 2 satirlik sarmalayici (panel 0.76 sinyali wpa_supplicant'tan alir; iw yalniz tarama/yedek). Test:
#   BA_KOK=<kok> BA_LOG=<dosya> sh bellek_ayar_v4.sh --tek-araclar (sahte kokte $K/usr/sbin, $K/araclar).
# bellek_ayar_v3 (23.10.2026, Claude; madde 125, scripts/557): v2 + PERIYODIK BELLEK SIKISTIRMA. Olcum: 10 sa'te
#   compact_stall 39476 / fail 39401 (dogrudan sikistirma neredeyse hep basarisiz), 2 OOM (order 2, 3). Elle bir
#   compact_memory 0.11 sn, >= 32 KB bos bloklardaki sayfa 2024 -> 2600. Acilista ve 5 dk'da bir; gunluge olcum satiri.
#   Kapatmak: $D/bellek-sikistirma-kapali. Test: BA_KOK=<kok> BA_LOG=<dosya> sh bellek_ayar_v3.sh --tek-sikistirma
# bellek_ayar_v2 (22.10.2026, Claude; madde 103): v1 - min_free_kbytes 2048 ve watermark_scale_factor 100 CIKARILDI.
#   #46 RAM provasi A/B (docs 103): bu ikisi acikken iki turda da sorun (yogun takas / VE 10 sn takilma, 13-16 kare/sn),
#   kapaliyken temiz (17-20 kare/sn). Order-3 sorununu RX havuzu (+K4 NORETRY) kaynaginda cozuyor. page-cluster 0 kalir.
# bellek_ayar_v1 (22.10.2026, Claude; OTONOM Sira 13 madde 102, kullanıcı: "ram tarafinda yapilabilecek iyilestirmeleri de bul").
# Olcum (docs 97, 102): PC Ekrani tam ekran harekette OOM killer panel / wifi-ekran / kole-gemini'yi olduruyor; tetikleyen
# ssv6xxx_tx_task order-3 (32 KB) ayirmasi, bellek parcali (sikistirma %99.9 basarisiz), zram takasi dolu.
# Bu betik YALNIZ calisan sistemi ayarlar (flash'a, /data'ya dokunmaz); acilista initramfs her sey eski haline gelir.
#  1) vm: page-cluster 0 (zram'da onden okuma gereksiz acma), min_free_kbytes 2048 + watermark_scale_factor 100
#     (kswapd daha erken calisir, yuksek dereceli bos blok kalir; canli degisim ~1 sn takilma yapar, acilista sorun degil)
#  2) Wi-Fi K1 RX havuzu (docs/k1-ssv-rx-havuz-v1.md): rx_kopya 1, rx_havuz 16 -> 16 KB tampon yeniden kullanilir
#     (13:10 olcumu: isabet 7382 / iska 51, 6.9 -> 8.6 kare/sn)
#  3) RAM'deki kok dosya sisteminden tek kullanimlik dosyalar: ssv6006.ko (modul yuklendikten sonra; wifi-baslat yalniz
#     modul yoksa insmod eder), usb-storage.ko (yuklu degilse), fbtest/evtest (once SD'ye $D/araclar/ kopyalanir) ~2.4 MB
#  4) OOM oncelikleri (20 sn'de bir, yeniden baslayan surecler icin): panel -900, wifi-ekran -800, kamera-kayit -300,
#     kole-asistan -100, kole-gemini +500 (kendi dongusu yeniden baslatir; ilk feda edilen o olsun)
# Kapatmak: $D/bellek-ayar-kapali dosyasi.
D=/mnt/sd/velinux
L=${BA_LOG:-/tmp/bellek-ayar.log}	# v3: test kancasi
K=${BA_KOK:-}	# v3: test kancasi (sahte /proc koku)
[ -f $D/bellek-ayar-kapali ] && { echo "bellek ayari kapali ($D/bellek-ayar-kapali)"; exit 0; }
case "$1" in --tek-*) ;; *) mkdir /tmp/bellek-ayar.lock 2>/dev/null || {	# v3/v4: test kilitsiz
	 echo "zaten calisiyor"; exit 0; } ;; esac
log() { echo "$(date '+%F %T') $*" >> $L; }
bos() { awk '/^MemAvailable/{print $2}' /proc/meminfo; }
# v3: >= order 3 (32 KB) bos bloklardaki sayfa (buddyinfo: alan 5 = order 0)
buyuk() { awk '{m = 8; for (i = 8; i <= NF; i++) {s += $i * m; m *= 2}} END {print s + 0}' $K/proc/buddyinfo; }	# ^ yok (busybox awk matematiksiz)
vmsay() { awk -v a="$1" '$1 == a {print $2; f = 1} END {if (!f) print 0}' $K/proc/vmstat; }
sikistir() {
	b0=$(buyuk); st=$(vmsay compact_stall); fa=$(vmsay compact_fail); oo=$(vmsay oom_kill)
	echo 1 > $K/proc/sys/vm/compact_memory
	log "sikistirma: >=32 KB bos $b0 -> $(buyuk) sayfa; son turdan beri compact_stall +$((st - ${son_st:-$st})) fail +$((fa - ${son_fa:-$fa})) oom_kill $oo"
	son_st=$st; son_fa=$fa
}
if [ "$1" = --tek-sikistirma ]; then	# v3: test (iki tur: ikincisi farki yazar)
	sikistir; sikistir; exit 0
fi
# v4: seyrek kullanilan Wi-Fi araclari SD'de; RAM'de yalniz sarmalayici. $1 kok (test), $2 SD arac dizini
araclar_sd() {
	for f in $1/usr/sbin/iw $1/usr/sbin/wpa_cli; do
		[ -f "$f" ] || continue
		[ "$(head -c 2 "$f")" = '#!' ] && continue	# zaten sarmalayici
		a="$2/${f##*/}"
		[ -f "$a" ] || cp "$f" "$a"
		cmp -s "$f" "$a" || { log "v4: $a farkli, $f birakildi"; continue; }
		printf '#!/bin/sh\nexec %s "$@"\n' "$a" > "$f.yeni" && chmod 755 "$f.yeni" && mv "$f.yeni" "$f"
	done
}
if [ "$1" = --tek-araclar ]; then	# v4: test (kilitsiz)
	araclar_sd "$K" "$K/araclar"; exit 0
fi

log "basladi; kullanilabilir $(bos) kB"
[ -w /proc/sys/vm/watermark_boost_factor ] && echo 0 > /proc/sys/vm/watermark_boost_factor	# v6 (§176)
echo 0 > /proc/sys/vm/page-cluster	# v2: min_free_kbytes / watermark_scale_factor cekirdek varsayilaninda
log "vm: page-cluster $(cat /proc/sys/vm/page-cluster), watermark_boost_factor $(cat /proc/sys/vm/watermark_boost_factor 2>/dev/null) (min_free_kbytes $(cat /proc/sys/vm/min_free_kbytes), watermark_scale_factor $(cat /proc/sys/vm/watermark_scale_factor) degistirilmedi)"

# Wi-Fi modulu yuklenene kadar bekle (en cok 3 dk)
i=0
while ! grep -q '^ssv6x5x ' /proc/modules && [ $i -lt 180 ]; do sleep 2; i=$((i + 2)); done
P=/sys/module/ssv6x5x/parameters
if [ -w $P/rx_kopya ] && [ -w $P/rx_havuz ]; then
	echo 1 > $P/rx_kopya && echo 16 > $P/rx_havuz
	log "K1: rx_kopya $(cat $P/rx_kopya), rx_havuz $(cat $P/rx_havuz)"
else
	log "K1 parametreleri yok (eski modul?), atlandi"
fi

once=$(bos)
grep -q '^ssv6x5x ' /proc/modules && rm -f /lib/wifi/ssv6006.ko
grep -q '^usb_storage ' /proc/modules || rm -f /lib/usb-storage.ko
mkdir -p $D/araclar
for f in /usr/bin/fbtest /usr/bin/evtest; do
	[ -f $f ] || continue
	[ -f $D/araclar/${f##*/} ] || cp $f $D/araclar/
	cmp -s $f $D/araclar/${f##*/} && rm -f $f
done
log "kok dosya sistemi: tek kullanimlik dosyalar silindi ($D/araclar'da fbtest/evtest); kullanilabilir $once -> $(bos) kB"
once=$(bos)
araclar_sd "" "$D/araclar"	# v4
log "v4: iw/wpa_cli SD'de ($D/araclar), RAM'de sarmalayici; kullanilabilir $once -> $(bos) kB"

[ -f $D/bellek-sikistirma-kapali ] || [ ! -w /proc/sys/vm/compact_memory ] || sikistir	# v3: ilk olcum
n=0
while true; do
	for p in /proc/[0-9]*; do
		{ read -r c < $p/comm; } 2>/dev/null || continue	# v2: kapanan surec uyarisi susturuldu
		case $c in
			panel) v=-900 ;;
			wpa_supplicant|udhcpc) v=-900 ;;	# v5
			wifi-ekran-*) v=-800 ;;
			kamera-kayit-*) v=-300 ;;
			kole-asistan-*) v=-100 ;;
			kole-gemini-*) v=500 ;;
			*) continue ;;
		esac
		{ read -r e < $p/oom_score_adj; } 2>/dev/null || continue
		[ "$e" = "$v" ] || echo $v > $p/oom_score_adj 2>/dev/null
	done
	n=$((n + 1))
	if [ $n -ge 15 ]; then	# v3: 5 dk'da bir (15 x 20 sn)
		n=0
		[ -f $D/bellek-sikistirma-kapali ] || [ ! -w /proc/sys/vm/compact_memory ] || sikistir
		[ "$(wc -c < $L)" -gt 65536 ] && { tail -c 32768 $L > $L.k && mv $L.k $L; }
	fi
	sleep 20
done
