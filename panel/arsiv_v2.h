/* arsiv_v2.h (21.10.2026, Claude; Sira 8 madde 58, scripts/451): v1 + cerceve_v2 tablolarini birakir (cf_tablo_birak).
 * --- v1 ---
 * arsiv_v1.h (20.10.2026, Claude): panel 0.56 kamera ARSIVI (kayit parcalari, time-lapse gunleri, fotograflar) ve canli
 * goruntu icin LVGL'siz yardimcilar; host'ta tests/arsiv_test_v1.c ile sinanir. SD SALT OKUMA (yalniz opendir/readdir/stat/
 * fopen "rb"). cerceve_v1.h'den sonra eklenir (cf_is_t, cf_girdi, cf_cikti, cf_exif_yon, cf_sigdir).
 *   ar_coz_f   : acik FILE*'dan (fmemopen ile bellekteki tek kare de olur) JPEG -> RGB565 kutuya sigdir; s_az = en az
 *                olcek (1: 1/2, 2: 1/4). cf_cikti hedefi kendisi buyutur -> LVGL'de olcekleme (transform) gerekmez.
 *   ar_kare_bul: .mjpg (art arda JPEG) icinde ofs'tan itibaren sonraki karenin [bas, uzunluk]: SOI'den SOS'a segmentler,
 *                sonra entropi verisinde FF xx (xx != 00, D0..D7 degil) = EOI.
 *   ar_listele : <kamera>/YYYYMMDD/HHMM.mjpg (kayit), <kamera>/zaman/YYYYMMDD.mjpg (time-lapse), <kamera>/fotolar/<ad>.jpg +
 *                <foto>/<ad>.jpg (fotograf); en yeni once. */
#ifndef ARSIV_V2_H
#define ARSIV_V2_H
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* ---------------------------------------------------------------- cozme */
static int ar_coz_f(FILE *f, uint16_t *tampon, int kutu_w, int kutu_h, int s_az, int *ow, int *oh, volatile int *iptal,
		    char *hata, size_t hn)
{
	cf_is_t c;
	JDEC jd;
	int rc = -1;
	memset(&c, 0, sizeof c);
	c.f = f;
	void *havuz = malloc(CF_HAVUZ);
	if (!havuz) {
		snprintf(hata, hn, "bellek");
		return -1;
	}
	c.yon = cf_exif_yon(f);	/* basa sarar */
	JRESULT j = jd_prepare(&jd, cf_girdi, havuz, CF_HAVUZ, &c);
	if (j != JDR_OK) {
		snprintf(hata, hn, j == JDR_FMT3 ? "desteklenmeyen JPEG (progresif?)" : "jd_prepare %d", (int)j);
		goto bit;
	}
	int yan = c.yon >= 5;
	int tw = yan ? jd.height : jd.width, th = yan ? jd.width : jd.height;
	cf_sigdir(tw, th, kutu_w, kutu_h, &c.dw, &c.dh);
	uint8_t s = 0;
	while (s < 2 && (tw >> (s + 1)) >= c.dw && (th >> (s + 1)) >= c.dh)
		s++;
	while (s < s_az && s < 3 && (tw >> (s + 1)) >= 8 && (th >> (s + 1)) >= 8)
		s++;	/* hiz icin daha kaba cozup cf_cikti'de buyut */
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
	snprintf(hata, hn, "%ux%u olcek 1/%d -> %dx%d", jd.width, jd.height, 1 << s, c.dw, c.dh);
	rc = 0;
bit:
	cf_tablo_birak(&c);
	free(havuz);
	return rc;
}

/* dosyanin [ofs, ofs+n) bolumunu bellege alip coz (fmemopen: cf_exif_yon'un rewind'i kare basina doner) */
static int ar_coz_parca(const char *yol, long ofs, long n, uint16_t *tampon, int kutu_w, int kutu_h, int s_az, int *ow, int *oh,
			volatile int *iptal, char *hata, size_t hn)
{
	if (n < 128 || n > 4L * 1024 * 1024) {
		snprintf(hata, hn, "kare boyu %ld", n);
		return -1;
	}
	FILE *f = fopen(yol, "rb");
	if (!f) {
		snprintf(hata, hn, "acilamadi");
		return -1;
	}
	uint8_t *b = malloc((size_t)n);
	int rc = -1;
	if (b && !fseek(f, ofs, SEEK_SET) && fread(b, 1, (size_t)n, f) == (size_t)n) {
		FILE *m = fmemopen(b, (size_t)n, "rb");
		if (m) {
			rc = ar_coz_f(m, tampon, kutu_w, kutu_h, s_az, ow, oh, iptal, hata, hn);
			fclose(m);
		} else {
			snprintf(hata, hn, "fmemopen");
		}
	} else if (!b) {
		snprintf(hata, hn, "bellek");
	} else {
		snprintf(hata, hn, "okunamadi");
	}
	free(b);
	fclose(f);
	return rc;
}

