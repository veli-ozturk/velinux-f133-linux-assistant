/* Panel 0.24 (13.10.2026, Claude): "Robot goz" ekran koruyucusu - SAF mantik (LVGL / dosya / saat cagrisi YOK).
 *
 * Iki buyuk ifadeli goz (Vector/EMO tarzi yuvarlak dikdortgen + gozbebegi + parilti). Davranis durum makinesi
 * tohumlu RNG (xorshift32) ile rastgele zamanlanir; ayni tohum + ayni girdi zamanlari = ayni poz dizisi (host testi).
 * Davranislar: bos (mikro bakis), kirp (tek/cift), bak (sol/sag/yukari/asagi/capraz sicrama), gezin, suphe (kisik),
 * mutlu (^ ^ yaylar + ziplama), saskin (iri), uykulu (yavas dusus), uyu (kapali + zzz; gece 23-07 baskin),
 * kizgin (nadir, egik kapak + titreme), kalp (nadir), tara (yatay supurme).
 * Tepkiler: kamera (/tmp/kamera-aktif) -> kirmizi "izleniyor" bakisi (hap ana programda), mikrofon -> "dinliyor"
 * (egik bas), dokunma -> dokunulan yere bakis + mutlu kirpma (ana program sonra koruyucuyu kapatir),
 * bildirim -> saskin. Kamera gece de uyandirir (gizlilik gorunur kalmali).
 *
 * Basarim: poz ancak gecis sirasinda degisir; rg_sonraki_ms() gecis sirasinda 40 ms (<= 25 fps) ya da yavas gecislerde
 * 80 ms, durgunken sonraki olaya kadar (en cok 1 sn) bekleme verir. rg_goz_hesapla() her gozun kapsayan kutusunu
 * (parilti dahil) verir: ana program yalniz (eski U yeni) kutulari gecersiz kilar.
 * Kullanim: tests/panel_024_test_v1.c (host), main_pc_oturum_v5.c (LVGL cizimi). */
#ifndef VELINUX_ROBOT_GOZ_V1_H
#define VELINUX_ROBOT_GOZ_V1_H
#include <stdint.h>
#include <string.h>

#define RG_W 1600
#define RG_H 600
#define RG_KARE_HIZLI 40	/* ms: hizli gecis (sicrama, kirpma) <= 25 fps */
#define RG_KARE_YAVAS 80	/* ms: yavas gecis (uyku, suzulme) 12.5 fps */
#define RG_BEKLE_AZAMI 1000	/* ms: durgunken en uzun bekleme */
#define RG_ZZZ_MS 250		/* zzz adimi (4 fps) */
#define RG_KUYRUK 20
#define RG_PARILTI 14		/* parilti halkasi genisligi (px) */

enum { RG_S_NORMAL, RG_S_MUTLU, RG_S_KAPALI, RG_S_KALP };
enum { RG_R_TEMA, RG_R_KIRMIZI };
enum rg_davranis {
	RG_BOS, RG_KIRP, RG_BAK, RG_GEZIN, RG_SUPHE, RG_MUTLU, RG_SASKIN, RG_UYKULU, RG_UYU, RG_KIZGIN, RG_KALP, RG_TARA,
	RG_IZLENIYOR, RG_DINLIYOR, RG_DOKUNMA, RG_UYAN, RG_D_N
};
static const char *const rg_davranis_adi[RG_D_N] = {
	"bos", "kirp", "bak", "gezin", "suphe", "mutlu", "saskin", "uykulu", "uyu", "kizgin", "kalp", "tara",
	"izleniyor", "dinliyor", "dokunma", "uyan"};

typedef struct {
	int16_t tx, ty;		/* taban kaymasi (ekran izi kalmasin), px */
	int16_t gx, gy;		/* bakis: iki goz birlikte, px */
	int16_t px, py;		/* gozbebegi ek kaymasi, px */
	int16_t olcek;		/* boyut, binde (1000 = yerlesim boyutu) */
	int16_t acik;		/* kapak acikligi, binde (iki goz) */
	int16_t sol_acik;	/* sol goz ek aciklik carpani, binde (goz kirpma / tek goz aralama) */
	int16_t ust_ic, ust_dis;	/* ust kapak kesigi ic / dis kosede, binde (kizgin: ic yuksek) */
	int16_t alt;		/* alt kapak, binde (suphe) */
	int16_t asim;		/* sag goz boy farki, binde (sol goz -asim): egik bas */
	uint8_t sekil, renk;
	uint8_t zzz;		/* 0 yok, 1..12 zzz fazi */
	uint8_t bos_;
} rg_poz_t;

