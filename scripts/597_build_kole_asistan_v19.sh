#!/bin/bash
# 597 (23.10.2026, Claude, 567 kalibi): kole-asistan v19 (kalkanda soru iptali; denetim bulgu 5). R: kaynak agaci
# (ana klasor baska daldayken worktree). Cikti ana klasor work/terminal-local/ota-068/kole-asistan-v19/ de. --- 567 ---
# 567 (23.10.2026, Claude, 493 kalibi): kole-asistan v18 (scripts/566: kole-gemini v40 soru soketi). --- 493 ---
# 493 (21.10.2026, Claude, 484 kalibi): kole-asistan v17 (scripts/492: sure kapisi). --- 484 ---
# 484 (21.10.2026, Claude, 475 kalibi): kole-asistan v16 (scripts/483: azalt_v2.h). Once azalt_kiyas_v1 (normal + ASAN)
# ve uyandirma_kiyas_v3: v15 zinciri azalt v1 ile v2 dokumu BIT BIT ayni (patlama 120 sn + sessiz 300 sn), sure kiyasi.
# --- 475 ---
# 475 (21.10.2026, Claude, 473 kalibi): kole-asistan v15 (scripts/474: ayni icerikli kole-gemini-v19 ortak ikili).
# --- 473 ---
# 473 (21.10.2026, Claude, 431 kalibi): kole-asistan v14 (scripts/472: tembel kepstrum). Once uyandirma_kiyas_v2:
# v13 (v5) ile v14 (v6) enerji + gurultu + parca kepstrumu BIT BIT ayni (patlama 120 sn + sessiz 300 sn), sure kiyasi.
# --- 431 ---
# 431 (21.10.2026, Claude, 399 kalibi): kole-asistan v13 (scripts/430: CPU — pencere/FIR halka tamponu, seyrek mel, poll).
# Once uyandirma_kiyas_v1: v12 zinciri (v4 + memmove azalt) ile v13 zinciri (v5 + azalt_v1) dokumu BIT BIT ayni olmali.
# --- 399 --- kole-asistan v12 (scripts/398: uyarlanir konusma sonu, kayit enerjisi gunlugu, kendini guncelleme).
# Cihaza "kole-asistan-v8" adiyla OTA. --- 370 ---
# 370: kole-asistan v11 (scripts/369: cevap olayinda soru). --- 360 ---
# 360 (20.10.2026, Claude, 352 kalibi): kole-asistan v10 (scripts/359: sabit cumleler sesli, kole-gemini v15 hazir sesleri).
# Cihaza "kole-asistan-v8" adiyla OTA. Host: uyandirma_test_v2 normal + ASAN/UBSAN, kole-asistan --self-test (normal + ASAN),
# 361 capraz denetim (asistan cumleleri == gemini v15 hazirCumleler, FNV C == Python); sonra RISC-V statik.
# Cikti ~/f133/out/kole-asistan-v12/. CIHAZA GONDERMEZ. --- 352 ---
set -euo pipefail
R="${R:-$HOME/f133/proj}"
P="$R/initramfs/src"
O="$HOME/f133/out/kole-asistan-v19"
mkdir -p "$O"
A=(-O2 -Wall -Wextra -Werror)
gcc "${A[@]}" "$P/tests/azalt_kiyas_v1.c" -lm -o "$O/azalt-kiyas"
"$O/azalt-kiyas"
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror "$P/tests/azalt_kiyas_v1.c" -lm \
    -o "$O/azalt-kiyas-asan"
"$O/azalt-kiyas-asan" 3000000 | tail -1 | sed 's/^/[asan] /'
for z in 1 2; do gcc "${A[@]}" -DSURUM=6 -DAZALT=$z "$P/tests/uyandirma_kiyas_v3.c" -lm -o "$O/kiyas-a$z"; done
for kip in "patlama 120" "sessiz 300"; do
	set -- $kip
	"$O/kiyas-a1" "$O/dokum-a1-$1.bin" "$2" "$1"
	"$O/kiyas-a2" "$O/dokum-a2-$1.bin" "$2" "$1"
	cmp "$O/dokum-a1-$1.bin" "$O/dokum-a2-$1.bin" && echo "KIYAS ($1): v15 zinciri azalt v1 ve v2 enerji + gurultu + parca kepstrumu BIT BIT ayni"
done
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror -DSURUM=6 -DAZALT=2 "$P/tests/uyandirma_kiyas_v3.c" -lm     -o "$O/kiyas-v6-asan"
"$O/kiyas-v6-asan" "$O/dokum-v6-asan.bin" 20 | sed 's/^/[asan] /'
gcc "${A[@]}" "$P/tests/uyandirma_test_v2.c" -lm -o "$O/uyandirma-test"
"$O/uyandirma-test" | tail -1
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror "$P/tests/uyandirma_test_v2.c" -lm -o "$O/uyandirma-test-asan"
"$O/uyandirma-test-asan" | tail -1 | sed 's/^/[asan] /'
gcc "${A[@]}" "$P/kole_asistan_v19.c" -lm -o "$O/kole-asistan-host"
"$O/kole-asistan-host" --self-test | tee "$O/self-test.txt"
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror "$P/kole_asistan_v19.c" -lm \
    -o "$O/kole-asistan-host-asan"
"$O/kole-asistan-host-asan" --self-test | sed 's/^/[asan] /'
python3 "$R/scripts/361_hazir_ses_capraz_denetim_v1.py" "$P/kole_asistan_v19.c" \
    "$R/go/kole-gemini-v15/hazir.go" "$O/self-test.txt"
riscv64-linux-gnu-gcc -static "${A[@]}" "$P/kole_asistan_v19.c" -lm -o "$O/kole-asistan-v19"
riscv64-linux-gnu-strip "$O/kole-asistan-v19"
file "$O/kole-asistan-v19"
sha256sum "$O/kole-asistan-v19" | tee "$O/kole-asistan-v19.sha256"
stat -c 'bytes=%s' "$O/kole-asistan-v19"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kole-asistan-v19"
mkdir -p "$H" && cp "$O/kole-asistan-v19" "$H/" && echo "kopyalandi: $H"
