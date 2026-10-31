#!/bin/bash
# 585 (23.10.2026, Claude; Sira 18 madde 135): kole-tlstest-v1 (kole_https_v1.h + BearSSL 0.6, scripts/584). Host --self-test
# (normal + ASAN), host canli deneme (WSL'den api.telegram.org), RISC-V statik. Cikti ~/f133/out/kole-tlstest-v1/ ve ana
# klasor work/terminal-local/ota-068/kole-tlstest-v1/. CIHAZA GONDERMEZ.
set -euo pipefail
P="$HOME/f133/proj"
B="$HOME/f133/out/bearssl-0.6"
I=(-I"$HOME/f133/bearssl-kaynak/bearssl-0.6/inc" -I"$B")
O="$HOME/f133/out/kole-tlstest-v1"
mkdir -p "$O"
A=(-O2 -Wall -Wextra -Werror)
S="$P/initramfs/src/kole_tlstest_v1.c"
gcc "${A[@]}" "${I[@]}" "$S" "$B/libbearssl-host.a" -o "$O/kole-tlstest-host"
"$O/kole-tlstest-host" --self-test
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror "${I[@]}" "$S" "$B/libbearssl-host.a" \
	-o "$O/kole-tlstest-host-asan"
"$O/kole-tlstest-host-asan" --self-test | sed 's/^/[asan] /'
echo "--- host canli (WSL):"
"$O/kole-tlstest-host-asan" api.telegram.org / | sed 's/^/[asan canli] /' || echo "(canli deneme basarisiz: ag?)"
riscv64-linux-gnu-gcc -static -Os -ffunction-sections -fdata-sections -Wl,--gc-sections -Wall -Wextra -Werror "${I[@]}" "$S" \
	"$B/libbearssl-riscv.a" -o "$O/kole-tlstest-v1"
riscv64-linux-gnu-strip "$O/kole-tlstest-v1"
file "$O/kole-tlstest-v1"
sha256sum "$O/kole-tlstest-v1" | tee "$O/kole-tlstest-v1.sha256"
stat -c 'bytes=%s' "$O/kole-tlstest-v1"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kole-tlstest-v1"
mkdir -p "$H" && cp "$O/kole-tlstest-v1" "$H/" && echo "kopyalandi: $H"
