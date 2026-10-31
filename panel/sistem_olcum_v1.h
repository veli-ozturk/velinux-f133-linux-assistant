/* Panel 0.23 (13.10.2026, Claude): Sistem monitoru icin /proc ve /sys metin cozuculeri. YALNIZ OKUR.
 * Kaynaklar: /proc/stat (ilk "cpu " satiri), /proc/meminfo (MemTotal, MemAvailable), /proc/self/status (VmRSS),
 * /proc/net/dev (arayuz rx/tx bayt), /sys/class/thermal/thermal_zone0/temp (milli-derece).
 * Saf islevler (dosya/LVGL yok); host testi tests/panel_023_test_v1.c. */
#ifndef VELINUX_SISTEM_OLCUM_V1_H
#define VELINUX_SISTEM_OLCUM_V1_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cpu_ornek {
	unsigned long long toplam, bos;
};

/* Donus: 0 tamam, -1 cozulemedi */
static int cpu_ornek_coz(const char *stat, struct cpu_ornek *o)
{
	unsigned long long v[8] = {0};
	if (!stat || strncmp(stat, "cpu ", 4))
		return -1;
	int n = sscanf(stat + 4, "%llu %llu %llu %llu %llu %llu %llu %llu", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5],
		       &v[6], &v[7]);
	if (n < 4)
		return -1;
	o->toplam = 0;
	for (int i = 0; i < n; i++)
		o->toplam += v[i];
	o->bos = v[3] + (n > 4 ? v[4] : 0);	/* idle + iowait */
	return 0;
}

/* a: onceki, b: simdiki. Donus: 0..100, sayac geri gittiyse ya da fark yoksa -1 */
static int cpu_yuzde(const struct cpu_ornek *a, const struct cpu_ornek *b)
{
	if (b->toplam <= a->toplam || b->bos < a->bos)
		return -1;
	unsigned long long dt = b->toplam - a->toplam, db = b->bos - a->bos;
	if (db > dt)
		return -1;
	return (int)(((dt - db) * 100 + dt / 2) / dt);
}

static long kb_degeri(const char *metin, const char *anahtar)
{
	const char *p = metin;
	size_t n = strlen(anahtar);
	while (p && *p) {
		if (!strncmp(p, anahtar, n) && p[n] == ':')
			return strtol(p + n + 1, NULL, 10);
		p = strchr(p, '\n');
		if (p)
			p++;
	}
	return -1;
}

struct bellek_durum {
	long toplam_kb, uygun_kb;
};

static int bellek_coz(const char *meminfo, struct bellek_durum *b)
{
	b->toplam_kb = kb_degeri(meminfo, "MemTotal");
	b->uygun_kb = kb_degeri(meminfo, "MemAvailable");
	if (b->uygun_kb < 0) {	/* eski cekirdek: MemFree + Cached yaklasik */
		long f = kb_degeri(meminfo, "MemFree"), c = kb_degeri(meminfo, "Cached");
		b->uygun_kb = f >= 0 ? f + (c > 0 ? c : 0) : -1;
	}
	return b->toplam_kb > 0 && b->uygun_kb >= 0 && b->uygun_kb <= b->toplam_kb ? 0 : -1;
}

static long vmrss_kb(const char *status)
{
	return kb_degeri(status, "VmRSS");
}

/* /proc/net/dev: "  wlan0: rx_bayt rx_paket ... (8 alan) tx_bayt ..." Donus: 0 bulundu, -1 yok */
static int netdev_coz(const char *metin, const char *ad, unsigned long long *rx, unsigned long long *tx)
{
	const char *p = metin;
	size_t n = strlen(ad);
	while (p && *p) {
		while (*p == ' ')
			p++;
		if (!strncmp(p, ad, n) && p[n] == ':') {
			unsigned long long v[9];
			if (sscanf(p + n + 1, "%llu %llu %llu %llu %llu %llu %llu %llu %llu", &v[0], &v[1], &v[2], &v[3], &v[4],
				   &v[5], &v[6], &v[7], &v[8]) == 9) {
				*rx = v[0];
				*tx = v[8];
				return 0;
			}
			return -1;
		}
		p = strchr(p, '\n');
		if (p)
			p++;
	}
	return -1;
}

/* milli-derece ("41250") ya da derece ("41"); Donus: 0 tamam, -1 gecersiz */
static int sicaklik_coz(const char *metin, double *c)
{
	char *bit;
	if (!metin)
		return -1;
	long v = strtol(metin, &bit, 10);
	if (bit == metin)
		return -1;
	double d = (v > 1000 || v < -1000) ? v / 1000.0 : (double)v;
	if (d < -40 || d > 150)
		return -1;
	*c = d;
	return 0;
}

/* saniye -> "3 gün 4 sa" / "5 sa 12 dk" / "7 dk" */
static void sure_bicim(char *o, size_t n, long sn)
{
	if (sn < 0)
		sn = 0;
	long g = sn / 86400, s = (sn / 3600) % 24, d = (sn / 60) % 60;
	if (g)
		snprintf(o, n, "%ld gün %ld sa", g, s);
	else if (s)
		snprintf(o, n, "%ld sa %ld dk", s, d);
	else
		snprintf(o, n, "%ld dk", d);
}

/* bayt/sn -> "1.2 MB/s" / "340 KB/s" / "12 B/s" */
static void hiz_bicim(char *o, size_t n, double bps)
{
	if (bps >= 1e6)
		snprintf(o, n, "%.1f MB/s", bps / 1e6);
	else if (bps >= 1e3)
		snprintf(o, n, "%.0f KB/s", bps / 1e3);
	else
		snprintf(o, n, "%.0f B/s", bps < 0 ? 0 : bps);
}
#endif