static int ar_coz_yol(const char *yol, uint16_t *tampon, int kutu_w, int kutu_h, int s_az, int *ow, int *oh, volatile int *iptal,
		      char *hata, size_t hn)
{
	struct stat st;
	if (stat(yol, &st) || st.st_size < 128 || st.st_size > CF_AZAMI_BAYT) {
		snprintf(hata, hn, "boyut uygun degil");
		return -1;
	}
	FILE *f = fopen(yol, "rb");
	if (!f) {
		snprintf(hata, hn, "acilamadi");
		return -1;
	}
	setvbuf(f, NULL, _IOFBF, 32768);
	int rc = ar_coz_f(f, tampon, kutu_w, kutu_h, s_az, ow, oh, iptal, hata, hn);
	fclose(f);
	return rc;
}

/* ---------------------------------------------------------------- .mjpg kare tarama */
struct ar_okuyucu {
	FILE *f;
	long ofs;		/* tamponun dosyadaki baslangici */
	int n, i;
	uint8_t b[16384];
};

static int ar_bayt(struct ar_okuyucu *r)
{
	if (r->i >= r->n) {
		r->ofs += r->n;
		r->n = (int)fread(r->b, 1, sizeof r->b, r->f);
		r->i = 0;
		if (r->n <= 0)
			return -1;
	}
	return r->b[r->i++];
}

static long ar_konum(const struct ar_okuyucu *r)
{
	return r->ofs + r->i;
}

/* ofs'tan sonraki ilk kare: 1 bulundu (*bas, *uz), 0 dosya sonu, -1 hata. azami_tarama: SOI aramasi bayt siniri */
static int ar_kare_bul(FILE *f, long ofs, long *bas, long *uz, long azami_tarama)
{
	struct ar_okuyucu r;
	memset(&r, 0, sizeof r);
	r.f = f;
	if (fseek(f, ofs, SEEK_SET))
		return -1;
	r.ofs = ofs;
	int a = -1, c;
	long sinir = ofs + azami_tarama;
	for (;;) {	/* SOI: FF D8 FF */
		c = ar_bayt(&r);
		if (c < 0)
			return 0;
		if (a == 0xFF && c == 0xD8) {
			int d = ar_bayt(&r);
			if (d < 0)
				return 0;
			if (d == 0xFF) {
				*bas = ar_konum(&r) - 3;
				r.i--;	/* FF'yi segment okumasina geri ver */
				break;
			}
			c = d;
		}
		a = c;
		if (ar_konum(&r) > sinir)
			return 0;
	}
	for (;;) {	/* segmentler: FF xx [uz16] ... SOS'a kadar */
		int m;
		do
			m = ar_bayt(&r);
		while (m == 0xFF);
		if (m < 0)
			return 0;
		if (m >= 0xD0 && m <= 0xD7)
			continue;
		if (m == 0xD9) {
			*uz = ar_konum(&r) - *bas;
			return 1;
		}
		int h = ar_bayt(&r), l = ar_bayt(&r);
		if (h < 0 || l < 0)
			return 0;
		long sl = (long)(h << 8 | l) - 2;
		if (sl < 0)
			return -1;
		if (m == 0xDA) {	/* SOS basligini atla, entropi verisi */
			while (sl-- > 0)
				if (ar_bayt(&r) < 0)
					return 0;
			int p = 0;
			for (;;) {
				c = ar_bayt(&r);
				if (c < 0)
					return 0;
				if (p == 0xFF && c != 0x00 && !(c >= 0xD0 && c <= 0xD7)) {
					if (c == 0xD9) {
						*uz = ar_konum(&r) - *bas;
						return 1;
					}
					if (c == 0xFF) {	/* dolgu */
						p = c;
						continue;
					}
					/* baska bir isaret (DHT/DQT/SOS: progresif): segment olarak oku */
					int hh = ar_bayt(&r), ll = ar_bayt(&r);
					if (hh < 0 || ll < 0)
						return 0;
					long s2 = (long)(hh << 8 | ll) - 2;
					while (s2-- > 0)
						if (ar_bayt(&r) < 0)
							return 0;
					p = 0;
					continue;
				}
				p = c;
			}
		}
		if (sl > 0) {	/* segment govdesini atla */
			long hedef = ar_konum(&r) + sl;
			if (fseek(f, hedef, SEEK_SET))
				return -1;
			r.ofs = hedef;
			r.n = r.i = 0;
		}
	}
}

/* ---------------------------------------------------------------- liste */
enum { AR_KAYIT, AR_ZAMAN, AR_FOTO, AR_TUR_N };
struct ar_oge {
	char yol[400];
	char ad[64];		/* gosterilecek: "20.10 14:05 · 6,1 MB" */
	long long bayt;
	long long anahtar;	/* siralama: YYYYMMDDHHMM ya da mtime */
};

static int ar_rakam(const char *s, int n)
{
	for (int i = 0; i < n; i++)
		if (s[i] < '0' || s[i] > '9')
			return 0;
	return 1;
}

