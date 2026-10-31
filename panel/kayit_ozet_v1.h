/* kayit_ozet_v1.h (19.10.2026, Claude): panel 0.46 Gizlilik > BUGUNUN KAYITLARI (yol haritasi v2 U3, OTONOM 17).
 * SD kamera kayitlari SALT OKUMA (kamera-kayit v2+ duzeni):
 *   <kok>/<YYYYMMDD>/HHMM.mjpg   1 dk'lik parcalar
 *   <kok>/<YYYYMMDD>/olay.csv    "HHMM,skor,kare,bayt,tepe,tepe_ms" (skor 0..1000 hareket puani, kamera_olay_v1.h)
 *   <kok>/zaman/<YYYYMMDD>.mjpg  zaman atlamali gunluk dosya
 * Hicbir dosya yazilmaz/silinmez/acilip degistirilmez (yalniz opendir/readdir/stat/fopen "r").
 * Host testi tests/panel_046_test_v1.c. */
#ifndef KAYIT_OZET_V1_H
#define KAYIT_OZET_V1_H
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define KO_ARALIK_N 4		/* gosterilen en yeni aralik sayisi */
#define KO_HAREKET_N 3		/* en guclu hareket anlari */
#define KO_HAREKET_ESIK 120	/* kamera-kayit bekci varsayilan esigi */

struct kayit_ozet {
	int dizin_var, parca;		/* bugunun klasoru var mi, farkli dakika parcasi */
	long long bayt;			/* parcalarin toplam boyutu */
	int aralik_n, aralik_top;	/* gosterilen (en yeni KO_ARALIK_N) / toplam aralik */
	int aralik[KO_ARALIK_N][2];	/* dakika (0..1439): ilk parca, son parca */
	int hareket_n;
	int hareket[KO_HAREKET_N][2];	/* dakika, skor (buyukten kucuge) */
	long long zaman_bayt;		/* zaman atlamali gunluk dosya; yoksa -1 */
};

static int ko_hhmm(const char *s)	/* "HHMM..." -> dakika; gecersizse -1 */
{
	if (strspn(s, "0123456789") < 4)
		return -1;
	int h = (s[0] - '0') * 10 + (s[1] - '0'), m = (s[2] - '0') * 10 + (s[3] - '0');
	return h < 24 && m < 60 ? h * 60 + m : -1;
}

static void ko_aralik_ekle(struct kayit_ozet *o, int bas, int son)
{
	o->aralik_top++;
	if (o->aralik_n == KO_ARALIK_N) {	/* en yenileri tut */
		memmove(o->aralik[0], o->aralik[1], sizeof o->aralik[0] * (KO_ARALIK_N - 1));
		o->aralik_n--;
	}
	o->aralik[o->aralik_n][0] = bas;
	o->aralik[o->aralik_n][1] = son;
	o->aralik_n++;
}

static void ko_hareket_ekle(struct kayit_ozet *o, int dk, int skor)
{
	int i;
	if (o->hareket_n < KO_HAREKET_N)
		i = o->hareket_n++;
	else if (skor > o->hareket[KO_HAREKET_N - 1][1])	/* dolu: yalniz sonuncudan buyukse yerine gecer */
		i = KO_HAREKET_N - 1;
	else
		return;
	o->hareket[i][0] = dk;
	o->hareket[i][1] = skor;
	for (; i > 0 && o->hareket[i][1] > o->hareket[i - 1][1]; i--) {
		int t0 = o->hareket[i][0], t1 = o->hareket[i][1];
		o->hareket[i][0] = o->hareket[i - 1][0];
		o->hareket[i][1] = o->hareket[i - 1][1];
		o->hareket[i - 1][0] = t0;
		o->hareket[i - 1][1] = t1;
	}
}

