/* ui_pasif_v2.h (22.10.2026, Claude; panel 0.72-v2, OTONOM madde 111): v1 + pc_sistem ve pc_medya da pasif.
 * v1 cihazda dogrulanirken (18:38) uzaktan acilan koruyucu yine 2 sn'de kapandi: PC'deki pc_sistem_v1 2 sn'de bir
 * {"t":"ui","pc_sistem":{...}}, pc_medya_v1 30 sn'de bir {"t":"ui","pc_medya":{...}} gonderiyor (ajan gunlugu 09:36-18:39:
 * 16440 pc_sistem, 1087 pc_medya; kullanici/otomasyon komutlari: sayfa 36, pc_full 18, ss 8, theme 5, dm 4, gunluk 3, ayar 2).
 * --- v1 ---
 * Bir {"t":"ui",...} iletisi kullanici etkinligi mi? PC ajani 2 sn'de bir {"t":"ui","pc_status":"..."} gonderiyor; panel
 * her ui iletisini etkinlik sayip koruyucuyu kapatiyor ve etkinsizlik sayacini sifirliyordu -> PC ajani bagliyken otomatik
 * koruyucu HIC acilmiyordu (3 sa 53 dk'da 0), uzaktan acilan da en cok 2 sn yasiyordu.
 * Pasif = "t" disindaki BUTUN anahtarlar durum/veri/tanilama anahtari: pc_status (ajanin durum satiri), brightness (ajanin
 * gece/gunduz otomatik parlakligi), pc_sistem (PC Sistem verisi), pc_medya (PC'de calan medya), gunluk + ara
 * (panel_gunluk_oku tanilama istegi). Baska herhangi bir anahtar varsa (sayfa, ss, dm, press, pc_full ...) ya da "t"
 * disinda anahtar yoksa ileti eskisi gibi etkinliktir. Yalniz cJSON. */
#ifndef UI_PASIF_V2_H
#define UI_PASIF_V2_H

#include <string.h>
#include "cJSON.h"

static int ui_pasif(const cJSON *j)
{
	static const char *const pasif[] = {"t", "pc_status", "brightness", "pc_sistem", "pc_medya", "gunluk", "ara"};
	int n = 0;

	if (!cJSON_IsObject(j))
		return 0;
	for (const cJSON *c = j->child; c; c = c->next) {
		int bul = 0;
		for (size_t i = 0; i < sizeof pasif / sizeof pasif[0] && !bul; i++)
			bul = c->string && !strcmp(c->string, pasif[i]);
		if (!bul)
			return 0;
		n += strcmp(c->string, "t") != 0;
	}
	return n > 0;
}

#endif
