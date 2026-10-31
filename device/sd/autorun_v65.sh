#!/bin/sh
# v65 (31.10.2026 Claude, scripts/681): v64 + kole-gemini-v51 (cevaplar kalici menuyle).
# v64 (31.10.2026 Claude, scripts/680): v63 + kole-gemini-v50 (Gezi dugmesi en ustte).
# v63 (31.10.2026 Claude, scripts/679): v62 + kole-gemini-v49 (/jp hatirlatma durumlari, 15 dk onbellek).
# v62 (30.10.2026 Claude, scripts/678): v61 + kole-gemini-v48 (Gezi kisayolu + sayfa baglantisi).
# v61 (30.10.2026 Claude, scripts/677): v60 + kole-gemini-v47 (/jp cevap etiketi).
# v60 (30.10.2026 Claude, scripts/676): v59 + kole-gemini-v46 (Telegram /jp Gezi sorusu).
# v59 (30.10.2026 Claude, scripts/674-675): v58 + kole-telgraf-v7 ve kole-gemini-v45 (Telegram izinli sohbetler: /data/asistan/telegram-izinli).
# v58 (29.10.2026 Claude, scripts/673): v57 + kole-gozetici-v5 ve kole-telgraf-v6 (yalniz gunluk etiketleri).
# v57 (29.10.2026 Claude, scripts/670): v56 + tailscale_baslat_v10 (/data/tailscale-env).
# v56 (29.10.2026 Claude, scripts/669): v55 + kole-telgraf-v5 (/foto C'de, madde 135 A2).
# v55 (28.10.2026 Claude, §188): v54 + kole-gemini-v44 (TTS sessizlik kirpma) secim listesinde.
# v54 (26.10.2026 Claude, scripts/666, §186): v53 + kole-gozetici-v4 (kosul "!") ve pano ikinci ekran acikken calismaz.
# v53 (26.10.2026 Claude, scripts/664): v52 + tailscale_baslat_v9 (tlsvekil v4, bosta kapanan Merkez'e yeniden baglanir).
# v52 (25.10.2026 Claude, scripts/662): v51 + tailscale_baslat_v8 (tlsvekil v3, gunluklu; 443 ve 8443 vekilde).
# v51 (25.10.2026 Claude, scripts/660): v50 + tailscale_baslat_v7 (tlsvekil v2: 443/8443 PROXY v1, guvenli istemci bilgisi).
# v50 (25.10.2026 Claude, scripts/658): v49 + tailscale_baslat_v6 (BearSSL TLS vekili 8443) ve bellek_ayar_v6 (watermark_boost 0).
# v49 (24.10.2026 Claude, scripts/639, OTONOM 149): v48 + kole-gemini v43 secilir (soru soketi bayat ses, 141 kucukleri).
# v48 (24.10.2026 Claude, scripts/630, denetim-2026-10-24-v3): v47 + gozetici v3, bellek_ayar v5, mcu_servis_sd v6,
#   amfi_bekci_sd v6, kamera_kopru_sd v12 secilir; wifi_bekci_sd_v1 baslatilir; NTP tekrar denemesi sinirsiz.
# v47 (24.10.2026 Claude, scripts/618): v46 + kamera_kopru_sd_v11 (kamera-kayit-v11: hareket varken 8 kare/sn kayit).
# v46 (23.10.2026 Claude, scripts/609, denetim-2026-10-23-v2): v45 + gozetici v2, wdt v2, telgraf v4, mcu/amfi v5,
#   tailscale_baslat v5, kamera_kopru_sd v10, kole-gemini v42 secilir; merkez/wifi-ekran gunluk siniri 256 KB; wifi-kaydet SD surumu
#   (yedekli, dogrulamali) RAM'e; NTP saat 2026 olana kadar 5 dk'da bir yeniden.
# v45 (23.10.2026 Claude, scripts/595, denetim-2026-10-23 bulgulari 3/4/10/18): v44 + kole-telgraf v3 > v2 > v1, kole-gemini
#   aramalarina 41, amfi_bekci_sd_v4 (cikarsa yeniden) ve mcu_servis_sd_v4 (SHA uyusmazligi gunlukte) tercih edilir, gec Wi-Fi
#   denemesinde IP gelmese de bayrak silinir.
# v44 (23.10.2026 Claude, scripts/592): v43 + en yuksek kole-telgraf-vN (v2: zamanli Go yalniz gerekince).
# v43 (23.10.2026 Claude, scripts/589): v42 + Telegram C kole-telgraf-v1 + Go v41 tek atis (surekli Go --telegram yok;
#   $D/telgraf-kapali ile eski Go satiri).
# v42 (23.10.2026 Claude, scripts/582): v41 + MCU KALP CEVABI EN BASTA. Acilistaki "Wi-Fi guc cokmesi" aslinda uyanik MCU'nun
#   kalp sifirlamasiydi (yeniden baslatmadan 9.4-10.2 sn sonra; Wi-Fi atlansa da). /usr/bin/mcu -> FIFO sarmalayicisi,
#   calisan gercek `mcu ad` durdurulur (ad FIFO'dan), mcu_servis_sd_v3 --erken. Yoksa v41 gibi (bolum 5).
# v41 (23.10.2026 Claude, Sira 18 madde 136 ek): v40 + acilista guc korumasiyla atlanan Wi-Fi icin 60 sn sonra bir
#   gec deneme (/data/.wifi-gec-deneme: gec deneme de cokmusse o acilista denenmez, bayrak silinir; dongu yok).
# v40 (23.10.2026 Claude, Sira 18 madde 136): v38 + kole-gozetici-v1 varsa (ve $D/gozetici-kapali yoksa) 5 kabuk dongusu
#   (wifi-ekran, Merkez, Hey Kole, Pano, Telegram) yerine /tmp/kole-gozetici.conf + tek C surec. v39 kurulmadi (TUN taslagi).
# v38 (23.10.2026 Claude, Sira 18 madde 134): v37 + watchdog beslemesi kole-wdt-v1'e (init'in sh dongusu 4 sn'de bir
#   surec aciyordu). /data/kole-wdt-deneme kalmissa (onceki devir 5 dk dayanmadi) devir yok + bayrak silinir.
# v37 (23.10.2026 Claude, Sira 18 madde 133): v36 + panel SD surumu RAM'e kopyalanmaz: /usr/bin/panel -> $D/panel
#   baglantisi (1.7 MB RAM); bellek_ayar_v4 (iw/wpa_cli SD'de) varsa onu. Yeni panel kurulunca yalniz killall panel.
# v36 (23.10.2026 Claude, madde 125): v35 + bellek_ayar_v3 varsa onu (5 dk'da bir bellek sikistirma + olcum), yoksa v2, v1.
# v35 (22.10.2026 Claude, madde 116): v34 + amfi bekcisi v3 varsa (BT muzigi -> BtMusic kaynagi, BT adi her IS'te), yoksa v2.
# v34 (22.10.2026 Claude, madde 103): v33 + bellek_ayar_v2 varsa onu (min_free/watermark degisikligi yok), yoksa v1.
# v33 (22.10.2026 Claude, madde 102): v32 + bellek_ayar_v1.sh (vm ayarlari, K1 RX havuzu, RAM'deki tek kullanimlik
#   dosyalar, OOM oncelikleri) + Tailscale gozeticisi v3 varsa (GOGC 50, GOMEMLIMIT 16 MiB), yoksa v2.
# v32 (19.10.2026 Codex): v30 + idle amplifier RAM mute; no heartbeat restart.
# v30 (19.10.2026 Claude, scripts/298): v29 + MCU kalp hizmeti mcu-servis-v6 (mcu_servis_sd_v2.sh; A3+A4 korumasi),
#   yoksa eskisi (v1 + mcu-servis-v3).
# v29 (19.10.2026 Claude): v28 + Pano verisi ve Telegram en yuksek kole-gemini-vN (2..19; v10 kategorili haber).
# v28 (15.10.2026 Claude): v27 + PC ikinci ekran alicisi en yeni surum (wifi-ekran-v19: H.264 VE donanim; yoksa v17).
# v27 (15.10.2026 Claude): v26 + cihaz merkezi en yeni surum (v4: kablosuz guncelleme /api/ota).
# v26 (15.10.2026 Claude): v25 + kamera_kopru_sd_v9 (en yeni kamera-kayit, v7: foto aciklamasi).
# v25 (15.10.2026 Claude): v24 + kamera_kopru_sd_v8 (kamera-kayit-v6: kamera saati esitleme + USB kurtarma).
# v24 (15.10.2026 Claude): v23 + kamera_kopru_sd_v7 (kamera-kayit-v3) + iki yonlu Telegram (15 sn, --telegram).
# v23 (15.10.2026 Claude): v22 + Telegram bildirim kuyrugu (hatirlatici/alarm, kamera hareketi).
# v22 (15.10.2026 Claude): v21 + kole-asistan-v8; Pano verisi en yuksek $D/asistan/kole-gemini-vN.
# v21 (15.10.2026 Claude): v20 + kole-asistan-v7 + PC'siz Pano verisi (hava/doviz, Ankara; 10 dk) $D/asistan/kole-gemini-v4 --veri.
# v20 (15.10.2026 Claude): v19, Hey Kole kole-asistan-v6 (Gemini soru-cevap: $D/asistan/kole-gemini-v2,
#   anahtar /data/asistan/gemini-anahtar); yoksa v5, v4.
# v19 (15.10.2026 Claude): v18, Hey Kole kole-asistan-v5 (karakter uslubu; yoksa v4).
# v18 (15.10.2026 Claude): v17 + HEY KOLE: kole-asistan-v4 dongusu (uyandirma; /data/asistan/kapali ile kapanir),
#   kamera_kopru_sd_v6 (kole-kopru-v9) ve cihaz merkezi v3: ikisi de mikrofonu acmadan once Hey Kole'den birakmasini ister.
# v17 (14.10.2026 Claude): v16 + G2D renk tablosu "sikistir" profili #41 cekirdekte (0x406133B0; v12-v16'da
#   bolum yoktu -> PC ikinci ekranda koyu sahnede karo ton farki / "kare kare golgelenme").
# v16 (14.10.2026 Claude): v15 + PC ikinci ekran alicisi wifi-ekran-v17 (panel 0.27 iki parmak kayitlari 4..9);
#   yoksa v16, o da yoksa v10.
# v15 (14.10.2026 Claude): v14, ama kamera kaydi her acilista zorla ACILMAZ (kullanıcı: "surekli kayit almasin, ben
#   istedigimde baslasin, oyle devam etsin"). Bayrak SD'de oldugu icin son secim yeniden acilista da gecerli.
# v14 (14.10.2026 Claude): v13 + SD onden okuma 128 KB -> 16 KB (read_ahead_kb). Olcum: swappiness 100 de yetmedi;
#   tailscaled (SD'de 17 MB) kodu her sayfa hatasinda 128 KB okuyup kucuk onbellegi kendisi bosaltiyordu: 20 sn'de 330 MB
#   SD okuma, CPU bosta %0, HTTPS 21 sn zaman asimi. 16 KB ile: 20 sn'de 1 MB, bosta %80, HTTPS 0.4 sn. tailscaled'den ONCE
#   ayarlanmali (dosya acilirken okunur).
# v13 (14.10.2026 Claude): v12 + swappiness 100. Olcum: telefondan canli izlerken swappiness 30 ile cekirdek
#   SD'deki 17 MB tailscaled kodunu atip yeniden okuyordu (30 sn'de 513 MB SD okuma, CPU bosta %0, "cihaz coktu" hissi);
#   100 ile anonim sayfalar ucuz zram'e gider: 30 sn'de 67 MB okuma, bosta ~%68, HTTPS 2-12 sn -> ~3 sn.
# v12 (14.10.2026 Claude): PC'siz uzaktan erisim + surekli kamera kaydi (kullanıcı karari: kamera + Tailscale hep acik,
#   PC ikinci ekrani yalniz istenince). v10'a gore:
#   - lo arayuzu acilir (cihaz merkezi 127.0.0.1:8780; acilista lo KAPALIYDI)
#   - zram0 + zram1 (2 x 16 MiB lzo takas, swappiness 30)
#   - wifi-ekran (PC ikinci ekran alicisi, ~16 MB + CMA'dan ~7.7 MB) OTOMATIK BASLAMAZ: /tmp/ikinci-ekran-acik bayragi
#     varken calisir (panel/elle: touch /tmp/ikinci-ekran-acik ; kapatmak: rm). G2D renk yamasi yalniz #40'ta.
#   - MIC3 yolu (codec "ADC3 Input MIC3 Boost Switch"=1) $D/ses-v2 ile
#   - kamera: kamera_kopru_sd_v5 (kamera-kayit-v2 + kole-kopru-v8; RTSP yalniz $D/rtsp-acik varsa)
#   - KAYIT HER ZAMAN ACIK: /mnt/sd/kamera/kayit-acik (2 kare/sn, 1 dk parca, 6 saat dongu). Kapatmak icin
#     $D/kayit-kalici-kapali dosyasi olusturulur (Merkez'den kapatma bir sonraki acilisa kadar gecerlidir).
#   - cihaz merkezi v2 (127.0.0.1:8780, yeniden baslatma dongusu, oom -500)
#   - Tailscale: tailscale_baslat_v2.sh (kucuk derleme, CA paketi, RAM gunlugu, oom -500)
# Geri donus: autorun_v10.sh -> autorun.sh (yedek: autorun-v10-before-v12-<ts>.sh). FLASH'A YAZMAZ. Satir sonlari LF.
D=/mnt/sd/velinux
exec >>/tmp/autorun.log 2>&1
echo "autorun v32 basladi: $(cat /proc/uptime)"

