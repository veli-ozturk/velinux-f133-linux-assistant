/* h264_ve_v3.h (23.10.2026, Claude; docs/denetim-2026-10-23-v2.md bulgu 7): v2 + akis dogrulamasi: PPS ve dilim
 * basligindaki ref sayisi <= 32 (rl[32] yigin dizisi), SPS yalniz HV_W_MB x HV_H_MU (1600x608, wifi-ekran) kabul edilir
 * (tamponlar SPS'ten ayrilmadan once; v2 1920x1088 + 6 konum ~19 MB ayirabiliyordu).
 * h264_ve_v2.h (15.10.2026, Claude): v1 + DIS TAMPONLAR (bit akisi ve konum 0 cagirandan) + tek mv_col (bellek; scripts/258).
 * h264_ve_v1.h (15.10.2026, Claude): F133 VE donanim H.264 cozucusu, gomulebilir modul (hv_ oneki).
 * Mantik h264_ve_prova_v4.c ile BIREBIR (cihazda 6 akis kare kare dogru: CAVLC/CABAC, cok dilim, cok referans DPB,
 * NVENC high/8x8; 4-6 ms/kare). Yazmac/SRAM: cedrus_h264.c; dilim basligi VE bit okuyucusuyla (se = ue esleme).
 * Kullanim (cagiran ENGINE_REQ yapmis ve VE yazmaclarini eslemis olmali):
 *   hv_baslat(ve_fd, ion_fd, ion_yigin_maskesi, R)  -> 0
 *   hv_au(veri, n, &kare)  -> 0: en az bir kare tamamlandi (kare.y/c IOMMU adresi, aw/ah hizali, w/h kirpilmis)
 *                           1: kare yok (yalniz SPS/PPS/SEI), <0 hata (-2 VE zaman asimi: cagiran VE_RESET yapar)
 *   hv_bitir()
 * Desteklenmeyen: alan/MBAFF, olcek matrisi, agirlikli tahmin, POC turu 1, B dilimi, MMCO/liste degisikligi (uyari). */
