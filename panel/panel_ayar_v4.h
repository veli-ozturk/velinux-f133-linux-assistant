/* Panel 0.44 (19.10.2026, Claude): panel_ayar_v3.h + koruyucu_tur 0..10 (7 kelime saati, 8 dunya saatleri,
 * 9 yildiz alani, 10 aurora). Host testi: tests/panel_044_test_v1.c.
 * --- v3 aciklamasi ---
 * Panel 0.40 (19.10.2026, Claude): panel_ayar_v2.h + varsayilan tema 1 (Kirmizi & Siyah, kullanıcı istegi) ve
 * koruyucu_tur 0..6 (3 analog saat, 4 kayan logo, 5 bilgi panosu, 6 karartma). Eski panel (v2) 3..6 degerini
 * aralik disi sayip varsayilana (robot goz) duser. Host testi: tests/panel_040_test_v1.c.
 * --- v2 aciklamasi ---
 * Panel 0.24 (13.10.2026, Claude): panel_ayar_v1.h + koruyucu_tur (0 saat, 1 robot goz, 2 karisik; varsayilan 1).
 * v1 (0.23) bu dosyayi okuyabilir (koruyucu_tur bilinmeyen anahtar -> atlanir); 0.23 kaydederse anahtar duser,
 * 0.24'e donunce varsayilan (robot goz) gelir. Host testi: tests/panel_024_test_v1.c.
 * --- v1 aciklamasi ---
 * Panel 0.23 (13.10.2026, Claude): /data/panel.conf ayarlari (anahtar=tamsayi satirlari).
 * 0.22 bicimiyle geriye uyumlu: eski anahtarlar ayni (parlaklik_otomatik, parlaklik_manuel, tema, koruyucu);
 * yeni anahtarlar eski panelde yok sayilir (0.22 fscanf dongusu bilinmeyen anahtari atlar).
 * Yeni: koruyucu_dk (1..60), ses_duzey (0..100), sessiz (0/1), rahatsiz_etme (0/1).
 * Saf islevler (dosya/LVGL yok): host testi tests/panel_023_test_v1.c. */
#ifndef VELINUX_PANEL_AYAR_V4_H
#define VELINUX_PANEL_AYAR_V4_H
#define PANEL_SS_TUR_N 11
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct panel_ayar {
	int br_auto, br_manual, tema, ss_on, ss_dk, ses, sessiz, dnd, ss_tur;
};

static void panel_ayar_varsayilan(struct panel_ayar *a)
{
	a->br_auto = 1;
	a->br_manual = 80;
	a->tema = 1;	/* 0.40: Kirmizi & Siyah */
	a->ss_on = 1;
	a->ss_dk = 3;
	a->ses = 60;
	a->sessiz = 0;
	a->dnd = 0;
	a->ss_tur = 1;	/* 0.24: Robot goz */
}

/* metin: dosya icerigi (NUL sonlu). Gecersiz/aralik disi degerler yok sayilir (varsayilan kalir). */
static void panel_ayar_coz(struct panel_ayar *a, const char *metin, int tema_n)
{
	const char *p = metin;
	while (p && *p) {
		const char *son = strchr(p, '\n');
		size_t n = son ? (size_t)(son - p) : strlen(p);
		char satir[64];
		if (n < sizeof satir) {
			memcpy(satir, p, n);
			satir[n] = 0;
			if (n && satir[n - 1] == '\r')
				satir[n - 1] = 0;
			char *esit = strchr(satir, '=');
			if (esit && esit != satir && esit[1]) {
				*esit = 0;
				char *bit;
				long v = strtol(esit + 1, &bit, 10);
				if (!*bit) {
					const char *k = satir;
					if (!strcmp(k, "parlaklik_otomatik"))
						a->br_auto = !!v;
					else if (!strcmp(k, "parlaklik_manuel") && v >= 5 && v <= 100)
						a->br_manual = (int)v;
					else if (!strcmp(k, "tema") && v >= 0 && v < tema_n)
						a->tema = (int)v;
					else if (!strcmp(k, "koruyucu"))
						a->ss_on = !!v;
					else if (!strcmp(k, "koruyucu_dk") && v >= 1 && v <= 60)
						a->ss_dk = (int)v;
					else if (!strcmp(k, "ses_duzey") && v >= 0 && v <= 100)
						a->ses = (int)v;
					else if (!strcmp(k, "sessiz"))
						a->sessiz = !!v;
					else if (!strcmp(k, "rahatsiz_etme"))
						a->dnd = !!v;
					else if (!strcmp(k, "koruyucu_tur") && v >= 0 && v < PANEL_SS_TUR_N)
						a->ss_tur = (int)v;
				}
			}
		}
		p = son ? son + 1 : NULL;
	}
}

/* Donus: yazilan uzunluk; sigmazsa -1. */
static int panel_ayar_yaz(const struct panel_ayar *a, char *out, size_t n)
{
	int r = snprintf(out, n,
			 "parlaklik_otomatik=%d\nparlaklik_manuel=%d\ntema=%d\nkoruyucu=%d\n"
			 "koruyucu_dk=%d\nses_duzey=%d\nsessiz=%d\nrahatsiz_etme=%d\nkoruyucu_tur=%d\n",
			 a->br_auto, a->br_manual, a->tema, a->ss_on, a->ss_dk, a->ses, a->sessiz, a->dnd, a->ss_tur);
	return r < 0 || (size_t)r >= n ? -1 : r;
}
#endif
