#!/bin/bash
# mcu-servis v6 (A3+A4 korumasi): host self-test + ASAN + statik RISC-V. Kurulum YAPMAZ.
set -euo pipefail
SRC="$(cd "$(dirname "$0")/.." && pwd)/initramfs/src/mcu_servis_v6.c"
OUT="$HOME/f133/out/mcu-servis-v6"
mkdir -p "$OUT"
gcc -O2 -Wall -Wextra -Werror "$SRC" -o "$OUT/mcu-servis-v6-host"
"$OUT/mcu-servis-v6-host" --self-test
gcc -O1 -g -fsanitize=address,undefined -Wall -Wextra "$SRC" -o "$OUT/mcu-servis-v6-asan"
"$OUT/mcu-servis-v6-asan" --self-test
riscv64-linux-gnu-gcc -static -O2 -Wall -Wextra -Werror "$SRC" -o "$OUT/mcu-servis-v6"
file "$OUT/mcu-servis-v6"
sha256sum "$OUT/mcu-servis-v6"