#ifndef H264_VE_V3_H
#define H264_VE_V3_H
#define HV_W_MB 100	/* v3: 1600 / 16 */
#define HV_H_MU 38	/* v3: 608 / 16 (600 + 8 kirpma) */
#define HV_REF_EN 32	/* v3: rl[] boyu */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define HV_VE_MAP_DMA_BUF 0x504
#define HV_VE_UNMAP_DMA_BUF 0x505
struct hv_dma_param { int fd; unsigned int phy_addr; };
struct hv_ion_ayir { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
#define HV_ION_IOC_ALLOC _IOWR('I', 0, struct hv_ion_ayir)

#define HV_MODE 0x00
#define HV_PRI_CHROMA_LEN 0xc4
#define HV_PRI_STRIDE 0xc8
#define HV_CHROMA_LEN 0xe8
#define HV_PRI_FMT 0xec
#define HV_SPS 0x200
#define HV_PPS 0x204
#define HV_SHS 0x208
#define HV_SHS2 0x20c
#define HV_SHS_WP 0x210
#define HV_SHS_QP 0x21c
#define HV_CTRL 0x220
#define HV_TRIG 0x224
#define HV_STATUS 0x228
#define HV_CUR_MB 0x22c
#define HV_VLD_ADDR 0x230
#define HV_VLD_OFFSET 0x234
#define HV_VLD_LEN 0x238
#define HV_VLD_END 0x23c
#define HV_SDROT 0x240
#define HV_OUT_IDX 0x24c
#define HV_EXTRA1 0x250
#define HV_EXTRA2 0x254
#define HV_BASIC_BITS 0x2dc
#define HV_SRAM_OFS 0x2e0
#define HV_SRAM_DATA 0x2e4
#define HV_KONUM 6

struct hv_tampon { int fd; uint8_t *m; uint32_t adr; size_t n; };
struct hv_kare { uint32_t y, c; int aw, ah, w, h, konum, idr; double ms; uint8_t *ym; };
/* v2: cagiranin tamponlari (m: CPU eslemesi, adr: IOMMU, n: boy); fd < 0 ise "dis" sayilir ve birakilmaz */
struct hv_dis { uint8_t *bit_m; uint32_t bit_adr; size_t bit_n; uint8_t *k0_m; uint32_t k0_adr; size_t k0_n; };

static struct {
	int ve_fd, ion_fd, hazir, konum_n, aw, ah;
	uint32_t maske;
	volatile uint32_t *R;
	struct hv_tampon bit, pic, kom, kare[HV_KONUM], mv[HV_KONUM];
	struct hv_dis dis;	/* v2 */
	uint32_t kom_adr;
	size_t ysz, alan;
	struct { int var, konum, fn, poc; } D[HV_KONUM];
	int prev_fn, prev_poc_msb, prev_poc_lsb, fno, prev_fno, poc, konum;
	double kare_ms;
	unsigned uyari;
} HV;

struct hv_sps { int var, profil, chroma, log2_fn, poc_tur, log2_poc, ref_sayi, w_mb, h_mu, fmo, mbaff, d8x8, crop_r, crop_b; };
struct hv_pps { int var, entropi, bfpo, ref0, ref1, wp, wbi, qp, cqp, cqp2, dbk, cip, rpc, t8x8; };
static struct hv_sps HS;
static struct hv_pps HP;

static void hv_w(unsigned o, uint32_t v) { __sync_synchronize(); HV.R[o / 4] = v; }
static uint32_t hv_r(unsigned o) { uint32_t v = HV.R[o / 4]; __sync_synchronize(); return v; }
static double hv_ms(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }

static int hv_ayir(struct hv_tampon *t, size_t n)
{
	struct hv_ion_ayir a = { .len = n, .heap_id_mask = HV.maske, .flags = 0 };
	memset(t, 0, sizeof *t);
	t->fd = -1;
	if (ioctl(HV.ion_fd, HV_ION_IOC_ALLOC, &a)) return -1;
	t->fd = (int)a.fd;
	t->n = n;
	t->m = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, t->fd, 0);
	if (t->m == MAP_FAILED) { t->m = NULL; return -2; }
	struct hv_dma_param p = { .fd = t->fd };
	if (ioctl(HV.ve_fd, HV_VE_MAP_DMA_BUF, &p)) return -3;
	t->adr = p.phy_addr;
	memset(t->m, 0, n);
	return 0;
}

static void hv_birak(struct hv_tampon *t)
{
	if (t->fd == -2) { memset(t, 0, sizeof *t); t->fd = -1; return; }	/* v2: dis tampon, sahibi cagiran */
	if (t->adr) { struct hv_dma_param p = { .fd = t->fd }; ioctl(HV.ve_fd, HV_VE_UNMAP_DMA_BUF, &p); }
	if (t->m) munmap(t->m, t->n);
	if (t->fd > 0) close(t->fd);
	memset(t, 0, sizeof *t);
	t->fd = -1;
}

/* ---- yazilim RBSP okuyucu */
struct hv_bit { uint8_t *b; size_t n, p; int hata; };
static size_t hv_rbsp(const uint8_t *s, size_t n, uint8_t *o)
{
	size_t k = 0, z = 0;
	for (size_t i = 0; i < n; i++) {
		if (z >= 2 && s[i] == 3) { z = 0; continue; }
		z = s[i] ? 0 : z + 1;
		o[k++] = s[i];
	}
	return k;
}
static uint32_t hv_bu(struct hv_bit *b, int n)
{
	uint32_t v = 0;
	for (int i = 0; i < n; i++) {
		if (b->p >= b->n * 8) { b->hata = 1; return 0; }
		v = v << 1 | ((b->b[b->p >> 3] >> (7 - (b->p & 7))) & 1);
		b->p++;
	}
	return v;
}
static uint32_t hv_bue(struct hv_bit *b)
{
	int z = 0;
	while (!hv_bu(b, 1) && !b->hata && z < 32) z++;
	return z ? ((1u << z) - 1 + hv_bu(b, z)) : 0;
}
static int32_t hv_bse(struct hv_bit *b)
{
	uint32_t k = hv_bue(b);
	return k & 1 ? (int32_t)((k + 1) / 2) : -(int32_t)(k / 2);
}
static int hv_daha(struct hv_bit *b)
{
	if (b->p >= b->n * 8) return 0;
	size_t son = b->n * 8;
	while (son > 0 && !((b->b[(son - 1) >> 3] >> (7 - ((son - 1) & 7))) & 1)) son--;
	return b->p < son - 1;
}

