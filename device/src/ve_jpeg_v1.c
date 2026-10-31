/* ve-jpeg v1 (11.10.2026, Claude): Cedar VE ile DONANIMSAL baseline JPEG cozme PROTOTIPI (D1s/F133, Tina 5.4 cedar_dev).
 * Yazmac dizisi: Link Mauve/Jernej "media: cedrus: Add a JPEG decoder" (2024, GPL-2, D1 dahil, birlesmedi) +
 * ana hat cedrus_regs.h. Guvenlik: KESME ACILMAZ (MPEG_CTRL irq bitleri 0), durum yazmaci yoklanir; 1 sn'de bitmezse
 * VE_MODE=7 + RESET_VE. Bellek ION (CMA 'reserved', onbelleksiz) + IOMMU (MAP_DMA_BUF): VE yanlis adrese yazamaz.
 * Dogrulama: ayni JPEG TJpgDec ile de cozulur, Y (parlaklik) duzleminin ortalama mutlak farki yazilir.
 *   ve-jpeg <dosya.jpg> [tekrar]      (arka planda calistirin; gunluk stdout)
 * Yalniz 8 bit, 3 bilesen, baseline (SOF0); 4:2:0 / 4:2:2 / 4:4:4 (FORCE_420 ile NV12 cikis). */
#define _GNU_SOURCE
#include "tjpgd.h"
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

