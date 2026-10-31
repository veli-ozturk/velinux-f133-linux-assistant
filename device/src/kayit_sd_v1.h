/* kayit_sd_v1.h (13.10.2026, Claude): SD kart kamera kayitlari, SALT OKUMA (kole-kopru v7 kodu, BIREBIR).
 * KAYIT_LISTE / KAYIT_OKU <ad> [ofs] [len] / KAYIT_IDX / KAYIT_KARE <ad> <i|t> / KAYIT_OLAYLAR <gun|*> [esik].
 * Ad dogrulamasi siki (kamera_olay_v1.h: YYYYMMDD/HHMM.mjpg); baska dosya okunmaz, hicbir sey yazilmaz.
 * Kullananlar: kole-kopru v8 ve ileride cihaz-ici web merkezi (PC'siz kayit izleme). */
#ifndef KAYIT_SD_V1_H
#define KAYIT_SD_V1_H
#include "kopru_ortak_v1.h"
#include "kamera_olay_v1.h"

static int mjpg_adi(const char *d)
{
	return strlen(d) == 9 && !strcmp(d + 4, ".mjpg") && strspn(d, "0123456789") == 4;
}

/* tamami rakam, en fazla 10 hane */
static int sayi_al(const char *a, unsigned long long *v)
{
	size_t n = a ? strlen(a) : 0;
	if (!n || n > 10 || strspn(a, "0123456789") != n) return -1;
	*v = strtoull(a, NULL, 10);
	return 0;
}

static int kayit_liste(int s)
{
	char satir[96];
	DIR *d = opendir(kok);
	struct dirent *e;
	while (d && (e = readdir(d))) {
		if (!gun_gecerli(e->d_name)) continue;
		char gun[512];
		snprintf(gun, sizeof gun, "%s/%.8s", kok, e->d_name);
		DIR *d2 = opendir(gun);
		struct dirent *e2;
		while (d2 && (e2 = readdir(d2))) {
			char ad[32];
			if (!mjpg_adi(e2->d_name)) continue;
			snprintf(ad, sizeof ad, "%.8s/%.9s", e->d_name, e2->d_name);
			if (!kayit_ad_gecerli(ad)) continue;
			char tam[600];
			struct stat st;
			snprintf(tam, sizeof tam, "%s/%s", kok, ad);
			if (stat(tam, &st) || !S_ISREG(st.st_mode)) continue;
			int m = snprintf(satir, sizeof satir, "%s %lld\n", ad, (long long)st.st_size);
			if (tam_yaz(s, satir, (size_t)m)) { closedir(d2); closedir(d); return 1; }
		}
		if (d2) closedir(d2);
	}
	if (d) closedir(d);
	return tam_yaz(s, "SON\n", 4);
}

/* KAYIT_OKU <ad> [ofs] [len] */
static int kayit_oku(int s, const char *ad, const char *ofs_s, const char *len_s)
{
	if (!kayit_ad_gecerli(ad)) return hata(s, "ad");
	unsigned long long ofs = 0, len = 0;
	if ((ofs_s && sayi_al(ofs_s, &ofs)) || (len_s && sayi_al(len_s, &len))) return hata(s, "aralik");
	char tam[600];
	snprintf(tam, sizeof tam, "%s/%s", kok, ad);
	int fd = open(tam, O_RDONLY | O_CLOEXEC);
	struct stat st;
	if (fd < 0 || fstat(fd, &st) || !S_ISREG(st.st_mode)) { if (fd >= 0) close(fd); return hata(s, "yok"); }
	unsigned long long boyut = (unsigned long long)st.st_size;
	if (ofs > boyut) { close(fd); return hata(s, "aralik"); }
	unsigned long long n = boyut - ofs;
	if (len && len < n) n = len;
	if (n > 0xffffffffULL || lseek(fd, (off_t)ofs, SEEK_SET) < 0) { close(fd); return hata(s, "aralik"); }
	uint8_t h[4] = { (uint8_t)n, (uint8_t)(n >> 8), (uint8_t)(n >> 16), (uint8_t)(n >> 24) };
	int rc = tam_yaz(s, h, 4);
	static uint8_t tb[65536];
	while (!rc && n) {
		ssize_t r = read(fd, tb, n < sizeof tb ? (size_t)n : sizeof tb);
		if (r <= 0) break;
		rc = tam_yaz(s, tb, (size_t)r);
		n -= (unsigned long long)r;
	}
	close(fd);
	return rc;
}