# v42: MCU kalp cevabi EN ONCE (yeniden baslatmada uyanik MCU ~9-10 sn cevapsiz B2 sonrasi karti sifirliyor)
MS=
MSB=$D/mcu_servis_sd_v6.sh; [ -x $MSB ] || MSB=$D/mcu_servis_sd_v5.sh; [ -x $MSB ] || MSB=$D/mcu_servis_sd_v4.sh; [ -x $MSB ] || MSB=$D/mcu_servis_sd_v3.sh	# v46
if [ -x "$MSB" ] && [ -x "$D/mcu-servis-v6" ]; then
	if [ "$(readlink /usr/bin/mcu)" = vl ]; then
		cat > /usr/bin/mcu.sarma <<'SARMA_SON'
#!/bin/sh
# autorun_v42: MCU UART'i mcu-servis'te. 'mcu ad' ikinci okuyucu olarak B2 kalp baytlarini bolup MCU'ya karti
# sifirlatabilirdi -> ad mcu-servis FIFO'suna "D4 AT#MM<ad>\r\n" satiri olarak verilir. Diger alt komutlar: vl mcu.
if [ "$1" = ad ] && [ -n "$2" ]; then
	i=0
	while [ $i -lt 30 ] && ! { [ -p ${MCU_FIFO:-/tmp/mcu-komut} ] && pidof mcu-servis-v7 mcu-servis-v6 >/dev/null; }; do sleep 0.1; i=$((i + 1)); done
	if [ $i -ge 30 ]; then echo "SONUC: mcu-servis FIFO'su yok, ad gonderilmedi"; exit 3; fi
	h=$(printf '%s' "$2" | od -An -tx1)
	( echo "D4 41 54 23 4d 4d" $h "0d 0a" > ${MCU_FIFO:-/tmp/mcu-komut} ) &
	p=$!
	i=0
	while [ $i -lt 20 ] && kill -0 $p 2>/dev/null; do sleep 0.1; i=$((i + 1)); done
	if kill $p 2>/dev/null; then echo "SONUC: FIFO yazilamadi"; exit 3; fi
	echo "SONUC: BT adi '$2' mcu-servis FIFO'suna verildi"
	exit 0