typedef struct {
	rg_poz_t p;
	uint16_t gecis, bekle;	/* ms */
	uint8_t yavas;
} rg_kare_t;

typedef struct {
	uint32_t rng;
	int davranis, onceki;
	rg_kare_t q[RG_KUYRUK];
	int qn, qi;
	rg_poz_t bas, hedef, simdi;
	uint32_t t0;		/* gecerli karenin baslangici (ms) */
	uint16_t gecis, bekle;
	uint8_t yavas;
	int kam, mik, gece;
	int16_t taban_x, taban_y;
	int dinle_yon;
	uint32_t uyku_t0;
	uint32_t sayac[RG_D_N];	/* davranis baslatma sayaci (test/gunluk) */
	int baslatildi;
} rg_robot_t;

/* Yerlesim: goz merkezleri (cx -+ ara/2, cy), taban goz boyutu w x h */
typedef struct {
	int16_t cx, cy, w, h, ara;
} rg_yerlesim_t;
static const rg_yerlesim_t RG_YER_ROBOT = {800, 292, 250, 280, 520};
static const rg_yerlesim_t RG_YER_KARISIK = {800, 252, 210, 236, 440};

/* Bir gozun cizim geometrisi (ekran koordinati). pr = 0: gozbebegi yok. kutu: cizilen her seyi kapsar. */
typedef struct {
	int16_t x1, y1, x2, y2, r;	/* govde */
	int16_t pcx, pcy, pr;		/* gozbebegi */
	int16_t hx, hy, hr;		/* parilti noktasi */
	int16_t ust_ic_y, ust_dis_y, alt_y;	/* kapak cizgileri (ust: y1'den buyukse kapak var; alt: y2'den kucukse) */
	int16_t ic_x, dis_x;		/* ic / dis kenar x (sol gozde ic = sag kenar) */
	int16_t cx, cy, olc;		/* sekil merkezi ve olcusu (yay/kalp icin) */
	uint8_t sekil, parilti;
	int16_t kx1, ky1, kx2, ky2;	/* kapsayan kutu (ekrana kirpilmis) */
} rg_goz_t;

static uint32_t rg_rand(rg_robot_t *r)
{
	uint32_t x = r->rng;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	r->rng = x;
	return x;
}

/* [a, b] araliginda tamsayi */
static int rg_ara(rg_robot_t *r, int a, int b)
{
	return b <= a ? a : a + (int)(rg_rand(r) % (uint32_t)(b - a + 1));
}

static int rg_isaret(rg_robot_t *r)
{
	return rg_rand(r) & 1 ? 1 : -1;
}

static rg_poz_t rg_temel(const rg_robot_t *r)
{
	rg_poz_t p;
	memset(&p, 0, sizeof p);
	p.tx = r->taban_x;
	p.ty = r->taban_y;
	p.olcek = 1000;
	p.acik = 1000;
	p.sol_acik = 1000;
	p.renk = r->kam ? RG_R_KIRMIZI : RG_R_TEMA;
	return p;
}

static void rg_ekle(rg_robot_t *r, rg_poz_t p, int gecis, int bekle, int yavas)
{
	if (r->qn >= RG_KUYRUK)
		return;
	rg_kare_t *k = &r->q[r->qn++];
	k->p = p;
	k->gecis = (uint16_t)(gecis < 0 ? 0 : gecis);
	k->bekle = (uint16_t)(bekle < 0 ? 0 : bekle);
	k->yavas = (uint8_t)yavas;
}

/* p pozunda goz kirp (kapanip acilir), sonra bekle */
static void rg_kirp_ekle(rg_robot_t *r, rg_poz_t p, int bekle)
{
	rg_poz_t k = p;
	k.acik = 60;
	rg_ekle(r, k, 80, 30, 0);
	rg_ekle(r, p, 110, bekle, 0);
}

/* bakis yonune gore gozbebegi paralaksi */
static void rg_bakis(rg_poz_t *p, int gx, int gy)
{
	p->gx = (int16_t)gx;
	p->gy = (int16_t)gy;
	p->px = (int16_t)(gx / 5);
	p->py = (int16_t)(gy / 4);
}

