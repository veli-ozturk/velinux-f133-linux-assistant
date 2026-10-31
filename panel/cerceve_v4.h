/* cerceve_v4.h (24.10.2026, Claude; OTONOM 150): v3 + tjpgd_v4.c (block_idct carpimlari 64 bit). --- v3 ---
 cerceve_v3.h (21.10.2026, Claude; Sira 9 madde 69, scripts/478): v2 + tjpgd_v3.c (4:2:0 1/2-1/4 kucultulmus IDCT);
 * baska degisiklik yok. --- v2 ---
 * cerceve_v2.h (21.10.2026, Claude; Sira 8 madde 58, scripts/451): v1 + tjpgd_v2.c (1/2-1/4 YUV ortalamasi) + yonsuz
 * karede hedef->kaynak esleme tablosu (v1 her pikselde 64 bit bolme + yon anahtari; cikti bit bit ayni).
 * cf_tablo_birak(&c) tablolari birakir (cf_coz / ar_coz_f sonunda). --- v1 ---
 * cerceve_v1.h (15.10.2026, Claude): panel 0.37 dijital cerceve icin JPEG -> RGB565 (LVGL'siz, host'ta test edilir).
 *
 * cf_coz(yol, tampon, kutu_w, kutu_h, ...): JPEG'i EXIF yonune gore dondurup kutuya en-boy oranini koruyarak sigdirir
 * (buyutur de), 4x4 Bayer titresimiyle RGB565 yazar (adim = cikis genisligi). Ara tampon YOK: tjpgd her MCU blogunu
 * verince hedefte o bloga dusen pikseller (merkez ornekleme) doldurulur. Kucultme once tjpgd'de (1/2, 1/4 kare
 * ortalamasi, en fazla hedef boyutuna kadar), kalan oran en yakin komsuyla.
 * Progresif JPEG tjpgd'de yok (JDR_FMT3) -> hata doner, cagiran sonraki fotografa gecer.
 * *iptal != 0 olunca cikis fonksiyonu 0 doner, cozme hemen durur (JDR_INTR).
 * Bu dosya tjpgd.c'yi icerir (panel Makefile yalniz main.c derler). */
#ifndef CERCEVE_V4_H
#define CERCEVE_V4_H

#include "tjpgd_v4.c"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define CF_AZAMI_BAYT (40L * 1024 * 1024)	/* bundan buyuk dosyaya dokunma */
#define CF_HAVUZ 32768				/* tjpgd calisma alani (JD_FASTDECODE 2 ~10 KB ister) */

typedef struct {
	FILE *f;
	uint16_t *hedef;
	int dw, dh;		/* hedef (cikti) boyutu */
	int sw, sh;		/* olcekli kaynak boyutu (tjpgd ciktisi) */
	int lw, lh;		/* yon uygulanmis mantiksal boyut */
	int yon;		/* EXIF 1..8 */
	volatile int *iptal;
	long doldurulan;	/* yazilan hedef piksel (test icin) */
	int tablo;		/* v2: 0 denenmedi, 1 hazir, -1 yok (yonlu kare / bellek) */
	int *tx, *ty;		/* v2: hedef x/y -> mantiksal (= kaynak, yon 1) */
} cf_is_t;

static const uint8_t cf_bayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

/* ".jpg" / ".jpeg" (buyuk-kucuk harf fark etmez), gizli dosya degil */
static int cf_jpg_mi(const char *ad)
{
	size_t n = strlen(ad);
	if (!n || ad[0] == '.')
		return 0;
	const char *u = strrchr(ad, '.');
	if (!u)
		return 0;
	char e[6] = {0};
	for (int i = 0; i < 5 && u[i + 1]; i++)
		e[i] = (char)tolower((unsigned char)u[i + 1]);
	return !strcmp(e, "jpg") || !strcmp(e, "jpeg");
}

static unsigned cf_u16(const uint8_t *p, int le)
{
	return le ? (unsigned)(p[0] | p[1] << 8) : (unsigned)(p[0] << 8 | p[1]);
}