fi
exec /usr/bin/vl mcu "$@"
SARMA_SON
		chmod 755 /usr/bin/mcu.sarma && mv /usr/bin/mcu.sarma /usr/bin/mcu && echo "mcu -> FIFO sarmalayicisi"
	fi
	ADGONDER=
	for p in $(pidof mcu); do	# init'in gercek `mcu ad`i (vl) UART'i tutuyorsa durdur, ad FIFO'dan gider
		[ "$(readlink /proc/$p/exe)" = /usr/bin/vl ] && kill $p 2>/dev/null && ADGONDER=1 && echo "gercek mcu ad ($p) durduruldu"
	done
	"$MSB" --erken </dev/null >/tmp/mcu-sd-launch.log 2>&1 &
	MS=v3
	echo "mcu-servis-v6 ERKEN istendi: $(cut -d' ' -f1 /proc/uptime)"
	if [ -n "$ADGONDER" ]; then
		BTAD="VeLinux KÖLE"; [ -s /data/bt-ad ] && BTAD=$(head -n 1 /data/bt-ad)
		( /usr/bin/mcu ad "$BTAD" >> /tmp/mcu.log 2>&1 ) </dev/null &
	fi
fi

# 0) G2D renk tablosu sikistir profili (yalniz cekirdek '#41 PREEMPT Tue Oct 13 23:50:40', tablo orijinal ise)
if [ -x $D/kmem32 ] && grep -q '#41 PREEMPT Tue Oct 13 23:50:40' /proc/version; then
	T=$($D/kmem32 oku 0x406133b0 12 | awk '{print $2}' | tr '\n' ' ')
	if [ "$T" = "0x00000400 0x00000000 0x0000059c 0xfffcf200 0x00000400 0xfffffea0 0xfffffd25 0x0001dd80 0x00000400 0x00000717 0x00000000 0xfffc3480 " ]; then
		# Y 1093, Cr->R 1532, Cb->G -376, Cr->G -780, Cb->B 1936; ofsetler -213584 / 130480 / -265296 (1/1024)
		ok=1
		$D/kmem32 yaz 0x406133b0 0x00000400 0x00000445 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133b8 0x0000059c 0x000005fc >/dev/null || ok=0
		$D/kmem32 yaz 0x406133bc 0xfffcf200 0xfffcbdb0 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133c0 0x00000400 0x00000445 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133c4 0xfffffea0 0xfffffe88 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133c8 0xfffffd25 0xfffffcf4 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133cc 0x0001dd80 0x0001fdb0 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133d0 0x00000400 0x00000445 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133d4 0x00000717 0x00000790 >/dev/null || ok=0
		$D/kmem32 yaz 0x406133dc 0xfffc3480 0xfffbf3b0 >/dev/null || ok=0
		[ $ok = 1 ] && echo "G2D renk tablosu: sikistir profili yazildi" || echo "G2D renk tablosu: YAZMA HATASI"
	else
		echo "G2D renk tablosu beklenenden farkli, DOKUNULMADI: $T"
	fi