static void rg_davranis_kur(rg_robot_t *r, int d)
{
	rg_poz_t t = rg_temel(r), p, p2;
	int i, n, s;

	r->qn = r->qi = 0;
	if (d < 0 || d >= RG_D_N)
		d = RG_BOS;
	r->onceki = r->davranis;
	r->davranis = d;
	r->sayac[d]++;
	switch (d) {
	case RG_BOS:
		p = t;
		rg_bakis(&p, rg_ara(r, -16, 16), rg_ara(r, -10, 10));
		rg_ekle(r, p, 120, rg_ara(r, 1500, 4000), 0);
		if (rg_ara(r, 0, 99) < 60)
			rg_kirp_ekle(r, p, rg_ara(r, 200, 600));
		break;
	case RG_KIRP:
		p = t;
		rg_bakis(&p, r->hedef.sekil == RG_S_NORMAL ? r->hedef.gx : 0, r->hedef.sekil == RG_S_NORMAL ? r->hedef.gy : 0);
		rg_kirp_ekle(r, p, rg_ara(r, 300, 900));
		if (rg_ara(r, 0, 99) < 30)
			rg_kirp_ekle(r, p, rg_ara(r, 400, 900));
		break;
	case RG_BAK:
		p = t;
		s = rg_ara(r, 0, 7);	/* 8 yon */
		{
			static const int8_t yx[8] = {-1, 1, 0, 0, -1, 1, -1, 1}, yy[8] = {0, 0, -1, 1, -1, -1, 1, 1};
			rg_bakis(&p, yx[s] * rg_ara(r, 110, 190), yy[s] * rg_ara(r, 45, 78));
		}
		rg_ekle(r, p, rg_ara(r, 90, 140), rg_ara(r, 700, 1800), 0);
		if (rg_ara(r, 0, 99) < 35)
			rg_kirp_ekle(r, p, rg_ara(r, 300, 700));
		if (rg_ara(r, 0, 99) < 55)
			rg_ekle(r, t, 120, rg_ara(r, 400, 1000), 0);
		break;
	case RG_GEZIN:
		n = rg_ara(r, 3, 5);
		for (i = 0; i < n; i++) {
			p = t;
			rg_bakis(&p, rg_ara(r, -180, 180), rg_ara(r, -70, 70));
			rg_ekle(r, p, rg_ara(r, 90, 150), rg_ara(r, 250, 700), 0);
		}
		rg_ekle(r, t, 130, rg_ara(r, 300, 800), 0);
		break;
	case RG_SUPHE:
		s = rg_isaret(r);
		p = t;
		p.acik = 520;
		p.alt = 170;
		p.sol_acik = 760;
		rg_ekle(r, p, 260, 500, 0);
		rg_bakis(&p, s * 130, 6);
		rg_ekle(r, p, 700, rg_ara(r, 700, 1100), 1);
		rg_bakis(&p, -s * 130, 6);
		rg_ekle(r, p, 900, rg_ara(r, 700, 1100), 1);
		rg_ekle(r, t, 220, rg_ara(r, 300, 700), 0);
		break;
	case RG_MUTLU:
		p = t;
		p.sekil = RG_S_MUTLU;
		p.gy = -8;
		rg_ekle(r, p, 200, 200, 0);
		p2 = p;
		p2.gy = -34;
		for (i = 0; i < 2; i++) {
			rg_ekle(r, p2, 120, 50, 0);
			rg_ekle(r, p, 120, i ? rg_ara(r, 700, 1300) : 60, 0);
		}
		rg_ekle(r, t, 200, rg_ara(r, 300, 700), 0);
		break;
	case RG_SASKIN:
		p = t;
		p.olcek = 1240;
		p.gy = -16;
		rg_ekle(r, p, 110, rg_ara(r, 800, 1200), 0);
		rg_ekle(r, t, 220, rg_ara(r, 300, 600), 0);
		break;
	case RG_UYKULU:
		p = t;
		p.acik = 380;
		p.gy = 16;
		rg_ekle(r, p, 1400, 700, 1);
		p.acik = 160;
		p.gy = 22;
		rg_ekle(r, p, 1000, 500, 1);
		if (!r->gece) {	/* gunduz: silkinip uyan (ya da kisa sekerleme, sonra UYU secilir) */
			rg_ekle(r, t, 150, 200, 0);
			rg_kirp_ekle(r, t, rg_ara(r, 400, 900));
		}
		break;
	case RG_UYU:
		if (r->gece && r->onceki == RG_UYU && rg_ara(r, 0, 99) < 20) {	/* tek gozu aralayip bak */
			p = t;
			p.acik = 430;
			p.sol_acik = 0;
			p.gy = 16;
			rg_bakis(&p, rg_ara(r, -90, 90), 16);
			rg_ekle(r, p, 400, 1200, 1);
		}
		p = t;
		p.sekil = RG_S_KAPALI;
		p.gy = 24;
		rg_ekle(r, p, 500, r->gece ? rg_ara(r, 20000, 60000) : rg_ara(r, 3000, 5000), 1);
		break;
	case RG_UYAN:
		p = t;
		p.acik = 420;
		rg_ekle(r, p, 500, 300, 1);
		rg_kirp_ekle(r, t, 150);
		rg_kirp_ekle(r, t, rg_ara(r, 500, 900));
		break;
	case RG_KIZGIN:
		p = t;
		p.ust_ic = 420;
		p.ust_dis = 60;
		p.acik = 900;
		p.olcek = 970;
		p.gy = 8;
		rg_ekle(r, p, 160, 300, 0);
		for (i = 0; i < 4; i++) {
			p2 = p;
			p2.gx = (int16_t)(i & 1 ? -14 : 14);
			rg_ekle(r, p2, 50, 0, 0);
		}
		rg_ekle(r, p, 50, rg_ara(r, 800, 1200), 0);
		rg_ekle(r, t, 220, rg_ara(r, 300, 600), 0);
		break;
	case RG_KALP:
		p = t;
		p.sekil = RG_S_KALP;
		p.olcek = 900;
		rg_ekle(r, p, 220, 150, 0);
		for (i = 0; i < 3; i++) {
			p2 = p;
			p2.olcek = 1060;
			rg_ekle(r, p2, 180, 60, 0);
			rg_ekle(r, p, 180, i == 2 ? 600 : 60, 0);
		}
		rg_ekle(r, t, 240, rg_ara(r, 300, 600), 0);
		break;
	case RG_TARA:
		s = rg_isaret(r);
		p = t;
		p.acik = 430;
		rg_bakis(&p, -s * 185, 0);
		rg_ekle(r, p, 200, 200, 0);
		rg_bakis(&p, s * 185, 0);
		rg_ekle(r, p, 1500, 200, 0);
		rg_bakis(&p, 0, 0);
		rg_ekle(r, p, 300, 150, 0);
		rg_ekle(r, t, 160, rg_ara(r, 300, 700), 0);
		break;
	case RG_IZLENIYOR:	/* kamera acik: kirmizi, odakli kisik bakis, ara sira yana kacamak */
		p = t;
		p.acik = 600;
		p.ust_ic = 150;
		p.alt = 60;
		rg_bakis(&p, 0, -22);
		rg_ekle(r, p, 220, rg_ara(r, 1800, 3500), 0);
		p2 = p;
		rg_bakis(&p2, rg_isaret(r) * rg_ara(r, 80, 150), -14);
		rg_ekle(r, p2, 130, rg_ara(r, 600, 1200), 0);
		rg_ekle(r, p, 130, 300, 0);
		rg_kirp_ekle(r, p, rg_ara(r, 600, 1400));
		break;
	case RG_DINLIYOR:	/* mikrofon acik: basi egik, yukari-yana dikkatle */
		r->dinle_yon = r->dinle_yon > 0 ? -1 : 1;
		p = t;
		p.asim = (int16_t)(110 * r->dinle_yon);
		rg_bakis(&p, 70 * r->dinle_yon, -42);
		rg_ekle(r, p, 260, rg_ara(r, 2200, 4000), 0);
		rg_kirp_ekle(r, p, rg_ara(r, 800, 1600));
		break;
	default:
		break;
	}
	if (!r->qn)
		rg_ekle(r, t, 120, 1000, 0);
}

