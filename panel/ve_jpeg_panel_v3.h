/* ve_jpeg_panel_v3.h (25.10.2026, Claude; scripts/655): v2 + G2D ile NV12 -> XRGB8888 olcekli donusum (vj_g2d_*).
 * G2D hedefi 3 ION tampon (sistem yigini, onbellekli; IOMMU adresi VE MAP_DMA_BUF ile, ENGINE_REQ acikken).
 * --- v2 ---
 * ve_jpeg_panel_v2.h (25.10.2026, Claude; scripts/649): v1 + vj_xrgb8888 (ekran XRGB8888: LVGL donusumsuz kopyalar).
 * --- v1 ---
 * ve_jpeg_panel_v1.h (25.10.2026, Claude; OTONOM 49, scripts/647): panel canli kamera icin Cedar VE donanim JPEG cozme.
 * Kaynak: wifi-ekran v23 VE yolu (sahada; ayristirici + tablolar oradan vj_ onekiyle) ve ve_jpeg_v1 prototipi (1600x600
 * 8.4 ms, tjpgd 164 ms). Guvenlik: kesme acilmaz, durum yoklanir; 300 ms'de bitmezse VE_RESET (cagiran yazilima doner).
 * Bellek ION + IOMMU (VE yanlis adrese yazamaz). Sira: ONCE ENGINE_REQ, SONRA MAP_DMA_BUF (tersi 11.10'da cekirdekte
 * takildi); her tampon UNMAP_DMA_BUF ile birakilir (surucu birakmiyor: CMA sizintisi). Cikis onbellekli + DMA-BUF senkron;
 * senkron yoksa VE kullanilmaz (bayat onbellek okunmasin).
 *   vj_ac() 0/-1 (vj_hata) | vj_coz(jpeg, n, &w, &h): 0 tamam, -1 bicim (bu kare yazilim), -2 donanim (VE'yi kapat)
 *   vj_rgb565(hedef, dw, dh, sw, sh): son kareyi dw x dh RGB565'e (en yakin komsu, BT.601 tam aralik) | vj_kapat() */
#ifndef VE_JPEG_PANEL_V3_H
#define VE_JPEG_PANEL_V3_H
#include <fcntl.h>
#include <linux/dma-buf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>	/* g2d_driver_tina.h bool kullanir */
#include "g2d_driver_tina.h"