fi

# 0) SD onden okuma 16 KB (sayfa hatasi basina 128 KB SD okumasi thrashing yapiyordu)
[ -w /sys/block/mmcblk0/queue/read_ahead_kb ] && echo 16 > /sys/block/mmcblk0/queue/read_ahead_kb && echo "mmcblk0 read_ahead_kb=16"

# 1) ag: loopback
ip link set lo up && echo "lo acildi: $(ip -4 addr show lo | grep -o 'inet [0-9./]*')"

# 2) zram x2
zram_ac() {
	z=$1
	[ -d /sys/block/$z ] || return 1
	grep -q "^/dev/$z " /proc/swaps && return 0
	[ "$(cat /sys/block/$z/disksize)" = 0 ] || return 1
	echo lzo > /sys/block/$z/comp_algorithm 2>/dev/null
	echo 16777216 > /sys/block/$z/disksize && mkswap /dev/$z >/dev/null && swapon -p 100 /dev/$z
}
zram_ac zram0 && echo "zram0 16 MiB takas"
if [ -e /sys/class/zram-control/hot_add ]; then
	n=$(cat /sys/class/zram-control/hot_add)
	zram_ac zram$n && echo "zram$n 16 MiB takas"
fi
echo 100 > /proc/sys/vm/swappiness

# 3) panel SD surumu (v10 ile ayni)
if [ -f $D/panel ] && [ "$(readlink /usr/bin/panel)" != "$D/panel" ]; then	# v37: kopya yerine baglanti
	rm -f /usr/bin/panel.yeni; ln -s $D/panel /usr/bin/panel.yeni && mv /usr/bin/panel.yeni /usr/bin/panel && \
		{ killall panel 2>/dev/null; echo "panel SD surumune baglandi (RAM'de kopya yok)"; }
fi

# v40: kabuk donguleri yerine C gozetici (varsa)
GZ=
[ -x $D/kole-gozetici-v1 ] && [ ! -f $D/gozetici-kapali ] && GZ=$D/kole-gozetici-v1
[ -x $D/kole-gozetici-v2 ] && [ ! -f $D/gozetici-kapali ] && GZ=$D/kole-gozetici-v2	# v46: gunlukler yerinde kirpilir
[ -x $D/kole-gozetici-v3 ] && [ ! -f $D/gozetici-kapali ] && GZ=$D/kole-gozetici-v3	# v48: cocuk OOM ayari
[ -x $D/kole-gozetici-v4 ] && [ ! -f $D/gozetici-kapali ] && GZ=$D/kole-gozetici-v4	# v54: kosul ! (pano)
[ -x $D/kole-gozetici-v5 ] && [ ! -f $D/gozetici-kapali ] && GZ=$D/kole-gozetici-v5	# v58: etiket
: > /tmp/kole-gozetici.conf
# 4) PC ikinci ekrani: yalniz bayrak varken
A=wifi-ekran-v16
for v in 29 28 27 26 25 24 23 22 21 20 19 18 17; do
	if [ -x $D/wifi-ekran-v$v ]; then A=wifi-ekran-v$v; break; fi
