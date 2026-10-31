/* Panel 0.22 (13.10.2026, Claude): kamera-kayit durumu (/tmp/kamera-kayit.json) ve Wi-Fi sinyal duzeyi.
 * YALNIZ OKUR: kameraya, USB'ye ya da SD'ye dokunmaz. JSON uretici: initramfs/src/kamera_kayit_v1.c durum_yaz()
 *   {"surum":"kamera-kayit-v1","canli":bool,"kayit":bool,"fps":f,"kareler":n,"parca":"..","parca_sayi":n,
 *    "toplam_mb":f,"bos_mb":n,"azami_parca":n,"kayit_fps":f,"hata":".."}
 * Hizmet her saniye yazar; dosya `yas_sn` saniyeden eskiyse hizmet calismiyor sayilir. */
#ifndef VELINUX_KAMERA_DURUM_V1_H
#define VELINUX_KAMERA_DURUM_V1_H
#include "cJSON.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

#define KAMERA_TAZE_SN 5.0

struct kamera_durum {
    int gecerli;	/* JSON okunabildi ve surum taniniyor */
    int taze;		/* hizmet son KAMERA_TAZE_SN icinde yazdi */
    int canli, kayit;
    double fps, toplam_mb, bos_mb, kayit_fps;
    long kareler;
    int parca_sayi, azami_parca;
    char parca[48];
    char hata[64];
};

static double kd_sayi(cJSON *o, const char *k, double d)
{
    cJSON *v = cJSON_GetObjectItemCaseSensitive(o, k);
    return cJSON_IsNumber(v) && isfinite(v->valuedouble) ? v->valuedouble : d;
}

static void kd_metin(cJSON *o, const char *k, char *out, size_t n)
{
    cJSON *v = cJSON_GetObjectItemCaseSensitive(o, k);
    out[0] = 0;
    if (cJSON_IsString(v) && v->valuestring)
        snprintf(out, n, "%s", v->valuestring);
}

/* yas_sn: dosyanin son degisikliginden beri gecen sure (bilinmiyorsa < 0 -> bayat sayilir) */
static struct kamera_durum kamera_durum_parse(const char *json, double yas_sn)
{
    struct kamera_durum k;
    memset(&k, 0, sizeof k);
    k.bos_mb = -1;
    cJSON *o = json ? cJSON_Parse(json) : NULL;
    if (!cJSON_IsObject(o))
        goto bitti;
    cJSON *s = cJSON_GetObjectItemCaseSensitive(o, "surum");
    if (!cJSON_IsString(s) || strncmp(s->valuestring, "kamera-kayit", 12))
        goto bitti;
    k.gecerli = 1;
    k.taze = yas_sn >= 0 && yas_sn < KAMERA_TAZE_SN;
    k.canli = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(o, "canli"));
    k.kayit = cJSON_IsTrue(cJSON_GetObjectItemCaseSensitive(o, "kayit"));
    k.fps = kd_sayi(o, "fps", 0);
    if (k.fps < 0 || k.fps > 1000)
        k.fps = 0;
    double kr = kd_sayi(o, "kareler", 0);
    k.kareler = kr >= 0 && kr < 1e15 ? (long)kr : 0;
    double ps = kd_sayi(o, "parca_sayi", 0), az = kd_sayi(o, "azami_parca", 0);
    k.parca_sayi = ps >= 0 && ps < 1e6 ? (int)ps : 0;
    k.azami_parca = az > 0 && az < 1e6 ? (int)az : 0;
    k.toplam_mb = kd_sayi(o, "toplam_mb", 0);
    if (k.toplam_mb < 0)
        k.toplam_mb = 0;
    k.bos_mb = kd_sayi(o, "bos_mb", -1);
    k.kayit_fps = kd_sayi(o, "kayit_fps", 0);
    kd_metin(o, "parca", k.parca, sizeof k.parca);
    kd_metin(o, "hata", k.hata, sizeof k.hata);
bitti:
    cJSON_Delete(o);
    return k;
}

/* MB -> "850 MB" / "12.4 GB" */
static void kd_boyut(char *o, size_t n, double mb)
{
    if (mb < 0)
        snprintf(o, n, "-");
    else if (mb >= 1024)
        snprintf(o, n, "%.1f GB", mb / 1024.0);
    else
        snprintf(o, n, "%.0f MB", mb);
}

/* iw "signal: -NN dBm" -> 0..4 cubuk (0: bagli degil/bilinmiyor; panel bilinmeyeni -1 ile bildirir) */
static int wifi_duzey(int dbm)
{
    if (dbm > -10 || dbm < -100)
        return 0;
    if (dbm >= -55)
        return 4;
    if (dbm >= -65)
        return 3;
    if (dbm >= -75)
        return 2;
    return 1;
}
#endif