#define VJ_RESET 0x104
#define VJ_ENGINE_REQ 0x206
#define VJ_ENGINE_REL 0x207
#define VJ_MAP_DMA_BUF 0x504
#define VJ_UNMAP_DMA_BUF 0x505
struct vj_dma_buf_param { int fd; unsigned int phy_addr; };
struct vj_ion_alloc { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
struct vj_ion_heap_data { char name[32]; uint32_t type, heap_id, r0, r1, r2; };
struct vj_ion_heap_query { uint32_t cnt, r0; uint64_t heaps; uint32_t r1, r2; };
#define VJ_ION_IOC_ALLOC _IOWR('I', 0, struct vj_ion_alloc)
#define VJ_ION_IOC_HEAP_QUERY _IOWR('I', 8, struct vj_ion_heap_query)
/* yazmaclar (VE tabani; wifi-ekran / cedrus ile ayni) */
#define VJ_MODE 0x00
#define VJ_PRIMARY_CHROMA_BUF_LEN 0xc4
#define VJ_PRIMARY_FB_LINE_STRIDE 0xc8
#define VJ_SECONDARY_FB_LINE_STRIDE 0xcc
#define VJ_CHROMA_BUF_LEN 0xe8
#define VJ_PRIMARY_OUT_FMT 0xec
#define VJ_MPEG_CTRL 0x114
#define VJ_MPEG_TRIGGER 0x118
#define VJ_MPEG_STATUS 0x11c
#define VJ_MPEG_VLD_ADDR 0x128
#define VJ_MPEG_VLD_OFFSET 0x12c
#define VJ_MPEG_VLD_LEN 0x130
#define VJ_MPEG_VLD_END 0x134
#define VJ_MPEG_IQMINPUT 0x180
#define VJ_MPEG_JPEG_SIZE 0x1b8
#define VJ_MPEG_JPEG_RES_INT 0x1c0
#define VJ_MPEG_ROT_LUMA 0x1cc
#define VJ_MPEG_ROT_CHROMA 0x1d0
#define VJ_MPEG_SD_ROT_DBLK_CTL 0x1d4
#define VJ_MPEG_SRAM_RW_OFFSET 0x1e0
#define VJ_MPEG_SRAM_RW_DATA 0x1e4
#define VJ_TRIG_JPEG (3u << 24)
#define VJ_TRIG_HW_JPEG_VLD 0x0e
#define VJ_CTRL_FORCE_420 (1u << 18)
#define VJ_BIT (512u * 1024u)	/* kamera karesi en cok (1280x720 ~60-200 KB) */
#define VJ_W 1280
#define VJ_H 736

static volatile uint32_t *vj_R;
static int vj_fd = -1, vj_ion = -1, vj_bfd = -1, vj_ofd = -1, vj_req, vj_acik;
static uint8_t *vj_bm, *vj_om;
static uint32_t vj_badr, vj_oadr;
static size_t vj_osz;
static unsigned vj_aw, vj_ah;	/* son karenin 16'ya hizali boyutu (NV12 adimi) */
static char vj_hata[96];
static int16_t vj_trv[256], vj_tgu[256], vj_tgv[256], vj_tbu[256];
static uint16_t vj_tr5[1024], vj_tg6[1024], vj_tb5[1024];
static uint8_t vj_kirp[1024];	/* v2: 8 bit kirpma (XRGB8888) */
static int vj_g2d = -1, vj_hd_fd[3] = { -1, -1, -1 }, vj_g2d_uv;	/* v3: G2D; uv 0 = V1U1V0U0 (wifi-ekran), 1 = U1V1U0V0 */
static uint8_t *vj_hd[3];
static uint32_t vj_hd_adr[3];
static size_t vj_hd_n;

static void vj_w(unsigned o, uint32_t v) { vj_R[o / 4] = v; }
static uint32_t vj_r(unsigned o) { return vj_R[o / 4]; }
static double vj_ms(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }

/* ------------------------------------------------------------ wifi-ekran v23'ten (scripts/647 cikarir) */
struct vj_jpg {
	unsigned w, h, ncomp, hs[3], vs[3], qsel[3], dcsel[3], acsel[3], dri;
	const uint8_t *qt[4]; unsigned qlen[4];		/* 64 (8 bit) ya da 128 (16 bit) */
	const uint8_t *ht[2][4]; unsigned hlen[2][4];	/* [sinif dc=0/ac=1][id]: 16 sayi + semboller */
	size_t ecs;					/* entropi kodlu verinin dosyadaki ofseti */
};

static int vj_jpeg_ayristir(const uint8_t *d, size_t n, struct vj_jpg *j)
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
			if (len < 17 || s[0] != 8) return -10;	/* v23: 3 bilesen = 17 bayt */
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
				if (s + 1 + l > e) return -15;	/* v23: tablo bolum disina tasmasin */
				j->qt[tq] = s + 1; j->qlen[tq] = l; s += 1 + l;
			}
		} else if (mk == 0xc4) {			/* DHT */
			while (s < e) {
				unsigned tc = s[0] >> 4, th = s[0] & 3, cnt = 0;
				if (s + 17 > e) return -13;	/* v23 */
				for (int i = 0; i < 16; i++) cnt += s[1 + i];
				if (tc > 1 || cnt > 256 || s + 17 + cnt > e) return -13;
				j->ht[tc][th] = s + 1; j->hlen[tc][th] = 16 + cnt; s += 17 + cnt;
			}
		} else if (mk == 0xdd) {
			if (len < 4) return -17;	/* 614: 2 bayt tasma */
			j->dri = s[0] << 8 | s[1];
		} else if (mk == 0xda) {			/* SOS */
			if (!sof || len < 12 || s[0] != 3) return -14;
			for (unsigned i = 0; i < 3; i++) {
				j->dcsel[i] = s[2 + 2 * i] >> 4; j->acsel[i] = s[2 + 2 * i] & 15;
				if (j->dcsel[i] > 3 || j->acsel[i] > 3) return -16;	/* v23: ht[][4] */
			}
			j->ecs = (size_t)(e - d);
			return 0;
		}
		p += 2 + len;
	}
	return -4;
}

