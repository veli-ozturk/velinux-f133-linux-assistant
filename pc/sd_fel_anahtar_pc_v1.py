"""sd_fel_anahtar_pc_v1 (29.10.2026, Claude; flash #48 acilmadi, kurtarma): SD kart PC'deyken FEL anahtarini dogrudan
diske yazar (cihaz kapali oldugu icin seri yol yok). YONETICI olarak calistirilir.
  python -X utf8 work/sd_fel_anahtar_pc_v1.py <disk_no> yaz|sil
Kapilar: disk USB + MBR, ilk bolum 2048. sektorden baslar, sektor 16..47 (8..24 KB) yazmadan once bos (ya da zaten anahtar).
Once ilk 1 MB outputs/ altina yedeklenir. 'sil': ayni alan sifirlanir (anahtar yalniz kurtarma icin; sonra silinmeli).
"""
import hashlib
import os
import struct
import sys
import time

W = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(W)
ANAHTAR = os.path.join(ROOT, '..', '..', '..', 'outputs', 'sd_fel_switcher.bin')
SHA = '591be84fcb388f376eff7846edc136d454942cece6fb43c328a37205f154f424'
BAS, UZ = 16 * 512, 32 * 512


def main():
    no, kip = int(sys.argv[1]), sys.argv[2]
    assert kip in ('yaz', 'sil')
    a = open(ANAHTAR, 'rb').read()
    if hashlib.sha256(a).hexdigest() != SHA or len(a) != UZ:
        sys.exit('HATA: anahtar dosyasi beklenen degil')
    yol = r'\\.\PhysicalDrive%d' % no
    with open(yol, 'rb', buffering=0) as f:
        ilk = f.read(1 << 20)
    if len(ilk) != 1 << 20 or ilk[510:512] != b'\x55\xaa' or struct.unpack_from('<I', ilk, 454)[0] != 2048:
        sys.exit('HATA: MBR ya da ilk bolum baslangici (2048) beklenen degil; yazilmadi')
    eski = ilk[BAS:BAS + UZ]
    hedef = a if kip == 'yaz' else bytes(UZ)
    if eski == hedef:
        print('zaten istenen durumda; yazilmadi')
        return
    if kip == 'yaz' and eski != bytes(UZ):
        sys.exit('HATA: sektor 16..47 bos degil; yazilmadi')
    y = os.path.join(ROOT, 'outputs', 'sd-ilk-1MB-pc-%s.bin' % time.strftime('%Y%m%d-%H%M%S'))
    os.makedirs(os.path.dirname(y), exist_ok=True)
    open(y, 'wb').write(ilk)
    print('yedek', y, hashlib.sha256(ilk).hexdigest()[:8])
    with open(yol, 'r+b', buffering=0) as f:
        f.seek(BAS)	# sektor hizali (8 KB), MBR'ye dokunulmaz
        f.write(hedef)
    with open(yol, 'rb', buffering=0) as f:
        son = f.read(1 << 20)
    beklenen = ilk[:BAS] + hedef + ilk[BAS + UZ:]
    print('DOGRULAMA', 'TAMAM' if son == beklenen else 'HATA', kip)
    sys.exit(0 if son == beklenen else 1)


if __name__ == '__main__':
    try:
        main()
    finally:
        if '--bekle' in sys.argv:
            input('Enter ile kapat')
