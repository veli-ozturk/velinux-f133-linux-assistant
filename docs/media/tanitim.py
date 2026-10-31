"""VeLinux KÖLE tanıtım: robot göz GIF'i (docs/media/robot-eyes.gif) + tanıtım videosu (docs/media/velinux-kole-tanitim.mp4).
Gözler panelin robot göz koruyucusundaki renk/ölçülerle (main_pc_oturum_v67.c rg_ciz_normal) Pillow'da çizilir."""
import math, subprocess, sys
from pathlib import Path
from PIL import Image, ImageDraw, ImageFilter, ImageFont

REPO = Path(sys.argv[1])
FFMPEG = sys.argv[2] if len(sys.argv) > 2 else None
OUT = REPO / 'docs' / 'media'
OUT.mkdir(parents=True, exist_ok=True)
IMG = REPO / 'docs' / 'images'

CYAN, GLOW, PUPIL = (0x22, 0xD3, 0xEE), (0x06, 0xB6, 0xD4), (0x05, 0x26, 0x2E)
RED, RGLOW, RPUPIL = (0xFF, 0x3B, 0x3B), (0xFF, 0x00, 0x00), (0x38, 0x05, 0x05)
PINK = (0xFF, 0x4F, 0x8B)
F = 'C:/Windows/Fonts/'


def font(ad, n):
    return ImageFont.truetype(F + ad, n)


def ease(t):
    return 0.5 - 0.5 * math.cos(math.pi * max(0.0, min(1.0, t)))


def lerp(a, b, t):
    return a + (b - a) * t


# --- göz çizimi (1600x600 tuval, 2x üst örnekleme ile) ---
W, H = 1600, 600
EX = (590, 1010)   # göz merkezleri
EW, EH, ER = 230, 250, 62


def heart(d, cx, cy, s, renk):
    pts = []
    for i in range(80):
        t = 2 * math.pi * i / 80
        x = 16 * math.sin(t) ** 3
        y = 13 * math.cos(t) - 5 * math.cos(2 * t) - 2 * math.cos(3 * t) - math.cos(4 * t)
        pts.append((cx + x * s, cy - y * s))
    d.polygon(pts, fill=renk)


def goz_kare(st):
    """st: acik(0..1), bx/by bakış(-1..1), olcek, kizgin(0..1), kip('normal'|'mutlu'|'kalp'|'uyku'), kirmizi(0..1), zzz(0..1)"""
    S = 2
    im = Image.new('RGB', (W * S, H * S), (0, 0, 0))
    glow = Image.new('RGB', (W * S, H * S), (0, 0, 0))
    gd, d = ImageDraw.Draw(glow), ImageDraw.Draw(im)
    k = st.get('kirmizi', 0)
    govde = tuple(int(lerp(c, r, k)) for c, r in zip(CYAN, RED))
    pir = tuple(int(lerp(c, r, k)) for c, r in zip(GLOW, RGLOW))
    beb = tuple(int(lerp(c, r, k)) for c, r in zip(PUPIL, RPUPIL))
    kip = st.get('kip', 'normal')
    for i, ex in enumerate(EX):
        sc = st.get('olcek', 1.0)
        cx = (ex + st.get('bx', 0) * 120) * S
        cy = (H / 2 + st.get('by', 0) * 60) * S
        w, h = EW * sc * S, EH * sc * S * max(0.06, st.get('acik', 1.0))
        r = min(ER * sc * S, h / 2)
        if kip == 'kalp':
            s = 9.5 * st.get('kalp_olcek', 1.0) * S
            heart(gd, cx, cy, s * 1.08, (0x80, 0x20, 0x40))
            heart(d, cx, cy, s, PINK)
            continue
        if kip in ('mutlu', 'uyku'):  # yay: mutlu ^ (üst), uyku ‿ (alt)
            bb = [cx - 95 * S, cy - 95 * S, cx + 95 * S, cy + 95 * S]
            a = (200, 340) if kip == 'mutlu' else (20, 160)
            gd.arc(bb, *a, fill=pir, width=34 * S)
            d.arc(bb, *a, fill=govde, width=22 * S)
            continue
        box = [cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2]
        gd.rounded_rectangle([box[0] - 18 * S, box[1] - 18 * S, box[2] + 18 * S, box[3] + 18 * S], r + 18 * S, fill=pir)
        d.rounded_rectangle(box, r, fill=govde)
        if st.get('acik', 1.0) > 0.35:
            pr = 42 * sc * S
            px = cx + st.get('bx', 0) * 40 * S - 18 * S
            py = cy + st.get('by', 0) * 30 * S - 8 * S
            d.ellipse([px - pr, py - pr, px + pr, py + pr], fill=beb)
            hr = 13 * sc * S
            d.ellipse([px - pr * 0.35 - hr, py - pr * 0.35 - hr, px - pr * 0.35 + hr, py - pr * 0.35 + hr], fill=(240, 250, 255))
            k2 = hr / 2
            d.ellipse([px + pr * 0.3 - k2, py + pr * 0.34 - k2, px + pr * 0.3 + k2, py + pr * 0.34 + k2], fill=(170, 190, 200))
        kz = st.get('kizgin', 0)
        if kz > 0:  # eğik üst kapak: iç köşe aşağıda
            ic = 1 if i == 0 else -1
            xi, xd = cx + ic * w / 2, cx - ic * w / 2
            yi = box[1] + h * 0.62 * kz
            yd = box[1] - 4 * S
            m = 30 * S
            d.polygon([(xd - ic * m, yd - m * 2), (xi + ic * m, yd - m * 2), (xi + ic * m, yi), (xd - ic * m, yd)], fill=(0, 0, 0))
            gd.polygon([(xd - ic * m, yd - m * 2), (xi + ic * m, yd - m * 2), (xi + ic * m, yi), (xd - ic * m, yd)], fill=(0, 0, 0))
    glow = glow.filter(ImageFilter.GaussianBlur(14 * S))
    im = Image.composite(im, glow, im.convert('L').point(lambda v: 255 if v > 0 else 0))
    if st.get('zzz', 0) > 0:
        d = ImageDraw.Draw(im)
        z = st['zzz']
        for j, (dx, dy, n) in enumerate([(0, 0, 40), (40, -45, 56), (90, -100, 74)]):
            if z * 3 > j:
                d.text(((1130 + dx) * S, (170 + dy) * S), 'z' if j < 2 else 'Z', font=font('seguisb.ttf', n * S), fill=govde)
    if k > 0.5:
        d = ImageDraw.Draw(im)
        bx0, by0 = 690 * S, 60 * S
        d.rounded_rectangle([bx0, by0, bx0 + 220 * S, by0 + 44 * S], 22 * S, fill=(0xE1, 0x1D, 0x48))
        d.ellipse([bx0 + 18 * S, by0 + 15 * S, bx0 + 32 * S, by0 + 29 * S], fill=(255, 255, 255))
        d.text((bx0 + 42 * S, by0 + 7 * S), 'KAMERA AÇIK', font=font('seguisb.ttf', 24 * S), fill=(255, 255, 255))
    return im.resize((W, H), Image.LANCZOS)


