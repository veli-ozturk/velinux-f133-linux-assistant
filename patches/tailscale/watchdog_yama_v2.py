"""Tailscale v1.104.1 ipn/ipnlocal/watchdog.go: kilit bekcisi panik yerine kisa gunluk (VeLinux, 14.10.2026)."""
import sys
from pathlib import Path
p = Path(sys.argv[1]) / 'ipn/ipnlocal/watchdog.go'
s = p.read_text()
eski = ('\tlogf("ipnlocal watchdog goroutine stacks:\\n%s", goroutineStacks())\n'
        '\tpanic("ipnlocal: watchdog timeout")\n')
yeni = ('\t// VeLinux KOLE (14.10.2026): tek cekirdekli, takas kullanan 64 MB cihazda kilitler 30 sn\'yi asabiliyor;\n'
        '\t// bekci sureci oldurmesin, yalniz kisa bir satir yazsin (256 KB goroutine dokumu SD\'ye yazilmasin).\n'
        '\tlogf("ipnlocal watchdog: kilit 30 sn\'den uzun bekledi (yavas cihaz); panik devre disi (VeLinux yamasi)")\n'
        '\t_ = goroutineStacks\n')
if yeni in s:
    print('yama zaten uygulanmis'); sys.exit(0)
assert s.count(eski) == 1, 'beklenen satirlar bulunamadi'
p.write_text(s.replace(eski, yeni))
print('yama uygulandi')
