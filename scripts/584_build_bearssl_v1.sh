#!/bin/bash
# 584 (23.10.2026, Claude; Sira 18 madde 135, kullanıcı 23.10 "evet basla"): BearSSL 0.6 -> host + RISC-V statik kutuphane,
# brssl araci, gomulu kok sertifikalari. Kaynak: https://bearssl.org/bearssl-0.6.tar.gz (765094 B, sha256 asagida),
# ~/f133/dl/bearssl-indir/ (indirme dizini), acilmis kopya ~/f133/bearssl-kaynak/. Kokler: Ubuntu ca-certificates
# (/etc/ssl/certs/ca-certificates.crt, Mozilla) -> ta_kokler_v1.h (`brssl ta`). Cikti ~/f133/out/bearssl-0.6/. CIHAZA GONDERMEZ.
set -euo pipefail
T="$HOME/f133/dl/bearssl-indir/bearssl-0.6.tar.gz"
echo "6705bba1714961b41a728dfc5debbe348d2966c117649392f8c8139efc83ff14  $T" | sha256sum -c -
K="$HOME/f133/bearssl-kaynak/bearssl-0.6"
[ -d "$K" ] || { mkdir -p "$HOME/f133/bearssl-kaynak"; tar xzf "$T" -C "$HOME/f133/bearssl-kaynak"; }
O="$HOME/f133/out/bearssl-0.6"
rm -rf "$O/obj-riscv"
mkdir -p "$O/obj-riscv"
# host (Unix.mk: cc -Os): libbearssl.a + brssl
make -C "$K" -j8 lib tools > "$O/make-host.log" 2>&1 || { tail -20 "$O/make-host.log"; exit 1; }
cp "$K/build/libbearssl.a" "$O/libbearssl-host.a"
# RISC-V: tum src/**/*.c, -ffunction-sections (baglayici kullanilmayani atar)
n=0
while IFS= read -r f; do
	riscv64-linux-gnu-gcc -Os -ffunction-sections -fdata-sections -W -Wall -I"$K/inc" -I"$K/src" -c "$K/$f" \
		-o "$O/obj-riscv/$(echo "${f%.c}" | tr / _).o"
	n=$((n + 1))
done < <(cd "$K" && find src -name '*.c' | sort)
rm -f "$O/libbearssl-riscv.a"
riscv64-linux-gnu-ar rcs "$O/libbearssl-riscv.a" "$O"/obj-riscv/*.o
echo "RISC-V nesne: $n, $(stat -c %s "$O/libbearssl-riscv.a") B"
"$K/build/brssl" ta /etc/ssl/certs/ca-certificates.crt > "$O/ta_kokler_v1.h" 2> "$O/brssl-ta.log"
grep -E '^#define TAs_NUM' "$O/ta_kokler_v1.h"
sha256sum /etc/ssl/certs/ca-certificates.crt | cut -c1-16 | sed 's/^/kok paketi sha256 /'
echo "584 BearSSL: TAMAM ($O)"
