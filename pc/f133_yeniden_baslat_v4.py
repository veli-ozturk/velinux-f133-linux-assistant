"""f133_yeniden_baslat_v4 (26.10.2026, Claude; docs §184): v3 + ACILISTA COM5 YOKLANMAZ. v3 ile 3 kez (25-26.10)
ilk acilis ~3 dk sonra bir kez daha sifirlandi; Merkez ile ve COM5'e 5.7 dk dokunmadan (26.10 12:56) sifirlanmadi.
v4: once IP okunur, reboot sonrasi acilis yalniz ping (TTL=) ile izlenir, COM5'e en erken 150 sn sonra bir kez baglanilir.
--- v3 ---
f133_yeniden_baslat_v3 (23.10.2026, Claude; scripts/582, docs bolum 136): v2 + yeniden baslatma SD'deki
yeniden_baslat_sd_v1.sh ile (uyanik MCU'nun B2 kalbine HIZALI; isareti o siler). Acilistaki "Wi-Fi guc cokmesi" aslinda
MCU kalp sifirlamasiydi; autorun_v42 mcu-servis'i ~6. sn'de baslatir. Yardimci SD'de yoksa v2 gibi satir ici reboot -f.
--- v2 ---
f133_yeniden_baslat_v2 (19.10.2026, Claude): v1 + YANLIS POZITIF DUZELTMESI. v1 "inet " ve dmesg sayisini ham
ciktida ariyordu; kabugun yansittigi KOMUT SATIRI da "inet" icerdigi icin Wi-Fi gelmeden PASS diyebiliyordu (17:41,
o seferki Wi-Fi ayrica dogru isaretlerle dogrulandi). v2 cihazda IP=, PANEL=, COKMUS= satirlari uretir ve yalniz satir
basinda arar.
f133_yeniden_baslat_v1: cihazi BILEREK yazilimla yeniden baslatir (OTONOM 14). wifi-baslat acilista /data/.wifi-deneme
isaretini gorurse onceki Wi-Fi denemesini "guc cokmesi" sayar ve o acilista Wi-Fi'yi ATLAR. Isaret Wi-Fi 30 sn sorunsuz
calisinca silinir; o 30 sn icinde yapilan reboot -f isareti yerinde birakir. Bu arac reboot'tan once isareti siler.
  --kuru : yalniz durum; yeniden baslatmaz
  --evet : rm -f /data/.wifi-deneme; sync; reboot -f -> COM5 yeni acilis (uptime < 300) -> Wi-Fi IP (en cok 90 sn)
           -> kmsg'de "denemesi cokmus" OLMAMALI.
Tek COM5 sahibi, sinirli yazma (write_timeout 3). Flash'a/SD'ye yazmaz, MCU'ya komut gondermez.
"""
import argparse
import re
import subprocess
import sys
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import f133_seri_v5 as seri  # noqa: E402

_k = None
DURUM = ("echo; cat /proc/uptime; ls /data/.wifi-deneme 2>&1; "
         "ip -4 addr show wlan0 | sed -n 's/.*inet \\([0-9.]*\\).*/IP=\\1/p'; pidof panel | sed 's/^/PANEL=/'; "
         "grep MemAvailable /proc/meminfo; dmesg | grep -c 'denemesi cokmus' | sed 's/^/COKMUS=/'")


def satir(out, onek):
    """cihazin urettigi 'ONEK=deger' satiri; komut yankisinda 'ONEK=' satir basinda gecmez"""
    m = re.search(r'(?m)^' + onek + r'=(\S*)\s*$', out.replace('\r', ''))
    return m.group(1) if m else None


def kabuk(cmd, wait=3):
    global _k
    if _k is None:
        asil = seri.old.serial.Serial

        def sinirli(*a, **kw):
            kw['write_timeout'] = 3
            return asil(*a, **kw)
        seri.old.serial.Serial = sinirli
        try:
            _k = seri.Kabuk('COM5')
        finally:
            seri.old.serial.Serial = asil
    out = _k.run(cmd, wait)
    print(out, flush=True)
    return out


