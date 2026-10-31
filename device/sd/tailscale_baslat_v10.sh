#!/bin/sh
# tailscale_baslat_v10 (29.10.2026, Claude; scripts/670): v9 + /data/tailscale-env (ek ortam, varsayilanlarin ardindan;
#   or. "GOMEMLIMIT=10MiB GOGC=30 GODEBUG=gctrace=1"). Yoksa v9 ile ayni.
# tailscale_baslat_v9 (26.10.2026, Claude; scripts/664, §183): v8 + kole-tlsvekil-v4 (bosta kapanan Merkez'e yeniden baglanir).
# tailscale_baslat_v8 (25.10.2026, Claude; scripts/662, §179): v7 + kole-tlsvekil-v3 (baglanti basina gunluk satiri).
# tailscale_baslat_v7 (25.10.2026, Claude; scripts/660): v6 + kole-tlsvekil-v2: serve 443 ve 8443 --tcp --proxy-protocol 1;
#   vekil PROXY satirindan istemci IP'sini, whois'ten kullaniciyi alir, taklit vekil basliklarini silip Merkez'in vekil
#   dinleyicisine (127.0.0.2:8780) X-Forwarded-For/-Proto + Tailscale-User-Login ile iletir. PROXY'siz baglanti reddedilir.
#   Geri donus: $D/tlsvekil-kapali + serve --https=443 http://127.0.0.2:8780 (docs §178).
# tailscale_baslat_v6 (25.10.2026, Claude; scripts/658, §174): v5 + BearSSL TLS vekili (kole-tlsvekil-v1, 127.0.0.1:8443 ->
#   Merkez 127.0.0.1:8780; tailscaled HTTPS 2.4-6.1 s yerine ~0.5-1.2 s). Sertifika tailscale cert ile /data/ts-cert (700,
#   anahtar 600), gunde bir yenilenir ve vekil yeniden baslar. Kapatma: $D/tlsvekil-kapali. Serve --tcp 8443 Tailscale
#   durumunda kalici. Anahtar icerigi hicbir yere yazilmaz.
# tailscale_baslat_v5 (23.10.2026, Claude; scripts/609, denetim v2 bulgu 8): v3 + gunluk 10 dk'da bir yerinde (": >")
#   256 KB'ta sifirlanir (tailscaled haftalarca acik kaliyor, v3 yalniz yeniden baslatmada kirpiyordu; mv olmaz: tailscaled
#   eski dosyayi acik tutar). v4 (TUN taslagi) kurulmadi; bu v3 tabanli.
# tailscale_baslat_v3 (22.10.2026, Claude; madde 102): v2 + Go bellegi sikilastirildi: GOGC 100 -> 50, GOMEMLIMIT 24 -> 16 MiB
#   (olcum: tailscaled RSS 4.3 MB + takas 8.7 MB; 24 MiB sinir hic devreye girmiyordu). Kilit/pid dosyalari v2 ile ayni.
# tailscale_baslat_v2 (14.10.2026, Claude): cihazda Tailscale gozeticisi (v1 yerine; v1 resmi 68 MB ikililerle cokuyordu).
#  - ikili: $T/tailscaled-kucuk (kendi derlememiz: scripts/152, watchdog panigi yamali, TS_BE_CLI=1 ile komut satiri)
#  - SSL_CERT_FILE=$T/ca-certificates.crt (cihazda CA deposu yok; Let's Encrypt/HTTPS serve icin sart)
#  - GOMEMLIMIT=24MiB GOGC=100 GOMAXPROCS=1; gunluk RAM'de /tmp/tailscaled.log (SD'ye yazmak cihazi kilitliyordu)
#  - oom_score_adj=-500 (bellek darliginda once baska surecler)
#  - giris ve `serve` ayarlari /data/tailscale durumunda kalici; yeniden giris/serve gerekmez
#  - cokerse artan beklemeyle yeniden baslatir. Bayrak $D/tailscale-acik yoksa hicbir sey yapmaz.
D=/mnt/sd/velinux
T=$D/tailscale
L=/tmp/tailscaled.log
[ -f "$D/tailscale-acik" ] || { echo "tailscale kapali ($D/tailscale-acik yok)"; exit 0; }
[ -x "$T/tailscaled-kucuk" ] || { echo "tailscaled-kucuk yok"; exit 1; }
mkdir /tmp/tailscale-gozetici.lock 2>/dev/null || { echo "zaten calisiyor"; exit 0; }
echo $$ > /tmp/tailscale-gozetici.pid
( while sleep 600; do [ -f "$L" ] && [ "$(wc -c < "$L")" -gt 262144 ] && : > "$L"; done ) </dev/null >/dev/null 2>&1 &	# v5
# v6: TLS vekili (tailscaled ayaga kalktiktan sonra; sertifika icin tailscaled gerekir)
VK=$D/kole-tlsvekil-v4
C=/data/ts-cert
AD=velinux.example.ts.net	# shortcut: sabit ad; cihaz adi degisirse burasi
if [ ! -f "$D/tlsvekil-kapali" ] && [ -x "$VK" ]; then
	(
		sleep 60
		while true; do
			mkdir -p $C && chmod 700 $C
			cd $C && TS_BE_CLI=1 "$T/tailscaled-kucuk" --socket=/tmp/tailscaled.sock cert --cert-file velinux.crt.yeni \
				--key-file velinux.key.yeni $AD >/dev/null 2>&1 && chmod 600 velinux.key.yeni && mv velinux.key.yeni velinux.key \
				&& mv velinux.crt.yeni velinux.crt
			rm -f $C/velinux.crt.yeni $C/velinux.key.yeni
			if [ -s $C/velinux.crt ] && [ -s $C/velinux.key ]; then
				"$VK" 8443 127.0.0.2 8780 $C/velinux.crt $C/velinux.key >>/tmp/tlsvekil.log 2>&1 &
				v=$!
				echo $v > /tmp/tlsvekil.pid
				echo -300 > /proc/$v/oom_score_adj 2>/dev/null
				i=0
				while kill -0 $v 2>/dev/null && [ $i -lt 86400 ]; do sleep 60; i=$((i + 60)); done	# gunde bir yenile
				kill $v 2>/dev/null
				wait $v 2>/dev/null
			fi
			[ "$(wc -c < /tmp/tlsvekil.log 2>/dev/null || echo 0)" -gt 65536 ] && : > /tmp/tlsvekil.log
			sleep 30
		done
	) </dev/null >/dev/null 2>&1 &