static int hv_sps_coz(const uint8_t *n, size_t len)
{
	static uint8_t t[4096];
	if (len > sizeof t || len < 2) return -1;
	struct hv_bit b = { t, hv_rbsp(n + 1, len - 1, t), 0, 0 };
	struct hv_sps S;
	memset(&S, 0, sizeof S);
	S.profil = (int)hv_bu(&b, 8);
	hv_bu(&b, 16);
	hv_bue(&b);
	S.chroma = 1;
	int p = S.profil, bd = 0;
	if (p == 100 || p == 110 || p == 122 || p == 244 || p == 44 || p == 83 || p == 86 || p == 118 || p == 128 || p == 138) {
		S.chroma = (int)hv_bue(&b);
		if (S.chroma == 3) hv_bu(&b, 1);
		bd = (int)hv_bue(&b);	/* 614: luma/chroma bit derinligi - 8; tamponlar 8 bit */
		bd |= (int)hv_bue(&b);
		hv_bu(&b, 1);
		if (hv_bu(&b, 1)) return -2;	/* olcek matrisi */
	}
	uint32_t l2 = hv_bue(&b);	/* 614: standart 0..12 (hv_vu(44) ve 1 << 44 tanimsiz) */
	if (l2 > 12) return -4;
	S.log2_fn = (int)l2 + 4;
	S.poc_tur = (int)hv_bue(&b);
	if (S.poc_tur == 0) {
		if ((l2 = hv_bue(&b)) > 12) return -4;
		S.log2_poc = (int)l2 + 4;
	}
	else if (S.poc_tur == 1) return -3;
	S.ref_sayi = (int)hv_bue(&b);
	hv_bu(&b, 1);
	S.w_mb = (int)hv_bue(&b) + 1;
	S.h_mu = (int)hv_bue(&b) + 1;
	S.fmo = (int)hv_bu(&b, 1);
	if (!S.fmo) S.mbaff = (int)hv_bu(&b, 1);
	S.d8x8 = (int)hv_bu(&b, 1);
	if (hv_bu(&b, 1)) {	/* kirpma (4:2:0: 2 piksel birimi) */
		hv_bue(&b);
		S.crop_r = (int)hv_bue(&b) * 2;
		hv_bue(&b);
		S.crop_b = (int)hv_bue(&b) * 2;
	}
	if (b.hata || !S.fmo || S.w_mb != HV_W_MB || S.h_mu != HV_H_MU) return -4;	/* v3 */
	if (S.chroma != 1 || bd || S.ref_sayi > 2) return -4;	/* 614: 4:2:0 8 bit; PC akisi ref 2 (3 konum, cihazda) */
	S.var = 1;
	HS = S;
	return 0;
}

static int hv_pps_coz(const uint8_t *n, size_t len)
{
	static uint8_t t[4096];
	if (len > sizeof t || len < 2) return -1;
	struct hv_bit b = { t, hv_rbsp(n + 1, len - 1, t), 0, 0 };
	struct hv_pps P;
	memset(&P, 0, sizeof P);
	hv_bue(&b);
	hv_bue(&b);
	P.entropi = (int)hv_bu(&b, 1);
	P.bfpo = (int)hv_bu(&b, 1);
	if (hv_bue(&b)) return -2;
	uint32_t r0 = hv_bue(&b), r1 = hv_bue(&b);
	if (r0 >= HV_REF_EN || r1 >= HV_REF_EN) return -5;	/* v3 */
	P.ref0 = (int)r0 + 1;
	P.ref1 = (int)r1 + 1;
	P.wp = (int)hv_bu(&b, 1);
	P.wbi = (int)hv_bu(&b, 2);
	P.qp = hv_bse(&b) + 26;
	hv_bse(&b);
	P.cqp = hv_bse(&b);
	P.dbk = (int)hv_bu(&b, 1);
	P.cip = (int)hv_bu(&b, 1);
	P.rpc = (int)hv_bu(&b, 1);
	P.cqp2 = P.cqp;
	if (hv_daha(&b)) {
		P.t8x8 = (int)hv_bu(&b, 1);
		if (hv_bu(&b, 1)) return -3;
		P.cqp2 = hv_bse(&b);
	}
	if (b.hata || P.wp || P.wbi) return -4;
	P.var = 1;
	HP = P;
	return 0;
}

