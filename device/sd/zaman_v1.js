// zaman.js — KÖLE Merkez PWA v6 (22.10.2026, Claude; madde 88): Kayıtlar > Time-lapse ve Fotoğraflar.
// Merkez v10 gerekir (/api/zaman/*, /api/foto/*); eski Merkez 404 verirse kartlar gizli kalır.
// - Time-lapse ÇEK biçimi: her kare ayrı istenir (GET /api/zaman/kare?gun=&ofs=&atla=), görüntüden sonra yenisi ->
//   Tailscale aktarıcısında kare birikmez. 1x = 5 kare/sn (panel arşivi ile aynı); 4x/8x kare atlayarak (en çok ~10 istek/sn).
// - Kip/aralık panel 0.55 ile aynı bayraklar (POST /api/zaman/mod); açmak parolalı (sıkı kip) ve denetime yazılır.
// - Kareler ve fotoğraflar yalnız bellekte (nesne URL'si); sayfadan çıkınca bırakılır.
'use strict';
const ZL = { gun: null, ofs: 0, boyut: 0, oynuyor: false, hiz: 1, zam: null, url: null, ac: null, gunler: [], kare: 0, nesil: 0 };
const ZL_ARALIK = [[10, '10 sn'], [30, '30 sn'], [60, '1 dk'], [300, '5 dk']];
const FOTO = { liste: [], goster: 12, buyukUrl: null };

const zlGunMetni = (g) => g.slice(6) + '.' + g.slice(4, 6) + '.' + g.slice(0, 4);
const zlMB = (b) => (b / 1048576).toFixed(1).replace('.', ',') + ' MB';
const zlAtla = () => (ZL.hiz >= 8 ? 3 : ZL.hiz >= 4 ? 1 : 0);

function zlKartlarGizle() { $('#zl-kart').hidden = true; $('#foto-kart').hidden = true; }

async function zlYenile() {
  let j;
  try { j = await api('/api/zaman/liste'); } catch (e) {
    if (e.durum === 404) { zlKartlarGizle(); return; }
    $('#zl-kart').hidden = false;
    $('#zl-hata').textContent = e.message;
    return;
  }
  $('#zl-kart').hidden = false;
  $('#zl-hata').textContent = '';
  $('#zl-acik').checked = !!j.acik;
  $('#zl-acik-metin').textContent = j.acik ? 'Açık · her ' + (ZL_ARALIK.find((a) => a[0] === j.aralik) || [0, j.aralik + ' sn'])[1] : 'Kapalı';
  $$('#zl-aralik button').forEach((b) => b.classList.toggle('secili', Number(b.dataset.sn) === j.aralik));
  $('#zl-kalkan').hidden = !j.kalkan;
  ZL.gunler = j.liste || [];
  const g = $('#zl-gunler');
  g.textContent = '';
  if (!ZL.gunler.length) g.append(el('p', 'soluk', 'Henüz time-lapse kaydı yok' + (j.acik ? '.' : ' (kip kapalı).')));
  ZL.gunler.forEach((x) => {
    const b = el('button', ZL.gun === x.gun ? 'secili' : '', zlGunMetni(x.gun));
    b.type = 'button';
    b.dataset.gun = x.gun;
    b.title = zlMB(x.bayt);
    b.addEventListener('click', () => zlSec(x.gun, 0));
    g.append(b);
  });
}

function zlSec(gun, oran) {
  zlDurdur();
  ZL.nesil++;
  ZL.gun = gun;
  const x = ZL.gunler.find((y) => y.gun === gun);
  ZL.boyut = x ? x.bayt : 0;
  ZL.ofs = Math.floor(ZL.boyut * Math.min(Math.max(oran, 0), 0.999));
  ZL.kare = 0;
  $$('#zl-gunler button').forEach((b) => b.classList.toggle('secili', b.dataset.gun === gun));
  ZL.oynuyor = true;
  zlDugmeCiz();
  zlAdim(ZL.nesil);
}

