"""com_kur_v12 (26.10.2026, Claude): v11 + --ip verilmezse IP cihazdan (seri kabukta wlan0 inet) okunur.
Sebep: 26.10 cihaz DHCP ile .128 aldi, .131 baska cihaz oldu; v11 sabit .131'e aktarip zaman asimina dusuyordu. --- v11 ---
com_kur_v11 (24.10.2026, Claude; denetim-2026-10-24-v3 A19): v10 + yedek kopyasi basarisizsa kurulmaz (v10 sonucuna
bakmiyordu ve var olmayan yedegi yaziyordu), yedek yalniz gercekten alindiysa yazilir; izin listesinde mcu-servis-vN ve
wifi_bekci_sd_vN.sh. --- v10 ---
com_kur_v10 (24.10.2026, Claude; denetim v2 bulgu 4): v9 + izin listesinde wifi_kaydet_sd_vN.sh (SD kökü; autorun_v46
RAM'deki wifi-kaydet'in yerine kopyalar). --- v9 ---
com_kur_v9 (23.10.2026, Claude; açılış MCU kalp sıfırlaması, scripts/582): v8 + izin listesinde mcu_servis_sd_vN.sh ve
yeniden_baslat_sd_vN.sh (SD gözetici betikleri; mcu-servis ikilisi DEĞİŞMEZ, sha256 betikte sabit). --- v8 ---
com_kur_v8 (23.10.2026, Claude; Sıra 18 madde 136): v7 + izin listesinde kole-<ad>-vN (yalnız kendi C/Go
ikililerimizin adlandırması: kole-gozetici-v1 gibi; SD kökü). --- v7 ---
com_kur_v7 (23.10.2026, Claude; Sıra 18 madde 132): v6 + aktarım panel_guncelle_ag_v9 (SD ara dosya 32 MiB). --- v6 ---
com_kur_v6 (23.10.2026, Claude; Sıra 18 madde 132/134): v5 + izin listesinde kole-wdt-vN (SD kökü) ve
tailscaled-<ad> (SD tailscale/ alt dizini; çalışan tailscaled-kucuk adına YAZILMAZ, ayrı adla kurulur). --- v5 ---
com_kur_v5 (23.10.2026, Claude; Sıra 18 madde 132): v4 + izin listesinde tun-vN.ko (çekirdek TUN modülü, SD kökü;
insmod ayrı adım, kurulum yalnız dosya). --- v4 ---
com_kur_v4 (22.10.2026, Claude; madde 116): v3 + izin listesinde amfi-bekci-vN ve amfi_bekci_sd_vN.sh (SD'ye; MCU'ya
komut göndermez, yalnız dosya kurar).
--- v3 ---
com_kur_v3 (22.10.2026, Claude; madde 112, kole-gemini v31): v2 + 4 MiB'tan büyük dosya RAM yerine SD ara dosyasıyla
(/mnt/sd/velinux/.com-kur-<ad>-<zaman>, panel_guncelle_ag_v8). Sebep: 7 MB'lık kole-gemini için cihazda MemAvailable
dosya + 10 MiB'tan azdı, v2 "SD değişmedi" diye duruyordu. Kurulum adımları (yedek, .yeni, SHA, mv) aynı.
    python -X utf8 work/com_kur_v3.py <yerel>=<sd_adi>=<sha256> ... [--ip <IP; yoksa cihazdan>] [--sd]   (--sd: hepsi SD ara dosyayla)
--- v2 ---
com_kur_v2 (22.10.2026, Claude; madde 102): v1 + izin listesinde bellek_ayar_vN.sh ve tailscale_baslat_vN.sh (SD gozetici
betikleri; Merkez OTA izin listesi degismedi, bu yalniz COM bakim yolu).
--- v1 ---
com_kur_v1 (21.10.2026, Claude; kullanıcı 21.10: "com varsa com üzerinden yap parola sormadan yoksa parola istersin"):
Merkez OTA'sının PAROLASIZ karşılığı: USB seri kabuk (COM5) + LAN nc aktarımı ile SD'ye kurulum.
    python -X utf8 work/com_kur_v2.py <yerel>=<sd_adi>=<sha256> ... [--ip 192.168.1.50]
Her dosya için:
  1. work/panel_guncelle_ag_v7.py ile /tmp/com-kur-<ad>-<pid> (RAM; MemAvailable denetimi, 256 KiB/s, SHA denetimli).
  2. COM5: /tmp kopyasının SHA'sı; SD hedefi aynı SHA'daysa atlanır; değilse eski dosya <hedef>.ota-yedek-<zaman>
     (Merkez ile aynı ad: sd_geri_donus_sira8_v1 bulur), <hedef>.yeni'ye kopya + sync + SHA, sonra mv (çalışan süreç eski
     dosyayı tutmaya devam eder), /tmp silinir.
  Ad kuralı Merkez v9 izin listesiyle aynı: kole-gemini-vN -> velinux/asistan/, diğerleri velinux/.
Yeniden başlatma YAPMAZ (ayrı karar). COM5 yoksa durur: o zaman ota_toplu_yukle_v6 + kullanıcı parolası."""
import hashlib
import re
import subprocess
import sys
import time
from pathlib import Path