/* ------------------------------------------------------------ VE tablolari */
static void vj_huff_baslik(const uint8_t *t)
{
	uint16_t sc[16]; uint8_t of[16]; unsigned cnt = 0, code = 0;
	for (int i = 0; i < 16; i++) { of[i] = (uint8_t)cnt; sc[i] = (uint16_t)code; cnt += t[i]; code += t[i]; code *= 2; }
	for (int i = 15; i >= 0 && !t[i]; i--) sc[i] = 0xffff;
	for (int i = 0; i < 8; i++) vj_w(VJ_MPEG_SRAM_RW_DATA, (uint32_t)sc[2 * i] | (uint32_t)sc[2 * i + 1] << 16);
	for (int i = 0; i < 4; i++) vj_w(VJ_MPEG_SRAM_RW_DATA, of[4 * i] | of[4 * i + 1] << 8 | of[4 * i + 2] << 16 | (uint32_t)of[4 * i + 3] << 24);
	for (int i = 0; i < 4; i++) vj_w(VJ_MPEG_SRAM_RW_DATA, 0);
}

static int vj_huff_tablolar(const struct vj_jpg *j)
{
	const uint8_t *t[4]; unsigned l[4];
	t[0] = j->ht[0][j->dcsel[0]]; l[0] = j->hlen[0][j->dcsel[0]];
	t[1] = j->ht[1][j->acsel[0]]; l[1] = j->hlen[1][j->acsel[0]];
	t[2] = j->ht[0][j->dcsel[1]]; l[2] = j->hlen[0][j->dcsel[1]];
	t[3] = j->ht[1][j->acsel[1]]; l[3] = j->hlen[1][j->acsel[1]];
	for (int i = 0; i < 4; i++) if (!t[i]) return -1;
	vj_w(VJ_MPEG_SRAM_RW_OFFSET, 0);
	for (int i = 0; i < 4; i++) vj_huff_baslik(t[i]);
	for (int i = 0; i < 192; i++) vj_w(VJ_MPEG_SRAM_RW_DATA, 0);
	for (int i = 0; i < 4; i++) {
		unsigned n = l[i] - 16, words = (n + 3) / 4;
		const uint8_t *s = t[i] + 16;
		for (unsigned k = 0; k < words; k++) {
			uint32_t v = 0;
			for (unsigned b = 0; b < 4; b++) if (4 * k + b < n) v |= (uint32_t)s[4 * k + b] << (8 * b);
			vj_w(VJ_MPEG_SRAM_RW_DATA, v);
		}
		for (unsigned k = words; k < 64; k++) vj_w(VJ_MPEG_SRAM_RW_DATA, 0);
	}
	return 0;
}

static void vj_niceleme(const uint8_t *q, unsigned len, uint32_t bayrak)
{
	for (unsigned i = 0; i < 64; i++) {
		uint32_t v = len > 64 ? (uint32_t)(q[2 * i] << 8 | q[2 * i + 1]) : q[i];
		vj_w(VJ_MPEG_IQMINPUT, ((i & 63) << 8) | (v & 0xff) | bayrak);
	}
}

/* ------------------------------------------------------------ donusum tablolari (wifi-ekran ve_baslat ile ayni) */
static void vj_tablolar(void)
{
	if (vj_tr5[1023]) return;
	for (int i = 0; i < 256; i++) {
		vj_trv[i] = (int16_t)((1436 * (i - 128)) >> 10);	/* 1.402 */
		vj_tgu[i] = (int16_t)((352 * (i - 128)) >> 10);	/* 0.344 */
		vj_tgv[i] = (int16_t)((731 * (i - 128)) >> 10);	/* 0.714 */
		vj_tbu[i] = (int16_t)((1815 * (i - 128)) >> 10);	/* 1.772 */
	}
	for (int i = 0; i < 1024; i++) {
		int v = i - 384;
		uint8_t k = (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v);
		vj_kirp[i] = k;
		vj_tr5[i] = (uint16_t)((k >> 3) << 11);
		vj_tg6[i] = (uint16_t)((k >> 2) << 5);
		vj_tb5[i] = (uint16_t)(k >> 3);
	}
}

