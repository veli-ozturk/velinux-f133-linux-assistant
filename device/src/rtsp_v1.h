/* rtsp_v1.h (13.10.2026, Claude): kole-kopru-v8 RTSP sunucusunun SAF yardimcilari (ag/aygit yok, host'ta sinanir).
 *   - MD5 (RFC 1321) ve RTSP Digest dogrulama (RFC 2617; qop=auth ya da qopsuz). Parola dosyasinda HA1 tutulur.
 *   - G.711 mu-law kodlayici (Sun g711.c / Python audioop.lin2ulaw ile birebir) ve 16k -> 8k FIR azaltici.
 *   - RTSP istek ayristirici (yontem, URL, CSeq, Authorization, Transport, Session, Content-Length; sinirli boyutlar).
 *   - RFC 2435 RTP/JPEG: baseline JPEG'i (SOF0/1, 3 bilesen, Y 2x1 -> tip 0, 2x2 -> tip 1, Cb/Cr 1x1, 8 bit DQT)
 *     ayristirir; Q=255 (tablolar bant icinde, ilk parcada), DRI varsa tip+64 ve Restart basligi (F=L=1, 0x3FFF).
 *     RFC 2435 alici tarafinda STANDART Huffman tablolarini varsayar: farkli DHT "dht_standart=0" olarak bildirilir.
 *   - RTCP SR + SDES CNAME (A/V esleme icin).
 * Hicbir islev bellek ayirmaz; tum tamponlar cagirandan gelir ve sinirlari denetlenir. */
#ifndef RTSP_V1_H
#define RTSP_V1_H
#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

/* ------------------------------------------------------------------ MD5 */
struct md5 { uint32_t h[4]; uint64_t n; uint8_t b[64]; };

static const uint32_t MD5_K[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 };
static const uint8_t MD5_R[16] = { 7, 12, 17, 22, 5, 9, 14, 20, 4, 11, 16, 23, 6, 10, 15, 21 };

static inline void md5_blok(struct md5 *m, const uint8_t *p)
{
	uint32_t M[16], a = m->h[0], b = m->h[1], c = m->h[2], d = m->h[3];
	for (int i = 0; i < 16; i++)
		M[i] = p[4 * i] | p[4 * i + 1] << 8 | p[4 * i + 2] << 16 | (uint32_t)p[4 * i + 3] << 24;
	for (int i = 0; i < 64; i++) {
		uint32_t f;
		int g;
		if (i < 16) { f = (b & c) | (~b & d); g = i; }
		else if (i < 32) { f = (d & b) | (~d & c); g = (5 * i + 1) & 15; }
		else if (i < 48) { f = b ^ c ^ d; g = (3 * i + 5) & 15; }
		else { f = c ^ (b | ~d); g = (7 * i) & 15; }
		f += a + MD5_K[i] + M[g];
		a = d; d = c; c = b;
		int s = MD5_R[(i >> 4) * 4 + (i & 3)];
		b += (f << s) | (f >> (32 - s));
	}
	m->h[0] += a; m->h[1] += b; m->h[2] += c; m->h[3] += d;
}

static inline void md5_bas(struct md5 *m)
{
	m->h[0] = 0x67452301; m->h[1] = 0xefcdab89; m->h[2] = 0x98badcfe; m->h[3] = 0x10325476;
	m->n = 0;
}

static inline void md5_ekle(struct md5 *m, const void *v, size_t n)
{
	const uint8_t *p = v;
	size_t dolu = (size_t)(m->n & 63);
	m->n += n;
	while (n) {
		size_t k = 64 - dolu < n ? 64 - dolu : n;
		memcpy(m->b + dolu, p, k);
		dolu += k; p += k; n -= k;
		if (dolu == 64) { md5_blok(m, m->b); dolu = 0; }
	}
}