#define VE_RESET 0x104
#define VE_ENGINE_REQ 0x206
#define VE_ENGINE_REL 0x207
#define VE_MAP_DMA_BUF 0x504
#define VE_UNMAP_DMA_BUF 0x505
struct dma_buf_param { int fd; unsigned int phy_addr; };
struct ion_allocation_data { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
struct ion_heap_data { char name[32]; uint32_t type, heap_id, r0, r1, r2; };
struct ion_heap_query { uint32_t cnt, r0; uint64_t heaps; uint32_t r1, r2; };
#define ION_IOC_ALLOC _IOWR('I', 0, struct ion_allocation_data)
#define ION_IOC_HEAP_QUERY _IOWR('I', 8, struct ion_heap_query)

/* yazmaclar (VE tabani 0x01C0E000'a gore) */
#define VE_MODE 0x00
#define VE_PRIMARY_CHROMA_BUF_LEN 0xc4
#define VE_PRIMARY_FB_LINE_STRIDE 0xc8
#define VE_SECONDARY_FB_LINE_STRIDE 0xcc
#define VE_CHROMA_BUF_LEN 0xe8
#define VE_PRIMARY_OUT_FMT 0xec
#define M 0x100
#define MPEG_CTRL (M + 0x14)
#define MPEG_TRIGGER (M + 0x18)
#define MPEG_STATUS (M + 0x1c)
#define MPEG_VLD_ADDR (M + 0x28)
#define MPEG_VLD_OFFSET (M + 0x2c)
#define MPEG_VLD_LEN (M + 0x30)
#define MPEG_VLD_END (M + 0x34)
#define MPEG_IQMINPUT (M + 0x80)
#define MPEG_JPEG_SIZE (M + 0xb8)
#define MPEG_JPEG_RES_INT (M + 0xc0)
#define MPEG_ERROR (M + 0xc4)
#define MPEG_ROT_LUMA (M + 0xcc)
#define MPEG_ROT_CHROMA (M + 0xd0)
#define MPEG_SD_ROT_DBLK_CTL (M + 0xd4)
#define MPEG_SRAM_RW_OFFSET (M + 0xe0)
#define MPEG_SRAM_RW_DATA (M + 0xe4)
#define TRIG_JPEG (3u << 24)
#define TRIG_HW_JPEG_VLD 0x0e
#define CTRL_FORCE_420 (1u << 18)

static volatile uint32_t *R;
static void w(unsigned off, uint32_t v) { R[off / 4] = v; }
static uint32_t r(unsigned off) { return R[off / 4]; }
static double ms(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }

/* ------------------------------------------------------------ JPEG basligi */
struct jpg {
	unsigned w, h, ncomp, hs[3], vs[3], qsel[3], dcsel[3], acsel[3], dri;
	const uint8_t *qt[4]; unsigned qlen[4];		/* 64 (8 bit) ya da 128 (16 bit) */
	const uint8_t *ht[2][4]; unsigned hlen[2][4];	/* [sinif dc=0/ac=1][id]: 16 sayi + semboller */
	size_t ecs;					/* entropi kodlu verinin dosyadaki ofseti */
};

static int jpeg_ayristir(const uint8_t *d, size_t n, struct jpg *j)
{
	memset(j, 0, sizeof *j);
	if (n < 4 || d[0] != 0xff || d[1] != 0xd8) return -1;
	size_t p = 2;
	int sof = 0;
	while (p + 4 <= n) {
		if (d[p] != 0xff) return -2;
		uint8_t mk = d[p + 1];
		if (mk == 0xff) { p++; continue; }
		unsigned len = d[p + 2] << 8 | d[p + 3];
		const uint8_t *s = d + p + 4, *e = d + p + 2 + len;
		if (p + 2 + len > n || len < 2) return -3;
		if (mk == 0xc0) {				/* SOF0 baseline */
			if (s[0] != 8) return -10;
			j->h = s[1] << 8 | s[2]; j->w = s[3] << 8 | s[4]; j->ncomp = s[5];
			if (j->ncomp != 3) return -11;
			for (unsigned i = 0; i < 3; i++) {
				j->hs[i] = s[7 + 3 * i] >> 4; j->vs[i] = s[7 + 3 * i] & 15; j->qsel[i] = s[8 + 3 * i] & 3;
			}
			sof = 1;
		} else if (mk >= 0xc1 && mk <= 0xcf && mk != 0xc4 && mk != 0xc8 && mk != 0xcc) {
			return -12;				/* progressive vb. */
		} else if (mk == 0xdb) {			/* DQT (birden cok tablo olabilir) */
			while (s < e) {
				unsigned pq = s[0] >> 4, tq = s[0] & 3, l = pq ? 128 : 64;
				j->qt[tq] = s + 1; j->qlen[tq] = l; s += 1 + l;
			}
		} else if (mk == 0xc4) {			/* DHT */
			while (s < e) {
				unsigned tc = s[0] >> 4, th = s[0] & 3, cnt = 0;
				for (int i = 0; i < 16; i++) cnt += s[1 + i];
				if (tc > 1 || cnt > 256) return -13;
				j->ht[tc][th] = s + 1; j->hlen[tc][th] = 16 + cnt; s += 17 + cnt;
			}
		} else if (mk == 0xdd) {
			j->dri = s[0] << 8 | s[1];
		} else if (mk == 0xda) {			/* SOS */
			if (!sof || s[0] != 3) return -14;
			for (unsigned i = 0; i < 3; i++) { j->dcsel[i] = s[2 + 2 * i] >> 4; j->acsel[i] = s[2 + 2 * i] & 15; }
			j->ecs = (size_t)(e - d);
			return 0;
		}
		p += 2 + len;
	}
	return -4;
}

/* ------------------------------------------------------------ VE tablolari */
static void huff_baslik(const uint8_t *t)
{
	uint16_t sc[16]; uint8_t of[16]; unsigned cnt = 0, code = 0;
	for (int i = 0; i < 16; i++) { of[i] = (uint8_t)cnt; sc[i] = (uint16_t)code; cnt += t[i]; code += t[i]; code *= 2; }
	for (int i = 15; i >= 0 && !t[i]; i--) sc[i] = 0xffff;
	for (int i = 0; i < 8; i++) w(MPEG_SRAM_RW_DATA, (uint32_t)sc[2 * i] | (uint32_t)sc[2 * i + 1] << 16);
	for (int i = 0; i < 4; i++) w(MPEG_SRAM_RW_DATA, of[4 * i] | of[4 * i + 1] << 8 | of[4 * i + 2] << 16 | (uint32_t)of[4 * i + 3] << 24);
	for (int i = 0; i < 4; i++) w(MPEG_SRAM_RW_DATA, 0);
}

static int huff_tablolar(const struct jpg *j)
{
	const uint8_t *t[4]; unsigned l[4];
	t[0] = j->ht[0][j->dcsel[0]]; l[0] = j->hlen[0][j->dcsel[0]];
	t[1] = j->ht[1][j->acsel[0]]; l[1] = j->hlen[1][j->acsel[0]];
	t[2] = j->ht[0][j->dcsel[1]]; l[2] = j->hlen[0][j->dcsel[1]];
	t[3] = j->ht[1][j->acsel[1]]; l[3] = j->hlen[1][j->acsel[1]];
	for (int i = 0; i < 4; i++) if (!t[i]) return -1;
	w(MPEG_SRAM_RW_OFFSET, 0);
	for (int i = 0; i < 4; i++) huff_baslik(t[i]);
	for (int i = 0; i < 192; i++) w(MPEG_SRAM_RW_DATA, 0);
	for (int i = 0; i < 4; i++) {
		unsigned n = l[i] - 16, words = (n + 3) / 4;
		const uint8_t *s = t[i] + 16;
		for (unsigned k = 0; k < words; k++) {
			uint32_t v = 0;
			for (unsigned b = 0; b < 4; b++) if (4 * k + b < n) v |= (uint32_t)s[4 * k + b] << (8 * b);
			w(MPEG_SRAM_RW_DATA, v);
		}
		for (unsigned k = words; k < 64; k++) w(MPEG_SRAM_RW_DATA, 0);
	}
	return 0;
}

static void niceleme(const uint8_t *q, unsigned len, uint32_t bayrak)
{
	for (unsigned i = 0; i < 64; i++) {
		uint32_t v = len > 64 ? (uint32_t)(q[2 * i] << 8 | q[2 * i + 1]) : q[i];
		w(MPEG_IQMINPUT, ((i & 63) << 8) | (v & 0xff) | bayrak);
	}
}

/* ------------------------------------------------------------ ION */
static int ion_fd = -1; static uint32_t ion_mask;
static int ion_ayir(size_t n, int ve, int *fd, uint8_t **map, uint32_t *ve_adr)
{
	struct ion_allocation_data a = { .len = n, .heap_id_mask = ion_mask, .flags = 0 };
	if (ioctl(ion_fd, ION_IOC_ALLOC, &a)) return -1;
	*fd = (int)a.fd;
	*map = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, *fd, 0);
	if (*map == MAP_FAILED) return -2;
	struct dma_buf_param p = { .fd = *fd };
	if (ioctl(ve, VE_MAP_DMA_BUF, &p)) return -3;
	*ve_adr = p.phy_addr;
	return 0;
}

