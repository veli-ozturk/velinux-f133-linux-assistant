#!/bin/bash
# 643 (25.10.2026, Claude): kole-cihaz-merkez v18 = v17 + sor/soyle/hatirlat cihazda (scripts/642) + E2E
#   kole_merkez_v18_test_v1. Kurulum adi kole-cihaz-merkez-v9. --- 640 ---
# 640 (24.10.2026, Claude): kole-cihaz-merkez v17 = v16 + OTONOM 149 A20 (127.0.0.2 dinleyici, vekil basliklari yalniz
#   oradan) + E2E kole_merkez_v17_test_v1. Kurulum adi kole-cihaz-merkez-v9. --- 638 ---
# 638 (24.10.2026, Claude): kole-cihaz-merkez v16 = v15 + OTONOM 150 B7 (foto listesi klasor basina 2000 stat). Zaman testi v5
#   (surum ikili adindan). Kurulum adi kole-cihaz-merkez-v9. --- 636 ---
# 636 (24.10.2026, Claude): kole-cihaz-merkez v15 = v14 + OTONOM 152 (B5 isareti kopru/panel kapatmasinda da; self-test v15).
#   Denetimler 627/635 ile ayni; zaman testi v4. Kurulum adi kole-cihaz-merkez-v9. --- 635 ---
# 635 (24.10.2026, Claude): kole-cihaz-merkez v14 = v13 + yalniz SURUM_TAM v14 (OTONOM 151); zaman testi v3 surumu bekler.
#   Denetimler 627 ile ayni. Kurulum adi kole-cihaz-merkez-v9. --- 627 ---
# 627 (24.10.2026, Claude): kole-cihaz-merkez v13 = v12 + denetim v3 A4/A5/A16/A19 + inceleme B1/B2/B5/B6 (scripts/626).
#   617 denetimleri yeni basliklarla (ses_akis_v4.h, kamera_akis_v2.h, RTSP yollari rtsp_sunucu_v2.h ile) + v13
#   self-test + E2E kole_merkez_v13_test_v1 (okumayan dinleyici, birak bayragi, jeton iptali 502). Yeniden baslatma
#   betigi ve reboot yalniz #ifndef KOLE_TEST icinde, birer kez. Kurulum adi kole-cihaz-merkez-v9. --- 617 ---
# 617 (24.10.2026, Claude): kole-cihaz-merkez v12 = v11 + denetim v1 12-15/20 + OTONOM 144 (scripts/616). 543 ile
#   ayni denetimler + v12 self-test; P degistirilebilir (worktree: P=~/f133/wt); ikili ana klasor
#   work/terminal-local/ota-068/kole-cihaz-merkez-v18/ altina da. Kurulum adi kole-cihaz-merkez-v9. --- 543 ---
# 543 (22.10.2026, Claude): kole-cihaz-merkez v11 = v10 + time-lapse 1 sn, olay kipi, /api/zaman/olay (scripts/542).
#   514 denetimleri (v11 uyarlamalari) + E2E kole_merkez_zaman_test_v2. Kurulum adi kole-cihaz-merkez-v9. --- 514 ---
# 514 (22.10.2026, Claude): kole-cihaz-merkez v10 = v9 + time-lapse/foto uclari + giris kullanici adi (scripts/513).
#   386 denetimleri + v10: zaman/foto yalniz okuma (yazma yalniz zaman-acik/zaman-aralik), E2E kole_merkez_zaman_test_v1.
#   Kurulum adi kole-cihaz-merkez-v9 (autorun v32). --- 386 ---
# 386 (20.10.2026, Claude): kole-cihaz-merkez v9 = v8 + parolali /api/yeniden-baslat + OTA adi wifi-ekran-vN (scripts/385).
#   384 denetimleri + "YENIDEN BASLATMA" yeniden_parola denetimi + E2E yeniden baslatma ve cek bicimli kare. --- 384 ---
# 384 (20.10.2026, Claude): kole-cihaz-merkez v8 = v6 + CEK bicimli canli kare + gomulu /canli (scripts/383).
#   304 denetimleri + E2E work/kole_merkez_kare_test_v1.py (KOLE_TEST + ASAN). --- 304 ---
# 304 (19.10.2026, Claude): kole-cihaz-merkez v6 = v5 + pid.mcu onekle (scripts/303). 261 ile ayni denetimler.
# (261:) kole-cihaz-merkez v5 = v4 + /api/cihaz hizmetler/surumler/ozellik (scripts/260).
# (241:) kole-cihaz-merkez v4 = v3 + kablosuz guncelleme (scripts/240). 179/145 denetimleri +
#   OTA denetimleri (iki modda parola: yeniden_parola "GUNCELLEME YUKLEME", Bearer gecmez, izin listesi self-test).
# (179:) kole-cihaz-merkez v3 = v2 + ses_akis_v2.h (scripts/178). 145 ile ayni denetimler.
# (145:) kole-cihaz-merkez v2 (v1 + "tek dogrulama" guvenlik modu, /api/guvenlik) derleme + denetim.
#   1) kaynak denetimleri: kamera yalniz READ(10) (v8 basliklari aynen), mikser yalniz "Headphone Switch", UART yok,
#      yazma noktalari listesi (goruntu/ses diske yazilmaz), RTSP yollari rtsp_sunucu_v1.h ile ayni
#   2) host: -Wall -Wextra -Werror; --self-test (kripto vektorleri, HTTP/WS/JSON bulanik, oturum, sinirlar, yollar, plan)
#   3) host ASAN+UBSAN: --self-test (sizinti dahil)
#   4) host KOLE_TEST (E2E icin, 146 kullanir) + ASAN/UBSAN KOLE_TEST surumu
#   5) RISC-V statik (KOLE_TEST YOK) uyarisiz; boyut + sha256
# CIHAZA KURMAZ, SD'YE YAZMAZ, INDIRME YAPMAZ.
set -euo pipefail
P="${P:-$HOME/f133/proj}"
D="$P/initramfs/src"
S="$D/kole_cihaz_merkez_v18.c"
O="$P/out"
mkdir -p "$O"
BAS=(merkez_canli_v1.h kopru_ortak_v1.h ses_akis_v4.h kamera_akis_v2.h kayit_sd_v1.h kamera_olay_v1.h rtsp_v1.h kripto_v1.h http_ws_v1.h json_mini_v1.h)
TUM=("$S")
for h in "${BAS[@]}"; do TUM+=("$D/$h"); done
echo "== 1) kaynak denetimleri"
[ "$(cat "${TUM[@]}" | grep -c 'uint8_t cb\[10\] = {')" = 1 ] || { echo "HATA: birden fazla CDB kurucusu"; exit 1; }
grep -q 'uint8_t cb\[10\] = { 0x28,' "$D/kamera_akis_v2.h" || { echo "HATA: CDB READ(10) degil"; exit 1; }
if grep -n -E 'cb\[0\] *=|USBDEVFS_CONTROL' "${TUM[@]}"; then echo "HATA: kamera yazma izi"; exit 1; fi
for f in "${TUM[@]}"; do
	case "$f" in */rtsp_v1.h|*/kripto_v1.h) continue;; esac	# Huffman/SHA sabitleri
	if grep -n -E '0x2[aA][^0-9a-fA-F]' "$f"; then echo "HATA: WRITE(10)=0x2A izi $f"; exit 1; fi
