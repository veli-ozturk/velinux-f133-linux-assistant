#!/bin/bash
# 646 (25.10.2026, Claude): kamera_kayit v13 (scripts/645: B11 kayit karari fonksiyonlarda, self-test gercek karar
#   koduyla). 633 ile ayni denetimler. Davranis v12 ile ayni; CIHAZA GONDERMEZ. --- 633 ---
# 633 (24.10.2026, Claude): kamera_kayit v12 (scripts/632: esik saniyede bir, bos bayrak 60 sn). 619 ile ayni
#   denetimler + kamera_kopru_sd_v12.sh sh -n. --- 619 ---
# 619 (24.10.2026, Claude): kamera_kayit v11 (scripts/618: hareket varken 8 kare/sn). 600 ile ayni denetimler +
#   kamera_kopru_sd_v12.sh ve autorun_v47.sh busybox sh -n. --- 600 ---
# 600 (23.10.2026, Claude, 541 kalibi): kamera_kayit v10 (denetim-2026-10-23-v2 #2/#3/#8/#12/#14/#15/#16: SD foto kuyrugu +
# kuyruk siniri, saat<2026 kayit yok, isaretli kayit gostergesi, foto zaman sirasi, bos kare uykusu, yarim kare kapatma,
# gunluk kirpma, ilk alarm). P: agac koku (vars. ~/f133/proj; worktree icin P=~/f133/wt). 541 ile AYNI salt-okuma/yazma
# denetimleri + --self-test (normal + ASAN/UBSAN) + durum JSON + RISC-V statik. kamera_kopru_sd_v12.sh busybox sh -n.
# Cikti ~/f133/out/kamera-kayit-v13/ ve ana klasor work/terminal-local/ota-068/kamera-kayit-v13/. CIHAZA GONDERMEZ.
# --- 541 ---
# 541 (22.10.2026, Claude): kamera_kayit v9 (scripts/540: time-lapse 1 sn, boyut siniri, olay tespiti); 458 ile AYNI
# denetimler + yeni oz-testler.
set -euo pipefail
P="${P:-$HOME/f133/proj}"
S="$P/initramfs/src/kamera_kayit_v13.c"
K="$P/sd/velinux/kamera_kopru_sd_v12.sh"
O="$HOME/f133/out/kamera-kayit-v13"
mkdir -p "$O"
# v5: kameraya YALNIZ saat komutu yazilir. Tam iki CDB kurucusu: uoku READ(10)=0x28 ve uyaz_saat WRITE(10)=0x2a;
# uyaz_saat ilk satirda dogrulama yapmali; 0x2a baska yerde gecmemeli; FORMAT/SIL/ISP/vendor izi yok.
[ "$(grep -c 'uint8_t cb\[10\] = {' "$S")" = 1 ] || { echo "HATA: READ CDB kurucusu sayisi"; exit 1; }
grep -q 'uint8_t cb\[10\] = { 0x28,' "$S" || { echo "HATA: CDB READ(10) degil"; exit 1; }
[ "$(grep -c 'uint8_t cw\[10\] = {' "$S")" = 1 ] || { echo "HATA: WRITE CDB kurucusu sayisi"; exit 1; }
[ "$(grep -c -i '0x2a' "$S")" = 1 ] || { echo "HATA: 0x2a birden fazla yerde"; exit 1; }
[ "$(grep -c 'ubulk(0x02' "$S")" = 3 ] || { echo "HATA: kameraya giden toplu aktarim sayisi (2 CBW + saat sektoru)"; exit 1; }	# 614
grep -A2 '^static int uyaz_saat' "$S" | grep -q 'if (!saat_sektoru_dogru(s) || !saat_lba_dogru(lba)) return -2;' || { echo "HATA: uyaz_saat dogrulamasi yok"; exit 1; }
[ "$(grep -c 'uyaz_saat(' "$S")" = 3 ] || { echo "HATA: uyaz_saat cagri sayisi (tanim + saat_esitle + self-test)"; exit 1; }
if grep -n -E 'cb\[0\] *=|WRITE\(10\)|USBDEVFS_CONTROL|0x55, *0xaa, *0x(1[147]|0[1-4]|46)' "$S" | grep -v '^\s*\*' | grep -v 'kirli\|reddedilir'; then
	echo "HATA: izinsiz kamera yazma izi"; exit 1
fi
# v10: /tmp foto kopyasi kalmadi
if grep -n '/tmp/kamera-%s' "$S"; then echo "HATA: /tmp foto kopyasi"; exit 1; fi
busybox sh -n "$K" && echo "sh -n tamam: ${K##*/}"
busybox sh -n "$P/sd/velinux/autorun_v48.sh" && echo "sh -n tamam: autorun_v48.sh"
W=(-Wall -Wextra -Werror -Wno-format-truncation)
gcc -O2 "${W[@]}" "$S" -o "$O/kamera-kayit-v13-host"
"$O/kamera-kayit-v13-host" --self-test
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all "${W[@]}" "$S" -o "$O/kamera-kayit-v13-asan"
"$O/kamera-kayit-v13-asan" --self-test
"$O/kamera-kayit-v13-host" --durum-ornek | python3 -m json.tool >/dev/null && echo "durum JSON gecerli"
riscv64-linux-gnu-gcc -static -O2 "${W[@]}" "$S" -o "$O/kamera-kayit-v13"
riscv64-linux-gnu-strip "$O/kamera-kayit-v13"
file "$O/kamera-kayit-v13"
sha256sum "$O/kamera-kayit-v13" | tee "$O/kamera-kayit-v13.sha256"
stat -c '%s bytes' "$O/kamera-kayit-v13"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kamera-kayit-v13"
mkdir -p "$H" && cp "$O/kamera-kayit-v13" "$O/kamera-kayit-v13.sha256" "$H/" && echo "kopyalandi: $H"
