/* kripto_v1.h (13.10.2026, Claude): kole-cihaz-merkez v1 icin SAF kripto yardimcilari (ag/dosya yok, host'ta sinanir).
 *   - SHA-256 (FIPS 180-4), HMAC-SHA256 (RFC 2104), PBKDF2-HMAC-SHA256 (RFC 8018; ic/dis durum onceden hesaplanir,
 *     tur basina 2 sikistirma)
 *   - SHA-1 (yalniz WebSocket Sec-WebSocket-Accept icin, RFC 6455)
 *   - base64 kodla/coz (kati: bosluk/eksik dolgu kabul edilmez), hex, sabit zamanli karsilastirma
 * Test vektorleri kole_cihaz_merkez_v1.c --self-test icinde (FIPS/RFC 4231/RFC 7914/RFC 6455). */
#ifndef KRIPTO_V1_H
#define KRIPTO_V1_H
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* ------------------------------------------------------------------ SHA-256 */
struct sha256 { uint32_t h[8]; uint64_t n; uint8_t b[64]; };

static const uint32_t SHA256_K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2 };

#define SR32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static inline void sha256_blok(uint32_t h[8], const uint8_t *p)
{
	uint32_t w[64], a, b, c, d, e, f, g, k;
	for (int i = 0; i < 16; i++)
		w[i] = (uint32_t)p[4 * i] << 24 | (uint32_t)p[4 * i + 1] << 16 | (uint32_t)p[4 * i + 2] << 8 | p[4 * i + 3];
	for (int i = 16; i < 64; i++) {
		uint32_t s0 = SR32(w[i - 15], 7) ^ SR32(w[i - 15], 18) ^ (w[i - 15] >> 3);
		uint32_t s1 = SR32(w[i - 2], 17) ^ SR32(w[i - 2], 19) ^ (w[i - 2] >> 10);
		w[i] = w[i - 16] + s0 + w[i - 7] + s1;
	}
	a = h[0]; b = h[1]; c = h[2]; d = h[3]; e = h[4]; f = h[5]; g = h[6]; k = h[7];
	for (int i = 0; i < 64; i++) {
		uint32_t t1 = k + (SR32(e, 6) ^ SR32(e, 11) ^ SR32(e, 25)) + ((e & f) ^ (~e & g)) + SHA256_K[i] + w[i];
		uint32_t t2 = (SR32(a, 2) ^ SR32(a, 13) ^ SR32(a, 22)) + ((a & b) ^ (a & c) ^ (b & c));
		k = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
	}
	h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e; h[5] += f; h[6] += g; h[7] += k;
}

static inline void sha256_bas(struct sha256 *s)
{
	static const uint32_t H0[8] = { 0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
					0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19 };
	memcpy(s->h, H0, sizeof H0);
	s->n = 0;
}

static inline void sha256_ekle(struct sha256 *s, const void *v, size_t n)
{
	const uint8_t *p = v;
	size_t dolu = (size_t)(s->n & 63);
	s->n += n;
	if (dolu) {
		size_t k = 64 - dolu < n ? 64 - dolu : n;
		memcpy(s->b + dolu, p, k);
		p += k; n -= k; dolu += k;
		if (dolu < 64) return;
		sha256_blok(s->h, s->b);
	}
	for (; n >= 64; p += 64, n -= 64)
		sha256_blok(s->h, p);
	if (n) memcpy(s->b, p, n);
}

static inline void sha256_bitir(struct sha256 *s, uint8_t o[32])
{
	uint64_t bit = s->n * 8;
	size_t dolu = (size_t)(s->n & 63);
	s->b[dolu++] = 0x80;
	if (dolu > 56) {
		memset(s->b + dolu, 0, 64 - dolu);
		sha256_blok(s->h, s->b);
		dolu = 0;
	}
	memset(s->b + dolu, 0, 56 - dolu);
	for (int i = 0; i < 8; i++) s->b[56 + i] = (uint8_t)(bit >> (56 - 8 * i));
	sha256_blok(s->h, s->b);
	for (int i = 0; i < 8; i++) {
		o[4 * i] = (uint8_t)(s->h[i] >> 24); o[4 * i + 1] = (uint8_t)(s->h[i] >> 16);
		o[4 * i + 2] = (uint8_t)(s->h[i] >> 8); o[4 * i + 3] = (uint8_t)s->h[i];
	}
	memset(s, 0, sizeof *s);
}

static inline void sha256_tek(const void *v, size_t n, uint8_t o[32])
{
	struct sha256 s;
	sha256_bas(&s);
	sha256_ekle(&s, v, n);
	sha256_bitir(&s, o);
}

/* ------------------------------------------------------------------ HMAC-SHA256 / PBKDF2 */
struct hmac256 { struct sha256 ic, dis; };

