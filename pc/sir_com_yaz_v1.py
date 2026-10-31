"""sir_com_yaz_v1 (30.10.2026, Claude; /jp anahtari): PC'deki bir sir dosyasini COM5 kabugu uzerinden cihaza YAZDIRMADAN
yazar. Yankı kapatılır (stty -echo), cikti hic basilmaz; yalniz iki tarafin sha256 ilk 8 hanesi gosterilir.
    python -X utf8 work/sir_com_yaz_v1.py <yerel_dosya> <cihaz_yolu>
Cihaz yolu /data/asistan/ altinda olmali; dosya 0600 yazilir. Sir yalniz [A-Za-z0-9._~+/=-] karakterleri icermeli.
"""
import hashlib
import os
import re
import sys
import time

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), 'serial-python-libs'))
import serial


def oku_bekle(s, isaret, sure):
    b, son = b'', time.time() + sure
    while time.time() < son and isaret not in b:
        b += s.read(4096)
    return b


def main():
    yerel, hedef = sys.argv[1], sys.argv[2]
    if not re.fullmatch(r'/data/asistan/[A-Za-z0-9._-]+', hedef):
        sys.exit('HATA: hedef /data/asistan/ altinda olmali')
    sir = open(yerel, 'rb').read().strip()
    if not sir or not re.fullmatch(rb'[A-Za-z0-9._~+/=-]+', sir) or len(sir) > 512:
        sys.exit('HATA: sir bicimi beklenen degil')
    yerel_sha = hashlib.sha256(sir).hexdigest()[:8]
    s = serial.Serial('COM5', 115200, timeout=0.3)
    s.write(b'\x03\r\n')
    time.sleep(0.5)
    s.read(65536)
    s.write(b'stty -echo; umask 077; head -c %d > %s.yeni && mv %s.yeni %s; stty echo; echo SIR$(sha256sum %s | cut -c1-8)SON\n'	# yalniz \n: \r\n'de icrnl ikinci satir sonunu head'e verirdi
            % (len(sir), hedef.encode(), hedef.encode(), hedef.encode(), hedef.encode()))
    time.sleep(0.8)
    s.write(sir + b'\n')	# kanonik kip: satir sonu gelmeden head okumaz; fazla \n kabuga bos satir olur
    cevap = oku_bekle(s, b'SON', 10)
    s.close()
    m = re.findall(rb'SIR([0-9a-f]{8})SON', cevap)
    cihaz = m[-1].decode() if m else '?'
    print('yerel', yerel_sha, 'cihaz', cihaz, 'AYNI' if cihaz == yerel_sha else 'FARKLI')
    sys.exit(0 if cihaz == yerel_sha else 1)


if __name__ == '__main__':
    main()