static int vj_ion_ayir(size_t n, uint32_t mask, uint8_t **map, uint32_t *adr, int *fdp, uint32_t bayrak)
{
	struct vj_ion_alloc a = { .len = n, .heap_id_mask = mask, .flags = bayrak };
	if (ioctl(vj_ion, VJ_ION_IOC_ALLOC, &a)) return -1;
	*fdp = (int)a.fd;
	*map = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, (int)a.fd, 0);
	if (*map == MAP_FAILED) { *map = NULL; return -2; }
	struct vj_dma_buf_param p = { .fd = (int)a.fd };
	if (ioctl(vj_fd, VJ_MAP_DMA_BUF, &p)) { munmap(*map, n); *map = NULL; return -3; }
	*adr = p.phy_addr;
	return 0;
}

static void vj_kapat(void)
{
	struct vj_dma_buf_param p;
	for (int i = 0; i < 3; i++) {	/* v3: G2D hedefleri (UNMAP ENGINE_REL'den once) */
		if (vj_hd[i]) { p = (struct vj_dma_buf_param){ .fd = vj_hd_fd[i] }; ioctl(vj_fd, VJ_UNMAP_DMA_BUF, &p); munmap(vj_hd[i], vj_hd_n); vj_hd[i] = NULL; }
		if (vj_hd_fd[i] >= 0) { close(vj_hd_fd[i]); vj_hd_fd[i] = -1; }
	}
	if (vj_g2d >= 0) { close(vj_g2d); vj_g2d = -1; }
	if (vj_R) { vj_w(VJ_MODE, 7); munmap((void *)vj_R, 4096); vj_R = NULL; }
	if (vj_bm) { p = (struct vj_dma_buf_param){ .fd = vj_bfd }; ioctl(vj_fd, VJ_UNMAP_DMA_BUF, &p); munmap(vj_bm, VJ_BIT); vj_bm = NULL; }
	if (vj_om) { p = (struct vj_dma_buf_param){ .fd = vj_ofd }; ioctl(vj_fd, VJ_UNMAP_DMA_BUF, &p); munmap(vj_om, vj_osz); vj_om = NULL; }
	if (vj_bfd >= 0) { close(vj_bfd); vj_bfd = -1; }
	if (vj_ofd >= 0) { close(vj_ofd); vj_ofd = -1; }
	if (vj_req) { ioctl(vj_fd, VJ_ENGINE_REL, 0); vj_req = 0; }
	if (vj_fd >= 0) { close(vj_fd); vj_fd = -1; }
	if (vj_ion >= 0) { close(vj_ion); vj_ion = -1; }
	vj_acik = 0;
}

