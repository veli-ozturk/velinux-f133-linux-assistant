/* Panel 0.23 (13.10.2026, Claude): Notlar / Yapilacaklar listesi (/data/notlar.txt).
 * Bicim: her satir "<0|1>\t<metin>" (1 = tamamlandi). En fazla NOT_MAX oge, metin UTF-8, en fazla NOT_UZUN-1 bayt (ardisik bosluklar teke iner).
 * Metindeki satir sonu / sekme / denetim karakterleri bosluga cevrilir, bastaki-sondaki bosluk atilir,
 * cok uzun metin UTF-8 karakter sinirinda kesilir. Saf islevler (dosya/LVGL yok); host testi tests/panel_023_test_v1.c. */
#ifndef VELINUX_NOTLAR_V1_H
#define VELINUX_NOTLAR_V1_H
#include <stdio.h>
#include <string.h>

#define NOT_MAX 16
#define NOT_UZUN 200
#define NOT_KARAKTER 80	/* ekrandaki metin kutusunun karakter siniri */

struct not_ogesi {
	int bitti;
	char metin[NOT_UZUN];
};

struct not_listesi {
	int n;
	struct not_ogesi o[NOT_MAX];
};

/* temizle: denetim karakterleri -> bosluk, kenar bosluklari at, UTF-8 sinirinda kes. Donus: uzunluk. */
static size_t not_temizle(char *dst, size_t n, const char *src)
{
	size_t k = 0;
	if (!n)
		return 0;
	while (*src == ' ' || *src == '\t' || *src == '\r' || *src == '\n')
		src++;
	for (; *src && k + 1 < n; src++) {
		unsigned char c = (unsigned char)*src;
		char ch = (c < 0x20 || c == 0x7F) ? ' ' : (char)c;
		if (ch == ' ' && k && dst[k - 1] == ' ')
			continue;	/* ardisik bosluklar teke iner */
		dst[k++] = ch;
	}
	dst[k] = 0;
	if (*src) {	/* kesildi: yarim kalan cok baytli karakteri at */
		while (k && ((unsigned char)dst[k - 1] & 0xC0) == 0x80)
			k--;
		if (k && ((unsigned char)dst[k - 1] & 0x80))
			k--;
		dst[k] = 0;
	}
	while (k && dst[k - 1] == ' ')
		dst[--k] = 0;
	return k;
}

static void notlar_coz(struct not_listesi *l, const char *metin)
{
	l->n = 0;
	const char *p = metin;
	while (p && *p && l->n < NOT_MAX) {
		const char *son = strchr(p, '\n');
		size_t n = son ? (size_t)(son - p) : strlen(p);
		if (n >= 2 && (p[0] == '0' || p[0] == '1') && p[1] == '\t') {
			char tmp[NOT_UZUN * 2];
			size_t m = n - 2 < sizeof tmp - 1 ? n - 2 : sizeof tmp - 1;
			memcpy(tmp, p + 2, m);
			tmp[m] = 0;
			struct not_ogesi *o = &l->o[l->n];
			if (not_temizle(o->metin, sizeof o->metin, tmp)) {
				o->bitti = p[0] == '1';
				l->n++;
			}
		}
		p = son ? son + 1 : NULL;
	}
}

/* Donus: yazilan uzunluk; sigmazsa -1. */
static int notlar_yaz(const struct not_listesi *l, char *out, size_t n)
{
	size_t k = 0;
	if (!n)
		return -1;
	out[0] = 0;
	for (int i = 0; i < l->n; i++) {
		int r = snprintf(out + k, n - k, "%d\t%s\n", l->o[i].bitti ? 1 : 0, l->o[i].metin);
		if (r < 0 || (size_t)r >= n - k)
			return -1;
		k += (size_t)r;
	}
	return (int)k;
}

/* Donus: yeni ogenin sirasi; liste doluysa ya da metin bossa -1. Yeni oge en uste eklenir. */
static int not_ekle(struct not_listesi *l, const char *metin)
{
	struct not_ogesi yeni = {0, ""};
	if (l->n >= NOT_MAX || !not_temizle(yeni.metin, sizeof yeni.metin, metin))
		return -1;
	memmove(&l->o[1], &l->o[0], sizeof l->o[0] * (size_t)l->n);
	l->o[0] = yeni;
	l->n++;
	return 0;
}

static void not_sil(struct not_listesi *l, int i)
{
	if (i < 0 || i >= l->n)
		return;
	memmove(&l->o[i], &l->o[i + 1], sizeof l->o[0] * (size_t)(l->n - i - 1));
	l->n--;
}

static int notlar_biten(const struct not_listesi *l)
{
	int b = 0;
	for (int i = 0; i < l->n; i++)
		b += !!l->o[i].bitti;
	return b;
}

/* tamamlananlari sil; donus: silinen sayisi */
static int notlar_biteni_temizle(struct not_listesi *l)
{
	int k = 0, s = 0;
	for (int i = 0; i < l->n; i++) {
		if (l->o[i].bitti) {
			s++;
			continue;
		}
		if (k != i)
			l->o[k] = l->o[i];
		k++;
	}
	l->n = k;
	return s;
}
#endif
