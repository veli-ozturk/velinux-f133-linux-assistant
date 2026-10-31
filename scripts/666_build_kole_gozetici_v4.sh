#!/bin/bash
# 666 (26.10.2026, Claude): kole-gozetici-v4 (host self-test normal + ASAN, RISC-V, ota-068). 631 kalibi.
set -euo pipefail
P="${P:-$HOME/f133/proj}"
O="$HOME/f133/out/kole-gozetici-v4"
mkdir -p "$O"
S="$P/initramfs/src/kole_gozetici_v4.c"
A=(-O2 -Wall -Wextra -Werror)
gcc "${A[@]}" "$S" -o "$O/kole-gozetici-v4-host"
"$O/kole-gozetici-v4-host" --self-test
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror "$S" -o "$O/kole-gozetici-v4-asan"
"$O/kole-gozetici-v4-asan" --self-test | sed 's/^/[asan] /'
riscv64-linux-gnu-gcc -static "${A[@]}" "$S" -o "$O/kole-gozetici-v4"
riscv64-linux-gnu-strip "$O/kole-gozetici-v4"
sha256sum "$O/kole-gozetici-v4" | tee "$O/kole-gozetici-v4.sha256"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/kole-gozetici-v4"
mkdir -p "$H" && cp "$O/kole-gozetici-v4" "$O/kole-gozetici-v4.sha256" "$H/"
echo "666 TAMAM"