static int vj_ac(void)
{
	if (vj_acik) return 0;
	vj_tablolar();
	vj_fd = open("/dev/cedar_dev", O_RDWR | O_CLOEXEC);
	vj_ion = open("/dev/ion", O_RDWR | O_CLOEXEC);
	if (vj_fd < 0 || vj_ion < 0) { snprintf(vj_hata, sizeof vj_hata, "/dev/cedar_dev ya da /dev/ion yok"); vj_kapat(); return -1; }
	struct vj_ion_heap_data hd[16];
	memset(hd, 0, sizeof hd);
	struct vj_ion_heap_query q = { .cnt = 16, .heaps = (uintptr_t)hd };
	uint32_t mask = 0;
	if (!ioctl(vj_ion, VJ_ION_IOC_HEAP_QUERY, &q))
		for (unsigned i = 0; i < q.cnt && i < 16; i++)
			if (hd[i].type == 2) { mask = 1u << hd[i].heap_id; break; }
	if (ioctl(vj_fd, VJ_ENGINE_REQ, 0)) { snprintf(vj_hata, sizeof vj_hata, "ENGINE_REQ basarisiz"); vj_kapat(); return -1; }
	vj_req = 1;
	vj_osz = (VJ_W * VJ_H * 3 / 2 + 4095) & ~4095u;
	if (!mask || vj_ion_ayir(VJ_BIT, mask, &vj_bm, &vj_badr, &vj_bfd, 0) ||
	    vj_ion_ayir(vj_osz, mask, &vj_om, &vj_oadr, &vj_ofd, 1u /* ION_FLAG_CACHED */)) {
		snprintf(vj_hata, sizeof vj_hata, "ION ayirma/esleme basarisiz");
		vj_kapat();
		return -1;
	}
	vj_R = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, vj_fd, 0);
	if (vj_R == MAP_FAILED) { vj_R = NULL; snprintf(vj_hata, sizeof vj_hata, "yazmac eslemesi basarisiz"); vj_kapat(); return -1; }
	struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
	if (ioctl(vj_ofd, DMA_BUF_IOCTL_SYNC, &sy)) { snprintf(vj_hata, sizeof vj_hata, "DMA-BUF senkron yok"); vj_kapat(); return -1; }
	sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
	ioctl(vj_ofd, DMA_BUF_IOCTL_SYNC, &sy);
	vj_acik = 1;
	return 0;
}

/* 0 tamam (NV12 vj_om'da), -1 bicim/boyut (bu kare yazilim), -2 donanim (cagiran vj_kapat + yazilim) */
static int vj_coz(const uint8_t *d, size_t n, unsigned *ow, unsigned *oh)
{
	struct vj_jpg j;
	if (!vj_acik || n > VJ_BIT || vj_jpeg_ayristir(d, n, &j)) return -1;
	unsigned idx = j.hs[0] << 20 | j.vs[0] << 16 | j.hs[1] << 12 | j.vs[1] << 8 | j.hs[2] << 4 | j.vs[2];
	uint32_t ss;
	switch (idx) {
	case 0x221111: ss = 0u << 27; break;
	case 0x211111: ss = 2u << 27; break;
	case 0x121111: ss = 4u << 27; break;
	default: return -1;	/* 4:4:4 vb.: wifi-ekran gibi yazilim */
	}
	if (!j.qt[j.qsel[0]] || !j.qt[j.qsel[1]]) return -1;
	unsigned AW = (j.w + 15) & ~15u, AH = (j.h + 15) & ~15u;
	if (!j.w || !j.h || AW > VJ_W || AH > VJ_H) return -1;
	size_t ysz = (size_t)AW * AH, csz = ysz / 2;
	memcpy(vj_bm, d, n);
	vj_w(VJ_MODE, 0x00130000);
	vj_w(VJ_PRIMARY_OUT_FMT, (4u << 4) | 4u);
	vj_w(VJ_PRIMARY_CHROMA_BUF_LEN, (uint32_t)(csz / 2));
	vj_w(VJ_CHROMA_BUF_LEN, ((uint32_t)(csz / 2) & 0x0fffffff) | (1u << 29) | (1u << 30));
	vj_w(VJ_PRIMARY_FB_LINE_STRIDE, AW | (AW / 2) << 16);
	vj_w(VJ_SECONDARY_FB_LINE_STRIDE, AW | (AW / 2) << 16);
	vj_w(VJ_MPEG_TRIGGER, VJ_TRIG_JPEG | ss);
	vj_w(VJ_MPEG_JPEG_RES_INT, j.dri);
	vj_w(VJ_MPEG_JPEG_SIZE, (((j.w + 8 * j.hs[0] - 1) / (8 * j.hs[0]) - 1) & 0x7ff) |
			       ((((j.h + 8 * j.vs[0] - 1) / (8 * j.vs[0]) - 1) & 0x7ff) << 16));
	vj_niceleme(j.qt[j.qsel[0]], j.qlen[j.qsel[0]], 1u << 14);
	vj_niceleme(j.qt[j.qsel[1]], j.qlen[j.qsel[1]], 0);
	if (vj_huff_tablolar(&j)) { vj_w(VJ_MODE, 7); return -1; }
	vj_w(VJ_MPEG_ROT_LUMA, vj_oadr);
	vj_w(VJ_MPEG_ROT_CHROMA, vj_oadr + (uint32_t)ysz);
	vj_w(VJ_MPEG_SD_ROT_DBLK_CTL, 0);
	vj_w(VJ_MPEG_VLD_OFFSET, (uint32_t)(8 * j.ecs));
	vj_w(VJ_MPEG_VLD_LEN, (uint32_t)(n * 8));
	vj_w(VJ_MPEG_VLD_ADDR, (vj_badr & 0x0ffffff0u) | (vj_badr >> 28) | (1u << 28) | (1u << 29) | (1u << 30));
	vj_w(VJ_MPEG_VLD_END, vj_badr + (uint32_t)n);
	vj_w(VJ_MPEG_STATUS, 0x7);
	vj_w(VJ_MPEG_CTRL, ss ? VJ_CTRL_FORCE_420 : 0);	/* kesme YOK */
	double t1 = vj_ms();
	vj_w(VJ_MPEG_TRIGGER, VJ_TRIG_HW_JPEG_VLD | VJ_TRIG_JPEG | ss);
	uint32_t st = 0;
	while (vj_ms() - t1 < 300 && !((st = vj_r(VJ_MPEG_STATUS)) & 0x7))
		;
	vj_w(VJ_MPEG_STATUS, 0x7);
	vj_w(VJ_MODE, 7);
	if (!(st & 0x7)) { ioctl(vj_fd, VJ_RESET, 0); snprintf(vj_hata, sizeof vj_hata, "zaman asimi (VE resetlendi)"); return -2; }
	if (!(st & 1)) { snprintf(vj_hata, sizeof vj_hata, "durum 0x%08x", st); return -1; }
	vj_aw = AW;
	vj_ah = AH;
	*ow = j.w;
	*oh = j.h;
	return 0;
}