/* parca yan dizini: once .idx, yoksa salt okunur tarama */
static struct kidx kx[OLAY_AZAMI_KARE];
static uint8_t tarama_t[300 * 1024];
static int idx_yukle(const char *ad, unsigned long long *boyut, int *tarandi)
{
	char tam[600], iy[600];
	struct stat st;
	snprintf(tam, sizeof tam, "%s/%s", kok, ad);
	if (stat(tam, &st) || !S_ISREG(st.st_mode)) return -1;
	*boyut = (unsigned long long)st.st_size;
	snprintf(iy, sizeof iy, "%s/%.13s.idx", kok, ad);
	*tarandi = 0;
	int n = idx_oku(iy, *boyut, kx, OLAY_AZAMI_KARE);
	if (n > 0) return n;
	int fd = open(tam, O_RDONLY | O_CLOEXEC);
	if (fd < 0) return -1;
	n = idx_tara(fd, *boyut, kx, OLAY_AZAMI_KARE, tarama_t, sizeof tarama_t);
	close(fd);
	*tarandi = 1;
	return n;
}

static int kayit_idx(int s, const char *ad)
{
	if (!kayit_ad_gecerli(ad)) return hata(s, "ad");
	unsigned long long boyut;
	int tarandi;
	int n = idx_yukle(ad, &boyut, &tarandi);
	if (n < 0) return hata(s, "yok");
	char satir[96];
	int m = snprintf(satir, sizeof satir, "IDX %d %llu %s\n", n, boyut, tarandi ? "tarama" : "idx");
	int rc = tam_yaz(s, satir, (size_t)m);
	for (int i = 0; !rc && i < n; i++) {
		m = snprintf(satir, sizeof satir, "%u %u %d\n", kx[i].ofs, kx[i].len, kx[i].ms);
		rc = tam_yaz(s, satir, (size_t)m);
	}
	return rc ? rc : tam_yaz(s, "SON\n", 4);
}

static int temsilci(int n)
{
	static uint32_t len[OLAY_AZAMI_KARE];
	for (int i = 0; i < n; i++) len[i] = kx[i].len;
	int tepe = n / 2;
	int skor = olay_skor(len, n, &tepe);
	return skor >= OLAY_ESIK_VARSAYILAN ? tepe : n / 2;
}

/* KAYIT_KARE <ad> <i|t> */
static int kayit_kare(int s, const char *ad, const char *sec)
{
	if (!kayit_ad_gecerli(ad)) return hata(s, "ad");
	unsigned long long boyut, i = 0;
	int tarandi;
	if (!sec || (strcmp(sec, "t") && sayi_al(sec, &i))) return hata(s, "kare");
	int n = idx_yukle(ad, &boyut, &tarandi);
	if (n <= 0) return hata(s, "yok");
	if (!strcmp(sec, "t")) i = (unsigned long long)temsilci(n);
	if (i >= (unsigned long long)n || kx[i].len > sizeof tarama_t) return hata(s, "kare");
	char tam[600];
	snprintf(tam, sizeof tam, "%s/%s", kok, ad);
	int fd = open(tam, O_RDONLY | O_CLOEXEC);
	if (fd < 0) return hata(s, "yok");
	ssize_t r = pread(fd, tarama_t, kx[i].len, kx[i].ofs);
	close(fd);
	if (r != (ssize_t)kx[i].len || tarama_t[0] != 0xff || tarama_t[1] != 0xd8) return hata(s, "kare");
	uint32_t L = kx[i].len;
	uint8_t h[4] = { (uint8_t)L, (uint8_t)(L >> 8), (uint8_t)(L >> 16), (uint8_t)(L >> 24) };
	return tam_yaz(s, h, 4) || tam_yaz(s, tarama_t, L);
}

static int str_karsilastir(const void *a, const void *c)
{
	return strcmp((const char *)a, (const char *)c);
}