/* ---- dilim basligi: VE bit okuyucusu (yazilim karsilastirmasi prova v1-v4'te yapildi) */
struct hv_baslik { int nal_ref, idr, ilk_mb, tur, fn, poc_lsb, ref0, cabac_idc, qp_d, dbk_idc, alfa, beta, mod, mmco; };
static int hv_okuma_hatasi;
static int hv_bekle(void)
{
	for (int i = 0; i < 200000; i++)
		if (!(hv_r(HV_STATUS) & (1u << 8))) return 0;
	hv_okuma_hatasi = 1;
	return -1;
}
static uint32_t hv_vu(int n) { if (!n) return 0; hv_w(HV_TRIG, 2u | (uint32_t)n << 8); hv_bekle(); return hv_r(HV_BASIC_BITS); }
static uint32_t hv_vue(void) { hv_w(HV_TRIG, 5); hv_bekle(); return hv_r(HV_BASIC_BITS); }
static int32_t hv_vse(void) { uint32_t k = hv_vue(); return k & 1 ? (int32_t)((k + 1) / 2) : -(int32_t)(k / 2); }

static int hv_baslik_oku(struct hv_baslik *h)
{
	memset(h, 0, sizeof *h);
	hv_okuma_hatasi = 0;
	uint32_t nh = hv_vu(8);
	h->nal_ref = (int)(nh >> 5) & 3;
	h->idr = (nh & 31) == 5;
	h->ilk_mb = (int)hv_vue();
	if ((uint32_t)h->ilk_mb >= (uint32_t)(HS.w_mb * HS.h_mu)) return -16;	/* 614: VE luma baslangici tampon disina */
	h->tur = (int)(hv_vue() % 5);
	hv_vue();
	h->fn = (int)hv_vu(HS.log2_fn);
	if (h->idr) hv_vue();
	if (HS.poc_tur == 0) {
		h->poc_lsb = (int)hv_vu(HS.log2_poc);
		if (HP.bfpo) hv_vse();
	}
	if (HP.rpc) hv_vue();
	h->ref0 = HP.ref0;
	if (h->tur == 0) {
		if (hv_vu(1)) {
			uint32_t r0 = hv_vue();
			if (r0 >= HV_REF_EN) return -15;	/* v3 */
			h->ref0 = (int)r0 + 1;
		}
		if (hv_vu(1)) {
			h->mod = 1;
			for (int k = 0; k < 33; k++) { if (hv_vue() == 3) break; hv_vue(); }
		}
	}
	if (h->nal_ref) {
		if (h->idr) { hv_vu(1); hv_vu(1); }
		else if (hv_vu(1)) {
			h->mmco = 1;
			for (int k = 0; k < 66; k++) {
				uint32_t mm = hv_vue();
				if (!mm) break;
				if (mm == 1 || mm == 3) hv_vue();
				if (mm == 2) hv_vue();
				if (mm == 3 || mm == 6) hv_vue();
				if (mm == 4) hv_vue();
			}
		}
	}
	if (HP.entropi && h->tur != 2) h->cabac_idc = (int)hv_vue();
	h->qp_d = hv_vse();
	if (HP.dbk) {
		h->dbk_idc = (int)hv_vue();
		if (h->dbk_idc != 1) { h->alfa = hv_vse(); h->beta = hv_vse(); }
	}
	if (h->tur != 0 && h->tur != 2) return -5;	/* B/SP/SI yok */
	return hv_okuma_hatasi ? -6 : 0;
}

