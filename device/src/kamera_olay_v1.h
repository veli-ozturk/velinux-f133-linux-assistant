/* kamera_olay_v1.h (13.10.2026, Claude): kamera-kayit-v2 ve kole-kopru-v7 ortak yardimcilari.
 * Yalniz SD'deki kayit dosyalarini (YYYYMMDD/HHMM.mjpg) OKUR; kameraya/USB'ye hic dokunmaz.
 *   - kayit_ad_gecerli : "YYYYMMDD/HHMM.mjpg" (tarih/saat araliklari dahil) -- baska dosya adi kabul edilmez
 *   - jpeg_son_c       : JPEG segmentlerini izleyerek gercek EOI'yi bulur (APP icindeki sahte FF D9'a aldanmaz)
 *   - olay_skor        : kare BOYUTLARI arasindaki goreli farklardan ucuz hareket/olay puani (JPEG cozulmez)
 *   - idx_oku / idx_tara : parca yan dizini (HHMM.idx: "KIDX1" + "<ofs> <len> <ms>" satirlari) ya da yoksa tarama
 * Yan dosyalar (kamera-kayit-v2 yazar):
 *   YYYYMMDD/HHMM.idx   her kare icin bayt ofseti, uzunlugu, dakika icindeki ms (-1 = bilinmiyor)
 *   YYYYMMDD/olay.csv   parca kapaninca "HHMM,skor,kare,bayt,tepe,tepe_ms" (ayni dakika icin SON satir gecerli) */
#ifndef KAMERA_OLAY_V1_H
#define KAMERA_OLAY_V1_H
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define OLAY_AZAMI_KARE 4096
#define OLAY_ESIK_VARSAYILAN 80	/* 0..1000; ~%8 ortalama boyut sicramasi */

struct kidx { uint32_t ofs, len; int32_t ms; };

static inline int rakam(const char *s, size_t n)
{
	return strspn(s, "0123456789") >= n;
}

static inline int iki_sayi(const char *s)
{
	return (s[0] - '0') * 10 + (s[1] - '0');
}

static inline int gun_gecerli(const char *g)
{
	if (!g || strlen(g) != 8 || !rakam(g, 8)) return 0;
	int ay = iki_sayi(g + 4), gun = iki_sayi(g + 6);
	return g[0] == '2' && ay >= 1 && ay <= 12 && gun >= 1 && gun <= 31;
}

/* "YYYYMMDD/HHMM.mjpg": tam 18 karakter, yol ayiraci yalniz 8. konumda, saat 00..23, dakika 00..59 */
static inline int kayit_ad_gecerli(const char *a)
{
	if (!a || strlen(a) != 18 || a[8] != '/' || strcmp(a + 13, ".mjpg") || !rakam(a + 9, 4)) return 0;
	char g[9];
	memcpy(g, a, 8);
	g[8] = 0;
	return gun_gecerli(g) && iki_sayi(a + 9) <= 23 && iki_sayi(a + 11) <= 59;
}

/* b[0..n) bir JPEG ile basliyorsa EOI sonrasi konum; eksikse 0; bozuksa -1 (Python jpeg_son ile ayni kurallar) */
static inline long jpeg_son_c(const uint8_t *b, size_t n)
{
	if (n < 2) return 0;
	if (b[0] != 0xff || b[1] != 0xd8) return -1;
	size_t i = 2;
	int tarama = 0;
	while (i < n) {
		if (tarama) {
			const uint8_t *p = memchr(b + i, 0xff, n - i);
			if (!p || (size_t)(p - b) + 1 >= n) return 0;
			i = (size_t)(p - b);
		}
		if (b[i] != 0xff) return -1;
		size_t p = i + 1;
		while (p < n && b[p] == 0xff) p++;
		if (p == n) return 0;
		uint8_t m = b[p];
		i = p + 1;
		if (tarama && (m == 0 || (m >= 0xd0 && m <= 0xd7))) continue;
		if (m == 0xd9) return (long)i;
		if (m == 0 || m == 0xd8) return -1;
		if (m == 1 || (m >= 0xd0 && m <= 0xd7)) continue;
		if (i + 2 > n) return 0;
		size_t L = (size_t)b[i] << 8 | b[i + 1];
		if (L < 2) return -1;
		if (i + L > n) return 0;
		i += L;
		tarama = m == 0xda;
	}
	return 0;
}