def kapat():
    global _k
    if _k is not None:
        try:
            _k.s.close()
        finally:
            _k = None


def uptime(out):
    m = re.search(r'(?m)^(\d+)\.\d+ ', out)
    return int(m.group(1)) if m else None


def yeniden():
    Y = '/mnt/sd/velinux/yeniden_baslat_sd_v1.sh'	# v3: B2'ye hizali
    ip = satir(kabuk(DURUM), 'IP')	# v4: acilisi agdan izlemek icin
    if not ip:
        raise RuntimeError('cihazin IP\'si okunamadi; yeniden baslatilmadi')
    out = kabuk(f'echo; if [ -f {Y} ]; then sh {Y} </dev/null >/dev/null 2>&1 & else rm -f /data/.wifi-deneme; sync; '
                '(sleep 2; reboot -f) </dev/null >/dev/null 2>&1 & fi; echo "YENIDEN""_BASLIYOR"')
    if not re.search(r'(?m)^YENIDEN_BASLIYOR\s*$', out):
        raise RuntimeError('yeniden baslatma komutu calismadi')
    kapat()
    t0 = time.monotonic()
    dustu = geldi = None
    while time.monotonic() - t0 < 240:	# v4: yalniz ping; COM5'e dokunma
        var = b'TTL=' in subprocess.run(['ping', '-n', '1', '-w', '1500', ip], capture_output=True).stdout
        sn = int(time.monotonic() - t0)
        if not var and dustu is None:
            dustu = sn
            print(f'{sn} sn: cihaz agdan dustu', flush=True)
        if var and dustu is not None and geldi is None:
            geldi = sn
            print(f'{sn} sn: cihaz aga dondu', flush=True)
        if geldi is not None and sn >= 150:
            break
        time.sleep(5)
    print(f'{int(time.monotonic() - t0)} sn: COM5 okunuyor (dustu={dustu}, geldi={geldi})', flush=True)
    u = uptime(kabuk('echo; cat /proc/uptime'))
    if u is None or u >= 300:
        raise RuntimeError(f'yeni acilis gorulmedi (uptime {u}); GUCU KESMEDEN once f133_ram_dmesg_v1.py dusun')
    print(f'yeni acilis: uptime {u} sn ({int(time.monotonic() - t0)} sn sonra)', flush=True)
    for _ in range(18):
        out = kabuk(DURUM)
        if satir(out, 'IP'):
            break
        time.sleep(5)
    sorun = []
    if not satir(out, 'IP'):
        sorun.append('Wi-Fi IP 90 sn icinde gelmedi')
    if satir(out, 'COKMUS') is None:
        sorun.append('COKMUS satiri okunamadi')
    elif satir(out, 'COKMUS') != '0':
        sorun.append('wifi-baslat "onceki deneme cokmus" dedi (isaret silinemedi mi?)')
    print(f"IP={satir(out, 'IP')} PANEL={satir(out, 'PANEL')} COKMUS={satir(out, 'COKMUS')}", flush=True)
    print('SORUNLAR:', sorun or 'yok', flush=True)
    if sorun:
        raise RuntimeError('; '.join(sorun))
    print('PASS: yeniden acildi, Wi-Fi geldi, guc korumasi yanlis alarm vermedi', flush=True)


if __name__ == '__main__':
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    g = ap.add_mutually_exclusive_group(required=True)
    g.add_argument('--kuru', action='store_true')
    g.add_argument('--evet', action='store_true')
    a = ap.parse_args()
    try:
        if a.kuru:
            out = kabuk(DURUM)
            print(f"KURU: IP={satir(out, 'IP')} PANEL={satir(out, 'PANEL')} COKMUS={satir(out, 'COKMUS')}; yeniden baslatilmadi",
                  flush=True)
        else:
            yeniden()
    finally:
        kapat()