done
K=$(grep -h -o 'bool_ayar("[^"]*"' "${TUM[@]}" | sort -u)
[ "$K" = 'bool_ayar("Headphone Switch"' ] || { echo "HATA: beklenmeyen mikser kontrolu: $K"; exit 1; }
if grep -n -E '/dev/tty|ttyS' "${TUM[@]}"; then echo "HATA: UART/MCU erisimi"; exit 1; fi
if grep -n -E 'usbdevfs|USBDEVFS|ioctl' "$S" "$D/kripto_v1.h" "$D/http_ws_v1.h" "$D/json_mini_v1.h"; then echo "HATA: merkez dosyasinda dogrudan aygit erisimi"; exit 1; fi
for y in rtsp-acik rtsp-parola rtsp-durum.json; do
	a=$(grep -o "\"[^\"]*$y\"" "$D/rtsp_sunucu_v2.h" | head -1); b=$(grep -o "\"[^\"]*$y\"" "$S" | head -1)
	[ "$a" = "$b" ] || { echo "HATA: RTSP yolu farkli: $a / $b"; exit 1; }
done
# v2: iki modda da parola isteyen islemler yeniden_parola'dan (hassas() DEGIL) gecmeli; ek "giris" icermemeli
for o in '"GUNCELLEME YUKLEME"' '"RTSP ACMA"' '"RTSP PAROLASI"' '"KISAYOL JETONU"' '"KISAYOL JETONU IPTALI"' '"OTURUM KAPATMA"' '"TUM OTURUMLARI KAPATMA"' '"YENIDEN BASLATMA"'; do
	grep -q "yeniden_parola(c, $o" "$S" || { echo "HATA: $o yeniden_parola ile degil"; exit 1; }