static unsigned long cf_u32(const uint8_t *p, int le)
{
	return le ? ((unsigned long)p[0] | (unsigned long)p[1] << 8 | (unsigned long)p[2] << 16 | (unsigned long)p[3] << 24)
		  : ((unsigned long)p[0] << 24 | (unsigned long)p[1] << 16 | (unsigned long)p[2] << 8 | (unsigned long)p[3]);
}

/* EXIF Orientation (0x0112) degeri; yoksa / bozuksa 1. Dosya konumu basa alinir. */
static int cf_exif_yon(FILE *f)
{
	uint8_t b[4];
	int yon = 1;

	rewind(f);
	if (fread(b, 1, 2, f) != 2 || b[0] != 0xFF || b[1] != 0xD8)
		goto son;
	for (int seg = 0; seg < 32; seg++) {
		if (fread(b, 1, 4, f) != 4 || b[0] != 0xFF)
			goto son;
		unsigned m = b[1], len = (unsigned)(b[2] << 8 | b[3]);
		if (m == 0xDA || m == 0xD9 || len < 2)
			goto son;	/* tarama basladi: EXIF yok */
		if (m != 0xE1 || len < 16) {
			if (fseek(f, (long)len - 2, SEEK_CUR))
				goto son;
			continue;
		}
		unsigned n = len - 2;
		if (n > 65533)
			goto son;
		uint8_t *d = malloc(n);
		if (!d)
			goto son;
		if (fread(d, 1, n, f) != n || memcmp(d, "Exif\0\0", 6)) {
			free(d);
			if (fseek(f, 0, SEEK_CUR))
				goto son;
			continue;	/* XMP gibi baska APP1 */
		}
		const uint8_t *t = d + 6;
		unsigned tn = n - 6;
		int le = t[0] == 'I' && t[1] == 'I';
		if ((le || (t[0] == 'M' && t[1] == 'M')) && tn >= 8 && cf_u16(t + 2, le) == 42) {
			unsigned long ifd = cf_u32(t + 4, le);
			if (ifd + 2 <= tn) {
				unsigned cnt = cf_u16(t + ifd, le);
				for (unsigned i = 0; i < cnt && ifd + 2 + (unsigned long)(i + 1) * 12 <= tn; i++) {
					const uint8_t *e = t + ifd + 2 + i * 12;
					if (cf_u16(e, le) == 0x0112 && cf_u16(e + 2, le) == 3) {
						unsigned v = cf_u16(e + 8, le);
						if (v >= 1 && v <= 8)
							yon = (int)v;
						break;
					}
				}
			}
		}
		free(d);
		goto son;
	}
son:
	rewind(f);
	return yon;
}

/* mantiksal (yon uygulanmis) -> olcekli kaynak */
static void cf_ters(const cf_is_t *c, int lx, int ly, int *sx, int *sy)
{
	switch (c->yon) {
	case 2: *sx = c->sw - 1 - lx; *sy = ly; break;
	case 3: *sx = c->sw - 1 - lx; *sy = c->sh - 1 - ly; break;
	case 4: *sx = lx; *sy = c->sh - 1 - ly; break;
	case 5: *sx = ly; *sy = lx; break;
	case 6: *sx = ly; *sy = c->sh - 1 - lx; break;
	case 7: *sx = c->sw - 1 - ly; *sy = c->sh - 1 - lx; break;
	case 8: *sx = c->sw - 1 - ly; *sy = lx; break;
	default: *sx = lx; *sy = ly; break;
	}
}

/* olcekli kaynak -> mantiksal (cf_ters'in tersi) */
static void cf_ileri(const cf_is_t *c, int sx, int sy, int *lx, int *ly)
{
	switch (c->yon) {
	case 2: *lx = c->sw - 1 - sx; *ly = sy; break;
	case 3: *lx = c->sw - 1 - sx; *ly = c->sh - 1 - sy; break;
	case 4: *lx = sx; *ly = c->sh - 1 - sy; break;
	case 5: *lx = sy; *ly = sx; break;
	case 6: *lx = c->sh - 1 - sy; *ly = sx; break;
	case 7: *lx = c->sh - 1 - sy; *ly = c->sw - 1 - sx; break;
	case 8: *lx = sy; *ly = c->sw - 1 - sx; break;
	default: *lx = sx; *ly = sy; break;
	}
}

