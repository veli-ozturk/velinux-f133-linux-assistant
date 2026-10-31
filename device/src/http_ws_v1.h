/* http_ws_v1.h (13.10.2026, Claude): kole-cihaz-merkez v1 icin SAF HTTP/1.1 istek ayristirici + WebSocket (RFC 6455)
 * cerceve kodlayici/cozucu. Ag/dosya erisimi yok, bellek ayirmaz; host'ta self-test ile sinanir.
 *   - Istek satiri: yalniz GET/POST/HEAD, yol '/' ile baslar, yazdirilabilir ASCII, <= 255 bayt; HTTP/1.0|1.1
 *   - Baslik bolumu <= HTTP_BASLIK_AZAMI; satir sonu CRLF zorunlu (yalniz LF / yalniz CR reddedilir); katlama
 *     (obs-fold), bosluklu ad, NUL, 64'ten cok satir reddedilir. Transfer-Encoding desteklenmez (istek kacakciligina
 *     karsi kesin red), birden cok / tutarsiz Content-Length reddedilir.
 *   - Sorgu/cerez okuyucular yuzde-kod cozumunde %00 ve gecersiz kacislari reddeder.
 *   - WebSocket: istemci cercevesi MASKELI olmali, RSV=0, denetim cercevesi FIN=1 ve <=125 bayt, uzunluk sinirli. */
#ifndef HTTP_WS_V1_H
#define HTTP_WS_V1_H
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define HTTP_BASLIK_AZAMI 8192
#define HTTP_SATIR_AZAMI 64

struct http_istek {
	char yontem[8];
	char yol[256];		/* sorgu haric */
	char sorgu[512];
	int surum;		/* 0: HTTP/1.0, 1: HTTP/1.1 */
	char host[128], cookie[768], origin[192], xkole[8], auth[192], upgrade[32], connection[96];
	char ws_key[64], ws_ver[8], xff[192], xfproto[16], xfhost[128], ua[192], ts_login[128], ctype[64];
	long long icerik_uzunluk;	/* -1 yok */
	int te_var;		/* Transfer-Encoding goruldu */
	int keepalive;
	size_t baslik_n;	/* istek satiri + basliklar + bos satir */
};

/* baslik degerini kopyala (bastaki/sondaki bosluklar atilir); sigmazsa -1 */
static inline int hb_kopya(char *o, size_t on, const char *v, size_t n)
{
	while (n && (*v == ' ' || *v == '\t')) { v++; n--; }
	while (n && (v[n - 1] == ' ' || v[n - 1] == '\t')) n--;
	if (n >= on) return -1;
	memcpy(o, v, n);
	o[n] = 0;
	return 0;
}

static inline int token_karakter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || strchr("!#$%&'*+-.^_`|~", c);
}