done
grep -q 'GUVENLIK MODU DEGISIMI" : "OTURUM BOSTA SURESI ARTIRMA", guvenlik_devam' "$S" || { echo "HATA: guvenlik modu parolasi"; exit 1; }
grep -q '^#define TEK_EK " (tek dogrulama)"' "$S" || { echo "HATA: TEK_EK PC v7 ile ayni degil"; exit 1; }
# v5: sir dosyalari (jeton/anahtar) yalniz dosya_var ile; icerik okunmaz
for v in tg_jeton_yol gemini_yol; do
	if grep -n "dosya_oku($v\|fopen($v\|open($v" "$S"; then echo "HATA: $v icerigi okunuyor"; exit 1; fi
done
# v4 OTA: iki uc da yalniz oturum (izin(c, 0)); hassas() ile gevsetilmez
grep -A1 '^static void api_ota_baslat' "$S" | grep -q '^{' && grep -A2 '^static void api_ota_baslat' "$S" | grep -q 'if (izin(c, 0)) return;' || { echo "HATA: ota_baslat izin"; exit 1; }
grep -A3 '^static void ota_yukle_basla' "$S" | grep -q 'if (izin(c, 0))' || { echo "HATA: ota_yukle izin"; exit 1; }
if grep -n 'hassas(c, "GUNCELLEME' "$S"; then echo "HATA: OTA hassas() ile"; exit 1; fi
echo "yazma noktalari (gozden gecirme; goruntu/ses YOK):"
grep -n -E 'O_WRONLY|O_RDWR|fopen\([^)]*"w' "$S" | sed "s#$D/##" | sed 's/^/  /'
CF=(-O2 -Wall -Wextra -Werror -Wno-format-truncation -I"$D")
echo "== 2) host"
gcc "${CF[@]}" "$S" -lm -o "$O/kole-cihaz-merkez-v18-host"
"$O/kole-cihaz-merkez-v18-host" --self-test
"$O/kole-cihaz-merkez-v18-host" --pbkdf2-olc
echo "== 3) host ASAN+UBSAN self-test"
gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined -Wall -Wextra -Werror \
	-Wno-format-truncation -I"$D" "$S" -lm -o "$O/kole-cihaz-merkez-v18-asan"
ASAN_OPTIONS=detect_leaks=1 "$O/kole-cihaz-merkez-v18-asan" --self-test
echo "== 4) host KOLE_TEST (+ASAN) E2E ikilileri"
gcc "${CF[@]}" -DKOLE_TEST "$S" -lm -o "$O/kole-cihaz-merkez-v18-test"
gcc -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=undefined -Wall -Wextra -Werror \
	-Wno-format-truncation -DKOLE_TEST -I"$D" "$S" -lm -o "$O/kole-cihaz-merkez-v18-test-asan"