BU = Path(__file__).resolve().parent
sys.path.insert(0, str(BU))
import f133_seri_v5 as seri  # noqa: E402

D = '/mnt/sd/velinux'
IZIN = re.compile(r'(panel|kole-asistan-v\d+|kole-gemini-v\d+|kamera-kayit-v\d+|kole-kopru-v\d+|kole-cihaz-merkez-v\d+|'
                  r'autorun_v\d+\.sh|kamera_kopru_sd_v\d+\.sh|wifi-ekran-v\d+|bellek_ayar_v\d+\.sh|tailscale_baslat_v\d+\.sh|'
                  r'amfi-bekci-v\d+|amfi_bekci_sd_v\d+\.sh|tun-v\d+\.ko|kole-wdt-v\d+|kole-[a-z]+-v\d+|tailscaled-[a-z0-9-]+|'
                  r'mcu_servis_sd_v\d+\.sh|yeniden_baslat_sd_v\d+\.sh|wifi_kaydet_sd_v\d+\.sh|mcu-servis-v\d+|wifi_bekci_sd_v\d+\.sh)')	# v2; v4: amfi; v5: tun; v6: wdt, tailscaled; v9: mcu/yeniden; v10: wifi_kaydet


def hedef_yolu(ad):
    if ad.startswith('tailscaled-'):
        if ad == 'tailscaled-kucuk':
            raise SystemExit('calisan tailscaled-kucuk uzerine yazilmaz (ayri ad kullan)')
        return f'{D}/tailscale/{ad}'
    return f'{D}/asistan/{ad}' if ad.startswith('kole-gemini-v') else f'{D}/{ad}'


def kabuk_ac():
    try:
        return seri.Kabuk('COM5')
    except Exception as e:  # noqa: BLE001
        sys.exit(f'COM5 açılamadı ({e}): USB yok -> python -X utf8 work/ota_toplu_yukle_v6.py ... (kullanıcı parolası)')


def calistir(k, cmd, bekle=4):
    out = k.run('echo; ' + cmd, bekle)
    return out


