/* h264_ve_prova_v4 (15.10.2026, Claude): v3 + cok referans DPB (kayan pencere, P listesi FrameNumWrap azalan; scripts/254).
 * h264_ve_prova_v3 (15.10.2026, Claude): v2 + COK DILIM: SHS bit 24/16 ilk MB x/y (mainline cedrus), FIRST_SLICE yalniz ilk
 *   dilimde, kare (konum/POC/kontrol/ref) son dilimde biter.
 * h264_ve_prova_v2 (15.10.2026, Claude): v1 + se okumasi VE ue uzerinden (v1: tetik 6 deger okumadi, qp_d 0 / dbk 6).
 * h264_ve_prova_v1 (15.10.2026, Claude): F133 VE (0x1667) ile DONANIM H.264 cozme provasi, kullanici alani.
 * Kaynaklar: Tina 5.4 drivers/staging/media/sunxi/cedrus/cedrus_h264.c + cedrus_regs.h (yazmac/SRAM duzeni),
 * libvdpau-sunxi yontemi (dilim basligi VE'nin bit okuyucusuyla: EPB'yi VE ayiklar), wifi_ekran_v17 (bu cipte
 * kanitli VE kurulumu: ENGINE_REQ -> MAP_DMA_BUF, VE_MODE 0x0013000x, dogrusal NV12 cikis, durum yoklama, VE_RESET).
 * Kapsam (v1): tek dilim/kare, CAVLC ya da CABAC, I/P (B yok), frame_mbs_only, olcek matrisi yok, agirlikli tahmin yok,
 * tek referans (son referans kare). Cikis NV12 dogrusal, 3 konum (0..2) donusumlu.
 * Kullanim:
 *   h264-ve-prova <akis.h264> [beklenen.kontrol] [--yalniz-baslik] [--en-cok N] [--dok /tmp/ve_y.bin]
 *   --yalniz-baslik : VE yalniz basligi okur (cozme TETIKLENMEZ) ve yazilimla karsilastirilir (ilk guvenli adim).
 * Kontrol: k = sum_y (y+1) * sum_x Y[y][x] mod 2^32 (work/h264_ve_akis_hazirla_v1.py ile ayni). */
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/* ---------------------------------------------------------------- VE / ION */
#define VE_RESET 0x104
#define VE_ENGINE_REQ 0x206
#define VE_ENGINE_REL 0x207
#define VE_MAP_DMA_BUF 0x504
#define VE_UNMAP_DMA_BUF 0x505
struct dma_buf_param { int fd; unsigned int phy_addr; };
struct ion_ayir { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
struct ion_yigin { char name[32]; uint32_t type, heap_id, r0, r1, r2; };
struct ion_sorgu { uint32_t cnt, r0; uint64_t heaps; uint32_t r1, r2; };
#define ION_IOC_ALLOC _IOWR('I', 0, struct ion_ayir)
#define ION_IOC_HEAP_QUERY _IOWR('I', 8, struct ion_sorgu)

#define VE_MODE 0x00
#define VE_PRIMARY_CHROMA_BUF_LEN 0xc4
#define VE_PRIMARY_FB_LINE_STRIDE 0xc8
#define VE_CHROMA_BUF_LEN 0xe8
#define VE_PRIMARY_OUT_FMT 0xec
#define H_SPS 0x200
#define H_PPS 0x204
#define H_SHS 0x208
#define H_SHS2 0x20c
#define H_SHS_WP 0x210
#define H_SHS_QP 0x21c
#define H_CTRL 0x220
#define H_TRIG 0x224
#define H_STATUS 0x228
#define H_CUR_MB 0x22c
#define H_VLD_ADDR 0x230
#define H_VLD_OFFSET 0x234
#define H_VLD_LEN 0x238
#define H_VLD_END 0x23c
#define H_SDROT 0x240
#define H_OUT_IDX 0x24c
#define H_EXTRA1 0x250
#define H_EXTRA2 0x254
#define H_BASIC_BITS 0x2dc
#define H_SRAM_OFS 0x2e0
#define H_SRAM_DATA 0x2e4
#define SRAM_FRAMELIST 0x100
#define SRAM_REFLIST0 0x190
#define SRAM_SCALE8_0 0x200
#define SRAM_SCALE8_1 0x210
#define SRAM_SCALE4 0x220

static int ve_fd = -1, ion_fd = -1, ve_req;
static volatile uint32_t *R;
static void w(unsigned o, uint32_t v) { __sync_synchronize(); R[o / 4] = v; }
static uint32_t r(unsigned o) { uint32_t v = R[o / 4]; __sync_synchronize(); return v; }

struct tampon { int fd; uint8_t *m; uint32_t adr; size_t n; };
static uint32_t ion_maske;

static double ms(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

static int ayir(struct tampon *t, size_t n)
{
	struct ion_ayir a = { .len = n, .heap_id_mask = ion_maske, .flags = 0 /* onbelleksiz: CPU yalniz kontrol okur */ };
	memset(t, 0, sizeof *t);
	t->fd = -1;
	if (ioctl(ion_fd, ION_IOC_ALLOC, &a)) return -1;
	t->fd = (int)a.fd;
	t->n = n;
	t->m = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, t->fd, 0);
	if (t->m == MAP_FAILED) { t->m = NULL; return -2; }
	struct dma_buf_param p = { .fd = t->fd };
	if (ioctl(ve_fd, VE_MAP_DMA_BUF, &p)) return -3;
	t->adr = p.phy_addr;
	memset(t->m, 0, n);
	return 0;
}

static void birak(struct tampon *t)
{
	if (t->adr) { struct dma_buf_param p = { .fd = t->fd }; ioctl(ve_fd, VE_UNMAP_DMA_BUF, &p); }
	if (t->m) munmap(t->m, t->n);
	if (t->fd >= 0) close(t->fd);
	memset(t, 0, sizeof *t);
	t->fd = -1;
}

/* ---------------------------------------------------------------- yazilim bit okuyucu (RBSP) */
struct bit { uint8_t *b; size_t n, p; int hata; };

static size_t rbsp(const uint8_t *s, size_t n, uint8_t *o)	/* EPB (00 00 03) ayikla */
{
	size_t k = 0, z = 0;
	for (size_t i = 0; i < n; i++) {
		if (z >= 2 && s[i] == 3) { z = 0; continue; }
		z = s[i] ? 0 : z + 1;
		o[k++] = s[i];
	}
	return k;
}
static uint32_t bu(struct bit *b, int n)
{
	uint32_t v = 0;
	for (int i = 0; i < n; i++) {
		if (b->p >= b->n * 8) { b->hata = 1; return 0; }
		v = v << 1 | ((b->b[b->p >> 3] >> (7 - (b->p & 7))) & 1);
		b->p++;
	}
	return v;
}
static uint32_t bue(struct bit *b)
{
	int z = 0;
	while (!bu(b, 1) && !b->hata && z < 32) z++;
	return z ? ((1u << z) - 1 + bu(b, z)) : 0;
}
static int32_t bse(struct bit *b)
{
	uint32_t k = bue(b);
	return k & 1 ? (int32_t)((k + 1) / 2) : -(int32_t)(k / 2);
}
static int daha_rbsp(struct bit *b)
{
	if (b->p >= b->n * 8) return 0;
	size_t son = b->n * 8;	/* son 1 biti (rbsp_stop) bul */
	while (son > 0 && !((b->b[(son - 1) >> 3] >> (7 - ((son - 1) & 7))) & 1)) son--;
	return b->p < son - 1;
}

/* ---------------------------------------------------------------- SPS / PPS */
struct sps {
	int var, profil, chroma, log2_fn, poc_tur, log2_poc, ref_sayi, w_mb, h_mu, fmo, mbaff, d8x8;
};
struct pps {
	int var, sps, entropi, bfpo, ref0, ref1, wp, wbi, qp, qs, cqp, cqp2, dbk, cip, rpc, t8x8;
};
static struct sps S;
static struct pps P;

static int sps_coz(const uint8_t *n, size_t len)
{
	static uint8_t t[4096];
	if (len > sizeof t) return -1;
	struct bit b = { t, rbsp(n + 1, len - 1, t), 0, 0 };
	memset(&S, 0, sizeof S);
	S.profil = (int)bu(&b, 8);
	bu(&b, 16);
	bue(&b);	/* sps id */
	S.chroma = 1;
	int p = S.profil;
	if (p == 100 || p == 110 || p == 122 || p == 244 || p == 44 || p == 83 || p == 86 || p == 118 || p == 128 || p == 138) {
		S.chroma = (int)bue(&b);
		if (S.chroma == 3) bu(&b, 1);
		bue(&b);
		bue(&b);
		bu(&b, 1);
		if (bu(&b, 1)) { puts("SPS: olcek matrisi var (v1 desteklemez)"); return -1; }
	}
	S.log2_fn = (int)bue(&b) + 4;
	S.poc_tur = (int)bue(&b);
	if (S.poc_tur == 0) S.log2_poc = (int)bue(&b) + 4;
	else if (S.poc_tur == 1) { puts("SPS: POC turu 1 desteklenmez"); return -1; }
	S.ref_sayi = (int)bue(&b);
	bu(&b, 1);
	S.w_mb = (int)bue(&b) + 1;
	S.h_mu = (int)bue(&b) + 1;
	S.fmo = (int)bu(&b, 1);
	if (!S.fmo) S.mbaff = (int)bu(&b, 1);
	S.d8x8 = (int)bu(&b, 1);
	S.var = !b.hata && S.fmo;
	printf("SPS: profil %d chroma %d %dx%d MB, log2_fn %d poc %d (log2 %d) ref %d fmo %d d8x8 %d\n", S.profil, S.chroma,
	       S.w_mb, S.h_mu, S.log2_fn, S.poc_tur, S.log2_poc, S.ref_sayi, S.fmo, S.d8x8);
	return S.var ? 0 : -1;
}

static int pps_coz(const uint8_t *n, size_t len)
{
	static uint8_t t[4096];
	if (len > sizeof t) return -1;
	struct bit b = { t, rbsp(n + 1, len - 1, t), 0, 0 };
	memset(&P, 0, sizeof P);
	bue(&b);
	P.sps = (int)bue(&b);
	P.entropi = (int)bu(&b, 1);
	P.bfpo = (int)bu(&b, 1);
	if (bue(&b)) { puts("PPS: dilim gruplari desteklenmez"); return -1; }
	P.ref0 = (int)bue(&b) + 1;
	P.ref1 = (int)bue(&b) + 1;
	P.wp = (int)bu(&b, 1);
	P.wbi = (int)bu(&b, 2);
	P.qp = bse(&b) + 26;
	P.qs = bse(&b) + 26;
	P.cqp = bse(&b);
	P.dbk = (int)bu(&b, 1);
	P.cip = (int)bu(&b, 1);
	P.rpc = (int)bu(&b, 1);
	P.cqp2 = P.cqp;
	if (daha_rbsp(&b)) {
		P.t8x8 = (int)bu(&b, 1);
		if (bu(&b, 1)) { puts("PPS: olcek matrisi desteklenmez"); return -1; }
		P.cqp2 = bse(&b);
	}
	P.var = !b.hata;
	printf("PPS: entropi %s ref0 %d wp %d qp %d cqp %d/%d dbk %d cip %d t8x8 %d\n", P.entropi ? "CABAC" : "CAVLC", P.ref0,
	       P.wp, P.qp, P.cqp, P.cqp2, P.dbk, P.cip, P.t8x8);
	if (P.wp || P.wbi) { puts("PPS: agirlikli tahmin desteklenmez"); return -1; }
	return P.var ? 0 : -1;
}

/* ---------------------------------------------------------------- dilim basligi */
struct baslik {
	int nal_ref, nal_tur, ilk_mb, tur, pps, fn, idr_id, poc_lsb, dpoc_b, ovrd, ref0, cabac_idc, qp_d, dbk_idc, alfa, beta;
	int idr, uzunluk_bit, mod, mmco;
};

/* okuyucu soyutlamasi: yazilim ya da VE */
struct okuyucu { int ve; struct bit *b; };
static int ve_bekle(void)
{
	for (int i = 0; i < 200000; i++)
		if (!(r(H_STATUS) & (1u << 8))) return 0;
	return -1;
}
static int ve_okuma_hatasi;
static uint32_t ou(struct okuyucu *o, int n)
{
	if (!o->ve) return bu(o->b, n);
	if (!n) return 0;
	w(H_TRIG, 2u | (uint32_t)n << 8);
	if (ve_bekle()) ve_okuma_hatasi = 1;
	return r(H_BASIC_BITS);
}
static uint32_t oue(struct okuyucu *o)
{
	if (!o->ve) return bue(o->b);
	w(H_TRIG, 5);
	if (ve_bekle()) ve_okuma_hatasi = 1;
	return r(H_BASIC_BITS);
}
static int32_t ose(struct okuyucu *o)
{
	if (!o->ve) return bse(o->b);
	uint32_t k = oue(o);	/* v2: tetik 6 bu VE'de se degil (v1 olcumu); se = ue esleme */
	return k & 1 ? (int32_t)((k + 1) / 2) : -(int32_t)(k / 2);
}

static int baslik_oku(struct okuyucu *o, struct baslik *h)
{
	memset(h, 0, sizeof *h);
	uint32_t nh = ou(o, 8);
	h->nal_ref = (int)(nh >> 5) & 3;
	h->nal_tur = (int)nh & 31;
	h->idr = h->nal_tur == 5;
	h->ilk_mb = (int)oue(o);
	h->tur = (int)(oue(o) % 5);
	h->pps = (int)oue(o);
	h->fn = (int)ou(o, S.log2_fn);
	if (h->idr) h->idr_id = (int)oue(o);
	if (S.poc_tur == 0) {
		h->poc_lsb = (int)ou(o, S.log2_poc);
		if (P.bfpo) h->dpoc_b = ose(o);
	}
	if (P.rpc) oue(o);
	h->ref0 = P.ref0;
	if (h->tur == 0) {	/* P */
		h->ovrd = (int)ou(o, 1);
		if (h->ovrd) h->ref0 = (int)oue(o) + 1;
		if (ou(o, 1)) {	/* ref_pic_list_modification_flag_l0 */
			h->mod = 1;
			for (int k = 0; k < 33; k++) {
				uint32_t idc = oue(o);
				if (idc == 3) break;
				oue(o);
			}
		}
	}
	if (h->nal_ref) {
		if (h->idr) { ou(o, 1); ou(o, 1); }
		else if (ou(o, 1)) {
			h->mmco = 1;
			for (int k = 0; k < 66; k++) {
				uint32_t mm = oue(o);
				if (!mm) break;
				if (mm == 1 || mm == 3) oue(o);
				if (mm == 2) oue(o);
				if (mm == 3 || mm == 6) oue(o);
				if (mm == 4) oue(o);
			}
		}
	}
	if (P.entropi && h->tur != 2) h->cabac_idc = (int)oue(o);
	h->qp_d = ose(o);
	if (P.dbk) {
		h->dbk_idc = (int)oue(o);
		if (h->dbk_idc != 1) { h->alfa = ose(o); h->beta = ose(o); }
	}
	if (!o->ve) h->uzunluk_bit = (int)o->b->p;
	return o->ve ? ve_okuma_hatasi : o->b->hata;
}

/* NAL'in ilk_mb degeri (yazilim, yalniz ilk ue) */
static int ilk_mb_oku(const uint8_t *n, size_t len)
{
	uint8_t t[16];
	struct bit b = { t, rbsp(n + 1, len - 1 < 15 ? len - 1 : 15, t), 0, 0 };
	return (int)bue(&b);
}

/* ---------------------------------------------------------------- SRAM */
static void sram(unsigned ofs, const void *v, size_t n)
{
	const uint32_t *p = v;
	w(H_SRAM_OFS, ofs << 2);
	for (size_t i = 0; i < (n + 3) / 4; i++) w(H_SRAM_DATA, p[i]);
}

struct ref_pic { uint32_t top_poc, bot_poc, info, luma, chroma, mv_top, mv_bot, ayrik; };

/* ---------------------------------------------------------------- ana */
#define KONUM 6	/* v4: en cok; kullanilan = num_ref_frames + 1 */
int main(int argc, char **argv)
{
	const char *akis = NULL, *bek = NULL, *dok = NULL;
	int yalniz_baslik = 0, en_cok = 1 << 30;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--yalniz-baslik")) yalniz_baslik = 1;
		else if (!strcmp(argv[i], "--en-cok") && i + 1 < argc) en_cok = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--dok") && i + 1 < argc) dok = argv[++i];
		else if (!akis) akis = argv[i];
		else bek = argv[i];
	}
	if (!akis) { fputs("kullanim: h264-ve-prova akis.h264 [beklenen.kontrol] [--yalniz-baslik] [--en-cok N] [--dok yol]\n", stderr); return 2; }
	FILE *f = fopen(akis, "rb");
	if (!f) { perror(akis); return 2; }
	fseek(f, 0, SEEK_END);
	long boy = ftell(f);
	rewind(f);
	uint8_t *v = malloc((size_t)boy + 16);
	if (!v || fread(v, 1, (size_t)boy, f) != (size_t)boy) return 2;
	fclose(f);
	uint32_t bekl[4096];
	int bek_n = 0;
	if (bek && (f = fopen(bek, "r"))) {
		while (bek_n < 4096 && fscanf(f, "%x", &bekl[bek_n]) == 1) bek_n++;
		fclose(f);
	}
	/* NAL'lar: [bas, son) baslik baytindan itibaren */
	static long nb[8192], ns[8192];
	int nn = 0;
	for (long i = 0; i + 3 < boy && nn < 8192; i++)
		if (v[i] == 0 && v[i + 1] == 0 && v[i + 2] == 1) {
			if (nn) { long s = i; while (s > nb[nn - 1] && v[s - 1] == 0) s--; ns[nn - 1] = s; }
			nb[nn++] = i + 3;
			i += 2;
		}
	if (nn) ns[nn - 1] = boy;
	printf("akis %s: %ld bayt, %d NAL\n", akis, boy, nn);

