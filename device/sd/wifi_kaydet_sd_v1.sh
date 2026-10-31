#!/bin/sh
# wifi_kaydet_sd_v1 (23.10.2026, Claude; scripts/609, denetim-2026-10-23-v2 bulgu 4; inceleme 24.10): /bin/wifi-kaydet yerine
#   (autorun_v46 RAM'e kopyalar). initramfs surumu /data/wifi/wpa.conf'u yedeksiz uzerine yaziyordu: yanlis ag ya da sifre
#   calisan ayari siliyordu (Wi-Fi uzaktan erisimin tek yolu), 63+ karakter sifre dosyayi bozuyordu, Turkce harfli SSID
#   (iw \xNN gosterir) hic eslesmiyordu.
#   - sifre 8..63 BAYT (wpa_supplicant bayt sayar), satir sonu yok; ssid=P"..." (wpa_supplicant \xNN ve \" cozer; iw "
#     isaretini kacissiz gosterir -> \")
#   - yarim kalmis onceki denemenin .eski'si (son calisan ayar) once geri alinir; yedek denetimli, yeni .yeni + mv
#   - yeniden baglanma: udhcpc + wpa_supplicant durdurulup CIKMASI beklenir (eski surec kontrol soketini birakmadan yenisi
#     baslarsa yenisi hemen cikar: cihaz Wi-Fi'siz kalirdi), sonra wifi-baslat -f; wpa_supplicant yoksa bir kez daha
#   - WK_BEKLE (40; udhcpc 3 istek/9 sn + 20 sn bekleme) sn icinde wpa_state=COMPLETED + ip_address yoksa eski ayar geri
#   Cikis: 0 baglandi, 1 /data yok ya da yazilamadi, 2 gecersiz girdi, 3 baglanamadi (eski ayar yoktu), 4 baglanamadi,
#   eski ayar geri yuklendi. Test kancasi: WK_C (wpa.conf yolu; /data denetimi atlanir), WK_BEKLE.
C=${WK_C:-/data/wifi/wpa.conf}
B=${WK_BEKLE:-40}
[ -n "$2" ] || { echo "kullanim: wifi-kaydet <ssid> <sifre>"; exit 2; }
n=$(printf %s "$2" | wc -c)
[ "$n" -ge 8 ] && [ "$n" -le 63 ] || { echo "sifre 8-63 bayt olmali ($n)"; exit 2; }
case "$1$2" in *'
'*) echo "satir sonu olamaz"; exit 2;; esac
if [ -z "$WK_C" ]; then mountpoint -q /data || { echo "/data bagli degil"; exit 1; }; fi
mkdir -p "${C%/*}"
umask 077
[ -f "$C.eski" ] && mv -f "$C.eski" "$C"
[ -f "$C" ] && { cp "$C" "$C.eski" || { echo "yedeklenemedi"; rm -f "$C.eski"; exit 1; }; }
q=$(printf %s "$1" | sed 's/"/\\"/g')
printf 'ctrl_interface=/var/run/wpa_supplicant\nnetwork={\n\tssid=P"%s"\n\tpsk="%s"\n\tkey_mgmt=WPA-PSK\n}\n' "$q" "$2" > "$C.yeni" &&
	mv "$C.yeni" "$C" || { echo "yazilamadi"; rm -f "$C.yeni" "$C.eski"; exit 1; }
sync
echo "kaydedildi: ssid=$1"
yeniden() {
	killall udhcpc wpa_supplicant 2>/dev/null
	j=0
	while pidof wpa_supplicant >/dev/null && [ $j -lt 10 ]; do sleep 1; j=$((j + 1)); done
	wifi-baslat -f
	sleep 1
	pidof wpa_supplicant >/dev/null || { echo "wpa_supplicant baslamadi, yeniden"; wifi-baslat -f; }
}
yeniden
i=0
while [ $i -lt "$B" ]; do
	sleep 1
	i=$((i + 1))
	s=$(wpa_cli -i wlan0 status 2>/dev/null)	# ip_address= satiri IP alininca gelir (cihazda dogrulandi)
	if echo "$s" | grep -q '^wpa_state=COMPLETED' && echo "$s" | grep -q '^ip_address='; then
		echo "baglandi ($i sn)"
		rm -f "$C.eski"
		sync
		exit 0
	fi
done
if [ -f "$C.eski" ]; then
	mv -f "$C.eski" "$C"
	sync
	echo "baglanamadi: eski ayar geri yuklendi"
	yeniden
	exit 4
fi
echo "baglanamadi (eski ayar yoktu)"
exit 3