static inline void md5_bitir(struct md5 *m, uint8_t o[16])
{
	uint64_t bit = m->n * 8;
	uint8_t s = 0x80, z = 0;
	md5_ekle(m, &s, 1);
	while ((m->n & 63) != 56) md5_ekle(m, &z, 1);
	uint8_t L[8];
	for (int i = 0; i < 8; i++) L[i] = (uint8_t)(bit >> (8 * i));
	md5_ekle(m, L, 8);
	for (int i = 0; i < 4; i++)
		for (int k = 0; k < 4; k++) o[4 * i + k] = (uint8_t)(m->h[i] >> (8 * k));
}

/* parcalari ':' ile birlestirip MD5 hex (33 bayt). parca listesi NULL ile biter */
static inline void md5_hex_parcalar(char o[33], const char *const *parca)
{
	struct md5 m;
	uint8_t d[16];
	md5_bas(&m);
	for (int i = 0; parca[i]; i++) {
		if (i) md5_ekle(&m, ":", 1);
		md5_ekle(&m, parca[i], strlen(parca[i]));
	}
	md5_bitir(&m, d);
	for (int i = 0; i < 16; i++) snprintf(o + 2 * i, 3, "%02x", d[i]);
}

static inline int hex32_mi(const char *s)
{
	if (!s || strlen(s) != 32) return 0;
	for (int i = 0; i < 32; i++)
		if (!isxdigit((unsigned char)s[i])) return 0;
	return 1;
}

/* sabit zamanli esitlik (ayni uzunluk bekler) */
static inline int ct_esit(const char *a, const char *b)
{
	size_t na = strlen(a), nb = strlen(b);
	unsigned d = (unsigned)(na ^ nb);
	for (size_t i = 0; i < na && i < nb; i++) d |= (unsigned char)(tolower((unsigned char)a[i]) ^ tolower((unsigned char)b[i]));
	return d == 0;
}

/* ------------------------------------------------------------------ Digest (RFC 2617) */
#define RTSP_REALM "VeLinux-KOLE"
struct ozet {
	char kullanici[64], realm[64], nonce[96], uri[256], yanit[40], qop[16], nc[16], cnonce[96], algo[16];
};

/* Authorization basligi degeri: "Digest k=v, k="v", ..." -> 0; bicim bozuksa -1 */
static inline int ozet_ayristir(const char *h, struct ozet *o)
{
	memset(o, 0, sizeof *o);
	if (!h || strncasecmp(h, "Digest ", 7)) return -1;
	const char *p = h + 7;
	for (int sayac = 0; *p && sayac < 16; sayac++) {
		while (*p == ' ' || *p == ',' || *p == '\t') p++;
		if (!*p) break;
		char ad[16];
		size_t an = 0;
		while (*p && *p != '=' && *p != ' ' && an < sizeof ad - 1) ad[an++] = *p++;
		ad[an] = 0;
		while (*p == ' ') p++;
		if (*p != '=') return -1;
		p++;
		while (*p == ' ') p++;
		char deger[256];
		size_t dn = 0;
		if (*p == '"') {
			p++;
			while (*p && *p != '"') {
				if (dn >= sizeof deger - 1) return -1;
				if (*p == '\\' && p[1]) p++;
				deger[dn++] = *p++;
			}
			if (*p != '"') return -1;
			p++;
		} else {
			while (*p && *p != ',' && *p != ' ') {
				if (dn >= sizeof deger - 1) return -1;
				deger[dn++] = *p++;
			}
		}
		deger[dn] = 0;
		struct { const char *ad; char *hedef; size_t n; } alan[] = {
			{ "username", o->kullanici, sizeof o->kullanici }, { "realm", o->realm, sizeof o->realm },
			{ "nonce", o->nonce, sizeof o->nonce }, { "uri", o->uri, sizeof o->uri },
			{ "response", o->yanit, sizeof o->yanit }, { "qop", o->qop, sizeof o->qop }, { "nc", o->nc, sizeof o->nc },
			{ "cnonce", o->cnonce, sizeof o->cnonce }, { "algorithm", o->algo, sizeof o->algo } };
		for (size_t i = 0; i < sizeof alan / sizeof alan[0]; i++)
			if (!strcasecmp(ad, alan[i].ad)) {
				if (dn >= alan[i].n) return -1;
				memcpy(alan[i].hedef, deger, dn + 1);
			}
	}
	if (!o->kullanici[0] || !o->nonce[0] || !o->uri[0] || strlen(o->yanit) != 32) return -1;
	if (o->algo[0] && strcasecmp(o->algo, "MD5")) return -1;
	if (o->qop[0] && (strcasecmp(o->qop, "auth") || !o->nc[0] || !o->cnonce[0])) return -1;
	return 0;
}