static int ar_kiyas(const void *a, const void *b)	/* en yeni once */
{
	long long x = ((const struct ar_oge *)a)->anahtar, y = ((const struct ar_oge *)b)->anahtar;
	return x < y ? 1 : x > y ? -1 : strcmp(((const struct ar_oge *)b)->yol, ((const struct ar_oge *)a)->yol);
}

static void ar_mb(char *o, size_t n, long long bayt)
{
	snprintf(o, n, "%.1f MB", bayt / 1048576.0);
	char *v = strchr(o, '.');
	if (v)
		*v = ',';	/* Turkce ondalik */
}

/* tur'e gore listele (en yeni once), oge sayisi */
static int ar_listele(const char *kamera, const char *foto, int tur, struct ar_oge *o, int azami)
{
	int k = 0;
	char y[400];
	struct stat st;
#define AR_KOY(YOL, BAYT, ANAHTAR, ...) do { \
		int _i = k; \
		if (k >= azami) { \
			_i = 0; \
			for (int _j = 1; _j < k; _j++) \
				if (o[_j].anahtar < o[_i].anahtar) \
					_i = _j; \
			if ((ANAHTAR) <= o[_i].anahtar) \
				break; \
		} else \
			k++; \
		snprintf(o[_i].yol, sizeof o[_i].yol, "%s", YOL); \
		o[_i].bayt = (BAYT); \
		o[_i].anahtar = (ANAHTAR); \
		snprintf(o[_i].ad, sizeof o[_i].ad, __VA_ARGS__); \
	} while (0)
	if (tur == AR_KAYIT) {
		DIR *dd = opendir(kamera);
		struct dirent *g;
		while (dd && (g = readdir(dd))) {
			if (strlen(g->d_name) != 8 || !ar_rakam(g->d_name, 8))
				continue;
			char gd[300];
			snprintf(gd, sizeof gd, "%.250s/%.8s", kamera, g->d_name);
			DIR *pd = opendir(gd);
			struct dirent *p;
			while (pd && (p = readdir(pd))) {
				if (strlen(p->d_name) != 9 || !ar_rakam(p->d_name, 4) || strcmp(p->d_name + 4, ".mjpg"))
					continue;
				snprintf(y, sizeof y, "%.300s/%.9s", gd, p->d_name);
				if (stat(y, &st) || st.st_size < 1024)
					continue;
				char mb[24];
				ar_mb(mb, sizeof mb, st.st_size);
				long long an = atoll(g->d_name) * 10000 + atoi(p->d_name) / 1;
				AR_KOY(y, st.st_size, an, "%.2s.%.2s %.2s:%.2s · %s", g->d_name + 6, g->d_name + 4, p->d_name,
				       p->d_name + 2, mb);
			}
			if (pd)
				closedir(pd);
		}
		if (dd)
			closedir(dd);
	} else if (tur == AR_ZAMAN) {
		char zd[300];
		snprintf(zd, sizeof zd, "%.250s/zaman", kamera);
		DIR *dd = opendir(zd);
		struct dirent *g;
		while (dd && (g = readdir(dd))) {
			if (strlen(g->d_name) != 13 || !ar_rakam(g->d_name, 8) || strcmp(g->d_name + 8, ".mjpg"))
				continue;
			snprintf(y, sizeof y, "%.300s/%.13s", zd, g->d_name);
			if (stat(y, &st) || st.st_size < 1024)
				continue;
			char mb[24];
			ar_mb(mb, sizeof mb, st.st_size);
			AR_KOY(y, st.st_size, atoll(g->d_name), "%.2s.%.2s.%.4s · %s", g->d_name + 6, g->d_name + 4, g->d_name, mb);
		}
		if (dd)
			closedir(dd);
	} else {
		const char *dz[2] = {NULL, foto};
		char fd[300];
		snprintf(fd, sizeof fd, "%.250s/fotolar", kamera);
		dz[0] = fd;
		for (int d = 0; d < 2; d++) {
			if (!dz[d])
				continue;
			DIR *dd = opendir(dz[d]);
			struct dirent *g;
			while (dd && (g = readdir(dd))) {
				if (!cf_jpg_mi(g->d_name) || strlen(g->d_name) > 80)
					continue;
				snprintf(y, sizeof y, "%.300s/%.80s", dz[d], g->d_name);
				if (stat(y, &st) || st.st_size < 128)
					continue;
				struct tm tm;
				time_t mt = st.st_mtime;
				localtime_r(&mt, &tm);
				AR_KOY(y, st.st_size, (long long)st.st_mtime, "%02d.%02d %02d:%02d · %.30s", tm.tm_mday, tm.tm_mon + 1,
				       tm.tm_hour, tm.tm_min, g->d_name);
			}
			if (dd)
				closedir(dd);
		}
	}
#undef AR_KOY
	qsort(o, (size_t)k, sizeof *o, ar_kiyas);
	return k;
}

#endif
