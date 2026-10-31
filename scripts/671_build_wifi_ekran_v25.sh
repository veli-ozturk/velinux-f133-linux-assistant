#!/bin/bash
# 671 (29.10.2026, Claude; docs §194): wifi_ekran_v25.c = v25 + CMA ara tampon yeniden denemesi. 665 kalibi.
# Host --self-test + 271 dosya testleri aynen, RISC-V statik. Cikti ~/f133/out/dogrudan-g2d-v25 ve ana klasor
# work/terminal-local/ota-068/wifi-ekran-v25/. CIHAZA GONDERMEZ.
set -euo pipefail
P="${P:-$HOME/f133/proj}"	# P: baska agac (ana klasor baska daldayken worktree)
O="$HOME/f133/out/dogrudan-g2d-v25"
F="$HOME/f133/out/wifi-ekran/tjpgd-hizli/a/b/tjpgd"
S="$HOME/f133/out/wifi-jpeg/ornek"
mkdir -p "$O"
A=(-O2 -Wall -Wextra -Werror -DLV_CONF_SKIP -DLV_USE_TJPGD=1 -I"$F" -I"$P/panel")
SRC=("$P/initramfs/src/wifi_ekran_v25.c" "$F/tjpgd.c")
gcc "${A[@]}" "${SRC[@]}" -o "$O/wifi-ekran-host-v25"
riscv64-linux-gnu-gcc -static "${A[@]}" "${SRC[@]}" -o "$O/wifi-ekran-v25"
riscv64-linux-gnu-strip "$O/wifi-ekran-v25"
X="$O/wifi-ekran-host-v25"
"$X" --self-test
R=$("$X" --file "$S/red-1600x600.jpg"); echo "$R"; grep -q 'status=0 w=1600 h=600 ilk=f800' <<< "$R"
"$X" --file "$S/parca-256x128-q90-444.jpg" --x 1344 --y 472 | grep -q 'status=0 w=256 h=128'
"$X" --file "$S/parca-256x128-q90-444.jpg" --x 1088 --y 344 --olcek2 | grep -q 'status=0 w=512 h=256'
for bad in '--x 1345 --y 0' '--x 0 --y 473' '--x 1089 --y 0 --olcek2'; do
  if "$X" --file "$S/parca-256x128-q90-444.jpg" $bad >/dev/null; then echo 'Bounds FAIL'; exit 1; fi
done
for f in over-1601x600 prog; do
  if "$X" --file "$S/$f.jpg" >/dev/null; then echo 'Invalid JPEG accepted'; exit 1; fi
done
grep -a -q 'ara tampon: ayrilamadi' "$O/wifi-ekran-host-v25" || { echo 'v25 yeniden deneme yok'; exit 1; }
grep -a -q 'yon: %s' "$O/wifi-ekran-host-v25" || { echo '180 destegi yok'; exit 1; }
echo 'PASS host: self-test, RGB565 red, true444/scale2, bounds and progressive rejection.'
file "$O/wifi-ekran-v25"
sha256sum "$O/wifi-ekran-v25" | tee "$O/wifi-ekran-v25.sha256"
stat -c 'bytes=%s' "$O/wifi-ekran-v25"
H="${VELINUX_ROOT:-$HOME/velinux}/work/terminal-local/ota-068/wifi-ekran-v25"
mkdir -p "$H" && cp "$O/wifi-ekran-v25" "$H/" && echo "kopyalandi: $H"