/* beklenen yanit (hex) */
static inline void ozet_beklenen(const struct ozet *o, const char *yontem, const char *ha1, char out[33])
{
	char ha2[33];
	const char *p2[] = { yontem, o->uri, NULL };
	md5_hex_parcalar(ha2, p2);
	if (o->qop[0]) {
		const char *p[] = { ha1, o->nonce, o->nc, o->cnonce, o->qop, ha2, NULL };
		md5_hex_parcalar(out, p);
	} else {
		const char *p[] = { ha1, o->nonce, ha2, NULL };
		md5_hex_parcalar(out, p);
	}
}

/* 1: dogru; 0: yanlis. kullanici ve realm de esit olmali */
static inline int ozet_dogrula(const struct ozet *o, const char *yontem, const char *kullanici, const char *ha1)
{
	char bek[33];
	ozet_beklenen(o, yontem, ha1, bek);
	int k = ct_esit(o->kullanici, kullanici);
	int r = !strcmp(o->realm, RTSP_REALM);
	int y = ct_esit(o->yanit, bek);
	return k & r & y;
}

/* ------------------------------------------------------------------ G.711 mu-law */
static inline uint8_t ulaw_kodla(int16_t pcm)
{
	static const int16_t uson[8] = { 0x3F, 0x7F, 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF };
	int v = pcm >> 2, maske;
	if (v < 0) { v = -v; maske = 0x7F; } else maske = 0xFF;
	if (v > 8159) v = 8159;
	v += 0x84 >> 2;
	int seg = 0;
	while (seg < 8 && v > uson[seg]) seg++;
	if (seg >= 8) return (uint8_t)(0x7F ^ maske);
	return (uint8_t)((((seg << 4) | ((v >> (seg + 1)) & 0xF))) ^ maske);
}

static inline int16_t ulaw_coz(uint8_t u)
{
	u = (uint8_t)~u;
	int t = ((u & 0x0F) << 3) + 0x84;
	t <<= (u & 0x70) >> 4;
	return (int16_t)((u & 0x80) ? (0x84 - t) : (t - 0x84));
}

/* ------------------------------------------------------------------ 16 kHz -> 8 kHz (FIR + 2'de 1) */
#define AZ8_TAP 31
struct az8 { float h[AZ8_TAP]; int faz; };
static float az8_fir[AZ8_TAP];
static inline void az8_hazirla(void)
{
	double top = 0, fc = 3400.0 / 16000.0;
	for (int i = 0; i < AZ8_TAP; i++) {
		int m = i - AZ8_TAP / 2;
		double s = m ? sin(2 * M_PI * fc * m) / (M_PI * m) : 2 * fc;
		double w = 0.54 - 0.46 * cos(2 * M_PI * i / (AZ8_TAP - 1));
		az8_fir[i] = (float)(s * w);
		top += az8_fir[i];
	}
	for (int i = 0; i < AZ8_TAP; i++) az8_fir[i] = (float)(az8_fir[i] / top);
}