done
[ -x $D/$A ] || A=wifi-ekran-v10
if [ -x $D/$A ] && [ -n "$GZ" ]; then	# v40
	echo "wifiekran|2|x|/tmp/wifi-ekran.log|256|0||/tmp/ikinci-ekran-acik|-|$D/$A --port 8766 --seconds 3600" >> /tmp/kole-gozetici.conf
	echo "$A istege bagli (gozetici; bayrak: /tmp/ikinci-ekran-acik)"
elif [ -x $D/$A ]; then
	( p=
	  while true; do
		if [ -f /tmp/ikinci-ekran-acik ]; then
			if [ -z "$p" ] || ! kill -0 "$p" 2>/dev/null; then
				$D/$A --port 8766 --seconds 3600 >>/tmp/wifi-ekran.log 2>&1 &
				p=$!
				echo "$(date '+%T') $A basladi ($p)" >>/tmp/wifi-ekran.log
			fi
		elif [ -n "$p" ]; then
			kill "$p" 2>/dev/null; p=
			echo "$(date '+%T') $A durduruldu (bayrak yok)" >>/tmp/wifi-ekran.log
		fi
		sleep 2
	  done ) &
	echo $! > /tmp/wifi-ekran-dongu.pid
	echo "$A istege bagli gozetici basladi (bayrak: /tmp/ikinci-ekran-acik)"
fi

# 5) MCU kalp hizmeti (v10 ile ayni)
if [ -n "$MS" ]; then	# v42
	echo "mcu-servis erken baslatildi ($MS)"
elif [ -x "$D/mcu_servis_sd_v2.sh" ] && [ -x "$D/mcu-servis-v6" ]; then	# v30
	"$D/mcu_servis_sd_v2.sh" </dev/null >/tmp/mcu-sd-launch.log 2>&1 &
	echo "mcu-servis-v6 SD supervisor requested (handshake=0, A3+A4 korumasi)"
elif [ -x "$D/mcu_servis_sd_v1.sh" ] && [ -x "$D/mcu-servis-v3" ]; then
	"$D/mcu_servis_sd_v1.sh" </dev/null >/tmp/mcu-sd-launch.log 2>&1 &
	echo "mcu-servis SD supervisor requested (handshake=0)"
fi

# v32: amp gate only via existing MCU FIFO, confirmed persistent A3=1.
AMB=$D/amfi_bekci_sd_v6.sh; [ -x $AMB ] || AMB=$D/amfi_bekci_sd_v5.sh; [ -x $AMB ] || AMB=$D/amfi_bekci_sd_v4.sh; [ -x $AMB ] || AMB=$D/amfi_bekci_sd_v3.sh	# v46
if [ -x "$D/amfi-bekci-v3" ] && [ -x "$AMB" ]; then	# v35
    setsid sh "$AMB" </dev/null >/tmp/amfi-launch.log 2>&1 &
    echo "amfi gate v3 requested (idle mute, BT muzigi kaynagi, BT adi; A3 unchanged)"
elif [ -x "$D/amfi-bekci-v2" ] && [ -x "$D/amfi_bekci_sd_v2.sh" ]; then
    setsid sh "$D/amfi_bekci_sd_v2.sh" </dev/null >/tmp/amfi-launch.log 2>&1 &
    echo "amfi gate v1 requested (idle mute, A3 unchanged)"
fi

# 6) mikrofon yolu (MIC3)
( i=0; while [ ! -e /dev/snd/controlC0 ] && [ $i -lt 30 ]; do sleep 1; i=$((i + 1)); done
  if [ -x $D/ses-v2 ] && [ -e /dev/snd/controlC0 ]; then
	timeout 5 $D/ses-v2 ayar 0 "ADC3 Input MIC3 Boost Switch" 1 && echo "MIC3 yolu acildi" || echo "MIC3 yolu ACILAMADI"
  else
	echo "MIC3: ses-v2 ya da ses karti yok"
  fi ) &

# 7) kamera + kopru; kayit son secime gore
mkdir -p /mnt/sd/kamera
# v15: kayit ZORLA ACILMAZ. /mnt/sd/kamera/kayit-acik bayragi SD'de kalici; Merkez/panelden acilinca dongude
#   (2 kare/sn, 1 dk parca, 6 saat) surer, kapatilinca kapali kalir. Acilista yalniz durumu yazar.
if [ -f /mnt/sd/kamera/kayit-acik ]; then echo "kamera kaydi acik (son secim)"; else echo "kamera kaydi kapali (son secim)"; fi
K=kamera_kopru_sd_v12.sh	# v48
[ -x "$D/$K" ] && [ -x "$D/kole-kopru-v9" ] || K=kamera_kopru_sd_v11.sh	# v47
[ -x "$D/$K" ] && [ -x "$D/kole-kopru-v9" ] || K=kamera_kopru_sd_v10.sh	# v46
[ -x "$D/$K" ] && [ -x "$D/kole-kopru-v9" ] || K=kamera_kopru_sd_v9.sh
[ -x "$D/$K" ] && [ -x "$D/kole-kopru-v9" ] || K=kamera_kopru_sd_v8.sh
[ -x "$D/$K" ] && [ -x "$D/kole-kopru-v9" ] || K=kamera_kopru_sd_v7.sh
[ -x "$D/$K" ] && [ -x "$D/kole-kopru-v9" ] || K=kamera_kopru_sd_v6.sh
[ -x "$D/$K" ] && [ -x "$D/kole-kopru-v9" ] || K=kamera_kopru_sd_v5.sh
if [ -x "$D/$K" ]; then
	"$D/$K" </dev/null >/tmp/kamera-kopru-sd.log 2>&1 &
	echo "kamera/kopru gozetici v5 istendi"