static size_t cf_girdi(JDEC *jd, uint8_t *buf, size_t n)
{
	cf_is_t *c = jd->device;
	if (!buf)
		return fseek(c->f, (long)n, SEEK_CUR) ? 0 : n;
	return fread(buf, 1, n, c->f);
}

/* hedef eksende mantiksal koordinat (piksel merkezi) */
static inline int cf_l(int d, int lboy, int dboy)
{
	return (int)(((2LL * d + 1) * lboy) / (2LL * dboy));
}

/* v2: yonsuz karede eslemeyi bir kez hesapla (cf_l ile AYNI degerler) */
static int cf_tablo_kur(cf_is_t *c)
{
	if (c->tablo)
		return c->tablo > 0;
	c->tablo = -1;
	if (c->yon >= 2 && c->yon <= 8)
		return 0;
	c->tx = malloc((size_t)c->dw * sizeof *c->tx);
	c->ty = malloc((size_t)c->dh * sizeof *c->ty);
	if (!c->tx || !c->ty) {
		free(c->tx);
		free(c->ty);
		c->tx = c->ty = NULL;
		return 0;
	}
	for (int d = 0; d < c->dw; d++)
		c->tx[d] = cf_l(d, c->lw, c->dw);
	for (int d = 0; d < c->dh; d++)
		c->ty[d] = cf_l(d, c->lh, c->dh);
	c->tablo = 1;
	return 1;
}

static void cf_tablo_birak(cf_is_t *c)
{
	free(c->tx);
	free(c->ty);
	c->tx = c->ty = NULL;
	c->tablo = 0;
}

static int cf_cikti(JDEC *jd, void *bmp, JRECT *r)
{
	cf_is_t *c = jd->device;
	const int T = cf_tablo_kur(c);
	if (c->iptal && *c->iptal)
		return 0;
	const uint8_t *p = bmp;	/* B,G,R */
	int bw = r->right - r->left + 1;
	int ax, ay, bx, by;
	cf_ileri(c, r->left, r->top, &ax, &ay);
	cf_ileri(c, r->right, r->bottom, &bx, &by);
	int lx0 = ax < bx ? ax : bx, lx1 = ax < bx ? bx : ax, ly0 = ay < by ? ay : by, ly1 = ay < by ? by : ay;
	int dx0 = (int)((long long)lx0 * c->dw / c->lw) - 1, dx1 = (int)((long long)(lx1 + 1) * c->dw / c->lw) + 1;
	int dy0 = (int)((long long)ly0 * c->dh / c->lh) - 1, dy1 = (int)((long long)(ly1 + 1) * c->dh / c->lh) + 1;
	if (dx0 < 0)
		dx0 = 0;
	if (dy0 < 0)
		dy0 = 0;
	if (dx1 > c->dw - 1)
		dx1 = c->dw - 1;
	if (dy1 > c->dh - 1)
		dy1 = c->dh - 1;
	for (int dy = dy0; dy <= dy1; dy++) {
		int ly = T ? c->ty[dy] : cf_l(dy, c->lh, c->dh);
		if (ly < ly0 || ly > ly1)
			continue;
		uint16_t *o = c->hedef + (size_t)dy * c->dw;
		const uint8_t *bay = cf_bayer[dy & 3];
		for (int dx = dx0; dx <= dx1; dx++) {
			int lx = T ? c->tx[dx] : cf_l(dx, c->lw, c->dw);
			if (lx < lx0 || lx > lx1)
				continue;
			int sx = lx, sy = ly;
			if (!T)
				cf_ters(c, lx, ly, &sx, &sy);
			if (sx < r->left || sx > r->right || sy < r->top || sy > r->bottom)
				continue;
			const uint8_t *q = p + ((size_t)(sy - r->top) * bw + (size_t)(sx - r->left)) * 3;
			unsigned t = bay[dx & 3];
			unsigned b = q[0] + (t >> 1), g = q[1] + (t >> 2), rr = q[2] + (t >> 1);
			if (b > 255)
				b = 255;
			if (g > 255)
				g = 255;
			if (rr > 255)
				rr = 255;
			o[dx] = (uint16_t)((rr & 0xF8) << 8 | (g & 0xFC) << 3 | b >> 3);
			c->doldurulan++;
		}
	}
	return 1;
}

