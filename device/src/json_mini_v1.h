/* json_mini_v1.h (13.10.2026, Claude): kole-cihaz-merkez v1 icin kucuk, sinirli JSON ayristirici + yazici.
 * Ayristirici jsmn benzeri: girdiyi KOPYALAMAZ, belirteç dizisi cagirandan gelir (tasarsa -1). Derinlik <= 32.
 * Kati: dizgede denetim karakteri, gecersiz kacis, sayi bicimi, true/false/null disi sozcukler, sonda cop reddedilir.
 * Yazici: sinirli buyuyen tampon (azami asilirsa hata bayragi), dizgeler JSON kacisli (</ dahil, U+2028/9 kacisli). */
#ifndef JSON_MINI_V1_H
#define JSON_MINI_V1_H
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { J_NESNE = 1, J_DIZI, J_DIZGE, J_ILKEL };
struct jtok { int tip, bas, son, boyut; };

struct jdurum { const char *s; int n, i, t, azami; struct jtok *tk; int derinlik; };

static int j_deger(struct jdurum *d);

static inline void j_bosluk(struct jdurum *d)
{
	while (d->i < d->n && (d->s[d->i] == ' ' || d->s[d->i] == '\t' || d->s[d->i] == '\n' || d->s[d->i] == '\r')) d->i++;
}

static inline int j_yeni(struct jdurum *d, int tip, int bas)
{
	if (d->t >= d->azami) return -1;
	d->tk[d->t] = (struct jtok){ tip, bas, bas, 0 };
	return d->t++;
}

static inline int j_dizge(struct jdurum *d)
{
	int k = j_yeni(d, J_DIZGE, d->i + 1);
	if (k < 0) return -1;
	d->i++;
	while (d->i < d->n) {
		unsigned char c = (unsigned char)d->s[d->i];
		if (c == '"') { d->tk[k].son = d->i; d->i++; return k; }
		if (c < 32) return -1;
		if (c == '\\') {
			if (++d->i >= d->n) return -1;
			c = (unsigned char)d->s[d->i];
			if (c == 'u') {
				for (int j = 1; j <= 4; j++) {
					if (d->i + j >= d->n) return -1;
					char h = d->s[d->i + j];
					if (!((h >= '0' && h <= '9') || (h >= 'a' && h <= 'f') || (h >= 'A' && h <= 'F'))) return -1;
				}
				d->i += 4;
			} else if (!strchr("\"\\/bfnrt", c)) return -1;
		}
		d->i++;
	}
	return -1;
}

static inline int j_ilkel(struct jdurum *d)
{
	int b = d->i;
	const char *s = d->s + b;
	int kalan = d->n - b;
	if (kalan >= 4 && !memcmp(s, "true", 4)) d->i += 4;
	else if (kalan >= 5 && !memcmp(s, "false", 5)) d->i += 5;
	else if (kalan >= 4 && !memcmp(s, "null", 4)) d->i += 4;
	else {
		int i = 0;
		if (i < kalan && s[i] == '-') i++;
		if (i >= kalan || s[i] < '0' || s[i] > '9') return -1;
		if (s[i] == '0') i++;
		else while (i < kalan && s[i] >= '0' && s[i] <= '9') i++;
		if (i < kalan && s[i] == '.') {
			i++;
			if (i >= kalan || s[i] < '0' || s[i] > '9') return -1;
			while (i < kalan && s[i] >= '0' && s[i] <= '9') i++;
		}
		if (i < kalan && (s[i] == 'e' || s[i] == 'E')) {
			i++;
			if (i < kalan && (s[i] == '+' || s[i] == '-')) i++;
			if (i >= kalan || s[i] < '0' || s[i] > '9') return -1;
			while (i < kalan && s[i] >= '0' && s[i] <= '9') i++;
		}
		d->i += i;
	}
	if (d->i < d->n && !strchr(" \t\r\n,]}", d->s[d->i])) return -1;
	int k = j_yeni(d, J_ILKEL, b);
	if (k < 0) return -1;
	d->tk[k].son = d->i;
	return k;
}

static int j_deger(struct jdurum *d)
{
	j_bosluk(d);
	if (d->i >= d->n) return -1;
	char c = d->s[d->i];
	if (c == '"') return j_dizge(d);
	if (c != '{' && c != '[') return j_ilkel(d);
	if (++d->derinlik > 32) return -1;
	int k = j_yeni(d, c == '{' ? J_NESNE : J_DIZI, d->i);
	if (k < 0) return -1;
	char kapa = c == '{' ? '}' : ']';
	d->i++;
	j_bosluk(d);
	if (d->i < d->n && d->s[d->i] == kapa) { d->i++; d->tk[k].son = d->i; d->derinlik--; return k; }
	for (;;) {
		if (c == '{') {
			j_bosluk(d);
			if (d->i >= d->n || d->s[d->i] != '"' || j_dizge(d) < 0) return -1;
			j_bosluk(d);
			if (d->i >= d->n || d->s[d->i] != ':') return -1;
			d->i++;
		}
		if (j_deger(d) < 0) return -1;
		d->tk[k].boyut++;
		j_bosluk(d);
		if (d->i >= d->n) return -1;
		if (d->s[d->i] == ',') { d->i++; continue; }
		if (d->s[d->i] == kapa) { d->i++; break; }
		return -1;
	}
	d->tk[k].son = d->i;
	d->derinlik--;
	return k;
}

