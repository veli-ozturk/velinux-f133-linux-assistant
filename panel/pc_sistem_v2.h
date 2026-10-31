/* pc_sistem_v2.h (22.10.2026, Claude; panel 0.73, scripts/534): v1 + "ilk_ram" (en cok BELLEK kullanan 5 surec,
 * [[ad, cpu x10, MB]]; work/pc_sistem_v2.py). Yoksa ram_n = 0.
 * --- v1 ---
 * pc_sistem_v1.h (21.10.2026, Claude; panel 0.67, scripts/502): PC Sistem sayfasinin VERI kismi (LVGL'siz, host'ta test
 * edilir). {"t":"ui","pc_sistem":{...}} (work/pc_sistem_v1.py, 2 sn'de bir) -> struct pcs + 90 ornek gecmis (3 dk).
 * Arayuz main'de (pcs_build/pcs_tick). Alanlar ve birimler pc_sistem_v1.py ile ayni. */
#ifndef PC_SISTEM_V2_H
#define PC_SISTEM_V2_H
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cJSON.h"

#define PCS_N 90	/* gecmis ornek sayisi (2 sn -> 3 dk) */
#define PCS_CEK 32
#define PCS_SUR 4
#define PCS_ILK 5

struct pcs_veri {
	char ad[32], gpu_ad[32];
	int cpu, cek_n, cek[PCS_CEK];
	int ram_y, ram_kul, ram_top, cm, cml;	/* MB */
	int gpu_var, gpu, vk, vt, gsic, gguc10, gsaat, gfan;
	int oku, yaz, etkin;	/* KB/s, % */
	int sur_n, sur_kul[PCS_SUR], sur_top[PCS_SUR];
	char sur_ad[PCS_SUR][4];
	int ag_in, ag_out;	/* KB/s */
	int surec, ip, calisma, sic;
	int ilk_n, ilk_cpu10[PCS_ILK], ilk_mb[PCS_ILK];
	char ilk_ad[PCS_ILK][26];
	int ram_n, ram_cpu10[PCS_ILK], ram_mb[PCS_ILK];	/* v2: en cok bellek */
	char ram_ad[PCS_ILK][26];
	/* gecmis (halka) */
	int16_t g_cpu[PCS_N], g_ram[PCS_N], g_gpu[PCS_N], g_disk[PCS_N];
	int32_t g_in[PCS_N], g_out[PCS_N];
	int bas, say;	/* en eski ornegin yeri, ornek sayisi */
	uint32_t n;	/* alinan toplam ileti */
};

static int pcs_int(const cJSON *o, const char *ad, int varsayilan)
{
	const cJSON *a = o ? cJSON_GetObjectItem(o, ad) : NULL;
	return cJSON_IsNumber(a) ? a->valueint : varsayilan;
}

static void pcs_str(const cJSON *o, const char *ad, char *hedef, size_t n)
{
	const cJSON *a = o ? cJSON_GetObjectItem(o, ad) : NULL;
	snprintf(hedef, n, "%s", cJSON_IsString(a) ? a->valuestring : "");
}

static int pcs_sinir(int v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : v; }

