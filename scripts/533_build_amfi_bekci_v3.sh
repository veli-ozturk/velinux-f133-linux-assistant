#!/bin/bash
# 533 (22.10.2026, Claude, 332 kalibi): amfi-bekci v3 (BT muzigi -> A7 0b 00 kaynak, BT IS -> AT#MM ad; madde 116). --- 332 ---
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SRC="$ROOT/initramfs/src/amfi_bekci_v3.c"
OUT="$HOME/f133/out/amfi-bekci-v3"
mkdir -p "$OUT"
gcc -O2 -Wall -Wextra -Werror "$SRC" -o "$OUT/amfi-host"
"$OUT/amfi-host" --self-test
gcc -O1 -g -fsanitize=address,undefined -Wall -Wextra -Werror "$SRC" -o "$OUT/amfi-asan"
"$OUT/amfi-asan" --self-test
riscv64-linux-gnu-gcc -static -O2 -Wall -Wextra -Werror "$SRC" -o "$OUT/amfi-bekci-v3"
sha256sum "$OUT/amfi-bekci-v3"