# --- zaman çizelgesi: (süre sn, başlangıç durumu, bitiş durumu) ---
N = {'acik': 1, 'bx': 0, 'by': 0, 'olcek': 1, 'kizgin': 0, 'kirmizi': 0, 'zzz': 0}


def D(**k):
    return {**N, **k}


KAPALI = D(acik=0.06)
CIZELGE = [
    (0.6, D(), D()),
    (0.12, D(), KAPALI), (0.12, KAPALI, D()),                        # kırp
    (0.5, D(), D(bx=-1)), (0.6, D(bx=-1), D(bx=-1)),               # sola bak
    (0.7, D(bx=-1), D(bx=1)), (0.6, D(bx=1), D(bx=1)),              # sağa bak
    (0.4, D(bx=1), D()),
    (0.12, D(), KAPALI), (0.12, KAPALI, D()),
    (0.25, D(), D(olcek=1.22, by=-0.2)), (0.7, D(olcek=1.22, by=-0.2), D(olcek=1.22, by=-0.2)),  # şaşkın
    (0.25, D(olcek=1.22, by=-0.2), D()),
    (1.1, D(kip='mutlu'), D(kip='mutlu')),                          # mutlu
    (1.4, D(kip='kalp'), D(kip='kalp')),                            # kalp
    (0.3, D(), D(kizgin=1)), (0.9, D(kizgin=1), D(kizgin=1)), (0.3, D(kizgin=1), D()),  # kızgın
    (0.5, D(), D(acik=0.45, by=0.3)), (0.5, D(acik=0.45, by=0.3), D(acik=0.45, by=0.3)),  # uykulu
    (1.6, D(kip='uyku', zzz=0), D(kip='uyku', zzz=1)),              # uyku zzz
    (0.12, KAPALI, D()),
    (0.35, D(), D(kirmizi=1)), (1.2, D(kirmizi=1), D(kirmizi=1, bx=0.4)), (0.35, D(kirmizi=1, bx=0.4), D()),  # kamera
    (0.4, D(), D()),
]


def goz_kareleri(fps):
    for sure, a, b in CIZELGE:
        n = max(1, round(sure * fps))
        for i in range(n):
            t = ease(i / max(1, n - 1)) if n > 1 else 1
            st = {}
            for key in set(a) | set(b):
                va, vb = a.get(key, N.get(key)), b.get(key, N.get(key))
                st[key] = lerp(va, vb, t) if isinstance(va, (int, float)) and isinstance(vb, (int, float)) else (va if t < 0.5 else vb)
            if st.get('kip') == 'kalp':
                st['kalp_olcek'] = 1 + 0.12 * math.sin(i / fps * 2 * math.pi * 1.6)
            yield goz_kare(st)


