"""telegram_bildir_v4 (31.10.2026, Claude; kullanıcı: "ikinci kullanıcıya önemli bilgilendirmeler gelmiyor"): v3 + --herkese: mesaj
sahibe ek olarak work/terminal-local/telegram_izinli.txt'teki sohbetlere de gider (satır başına kimlik; git dışı; cihazdaki
/data/asistan/telegram-izinli ile aynı kişiler). Varsayılan v3 gibi yalnız sahip (geliştirme bildirimleri eşe gitmez).
    python -X utf8 work/telegram_bildir_v4.py --herkese --dosya duyuru.txt
--- v3 ---
telegram_bildir_v3 (21.10.2026, Claude; Sıra 8 madde 55 ölçümü): v2 + --sohbet-bul cihazı RAHATSIZ ETMEZ.
Ölçüm (21.10 10:15, iki iş parçacığı): YENİ bir getUpdates (timeout 0 bile) bekleyen uzun sorguyu 409 "terminated by other
getUpdates request" ile keser. v1/v2'nin 5 sn'lik göz atması cihazın (kole-gemini --telegram) uzun sorgusunu 5 sn'de bir
kesiyordu: v24 o turu bitirip çıkıyor, autorun 15 sn uyuyor -> süreç başlatma ~3 kat, düğme gecikmesi 15 sn'ye kadar.
v3: --aralik (varsayılan 60) sn'de bir ANA göz atma + ardından 2 sn arayla 6 kısa göz atma (cihaz, ana göz atmanın kestiği
turdan sonra 15 sn uyurken; o sırada gelen mesaj onaylanmamış kalır). Cihaza ek kesinti dakikada en çok 1 (v24'ün doğal
turu ~56 sn).
--- v2 ---
telegram_bildir_v2 (21.10.2026, Claude): v1 + KUYRUK: sohbet kimliği yoksa mesaj work/terminal-local/telegram-kuyruk/'a
yazılır; kimlik bulununca (--sohbet-bul ya da sonraki gönderim) sırayla gönderilir. --kuyruk-gonder yalnız kuyruğu boşaltır.
--- v1 ---
telegram_bildir_v1 (21.10.2026, Claude): yapılan işleri kullanıcıya KÖLE Telegram botuyla gönderir (PC'den).
kullanıcı 21.10: "yaptıklarını bana telegram botuyla gönder kuralı değiştir" (önceki kural "PC'den Telegram'a mesaj YOK" kalktı).
    python -X utf8 work/telegram_bildir_v1.py --metin "..."          | --dosya ozet.txt
    python -X utf8 work/telegram_bildir_v1.py --sohbet-bul [--dk 30]  (sohbet kimliğini bekle/bul, kaydet)
Jeton: ANA klasör work/terminal-local/telegram_jeton.txt; sohbet: work/terminal-local/telegram_sohbet.txt (ikisi git dışı,
hiçbir yere yazdırılmaz). Sohbet kimliği yoksa getUpdates'e YALNIZ GÖZ ATILIR: offset verilmez (güncellemeler onaylanmaz,
cihazdaki bot kendi mesajlarını almaya devam eder) ve timeout 0 (cihazın sorgusuyla 409 çakışması olmasın); bota bir mesaj
yazıldığında (cihaz onaylamadan önceki ~15 sn içinde) sahibin özel sohbeti bulunur. 4000 harften uzun metin bölünür."""
import argparse
import json
import sys
import time
import urllib.parse
import urllib.request
from pathlib import Path

BU = Path(__file__).resolve()
ANA = Path(*BU.parts[:BU.parts.index('.claude')]) if '.claude' in BU.parts else BU.parents[1]
LOCAL = ANA / 'work' / 'terminal-local'
JETON = LOCAL / 'telegram_jeton.txt'
SOHBET = LOCAL / 'telegram_sohbet.txt'
IZINLI = LOCAL / 'telegram_izinli.txt'	# v4
KUYRUK = LOCAL / 'telegram-kuyruk'


def jeton():
    return JETON.read_text(encoding='utf-8').strip()


def istek(metod, alanlar=None, zaman=20):
    veri = urllib.parse.urlencode(alanlar or {}).encode()
    req = urllib.request.Request(f'https://api.telegram.org/bot{jeton()}/{metod}', data=veri)
    try:
        with urllib.request.urlopen(req, timeout=zaman) as r:
            return json.loads(r.read().decode('utf-8'))
    except urllib.error.HTTPError as e:
        try:
            return json.loads(e.read().decode('utf-8'))
        except ValueError:
            return {'ok': False, 'description': f'HTTP {e.code}'}
    except OSError as e:
        return {'ok': False, 'description': str(e).replace(jeton(), '***')}


