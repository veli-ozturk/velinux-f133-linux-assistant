#!/bin/bash
# v10 (panel 0.14): kablosuz PC ekrani ortak bellek tuketicisi (pc_shm_v1.h, wifi-ekran-v4 ile) + tam ekran gorunum.
# Eski VFR1 dosya tuketicisi (pc_frame_v1.h, 41/44 betikleri) 0.14'te kullanilmaz; dosyalar tarihce olarak duruyor.
set -euo pipefail
P=~/f133/proj
UI=~/f133/ui
O=~/f133/out/panel
mkdir -p "$O" "$UI/src"
gcc -O2 -Wall -Wextra -Werror "$P/panel/tests/pc_shm_test.c" -o "$O/pc-shm-test"
"$O/pc-shm-test"
cp "$P/panel/pc_shm_v1.h" "$UI/src/pc_shm_v1.h"
bash "$P/scripts/35_build_panel_v5.sh"
grep -c "0.14 (11.10.2026)" "$O/panel" >/dev/null && echo "panel 0.14 ikilisi hazir"
