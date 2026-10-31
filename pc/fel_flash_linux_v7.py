# -*- coding: utf-8 -*-
"""
(v7, 24.10.2026 Claude, docs/denetim-2026-10-24-v3.md A17: --dizin ZORUNLU (varsayilan linux-build/flash 11.10
 tarihli eski paketti); toc1.bin <dizin>/SHA256SUMS ile karsilastirilir (yoksa ya da tutmazsa yazilmaz); baslik sifirlanmadan
 once FEL 15 sn sonra yeniden denetlenir (uyanik MCU ~10 sn'de FEL'i dusuruyor: amfi_bekci_sd_v*.sh .fel-bekle yapilmadiysa
 hicbir sey yazilmaz). Gerisi v6 ile ayni.)
(v6, 23.10.2026 Claude, docs/denetim-2026-10-23-v1.md bulgulari 1, 11, 19: oku() once eski gecici dosyayi siler (xfel hata
 verip 0 donerse eski dogru boyutlu dosya "okundu" sayiliyordu: eski sifir baslik kontrolu gecer, govde gecerli eski
 basligin altina yazilabilirdi); yedekte boot0 imzasi (eGON.BT0) yoksa yedek KAYDEDILMEZ; girdi denetimleri assert degil
 (python -O ile kaybolmasin) + toc1 en az 4 KB baslik + govde; sonda boot0 bozuksa TAMAM denmez. Gerisi v5 ile ayni.)
(v5, 14.10.2026 Claude: yedek ve dogrulama TEK okuma; geri kalan v4 ile ayni)
(v4: --dizin <klasor>) F133 SPI NOR'a Linux toc1 paketini yazar (boot0 0x0-0xFFFF ve UDISK 0xFE0000+ DOKUNULMAZ).
v3 (11.10.2026): yazmadan ONCE eski toc1 basligi sifirlanir -> ogeler yazilirken kopma olursa gecerli toc1 kalmaz,
    boot0 FEL'e duser (v2'de eski baslik yarim ogeleri gosterdi, cihaz ancak SD FEL kartiyla kurtarildi).
    Ayrica toc1'in data bolumune (0xC00000) tasmadigi denetlenir. v2: zaman damgali yedek, 'hepsi'.
  python work\\fel_flash_linux_v6.py yedek  -> 16 MB'i IKI KEZ oku, karsilastir, outputs\\ altina kaydet (salt okuma)
  python work\\fel_flash_linux_v6.py yaz|hepsi     -> once yedek sart (bugunun yedegi aranir), sonra:
       1) ogeler: toc1.bin[0x1000:] -> flash 0x11000 (yaz, iki kez geri oku, karsilastir)
       2) EN SON toc1 baslik sektoru: toc1.bin[0:0x1000] -> flash 0x10000 (yaz, iki kez geri oku)
     Yarida kalirsa 0x10000'de magic olmaz -> boot0 FEL'e doner -> yedekten geri yazilabilir.
Kurallar (CLAUDE.md): yazmadan once yedek, yazdiktan sonra cift okumayla dogrulama, checksum/baslik sektoru en son.
"""
import os, sys, subprocess, time, hashlib, datetime, glob

W = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(W)
XFEL = os.path.join(W, "tools", "xfel", "xfel.exe")
FL = None	# v7: --dizin zorunlu (yaz/hepsi)
if "--dizin" in sys.argv:
    FL = os.path.abspath(sys.argv[sys.argv.index("--dizin") + 1])
FEL_TEKRAR_SN = 15	# v7: uyanik MCU FEL'i ~10 sn'de dusuruyor
OUTD = os.path.join(ROOT, "outputs")
TMP = os.path.join(OUTD, "linux-ram-test", "flash-tmp")
os.makedirs(TMP, exist_ok=True)
FLASH = 0x1000000
TOC = 0x10000
GUN = datetime.date.today().strftime("%Y%m%d")

def say(s=""):
    print(s, flush=True)

def xf(args, timeout=1800):
    t0 = time.time()
    p = subprocess.run([XFEL] + args, capture_output=True, timeout=timeout)
    out = (p.stdout + p.stderr).decode("utf-8", "replace")
    return p.returncode, out, time.time() - t0

