/* g2d_prova_v2 (15.10.2026, Claude): v1 olcumu -> G2D dondurme birimi kaynak/hedefi AYNI kipte ister (ikisi fiziksel ya da
 * ikisi fd); fb dma-buf (FBIOGET_DMABUF) referans almadan fd verdigi icin KULLANILMAZ. Bu surum alicinin kanitli yolu:
 * ION (sistem yigini, onbellekli) -> VE MAP_DMA_BUF ile IOMMU adresi (once ENGINE_REQ, sonra MAP, sonra REL; sayacli),
 * CPU yazar -> DMA_BUF_IOCTL_SYNC(START|END WRITE) onbellek temizligi -> G2D ROT_90 (fiziksel) -> dikey fb.
 * Panel LVGL PARTIAL tamponlariyla ayni boy (1600x60). Ekran degismez: fb seridi ters dondurulup tampona konur, G2D ile
 * geri yazilir, fb seridi karsilastirilir. Panel (verilirse PID) olcum boyunca SIGSTOP, sonra SIGCONT.
 * Calistirma: g2d-prova-v2 [panel_pid] [yigin: sistem|ayrilmis] */
#include <errno.h>
#include <fcntl.h>
#include <linux/dma-buf.h>
#include <linux/fb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include "g2d_driver_tina.h"