/* kuyruk bittiginde sonraki davranis: tepkiler > gece > agirlikli rastgele */
static void rg_sec(rg_robot_t *r)
{
	if (r->kam) {
		rg_davranis_kur(r, RG_IZLENIYOR);
		return;
	}
	if (r->mik) {
		rg_davranis_kur(r, RG_DINLIYOR);
		return;
	}
	if (r->gece) {
		rg_davranis_kur(r, r->davranis == RG_UYU || r->davranis == RG_UYKULU ? RG_UYU : RG_UYKULU);
		return;
	}
	if (r->davranis == RG_UYU) {
		rg_davranis_kur(r, RG_UYAN);
		return;
	}
	if (r->davranis == RG_UYKULU && rg_ara(r, 0, 99) < 35) {	/* gunduz kisa sekerleme */
		rg_davranis_kur(r, RG_UYU);
		return;
	}
	static const uint8_t agirlik[RG_TARA + 1] = {
		/* bos kirp bak gezin suphe mutlu saskin uykulu uyu kizgin kalp tara */
		26, 18, 20, 9, 5, 7, 2, 4, 0, 2, 1, 6};
	int toplam = 0, i;
	for (i = 0; i <= RG_TARA; i++)
		toplam += agirlik[i];
	int x = rg_ara(r, 0, toplam - 1);
	for (i = 0; i <= RG_TARA; i++) {
		if (x < agirlik[i])
			break;
		x -= agirlik[i];
	}
	if (i > RG_TARA)
		i = RG_BOS;
	/* ayni "nadir/ifade" davranisi art arda gelmesin */
	if (i == r->davranis && i != RG_BOS && i != RG_KIRP && i != RG_BAK)
		i = RG_BOS;
	rg_davranis_kur(r, i);
}