/* donus: belirteç sayisi ya da -1 */
static inline int json_ayristir(const char *s, int n, struct jtok *tk, int azami)
{
	struct jdurum d = { s, n, 0, 0, azami, tk, 0 };
	if (j_deger(&d) < 0) return -1;
	j_bosluk(&d);
	return d.i == n ? d.t : -1;
}

/* i'den baslayan alt agacin sonrasindaki belirteç */
static inline int j_atla(const struct jtok *tk, int t, int i)
{
	int kalan = 1;
	while (kalan && i < t) {
		if (tk[i].tip == J_NESNE) kalan += 2 * tk[i].boyut;
		else if (tk[i].tip == J_DIZI) kalan += tk[i].boyut;
		kalan--;
		i++;
	}
	return i;
}

static inline int j_anahtar_esit(const char *s, const struct jtok *k, const char *ad)
{
	size_t n = strlen(ad);
	return k->tip == J_DIZGE && (size_t)(k->son - k->bas) == n && !memcmp(s + k->bas, ad, n);
}

/* nesne (o) icinde anahtar -> deger belirteci, yoksa -1 */
static inline int j_al(const char *s, const struct jtok *tk, int t, int o, const char *ad)
{
	if (o < 0 || o >= t || tk[o].tip != J_NESNE) return -1;
	int i = o + 1;
	for (int k = 0; k < tk[o].boyut && i < t; k++) {
		if (j_anahtar_esit(s, &tk[i], ad)) return i + 1 < t ? i + 1 : -1;
		i = j_atla(tk, t, i + 1);
	}
	return -1;
}

/* dizi (a) icinde k. oge */
static inline int j_oge(const struct jtok *tk, int t, int a, int k)
{
	if (a < 0 || a >= t || tk[a].tip != J_DIZI || k >= tk[a].boyut) return -1;
	int i = a + 1;
	while (k-- > 0 && i < t) i = j_atla(tk, t, i);
	return i < t ? i : -1;
}

/* dizge belirtecini UTF-8 olarak coz; 0 ok, -1 tip/sigma/gecersiz (NUL dahil reddedilir) */
static inline int j_dizge_al(const char *s, const struct jtok *k, char *o, size_t on)
{
	if (!k || k->tip != J_DIZGE) return -1;
	size_t w = 0;
	for (int i = k->bas; i < k->son; i++) {
		unsigned c = (unsigned char)s[i];
		if (c == '\\') {
			c = (unsigned char)s[++i];
			if (c == 'u') {
				unsigned v = (unsigned)strtoul((char[5]){ s[i + 1], s[i + 2], s[i + 3], s[i + 4], 0 }, NULL, 16);
				i += 4;
				if (v >= 0xd800 && v <= 0xdbff) {
					if (i + 6 >= k->son) return -1;
					if (s[i + 1] != '\\' || s[i + 2] != 'u') return -1;
					unsigned v2 = (unsigned)strtoul((char[5]){ s[i + 3], s[i + 4], s[i + 5], s[i + 6], 0 }, NULL, 16);
					if (v2 < 0xdc00 || v2 > 0xdfff) return -1;
					v = 0x10000 + ((v - 0xd800) << 10) + (v2 - 0xdc00);
					i += 6;
				} else if (v >= 0xdc00 && v <= 0xdfff) return -1;
				if (!v) return -1;
				uint8_t u[4];
				size_t un;
				if (v < 0x80) { u[0] = (uint8_t)v; un = 1; }
				else if (v < 0x800) { u[0] = (uint8_t)(0xc0 | v >> 6); u[1] = (uint8_t)(0x80 | (v & 63)); un = 2; }
				else if (v < 0x10000) { u[0] = (uint8_t)(0xe0 | v >> 12); u[1] = (uint8_t)(0x80 | (v >> 6 & 63)); u[2] = (uint8_t)(0x80 | (v & 63)); un = 3; }
				else { u[0] = (uint8_t)(0xf0 | v >> 18); u[1] = (uint8_t)(0x80 | (v >> 12 & 63)); u[2] = (uint8_t)(0x80 | (v >> 6 & 63)); u[3] = (uint8_t)(0x80 | (v & 63)); un = 4; }
				if (w + un >= on) return -1;
				memcpy(o + w, u, un);
				w += un;
				continue;
			}
			c = c == 'b' ? '\b' : c == 'f' ? '\f' : c == 'n' ? '\n' : c == 'r' ? '\r' : c == 't' ? '\t' : c;
		}
		if (w + 1 >= on) return -1;
		o[w++] = (char)c;
	}
	o[w] = 0;
	return 0;
}