static void hv_sram(unsigned ofs, const void *v, size_t n)
{
	const uint32_t *p = v;
	hv_w(HV_SRAM_OFS, ofs << 2);
	for (size_t i = 0; i < (n + 3) / 4; i++) hv_w(HV_SRAM_DATA, p[i]);
}
struct hv_ref_pic { uint32_t top_poc, bot_poc, info, luma, chroma, mv_top, mv_bot, ayrik; };

static int hv_ilk_mb(const uint8_t *n, size_t len)
{
	uint8_t t[16];
	if (len < 2) return -1;
	struct hv_bit b = { t, hv_rbsp(n + 1, len - 1 < 15 ? len - 1 : 15, t), 0, 0 };
	return (int)hv_bue(&b);
}

static int hv_tamponlar(void)
{
	int aw = HS.w_mb * 16, ah = HS.h_mu * 16;
	int n = HS.ref_sayi + 1 < 2 ? 2 : HS.ref_sayi + 1 > HV_KONUM ? HV_KONUM : HS.ref_sayi + 1;
	if (aw == HV.aw && ah == HV.ah && n <= HV.konum_n) return 0;
	for (int i = HV_KONUM - 1; i >= 0; i--) { hv_birak(&HV.kare[i]); hv_birak(&HV.mv[i]); }	/* paylasilanlar once */
	HV.aw = aw;
	HV.ah = ah;
	HV.ysz = (size_t)aw * ah;
	HV.alan = (size_t)HS.w_mb * HS.h_mu * 16 * 2 * 2;
	HV.konum_n = n;
	memset(HV.D, 0, sizeof HV.D);
	size_t kn = HV.ysz + HV.ysz / 2, ek = 0;
	for (int i = 0; i < n; i++) {
		if (i == 0 && HV.dis.k0_m && HV.dis.k0_n >= kn) {	/* v2: konum 0 = cagiranin tamponu */
			HV.kare[0] = (struct hv_tampon){ -2, HV.dis.k0_m, HV.dis.k0_adr, HV.dis.k0_n };
			continue;
		}
		if (hv_ayir(&HV.kare[i], kn)) { HV.konum_n = 0; return -1; }
		ek += kn;
	}
	if (hv_ayir(&HV.mv[0], HV.alan * 2)) { HV.konum_n = 0; return -1; }	/* v2: tek mv_col (B yok) */
	for (int i = 1; i < n; i++) HV.mv[i] = (struct hv_tampon){ -2, HV.mv[0].m, HV.mv[0].adr, HV.mv[0].n };
	ek += HV.alan * 2;
	printf("H.264 VE: %dx%d, %d konum, ek bellek %zu KB (konum 0 %s)\n", aw, ah, n, ek / 1024, HV.kare[0].fd == -2 ? "dis" : "ayri");
	return 0;
}