/* in: 16k s16; out: 8k mu-law. Donen: uretilen ornek sayisi */
static inline size_t az8_ulaw(struct az8 *a, const int16_t *in, size_t n, uint8_t *out)
{
	size_t k = 0;
	for (size_t i = 0; i < n; i++) {
		memmove(a->h, a->h + 1, (AZ8_TAP - 1) * sizeof(float));
		a->h[AZ8_TAP - 1] = in[i];
		if (++a->faz == 2) {
			a->faz = 0;
			float s = 0;
			for (int t = 0; t < AZ8_TAP; t++) s += a->h[t] * az8_fir[t];
			int v = (int)lrintf(s);
			out[k++] = ulaw_kodla((int16_t)(v > 32767 ? 32767 : v < -32768 ? -32768 : v));
		}
	}
	return k;
}

/* ------------------------------------------------------------------ RTSP istek ayristirici */
struct rtsp_istek {
	char yontem[20], url[256], yetki[700], tasima[256], oturum[80];
	int cseq;
	long govde;
};

/* b: "\r\n\r\n" dahil baslik blogu (n bayt). 0 ok, -1 bozuk/sinir disi */
static inline int rtsp_ayristir(const char *b, size_t n, struct rtsp_istek *r)
{
	memset(r, 0, sizeof *r);
	r->cseq = -1;
	char satir[1024];
	size_t i = 0;
	int ilk = 1;
	while (i < n) {
		size_t s = i;
		while (i < n && b[i] != '\n') i++;
		if (i >= n) return -1;
		size_t L = i - s;
		if (L && b[i - 1] == '\r') L--;
		i++;
		if (L >= sizeof satir) return -1;
		memcpy(satir, b + s, L);
		satir[L] = 0;
		if (!L) break;
		if (ilk) {
			char ver[16];
			if (sscanf(satir, "%19s %255s %15s", r->yontem, r->url, ver) != 3 || strcmp(ver, "RTSP/1.0")) return -1;
			for (char *p = r->yontem; *p; p++)
				if (*p < 'A' || *p > 'Z') { if (*p != '_') return -1; }
			ilk = 0;
			continue;
		}
		char *c = strchr(satir, ':');
		if (!c) return -1;
		*c = 0;
		char *v = c + 1;
		while (*v == ' ' || *v == '\t') v++;
		if (!strcasecmp(satir, "CSeq")) {
			char *son;
			long x = strtol(v, &son, 10);
			if (son == v || x < 0 || x > 0x7fffffff) return -1;
			r->cseq = (int)x;
		} else if (!strcasecmp(satir, "Authorization")) {
			if (strlen(v) >= sizeof r->yetki) return -1;
			strcpy(r->yetki, v);
		} else if (!strcasecmp(satir, "Transport")) {
			if (strlen(v) >= sizeof r->tasima) return -1;
			strcpy(r->tasima, v);
		} else if (!strcasecmp(satir, "Session")) {
			size_t k = strcspn(v, "; ");
			if (k >= sizeof r->oturum) return -1;
			memcpy(r->oturum, v, k);
			r->oturum[k] = 0;
		} else if (!strcasecmp(satir, "Content-Length")) {
			char *son;
			long x = strtol(v, &son, 10);
			if (son == v || x < 0 || x > 4096) return -1;
			r->govde = x;
		}
	}
	return ilk || r->cseq < 0 ? -1 : 0;
}

/* "rtsp://host:port/yol?x" ya da "/yol" -> yol (sorgu atilir). Bossa "/" */
static inline void url_yol(const char *url, char *o, size_t on)
{
	const char *p = url;
	if (!strncasecmp(p, "rtsp://", 7) || !strncasecmp(p, "rtsps://", 8)) {
		p = strchr(p + 7 + (p[4] == 's'), '/');
		if (!p) p = "/";
	}
	size_t k = strcspn(p, "?#");
	if (k >= on) k = on - 1;
	memcpy(o, p, k);
	o[k] = 0;
	if (!o[0]) snprintf(o, on, "/");
}