/* ileti -> veri + gecmise bir ornek; 0 tamam, -1 bicim yok */
static int pcs_isle(struct pcs_veri *p, const cJSON *o)
{
	if (!cJSON_IsObject(o))
		return -1;
	pcs_str(o, "ad", p->ad, sizeof p->ad);
	p->cpu = pcs_sinir(pcs_int(o, "cpu", 0), 0, 100);
	const cJSON *c = cJSON_GetObjectItem(o, "cek");
	p->cek_n = 0;
	if (cJSON_IsArray(c))
		for (const cJSON *e = c->child; e && p->cek_n < PCS_CEK; e = e->next)
			p->cek[p->cek_n++] = cJSON_IsNumber(e) ? pcs_sinir(e->valueint, 0, 100) : 0;
	const cJSON *r = cJSON_GetObjectItem(o, "ram");
	p->ram_y = pcs_sinir(pcs_int(r, "y", 0), 0, 100);
	p->ram_kul = pcs_int(r, "kul", 0);
	p->ram_top = pcs_int(r, "top", 0);
	p->cm = pcs_int(r, "cm", 0);
	p->cml = pcs_int(r, "cml", 0);
	const cJSON *g = cJSON_GetObjectItem(o, "gpu");
	p->gpu_var = cJSON_IsObject(g);
	pcs_str(g, "ad", p->gpu_ad, sizeof p->gpu_ad);
	p->gpu = pcs_int(g, "kul", -1);
	p->vk = pcs_int(g, "vk", -1);
	p->vt = pcs_int(g, "vt", -1);
	p->gsic = pcs_int(g, "sic", -1);
	p->gguc10 = pcs_int(g, "guc", -1);
	p->gsaat = pcs_int(g, "saat", -1);
	p->gfan = pcs_int(g, "fan", -1);
	const cJSON *d = cJSON_GetObjectItem(o, "disk");
	p->oku = pcs_int(d, "oku", 0);
	p->yaz = pcs_int(d, "yaz", 0);
	p->etkin = pcs_sinir(pcs_int(d, "etkin", 0), 0, 100);
	const cJSON *s = d ? cJSON_GetObjectItem(d, "sur") : NULL;
	p->sur_n = 0;
	if (cJSON_IsArray(s))
		for (const cJSON *e = s->child; e && p->sur_n < PCS_SUR; e = e->next) {
			const cJSON *a = cJSON_GetArrayItem(e, 0), *k = cJSON_GetArrayItem(e, 1), *t = cJSON_GetArrayItem(e, 2);
			if (!cJSON_IsString(a) || !cJSON_IsNumber(k) || !cJSON_IsNumber(t))
				continue;
			snprintf(p->sur_ad[p->sur_n], sizeof p->sur_ad[0], "%s", a->valuestring);
			p->sur_kul[p->sur_n] = k->valueint;
			p->sur_top[p->sur_n] = t->valueint;
			p->sur_n++;
		}
	const cJSON *n = cJSON_GetObjectItem(o, "ag");
	p->ag_in = pcs_int(n, "in", 0);
	p->ag_out = pcs_int(n, "out", 0);
	p->surec = pcs_int(o, "surec", 0);
	p->ip = pcs_int(o, "ip", 0);
	p->calisma = pcs_int(o, "calisma", 0);
	p->sic = pcs_int(o, "sic", -1);
	const cJSON *il = cJSON_GetObjectItem(o, "ilk");
	p->ilk_n = 0;
	if (cJSON_IsArray(il))
		for (const cJSON *e = il->child; e && p->ilk_n < PCS_ILK; e = e->next) {
			const cJSON *a = cJSON_GetArrayItem(e, 0), *u = cJSON_GetArrayItem(e, 1), *m = cJSON_GetArrayItem(e, 2);
			if (!cJSON_IsString(a) || !cJSON_IsNumber(u) || !cJSON_IsNumber(m))
				continue;
			snprintf(p->ilk_ad[p->ilk_n], sizeof p->ilk_ad[0], "%s", a->valuestring);
			p->ilk_cpu10[p->ilk_n] = u->valueint;
			p->ilk_mb[p->ilk_n] = m->valueint;
			p->ilk_n++;
		}
	const cJSON *ir = cJSON_GetObjectItem(o, "ilk_ram");	/* v2 */
	p->ram_n = 0;
	if (cJSON_IsArray(ir))
		for (const cJSON *e = ir->child; e && p->ram_n < PCS_ILK; e = e->next) {
			const cJSON *a = cJSON_GetArrayItem(e, 0), *u = cJSON_GetArrayItem(e, 1), *m = cJSON_GetArrayItem(e, 2);
			if (!cJSON_IsString(a) || !cJSON_IsNumber(u) || !cJSON_IsNumber(m))
				continue;
			snprintf(p->ram_ad[p->ram_n], sizeof p->ram_ad[0], "%s", a->valuestring);
			p->ram_cpu10[p->ram_n] = u->valueint;
			p->ram_mb[p->ram_n] = m->valueint;
			p->ram_n++;
		}
	/* gecmis: halka dolunca en eskinin yerine */
	int j = p->say < PCS_N ? (p->bas + p->say) % PCS_N : p->bas;
	p->g_cpu[j] = (int16_t)p->cpu;
	p->g_ram[j] = (int16_t)p->ram_y;
	p->g_gpu[j] = (int16_t)(p->gpu < 0 ? 0 : pcs_sinir(p->gpu, 0, 100));
	p->g_disk[j] = (int16_t)p->etkin;
	p->g_in[j] = p->ag_in < 0 ? 0 : p->ag_in;
	p->g_out[j] = p->ag_out < 0 ? 0 : p->ag_out;
	if (p->say < PCS_N)
		p->say++;
	else
		p->bas = (p->bas + 1) % PCS_N;
	p->n++;
	return 0;
}

/* k. en eski ornek (0 = en eski) */
static int pcs_gecmis_i(const struct pcs_veri *p, int k) { return (p->bas + k) % PCS_N; }

/* ag gecmisinin en buyugu (grafik ust siniri; en az 64 KB/s) */
static int32_t pcs_ag_tepe(const struct pcs_veri *p)
{
	int32_t m = 64;
	for (int k = 0; k < p->say; k++) {
		int j = pcs_gecmis_i(p, k);
		if (p->g_in[j] > m) m = p->g_in[j];
		if (p->g_out[j] > m) m = p->g_out[j];
	}
	return m;
}

/* KB/s -> "512 KB/s" | "3.4 MB/s" */
static void pcs_hiz(char *o, size_t n, int kb)
{
	if (kb < 1000)
		snprintf(o, n, "%d KB/s", kb < 0 ? 0 : kb);
	else
		snprintf(o, n, "%d.%d MB/s", kb / 1024, (kb % 1024) * 10 / 1024);
}
#endif