/* tek dilim; son_dilim: kare tamamlanir. 0 tamam, <0 hata */
static int hv_dilim(const uint8_t *n, size_t len, int son_dilim, struct hv_kare *k)
{
	if (len + 64 > HV.bit.n) return -7;
	memcpy(HV.bit.m, n, len);
	memset(HV.bit.m + len, 0, 64);
	__sync_synchronize();
	hv_w(HV_MODE, 0x00130001);
	hv_w(HV_VLD_LEN, (uint32_t)(len * 8));
	hv_w(HV_VLD_OFFSET, 0);
	hv_w(HV_VLD_END, HV.bit.adr + (uint32_t)len + 64);
	hv_w(HV_VLD_ADDR, (HV.bit.adr & 0x0ffffff0u) | (HV.bit.adr >> 28) | (1u << 28) | (1u << 29) | (1u << 30));
	hv_w(HV_TRIG, 7);
	struct hv_baslik h;
	int e = hv_baslik_oku(&h);
	if (e) { hv_w(HV_MODE, 7); return e; }
	if ((h.mod || h.mmco) && HV.uyari++ < 5) puts("H.264 VE: UYARI ref listesi degisikligi/MMCO uygulanmiyor");
	if (h.ilk_mb == 0) {
		if (h.idr) { HV.prev_poc_msb = HV.prev_poc_lsb = 0; HV.fno = HV.prev_fno = HV.prev_fn = 0; memset(HV.D, 0, sizeof HV.D); }
		if (HS.poc_tur == 0) {
			int maxl = 1 << HS.log2_poc, msb = HV.prev_poc_msb;
			if (h.poc_lsb < HV.prev_poc_lsb && HV.prev_poc_lsb - h.poc_lsb >= maxl / 2) msb += maxl;
			else if (h.poc_lsb > HV.prev_poc_lsb && h.poc_lsb - HV.prev_poc_lsb > maxl / 2) msb -= maxl;
			HV.poc = msb + h.poc_lsb;
			if (h.nal_ref) { HV.prev_poc_msb = msb; HV.prev_poc_lsb = h.poc_lsb; }
		} else {
			if (!h.idr) HV.fno = HV.prev_fn > h.fn ? HV.prev_fno + (1 << HS.log2_fn) : HV.prev_fno;
			HV.poc = 2 * (HV.fno + h.fn) - (h.nal_ref ? 0 : 1);
			HV.prev_fno = HV.fno;
			HV.prev_fn = h.fn;
		}
		for (HV.konum = 0; HV.konum < HV.konum_n; HV.konum++) {
			int dolu = 0;
			for (int i = 0; i < HV_KONUM; i++) dolu |= HV.D[i].var && HV.D[i].konum == HV.konum;
			if (!dolu) break;
		}
		if (HV.konum >= HV.konum_n) { hv_w(HV_MODE, 7); return -8; }
		HV.kare_ms = 0;
	}
	int konum = HV.konum, aw = HV.aw;
	size_t ysz = HV.ysz, alan = HV.alan;
	hv_w(HV_PRI_FMT, 4u << 4);
	hv_w(HV_CHROMA_LEN, 0);
	hv_w(HV_PRI_CHROMA_LEN, (uint32_t)(ysz / 2 / 2));
	hv_w(HV_PRI_STRIDE, (uint32_t)aw | ((uint32_t)aw / 2) << 16);
	hv_w(HV_SDROT, 0);
	hv_w(HV_EXTRA1, HV.pic.adr);
	hv_w(HV_EXTRA2, HV.kom_adr);
	static uint8_t duz[96];
	memset(duz, 16, sizeof duz);
	hv_sram(0x200, duz, 64);
	hv_sram(0x210, duz, 64);
	hv_sram(0x220, duz, 96);
	struct hv_ref_pic pl[18];
	memset(pl, 0, sizeof pl);
	for (int i = 0; i < HV_KONUM; i++) {
		if (!HV.D[i].var) continue;
		int k2 = HV.D[i].konum;
		struct hv_ref_pic *e2 = &pl[k2];
		e2->top_poc = e2->bot_poc = (uint32_t)HV.D[i].poc;
		e2->luma = HV.kare[k2].adr;
		e2->chroma = HV.kare[k2].adr + (uint32_t)ysz;
		e2->mv_top = HV.mv[k2].adr;
		e2->mv_bot = HV.mv[k2].adr + (uint32_t)alan;
	}
	struct hv_ref_pic *c = &pl[konum];
	c->top_poc = c->bot_poc = (uint32_t)HV.poc;
	c->luma = HV.kare[konum].adr;
	c->chroma = HV.kare[konum].adr + (uint32_t)ysz;
	c->mv_top = HV.mv[konum].adr;
	c->mv_bot = HV.mv[konum].adr + (uint32_t)alan;
	hv_sram(0x100, pl, sizeof pl);
	hv_w(HV_OUT_IDX, (uint32_t)konum);
	if (h.tur == 0) {
		int sira[HV_KONUM], m = 0, maxfn = 1 << HS.log2_fn;
		for (int i = 0; i < HV_KONUM; i++) if (HV.D[i].var) sira[m++] = i;
		for (int a = 0; a < m; a++)
			for (int b2 = a + 1; b2 < m; b2++) {
				int wa = HV.D[sira[a]].fn > h.fn ? HV.D[sira[a]].fn - maxfn : HV.D[sira[a]].fn;
				int wb = HV.D[sira[b2]].fn > h.fn ? HV.D[sira[b2]].fn - maxfn : HV.D[sira[b2]].fn;
				if (wb > wa) { int t2 = sira[a]; sira[a] = sira[b2]; sira[b2] = t2; }
			}
		if (!m) { hv_w(HV_MODE, 7); return -9; }
		uint8_t rl[32];
		memset(rl, 0, sizeof rl);
		for (int i = 0; i < h.ref0 && i < 32; i++) rl[i] = (uint8_t)(HV.D[sira[i < m ? i : m - 1]].konum << 1);
		hv_sram(0x190, rl, (size_t)((h.ref0 + 3) & ~3));
	}
	uint32_t reg = ((uint32_t)(h.ref0 - 1) & 0x1f) << 10 | ((uint32_t)(HP.ref1 - 1) & 0x1f) << 5 | ((uint32_t)HP.wbi & 3) << 2;
	if (HP.entropi) reg |= 1u << 15;
	if (HP.cip) reg |= 1u << 1;
	if (HP.t8x8) reg |= 1u << 0;
	hv_w(HV_PPS, reg);
	reg = ((uint32_t)HS.chroma & 7) << 19 | ((uint32_t)(HS.w_mb - 1) & 0xff) << 8 | ((uint32_t)(HS.h_mu - 1) & 0xff);
	if (HS.fmo) reg |= 1u << 18;
	if (HS.mbaff) reg |= 1u << 17;
	if (HS.d8x8) reg |= 1u << 16;
	hv_w(HV_SPS, reg);
	reg = (h.nal_ref ? 1u << 12 : 0) | ((uint32_t)h.tur & 0xf) << 8 | ((uint32_t)h.cabac_idc & 3) |
	      ((uint32_t)(h.ilk_mb % HS.w_mb) & 0xff) << 24 | ((uint32_t)(h.ilk_mb / HS.w_mb) & 0xff) << 16 | (h.ilk_mb == 0 ? 1u << 5 : 0);
	hv_w(HV_SHS, reg);
	reg = 1u << 12 | ((uint32_t)(h.ref0 - 1) & 0x1f) << 24 | ((uint32_t)(HP.ref1 - 1) & 0x1f) << 16 |
	      ((uint32_t)h.dbk_idc & 3) << 8 | ((uint32_t)h.alfa & 0xf) << 4 | ((uint32_t)h.beta & 0xf);
	hv_w(HV_SHS2, reg);
	hv_w(HV_SHS_WP, 0);
	reg = ((uint32_t)HP.cqp2 & 0x3f) << 16 | ((uint32_t)HP.cqp & 0x3f) << 8 | ((uint32_t)(HP.qp + h.qp_d) & 0x3f);
	hv_w(HV_SHS_QP, reg);
	hv_w(HV_STATUS, hv_r(HV_STATUS));
	hv_w(HV_CTRL, 0);
	double t0 = hv_ms();
	hv_w(HV_TRIG, 8);
	uint32_t st = 0;
	while (hv_ms() - t0 < 500 && !((st = hv_r(HV_STATUS)) & 7))
		;
	HV.kare_ms += hv_ms() - t0;
	hv_w(HV_STATUS, 7);
	hv_w(HV_MODE, 7);
	if (!(st & 7)) return -2;	/* zaman asimi: cagiran VE_RESET */
	if (!(st & 1) || (st & 6)) return -10;
	if (!son_dilim) return 1;
	if (h.nal_ref) {	/* kayan pencere */
		int n_ref = 0, en_kucuk = -1, maxfn = 1 << HS.log2_fn, ek = 1 << 30;
		for (int i = 0; i < HV_KONUM; i++) {
			if (!HV.D[i].var) continue;
			n_ref++;
			int wr = HV.D[i].fn > h.fn ? HV.D[i].fn - maxfn : HV.D[i].fn;
			if (wr < ek) { ek = wr; en_kucuk = i; }
		}
		int sinir = HS.ref_sayi < 1 ? 1 : HS.ref_sayi;
		if (n_ref >= sinir && en_kucuk >= 0) HV.D[en_kucuk].var = 0;
		for (int i = 0; i < HV_KONUM; i++)
			if (!HV.D[i].var) { HV.D[i].var = 1; HV.D[i].konum = konum; HV.D[i].fn = h.fn; HV.D[i].poc = HV.poc; break; }
	}
	k->y = HV.kare[konum].adr;
	k->c = HV.kare[konum].adr + (uint32_t)ysz;
	k->ym = HV.kare[konum].m;
	k->aw = aw;
	k->ah = HV.ah;
	k->w = aw - HS.crop_r;
	k->h = HV.ah - HS.crop_b;
	k->konum = konum;
	k->idr = h.idr;
	k->ms = HV.kare_ms;
	return 0;
}