def main():
    argv = sys.argv[1:]
    ip = None	# v12: verilmezse cihazdan
    hep_sd = '--sd' in argv	# v3
    argv = [x for x in argv if x != '--sd']
    if '--ip' in argv:
        i = argv.index('--ip')
        ip = argv[i + 1]
        del argv[i:i + 2]
    isler = []
    for a in argv:
        try:
            yerel, ad, sha = a.split('=')
        except ValueError:
            sys.exit(__doc__)
        y = Path(yerel)
        if not IZIN.fullmatch(ad):
            sys.exit(f'HATA: {ad} izin listesinde değil (Merkez v9 ile aynı kural)')
        h = hashlib.sha256(y.read_bytes()).hexdigest()
        if h != sha:
            sys.exit(f'HATA: {y} yerel SHA {h[:16]} != beklenen {sha[:16]}')
        print(f'yerel SHA tamam: {y.name} -> {ad} ({sha[:16]})', flush=True)
        isler.append((y, ad, sha))
    if not isler:
        sys.exit(__doc__)
    k = kabuk_ac()
    out = calistir(k, f"cat /proc/uptime; df -k {D} | tail -1; awk '/MemAvailable/ {{print \"MEMKB\", $2}}' /proc/meminfo; "
                      "ip -4 addr show wlan0 | sed -n 's/.*inet \\([0-9.]*\\).*/WLANIP \\1/p'")
    k.s.close()
    print(out.strip(), flush=True)
    if not ip:	# v12
        m = re.search(r'WLANIP (\d+\.\d+\.\d+\.\d+)', out)
        if not m:
            sys.exit('HATA: cihazin Wi-Fi IP\'si okunamadi; --ip ile verin')
        ip = m.group(1)
        print(f'cihaz IP (wlan0): {ip}', flush=True)
    for y, ad, sha in isler:
        hedef = hedef_yolu(ad)
        k = kabuk_ac()
        out = calistir(k, f'sha256sum {hedef} 2>/dev/null | cut -c1-64; echo SON')
        k.s.close()
        if sha in out:
            print(f'{ad}: cihazda zaten aynı ({sha[:16]}), atlandı', flush=True)
            continue
        sd = hep_sd or y.stat().st_size > 4 * 1024 * 1024	# v3
        gecici = (f'{D}/.com-kur-' if sd else '/tmp/com-kur-') + f'{ad}-{int(time.time())}'
        print(f'aktarılıyor: {y.name} ({y.stat().st_size} bayt) -> {gecici}', flush=True)
        t0 = time.monotonic()
        r = subprocess.run([sys.executable, '-X', 'utf8', str(BU / 'panel_guncelle_ag_v9.py'), str(y), gecici, '--ip', ip],
                           capture_output=True, text=True, encoding='utf-8', errors='replace')
        if 'TRANSFER PASS' not in r.stdout:
            print(r.stdout[-1500:], r.stderr[-800:], flush=True)
            sys.exit(f'HATA: {ad} aktarılamadı; SD değişmedi.')
        print(f'  {"SD ara dosya" if sd else "RAM"} aktarımı tamam ({time.monotonic() - t0:.0f} sn)', flush=True)
        k = kabuk_ac()
        try:
            out = calistir(k, f'sha256sum {gecici} | cut -c1-64')
            if sha not in out:
                sys.exit(f'HATA: {gecici} SHA tutmadı; SD değişmedi.')
            z = time.strftime('%Y%m%d-%H%M%S')
            out = calistir(k, f'H={hedef}; T={gecici}; '
                              f'{{ [ ! -f $H ] || cp $H $H.ota-yedek-{z}; }} && cp $T $H.yeni && sync && '
                              f'[ "$(sha256sum $H.yeni | cut -c1-64)" = {sha} ] && mv $H.yeni $H && chmod 755 $H 2>/dev/null; '
                              f'sync; [ "$(sha256sum $H | cut -c1-64)" = {sha} ] && echo KURULDU; [ -f $H.ota-yedek-{z} ] && echo YEDEKVAR; rm -f $T', 30)
            if '\nKURULDU' not in out:
                print(out[-800:], flush=True)
                sys.exit(f'HATA: {ad} SD\'ye yazılamadı (yedek {hedef}.ota-yedek-{z}, {hedef}.yeni bakın).')
            print(f'  KURULDU: {hedef} ({sha[:16]}), ' + (f'eski: {hedef}.ota-yedek-{z}' if '\nYEDEKVAR' in out else 'ilk kurulum (yedek yok)'), flush=True)
        finally:
            k.s.close()
    print('PASS: hepsi kuruldu (SHA doğrulandı). Yeniden başlatılmadı.', flush=True)


if __name__ == '__main__':
    main()