def gif_yap():
    fps = 20
    kareler = [k.resize((640, 240), Image.LANCZOS) for k in goz_kareleri(fps)]
    ornek = kareler[::6]  # palet tüm durumlardan (camgöbeği, pembe, kırmızı) oluşsun
    mont = Image.new('RGB', (640, 240 * len(ornek)))
    for i, k in enumerate(ornek):
        mont.paste(k, (0, 240 * i))
    pal = mont.quantize(colors=128, method=Image.Quantize.MEDIANCUT)
    q = [k.quantize(palette=pal, dither=Image.Dither.NONE) for k in kareler]
    yol = OUT / 'robot-eyes.gif'
    q[0].save(yol, save_all=True, append_images=q[1:], duration=1000 // fps, loop=0, optimize=True, disposal=1)
    print('gif', yol.stat().st_size, len(q), 'kare')
    return kareler


# --- video ---
VW, VH, VFPS = 1280, 720, 30
BG = (5, 5, 5)


def kart(baslik, alt, ust=None, gorsel=None, gorsel_w=1120):
    im = Image.new('RGB', (VW, VH), BG)
    d = ImageDraw.Draw(im)
    for y in range(VH):  # hafif kırmızı ışıma
        a = max(0, 1 - abs(y - VH * 0.15) / (VH * 0.9))
        d.line([(0, y), (VW, y)], fill=(int(5 + 30 * a * a), int(5 + 4 * a), int(8 + 8 * a)))
    if ust:
        d.text((VW / 2, 52), ust, font=font('seguisb.ttf', 22), fill=(0xE1, 0x1D, 0x48), anchor='mm')
    d.text((VW / 2, 92), baslik, font=font('seguisb.ttf', 40), fill=(245, 245, 245), anchor='mm')
    if alt:
        d.text((VW / 2, 134), alt, font=font('segoeui.ttf', 24), fill=(168, 162, 158), anchor='mm')
    if gorsel is not None:
        g = gorsel.copy()
        oran = min(gorsel_w / g.width, (VH - 200) / g.height)
        g = g.resize((int(g.width * oran), int(g.height * oran)), Image.LANCZOS)
        x, y = (VW - g.width) // 2, 170 + (VH - 190 - g.height) // 2
        golge = Image.new('RGB', (g.width + 24, g.height + 24), (0, 0, 0))
        im.paste(golge.filter(ImageFilter.GaussianBlur(10)), (x - 12, y - 6))
        im.paste(g, (x, y))
    return im


def ac(p):
    return Image.open(IMG / p).convert('RGB')


def sahneler(goz):
    logo = Image.open(REPO / 'go' / 'kole-gemini' / 'kole-logo.jpg').convert('RGB')
    giris = Image.new('RGB', (VW, VH), BG)
    l = logo.resize((300, 300), Image.LANCZOS)
    giris.paste(l, ((VW - 300) // 2, 120))
    d = ImageDraw.Draw(giris)
    d.text((VW / 2, 480), 'VeLinux KÖLE', font=font('seguisb.ttf', 64), fill=(245, 245, 245), anchor='mm')
    d.text((VW / 2, 545), 'A cheap F133 smart display, rebuilt into a Linux home assistant', font=font('segoeui.ttf', 26), fill=(200, 200, 200), anchor='mm')
    d.text((VW / 2, 585), 'Ucuz bir F133 akıllı ekran, Linux ev asistanına dönüştü', font=font('segoeui.ttf', 26), fill=(168, 162, 158), anchor='mm')
    d.text((VW / 2, 650), 'RISC-V · 64 MB RAM · 16 MB flash · Linux 5.4 · Go + C · Gemini · Telegram · Tailscale', font=font('segoeui.ttf', 20), fill=(0xE1, 0x1D, 0x48), anchor='mm')
    yield 'sabit', giris, 3.5
    yield 'goz', goz, None
    liste = [
        ('Touch panel · Dokunmatik panel', 'LVGL 9.2 on the framebuffer, 1600×600 · framebuffer üzerinde', 'panel-pano.png'),
        ('Themes · Temalar', 'Red/black, blue, white · Kırmızı/siyah, mavi, beyaz', 'panel-tema-mavi.png'),
        ('Assistant · Asistan', 'Ask Gemini by voice or touch · Sesle ya da dokunarak Gemini\'ye sor', 'panel-asistan.png'),
        ('Media · Medya', 'PC media keys and Bluetooth phone · PC medya tuşları ve Bluetooth telefon', 'panel-medya.png'),
        ('PC System · PC Sistem', 'Live stats from the Windows PC · Windows PC\'den canlı bilgi', 'panel-pcsistem.png'),
        ('Control center · Denetim merkezi', 'Pull down for quick settings · Hızlı ayarlar için aşağı çek', 'panel-denetim-merkezi.png'),
        ('System & services · Sistem ve hizmetler', 'Everything supervised, all on 55 MB · Hepsi gözetimli, 55 MB\'ta', 'panel-ayar-sistem.png'),
        ('Screen savers · Ekran koruyucular', 'Clock, logo, words, world, stars, aurora…', 'panel-koruyucular.png'),
        ('Merkez PWA', 'Phone app over Tailscale, works behind CGNAT · Tailscale ile telefon uygulaması', 'merkez-pwa.png'),
        ('PC Merkez', 'Windows companion: status, time-lapse · Windows yardımcısı: durum, time-lapse', 'pc-merkez-v8-genel.png'),
    ]
    for b, a, p in liste:
        yield 'sabit', kart(b, a, 'VeLinux KÖLE', ac(p)), 3.2
    son = kart('Measured · Ölçüldü', None, 'VeLinux KÖLE')
    d = ImageDraw.Draw(son)
    satir = [('Second screen · İkinci ekran', '6.9 → 19.8 fps'), ('Idle CPU · Boşta işlemci', '35% → 86%'),
             ('Boot (init) · Açılış', '7.8× faster · daha hızlı'), ('Tailscale RAM', '12 → 5 MB'),
             ('Assistant versions · Asistan sürümü', '51'), ('Measured experiments · Ölçülen deney', '200+')]
    for i, (s, v) in enumerate(satir):
        y = 210 + i * 72
        d.rounded_rectangle([200, y - 28, 1080, y + 28], 14, fill=(22, 16, 16))
        d.text((230, y), s, font=font('segoeui.ttf', 28), fill=(220, 220, 220), anchor='lm')
        d.text((1050, y), v, font=font('seguisb.ttf', 30), fill=(0xF9, 0x73, 0x16), anchor='rm')
    yield 'sabit', son, 4.0
    bitis = Image.new('RGB', (VW, VH), BG)
    bitis.paste(logo.resize((180, 180), Image.LANCZOS), ((VW - 180) // 2, 170))
    d = ImageDraw.Draw(bitis)
    d.text((VW / 2, 410), 'Open source · Açık kaynak — MIT', font=font('seguisb.ttf', 40), fill=(245, 245, 245), anchor='mm')
    d.text((VW / 2, 470), 'github.com/veli-ozturk/velinux-f133-linux-assistant', font=font('segoeui.ttf', 30), fill=(0x22, 0xD3, 0xEE), anchor='mm')
    d.text((VW / 2, 530), 'Firmware, recovery, build scripts · Firmware, kurtarma, derleme betikleri', font=font('segoeui.ttf', 22), fill=(168, 162, 158), anchor='mm')
    yield 'sabit', bitis, 3.5


def video_yap():
    yol = OUT / 'velinux-kole-tanitim.mp4'
    p = subprocess.Popen([FFMPEG, '-y', '-loglevel', 'error', '-f', 'rawvideo', '-pix_fmt', 'rgb24', '-s', f'{VW}x{VH}',
                          '-r', str(VFPS), '-i', '-', '-c:v', 'libx264', '-preset', 'slow', '-crf', '23', '-pix_fmt', 'yuv420p',
                          '-movflags', '+faststart', str(yol)], stdin=subprocess.PIPE)
    onceki, n = None, 0

    def yaz(im):
        nonlocal n
        p.stdin.write(im.tobytes())
        n += 1

    def gecis(a, b, sn=0.4):
        if a is None:
            return
        for i in range(int(sn * VFPS)):
            yaz(Image.blend(a, b, ease(i / (sn * VFPS))))

    for tur, im, sure in sahneler(None):
        if tur == 'goz':
            ilk = None
            gk = kart('“Hey Köle” robot eyes · Robot gözler', 'Panel screen saver and assistant face · Panel koruyucusu ve asistanın yüzü', 'VeLinux KÖLE')
            for k in goz_kareleri(VFPS):
                v = gk.copy()
                g = k.resize((1200, 450), Image.LANCZOS)
                v.paste(g, ((VW - 1200) // 2, 200))
                if ilk is None:
                    gecis(onceki, v)
                    ilk = v
                yaz(v)
                onceki = v
            continue
        gecis(onceki, im)
        for _ in range(int(sure * VFPS)):
            yaz(im)
        onceki = im
    p.stdin.close()
    p.wait()
    print('video', yol.stat().st_size, n, 'kare', round(n / VFPS, 1), 'sn')


gif_yap()
if FFMPEG:
    video_yap()