def fel_bekle():
    t0 = time.time()
    while True:
        rc, out, _ = xf(["version"], 5)
        if rc == 0 and "AWUSBFEX" in out:
            say("  FEL var: " + out.strip()); return
        sys.stdout.write("\r  FEL bekleniyor: %3d sn " % (time.time() - t0)); sys.stdout.flush()
        time.sleep(1)

def oku(addr, ln, path):
    if os.path.exists(path):  # v6: eski dosya "okundu" sayilmasin
        os.remove(path)
    rc, out, dt = xf(["spinor", "read", hex(addr), hex(ln), path])
    if rc != 0 or not os.path.exists(path) or os.path.getsize(path) != ln:
        raise SystemExit("HATA: spinor read 0x%x+0x%x: %s" % (addr, ln, out.strip()[-300:]))
    say("    okundu 0x%07x +0x%x  (%.0f sn, %.0f KB/s)" % (addr, ln, dt, ln / 1024 / max(dt, 0.01)))
    return open(path, "rb").read()

def cift_oku(addr, ln, ad):
    # v5 (14.10.2026, Claude): kullanıcı istegi "cift yedek yapma, tek yap, hizli yap": TEK okuma.
    # Yazilan bolgeler bilinen veriyle bayt bayt karsilastirildigi icin tek okuma yeterli kanittir.
    return oku(addr, ln, os.path.join(TMP, ad + "-1.bin"))

def yedek():
    fel_bekle()
    rc, out, _ = xf(["spinor"], 30)
    say("  " + out.strip())
    say("== 16 MB tam yedek (iki okuma) ==")
    d = cift_oku(0, FLASH, "tam")
    h = hashlib.sha256(d).hexdigest()
    if d[4:12] != b"eGON.BT0":  # v6: sifir/bozuk okuma yedek diye kaydedilmesin
        raise SystemExit("HATA: okunan goruntude boot0 imzasi yok (%r); yedek KAYDEDILMEDI" % d[4:12])
    p = os.path.join(OUTD, "FEL-flash-yedek-%s.bin" % datetime.datetime.now().strftime("%Y%m%d-%H%M%S"))
    if os.path.exists(p):
        raise SystemExit("HATA: yedek dosyasi zaten var: " + p)
    open(p, "wb").write(d)
    open(p + ".sha256", "w").write("%s  %s\n" % (h, os.path.basename(p)))
    say("  yedek: %s\n  sha256 %s" % (p, h))
    say("  0x10000 magic: %s, boot0: %s" % (d[TOC + 16:TOC + 20].hex(), d[4:12]))
    return 0