/* gecerli gecisi kes: yeni kare simdiki pozdan baslar */
static void rg_kes(rg_robot_t *r, uint32_t now)
{
	r->bas = r->simdi;
	r->hedef = r->simdi;
	r->qn = r->qi = 0;
	r->t0 = now;
	r->gecis = r->bekle = 0;
}

static void rg_baslat(rg_robot_t *r, uint32_t tohum, uint32_t now)
{
	memset(r, 0, sizeof *r);
	r->rng = tohum ? tohum : 0x9E3779B9u;
	for (int i = 0; i < 4; i++)	/* kucuk tohumlar da iyi karissin */
		rg_rand(r);
	r->dinle_yon = 1;
	r->davranis = r->onceki = RG_BOS;
	rg_poz_t t = rg_temel(r);
	t.acik = 60;
	r->bas = r->hedef = r->simdi = t;
	r->t0 = now;
	r->gecis = 0;
	r->bekle = 300;	/* kapali gozle kisa bekleme, sonra acilis */
	rg_davranis_kur(r, RG_UYAN);
	r->baslatildi = 1;
}

static int16_t rg_lerp(int16_t a, int16_t b, int32_t k)
{
	return (int16_t)(a + (int32_t)(b - a) * k / 1024);
}

/* zaman ilerlet; poz degistiyse 1 */
static int rg_adim(rg_robot_t *r, uint32_t now)
{
	rg_poz_t eski = r->simdi;
	int dongu = 0;

	for (;;) {
		int32_t e = (int32_t)(now - r->t0);
		if (e < 0)
			e = 0;
		if (e < r->gecis) {
			int32_t s = e * 1024 / r->gecis;
			int32_t k = ((s * s) >> 10) * (3072 - 2 * s) >> 10;	/* smoothstep */
			rg_poz_t *a = &r->bas, *b = &r->hedef, *c = &r->simdi;
			*c = *b;
			c->tx = rg_lerp(a->tx, b->tx, k);
			c->ty = rg_lerp(a->ty, b->ty, k);
			c->gx = rg_lerp(a->gx, b->gx, k);
			c->gy = rg_lerp(a->gy, b->gy, k);
			c->px = rg_lerp(a->px, b->px, k);
			c->py = rg_lerp(a->py, b->py, k);
			c->olcek = rg_lerp(a->olcek, b->olcek, k);
			c->acik = rg_lerp(a->acik, b->acik, k);
			c->sol_acik = rg_lerp(a->sol_acik, b->sol_acik, k);
			c->ust_ic = rg_lerp(a->ust_ic, b->ust_ic, k);
			c->ust_dis = rg_lerp(a->ust_dis, b->ust_dis, k);
			c->alt = rg_lerp(a->alt, b->alt, k);
			c->asim = rg_lerp(a->asim, b->asim, k);
			if (a->sekil != b->sekil) {	/* sekil degisimi: kapanip yeni sekille acilir */
				int32_t m = k < 512 ? 1024 - 2 * k : 2 * k - 1024;
				c->sekil = k < 512 ? a->sekil : b->sekil;
				if (c->sekil == RG_S_NORMAL)
					c->acik = (int16_t)(c->acik * m / 1024);
				else
					c->olcek = (int16_t)(c->olcek * (300 + 700 * m / 1024) / 1000);
			}
			break;
		}
		if (e < (int32_t)r->gecis + r->bekle) {
			r->simdi = r->hedef;
			break;
		}
		/* sonraki kare */
		if (++dongu > 64) {	/* uzun takilma (cihaz donmasi): zaman cizgisini simdiye cek */
			r->t0 = now;
			r->gecis = r->bekle = 0;
			r->simdi = r->hedef;
			break;
		}
		if (r->qi >= r->qn)
			rg_sec(r);
		rg_kare_t *k = &r->q[r->qi++];
		r->t0 += (uint32_t)r->gecis + r->bekle;
		r->bas = r->hedef;
		r->hedef = k->p;
		r->gecis = k->gecis;
		r->bekle = k->bekle;
		r->yavas = k->yavas;
		if (k->p.sekil == RG_S_KAPALI && r->bas.sekil != RG_S_KAPALI)
			r->uyku_t0 = r->t0;
	}
	r->simdi.zzz = 0;
	if (r->davranis == RG_UYU && r->simdi.sekil == RG_S_KAPALI)
		r->simdi.zzz = (uint8_t)(1 + ((now - r->uyku_t0) / RG_ZZZ_MS) % 12);
	return memcmp(&eski, &r->simdi, sizeof eski) != 0;
}