/* son kareyi (sw x sh) dw x dh RGB565'e: en yakin komsu, NV12 (U, V); okuma DMA-BUF senkron arasinda */
static int vj_rgb565(uint16_t *dst, int dw, int dh, unsigned sw, unsigned sh)
{
	static uint16_t sx[1024];
	if (dw <= 0 || dh <= 0 || dw > 1024 || !sw || !sh || sw > vj_aw || sh > vj_ah) return -1;
	vj_tablolar();
	for (int x = 0; x < dw; x++) sx[x] = (uint16_t)(((2u * (unsigned)x + 1) * sw) / (2u * (unsigned)dw));
	struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
	if (vj_ofd >= 0) ioctl(vj_ofd, DMA_BUF_IOCTL_SYNC, &sy);
	const uint8_t *UVd = vj_om + (size_t)vj_aw * vj_ah;
	for (int y = 0; y < dh; y++) {
		unsigned syy = ((2u * (unsigned)y + 1) * sh) / (2u * (unsigned)dh);
		const uint8_t *Y = vj_om + (size_t)syy * vj_aw, *UV = UVd + (size_t)(syy / 2) * vj_aw;
		uint16_t *o = dst + (size_t)y * (unsigned)dw;
		for (int x = 0; x < dw; x++) {
			unsigned s = sx[x], c = s & ~1u;
			int u = UV[c], v = UV[c + 1], yy = Y[s] + 384;
			o[x] = (uint16_t)(vj_tr5[yy + vj_trv[v]] | vj_tg6[yy - vj_tgu[u] - vj_tgv[v]] | vj_tb5[yy + vj_tbu[u]]);
		}
	}
	sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
	if (vj_ofd >= 0) ioctl(vj_ofd, DMA_BUF_IOCTL_SYNC, &sy);
	return 0;
}

