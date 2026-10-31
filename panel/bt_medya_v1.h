/* Panel 0.23 (13.10.2026, Claude): /tmp/bt-durum.json'dan muzik durumu (mcu-servis v4 "music_state"/"music_code").
 * YALNIZ OKUR; MCU'ya hicbir sey gondermez. Tazelik kurallari bt_status.h ile ayni (bt_status_parse cagrilir).
 * mcu-servis v4: MB -> 3, MA/MC -> 2, digerleri 0, bilinmiyor -1. Anlamlar docs/mcu-bt-protokol.md'de "tahmin":
 * 3 = caliyor, 2 = duraklatildi/durdu kabul edilir; arayuz bunu "(tahmini)" diye gosterir. */
#ifndef VELINUX_BT_MEDYA_V1_H
#define VELINUX_BT_MEDYA_V1_H
#include "bt_status.h"
#include <stdio.h>

struct bt_medya {
	int taze, baglanti, durum, muzik_olayi;
	double yas;
	char kod[8];
};

static struct bt_medya bt_medya_coz(const char *json, double now)
{
	struct bt_medya m = {0, -1, -1, 0, 0, ""};
	struct bt_status s = bt_status_parse(json, now);
	m.taze = s.fresh;
	m.baglanti = s.connection;
	m.muzik_olayi = s.music_event;
	m.yas = s.music_age;
	if (!s.fresh)
		return m;
	cJSON *o = cJSON_Parse(json);
	if (cJSON_IsObject(o)) {
		cJSON *d = cJSON_GetObjectItemCaseSensitive(o, "music_state");
		if (cJSON_IsNumber(d) && (d->valueint == 0 || d->valueint == 2 || d->valueint == 3))
			m.durum = d->valueint;
		cJSON *k = cJSON_GetObjectItemCaseSensitive(o, "music_code");
		if (s.music_event && cJSON_IsString(k) && k->valuestring)
			snprintf(m.kod, sizeof m.kod, "%.2s", k->valuestring);
	}
	cJSON_Delete(o);
	return m;
}

static const char *bt_medya_durum_metni(const struct bt_medya *m)
{
	if (!m->taze)
		return "Bilgi yok";
	if (m->durum == 3)
		return "Çalıyor (tahmini)";
	if (m->durum == 2)
		return "Duraklatıldı (tahmini)";
	return m->muzik_olayi ? "Durum bilinmiyor" : "Müzik olayı yok";
}
#endif
