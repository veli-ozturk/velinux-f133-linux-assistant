#!/bin/sh
# yuk_olc_v1 (22.10.2026, Claude; Sira 15 optimizasyon): N sn boyunca surec basina CPU (tik), toplam CPU dagilimi, takas
# ve buyuk sayfa hatasi hizi, baglam degisimi. Salt okuma.   sh /mnt/sd/velinux/yuk_olc_v1.sh [sn=60]
N=${1:-60}
snap() {
	for p in /proc/[0-9]*; do
		[ -r $p/stat ] || continue
		set -- $(sed 's/([^)]*)/X/' $p/stat 2>/dev/null)
		[ -n "$1" ] && echo "$1 $((${14} + ${15})) $(cat $p/comm 2>/dev/null)"
	done
}
vm() { grep -E '^(pswpin|pswpout|pgmajfault) ' /proc/vmstat | tr '\n' ' '; grep '^ctxt' /proc/stat; }
snap > /tmp/yo-c1; head -1 /proc/stat > /tmp/yo-s1; vm > /tmp/yo-v1
sleep $N
snap > /tmp/yo-c2; head -1 /proc/stat > /tmp/yo-s2; vm > /tmp/yo-v2
echo "== $N sn, surec basina CPU tiki (100/sn)"
awk 'NR==FNR{a[$1]=$2;next}{d=$2-a[$1]; if(d>0) printf "%5d %s %s\n", d, $1, $3}' /tmp/yo-c1 /tmp/yo-c2 | sort -rn | head -12
paste /tmp/yo-s1 /tmp/yo-s2 | awk '{u=$13-$2; n=$14-$3; s=$15-$4; i=$16-$5; w=$17-$6; q=$18-$7+$19-$8; t=u+n+s+i+w+q
	printf "cpu: kull %.0f%% sis %.0f%% bos %.0f%% iobek %.0f%% irq %.0f%%\n", 100*u/t, 100*s/t, 100*i/t, 100*w/t, 100*q/t}'
paste /tmp/yo-v1 /tmp/yo-v2 | awk -v n=$N '{printf "takas giris %.0f/sn cikis %.0f/sn buyuk hata %.0f/sn ctxt %.0f/sn\n", ($10-$2)/n, ($12-$4)/n, ($14-$6)/n, ($16-$8)/n}'
cat /proc/loadavg
rm -f /tmp/yo-c1 /tmp/yo-c2 /tmp/yo-s1 /tmp/yo-s2 /tmp/yo-v1 /tmp/yo-v2