/* v2: son kareyi dw x dh XRGB8888'e (ekran bicimi; LVGL donusumsuz kopyalar), vj_rgb565 ile ayni ornekleme */
static int vj_xrgb8888(uint32_t *dst, int dw, int dh, unsigned sw, unsigned sh)
{
	static uint16_t sx[1024];
	if (dw <= 0 || dh <= 0 || dw > 1024 || !sw || !sh || sw > vj_aw || sh > vj_ah) return -1;
	vj_tablolar();
	for (int x = 0; x < dw; x++) sx[x] = (uint16_t)(((2u * (unsigned)x + 1) * sw) / (2u * (unsigned)dw));
	struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
	if (vj_ofd >= 0) ioctl(vj_ofd, DMA_BUF_IOCTL_SYNC, &sy);
	const uint8_t *UVd = vj_om + (size_t)vj_aw * vj_ah;
	for (int y = 0; y < dh; y++) {
		unsigned syy = ((2u * (unsigned)y + 1) * sh) / (2u * (unsigned)dh);
		const uint8_t *Y = vj_om + (size_t)syy * vj_aw, *UV = UVd + (size_t)(syy / 2) * vj_aw;
		uint32_t *o = dst + (size_t)y * (unsigned)dw;
		for (int x = 0; x < dw; x++) {
			unsigned s = sx[x], c = s & ~1u;
			int u = UV[c], v = UV[c + 1], yy = Y[s] + 384;
			o[x] = 0xFF000000u | (uint32_t)vj_kirp[yy + vj_trv[v]] << 16 | (uint32_t)vj_kirp[yy - vj_tgu[u] - vj_tgv[v]] << 8 |
			       vj_kirp[yy + vj_tbu[u]];
		}
	}
	sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
	if (vj_ofd >= 0) ioctl(vj_ofd, DMA_BUF_IOCTL_SYNC, &sy);
	return 0;
}

/* v3: G2D hedefi: adet (1..3) x n bayt ION (sistem yigini, onbellekli) + IOMMU adresi; vj_ac'tan sonra (ENGINE_REQ
 * acik). Panel bunlari canli goruntunun KENDI tamponlari olarak kullanir (ek bellek yok; 3 x 1.8 MB ayri tampon OOM'du). */
static int vj_g2d_ac(size_t n, int adet)
{
	if (vj_hd[0]) return 0;
	if (adet < 1 || adet > 3) return -1;
	if (!vj_acik) return -1;
	vj_g2d = open("/dev/g2d", O_RDWR | O_CLOEXEC);
	if (vj_g2d < 0) { snprintf(vj_hata, sizeof vj_hata, "/dev/g2d yok"); return -1; }
	struct vj_ion_heap_data hd[16];
	memset(hd, 0, sizeof hd);
	struct vj_ion_heap_query q = { .cnt = 16, .heaps = (uintptr_t)hd };
	uint32_t mask = 0;
	if (!ioctl(vj_ion, VJ_ION_IOC_HEAP_QUERY, &q))
		for (unsigned i = 0; i < q.cnt && i < 16; i++)
			if (hd[i].type == 0) mask = 1u << hd[i].heap_id;	/* sistem yigini (panel pg_hazirla gibi; CMA'ya dokunmaz) */
	vj_hd_n = n;
	for (int i = 0; i < adet; i++)
		if (!mask || vj_ion_ayir(n, mask, &vj_hd[i], &vj_hd_adr[i], &vj_hd_fd[i], 1u /* ION_FLAG_CACHED */)) {
			snprintf(vj_hata, sizeof vj_hata, "G2D hedef ION ayirma basarisiz");
			for (int k = 0; k <= i; k++) {
				struct vj_dma_buf_param p = { .fd = vj_hd_fd[k] };
				if (vj_hd[k]) { ioctl(vj_fd, VJ_UNMAP_DMA_BUF, &p); munmap(vj_hd[k], n); vj_hd[k] = NULL; }
				if (vj_hd_fd[k] >= 0) { close(vj_hd_fd[k]); vj_hd_fd[k] = -1; }
			}
			close(vj_g2d);
			vj_g2d = -1;
			return -1;
		}
	return 0;
}

