/* zaman_durum_v1.h (22.10.2026, Claude; panel 0.74, scripts/545): time-lapse durum metinleri. kamera-kayit v9
 * /tmp/kamera-kayit.json alanlari: zaman_olay, zaman_mb (tum gunler, -1 bilinmiyor), zaman_sinir_mb, zaman_dolu.
 * Eski kamera-kayit (zaman_sinir_mb yok): gecerli = 0, doluluk satiri bos. Saf C + cJSON; tests/zaman_durum_test_v1.c. */
#ifndef ZAMAN_DURUM_V1_H
#define ZAMAN_DURUM_V1_H

#include <stdio.h>
#include "cJSON.h"

struct zaman_durum {
	int gecerli, olay, dolu, sinir_mb;
	double mb;	/* -1: bilinmiyor */
};

static inline struct zaman_durum zaman_durum_parse(const char *json)
{
	struct zaman_durum z = {0, 0, 0, 0, -1};
	cJSON *j = json ? cJSON_Parse(json) : NULL;
	if (!j)
		return z;
	const cJSON *s = cJSON_GetObjectItem(j, "zaman_sinir_mb"), *mb = cJSON_GetObjectItem(j, "zaman_mb");
	if (cJSON_IsNumber(s) && s->valueint > 0) {
		z.gecerli = 1;
		z.sinir_mb = s->valueint;
		z.mb = cJSON_IsNumber(mb) ? mb->valuedouble : -1;
		z.olay = cJSON_IsTrue(cJSON_GetObjectItem(j, "zaman_olay"));
		z.dolu = cJSON_IsTrue(cJSON_GetObjectItem(j, "zaman_dolu"));
	}
	cJSON_Delete(j);
	return z;
}

/* zaman-aralik dosyasindaki deger: 1..3600 gecerli, yoksa 60 (kamera-kayit v9 varsayilani) */
static inline int zaman_aralik_dogrula(int v)
{
	return v >= 1 && v <= 3600 ? v : 60;
}

/* ust satir: "kapalı" ya da "açık · her 3 sn · olay tespiti · bugün 12.3 MB" (+ kalkan) */
static inline void zaman_durum_metni(char *o, size_t n, int ac, int aralik, int olay, double bugun_mb, int kalkan)
{
	if (!ac) {
		snprintf(o, n, "kapalı%s", olay ? " · olay tespiti hazır" : "");
		return;
	}
	char ar[24];
	if (aralik >= 60 && aralik % 60 == 0)
		snprintf(ar, sizeof ar, "%d dk", aralik / 60);
	else
		snprintf(ar, sizeof ar, "%d sn", aralik);
	snprintf(o, n, "açık · her %s%s · bugün %.1f MB%s", ar, olay ? " · olay tespiti" : "", bugun_mb < 0 ? 0 : bugun_mb,
		 kalkan ? " · kalkan bekletiyor" : "");
}

/* alt satir: doluluk (eski kamera-kayit ya da bilinmiyorsa bos) */
static inline void zaman_doluluk_metni(char *o, size_t n, const struct zaman_durum *z)
{
	if (!z->gecerli || z->mb < 0) {
		snprintf(o, n, "%s", "");
		return;
	}
	snprintf(o, n, "Depolama %.0f / %d MB%s", z->mb, z->sinir_mb,
		 z->dolu ? " · BUGÜN DOLDU: yeni kare eklenmiyor" : " · sınır aşılınca en eski gün silinir");
}

#endif