fi

# 8) cihaz merkezi (PC'siz web)
CM=kole-cihaz-merkez-v2
for v in 9 8 7 6 5 4 3; do
	if [ -x $D/kole-cihaz-merkez-v$v ]; then CM=kole-cihaz-merkez-v$v; break; fi
done
if [ -x $D/$CM ] && [ -n "$GZ" ]; then	# v40
	echo "merkez|3|-500|/tmp/kole-cihaz-merkez.log|256|0|||-|$D/$CM" >> /tmp/kole-gozetici.conf
	echo "cihaz merkezi (gozetici)"
elif [ -x $D/$CM ]; then
	( while true; do
		$D/$CM </dev/null >>/tmp/kole-cihaz-merkez.log 2>&1 &
		m=$!
		sleep 1; echo -500 > /proc/$m/oom_score_adj 2>/dev/null
		wait $m
		echo "$(date '+%T') cihaz merkezi cikti ($?), 3 sn sonra yeniden" >>/tmp/kole-cihaz-merkez.log
		sleep 3
	  done ) &
	echo "cihaz merkezi dongusu basladi"
fi

# 9) Tailscale
TB=tailscale_baslat_v10.sh	# v57
[ -x $D/$TB ] || TB=tailscale_baslat_v9.sh	# v53
[ -x $D/$TB ] || TB=tailscale_baslat_v8.sh	# v52
[ -x $D/$TB ] || TB=tailscale_baslat_v7.sh	# v51
[ -x $D/$TB ] || TB=tailscale_baslat_v6.sh	# v50
[ -x $D/$TB ] || TB=tailscale_baslat_v5.sh	# v46
[ -x $D/$TB ] || TB=tailscale_baslat_v3.sh	# v33
[ -x $D/$TB ] || TB=tailscale_baslat_v2.sh
if [ -x $D/$TB ]; then
	setsid sh $D/$TB </dev/null >/tmp/tailscale-baslat.out 2>&1 &
	echo "tailscale gozetici istendi ($TB)"
fi
# Hey Kole asistani: mikrofonu dinler, panel 0.29 konusan yuzle cevaplar. /data/asistan/kapali varsa mikrofonu acmaz.
KA=kole-asistan-v8
[ -x $D/$KA ] || KA=kole-asistan-v7
[ -x $D/$KA ] || KA=kole-asistan-v6
[ -x $D/$KA ] || KA=kole-asistan-v5
[ -x $D/$KA ] || KA=kole-asistan-v4
if [ -x $D/$KA ] && [ -n "$GZ" ]; then	# v40
	echo "asistan|5|x|/tmp/asistan.log|256|0|||-|$D/$KA" >> /tmp/kole-gozetici.conf
	echo "Hey Kole asistani (gozetici)"
elif [ -x $D/$KA ]; then
	( while true; do
		$D/$KA </dev/null >>/tmp/asistan.log 2>&1
		echo "$(date '+%T') kole-asistan cikti ($?), 5 sn sonra yeniden" >>/tmp/asistan.log
		[ "$(wc -c < /tmp/asistan.log)" -gt 262144 ] && tail -c 65536 /tmp/asistan.log > /tmp/asistan.log.k && mv /tmp/asistan.log.k /tmp/asistan.log
		sleep 5
	  done ) &
	echo "Hey Kole asistani dongusu basladi"
fi
# PC'siz Pano verisi: hava + doviz (panel 0.32 /tmp/panel-veri.json okur). Saat dogru olunca, sonra 10 dk'da bir.
G=
for n in 51 50 49 48 47 46 45 44 43 42 41 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2; do [ -x $D/asistan/kole-gemini-v$n ] && { G=$D/asistan/kole-gemini-v$n; break; }; done
if [ -n "$G" ] && [ -n "$GZ" ]; then	# v40
	PK=; [ "$GZ" = $D/kole-gozetici-v4 ] || [ "$GZ" = $D/kole-gozetici-v5 ] && PK='!/tmp/ikinci-ekran-acik'	# v54: ikinci ekran acikken pano yok
	echo "pano|600|x|/tmp/panel-veri.log|64|180|$PK||SSL_CERT_FILE=$D/tailscale/ca-certificates.crt;GOMEMLIMIT=12MiB;GOGC=50|$G --veri" >> /tmp/kole-gozetici.conf
	echo "Pano verisi (gozetici)"
elif [ -n "$G" ]; then
	( i=0; while [ "$(date +%Y)" -lt 2026 ] && [ $i -lt 180 ]; do sleep 2; i=$((i + 2)); done
	  while true; do
		SSL_CERT_FILE=$D/tailscale/ca-certificates.crt GOMEMLIMIT=12MiB GOGC=50 $G --veri >>/tmp/panel-veri.log 2>&1
		[ "$(wc -c < /tmp/panel-veri.log)" -gt 65536 ] && : > /tmp/panel-veri.log
		sleep 600
	  done ) </dev/null >/dev/null 2>&1 &
	echo "Pano verisi dongusu basladi (PC'siz)"