async function zlAdim(nesil) {
  if (!ZL.oynuyor || !ZL.gun || nesil !== ZL.nesil || bolum !== 'kayit') return;
  const t0 = performance.now();
  ZL.ac = new AbortController();
  const zaman = setTimeout(() => ZL.ac.abort(), 15000);
  let r;
  try {
    r = await fetch(`/api/zaman/kare?gun=${ZL.gun}&ofs=${ZL.ofs}&atla=${ZL.kare ? zlAtla() : 0}`,
      { credentials: 'same-origin', cache: 'no-store', signal: ZL.ac.signal });
  } catch (e) {
    clearTimeout(zaman);
    if (ZL.oynuyor && nesil === ZL.nesil) { $('#zl-bilgi').textContent = 'Bağlantı yavaş, yeniden deneniyor…'; ZL.zam = setTimeout(() => zlAdim(nesil), 2000); }
    return;
  }
  clearTimeout(zaman);
  if (nesil !== ZL.nesil) return;
  if (r.status === 204) {
    ZL.oynuyor = false;
    zlDugmeCiz();
    $('#zl-bilgi').textContent = zlGunMetni(ZL.gun) + ' · bitti (' + ZL.kare + ' kare)';
    $('#zl-ilerleme-dolu').style.width = '100%';
    return;
  }
  if (r.status === 401) { oturumBitti(); return; }
  if (r.status === 423) {
    ZL.oynuyor = false; zlDugmeCiz();
    $('#zl-bilgi').textContent = 'Kayıtlar kilitli: yukarıdaki "Kayıtlar kilitli" kartından kilidi açın.';
    if (typeof izinAyarla === 'function') izinAyarla(0);
    return;
  }
  if (!r.ok) {
    const j = await r.json().catch(() => ({}));
    ZL.oynuyor = false; zlDugmeCiz();
    $('#zl-hata').textContent = j.hata || ('HTTP ' + r.status);
    return;
  }
  const blob = await r.blob();
  if (nesil !== ZL.nesil) return;
  ZL.ofs = Number(r.headers.get('X-Sonraki')) || ZL.ofs;
  ZL.boyut = Number(r.headers.get('X-Boyut')) || ZL.boyut;
  const ilerleme = Number(r.headers.get('X-Ilerleme')) || 0;
  const img = $('#zl-img');
  if (ZL.url) URL.revokeObjectURL(ZL.url);
  ZL.url = URL.createObjectURL(blob);
  img.src = ZL.url;
  img.hidden = false;
  $('#zl-bos').hidden = true;
  ZL.kare++;
  $('#zl-ilerleme-dolu').style.width = (ilerleme * 100).toFixed(1) + '%';
  $('#zl-bilgi').textContent = zlGunMetni(ZL.gun) + ' · %' + Math.round(ilerleme * 100) + ' · ' + ZL.hiz + 'x · ' + ZL.kare + ' kare';
  if (!ZL.oynuyor) return;   // duraklatıldı: kare gösterildi, devam yok
  const aralik = 200 * (1 + zlAtla()) / ZL.hiz;   // 1x = 5 kare/sn
  ZL.zam = setTimeout(() => zlAdim(nesil), Math.max(0, aralik - (performance.now() - t0)));
}

function zlDurdur() {
  ZL.oynuyor = false;
  clearTimeout(ZL.zam);
  if (ZL.ac) ZL.ac.abort();
  zlDugmeCiz();
}

function zlBirak() {   // sayfadan çıkınca: akış durur, kare bellekten atılır
  zlDurdur();
  ZL.nesil++;
  if (ZL.url) { URL.revokeObjectURL(ZL.url); ZL.url = null; }
  const img = $('#zl-img');
  if (img) { img.removeAttribute('src'); img.hidden = true; }
  const bos = $('#zl-bos');
  if (bos) bos.hidden = false;
  fotoBuyukKapat();
}

function zlDugmeCiz() {
  const b = $('#zl-oynat');
  if (!b) return;
  b.querySelector('use').setAttribute('href', ZL.oynuyor ? '#i-duraklat' : '#i-oynat');
  b.setAttribute('aria-label', ZL.oynuyor ? 'Duraklat' : 'Oynat');
}

$('#zl-oynat').addEventListener('click', () => {
  if (!ZL.gun) { if (ZL.gunler.length) zlSec(ZL.gunler[0].gun, 0); return; }
  if (ZL.oynuyor) { zlDurdur(); return; }
  ZL.oynuyor = true;
  ZL.nesil++;
  zlDugmeCiz();
  zlAdim(ZL.nesil);
});
$('#zl-ilerleme').addEventListener('click', (e) => {
  if (!ZL.gun) return;
  const k = e.currentTarget.getBoundingClientRect();
  zlSec(ZL.gun, (e.clientX - k.left) / k.width);
});
$$('#zl-hiz button').forEach((b) => b.addEventListener('click', () => {
  ZL.hiz = Number(b.dataset.hiz);
  $$('#zl-hiz button').forEach((x) => x.classList.toggle('secili', x === b));
}));
$('#zl-tam').addEventListener('click', () => { if (typeof tamEkran === 'function') tamEkran($('#zl-sahne')); });
$('#zl-anlik').addEventListener('click', () => {
  if (!ZL.url) return;
  const a = el('a');
  a.href = ZL.url;
  a.download = 'kole-timelapse-' + (ZL.gun || '') + '-' + ZL.kare + '.jpg';
  a.click();
});