/* ------------------------------------------------------------ yazilim referansi (TJpgDec, Y duzlemi) */
static const uint8_t *sw_veri; static size_t sw_n, sw_off; static uint8_t *sw_y; static unsigned sw_w;
static size_t sw_in(JDEC *jd, uint8_t *b, size_t n) { (void)jd; if (n > sw_n - sw_off) n = sw_n - sw_off; if (b) memcpy(b, sw_veri + sw_off, n); sw_off += n; return n; }
static int sw_out(JDEC *jd, void *bmp, JRECT *rc)
{
	(void)jd; const uint8_t *p = bmp;	/* LVGL kopyasi: B,G,R */
	for (unsigned y = rc->top; y <= rc->bottom; y++)
		for (unsigned x = rc->left; x <= rc->right; x++, p += 3)
			sw_y[(size_t)y * sw_w + x] = (uint8_t)((29 * p[0] + 150 * p[1] + 77 * p[2] + 128) >> 8);
	return 1;
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc < 2) { puts("kullanim: ve-jpeg <dosya.jpg> [tekrar]"); return 2; }
	int tekrar = argc > 2 ? atoi(argv[2]) : 3;
	FILE *f = fopen(argv[1], "rb");
	if (!f) { perror(argv[1]); return 2; }
	static uint8_t dosya[1 << 20];
	size_t n = fread(dosya, 1, sizeof dosya, f);
	fclose(f);
	struct jpg j;
	int pr = jpeg_ayristir(dosya, n, &j);
	printf("JPEG %zu B ayristirma=%d %ux%u ornekleme %u%u/%u%u/%u%u ecs=%zu dri=%u\n", n, pr, j.w, j.h,
	       j.hs[0], j.vs[0], j.hs[1], j.vs[1], j.hs[2], j.vs[2], j.ecs, j.dri);
	if (pr) return 3;
	unsigned idx = j.hs[0] << 20 | j.vs[0] << 16 | j.hs[1] << 12 | j.vs[1] << 8 | j.hs[2] << 4 | j.vs[2];
	uint32_t ss;
	switch (idx) {
	case 0x221111: ss = 0u << 27; break;
	case 0x211111: ss = 2u << 27; break;
	case 0x111111: ss = 3u << 27; break;
	case 0x121111: ss = 4u << 27; break;
	default: printf("desteklenmeyen ornekleme %06x\n", idx); return 4;
	}
	unsigned hmax = j.hs[0], vmax = j.vs[0];
	unsigned AW = (j.w + 15) & ~15u, AH = (j.h + 15) & ~15u;
	size_t ysz = (size_t)AW * AH, csz = ysz / 2;

	int ve = open("/dev/cedar_dev", O_RDWR | O_CLOEXEC);
	ion_fd = open("/dev/ion", O_RDWR | O_CLOEXEC);
	if (ve < 0 || ion_fd < 0) { perror("cedar/ion"); return 5; }
	struct ion_heap_data hd[16]; memset(hd, 0, sizeof hd);
	struct ion_heap_query q = { .cnt = 16, .heaps = (uintptr_t)hd };
	if (ioctl(ion_fd, ION_IOC_HEAP_QUERY, &q)) return 6;
	for (unsigned i = 0; i < q.cnt && i < 16; i++) if (hd[i].type == 2) { ion_mask = 1u << hd[i].heap_id; break; }
	int bfd, ofd; uint8_t *bm, *om; uint32_t badr, oadr;
	size_t bsz = (n + 4095) & ~4095ul, osz = (ysz + csz + 4095) & ~4095ul;
	if (ion_ayir(bsz, ve, &bfd, &bm, &badr) || ion_ayir(osz, ve, &ofd, &om, &oadr)) { puts("ION/MAP hatasi"); return 7; }
	memcpy(bm, dosya, n);
	memset(om, 0x5a, osz);
	printf("bit akisi VE adr 0x%08x (%zu B), cikis VE adr 0x%08x (%zu B)\n", badr, bsz, oadr, osz);
	if (ioctl(ve, VE_ENGINE_REQ, 0)) { puts("ENGINE_REQ hatasi"); return 8; }
	R = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, ve, 0);
	if (R == MAP_FAILED) { puts("mmap hatasi"); return 9; }
	printf("VE surum 0x%08x, mod 0x%08x\n", r(0xf0), r(VE_MODE));

	double toplam = 0; int basari = 0;
	for (int t = 0; t < tekrar; t++) {
		double t0 = ms();
		w(VE_MODE, 0x00130000);			/* MPEG motoru, 2MB yazma kipi, DDR BW 128 */
		/* cikis bicimi: dogrusal NV12 (birincil + ikincil/rot) */
		w(VE_PRIMARY_OUT_FMT, (4u << 4) | 4u);
		w(VE_PRIMARY_CHROMA_BUF_LEN, (uint32_t)(csz / 2));
		w(VE_CHROMA_BUF_LEN, ((uint32_t)(csz / 2) & 0x0fffffff) | (1u << 29) | (1u << 30));
		w(VE_PRIMARY_FB_LINE_STRIDE, AW | (AW / 2) << 16);
		w(VE_SECONDARY_FB_LINE_STRIDE, AW | (AW / 2) << 16);
		w(MPEG_TRIGGER, TRIG_JPEG | ss);
		w(MPEG_JPEG_RES_INT, j.dri);
		w(MPEG_JPEG_SIZE, (((j.w + 8 * hmax - 1) / (8 * hmax) - 1) & 0x7ff) |
				  ((((j.h + 8 * vmax - 1) / (8 * vmax) - 1) & 0x7ff) << 16));
		if (!j.qt[j.qsel[0]] || !j.qt[j.qsel[1]]) { puts("niceleme tablosu yok"); break; }
		niceleme(j.qt[j.qsel[0]], j.qlen[j.qsel[0]], 1u << 14);
		niceleme(j.qt[j.qsel[1]], j.qlen[j.qsel[1]], 0);
		if (huff_tablolar(&j)) { puts("Huffman tablosu yok"); break; }
		w(MPEG_ROT_LUMA, oadr);
		w(MPEG_ROT_CHROMA, oadr + (uint32_t)ysz);
		w(MPEG_SD_ROT_DBLK_CTL, 0);
		w(MPEG_VLD_OFFSET, (uint32_t)(8 * j.ecs));
		w(MPEG_VLD_LEN, (uint32_t)(n * 8));
		w(MPEG_VLD_ADDR, (badr & 0x0ffffff0u) | (badr >> 28) | (1u << 28) | (1u << 29) | (1u << 30));
		w(MPEG_VLD_END, badr + (uint32_t)n);
		w(MPEG_STATUS, 0x7);			/* eski durum temizle */
		w(MPEG_CTRL, ss ? CTRL_FORCE_420 : 0);	/* KESME YOK */
		double t1 = ms();
		w(MPEG_TRIGGER, TRIG_HW_JPEG_VLD | TRIG_JPEG | ss);
		uint32_t st = 0;
		while (ms() - t1 < 1000 && !((st = r(MPEG_STATUS)) & 0x7))
			;
		double t2 = ms();
		uint32_t err = r(MPEG_ERROR);
		printf("tur %d: durum 0x%08x hata 0x%08x | kurulum %.2f ms, VE cozme %.2f ms\n", t, st, err, t1 - t0, t2 - t1);
		w(MPEG_STATUS, 0x7);
		w(VE_MODE, 7);
		if (!(st & 0x7)) { puts("ZAMAN ASIMI: VE resetleniyor"); ioctl(ve, VE_RESET, 0); break; }
		if (st & 1) { basari++; toplam += t2 - t1; }
		else break;
	}
	if (basari) {
		printf("BASARILI %d/%d, ortalama VE cozme %.2f ms\n", basari, tekrar, toplam / basari);
		/* yazilim referansi */
		static uint8_t pool[65536];
		sw_y = calloc(ysz, 1); sw_w = AW; sw_veri = dosya; sw_n = n; sw_off = 0;
		JDEC jd;
		double s0 = ms();
		int rr = jd_prepare(&jd, sw_in, pool, sizeof pool, NULL);
		if (!rr) rr = jd_decomp(&jd, sw_out, 0);
		double s1 = ms();
		uint64_t fark = 0, cok = 0;
		for (unsigned y = 0; y < j.h; y++)
			for (unsigned x = 0; x < j.w; x++) {
				int dlt = (int)om[(size_t)y * AW + x] - (int)sw_y[(size_t)y * AW + x];
				if (dlt < 0) dlt = -dlt;
				fark += (unsigned)dlt;
				if (dlt > 24) cok++;
			}
		printf("TJpgDec %d, yazilim cozme %.1f ms | Y ort. mutlak fark %.2f, >24 fark %llu piksel / %u\n", rr, s1 - s0,
		       (double)fark / (j.w * j.h), (unsigned long long)cok, j.w * j.h);
		printf("VE Y ornek: %u %u %u | yazilim Y: %u %u %u\n", om[0], om[ysz / 2 + AW / 2], om[(size_t)(j.h - 1) * AW + j.w - 1],
		       sw_y[0], sw_y[ysz / 2 + AW / 2], sw_y[(size_t)(j.h - 1) * AW + j.w - 1]);
	}
	munmap((void *)R, 4096);
	ioctl(ve, VE_ENGINE_REL, 0);
	struct dma_buf_param p1 = { .fd = bfd }, p2 = { .fd = ofd };
	ioctl(ve, VE_UNMAP_DMA_BUF, &p1);
	ioctl(ve, VE_UNMAP_DMA_BUF, &p2);
	puts("VE-JPEG BITTI");
	return basari ? 0 : 1;
}