/* 0: eksik (daha veri gerek), -1: bozuk (400), -2: cok buyuk (431), -3: desteklenmeyen (501), >0: baslik boyu */
static inline long http_ayristir(const char *b, size_t n, struct http_istek *r)
{
	const char *son = NULL;
	for (size_t i = 0; i + 3 < n; i++)
		if (b[i] == '\r' && b[i + 1] == '\n' && b[i + 2] == '\r' && b[i + 3] == '\n') { son = b + i; break; }
	if (!son) return n >= HTTP_BASLIK_AZAMI ? -2 : 0;
	size_t bn = (size_t)(son - b) + 4;
	if (bn > HTTP_BASLIK_AZAMI) return -2;
	memset(r, 0, sizeof *r);
	r->icerik_uzunluk = -1;
	for (size_t i = 0; i < bn - 2; i++) {
		unsigned char c = (unsigned char)b[i];
		if (!c || (c == '\r' && b[i + 1] != '\n') || (c == '\n' && (!i || b[i - 1] != '\r'))) return -1;
	}
	/* istek satiri */
	const char *p = b, *e = memchr(b, '\r', bn);
	const char *sp1 = memchr(p, ' ', (size_t)(e - p));
	if (!sp1 || sp1 == p || (size_t)(sp1 - p) >= sizeof r->yontem) return -1;
	for (const char *q = p; q < sp1; q++) if (*q < 'A' || *q > 'Z') return -1;
	memcpy(r->yontem, p, (size_t)(sp1 - p));
	const char *u = sp1 + 1, *sp2 = memchr(u, ' ', (size_t)(e - u));
	if (!sp2 || *u != '/') return -1;
	const char *s3 = sp2 + 1;
	if ((size_t)(e - s3) != 8 || memcmp(s3, "HTTP/1.", 7) || (s3[7] != '0' && s3[7] != '1')) return -1;
	r->surum = s3[7] - '0';
	const char *qm = memchr(u, '?', (size_t)(sp2 - u));
	const char *yb = qm ? qm : sp2;
	if ((size_t)(yb - u) >= sizeof r->yol) return -1;
	for (const char *q = u; q < sp2; q++) if (*q <= 32 || *q >= 127) return -1;
	memcpy(r->yol, u, (size_t)(yb - u));
	if (qm) {
		if ((size_t)(sp2 - qm - 1) >= sizeof r->sorgu) return -1;
		memcpy(r->sorgu, qm + 1, (size_t)(sp2 - qm - 1));
	}
	if (strcmp(r->yontem, "GET") && strcmp(r->yontem, "POST") && strcmp(r->yontem, "HEAD")) return -3;
	r->keepalive = r->surum == 1;
	/* basliklar */
	int satir = 0;
	p = e + 2;
	while (p < son + 2) {
		e = memchr(p, '\r', (size_t)(son + 2 - p));
		if (!e) return -1;
		if (e == p) break;
		if (++satir > HTTP_SATIR_AZAMI) return -2;
		if (*p == ' ' || *p == '\t') return -1;	/* obs-fold */
		const char *ik = memchr(p, ':', (size_t)(e - p));
		if (!ik || ik == p) return -1;
		for (const char *q = p; q < ik; q++) if (!token_karakter(*q)) return -1;
		size_t an = (size_t)(ik - p);
		const char *v = ik + 1;
		size_t vn = (size_t)(e - v);
		for (size_t i = 0; i < vn; i++) if ((unsigned char)v[i] < 32 && v[i] != '\t') return -1;
#define HB(ad, alan) (an == sizeof(ad) - 1 && !strncasecmp(p, ad, an)) { if (hb_kopya(r->alan, sizeof r->alan, v, vn)) return -1; }
		if HB("Host", host)
		else if HB("Cookie", cookie)
		else if HB("Origin", origin)
		else if HB("X-Kole", xkole)
		else if HB("Authorization", auth)
		else if HB("Upgrade", upgrade)
		else if HB("Connection", connection)
		else if HB("Sec-WebSocket-Key", ws_key)
		else if HB("Sec-WebSocket-Version", ws_ver)
		else if HB("X-Forwarded-For", xff)
		else if HB("X-Forwarded-Proto", xfproto)
		else if HB("X-Forwarded-Host", xfhost)
		else if HB("User-Agent", ua)
		else if HB("Tailscale-User-Login", ts_login)
		else if HB("Content-Type", ctype)
		else if (an == 14 && !strncasecmp(p, "Content-Length", 14)) {
			char t[24];
			if (hb_kopya(t, sizeof t, v, vn) || !t[0] || strspn(t, "0123456789") != strlen(t) || strlen(t) > 12) return -1;
			long long L = atoll(t);
			if (r->icerik_uzunluk >= 0 && r->icerik_uzunluk != L) return -1;
			r->icerik_uzunluk = L;
		} else if (an == 17 && !strncasecmp(p, "Transfer-Encoding", 17)) {
			r->te_var = 1;
		}
#undef HB
		p = e + 2;
	}
	if (r->te_var) return -3;
	if (r->connection[0]) {
		/* virgullu liste: "keep-alive, Upgrade" */
		char t[96], *s = t, *k;
		snprintf(t, sizeof t, "%s", r->connection);
		while ((k = strsep(&s, ","))) {
			while (*k == ' ') k++;
			size_t L = strlen(k);
			while (L && k[L - 1] == ' ') k[--L] = 0;
			if (!strcasecmp(k, "close")) r->keepalive = 0;
			else if (!strcasecmp(k, "keep-alive")) r->keepalive = 1;
		}
	}
	r->baslik_n = bn;
	return (long)bn;
}

static inline int baglanti_upgrade_mi(const struct http_istek *r)
{
	char t[96], *s = t, *k;
	snprintf(t, sizeof t, "%s", r->connection);
	while ((k = strsep(&s, ","))) {
		while (*k == ' ') k++;
		size_t L = strlen(k);
		while (L && k[L - 1] == ' ') k[--L] = 0;
		if (!strcasecmp(k, "upgrade")) return 1;
	}
	return 0;
}

static inline int hexd(char c)
{
	return c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
}

/* yuzde-kod cozumu ('+' -> bosluk); %00, gecersiz kacis ya da sigmama -> -1 */
static inline int url_coz(const char *s, size_t n, char *o, size_t on)
{
	size_t k = 0;
	for (size_t i = 0; i < n; i++) {
		char c = s[i];
		if (c == '%') {
			if (i + 2 >= n) return -1;
			int a = hexd(s[i + 1]), b = hexd(s[i + 2]);
			if (a < 0 || b < 0) return -1;
			c = (char)(a << 4 | b);
			if (!c) return -1;
			i += 2;
		} else if (c == '+') c = ' ';
		if (k + 1 >= on) return -1;
		o[k++] = c;
	}
	o[k] = 0;
	return 0;
}