def yaz():
    yed = [p for p in glob.glob(os.path.join(OUTD, "FEL-flash-yedek-*.bin"))
           if time.time() - os.path.getmtime(p) < 3 * 3600 and os.path.getsize(p) == FLASH]
    if not yed:
        raise SystemExit("HATA: son 3 saatte alinmis tam yedek yok, once 'yedek' calistir")
    say("yedek: " + max(yed, key=os.path.getmtime))
    if not FL:
        raise SystemExit("HATA: --dizin <paket klasoru> zorunlu (or. linux-build/flash-v8-46)")
    toc = open(os.path.join(FL, "toc1.bin"), "rb").read()
    # v6: assert degil (python -O ile kaybolur); baslik + en az bir govde sektoru
    if toc[0:13] != b"sunxi-package" or len(toc) % 0x1000 or len(toc) <= 0x1000:
        raise SystemExit("HATA: toc1.bin sunxi-package degil, 4 KB hizali degil ya da govdesiz (%d B)" % len(toc))
    if TOC + len(toc) > 0xFE0000:
        raise SystemExit("HATA: toc1 UDISK'e tasiyor")
    say("toc1.bin (%s): %d B, sha256 %s" % (FL, len(toc), hashlib.sha256(toc).hexdigest()))
    if TOC + len(toc) > 0xC00000:
        raise SystemExit("HATA: toc1 data bolumune (0xC00000) tasiyor")
    toplam = os.path.join(FL, "SHA256SUMS")	# v7: paket SHA'si tutmali
    bek = [l.split()[0] for l in (open(toplam, encoding="ascii").read().splitlines() if os.path.exists(toplam) else [])
           if l.split()[1:2] == ["toc1.bin"]]
    if bek != [hashlib.sha256(toc).hexdigest()]:
        raise SystemExit("HATA: toc1.bin SHA256SUMS ile tutmuyor ya da SHA256SUMS yok (%s)" % FL)
    fel_bekle()
    say("  FEL %d sn sonra yeniden denetleniyor (uyanik MCU dusuruyor mu)..." % FEL_TEKRAR_SN)	# v7
    time.sleep(FEL_TEKRAR_SN)
    rc, out, _ = xf(["version"], 5)
    if rc != 0 or "AWUSBFEX" not in out:
        raise SystemExit("HATA: FEL %d sn icinde dustu (uyanik MCU?). amfi_bekci_sd_v*.sh -> .fel-bekle, bir FEL turu, yeniden dene. "
                         "Flash'a hicbir sey yazilmadi." % FEL_TEKRAR_SN)
    # v3: ONCE eski basligi gecersiz kil. Ogeler yazilirken kopma olursa flash'ta gecerli toc1 kalmaz ->
    # boot0 FEL'e doner (11.10.2026: v2 yarida kaldi, eski baslik yarim ogeleri gosterdi, cihaz SD'siz kurtarilamadi).
    say("== 0) eski toc1 basligi gecersiz kiliniyor (0x%x, 4 KB sifir) ==" % TOC)
    zp = os.path.join(TMP, "baslik-sifir.bin"); open(zp, "wb").write(b"\0" * 0x1000)
    rc, out, dt = xf(["spinor", "write", hex(TOC), zp])
    if rc != 0:
        raise SystemExit("HATA: baslik sifirlama: " + out.strip()[-200:])
    if cift_oku(TOC, 0x1000, "baslik0") != b"\0" * 0x1000:
        raise SystemExit("HATA: baslik sifirlanamadi")
    say("    eski baslik silindi (yarida kalirsa boot0 -> FEL)")
    govde = toc[0x1000:]
    gp = os.path.join(TMP, "govde.bin"); open(gp, "wb").write(govde)
    say("== 1) ogeler -> 0x%x (%d B) ==" % (TOC + 0x1000, len(govde)))
    rc, out, dt = xf(["spinor", "write", hex(TOC + 0x1000), gp])
    say("    yazildi (%.0f sn) rc=%d %s" % (dt, rc, out.strip()[-200:]))
    if rc != 0:
        raise SystemExit("HATA: yazma")
    if cift_oku(TOC + 0x1000, len(govde), "govde") != govde:
        raise SystemExit("HATA: ogeler dogrulanamadi; baslik YAZILMADI (flash FEL'e duser, yedekten donulebilir)")
    say("    ogeler DOGRU")
    say("== 2) toc1 baslik sektoru -> 0x%x (EN SON) ==" % TOC)
    hp = os.path.join(TMP, "baslik.bin"); open(hp, "wb").write(toc[:0x1000])
    rc, out, dt = xf(["spinor", "write", hex(TOC), hp])
    if rc != 0:
        raise SystemExit("HATA: baslik yazma")
    if cift_oku(TOC, 0x1000, "baslik") != toc[:0x1000]:
        raise SystemExit("HATA: baslik dogrulanamadi")
    say("    baslik DOGRU")
    b0 = cift_oku(0, 0x1000, "boot0")
    if b0[4:12] != b"eGON.BT0":  # v6
        raise SystemExit("HATA: boot0 imzasi bozuk (%r)! Cihazi KAPATMA; FEL acikken yedekten 0x0-0xFFFF geri yaz." % b0[4:12])
    say("  boot0 bozulmadi")
    say("\nTAMAM. Gucu sok-tak (ya da yazilimla yeniden baslat): boot0 -> OpenSBI -> Linux flash'tan acilmali.")
    say("Kurtarma: SD'ye FEL anahtari (outputs/sd_fel_switcher.bin, 8 KB'a) yazilir -> FEL -> xfel spinor write 0 <yedek>.")
    return 0

if __name__ == "__main__":
    if len(sys.argv) < 2 or sys.argv[1] not in ("yedek", "yaz", "hepsi"):
        print(__doc__); sys.exit(2)
    T0 = time.time()
    if sys.argv[1] in ("yedek", "hepsi"):
        yedek()
    if sys.argv[1] in ("yaz", "hepsi"):
        yaz()
    say("toplam %.0f sn" % (time.time() - T0))