/* KAYIT_OLAYLAR <gun|*> [esik] */
static int kayit_olaylar(int s, const char *gun, const char *esik_s)
{
	unsigned long long esik = 0;
	if (esik_s && (sayi_al(esik_s, &esik) || esik > 1000)) return hata(s, "esik");
	if (!gun || (strcmp(gun, "*") && !gun_gecerli(gun))) return hata(s, "gun");
	static char gunler[128][9];
	int gn = 0;
	if (!strcmp(gun, "*")) {
		DIR *d = opendir(kok);
		struct dirent *e;
		while (d && (e = readdir(d)) && gn < 128)
			if (gun_gecerli(e->d_name)) { memcpy(gunler[gn], e->d_name, 9); gn++; }
		if (d) closedir(d);
		qsort(gunler, (size_t)gn, sizeof gunler[0], str_karsilastir);
	} else {
		memcpy(gunler[gn++], gun, 9);
	}
	static int skor[1440], kare[1440], tepe[1440], tms[1440];
	static unsigned long long bayt[1440];
	static uint8_t var_[1440];
	int hesaplanan = 0, rc = 0;
	for (int g = 0; !rc && g < gn; g++) {
		char yol[600], satir[128];
		memset(var_, 0, sizeof var_);
		for (int m = 0; m < 1440; m++) skor[m] = -1;
		snprintf(yol, sizeof yol, "%s/%s", kok, gunler[g]);
		DIR *d = opendir(yol);
		struct dirent *e;
		while (d && (e = readdir(d)))
			if (mjpg_adi(e->d_name)) {
				int sa = iki_sayi(e->d_name), dk = iki_sayi(e->d_name + 2);
				if (sa <= 23 && dk <= 59) var_[sa * 60 + dk] = 1;
			}
		if (d) closedir(d);
		snprintf(yol, sizeof yol, "%s/%s/olay.csv", kok, gunler[g]);
		FILE *f = fopen(yol, "r");
		while (f && fgets(satir, sizeof satir, f)) {
			char hm[5];
			int sk, kr, tp, tm_;
			unsigned long long by;
			if (sscanf(satir, "%4[0-9],%d,%d,%llu,%d,%d", hm, &sk, &kr, &by, &tp, &tm_) != 6 || strlen(hm) != 4) continue;
			int sa = iki_sayi(hm), dk = iki_sayi(hm + 2);
			if (sa > 23 || dk > 59 || sk < 0 || sk > 1000 || kr < 0) continue;
			int m = sa * 60 + dk;
			skor[m] = sk; kare[m] = kr; bayt[m] = by; tepe[m] = tp; tms[m] = tm_;
		}
		if (f) fclose(f);
		for (int m = 0; !rc && m < 1440; m++) {
			if (!var_[m]) continue;
			char ad[32];
			snprintf(ad, sizeof ad, "%.8s/%02d%02d.mjpg", gunler[g], m / 60, m % 60);
			if (skor[m] < 0 && hesaplanan < 64) {	/* ozet yok: yalniz yan dizinden hesapla (tarama yok) */
				char tam[600], iy[600];
				struct stat st;
				snprintf(tam, sizeof tam, "%s/%s", kok, ad);
				snprintf(iy, sizeof iy, "%s/%.13s.idx", kok, ad);
				if (!stat(tam, &st) && dosya_var(iy)) {
					hesaplanan++;
					int n = idx_oku(iy, (unsigned long long)st.st_size, kx, OLAY_AZAMI_KARE);
					if (n > 0) {
						static uint32_t len[OLAY_AZAMI_KARE];
						unsigned long long top = 0;
						for (int i = 0; i < n; i++) { len[i] = kx[i].len; top += kx[i].len; }
						int tp = 0;
						skor[m] = olay_skor(len, n, &tp);
						kare[m] = n; bayt[m] = top; tepe[m] = tp; tms[m] = kx[tp].ms;
					}
				}
			}
			if (skor[m] < 0 || (unsigned long long)skor[m] < esik) continue;
			int k = snprintf(satir, sizeof satir, "%s %d %d %llu %d %d\n", ad, skor[m], kare[m], bayt[m], tepe[m], tms[m]);
			rc = tam_yaz(s, satir, (size_t)k);
		}
	}
	return rc ? rc : tam_yaz(s, "SON\n", 4);
}
#endif