/* Transport: TCP interleaved istegi. 1: TCP (kanal *a,*b), 0: TCP degil (UDP) */
static inline int tasima_tcp(const char *t, int *a, int *b)
{
	*a = *b = -1;
	if (!strstr(t, "RTP/AVP/TCP")) return 0;
	const char *i = strstr(t, "interleaved=");
	if (i && sscanf(i + 12, "%d-%d", a, b) != 2) { *b = -1; if (sscanf(i + 12, "%d", a) == 1) *b = *a + 1; }
	if (*a < 0 || *a > 253 || *b != *a + 1) *a = *b = -1;	/* sunucu atar */
	return 1;
}

/* ------------------------------------------------------------------ RFC 2435 RTP/JPEG */
/* Standart Huffman tablolari (ITU T.81 Ek K.3): RFC 2435 alicisi bunlari varsayar */
static const uint8_t HUF_DC_Y_BITS[16] = { 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
static const uint8_t HUF_DC_C_BITS[16] = { 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
static const uint8_t HUF_DC_VAL[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
static const uint8_t HUF_AC_Y_BITS[16] = { 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
static const uint8_t HUF_AC_Y_VAL[162] = {
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
	0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08, 0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
	0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
	0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
	0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
	0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
	0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa };
static const uint8_t HUF_AC_C_BITS[16] = { 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
static const uint8_t HUF_AC_C_VAL[162] = {
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
	0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
	0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34, 0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
	0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
	0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
	0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
	0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
	0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
	0xf9, 0xfa };

/* tc: 0 DC 1 AC; th: 0 Y 1 C */
static inline int huf_standart(int tc, int th, const uint8_t *bits, const uint8_t *val, int nval)
{
	const uint8_t *sb = tc ? (th ? HUF_AC_C_BITS : HUF_AC_Y_BITS) : (th ? HUF_DC_C_BITS : HUF_DC_Y_BITS);
	const uint8_t *sv = tc ? (th ? HUF_AC_C_VAL : HUF_AC_Y_VAL) : HUF_DC_VAL;
	int sn = tc ? 162 : 12;
	return th <= 1 && nval == sn && !memcmp(bits, sb, 16) && !memcmp(val, sv, (size_t)sn);
}

struct rjpeg {
	int tip;			/* 0 (4:2:2) ya da 1 (4:2:0); DRI varsa RTP'de +64 */
	int g8, y8;			/* genislik/8, yukseklik/8 */
	int genislik, yukseklik;
	unsigned dri;
	int dht_standart;		/* 1: DHT yok ya da hepsi standart */
	uint8_t q[128];			/* Y ve C tablolari (zigzag, 8 bit) */
	const uint8_t *tarama;
	size_t tarama_n;
};

static inline unsigned be16(const uint8_t *p) { return (unsigned)p[0] << 8 | p[1]; }

/* NULL: uygun; aksi halde Turkce neden */
static inline const char *rjpeg_ayristir(const uint8_t *j, size_t n, struct rjpeg *r)
{
	memset(r, 0, sizeof *r);
	r->dht_standart = 1;
	if (n < 4 || j[0] != 0xff || j[1] != 0xd8) return "JPEG degil";
	uint8_t qt[4][64];
	int qvar[4] = { 0, 0, 0, 0 }, sof = 0, qy = -1, qc = -1;
	size_t i = 2;
	while (i + 4 <= n) {
		if (j[i] != 0xff) return "isaretci bozuk";
		size_t p = i + 1;
		while (p < n && j[p] == 0xff) p++;
		if (p >= n) return "isaretci bozuk";
		uint8_t m = j[p];
		i = p + 1;
		if (m == 0xd8 || m == 0x01 || (m >= 0xd0 && m <= 0xd7)) continue;
		if (m == 0xd9) return "tarama yok";
		if (i + 2 > n) return "kisa";
		size_t L = be16(j + i);
		if (L < 2 || i + L > n) return "segment boyu";
		const uint8_t *s = j + i + 2;
		size_t sn = L - 2;
		if (m == 0xdb) {
			size_t k = 0;
			while (k < sn) {
				int pq = s[k] >> 4, tq = s[k] & 15;
				if (pq) return "16 bit DQT desteklenmiyor";
				if (tq > 3 || k + 65 > sn) return "DQT bozuk";
				memcpy(qt[tq], s + k + 1, 64);
				qvar[tq] = 1;
				k += 65;
			}
		} else if (m == 0xc0 || m == 0xc1) {
			if (sn < 15 || s[0] != 8) return "SOF bicimi (8 bit, 3 bilesen gerekli)";
			r->yukseklik = (int)be16(s + 1);
			r->genislik = (int)be16(s + 3);
			if (s[5] != 3 || sn < 6 + 9) return "3 bilesen gerekli";
			int h = s[7] >> 4, v = s[7] & 15;
			if (s[10] != 0x11 || s[13] != 0x11) return "Cb/Cr ornekleme 1x1 olmali";
			if (h == 2 && v == 1) r->tip = 0;
			else if (h == 2 && v == 2) r->tip = 1;
			else return "Y ornekleme 2x1 ya da 2x2 olmali";
			qy = s[8];
			qc = s[11];
			if (s[14] != qc || qy > 3 || qc > 3) return "Cb/Cr ayni Q tablosunu kullanmali";
			if (!r->genislik || !r->yukseklik || r->genislik > 2040 || r->yukseklik > 2040) return "boyut (en fazla 2040)";
			r->g8 = (r->genislik + 7) / 8;
			r->y8 = (r->yukseklik + 7) / 8;
			sof = 1;
		} else if (m >= 0xc2 && m <= 0xcf && m != 0xc4 && m != 0xc8 && m != 0xcc) {
			return "baseline degil (progressive/aritmetik)";
		} else if (m == 0xc4) {
			size_t k = 0;
			while (k < sn) {
				if (k + 17 > sn) return "DHT bozuk";
				int tc = s[k] >> 4, th = s[k] & 15, top = 0;
				for (int b = 0; b < 16; b++) top += s[k + 1 + b];
				if (k + 17 + (size_t)top > sn || tc > 1 || th > 3) return "DHT bozuk";
				if (!huf_standart(tc, th, s + k + 1, s + k + 17, top)) r->dht_standart = 0;
				k += 17 + (size_t)top;
			}
		} else if (m == 0xdd) {
			if (sn < 2) return "DRI bozuk";
			r->dri = be16(s);
		} else if (m == 0xda) {
			if (!sof) return "SOF yok";
			if (sn < 1 || s[0] != 3) return "SOS 3 bilesen olmali";
			size_t b = i + L, e = b;
			for (;;) {
				const uint8_t *f = memchr(j + e, 0xff, n - e);
				if (!f || (size_t)(f - j) + 1 >= n) return "EOI yok";
				e = (size_t)(f - j);
				uint8_t x = j[e + 1];
				if (x == 0xd9) break;
				if (x == 0x00 || (x >= 0xd0 && x <= 0xd7) || x == 0xff) { e += x == 0xff ? 1 : 2; continue; }
				return "taramada beklenmeyen isaretci";
			}
			if (!qvar[qy] || !qvar[qc]) return "Q tablosu eksik";
			memcpy(r->q, qt[qy], 64);
			memcpy(r->q + 64, qt[qc], 64);
			r->tarama = j + b;
			r->tarama_n = e - b;
			return r->tarama_n ? NULL : "tarama bos";
		}
		i += L;
	}
	return "SOS yok";
}

typedef int (*paket_cb)(void *ctx, const uint8_t *p, size_t n);

static inline void rtp_baslik(uint8_t *o, int isaret, int pt, uint16_t seq, uint32_t ts, uint32_t ssrc)
{
	o[0] = 0x80;
	o[1] = (uint8_t)((isaret ? 0x80 : 0) | pt);
	o[2] = (uint8_t)(seq >> 8); o[3] = (uint8_t)seq;
	o[4] = (uint8_t)(ts >> 24); o[5] = (uint8_t)(ts >> 16); o[6] = (uint8_t)(ts >> 8); o[7] = (uint8_t)ts;
	o[8] = (uint8_t)(ssrc >> 24); o[9] = (uint8_t)(ssrc >> 16); o[10] = (uint8_t)(ssrc >> 8); o[11] = (uint8_t)ssrc;
}

#define RTP_AZAMI 1500
/* mtu: RTP paketinin en buyuk boyu (baslik dahil, 300..1500). Donen: paket sayisi, hata -1 */
static inline int rtp_jpeg_paketle(const struct rjpeg *r, uint32_t ts, uint32_t ssrc, uint16_t *seq, size_t mtu,
				   paket_cb cb, void *ctx)
{
	if (mtu < 300 || mtu > RTP_AZAMI || !r->tarama_n || r->tarama_n >= (1u << 24)) return -1;
	uint8_t p[RTP_AZAMI];
	size_t ofs = 0;
	int sayi = 0;
	while (ofs < r->tarama_n) {
		size_t h = 12;
		uint8_t *J = p + 12;
		J[0] = 0;
		J[1] = (uint8_t)(ofs >> 16); J[2] = (uint8_t)(ofs >> 8); J[3] = (uint8_t)ofs;
		J[4] = (uint8_t)(r->tip + (r->dri ? 64 : 0));
		J[5] = 255;
		J[6] = (uint8_t)r->g8;
		J[7] = (uint8_t)r->y8;
		h += 8;
		if (r->dri) {
			p[h] = (uint8_t)(r->dri >> 8); p[h + 1] = (uint8_t)r->dri; p[h + 2] = 0xff; p[h + 3] = 0xff;
			h += 4;
		}
		if (!ofs) {
			p[h] = 0; p[h + 1] = 0; p[h + 2] = 0; p[h + 3] = 128;
			memcpy(p + h + 4, r->q, 128);
			h += 4 + 128;
		}
		size_t k = mtu - h;
		if (k > r->tarama_n - ofs) k = r->tarama_n - ofs;
		memcpy(p + h, r->tarama + ofs, k);
		ofs += k;
		rtp_baslik(p, ofs == r->tarama_n, 26, *seq, ts, ssrc);
		(*seq)++;
		if (cb(ctx, p, h + k)) return -1;
		sayi++;
	}
	return sayi;
}

/* RTCP SR + SDES(CNAME) bilesik paketi. Donen: boyut (en fazla 64) */
static inline size_t rtcp_sr(uint8_t *o, uint32_t ssrc, uint64_t ntp, uint32_t rtpts, uint32_t pk, uint32_t okt)
{
	uint32_t v[6] = { ssrc, (uint32_t)(ntp >> 32), (uint32_t)ntp, rtpts, pk, okt };
	o[0] = 0x80; o[1] = 200; o[2] = 0; o[3] = 6;
	for (int i = 0; i < 6; i++) {
		o[4 + 4 * i] = (uint8_t)(v[i] >> 24); o[5 + 4 * i] = (uint8_t)(v[i] >> 16);
		o[6 + 4 * i] = (uint8_t)(v[i] >> 8); o[7 + 4 * i] = (uint8_t)v[i];
	}
	/* SDES: 4 baslik + 4 ssrc + 2 + 7 "velinux" + 1 son = 18 -> 20 (dolgu) */
	uint8_t *s = o + 28;
	s[0] = 0x81; s[1] = 202; s[2] = 0; s[3] = 4;
	s[4] = (uint8_t)(ssrc >> 24); s[5] = (uint8_t)(ssrc >> 16); s[6] = (uint8_t)(ssrc >> 8); s[7] = (uint8_t)ssrc;
	s[8] = 1; s[9] = 7;
	memcpy(s + 10, "velinux", 7);
	s[17] = 0; s[18] = 0; s[19] = 0;
	return 48;
}
#endif