/* gun: "YYYYMMDD" (yerel tarih). Bos klasor/yok: dizin_var 0. */
static void kayit_ozet_oku(const char *kok, const char *gun, struct kayit_ozet *o)
{
	unsigned char dk[1440];
	char yol[400], tam[480];
	memset(o, 0, sizeof *o);
	memset(dk, 0, sizeof dk);
	o->zaman_bayt = -1;
	snprintf(yol, sizeof yol, "%.300s/%.8s", kok, gun);
	DIR *d = opendir(yol);
	if (d) {
		struct dirent *e;
		o->dizin_var = 1;
		while ((e = readdir(d))) {
			const char *n = e->d_name;
			int m;
			struct stat st;
			if (strlen(n) != 9 || strcmp(n + 4, ".mjpg") || (m = ko_hhmm(n)) < 0)
				continue;
			snprintf(tam, sizeof tam, "%s/%.9s", yol, n);
			if (stat(tam, &st) || !S_ISREG(st.st_mode))
				continue;
			if (!dk[m]) {
				dk[m] = 1;
				o->parca++;
			}
			o->bayt += (long long)st.st_size;
		}
		closedir(d);
	}
	int bas = -1, son = -1;
	for (int m = 0; m < 1440; m++) {
		if (!dk[m])
			continue;
		if (bas >= 0 && m - son <= 2) {	/* 2 dk'ya kadar bosluk ayni aralik */
			son = m;
			continue;
		}
		if (bas >= 0)
			ko_aralik_ekle(o, bas, son);
		bas = son = m;
	}
	if (bas >= 0)
		ko_aralik_ekle(o, bas, son);
	snprintf(tam, sizeof tam, "%s/olay.csv", yol);
	FILE *f = o->dizin_var ? fopen(tam, "r") : NULL;
	char s[160];
	while (f && fgets(s, sizeof s, f)) {
		int m = ko_hhmm(s), skor;
		if (m >= 0 && s[4] == ',' && sscanf(s + 5, "%d", &skor) == 1 && skor >= KO_HAREKET_ESIK && skor <= 1000)
			ko_hareket_ekle(o, m, skor);
	}
	if (f)
		fclose(f);
	struct stat st;
	snprintf(tam, sizeof tam, "%.300s/zaman/%.8s.mjpg", kok, gun);
	if (!stat(tam, &st) && S_ISREG(st.st_mode))
		o->zaman_bayt = (long long)st.st_size;
}

static void ko_boyut(char *b, size_t n, long long bayt)	/* Turkce ondalik virgul */
{
	double mb = (double)bayt / 1048576.0;
	if (mb >= 1024.0)
		snprintf(b, n, "%.1f GB", mb / 1024.0);
	else if (mb >= 10.0)
		snprintf(b, n, "%.0f MB", mb);
	else
		snprintf(b, n, "%.1f MB", mb);
	char *v = strchr(b, '.');
	if (v)
		*v = ',';
}

static void ko_sure(char *b, size_t n, int dk)
{
	if (dk >= 60)
		snprintf(b, n, "%d sa %d dk", dk / 60, dk % 60);
	else
		snprintf(b, n, "%d dk", dk);
}

/* panelde gosterilen dort satir */
static void kayit_ozet_metin(const struct kayit_ozet *o, char *ozet, size_t no, char *ara, size_t na, char *har, size_t nh,
			     char *zam, size_t nz)
{
	char b1[32], b2[32];
	zam[0] = 0;
	if (o->zaman_bayt >= 0) {
		ko_boyut(b1, sizeof b1, o->zaman_bayt);
		snprintf(zam, nz, "Zaman atlamalı: %s", b1);
	}
	if (!o->parca) {
		snprintf(ozet, no, "Bugün kayıt yok");
		ara[0] = har[0] = 0;
		return;
	}
	ko_sure(b1, sizeof b1, o->parca);
	ko_boyut(b2, sizeof b2, o->bayt);
	snprintf(ozet, no, "%d parça · %s · %s", o->parca, b1, b2);
	size_t u = (size_t)snprintf(ara, na, "Aralıklar: ");
	for (int i = 0; i < o->aralik_n && u < na; i++) {
		int a = o->aralik[i][0], s = o->aralik[i][1] + 1;	/* son parca dakikasinin sonu */
		if (s >= 1440)
			s = 1439;
		u += (size_t)snprintf(ara + u, na - u, "%s%02d:%02d–%02d:%02d", i ? ", " : "", a / 60, a % 60, s / 60, s % 60);
	}
	if (u < na && o->aralik_top > o->aralik_n)
		snprintf(ara + u, na - u, " (+%d önceki)", o->aralik_top - o->aralik_n);
	if (!o->hareket_n) {
		snprintf(har, nh, "Hareket: eşik (%d) üstü yok", KO_HAREKET_ESIK);
		return;
	}
	u = (size_t)snprintf(har, nh, "Hareket: ");
	for (int i = 0; i < o->hareket_n && u < nh; i++)
		u += (size_t)snprintf(har + u, nh - u, "%s%02d:%02d (%d)", i ? ", " : "", o->hareket[i][0] / 60,
				      o->hareket[i][0] % 60, o->hareket[i][1]);
}
#endif