/* kutuya sigdir: (lw, lh) oraninda, kutu_w x kutu_h icinde en buyuk */
static void cf_sigdir(int lw, int lh, int kw, int kh, int *dw, int *dh)
{
	if ((long long)lw * kh > (long long)lh * kw) {
		*dw = kw;
		*dh = (int)((long long)lh * kw / lw);
	} else {
		*dh = kh;
		*dw = (int)((long long)lw * kh / lh);
	}
	if (*dw < 1)
		*dw = 1;
	if (*dh < 1)
		*dh = 1;
}

/* 0: tamam (*ow x *oh RGB565, adim *ow). <0: hata (hata metni). tampon en az kutu_w*kutu_h uint16. */
static int cf_coz(const char *yol, uint16_t *tampon, int kutu_w, int kutu_h, int *ow, int *oh, volatile int *iptal,
		  char *hata, size_t hn)
{
	struct stat st;
	cf_is_t c;
	JDEC jd;
	int rc = -1;

	memset(&c, 0, sizeof c);
	if (stat(yol, &st) || st.st_size < 128 || st.st_size > CF_AZAMI_BAYT) {
		snprintf(hata, hn, "boyut uygun degil");
		return -1;
	}
	c.f = fopen(yol, "rb");
	if (!c.f) {
		snprintf(hata, hn, "acilamadi");
		return -1;
	}
	setvbuf(c.f, NULL, _IOFBF, 32768);
	void *havuz = malloc(CF_HAVUZ);
	if (!havuz) {
		snprintf(hata, hn, "bellek");
		goto bit;
	}
	c.yon = cf_exif_yon(c.f);
	JRESULT j = jd_prepare(&jd, cf_girdi, havuz, CF_HAVUZ, &c);
	if (j != JDR_OK) {
		snprintf(hata, hn, j == JDR_FMT3 ? "desteklenmeyen JPEG (progresif?)" : "jd_prepare %d", (int)j);
		goto bit;
	}
	int yan = c.yon >= 5;	/* 90 derece: mantiksal genislik = kaynak yuksekligi */
	int tw = yan ? jd.height : jd.width, th = yan ? jd.width : jd.height;
	cf_sigdir(tw, th, kutu_w, kutu_h, &c.dw, &c.dh);
	uint8_t s = 0;
	while (s < 2 && (tw >> (s + 1)) >= c.dw && (th >> (s + 1)) >= c.dh)
		s++;
	c.sw = jd.width >> s;
	c.sh = jd.height >> s;
	c.lw = yan ? c.sh : c.sw;
	c.lh = yan ? c.sw : c.sh;
	if (c.sw < 1 || c.sh < 1) {
		snprintf(hata, hn, "cok kucuk");
		goto bit;
	}
	c.hedef = tampon;
	c.iptal = iptal;
	memset(tampon, 0, (size_t)c.dw * c.dh * 2);
	j = jd_decomp(&jd, cf_cikti, s);
	if (j != JDR_OK) {
		snprintf(hata, hn, j == JDR_INTR ? "iptal" : "jd_decomp %d", (int)j);
		goto bit;
	}
	*ow = c.dw;
	*oh = c.dh;
	snprintf(hata, hn, "%ux%u yon %d olcek 1/%d -> %dx%d", jd.width, jd.height, c.yon, 1 << s, c.dw, c.dh);
	rc = 0;
bit:
	cf_tablo_birak(&c);
	free(havuz);
	fclose(c.f);
	return rc;
}

#endif
