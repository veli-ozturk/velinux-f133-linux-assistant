"""fel_flash_linux_v7_test (24.10.2026, Claude): v7 (+ v6) denetim düzeltmeleri sahte xfel ile (donanım yok, flash'a dokunmaz).
    python -X utf8 work/fel_flash_linux_v7_test.py"""
import os
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import fel_flash_linux_v7 as f  # noqa: E402

t = tempfile.mkdtemp()
f.TMP, f.OUTD = t, t
f.fel_bekle = lambda: None
f.FEL_TEKRAR_SN = 0


def hata_mi(fn):
    try:
        fn()
    except SystemExit as e:
        return str(e)
    return None


# 1) xfel 0 döner ama dosya yazmaz: eski doğru boyutlu dosya "okundu" sayılmamalı
eski = os.path.join(t, "baslik0-1.bin")
open(eski, "wb").write(b"\0" * 0x1000)
f.xf = lambda args, timeout=1800: (0, "", 0.1)
assert "spinor read" in (hata_mi(lambda: f.oku(0x10000, 0x1000, eski)) or ""), "eski dosya okundu sayildi"
assert not os.path.exists(eski)


# 2) sıfır görüntü yedek olarak kaydedilmez
def sahte_oku(args, timeout=1800):
    if args[:2] == ["spinor", "read"]:
        open(args[4], "wb").write(b"\0" * int(args[3], 16))
    return 0, "", 0.1


f.xf = sahte_oku
f.FLASH = 0x2000  # test: küçük görüntü
m = hata_mi(f.yedek) or ""
assert "boot0 imzasi yok" in m and not [x for x in os.listdir(t) if x.startswith("FEL-flash-yedek")], m

# 3) govdesiz / hizasız toc1 reddedilir (assert değil, -O ile de)
open(os.path.join(t, "FEL-flash-yedek-x.bin"), "wb").write(b"\0" * f.FLASH)
f.FLASH = os.path.getsize(os.path.join(t, "FEL-flash-yedek-x.bin"))
f.FL = t
open(os.path.join(t, "toc1.bin"), "wb").write(b"sunxi-package" + b"\0" * (0x1000 - 13))
assert "govdesiz" in (hata_mi(f.yaz) or "")
open(os.path.join(t, "toc1.bin"), "wb").write(b"sunxi-package" + b"\0" * 100)
assert "hizali" in (hata_mi(f.yaz) or "")
# v7: --dizin yoksa yazilmaz
f.FL = None
assert "--dizin" in (hata_mi(f.yaz) or "")
f.FL = t
# v7: SHA256SUMS yok / tutmuyor -> yazilmaz; tutarsa FEL dusmusse yine yazilmaz (baslik sifirlanmadan)
gecerli = b"sunxi-package" + b"\0" * (0x2000 - 13)
open(os.path.join(t, "toc1.bin"), "wb").write(gecerli)
if os.path.exists(os.path.join(t, "SHA256SUMS")):
    os.remove(os.path.join(t, "SHA256SUMS"))
assert "SHA256SUMS" in (hata_mi(f.yaz) or "")
open(os.path.join(t, "SHA256SUMS"), "w").write("0" * 64 + "  toc1.bin\n")
assert "SHA256SUMS" in (hata_mi(f.yaz) or "")
import hashlib  # noqa: E402
open(os.path.join(t, "SHA256SUMS"), "w").write(hashlib.sha256(gecerli).hexdigest() + "  toc1.bin\n")
yazildi = []


def fel_yok(args, timeout=1800):
    if args[:2] == ["spinor", "write"]:
        yazildi.append(args)
    return (1, "", 0.1) if args == ["version"] else (0, "", 0.1)


f.xf = fel_yok
assert "FEL" in (hata_mi(f.yaz) or "") and not yazildi, yazildi
print("fel_flash_linux_v7_test: OK (eski dosya, sifir yedek, govdesiz/hizasiz toc1; v7: --dizin zorunlu, SHA256SUMS, FEL dustu -> yazma yok)")