/* bir erisim birimi (Annex-B; bir ya da daha cok NAL). 0: kare (son tamamlanan), 1: kare yok, <0 hata */
static int hv_au(const uint8_t *v, size_t boy, struct hv_kare *k)
{
	if (!HV.hazir) return -1;
	size_t nb[512], ns[512];
	int nn = 0;
	for (size_t i = 0; i + 3 < boy && nn < 512; i++)
		if (v[i] == 0 && v[i + 1] == 0 && v[i + 2] == 1) {
			if (nn) { size_t s = i; while (s > nb[nn - 1] && v[s - 1] == 0) s--; ns[nn - 1] = s; }
			nb[nn++] = i + 3;
			i += 2;
		}
	if (nn) ns[nn - 1] = boy;
	int sonuc = 1;
	for (int ni = 0; ni < nn; ni++) {
		const uint8_t *n = v + nb[ni];
		size_t len = ns[ni] - nb[ni];
		int tur = n[0] & 31;
		if (tur == 7) { if (hv_sps_coz(n, len)) return -11; continue; }
		if (tur == 8) { if (hv_pps_coz(n, len)) return -12; continue; }
		if (tur != 1 && tur != 5) continue;
		if (!HS.var || !HP.var) return -13;
		if (hv_tamponlar()) return -14;
		int son = 1;
		for (int nj = ni + 1; nj < nn; nj++) {
			int tj = v[nb[nj]] & 31;
			if (tj == 1 || tj == 5) { son = hv_ilk_mb(v + nb[nj], ns[nj] - nb[nj]) == 0; break; }
		}
		int e = hv_dilim(n, len, son, k);
		if (e < 0) return e;
		if (e == 0) sonuc = 0;
	}
	return sonuc;
}