static inline void hmac256_anahtar(struct hmac256 *m, const void *k, size_t kn)
{
	uint8_t kb[64], pad[64];
	memset(kb, 0, sizeof kb);
	if (kn > 64) sha256_tek(k, kn, kb);
	else memcpy(kb, k, kn);
	for (int i = 0; i < 64; i++) pad[i] = kb[i] ^ 0x36;
	sha256_bas(&m->ic);
	sha256_ekle(&m->ic, pad, 64);
	for (int i = 0; i < 64; i++) pad[i] = kb[i] ^ 0x5c;
	sha256_bas(&m->dis);
	sha256_ekle(&m->dis, pad, 64);
	memset(kb, 0, sizeof kb);
	memset(pad, 0, sizeof pad);
}

static inline void hmac256_hesap(const struct hmac256 *m, const void *v, size_t n, uint8_t o[32])
{
	struct sha256 s = m->ic;
	uint8_t ic[32];
	sha256_ekle(&s, v, n);
	sha256_bitir(&s, ic);
	s = m->dis;
	sha256_ekle(&s, ic, 32);
	sha256_bitir(&s, o);
	memset(ic, 0, sizeof ic);
}

static inline void hmac_sha256(const void *k, size_t kn, const void *v, size_t n, uint8_t o[32])
{
	struct hmac256 m;
	hmac256_anahtar(&m, k, kn);
	hmac256_hesap(&m, v, n, o);
	memset(&m, 0, sizeof m);
}

/* 32 baytlik ozet uzerinde HMAC: tek blok, dogrudan sikistirma (PBKDF2 ic dongusu) */
static inline void hmac256_32(const struct hmac256 *m, const uint8_t v[32], uint8_t o[32])
{
	uint32_t h[8];
	uint8_t b[64];
	memcpy(h, m->ic.h, sizeof h);
	memcpy(b, v, 32);
	b[32] = 0x80;
	memset(b + 33, 0, 31);
	b[62] = 0x03;	/* (64 + 32) * 8 = 768 bit = 0x300 */
	b[63] = 0x00;
	sha256_blok(h, b);
	for (int i = 0; i < 8; i++) {
		b[4 * i] = (uint8_t)(h[i] >> 24); b[4 * i + 1] = (uint8_t)(h[i] >> 16);
		b[4 * i + 2] = (uint8_t)(h[i] >> 8); b[4 * i + 3] = (uint8_t)h[i];
	}
	memcpy(h, m->dis.h, sizeof h);
	b[32] = 0x80;
	memset(b + 33, 0, 31);
	b[62] = 0x03;
	sha256_blok(h, b);
	for (int i = 0; i < 8; i++) {
		o[4 * i] = (uint8_t)(h[i] >> 24); o[4 * i + 1] = (uint8_t)(h[i] >> 16);
		o[4 * i + 2] = (uint8_t)(h[i] >> 8); o[4 * i + 3] = (uint8_t)h[i];
	}
}

static inline void pbkdf2_sha256(const void *p, size_t pn, const uint8_t *tuz, size_t tn, uint32_t tur,
				 uint8_t *o, size_t on)
{
	struct hmac256 m;
	hmac256_anahtar(&m, p, pn);
	uint8_t u[32], t[32], blok[4];
	for (uint32_t i = 1; on; i++) {
		struct sha256 s = m.ic;
		blok[0] = (uint8_t)(i >> 24); blok[1] = (uint8_t)(i >> 16); blok[2] = (uint8_t)(i >> 8); blok[3] = (uint8_t)i;
		sha256_ekle(&s, tuz, tn);
		sha256_ekle(&s, blok, 4);
		uint8_t ic[32];
		sha256_bitir(&s, ic);
		s = m.dis;
		sha256_ekle(&s, ic, 32);
		sha256_bitir(&s, u);
		memcpy(t, u, 32);
		for (uint32_t c = 1; c < tur; c++) {
			hmac256_32(&m, u, u);
			for (int k = 0; k < 32; k++) t[k] ^= u[k];
		}
		size_t k = on < 32 ? on : 32;
		memcpy(o, t, k);
		o += k;
		on -= k;
	}
	memset(&m, 0, sizeof m);
	memset(u, 0, sizeof u);
	memset(t, 0, sizeof t);
}

/* ------------------------------------------------------------------ SHA-1 (yalniz WebSocket kabul anahtari) */
static inline void sha1_blok(uint32_t h[5], const uint8_t *p)
{
	uint32_t w[80];
	for (int j = 0; j < 16; j++)
		w[j] = (uint32_t)p[4 * j] << 24 | (uint32_t)p[4 * j + 1] << 16 | (uint32_t)p[4 * j + 2] << 8 | p[4 * j + 3];
	for (int j = 16; j < 80; j++) {
		uint32_t x = w[j - 3] ^ w[j - 8] ^ w[j - 14] ^ w[j - 16];
		w[j] = x << 1 | x >> 31;
	}
	uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
	for (int j = 0; j < 80; j++) {
		uint32_t f, k;
		if (j < 20) { f = (b & c) | (~b & d); k = 0x5a827999; }
		else if (j < 40) { f = b ^ c ^ d; k = 0x6ed9eba1; }
		else if (j < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8f1bbcdc; }
		else { f = b ^ c ^ d; k = 0xca62c1d6; }
		uint32_t t = (a << 5 | a >> 27) + f + e + k + w[j];
		e = d; d = c; c = b << 30 | b >> 2; b = a; a = t;
	}
	h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
}