fi
# Telegram bildirim kuyrugu: panel/kamera /tmp/bildirim-kuyruk/*.txt yazar; jeton varsa 10 sn'de bir gonderilir.
TG=
for n in 51 50 49 48 47 46 45 44 43 42 41 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2; do [ -x $D/asistan/kole-gemini-v$n ] && { TG=$D/asistan/kole-gemini-v$n; break; }; done
if [ -n "$GZ" ] && [ -n "$TG" ] && [ "${TG##*-v}" -ge 7 ]; then	# v40
	TGF=$D/kole-telgraf-v7; [ -x $TGF ] || TGF=$D/kole-telgraf-v6; [ -x $TGF ] || TGF=$D/kole-telgraf-v5; [ -x $TGF ] || TGF=$D/kole-telgraf-v4; [ -x $TGF ] || TGF=$D/kole-telgraf-v3; [ -x $TGF ] || TGF=$D/kole-telgraf-v2; [ -x $TGF ] || TGF=$D/kole-telgraf-v1	# v46
	if [ -x $TGF ] && [ "${TG##*-v}" -ge 41 ] && [ ! -f $D/telgraf-kapali ]; then	# v46: en yeni Go (v41+ tek atis)
		echo "telegram|15|x|/tmp/bildirim.log|64|-1|/data/asistan/telegram-jeton||SSL_CERT_FILE=$D/tailscale/ca-certificates.crt;GOMEMLIMIT=12MiB;GOGC=50|$TGF --go $TG" >> /tmp/kole-gozetici.conf
		echo "Telegram (gozetici: C ${TGF##*/} + Go ${TG##*/} tek atis)"
	else
	echo "telegram|15|x|/tmp/bildirim.log|64|-1|/data/asistan/telegram-jeton||SSL_CERT_FILE=$D/tailscale/ca-certificates.crt;GOMEMLIMIT=12MiB;GOGC=50|$TG --telegram" >> /tmp/kole-gozetici.conf
	echo "Telegram (gozetici)"
	fi
	setsid $GZ /tmp/kole-gozetici.conf </dev/null >>/tmp/kole-gozetici.log 2>&1 &
	echo "kole-gozetici basladi ($(grep -c . /tmp/kole-gozetici.conf) is)"
else
( while true; do
	if [ -f /data/asistan/telegram-jeton ] && [ "$(date +%Y)" -ge 2026 ]; then
		B=
		for n in 51 50 49 48 47 46 45 44 43 42 41 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2; do [ -x $D/asistan/kole-gemini-v$n ] && { B=$D/asistan/kole-gemini-v$n; break; }; done
		if [ -n "$B" ]; then
			if [ "${B##*-v}" -ge 7 ]; then K2=--telegram; else K2=--bildirim-gonder; fi
			SSL_CERT_FILE=$D/tailscale/ca-certificates.crt GOMEMLIMIT=12MiB GOGC=50 $B $K2 >>/tmp/bildirim.log 2>&1
		fi
		[ "$(wc -c < /tmp/bildirim.log)" -gt 65536 ] && : > /tmp/bildirim.log
	fi
	sleep 15
  done ) </dev/null >/dev/null 2>&1 &
echo "Telegram dongusu basladi (iki yonlu)"
[ -n "$GZ" ] && [ -s /tmp/kole-gozetici.conf ] && { setsid $GZ /tmp/kole-gozetici.conf </dev/null >>/tmp/kole-gozetici.log 2>&1 & echo "kole-gozetici basladi (Telegram eski dongude)"; }
fi
# v33: bellek ayarlari (vm, K1 havuzu, tek kullanimlik dosyalar, OOM oncelikleri); $D/bellek-ayar-kapali ile kapanir
BA=bellek_ayar_v6.sh	# v50
[ -x $D/$BA ] || BA=bellek_ayar_v5.sh	# v48
[ -x $D/$BA ] || BA=bellek_ayar_v4.sh	# v37
[ -x $D/$BA ] || BA=bellek_ayar_v3.sh	# v36
[ -x $D/$BA ] || BA=bellek_ayar_v2.sh	# v34
[ -x $D/$BA ] || BA=bellek_ayar_v1.sh
if [ -x $D/$BA ]; then
	setsid sh $D/$BA </dev/null >/tmp/bellek-ayar.out 2>&1 &
	echo "bellek ayari istendi ($BA)"
fi
# v38: watchdog beslemesi C'ye (arka planda; acilisi bekletmez)
W=$D/kole-wdt-v2; [ -x $W ] || W=$D/kole-wdt-v1	# v46: v2 bellege kilitli
if [ -x $W ] && [ ! -e /data/wdt-kapali ]; then
	( if [ -e /data/kole-wdt-deneme ]; then
		rm -f /data/kole-wdt-deneme
		echo "kole-wdt: onceki devir 5 dk dayanmadi, bu acilista init dongusu kaliyor"
	  else
		IP=
		for p in /proc/[0-9]*; do
			case "$(tr '\0' ' ' < $p/cmdline 2>/dev/null)" in *"devmem 0x020500B0 32 0x14af; sleep 4"*) IP=${p#/proc/} ;; esac
		done
		if [ -n "$IP" ]; then
			: > /data/kole-wdt-deneme; sync
			rm -f /tmp/kole-wdt.durum
			setsid $W </dev/null >>/tmp/kole-wdt.log 2>&1 &
			i=0	# SD yuklemesi gecikebilir (sync sonrasi 9 sn olculdu): 20 sn'ye kadar 2+ besleme bekle
			while [ $i -lt 20 ] && ! grep -qE '^besleme ([2-9]|[1-9][0-9]+) ' /tmp/kole-wdt.durum 2>/dev/null; do sleep 1; i=$((i + 1)); done
			if grep -qE '^besleme ([2-9]|[1-9][0-9]+) ' /tmp/kole-wdt.durum 2>/dev/null; then
				kill $IP && echo "kole-wdt: besleme devralindi (init dongusu $IP durdu)"
			else
				echo "kole-wdt: besleme dogrulanamadi, init dongusu kaliyor"
			fi
		fi
	  fi ) >>/tmp/kole-wdt.log 2>&1 &