fi
# saat dogru olmadan TLS basarisiz olur (NTP)
i=0
while [ "$(date +%Y)" -lt 2026 ] && [ $i -lt 180 ]; do sleep 2; i=$((i + 2)); done
bekle=5
while true; do
	if [ -f "$L" ] && [ "$(wc -c < "$L")" -gt 262144 ]; then tail -c 65536 "$L" > "$L.k" && mv "$L.k" "$L"; fi
	TE=$(cat /data/tailscale-env 2>/dev/null)	# v10
	env SSL_CERT_FILE=$T/ca-certificates.crt GOMEMLIMIT=16MiB GOGC=50 GOMAXPROCS=1 $TE \
		"$T/tailscaled-kucuk" --tun=userspace-networking --statedir=/data/tailscale --socket=/tmp/tailscaled.sock \
		--port=41641 --no-logs-no-support </dev/null >>"$L" 2>&1 &
	p=$!
	echo "$p" > /tmp/tailscaled.pid
	sleep 1
	echo -500 > /proc/$p/oom_score_adj 2>/dev/null
	bas=$(cut -d. -f1 /proc/uptime)
	wait "$p"
	rc=$?
	sure=$(( $(cut -d. -f1 /proc/uptime) - bas ))
	echo "$(date '+%F %T') tailscaled cikti rc=$rc ($sure sn calisti); $bekle sn sonra yeniden" >> "$L"
	[ $sure -gt 600 ] && bekle=5
	sleep $bekle
	[ $bekle -lt 120 ] && bekle=$((bekle * 2))
done
