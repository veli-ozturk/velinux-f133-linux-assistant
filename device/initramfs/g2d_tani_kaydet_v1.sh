#!/bin/sh
# Numeric/status/text diagnostics only. Never read PC frame SHM or Wi-Fi secrets.
sleep 10
i=0
while [ "$i" -lt 9 ]; do
    D=/tmp/g2d-tani-v2
    mountpoint -q /mnt/sd && D=/mnt/sd/velinux/g2d-tani-v2
    mkdir -p "$D" || exit 1
    dmesg > "$D/dmesg-$i.txt"
    {
        echo "snapshot=$i"
        cat /proc/uptime /proc/meminfo /proc/swaps
        cat /sys/block/zram0/mm_stat 2>/dev/null
        ip -4 addr show wlan0
        cat /tmp/pc-ekran-durum.json /tmp/zram.log 2>/dev/null
        cat /tmp/panel.log /tmp/init.log 2>/dev/null
        ps
    } > "$D/status-$i.txt" 2>&1
    sync
    i=$((i+1))
    sleep 15
done