gcc "${CF[@]}" -DKOLE_TEST -static "$S" -lm -o "$O/kole-cihaz-merkez-v18-test-static"
echo "== 5) RISC-V statik"
riscv64-linux-gnu-gcc -static "${CF[@]}" "$S" -lm -o "$O/kole-cihaz-merkez-v18"
riscv64-linux-gnu-strip "$O/kole-cihaz-merkez-v18"
if strings "$O/kole-cihaz-merkez-v18" | grep -q '_test/saat\|--test-kok'; then echo "HATA: test kipi RISC-V ikilisinde"; exit 1; fi
file "$O/kole-cihaz-merkez-v18" | grep -q 'RISC-V.*statically linked' || { echo "HATA: RISC-V statik degil"; exit 1; }
sha256sum "$O/kole-cihaz-merkez-v18"
stat -c '%s bytes' "$O/kole-cihaz-merkez-v18"
grep -q 'd->p_mcu = hz\[6\].pid;' "$S" || { echo "HATA: pid.mcu onek degil"; exit 1; }
echo "== 6) E2E cek bicimli kare (KOLE_TEST + ASAN)"
K="$(readlink -f "$P/..")"
python3 "$K/work/kole_merkez_kare_test_v1.py" --ikili "$O/kole-cihaz-merkez-v18-test-asan"
echo "== 7) E2E parolali yeniden baslatma (KOLE_TEST: reboot cagrilmaz)"
python3 "$K/work/kole_merkez_yeniden_test_v1.py" --ikili "$O/kole-cihaz-merkez-v18-test-asan"
# v13 (A19): reboot ve betik exec'i birer kez, yalniz yeniden_baslat_tick'te ve #ifndef KOLE_TEST icinde; betik yolu sabit
[ "$(grep -cE '^\s+reboot\(RB_AUTOBOOT\);' "$S")" = 1 ] || { echo "HATA: reboot cagrisi sayisi"; exit 1; }
[ "$(grep -c 'execl(' "$S")" = 3 ] || { echo "HATA: execl sayisi (v18: 1 betik + 2 kole-gemini)"; exit 1; }
sed -n '/^static void yeniden_baslat_tick(void)/,/^}/p' "$S" | awk '/^#ifndef KOLE_TEST/{i=1} /^#endif/{i=0}
	/reboot\(RB_AUTOBOOT\);|execl\(/{n++; if (!i) b=1} END{exit !(n == 2 && !b)}' || { echo "HATA: reboot/betik KOLE_TEST disinda"; exit 1; }
grep -q '^#define YB_BETIK "/mnt/sd/velinux/yeniden_baslat_sd_v1.sh"' "$S" || { echo "HATA: yeniden baslatma betigi yolu"; exit 1; }
if grep -n -E '0x5AA5A55A|0x07090108|fel-gir' "$S"; then echo "HATA: FEL izi"; exit 1; fi
grep -q 'api_kamera_kare' "$S" && grep -qF 'c->tur == B_CANLI && cikis_ekle(c, "\r\n", 2)' "$S" || { echo "HATA: v8 degisiklikleri yok"; exit 1; }
# v10: yeni uclar yalniz okur; tek yeni yazma noktasi zaman-aralik (O_TRUNC) + bayrak_yaz(zaman-acik)
[ "$(grep -c 'zaman-aralik", kok);' "$S")" -ge 1 ] || { echo "HATA: zaman-aralik yolu"; exit 1; }
grep -q 'hassas(c, acik == 1 ? "TIME-LAPSE ACMA" : "TIME-LAPSE OLAY TESPITI ACMA", zaman_mod_devam);' "$S" || { echo "HATA: time-lapse/olay acma hassas() ile degil"; exit 1; }
# v11: olay ucu yalniz okur (oturum izni), olay bayragi yalniz bayrak_yaz ile
grep -A2 "^static void api_zaman_olay(struct bag \*c)" "$S" | grep -q 'if (izin(c, 0)) return;' || { echo "HATA: api_zaman_olay izin"; exit 1; }
[ "$(grep -c 'zaman-olay", kok);' "$S")" = 2 ] || { echo "HATA: zaman-olay yolu sayisi"; exit 1; }
for f in api_zaman_liste api_zaman_kare api_zaman_mod api_foto_liste api_foto_al; do
	grep -A2 "^static void $f(struct bag \*c)" "$S" | grep -q 'if (izin(c, 0)) return;' || { echo "HATA: $f izin"; exit 1; }
done
for f in api_zaman_kare api_foto_al; do
	sed -n "/^static void $f(struct bag \*c)/,/^}/p" "$S" | grep -q 'kayit_izin_aktif(c->oturum)' || { echo "HATA: $f kayit izni"; exit 1; }
	sed -n "/^static void $f(struct bag \*c)/,/^}/p" "$S" | grep -q 'if (kalkan())' || { echo "HATA: $f kalkan"; exit 1; }
done
echo "== 8) E2E time-lapse / foto / giris adi (KOLE_TEST + ASAN)"
python3 "$K/work/kole_merkez_zaman_test_v5.py" --ikili "$O/kole-cihaz-merkez-v18-test-asan"
# v13 (A4, A5, A16, B1)
grep -qF 'B[i].tur == B_WS_DINLE && !B[i].kapat_sonra;' "$S" || { echo "HATA: dinleyici_sayisi kapat_sonra sayiyor"; exit 1; }
if grep -n 'kalan + 5\|mic_akis(sp\[1\], 1800' "$S"; then echo "HATA: canli cocuk suresi oturuma bagli"; exit 1; fi
grep -qF 'sahipliyse_sil(asistan_birak_yol, "kopru ", OLEN[i].pid);' "$S" || { echo "HATA: olen cocuk birak bayragi"; exit 1; }
grep -qF 'bayrak_yaz_icerik(yol, ist, "plan\n");' "$S" || { echo "HATA: plan bayragi icerigi"; exit 1; }
echo "== 9) E2E v13: okumayan dinleyici / birak bayragi / jeton iptali 502 (KOLE_TEST + ASAN)"
python3 "$K/work/kole_merkez_v13_test_v1.py" --ikili "$O/kole-cihaz-merkez-v18-test-asan"
if grep -n 'pid_bul(' "$S"; then echo "HATA: v12 tek /proc taramasi degil"; exit 1; fi
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kole-cihaz-merkez-v18"
mkdir -p "$H" && cp "$O/kole-cihaz-merkez-v18" "$H/" && echo "kopyalandi: $H"
grep -q "v15 (OTONOM 152, B5)" "$S" || { echo "HATA: v15 B5 blogu yok"; exit 1; }
grep -q "if (++taranan > FOTO_TARAMA_AZAMI) break;" "$S" || { echo "HATA: B7 siniri yok"; exit 1; }
echo "== 10) E2E v17: vekil basliklari yalniz 127.0.0.2 (KOLE_TEST + ASAN)"
python3 "$K/work/kole_merkez_v17_test_v1.py" --ikili "$O/kole-cihaz-merkez-v18-test-asan"
[ "$(grep -c 'c->peer_lo && c->r.ts_login\|c->peer_lo && r->xff\|c->https = c->peer_lo' "$S")" = 0 ] || { echo "HATA: vekil basligi peer_lo ile"; exit 1; }
echo "== 11) E2E v18: sor / soyle / hatirlat (KOLE_TEST + ASAN)"
python3 "$K/work/kole_merkez_v18_test_v1.py" --ikili "$O/kole-cihaz-merkez-v18-test-asan"
echo "643 TAMAM"