def goz_at():
    y = istek('getUpdates', {'timeout': 0, 'limit': 100})	# offset YOK: onaylamaz
    for u in y.get('result', []) if y.get('ok') else []:
        m = u.get('message') or (u.get('callback_query') or {}).get('message') or {}
        c = m.get('chat') or {}
        if c.get('type') == 'private' and c.get('id'):
            SOHBET.write_text(str(c['id']) + '\n', encoding='utf-8')
            print('sohbet bulundu ve kaydedildi', flush=True)
            return True
    return False


def sohbet_bul(dk, aralik=60):
    son = time.monotonic() + dk * 60
    while time.monotonic() < son:
        tur = time.monotonic()
        for i in range(7):	# ana göz atma + cihazın 15 sn'lik uykusunda 6 kısa göz atma
            if goz_at():
                return True
            time.sleep(2)
        time.sleep(max(1, aralik - (time.monotonic() - tur)))
    print('sohbet bulunamadı (bota bir mesaj yazılmadı)', flush=True)
    return False


def kuyruga(metin):
    KUYRUK.mkdir(parents=True, exist_ok=True)
    y = KUYRUK / (time.strftime('%Y%m%d-%H%M%S') + f'-{time.monotonic_ns() % 100000:05d}.txt')
    y.write_text(metin, encoding='utf-8')
    print('sohbet kimliği yok: mesaj kuyruğa alındı', y.name, flush=True)
    return True


def kuyruk_gonder():
    if not SOHBET.exists() or not KUYRUK.exists():
        return 0
    n = 0
    for y in sorted(KUYRUK.glob('*.txt')):
        if not tek_gonder(y.read_text(encoding='utf-8')):
            break
        y.unlink()
        n += 1
    if n:
        print(f'kuyruktan {n} mesaj gönderildi', flush=True)
    return n


def izinliler(sahip):
    """v4: geçerli, sahipten farklı, tekrarsız kimlikler"""
    if not IZINLI.exists():
        return []
    l = []
    for s in IZINLI.read_text(encoding='utf-8').split():
        if s.lstrip('-').isdigit() and s != sahip and s not in l:
            l.append(s)
    return l


def gonder(metin, herkese=False):
    if not SOHBET.exists():
        return kuyruga(metin)
    kuyruk_gonder()
    return tek_gonder(metin, herkese)


def tek_gonder(metin, herkese=False):
    sahip = SOHBET.read_text(encoding='utf-8').strip()
    alicilar = [sahip] + (izinliler(sahip) if herkese else [])
    parcalar = [metin[i:i + 3900] for i in range(0, len(metin), 3900)] or ['']
    tamam = True
    for i, sohbet in enumerate(alicilar):
        for p in parcalar:
            y = istek('sendMessage', {'chat_id': sohbet, 'text': p, 'disable_web_page_preview': 'true'})
            if not y.get('ok'):
                print(f'gönderilemedi (alıcı {i + 1}):', y.get('description'), flush=True)
                tamam = False
                break
    print(f'gönderildi ({len(parcalar)} mesaj, {len(metin)} harf, {len(alicilar)} alıcı)', flush=True)
    return tamam


def main():
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--metin')
    p.add_argument('--dosya')
    p.add_argument('--sohbet-bul', action='store_true')
    p.add_argument('--dk', type=float, default=30)
    p.add_argument('--kuyruk-gonder', action='store_true')
    p.add_argument('--aralik', type=float, default=60)
    p.add_argument('--herkese', action='store_true', help='v4: izinli sohbetlere de (telegram_izinli.txt)')
    a = p.parse_args()
    if a.sohbet_bul:
        ok = sohbet_bul(a.dk, a.aralik)
        if ok:
            kuyruk_gonder()
        sys.exit(0 if ok else 1)
    if a.kuyruk_gonder:
        kuyruk_gonder()
        sys.exit(0)
    metin = a.metin if a.metin is not None else Path(a.dosya).read_text(encoding='utf-8')
    sys.exit(0 if gonder(metin, a.herkese) else 1)


if __name__ == '__main__':
    main()