/* v3: son VE karesini (sw x sh, NV12) G2D ile vj_hd[i]'ye dw x dh ARGB8888 (alfa 0xff) olarak olcekle + donustur.
 * Alanlar wifi-ekran v23 dg_ciz_g2d_k ile ayni. Sonra DMA-BUF senkron: CPU/LVGL taze veriyi okusun. 0/-1 */
static int vj_g2d_cevir(int i, int dw, int dh, unsigned sw, unsigned sh)
{
	if (vj_g2d < 0 || i < 0 || i > 2 || !vj_hd[i] || (size_t)dw * (size_t)dh * 4 > vj_hd_n || !sw || !sh) return -1;
	g2d_blt_h b;
	memset(&b, 0, sizeof b);
	b.flag_h = G2D_BLT_NONE_H;
	b.src_image_h.format = vj_g2d_uv ? G2D_FORMAT_YUV420UVC_U1V1U0V0 : G2D_FORMAT_YUV420UVC_V1U1V0U0;
	b.src_image_h.laddr[0] = vj_oadr;
	b.src_image_h.laddr[1] = vj_oadr + vj_aw * vj_ah;
	b.src_image_h.width = vj_aw;
	b.src_image_h.height = vj_ah;
	b.src_image_h.clip_rect = (g2d_rect){ 0, 0, sw, sh };
	b.src_image_h.alpha = 0xff;
	b.src_image_h.mode = G2D_PIXEL_ALPHA;
	b.src_image_h.gamut = G2D_BT601;
	b.src_image_h.color_range = COLOR_RANGE_0_255;
	b.src_image_h.use_phy_addr = 1;
	b.dst_image_h.format = G2D_FORMAT_ARGB8888;
	b.dst_image_h.laddr[0] = vj_hd_adr[i];
	b.dst_image_h.width = (unsigned)dw;
	b.dst_image_h.height = (unsigned)dh;
	b.dst_image_h.clip_rect = (g2d_rect){ 0, 0, (unsigned)dw, (unsigned)dh };
	b.dst_image_h.alpha = 0xff;
	b.dst_image_h.mode = G2D_PIXEL_ALPHA;
	b.dst_image_h.gamut = G2D_BT601;
	b.dst_image_h.color_range = COLOR_RANGE_0_255;
	b.dst_image_h.use_phy_addr = 1;
	if (ioctl(vj_g2d, G2D_CMD_BITBLT_H, &b)) { snprintf(vj_hata, sizeof vj_hata, "G2D_CMD_BITBLT_H basarisiz"); return -1; }
	struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };	/* onbellegi gecersiz kil: G2D DRAM'a yazdi */
	ioctl(vj_hd_fd[i], DMA_BUF_IOCTL_SYNC, &sy);
	sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
	ioctl(vj_hd_fd[i], DMA_BUF_IOCTL_SYNC, &sy);
	return 0;
}

/* v3: iki XRGB8888 tamponun ortalama mutlak farki (R/G/B ortalamasi, 8 bit) */
static double vj_fark_x(const uint32_t *a, const uint32_t *b, size_t n)
{
	unsigned long long t = 0;
	for (size_t i = 0; i < n; i++)
		for (int s = 0; s < 24; s += 8) {
			int d = (int)((a[i] >> s) & 255) - (int)((b[i] >> s) & 255);
			t += (unsigned)(d < 0 ? -d : d);
		}
	return n ? (double)t / (3.0 * (double)n) : 0;
}

/* iki RGB565 tamponun ortalama mutlak farki (8 bit olcekte, R/G/B ortalamasi) */
static double vj_fark(const uint16_t *a, const uint16_t *b, size_t n)
{
	unsigned long long t = 0;
	for (size_t i = 0; i < n; i++) {
		int dr = (a[i] >> 11) - (b[i] >> 11), dg = ((a[i] >> 5) & 63) - ((b[i] >> 5) & 63), db = (a[i] & 31) - (b[i] & 31);
		t += (unsigned)((dr < 0 ? -dr : dr) * 8 + (dg < 0 ? -dg : dg) * 4 + (db < 0 ? -db : db) * 8);
	}
	return n ? (double)t / (3.0 * (double)n) : 0;
}
#endif