static inline void sha1_tek(const void *v, size_t n, uint8_t o[20])
{
	uint32_t h[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };
	const uint8_t *p = v;
	uint8_t b[128];
	uint64_t bit = (uint64_t)n * 8;
	size_t i = 0;
	for (; n - i >= 64; i += 64)
		sha1_blok(h, p + i);
	size_t k = n - i;
	memcpy(b, p + i, k);
	b[k++] = 0x80;
	size_t top = k <= 56 ? 64 : 128;
	memset(b + k, 0, top - k);
	for (int j = 0; j < 8; j++) b[top - 8 + j] = (uint8_t)(bit >> (56 - 8 * j));
	sha1_blok(h, b);
	if (top == 128) sha1_blok(h, b + 64);
	for (int j = 0; j < 5; j++) {
		o[4 * j] = (uint8_t)(h[j] >> 24); o[4 * j + 1] = (uint8_t)(h[j] >> 16);
		o[4 * j + 2] = (uint8_t)(h[j] >> 8); o[4 * j + 3] = (uint8_t)h[j];
	}
}

/* ------------------------------------------------------------------ base64 / hex / sabit zaman */
static const char B64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static inline size_t b64_kodla(const uint8_t *v, size_t n, char *o, size_t on)
{
	size_t k = 0;
	for (size_t i = 0; i < n; i += 3) {
		uint32_t x = (uint32_t)v[i] << 16 | (i + 1 < n ? (uint32_t)v[i + 1] << 8 : 0) | (i + 2 < n ? v[i + 2] : 0);
		if (k + 5 > on) return 0;
		o[k++] = B64[x >> 18 & 63];
		o[k++] = B64[x >> 12 & 63];
		o[k++] = i + 1 < n ? B64[x >> 6 & 63] : '=';
		o[k++] = i + 2 < n ? B64[x & 63] : '=';
	}
	if (k >= on) return 0;
	o[k] = 0;
	return k;
}

static inline int b64_deger(char c)
{
	const char *p = c ? strchr(B64, c) : NULL;
	return p ? (int)(p - B64) : -1;
}

/* kati cozum: uzunluk 4'un kati, dolgu yalniz sonda; donus bayt sayisi ya da -1 */
static inline long b64_coz(const char *s, uint8_t *o, size_t on)
{
	size_t n = strlen(s), k = 0;
	if (!n || n % 4) return -1;
	for (size_t i = 0; i < n; i += 4) {
		int v[4], dolgu = 0;
		for (int j = 0; j < 4; j++) {
			if (s[i + j] == '=') {
				if (i + 4 != n || j < 2) return -1;
				v[j] = 0;
				dolgu++;
			} else {
				if (dolgu) return -1;
				v[j] = b64_deger(s[i + j]);
				if (v[j] < 0) return -1;
			}
		}
		uint32_t x = (uint32_t)v[0] << 18 | (uint32_t)v[1] << 12 | (uint32_t)v[2] << 6 | (uint32_t)v[3];
		if (k + 3 - (size_t)dolgu > on) return -1;
		o[k++] = (uint8_t)(x >> 16);
		if (dolgu < 2) o[k++] = (uint8_t)(x >> 8);
		if (dolgu < 1) o[k++] = (uint8_t)x;
	}
	return (long)k;
}

static inline void hex_yaz(const uint8_t *v, size_t n, char *o)
{
	static const char H[] = "0123456789abcdef";
	for (size_t i = 0; i < n; i++) { o[2 * i] = H[v[i] >> 4]; o[2 * i + 1] = H[v[i] & 15]; }
	o[2 * n] = 0;
}

static inline int hex_coz(const char *s, uint8_t *o, size_t n)
{
	if (strlen(s) != 2 * n) return -1;
	for (size_t i = 0; i < 2 * n; i++) {
		char c = s[i];
		int v = c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1;
		if (v < 0) return -1;
		if (i & 1) o[i / 2] = (uint8_t)(o[i / 2] | v);
		else o[i / 2] = (uint8_t)(v << 4);
	}
	return 0;
}

static inline int sabit_esit(const void *a, const void *b, size_t n)
{
	const volatile uint8_t *x = a, *y = b;
	uint8_t d = 0;
	for (size_t i = 0; i < n; i++) d |= (uint8_t)(x[i] ^ y[i]);
	return d == 0;
}
#endif