/* sonraki guncellemeye kadar bekleme (ms) */
static uint32_t rg_sonraki_ms(const rg_robot_t *r, uint32_t now)
{
	int32_t e = (int32_t)(now - r->t0);
	if (e < 0)
		e = 0;
	if (e < r->gecis)
		return r->yavas ? RG_KARE_YAVAS : RG_KARE_HIZLI;
	int32_t kalan = (int32_t)r->gecis + r->bekle - e;
	if (r->simdi.zzz) {
		int32_t z = RG_ZZZ_MS - (int32_t)((now - r->uyku_t0) % RG_ZZZ_MS);
		if (z < kalan)
			kalan = z;
	}
	if (kalan < RG_KARE_HIZLI)
		kalan = RG_KARE_HIZLI;
	if (kalan > RG_BEKLE_AZAMI)
		kalan = RG_BEKLE_AZAMI;
	return (uint32_t)kalan;
}

/* girdiler (saniyede bir): kamera / mikrofon bayragi, gece (23-07) */
static void rg_girdi(rg_robot_t *r, int kam, int mik, int gece, uint32_t now)
{
	kam = !!kam;
	mik = !!mik;
	gece = !!gece;
	int dk = kam != r->kam, dm = mik != r->mik, dg = gece != r->gece;
	r->kam = kam;
	r->mik = mik;
	r->gece = gece;
	if (r->davranis == RG_DOKUNMA)
		return;	/* dokunma tepkisi bitsin (koruyucu zaten kapanacak) */
	if (dk) {
		rg_kes(r, now);
		rg_davranis_kur(r, kam ? RG_IZLENIYOR : RG_MUTLU);	/* kapaninca rahatlama */
	} else if (dm && !kam) {
		rg_kes(r, now);
		rg_davranis_kur(r, mik ? RG_DINLIYOR : RG_KIRP);
	} else if (dg && !kam && !mik && !gece && r->davranis == RG_UYU) {
		rg_kes(r, now);
		rg_davranis_kur(r, RG_UYAN);
	}
}

/* dokunma: dokunulan yere bak + mutlu kirpma; donus = koruyucu kapanmadan once beklenecek ms */
static uint32_t rg_dokun(rg_robot_t *r, int x, int y, const rg_yerlesim_t *yer, uint32_t now)
{
	rg_kes(r, now);
	r->qn = r->qi = 0;
	r->onceki = r->davranis;
	r->davranis = RG_DOKUNMA;
	r->sayac[RG_DOKUNMA]++;
	rg_poz_t p = rg_temel(r);
	int gx = (x - (yer->cx + r->taban_x)) * 200 / 700, gy = (y - (yer->cy + r->taban_y)) * 80 / 260;
	gx = gx < -200 ? -200 : gx > 200 ? 200 : gx;
	gy = gy < -80 ? -80 : gy > 80 ? 80 : gy;
	rg_bakis(&p, gx, gy);
	rg_ekle(r, p, 90, 90, 0);
	rg_poz_t m = p;
	m.sekil = RG_S_MUTLU;
	m.gy = (int16_t)(p.gy - 12);
	rg_ekle(r, m, 160, 260, 0);
	return 90 + 90 + 160 + 260;
}

/* bildirim: saskin (dokunma tepkisi sirasinda yok sayilir) */
static void rg_bildirim(rg_robot_t *r, uint32_t now)
{
	if (r->davranis == RG_DOKUNMA)
		return;
	rg_kes(r, now);
	rg_davranis_kur(r, RG_SASKIN);
}

