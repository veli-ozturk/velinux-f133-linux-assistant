#!/bin/bash
# 625 (24.10.2026, Claude; denetim-2026-10-24-v3 A5, A14, A18; 604 kalibi): kole_kopru v12 = v11 + ses_akis_v4.h +
# rtsp_sunucu_v2.h + kamera_akis_v2.h + komut satiri toplam 5 sn (scripts/624). 604 denetimleri yeni basliklarla +
# v12: birak bayragi O_EXCL, nonce IP'li. Cikti ~/f133/out/kole-kopru-v12/ ve work/terminal-local/ota-068/kole-kopru-v12/.
# CIHAZA KURMAZ. --- 604 ---
# 604 (23.10.2026, Claude; denetim-2026-10-23-v2 bulgu 6, 8; 180 kalibi): kole_kopru v11 = v9 + izinli_ip_v1.h (8767 ve RTSP
# 8554 kabulunde) + stdout gunlugu 256 KB kirpma. 180 ile ayni salt-okuma/ses/UART denetimleri + host self-test + DURUM JSON
# + RISC-V statik. Cikti ~/f133/out/kole-kopru-v12/ ve ana klasor work/terminal-local/ota-068/kole-kopru-v12/. CIHAZA KURMAZ.
# (180:) kole_kopru v9 = v8 + ses_akis_v2.h (scripts/178). (133:) v7 + MIC_AKIS / PLAY_AKIS + asgari RTSP sunucusu.
set -euo pipefail
P="${P:-$HOME/f133/proj}"	# P: baska agac (ana klasor baska daldayken worktree)
D="$P/initramfs/src"
S="$D/kole_kopru_v12.c"
O="$HOME/f133/out/kole-kopru-v12"
TUM=("$S")
for h in kopru_ortak_v1.h ses_akis_v4.h kamera_akis_v2.h kayit_sd_v1.h rtsp_v1.h rtsp_sunucu_v2.h kamera_olay_v1.h izinli_ip_v1.h; do
	TUM+=("$D/$h")
done
mkdir -p "$O"
# 1) Kamera yalniz okunur: tek CDB kurucusu, READ(10)=0x28; yazma/vendor komutu, kontrol aktarimi yok.
[ "$(cat "${TUM[@]}" | grep -c 'uint8_t cb\[10\] = {')" = 1 ] || { echo "HATA: birden fazla CDB kurucusu"; exit 1; }
grep -q 'uint8_t cb\[10\] = { 0x28,' "$D/kamera_akis_v2.h" || { echo "HATA: CDB READ(10) degil"; exit 1; }
if grep -n -E 'cb\[0\] *=|USBDEVFS_CONTROL' "${TUM[@]}"; then echo "HATA: kamera yazma izi"; exit 1; fi
# rtsp_v1.h saf yardimci (Huffman tablolarinda 0x2a sabiti var): USB/ioctl/dosya erisimi icermemeli
if grep -n -E 'usbdevfs|ioctl|open\(|USBDEVFS' "$D/rtsp_v1.h"; then echo "HATA: rtsp_v1.h saf degil"; exit 1; fi
for f in "${TUM[@]}"; do
	[ "$f" = "$D/rtsp_v1.h" ] && continue
	if grep -n -E '0x2[aA][^0-9a-fA-F]' "$f"; then echo "HATA: kamera yazma izi (WRITE(10)=0x2A) $f"; exit 1; fi
done
# 2) ALSA: yalniz "Headphone Switch" (v7 PLAY ile ayni); baska mikser/yonlendirme degisikligi yok.
K=$(grep -h -o 'bool_ayar("[^"]*"' "${TUM[@]}" | sort -u)
[ "$K" = 'bool_ayar("Headphone Switch"' ] || { echo "HATA: beklenmeyen mikser kontrolu: $K"; exit 1; }
if grep -n -E 'ELEM_WRITE' "${TUM[@]}" | grep -v 'ses_akis_v4.h'; then echo "HATA: ses_akis disinda mikser yazimi"; exit 1; fi
# 3) MCU'ya cerceve yok: UART aygiti acilmaz.
if grep -n -E '/dev/tty|ttyS' "${TUM[@]}"; then echo "HATA: UART/MCU erisimi"; exit 1; fi
# 4) Ses/goruntu diske yazilmaz: fopen/open yazma yalniz bayrak, durum, parola ve self-test dosyalarinda.
echo "yazma noktalari (gozden gecirme icin):"
grep -n -E 'O_WRONLY|fopen\([^)]*"w' "${TUM[@]}" | sed "s#$D/##" | sed 's/^/  /'
# 5) v11: her iki kabul noktasi izinli_ip'den gecer (8767 accept, RTSP accept4 sarmalayicisi)
[ "$(grep -c 'izinli_soket(s, NULL)' "$S")" = 2 ] && grep -q '^#define accept4 izinli_accept4' "$S" &&
	[ "$(grep -c 'accept4(' "$D/rtsp_sunucu_v2.h")" = 1 ] || { echo "HATA: izinli_ip kabul denetimi eksik"; exit 1; }
gcc -O2 -Wall -Wextra -Werror -Wno-format-truncation -I"$D" "$S" -lm -o "$O/kole-kopru-v12-host"
"$O/kole-kopru-v12-host" --self-test
# v12: birak bayragi yalniz O_EXCL ile acilir (TTS'inkini ezmez), nonce IP'li, artik bayrak temizligi baslangicta
grep -q 'open(asistan_birak_yol, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644)' "$D/ses_akis_v4.h" || { echo "HATA: birak bayragi O_EXCL degil"; exit 1; }
if grep -n 'bayrak_yaz_icerik(ASISTAN_BIRAK\|unlink(ASISTAN_BIRAK' "$D/ses_akis_v4.h"; then echo "HATA: sahipsiz birak bayragi"; exit 1; fi
grep -q 'nonce_uret(n, c->ip);' "$D/rtsp_sunucu_v2.h" || { echo "HATA: nonce IP'siz"; exit 1; }
"$O/kole-kopru-v12-host" --durum-ornek | python3 -m json.tool >/dev/null && echo "DURUM JSON gecerli"
riscv64-linux-gnu-gcc -static -O2 -Wall -Wextra -Werror -Wno-format-truncation -I"$D" "$S" -lm -o "$O/kole-kopru-v12"
riscv64-linux-gnu-strip "$O/kole-kopru-v12"
file "$O/kole-kopru-v12"
sha256sum "$O/kole-kopru-v12" | tee "$O/kole-kopru-v12.sha256"
stat -c 'bytes=%s' "$O/kole-kopru-v12"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kole-kopru-v12"
mkdir -p "$H" && cp "$O/kole-kopru-v12" "$H/" && echo "kopyalandi: $H"