/* sorgu parametresi: 1 bulundu, 0 yok, -1 bozuk */
static inline int sorgu_al(const char *q, const char *ad, char *o, size_t on)
{
	size_t an = strlen(ad);
	const char *p = q;
	while (p && *p) {
		const char *amp = strchr(p, '&');
		size_t n = amp ? (size_t)(amp - p) : strlen(p);
		if (n > an && p[an] == '=' && !strncmp(p, ad, an))
			return url_coz(p + an + 1, n - an - 1, o, on) ? -1 : 1;
		if (n == an && !strncmp(p, ad, an)) { if (on) o[0] = 0; return 1; }
		p = amp ? amp + 1 : NULL;
	}
	return 0;
}

/* Cookie: "a=1; b=2" -> b'nin degeri; 1 bulundu */
static inline int cerez_al(const char *c, const char *ad, char *o, size_t on)
{
	size_t an = strlen(ad);
	const char *p = c;
	while (p && *p) {
		while (*p == ' ' || *p == ';') p++;
		const char *sc = strchr(p, ';');
		size_t n = sc ? (size_t)(sc - p) : strlen(p);
		if (n > an && p[an] == '=' && !strncmp(p, ad, an)) {
			size_t vn = n - an - 1;
			while (vn && p[an + 1 + vn - 1] == ' ') vn--;
			if (vn >= on) return 0;
			memcpy(o, p + an + 1, vn);
			o[vn] = 0;
			return 1;
		}
		p = sc;
	}
	return 0;
}

/* ------------------------------------------------------------------ WebSocket */
struct ws_cerceve { int fin, op; size_t n; const uint8_t *veri; };

/* b[0..n): 0 eksik, -1 protokol hatasi (1002), -2 cok buyuk (1009), >0 tuketilen bayt. Veri yerinde maskesi acilir. */
static inline long ws_coz(uint8_t *b, size_t n, size_t azami, struct ws_cerceve *f)
{
	if (n < 2) return 0;
	if (b[0] & 0x70) return -1;	/* RSV */
	if (!(b[1] & 0x80)) return -1;	/* istemci maskesiz */
	int op = b[0] & 0x0f;
	if ((op >= 3 && op <= 7) || op >= 11) return -1;
	uint64_t L = b[1] & 0x7f;
	size_t h = 2;
	if (L == 126) {
		if (n < 4) return 0;
		L = (uint64_t)b[2] << 8 | b[3];
		if (L < 126) return -1;
		h = 4;
	} else if (L == 127) {
		if (n < 10) return 0;
		if (b[2] & 0x80) return -1;
		L = 0;
		for (int i = 0; i < 8; i++) L = L << 8 | b[2 + i];
		if (L < 65536) return -1;
		h = 10;
	}
	if (op >= 8 && (!(b[0] & 0x80) || L > 125)) return -1;
	if (L > azami) return -2;
	if (n < h + 4 + L) return 0;
	uint8_t *m = b + h, *v = b + h + 4;
	for (uint64_t i = 0; i < L; i++) v[i] ^= m[i & 3];
	f->fin = !!(b[0] & 0x80);
	f->op = op;
	f->n = (size_t)L;
	f->veri = v;
	return (long)(h + 4 + L);
}

/* sunucu cercevesi basligi (maskesiz); donus baslik boyu (2/4/10) */
static inline size_t ws_baslik(uint8_t *o, int op, size_t n)
{
	o[0] = (uint8_t)(0x80 | op);
	if (n < 126) { o[1] = (uint8_t)n; return 2; }
	if (n < 65536) { o[1] = 126; o[2] = (uint8_t)(n >> 8); o[3] = (uint8_t)n; return 4; }
	o[1] = 127;
	for (int i = 0; i < 8; i++) o[2 + i] = (uint8_t)((uint64_t)n >> (56 - 8 * i));
	return 10;
}

/* Origin "http(s)://host[:port]" -> netloc (kucuk harf); uygun degilse -1 */
static inline int koken_netloc(const char *o, char *n, size_t nn)
{
	const char *p;
	if (!strncasecmp(o, "https://", 8)) p = o + 8;
	else if (!strncasecmp(o, "http://", 7)) p = o + 7;
	else return -1;
	size_t L = strcspn(p, "/?#");
	if (!L || L >= nn || p[L]) return -1;	/* yol/sorgu iceren Origin gecersiz */
	for (size_t i = 0; i < L; i++) {
		char c = p[i];
		if (c >= 'A' && c <= 'Z') c = (char)(c + 32);
		if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || strchr(".-:[]", c))) return -1;
		n[i] = c;
	}
	n[L] = 0;
	return 0;
}
#endif