$('#zl-acik').addEventListener('change', async (e) => {
  const acik = e.target.checked;
  let govde = { acik };
  if (acik) {
    const c = await sor({
      baslik: 'Time-lapse açılsın mı?',
      metin: 'Kamera seçtiğiniz aralıkla birer kare alıp günlük time-lapse dosyasına ekler (30 gün saklanır). '
        + (tekGiris() ? 'Açılış denetim günlüğüne yazılır.' : 'Onaylamak için parolanızı girin.'),
      sifre: !tekGiris(), tamam: 'Aç', tehlike: true,
    });
    if (!c.ok) { e.target.checked = false; return; }
    if (c.sifre) govde.sifre = c.sifre;
  }
  try { await api('/api/zaman/mod', govde); bildir(acik ? 'Time-lapse açıldı' : 'Time-lapse kapatıldı'); $('#zl-hata').textContent = ''; }
  catch (err) { $('#zl-hata').textContent = err.message; }
  zlYenile();
});
$$('#zl-aralik button').forEach((b) => b.addEventListener('click', async () => {
  try { await api('/api/zaman/mod', { aralik: Number(b.dataset.sn) }); bildir('Time-lapse aralığı: ' + b.textContent); }
  catch (err) { $('#zl-hata').textContent = err.message; }
  zlYenile();
}));

// ---------------------------------------------------------------- fotoğraflar
async function fotoYenile() {
  let j;
  try { j = await api('/api/foto/liste'); } catch (e) {
    if (e.durum === 404) { $('#foto-kart').hidden = true; return; }
    $('#foto-kart').hidden = false;
    $('#foto-hata').textContent = e.message;
    return;
  }
  $('#foto-kart').hidden = false;
  $('#foto-hata').textContent = '';
  FOTO.liste = j.liste || [];
  FOTO.goster = 12;
  fotoCiz();
}

function fotoUrl(f) { return '/api/foto/al?k=' + encodeURIComponent(f.k) + '&ad=' + encodeURIComponent(f.ad); }

function fotoCiz() {
  const iz = $('#foto-izgara');
  iz.textContent = '';
  $('#foto-sayi').textContent = FOTO.liste.length ? FOTO.liste.length + ' fotoğraf' : '';
  if (!FOTO.liste.length) { iz.append(el('p', 'soluk', 'Fotoğraf yok (kamera fotoğrafları ve çerçeve klasörü boş).')); }
  if (!izinVar()) { iz.append(el('p', 'soluk', 'Fotoğrafları görmek için kayıt kilidini açın.')); }
  FOTO.liste.slice(0, FOTO.goster).forEach((f) => {
    const b = el('button', 'foto-karo');
    b.type = 'button';
    const d = new Date(f.t * 1000);
    b.title = f.ad + ' · ' + d.toLocaleString('tr-TR') + ' · ' + zlMB(f.bayt);
    if (izinVar()) {
      const im = el('img');
      im.loading = 'lazy';
      im.alt = f.ad;
      im.src = fotoUrl(f);
      b.append(im);
    }
    const etiket = el('span', 'foto-etiket', (f.k === 'kamera' ? 'Kamera · ' : 'Çerçeve · ') + iki(d.getDate()) + '.' + iki(d.getMonth() + 1) + ' ' + iki(d.getHours()) + ':' + iki(d.getMinutes()));
    b.append(etiket);
    b.addEventListener('click', () => fotoBuyukAc(f));
    iz.append(b);
  });
  $('#foto-daha').hidden = FOTO.liste.length <= FOTO.goster;
}

async function fotoBuyukAc(f) {
  let r;
  try { r = await fetch(fotoUrl(f), { credentials: 'same-origin', cache: 'no-store' }); } catch (e) { $('#foto-hata').textContent = 'Merkeze ulaşılamadı'; return; }
  if (r.status === 423) { $('#foto-hata').textContent = 'Fotoğraflar için kayıt kilidini açın.'; return; }
  if (!r.ok) { const j = await r.json().catch(() => ({})); $('#foto-hata').textContent = j.hata || ('HTTP ' + r.status); return; }
  fotoBuyukKapat();
  FOTO.buyukUrl = URL.createObjectURL(await r.blob());
  $('#foto-buyuk-img').src = FOTO.buyukUrl;
  $('#foto-buyuk-ad').textContent = f.ad;
  const a = $('#foto-buyuk-indir');
  a.href = FOTO.buyukUrl;
  a.download = f.ad;
  $('#foto-buyuk').hidden = false;
}

function fotoBuyukKapat() {
  const k = $('#foto-buyuk');
  if (k) k.hidden = true;
  if (FOTO.buyukUrl) { URL.revokeObjectURL(FOTO.buyukUrl); FOTO.buyukUrl = null; }
  const im = $('#foto-buyuk-img');
  if (im) im.removeAttribute('src');
}

$('#foto-daha').addEventListener('click', () => { FOTO.goster += 12; fotoCiz(); });
$('#foto-yenile').addEventListener('click', fotoYenile);
$('#zl-yenile').addEventListener('click', zlYenile);
$('#foto-buyuk-kapat').addEventListener('click', fotoBuyukKapat);
$('#foto-buyuk').addEventListener('click', (e) => { if (e.target.id === 'foto-buyuk') fotoBuyukKapat(); });
document.addEventListener('keydown', (e) => { if (e.key === 'Escape' && !$('#foto-buyuk').hidden) fotoBuyukKapat(); });
