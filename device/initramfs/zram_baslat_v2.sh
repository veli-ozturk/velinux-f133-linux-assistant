#!/bin/sh
# v2: mem_limit is write-only in Tina; validate with mm_stat. RAM swap only.
set -eu
exec > /tmp/zram.log 2>&1
Z=/sys/block/zram0
[ -d "$Z" ] && [ -b /dev/zram0 ] || { echo 'zram0 unavailable'; exit 1; }
grep -q '^/dev/zram0 ' /proc/swaps && { echo 'zram already active'; exit 0; }
[ "$(cat "$Z/disksize")" = 0 ] || { echo 'zram already configured; no reset'; exit 1; }
echo lzo > "$Z/comp_algorithm"
echo 4194304 > "$Z/mem_limit"
echo 8388608 > "$Z/disksize"
cat "$Z/mm_stat"
awk '$4 != 4194304 { exit 1 }' "$Z/mm_stat"
mkswap /dev/zram0
swapon -p 100 /dev/zram0
echo 20 > /proc/sys/vm/swappiness
cat /proc/swaps "$Z/mm_stat"
echo '[zram] 8 MiB RAM swap, 4 MiB allocation cap, lzo, swappiness=20; mem_limit verified via mm_stat' > /dev/kmsg