static int hv_baslat(int ve_fd, int ion_fd, uint32_t maske, volatile uint32_t *R, const struct hv_dis *dis)
{
	memset(&HV, 0, sizeof HV);
	for (int i = 0; i < HV_KONUM; i++) HV.kare[i].fd = HV.mv[i].fd = -1;
	HV.ve_fd = ve_fd;
	HV.ion_fd = ion_fd;
	HV.maske = maske;
	HV.R = R;
	if (dis) HV.dis = *dis;
	if (HV.dis.bit_m && HV.dis.bit_n >= 64 * 1024)	/* v2: bit akisi cagirandan */
		HV.bit = (struct hv_tampon){ -2, HV.dis.bit_m, HV.dis.bit_adr, HV.dis.bit_n };
	else if (hv_ayir(&HV.bit, 1 << 20)) return -1;
	if (hv_ayir(&HV.pic, 128 * 1024) || hv_ayir(&HV.kom, 32 * 1024)) return -1;
	HV.kom_adr = (HV.kom.adr + 0x3fff) & ~0x3fffu;
	HV.hazir = 1;
	return 0;
}

static void hv_bitir(void)
{
	for (int i = HV_KONUM - 1; i >= 0; i--) { hv_birak(&HV.kare[i]); hv_birak(&HV.mv[i]); }
	hv_birak(&HV.bit);
	hv_birak(&HV.pic);
	hv_birak(&HV.kom);
	HV.hazir = 0;
}

#endif