fi
# v46: wifi-kaydet denemesi sirasinda reset olduysa (.eski kaldi) son calisan ayar geri (inceleme 24.10). init'in
#   wifi-baslat'i hatali ayarla baslamis olabilir: 15 sn'de IP yoksa durdur, cikmasini bekle, yeniden baslat.
if [ -f /data/wifi/wpa.conf.eski ]; then
	mv -f /data/wifi/wpa.conf.eski /data/wifi/wpa.conf && sync
	echo "[wifi] yarim kalan wifi-kaydet: son calisan ayar geri yuklendi" > /dev/kmsg
	( sleep 15
	  if pidof wpa_supplicant >/dev/null && ! ip -4 addr show wlan0 2>/dev/null | grep -q 'inet '; then
		killall udhcpc wpa_supplicant 2>/dev/null
		i=0
		while pidof wpa_supplicant >/dev/null && [ $i -lt 10 ]; do sleep 1; i=$((i + 1)); done
		wifi-baslat -f
	  fi ) </dev/null >/dev/null 2>&1 &
fi
# v41: acilista Wi-Fi atlandiysa gec deneme (sistem oturunca radyo acilisi guc cokmesi yapmiyor: 23.10 olcumu)
( if dmesg | grep -q 'denemesi cokmus'; then
	if [ -e /data/.wifi-gec-deneme ]; then
		rm -f /data/.wifi-gec-deneme; sync
		echo "[wifi] gec deneme de cokmustu -> bu acilista Wi-Fi yok (elle: wifi-baslat)" > /dev/kmsg
	else
		sleep 60
		if ! ip -4 addr show wlan0 2>/dev/null | grep -q 'inet '; then
			: > /data/.wifi-gec-deneme; sync
			echo "[wifi] acilista atlanmisti -> gec deneme (uptime $(cut -d' ' -f1 /proc/uptime))" > /dev/kmsg
			wifi-baslat
			i=0
			while [ $i -lt 35 ] && ! ip -4 addr show wlan0 2>/dev/null | grep -q 'inet '; do sleep 1; i=$((i + 1)); done
			if ip -4 addr show wlan0 2>/dev/null | grep -q 'inet '; then
				rm -f /data/.wifi-gec-deneme; sync
				echo "[wifi] gec deneme basarili: $(ip -4 addr show wlan0 | grep -o 'inet [0-9.]*')" > /dev/kmsg
			else	# v45: buraya varmak cokmedigi demek (ornek: modem kapali); bayrak kalirsa sonraki acilis Wi-Fi'yi hic denemez
				rm -f /data/.wifi-gec-deneme; sync
				echo "[wifi] gec deneme: 35 sn'de IP yok (cokme degil), bayrak silindi" > /dev/kmsg
			fi
		fi
	fi
  fi ) </dev/null >/dev/null 2>&1 &
# v46: wifi-kaydet SD surumu (yedekli, dogrulamali; denetim v2 bulgu 4). $D/wifi-kaydet-eski ile kapanir.
WK=$(command -v wifi-kaydet)
if [ -n "$WK" ] && [ -f $D/wifi_kaydet_sd_v1.sh ] && [ ! -f $D/wifi-kaydet-eski ]; then
	cp $D/wifi_kaydet_sd_v1.sh "$WK.yeni" && chmod 755 "$WK.yeni" && mv "$WK.yeni" "$WK" && echo "wifi-kaydet SD surumu: $WK"
fi
# v46: NTP tekrar (wifi-baslat yalniz bir kez deniyor; saat 1970'te kalirsa kamera kaydi durur, TLS dogrulamasi gecmez)
( while :; do	# v48: sinirsiz (24 sa'ten uzun internet kesintisinde saat 1970'te kaliyordu)
	sleep 300
	[ "$(date +%Y)" -ge 2026 ] && break
	if ntpd -q -p time.google.com -p pool.ntp.org >/dev/null 2>&1; then
		echo "[ntp] tekrar denemede saat ayarlandi: $(date)" > /dev/kmsg
		break
	fi
  done ) </dev/null >/dev/null 2>&1 &
# v48: Wi-Fi bekcisi (wpa_supplicant/udhcpc olurse geri getirir; $D/wifi-bekci-kapali ile kapanir)
[ -x $D/wifi_bekci_sd_v1.sh ] && [ ! -f $D/wifi-bekci-kapali ] && { $D/wifi_bekci_sd_v1.sh </dev/null >/dev/null 2>&1 & echo "wifi bekcisi basladi"; }
echo "autorun v48 bitti: $(cat /proc/uptime)"