/* uzaktan / test: davranisi hemen baslat (dokunma haric). Ek adlar: sol/sag/yukari/asagi = o yone bak. */
static int rg_zorla(rg_robot_t *r, const char *ad, uint32_t now)
{
	static const char *const yon_ad[4] = {"sol", "sag", "yukari", "asagi"};
	static const int8_t yx[4] = {-1, 1, 0, 0}, yy[4] = {0, 0, -1, 1};
	for (int i = 0; i < 4; i++)
		if (!strcmp(ad, yon_ad[i])) {
			rg_kes(r, now);
			rg_davranis_kur(r, RG_BAK);
			r->qn = r->qi = 0;
			rg_poz_t t = rg_temel(r), p = t;
			rg_bakis(&p, yx[i] * 170, yy[i] * 70);
			rg_ekle(r, p, 110, 2500, 0);
			rg_ekle(r, t, 120, 600, 0);
			return RG_BAK;
		}
	for (int i = 0; i < RG_D_N; i++)
		if (i != RG_DOKUNMA && !strcmp(ad, rg_davranis_adi[i])) {
			rg_kes(r, now);
			rg_davranis_kur(r, i);
			return i;
		}
	return -1;
}

/* taban kaymasi (dakikada bir; sonraki karelerle yumusakca uygulanir) */
static void rg_taban(rg_robot_t *r, int x, int y)
{
	r->taban_x = (int16_t)x;
	r->taban_y = (int16_t)y;
}

static int16_t rg_kirp16(int v, int a, int b)
{
	return (int16_t)(v < a ? a : v > b ? b : v);
}

/* zzz harflerinin alani (sag gozun sag ustu): capa noktasi (ax, ay); harfler [ax, ax+RG_ZZZ_GEN] x
 * [ay-RG_ZZZ_YUK, ay] icinde kalir (en buyuk harf 64 px yazitipi). */
#define RG_ZZZ_GEN 200
#define RG_ZZZ_YUK 200

static void rg_zzz_capa(const rg_poz_t *p, const rg_yerlesim_t *y, int *ax, int *ay)
{
	int persp = p->gx * 70 / 200, o = p->olcek + persp;
	int w = y->w * o / 1000, h = y->h * (o + p->asim) / 1000;
	*ax = y->cx + y->ara / 2 + p->tx + p->gx + w / 2 - 10;
	*ay = y->cy + p->ty + p->gy - h / 2 + 30;
}

