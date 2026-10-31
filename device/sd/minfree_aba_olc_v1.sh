#!/bin/sh
# minfree_aba_olc_v1 (15.10.2026, Claude): ikinci ekran takilma A/B/A olcumu. A: min_free_kbytes eski, B: 4096, C: eski.
# Her evre 120 sn; compact_stall/allocstall_normal/pgmajfault farki ve MemAvailable /tmp/minfree-aba.log'a. Sonunda eski deger.
L=/tmp/minfree-aba.log
E=$(cat /proc/sys/vm/min_free_kbytes)
V() { grep -E "^(compact_stall|allocstall_normal|pgmajfault) " /proc/vmstat | awk '{printf "%s ", $2}'; }
evre() {
	echo $2 > /proc/sys/vm/min_free_kbytes
	a=$(V); echo "$(date +%T) $1 basla min_free=$2 MemAvail=$(grep MemAvail /proc/meminfo | awk '{print $2}')" >> $L
	sleep 120
	b=$(V); echo "$(date +%T) $1 bitti once=[$a] sonra=[$b] MemAvail=$(grep MemAvail /proc/meminfo | awk '{print $2}')" >> $L
}
trap 'echo $E > /proc/sys/vm/min_free_kbytes' EXIT INT TERM
evre A $E
evre B 4096
evre C $E
echo "$(date +%T) TAMAM min_free=$(cat /proc/sys/vm/min_free_kbytes)" >> $L
