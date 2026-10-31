#!/bin/bash
# 574 (23.10.2026, Claude; Sira 18 madde 134): kole-wdt-v1 (scripts/573). Host --self-test (normal + ASAN), busybox sh -n
# autorun_v38, RISC-V statik. Cikti ~/f133/out/kole-wdt-v1/. CIHAZA GONDERMEZ.
set -euo pipefail
P="$HOME/f133/proj"
O="$HOME/f133/out/kole-wdt-v1"
mkdir -p "$O"
A=(-O2 -Wall -Wextra -Werror)
gcc "${A[@]}" "$P/initramfs/src/kole_wdt_v1.c" -o "$O/kole-wdt-host"
"$O/kole-wdt-host" --self-test
gcc -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all -Wall -Wextra -Werror "$P/initramfs/src/kole_wdt_v1.c" \
    -o "$O/kole-wdt-host-asan"
"$O/kole-wdt-host-asan" --self-test | sed 's/^/[asan] /'
busybox sh -n "$P/sd/velinux/autorun_v38.sh" && echo "autorun_v38 sh -n OK"
riscv64-linux-gnu-gcc -static "${A[@]}" "$P/initramfs/src/kole_wdt_v1.c" -o "$O/kole-wdt-v1"
riscv64-linux-gnu-strip "$O/kole-wdt-v1"
file "$O/kole-wdt-v1"
sha256sum "$O/kole-wdt-v1" | tee "$O/kole-wdt-v1.sha256"
stat -c 'bytes=%s' "$O/kole-wdt-v1"