	ve_fd = open("/dev/cedar_dev", O_RDWR | O_CLOEXEC);
	ion_fd = open("/dev/ion", O_RDWR | O_CLOEXEC);
	if (ve_fd < 0 || ion_fd < 0) { puts("VE/ION acilamadi"); return 1; }
	struct ion_yigin hd[16];
	memset(hd, 0, sizeof hd);
	struct ion_sorgu q = { .cnt = 16, .heaps = (uintptr_t)hd };
	if (ioctl(ion_fd, ION_IOC_HEAP_QUERY, &q)) return 1;
	for (unsigned i = 0; i < q.cnt && i < 16; i++)
		if (hd[i].type == 0) ion_maske = 1u << hd[i].heap_id;	/* sistem yigini (IOMMU ile esler; CMA'ya dokunmaz) */
	if (!ion_maske) return 1;
	if (ioctl(ve_fd, VE_ENGINE_REQ, 0)) { perror("ENGINE_REQ"); return 1; }
	ve_req = 1;
	R = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, ve_fd, 0);
	if (R == MAP_FAILED) { perror("VE mmap"); return 1; }
	printf("VE surum 0x%08x\n", r(0xf0));

	struct tampon bit_t, pic_t, kom_t, kare_t[KONUM], mv_t[KONUM];
	int hata = 0, aw = 0, ah = 0;
	memset(kare_t, 0, sizeof kare_t);
	memset(mv_t, 0, sizeof mv_t);
	for (int i = 0; i < KONUM; i++) kare_t[i].fd = mv_t[i].fd = -1;
	hata |= ayir(&bit_t, 1 << 20);
	hata |= ayir(&pic_t, 128 * 1024);
	hata |= ayir(&kom_t, 32 * 1024);	/* komsu bilgisi: 16 KB, 16 KB hizali */
	if (hata) { puts("tampon ayrilamadi"); goto bitir; }
	uint32_t kom_adr = (kom_t.adr + 0x3fff) & ~0x3fffu;

	int kare = 0, konum = 0, uyusma = 0, uyusmaz = 0, konum_n = 0;
	/* v4 DPB: kisa sureli referanslar */
	struct dpb { int var, konum, fn, poc; } D[KONUM];
	memset(D, 0, sizeof D);
	int prev_fn = 0, prev_poc_msb = 0, prev_poc_lsb = 0, fno = 0, prev_fno = 0;
	double top_ms = 0, en_ms = 0;
	size_t ysz = 0;
	for (int ni = 0; ni < nn && kare < en_cok; ni++) {
		const uint8_t *n = v + nb[ni];
		size_t len = (size_t)(ns[ni] - nb[ni]);
		int tur = n[0] & 31;
		if (tur == 7) { if (sps_coz(n, len)) { hata = 1; break; } continue; }
		if (tur == 8) { if (pps_coz(n, len)) { hata = 1; break; } continue; }
		if (tur != 1 && tur != 5) continue;
		if (!S.var || !P.var) { puts("SPS/PPS yok"); hata = 1; break; }
		if (!aw) {
			aw = S.w_mb * 16;
			ah = S.h_mu * 16;
			ysz = (size_t)aw * ah;
			size_t alan = (size_t)S.w_mb * S.h_mu * 16 * 2 * 2;	/* cedrus: alan boyu (d8x8/fmo kosulsuz x2 x2) */
			konum_n = S.ref_sayi + 1 < 2 ? 2 : S.ref_sayi + 1 > KONUM ? KONUM : S.ref_sayi + 1;
			for (int i = 0; i < konum_n && !hata; i++) {
				hata |= ayir(&kare_t[i], ysz + ysz / 2);
				hata |= ayir(&mv_t[i], alan * 2);
			}
			if (hata) { puts("kare tamponu ayrilamadi"); break; }
			printf("kare %dx%d (hizali), NV12 %zu B x %d, mv_col %zu B x %d\n", aw, ah, ysz * 3 / 2, konum_n, alan * 2, konum_n);
		}
		/* yazilim basligi */
		static uint8_t t[1 << 20];
		struct bit sb = { t, rbsp(n, len < 4096 ? len : 4096, t), 0, 0 };
		struct okuyucu so = { 0, &sb };
		struct baslik hs, hv;
		if (baslik_oku(&so, &hs)) { puts("yazilim basligi okunamadi"); hata = 1; break; }
		int son_dilim = 1;	/* v3: sonraki dilim yeni kare mi */
		for (int nj = ni + 1; nj < nn; nj++) {
			int tj = v[nb[nj]] & 31;
			if (tj == 1 || tj == 5) { son_dilim = ilk_mb_oku(v + nb[nj], (size_t)(ns[nj] - nb[nj])) == 0; break; }
		}
		/* bit akisini VE tamponuna koy (NAL baslik baytindan) */
		if (len > bit_t.n - 64) { puts("NAL cok buyuk"); hata = 1; break; }
		memcpy(bit_t.m, n, len);
		memset(bit_t.m + len, 0, 64);
		__sync_synchronize();
		w(VE_MODE, 0x00130001);	/* REC_WR 2MB | DDR BW128 | DEC_H264 (JPEG: 0x00130000) */
		w(H_VLD_LEN, (uint32_t)(len * 8));
		w(H_VLD_OFFSET, 0);
		w(H_VLD_END, bit_t.adr + (uint32_t)len + 64);
		w(H_VLD_ADDR, (bit_t.adr & 0x0ffffff0u) | (bit_t.adr >> 28) | (1u << 28) | (1u << 29) | (1u << 30));
		w(H_TRIG, 7);	/* INIT_SWDEC */
		ve_okuma_hatasi = 0;
		struct okuyucu vo = { 1, NULL };
		int vh = baslik_oku(&vo, &hv);
		int ayni = !vh && hv.nal_ref == hs.nal_ref && hv.nal_tur == hs.nal_tur && hv.tur == hs.tur && hv.fn == hs.fn &&
			   hv.poc_lsb == hs.poc_lsb && hv.qp_d == hs.qp_d && hv.dbk_idc == hs.dbk_idc && hv.ref0 == hs.ref0 &&
			   hv.cabac_idc == hs.cabac_idc && hv.idr_id == hs.idr_id;
		if (kare < 3 || !ayni)
			printf("kare %d: baslik yazilim{nal %d/%d tur %d fn %d poc %d qp_d %d dbk %d ref0 %d cabac %d, %d bit} VE{%s nal %d/%d tur %d fn %d poc %d qp_d %d dbk %d} %s\n",
			       kare, hs.nal_ref, hs.nal_tur, hs.tur, hs.fn, hs.poc_lsb, hs.qp_d, hs.dbk_idc, hs.ref0, hs.cabac_idc,
			       hs.uzunluk_bit, vh ? "OKUMA HATASI" : "", hv.nal_ref, hv.nal_tur, hv.tur, hv.fn, hv.poc_lsb, hv.qp_d,
			       hv.dbk_idc, ayni ? "AYNI" : "FARKLI");
		if (!ayni) { puts("VE basligi yazilimla uyusmuyor: cozme yapilmadi"); hata = 1; w(VE_MODE, 7); break; }
		if (yalniz_baslik) { kare++; w(VE_MODE, 7); continue; }

		/* POC (v3: yalniz karenin ilk diliminde) */
		static int poc;
		if (hs.ilk_mb == 0) {
		if (hs.idr) { prev_poc_msb = prev_poc_lsb = 0; fno = 0; prev_fno = 0; prev_fn = 0; }
		if (S.poc_tur == 0) {
			int maxl = 1 << S.log2_poc, msb = prev_poc_msb;
			if (hs.poc_lsb < prev_poc_lsb && prev_poc_lsb - hs.poc_lsb >= maxl / 2) msb += maxl;
			else if (hs.poc_lsb > prev_poc_lsb && hs.poc_lsb - prev_poc_lsb > maxl / 2) msb -= maxl;
			poc = msb + hs.poc_lsb;
			if (hs.nal_ref) { prev_poc_msb = msb; prev_poc_lsb = hs.poc_lsb; }
		} else {
			if (!hs.idr) fno = prev_fn > hs.fn ? prev_fno + (1 << S.log2_fn) : prev_fno;
			poc = 2 * (fno + hs.fn) - (hs.nal_ref ? 0 : 1);
			prev_fno = fno;
			prev_fn = hs.fn;
		}
		/* cikis konumu: hicbir referansin kullanmadigi konum (v4) */
		if (hs.idr)
			memset(D, 0, sizeof D);
		for (konum = 0; konum < konum_n; konum++) {
			int dolu = 0;
			for (int i = 0; i < KONUM; i++) dolu |= D[i].var && D[i].konum == konum;
			if (!dolu) break;
		}
		if (konum >= konum_n) { puts("bos konum yok (DPB dolu)"); hata = 1; break; }
		if ((hs.mod || hs.mmco) && kare < 50) printf("kare %d: UYARI ref listesi degisikligi/MMCO var, uygulanmiyor\n", kare);
		}
		/* cikis bicimi (dogrusal NV12), ek tamponlar */
		w(VE_PRIMARY_OUT_FMT, 4u << 4);
		w(VE_CHROMA_BUF_LEN, 0);
		w(VE_PRIMARY_CHROMA_BUF_LEN, (uint32_t)(ysz / 2 / 2));
		w(VE_PRIMARY_FB_LINE_STRIDE, (uint32_t)aw | ((uint32_t)aw / 2) << 16);
		w(H_SDROT, 0);
		w(H_EXTRA1, pic_t.adr);
		w(H_EXTRA2, kom_adr);
		/* olcek listeleri: duz 16 */
		static uint8_t duz[96];
		memset(duz, 16, sizeof duz);
		sram(SRAM_SCALE8_0, duz, 64);
		sram(SRAM_SCALE8_1, duz, 64);
		sram(SRAM_SCALE4, duz, 96);
		/* cerceve listesi */
		struct ref_pic pl[18];
		memset(pl, 0, sizeof pl);
		size_t alan = mv_t[0].n / 2;
		for (int i = 0; i < KONUM; i++) {	/* v4: tum referanslar */
			if (!D[i].var) continue;
			struct ref_pic *e = &pl[D[i].konum];
			int k2 = D[i].konum;
			e->top_poc = e->bot_poc = (uint32_t)D[i].poc;
			e->luma = kare_t[k2].adr;
			e->chroma = kare_t[k2].adr + (uint32_t)ysz;
			e->mv_top = mv_t[k2].adr;
			e->mv_bot = mv_t[k2].adr + (uint32_t)alan;
		}
		struct ref_pic *c = &pl[konum];
		c->top_poc = c->bot_poc = (uint32_t)poc;
		c->info = 0;	/* FRAME << 8 */
		c->luma = kare_t[konum].adr;
		c->chroma = kare_t[konum].adr + (uint32_t)ysz;
		c->mv_top = mv_t[konum].adr;
		c->mv_bot = mv_t[konum].adr + (uint32_t)alan;
		sram(SRAM_FRAMELIST, pl, sizeof pl);
		w(H_OUT_IDX, (uint32_t)konum);
		if (hs.tur == 0) {	/* v4: liste0 = FrameNumWrap azalan */
			int sira[KONUM], m = 0, maxfn = 1 << S.log2_fn;
			for (int i = 0; i < KONUM; i++) if (D[i].var) sira[m++] = i;
			for (int a = 0; a < m; a++)
				for (int b2 = a + 1; b2 < m; b2++) {
					int wa = D[sira[a]].fn > hs.fn ? D[sira[a]].fn - maxfn : D[sira[a]].fn;
					int wb = D[sira[b2]].fn > hs.fn ? D[sira[b2]].fn - maxfn : D[sira[b2]].fn;
					if (wb > wa) { int t2 = sira[a]; sira[a] = sira[b2]; sira[b2] = t2; }
				}
			if (!m) { puts("P dilimi icin referans yok"); hata = 1; break; }
			uint8_t rl[32];
			memset(rl, 0, sizeof rl);
			for (int i = 0; i < hs.ref0 && i < 32; i++) rl[i] = (uint8_t)(D[sira[i < m ? i : m - 1]].konum << 1);
			sram(SRAM_REFLIST0, rl, (size_t)((hs.ref0 + 3) & ~3));
		}
		uint32_t reg = ((uint32_t)(hs.ref0 - 1) & 0x1f) << 10 | ((uint32_t)(P.ref1 - 1) & 0x1f) << 5 | ((uint32_t)P.wbi & 3) << 2;
		if (P.entropi) reg |= 1u << 15;
		if (P.cip) reg |= 1u << 1;
		if (P.t8x8) reg |= 1u << 0;
		w(H_PPS, reg);
		reg = ((uint32_t)S.chroma & 7) << 19 | ((uint32_t)(S.w_mb - 1) & 0xff) << 8 | ((uint32_t)(S.h_mu - 1) & 0xff);
		if (S.fmo) reg |= 1u << 18;
		if (S.mbaff) reg |= 1u << 17;
		if (S.d8x8) reg |= 1u << 16;
		w(H_SPS, reg);
		reg = (hs.nal_ref ? 1u << 12 : 0) | ((uint32_t)hs.tur & 0xf) << 8 | ((uint32_t)hs.cabac_idc & 3) |
		      ((uint32_t)(hs.ilk_mb % S.w_mb) & 0xff) << 24 | ((uint32_t)(hs.ilk_mb / S.w_mb) & 0xff) << 16 |
		      (hs.ilk_mb == 0 ? 1u << 5 : 0);
		w(H_SHS, reg);
		reg = 1u << 12 | ((uint32_t)(hs.ref0 - 1) & 0x1f) << 24 | ((uint32_t)(P.ref1 - 1) & 0x1f) << 16 |
		      ((uint32_t)hs.dbk_idc & 3) << 8 | ((uint32_t)hs.alfa & 0xf) << 4 | ((uint32_t)hs.beta & 0xf);
		w(H_SHS2, reg);
		w(H_SHS_WP, 0);
		reg = ((uint32_t)P.cqp2 & 0x3f) << 16 | ((uint32_t)P.cqp & 0x3f) << 8 | ((uint32_t)(P.qp + hs.qp_d) & 0x3f);
		w(H_SHS_QP, reg);
		w(H_STATUS, r(H_STATUS));
		w(H_CTRL, 0);	/* kesme yok, yoklama (JPEG yolu gibi) */
		double t0 = ms();
		w(H_TRIG, 8);	/* AVC_SLICE_DECODE */
		uint32_t st = 0;
		while (ms() - t0 < 500 && !((st = r(H_STATUS)) & 7))
			;
		double d = ms() - t0;
		uint32_t mb = r(H_CUR_MB);
		w(H_STATUS, 7);
		w(VE_MODE, 7);
		if (!(st & 1) || (st & 6)) {
			printf("kare %d: VE HATA durum 0x%08x, cur_mb 0x%08x, %.1f ms\n", kare, st, mb, d);
			if (!(st & 7)) ioctl(ve_fd, VE_RESET, 0);
			hata = 1;
			break;
		}
		top_ms += d;
		static double kare_ms;
		kare_ms += d;
		if (!son_dilim)
			continue;	/* v3: kare son dilimde biter */
		d = kare_ms;
		kare_ms = 0;
		if (d > en_ms) en_ms = d;
		/* kontrol */
		uint32_t k = 0;
		static uint8_t sat[4096];
		for (int y = 0; y < S.h_mu * 16 && y < ah; y++) {
			memcpy(sat, kare_t[konum].m + (size_t)y * aw, (size_t)aw);	/* onbelleksiz: satir satir kopyala */
			uint32_t s = 0;
			for (int x = 0; x < aw; x++) s += sat[x];
			k += (uint32_t)(y + 1) * s;
		}
		/* kontrol PyAV'de kirpilmis (600) yukseklik ve genislik uzerinden: kirpma burada yeniden hesaplanir */
		uint32_t kk = 0;
		int gy = bek_n ? 600 : ah, gx = bek_n ? 1600 : aw;
		for (int y = 0; y < gy && y < ah; y++) {
			memcpy(sat, kare_t[konum].m + (size_t)y * aw, (size_t)aw);
			uint32_t s = 0;
			for (int x = 0; x < gx; x++) s += sat[x];
			kk += (uint32_t)(y + 1) * s;
		}
		(void)k;
		int dogru = kare < bek_n && kk == bekl[kare];
		if (kare < bek_n) { if (dogru) uyusma++; else uyusmaz++; }
		if (kare < 5 || !dogru)
			printf("kare %d: %s %.2f ms, kontrol %08x%s\n", kare, hs.tur == 2 ? "I" : "P", d, kk,
			       kare < bek_n ? (dogru ? " DOGRU" : " YANLIS") : "");
		if (dok && kare == 0) {
			FILE *df = fopen(dok, "wb");
			if (df) {
				for (int y = 0; y < ah; y++) { memcpy(sat, kare_t[konum].m + (size_t)y * aw, (size_t)aw); fwrite(sat, 1, (size_t)aw, df); }
				fclose(df);
			}
		}
		if (hs.nal_ref) {	/* v4: kayan pencere */
			int n_ref = 0, en_kucuk = -1, maxfn = 1 << S.log2_fn, ek_wrap = 1 << 30;
			for (int i = 0; i < KONUM; i++) {
				if (!D[i].var) continue;
				n_ref++;
				int wr = D[i].fn > hs.fn ? D[i].fn - maxfn : D[i].fn;
				if (wr < ek_wrap) { ek_wrap = wr; en_kucuk = i; }
			}
			int sinir = S.ref_sayi < 1 ? 1 : S.ref_sayi;
			if (n_ref >= sinir && en_kucuk >= 0) D[en_kucuk].var = 0;
			for (int i = 0; i < KONUM; i++)
				if (!D[i].var) { D[i].var = 1; D[i].konum = konum; D[i].fn = hs.fn; D[i].poc = poc; break; }
		}
		kare++;
	}
	if (!yalniz_baslik && kare)
		printf("SONUC: %d kare, VE cozme ort %.2f ms en cok %.2f ms; kontrol %d dogru %d yanlis (beklenen %d)\n", kare,
		       top_ms / kare, en_ms, uyusma, uyusmaz, bek_n);
	else if (yalniz_baslik)
		printf("SONUC: %d dilim basligi VE ile okundu, hepsi yazilimla ayni\n", kare);
bitir:
	if (R && R != MAP_FAILED) w(VE_MODE, 7);
	for (int i = 0; i < KONUM; i++) { birak(&kare_t[i]); birak(&mv_t[i]); }
	birak(&bit_t);
	birak(&pic_t);
	birak(&kom_t);
	if (R && R != MAP_FAILED) munmap((void *)R, 4096);
	if (ve_req) ioctl(ve_fd, VE_ENGINE_REL, 0);
	close(ve_fd);
	close(ion_fd);
	free(v);
	return hata;
}