struct ion_allocation_data { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
struct ion_heap_data { char name[32]; uint32_t type, heap_id, r0, r1, r2; };
struct ion_heap_query { uint32_t cnt, r0; uint64_t heaps; uint32_t r1, r2; };
#define ION_IOC_ALLOC _IOWR('I', 0, struct ion_allocation_data)
#define ION_IOC_HEAP_QUERY _IOWR('I', 8, struct ion_heap_query)
#define VE_ENGINE_REQ 0x206
#define VE_ENGINE_REL 0x207
#define VE_MAP_DMA_BUF 0x504
#define VE_UNMAP_DMA_BUF 0x505
struct dma_buf_param { int fd; unsigned int phy_addr; };

#define LW 1600
#define LH 600
#define SH 60

static double ms(const struct timespec *a)
{
	struct timespec b;
	clock_gettime(CLOCK_MONOTONIC, &b);
	return (b.tv_sec - a->tv_sec) * 1e3 + (b.tv_nsec - a->tv_nsec) / 1e6;
}

static int g2d, fb_fd, ve_fd, ion_fd;
static uint32_t fb_phys;

static int dondur(uint32_t src, int w, int h, int x, int y)
{
	g2d_blt_h b;
	memset(&b, 0, sizeof b);
	b.flag_h = G2D_ROT_90;
	b.src_image_h.format = G2D_FORMAT_ARGB8888;
	b.src_image_h.laddr[0] = src;
	b.src_image_h.use_phy_addr = 1;
	b.src_image_h.width = (unsigned)w;
	b.src_image_h.height = (unsigned)h;
	b.src_image_h.clip_rect = (g2d_rect){ 0, 0, (unsigned)w, (unsigned)h };
	b.src_image_h.alpha = 0xff;
	b.dst_image_h.format = G2D_FORMAT_ARGB8888;
	b.dst_image_h.laddr[0] = fb_phys;
	b.dst_image_h.use_phy_addr = 1;
	b.dst_image_h.width = LH;
	b.dst_image_h.height = LW;
	b.dst_image_h.clip_rect = (g2d_rect){ LH - y - h, x, (unsigned)h, (unsigned)w };
	b.dst_image_h.alpha = 0xff;
	return ioctl(g2d, G2D_CMD_BITBLT_H, &b);
}

static int esitle(int fd, int bas)
{
	struct dma_buf_sync s = { (bas ? DMA_BUF_SYNC_START : DMA_BUF_SYNC_END) | DMA_BUF_SYNC_WRITE };
	return ioctl(fd, DMA_BUF_IOCTL_SYNC, &s);
}

int main(int argc, char **argv)
{
	int panel = argc > 1 ? atoi(argv[1]) : 0;
	int ayrilmis = argc > 2 && !strcmp(argv[2], "ayrilmis");
	g2d = open("/dev/g2d", O_RDWR | O_CLOEXEC);
	ion_fd = open("/dev/ion", O_RDWR | O_CLOEXEC);
	fb_fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
	ve_fd = open("/dev/cedar_dev", O_RDWR | O_CLOEXEC);
	if (g2d < 0 || ion_fd < 0 || fb_fd < 0 || ve_fd < 0) { puts("ACILAMADI"); return 1; }
	struct fb_fix_screeninfo f;
	struct fb_var_screeninfo v;
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &f) || ioctl(fb_fd, FBIOGET_VSCREENINFO, &v) || v.xres != LH || v.yres != LW) return 1;
	fb_phys = (uint32_t)f.smem_start;
	struct ion_heap_data hd[16];
	memset(hd, 0, sizeof hd);
	struct ion_heap_query q = { .cnt = 16, .heaps = (uintptr_t)hd };
	uint32_t mask = 0;
	if (ioctl(ion_fd, ION_IOC_HEAP_QUERY, &q)) return 1;
	for (unsigned i = 0; i < q.cnt && i < 16; i++)
		if (hd[i].type == (ayrilmis ? 2u : 0u)) mask = 1u << hd[i].heap_id;
	if (!mask) { puts("yigin yok"); return 1; }
	size_t n = (size_t)LW * SH * 4;
	struct ion_allocation_data a = { .len = n, .heap_id_mask = mask, .flags = 1 /* ION_FLAG_CACHED */ };
	if (ioctl(ion_fd, ION_IOC_ALLOC, &a)) { perror("ION"); return 1; }
	int bfd = (int)a.fd;
	uint32_t *m = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, bfd, 0);
	if (m == MAP_FAILED) { perror("mmap"); return 1; }
	struct timespec t0;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	if (ioctl(ve_fd, VE_ENGINE_REQ, 0)) { perror("ENGINE_REQ"); return 1; }
	struct dma_buf_param p = { .fd = bfd };
	int mr = ioctl(ve_fd, VE_MAP_DMA_BUF, &p);
	ioctl(ve_fd, VE_ENGINE_REL, 0);
	if (mr) { perror("MAP_DMA_BUF"); return 1; }
	printf("yigin %s, tampon %zu B, IOMMU 0x%08x (esleme %.2f ms)\n", ayrilmis ? "ayrilmis" : "sistem", n, p.phy_addr, ms(&t0));
	uint32_t *fbm = mmap(NULL, (size_t)LW * LH * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if (fbm == MAP_FAILED) return 1;
	uint32_t *kopya = malloc((size_t)LW * LH * 4);
	if (!kopya) return 1;
	if (panel > 0) kill(panel, SIGSTOP);
	usleep(100000);
	memcpy(kopya, fbm, (size_t)LW * LH * 4);
	double t_sync = 0, t_g2d = 0, t_cpu = 0, enaz = 1e9;
	int hata = 0;
	for (int y0 = 0; y0 < LH; y0 += SH) {
		clock_gettime(CLOCK_MONOTONIC, &t0);
		hata |= esitle(bfd, 1);
		for (int y = 0; y < SH; y++)	/* mantiksal (x, y0+y) = fb(599-(y0+y), x) */
			for (int x = 0; x < LW; x++) m[y * LW + x] = kopya[x * LH + (LH - 1 - (y0 + y))];
		t_cpu += ms(&t0);
		clock_gettime(CLOCK_MONOTONIC, &t0);
		hata |= esitle(bfd, 0);
		t_sync += ms(&t0);
		clock_gettime(CLOCK_MONOTONIC, &t0);
		hata |= dondur(p.phy_addr, LW, SH, 0, y0);
		double d = ms(&t0);
		t_g2d += d;
		if (d < enaz) enaz = d;
	}
	long fark = 0;
	for (size_t i = 0; i < (size_t)LW * LH; i++) fark += fbm[i] != kopya[i];
	/* dogruluk 2: bilinen desenle bir serit, sonra geri al */
	esitle(bfd, 1);
	for (int y = 0; y < SH; y++)
		for (int x = 0; x < LW; x++) m[y * LW + x] = 0xFF000000u | (uint32_t)(x * 7 + y * 131);
	esitle(bfd, 0);
	hata |= dondur(p.phy_addr, LW, SH, 0, 300);
	long desen = 0;
	for (int y = 0; y < SH; y++)
		for (int x = 0; x < LW; x++) desen += fbm[x * LH + (LH - 1 - (300 + y))] != (0xFF000000u | (uint32_t)(x * 7 + y * 131));
	memcpy(fbm, kopya, (size_t)LW * LH * 4);	/* ekrani geri koy */
	if (panel > 0) kill(panel, SIGCONT);
	printf("tam ekran (10 serit): esitleme %.2f ms, G2D %.2f ms (serit en az %.2f ms), CPU doldurma %.1f ms; ioctl %s\n",
	       t_sync, t_g2d, enaz, t_cpu, hata ? "HATA" : "ok");
	printf("dogruluk: geri dondurmede fark %ld piksel, desen seridinde fark %ld piksel\n", fark, desen);
	p.fd = bfd;
	ioctl(ve_fd, VE_UNMAP_DMA_BUF, &p);
	munmap(m, n);
	close(bfd);
	return hata || fark || desen;
}