/* tam sayi (kesir/us yok); 0 ok */
static inline int j_tamsayi(const char *s, const struct jtok *k, long long *v)
{
	if (!k || k->tip != J_ILKEL) return -1;
	int i = k->bas, n = k->son - k->bas;
	if (n < 1 || n > 18) return -1;
	for (int j = 0; j < n; j++) {
		char c = s[i + j];
		if (!(c == '-' && j == 0) && (c < '0' || c > '9')) return -1;
	}
	char t[24];
	memcpy(t, s + i, (size_t)n);
	t[n] = 0;
	*v = atoll(t);
	return 0;
}

/* sayi (kesirli olabilir) */
static inline int j_sayi(const char *s, const struct jtok *k, double *v)
{
	if (!k || k->tip != J_ILKEL) return -1;
	char c = s[k->bas];
	if (c != '-' && (c < '0' || c > '9')) return -1;
	char t[40];
	int n = k->son - k->bas;
	if (n >= (int)sizeof t) return -1;
	memcpy(t, s + k->bas, (size_t)n);
	t[n] = 0;
	*v = strtod(t, NULL);
	return 0;
}

/* 1 true, 0 false, -1 bool degil */
static inline int j_bool(const char *s, const struct jtok *k)
{
	if (!k || k->tip != J_ILKEL) return -1;
	if (k->son - k->bas == 4 && !memcmp(s + k->bas, "true", 4)) return 1;
	if (k->son - k->bas == 5 && !memcmp(s + k->bas, "false", 5)) return 0;
	return -1;
}

static inline int j_null(const char *s, const struct jtok *k)
{
	return k && k->tip == J_ILKEL && k->son - k->bas == 4 && !memcmp(s + k->bas, "null", 4);
}

/* ------------------------------------------------------------------ yazici */
struct jy { char *b; size_t n, cap, azami; int hata; };

static inline void jy_ekle(struct jy *y, const char *v, size_t n)
{
	if (y->hata) return;
	if (y->n + n + 1 > y->cap) {
		size_t c = y->cap ? y->cap : 1024;
		while (c < y->n + n + 1) c *= 2;
		if (c > y->azami) c = y->azami;
		if (y->n + n + 1 > c) { y->hata = 1; return; }
		char *nb = realloc(y->b, c);
		if (!nb) { y->hata = 1; return; }
		y->b = nb;
		y->cap = c;
	}
	memcpy(y->b + y->n, v, n);
	y->n += n;
	y->b[y->n] = 0;
}

static inline void jy_ham(struct jy *y, const char *v) { jy_ekle(y, v, strlen(v)); }

static inline void jy_bicim(struct jy *y, const char *f, ...) __attribute__((format(printf, 2, 3)));
static inline void jy_bicim(struct jy *y, const char *f, ...)
{
	char t[512];
	va_list a;
	va_start(a, f);
	int n = vsnprintf(t, sizeof t, f, a);
	va_end(a);
	if (n < 0 || n >= (int)sizeof t) { y->hata = 1; return; }
	jy_ekle(y, t, (size_t)n);
}

/* JSON dizgesi; gecersiz UTF-8 baytlari U+FFFD olur */
static inline void jy_dizge_n(struct jy *y, const char *s, size_t n)
{
	jy_ekle(y, "\"", 1);
	for (size_t i = 0; i < n;) {
		unsigned char c = (unsigned char)s[i];
		char t[8];
		if (c == '"' || c == '\\') { t[0] = '\\'; t[1] = (char)c; jy_ekle(y, t, 2); i++; }
		else if (c == '<' || c == '>' || c == '&') { snprintf(t, sizeof t, "\\u%04x", c); jy_ekle(y, t, 6); i++; }
		else if (c < 32 || c == 127) { snprintf(t, sizeof t, "\\u%04x", c); jy_ekle(y, t, 6); i++; }
		else if (c < 128) { jy_ekle(y, (char *)&c, 1); i++; }
		else {
			size_t L = (c & 0xe0) == 0xc0 ? 2 : (c & 0xf0) == 0xe0 ? 3 : (c & 0xf8) == 0xf0 ? 4 : 0;
			int ok = L && i + L <= n && c >= 0xc2 && c <= 0xf4;
			for (size_t j = 1; ok && j < L; j++) ok = ((unsigned char)s[i + j] & 0xc0) == 0x80;
			if (ok && L == 3 && c == 0xe2 && (unsigned char)s[i + 1] == 0x80 &&
			    ((unsigned char)s[i + 2] == 0xa8 || (unsigned char)s[i + 2] == 0xa9)) {
				jy_ham(y, (unsigned char)s[i + 2] == 0xa8 ? "\\u2028" : "\\u2029");
				i += 3;
			} else if (ok) { jy_ekle(y, s + i, L); i += L; }
			else { jy_ham(y, "\\ufffd"); i++; }
		}
	}
	jy_ekle(y, "\"", 1);
}

static inline void jy_dizge(struct jy *y, const char *s) { if (s) jy_dizge_n(y, s, strlen(s)); else jy_ham(y, "null"); }

/* "ad": anahtar yaz (virgul cagirana ait) */
static inline void jy_anahtar(struct jy *y, const char *ad)
{
	jy_dizge(y, ad);
	jy_ekle(y, ":", 1);
}

static inline void jy_bitir(struct jy *y) { free(y->b); y->b = NULL; y->n = y->cap = 0; }
#endif