/* Hareket puani 0..1000: ardisik kare boyutlari arasindaki goreli farkin (|b-a| / ort) en buyuk 3 degerinin
 * ortalamasi x1000. Sabit sahne + sensor gurultusu ~%1-3 (10-30), hareket/isik degisimi belirgin sekilde ustte.
 * *tepe: en buyuk sicramanin oldugu kare (yoksa ortadaki). JPEG cozulmez; yalniz uzunluklar. */
static inline int olay_skor(const uint32_t *len, int n, int *tepe)
{
	double en[3] = { 0, 0, 0 };
	int ti = n > 0 ? n / 2 : 0, k = 0;
	double tmax = -1;
	for (int i = 1; i < n; i++) {
		double a = len[i - 1], b = len[i], ort = (a + b) / 2;
		if (ort < 1) continue;
		double d = (b > a ? b - a : a - b) / ort;
		k++;
		if (d > tmax) { tmax = d; ti = i; }
		if (d > en[0]) { en[2] = en[1]; en[1] = en[0]; en[0] = d; }
		else if (d > en[1]) { en[2] = en[1]; en[1] = d; }
		else if (d > en[2]) en[2] = d;
	}
	if (tepe) *tepe = ti;
	if (k <= 0) return 0;
	int m = k < 3 ? k : 3;
	double s = (en[0] + en[1] + en[2]) / m * 1000.0 + 0.5;
	return s > 1000 ? 1000 : (int)s;
}

/* HHMM.idx dosyasini oku: yalniz dosya_boyut icinde kalan, artan ofsetli, cakismayan satirlar kabul edilir
 * (yarim yazilmis son satir / bozuk satir sessizce atlanir). Donus: kare sayisi (0 = yok/gecersiz). */
static inline int idx_oku(const char *yol, uint64_t dosya_boyut, struct kidx *k, int azami)
{
	FILE *f = fopen(yol, "r");
	if (!f) return 0;
	char s[96];
	int n = 0;
	if (!fgets(s, sizeof s, f) || strncmp(s, "KIDX1", 5)) { fclose(f); return 0; }
	uint64_t son = 0;
	while (n < azami && fgets(s, sizeof s, f)) {
		unsigned long long o, l;
		long ms;
		if (!strchr(s, '\n')) break;	/* yarim satir */
		if (sscanf(s, "%llu %llu %ld", &o, &l, &ms) != 3) continue;
		if (l < 4 || l > 300000 || o < son || o + l > dosya_boyut || ms < -1 || ms > 3600000) continue;
		k[n].ofs = (uint32_t)o;
		k[n].len = (uint32_t)l;
		k[n].ms = (int32_t)ms;
		son = o + l;
		n++;
	}
	fclose(f);
	return n;
}

/* Yan dizin yoksa (v1 parcalari): dosyayi tarayarak JPEG sinirlarini bul. t: en az 300 KB tampon.
 * Kareler arasinda cop varsa sonraki "FF D8 FF"ye hizalanir. ms = -1. */
static inline int idx_tara(int fd, uint64_t boyut, struct kidx *k, int azami, uint8_t *t, size_t tn)
{
	uint64_t ofs = 0;
	int n = 0;
	while (ofs + 4 <= boyut && n < azami) {
		size_t iste = boyut - ofs < tn ? (size_t)(boyut - ofs) : tn;
		ssize_t r = pread(fd, t, iste, (off_t)ofs);
		if (r < 4) break;
		size_t pos = 0;
		int ilerledi = 0;
		while (pos + 3 <= (size_t)r && n < azami) {
			if (!(t[pos] == 0xff && t[pos + 1] == 0xd8 && t[pos + 2] == 0xff)) {
				const uint8_t *p = memchr(t + pos + 1, 0xff, (size_t)r - pos - 1);
				pos = p ? (size_t)(p - t) : (size_t)r;
				ilerledi = 1;
				continue;
			}
			long e = jpeg_son_c(t + pos, (size_t)r - pos);
			if (e > 0) {
				k[n].ofs = (uint32_t)(ofs + pos);
				k[n].len = (uint32_t)e;
				k[n].ms = -1;
				n++;
				pos += (size_t)e;
				ilerledi = 1;
				continue;
			}
			if (e == 0 && pos > 0) break;	/* pencere sonunda yarim kare: oradan yeniden oku */
			if (e == 0 && (size_t)r < tn) { pos = (size_t)r; break; }	/* dosya sonu: yarim kare */
			pos++;	/* bozuk ya da pencereden buyuk: bir sonraki SOI'yi ara */
			ilerledi = 1;
		}
		if (!ilerledi && pos == 0) break;
		if (pos == 0) pos = 1;
		ofs += pos;
		if (pos >= (size_t)r && (size_t)r < tn) break;
	}
	return n;
}
#endif