/* sag: 0 sol goz (ekranda solda), 1 sag goz */
static void rg_goz_hesapla(const rg_poz_t *p, int sag, const rg_yerlesim_t *y, rg_goz_t *g)
{
	int persp = (sag ? p->gx : -p->gx) * 70 / 200;	/* bakilan yondeki goz biraz buyur (3B his) */
	int ow = p->olcek + persp, oh = p->olcek + persp + (sag ? p->asim : -p->asim);
	int w = y->w * ow / 1000, h = y->h * oh / 1000;
	int a = p->acik * (sag ? 1000 : p->sol_acik) / 1000;
	int x = y->cx + (sag ? 1 : -1) * y->ara / 2 + p->tx + p->gx;
	int cy = y->cy + p->ty + p->gy;
	int he, G = RG_PARILTI + 3;

	memset(g, 0, sizeof *g);
	if (w < 20)
		w = 20;
	if (h < 20)
		h = 20;
	g->sekil = p->sekil;
	g->cx = (int16_t)x;
	g->cy = (int16_t)cy;
	g->olc = (int16_t)w;
	if (p->sekil == RG_S_NORMAL) {
		he = h * a / 1000;
		if (he < 14)
			he = 14;
		cy += (h - he) / 4;	/* kapak ustten daha cok iner */
		g->x1 = (int16_t)(x - w / 2);
		g->x2 = (int16_t)(x - w / 2 + w - 1);
		g->y1 = (int16_t)(cy - he / 2);
		g->y2 = (int16_t)(cy - he / 2 + he - 1);
		int kisa = w < he ? w : he;
		g->r = (int16_t)(kisa * 30 / 100 > 6 ? kisa * 30 / 100 : kisa / 2);
		g->parilti = a > 250;
		int pr = (w < h ? w : h) * 22 / 100;
		if (a >= 450 && he >= 2 * pr + 20) {
			int pcx = x + p->px, pcy = cy + p->py;
			int m = pr + 10;
			g->pr = (int16_t)pr;
			g->pcx = rg_kirp16(pcx, g->x1 + m, g->x2 - m);
			g->pcy = rg_kirp16(pcy, g->y1 + m, g->y2 - m);
			g->hr = (int16_t)(pr * 30 / 100);
			g->hx = (int16_t)(g->pcx - pr * 36 / 100);
			g->hy = (int16_t)(g->pcy - pr * 38 / 100);
		}
		g->ic_x = sag ? g->x1 : g->x2;
		g->dis_x = sag ? g->x2 : g->x1;
		g->ust_ic_y = (int16_t)(g->y1 + p->ust_ic * he / 1000);
		g->ust_dis_y = (int16_t)(g->y1 + p->ust_dis * he / 1000);
		g->alt_y = (int16_t)(g->y2 - p->alt * he / 1000);
		g->kx1 = (int16_t)(g->x1 - G);
		g->ky1 = (int16_t)(g->y1 - G);
		g->kx2 = (int16_t)(g->x2 + G);
		g->ky2 = (int16_t)(g->y2 + G);
	} else if (p->sekil == RG_S_MUTLU) {	/* ^ : merkezi asagida ust yay */
		int R = w * 48 / 100;
		g->olc = (int16_t)R;
		g->cy = (int16_t)(cy + R * 40 / 100);
		g->kx1 = (int16_t)(x - R - 4);
		g->kx2 = (int16_t)(x + R + 4);
		g->ky1 = (int16_t)(g->cy - R - 4);
		g->ky2 = (int16_t)(g->cy + R * 45 / 100 + 4);
	} else if (p->sekil == RG_S_KAPALI) {	/* kapali goz: merkezi yukarida alt yay */
		int R = w * 50 / 100;
		g->olc = (int16_t)R;
		g->cy = (int16_t)(cy - R * 55 / 100);
		g->kx1 = (int16_t)(x - R - 4);
		g->kx2 = (int16_t)(x + R + 4);
		g->ky1 = (int16_t)(g->cy + R * 12 / 100 - 4);
		g->ky2 = (int16_t)(g->cy + R + 4);
	} else {	/* kalp: olc = kenar */
		int s = w * 105 / 100;
		g->olc = (int16_t)s;
		g->kx1 = (int16_t)(x - s / 2 - 4);
		g->kx2 = (int16_t)(x + s / 2 + 4);
		g->ky1 = (int16_t)(cy - s * 40 / 100 - 4);
		g->ky2 = (int16_t)(cy + s * 56 / 100 + 4);
	}
	if (sag && p->zzz) {	/* zzz alani kutuya eklenir */
		int ax, ay;
		rg_zzz_capa(p, y, &ax, &ay);
		if (ax < g->kx1)
			g->kx1 = (int16_t)ax;
		if (ax + RG_ZZZ_GEN > g->kx2)
			g->kx2 = (int16_t)(ax + RG_ZZZ_GEN);
		if (ay - RG_ZZZ_YUK < g->ky1)
			g->ky1 = (int16_t)(ay - RG_ZZZ_YUK);
		if (ay > g->ky2)
			g->ky2 = (int16_t)ay;
	}
	g->kx1 = rg_kirp16(g->kx1, 0, RG_W - 1);
	g->kx2 = rg_kirp16(g->kx2, 0, RG_W - 1);
	g->ky1 = rg_kirp16(g->ky1, 0, RG_H - 1);
	g->ky2 = rg_kirp16(g->ky2, 0, RG_H - 1);
}

/* zzz harf konumu (i = 0..2, sol ust kose); gorunmuyorsa 0. Harf yuksekligi <= 75 px, genisligi <= 50 px. */
static int rg_zzz_harf(const rg_poz_t *p, const rg_yerlesim_t *y, int i, int *ox, int *oy)
{
	if (!p->zzz || i < 0 || i > 2)
		return 0;
	int f = p->zzz - 1;	/* 0..11 */
	if (f < i * 3)
		return 0;
	int yas = f - i * 3, ax, ay;	/* harf yasi: yukari suzulur */
	rg_zzz_capa(p, y, &ax, &ay);
	*ox = ax + 6 + i * 52 + yas * 2;
	*oy = ay - 44 - i * 52 - yas * 3;
	return 1;
}

/* iki kutunun birlesimi; donus alan (px) */
static int32_t rg_birlesim(const rg_goz_t *a, const rg_goz_t *b, int16_t *x1, int16_t *y1, int16_t *x2, int16_t *y2)
{
	*x1 = a->kx1 < b->kx1 ? a->kx1 : b->kx1;
	*y1 = a->ky1 < b->ky1 ? a->ky1 : b->ky1;
	*x2 = a->kx2 > b->kx2 ? a->kx2 : b->kx2;
	*y2 = a->ky2 > b->ky2 ? a->ky2 : b->ky2;
	return (int32_t)(*x2 - *x1 + 1) * (*y2 - *y1 + 1);
}
#endif
