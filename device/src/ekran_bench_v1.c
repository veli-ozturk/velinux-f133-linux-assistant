/* ekran-bench v1: tam ekran (1600x600) kablosuz goruntu icin cihazda maliyet olcumu.
 * Ag, framebuffer, UART, SD, flash ERISIMI YOK; yalniz verilen JPEG dosyalarini okur ve sahte tamponlara yazar.
 *   ekran-bench <dosya.jpg> [...]
 * Olculenler: TJpgDec cozme (RGB565 tam kareye, hedef konumuna), 1600x600 RGB565 -> 600x1600 XRGB dondurerek
 * kopyalama (dogrudan blit; naif ve bloklu), mevcut panelin LVGL flush yolu (1600x60 XRGB serit -> dondurme),
 * 1.92 MB memcpy ve FNV (v3 dosya yolunun maliyeti). */
#define _POSIX_C_SOURCE 200809L
#include "tjpgd.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define FW 1600
#define FH 600
static uint8_t jpeg[1024 * 1024], pool[65536];
static uint16_t frame[FW * FH];
static uint32_t fake_fb[FH * FW];   /* fiziksel 600 genis x 1600 satir */
static uint32_t strip[FW * 60];
static uint8_t copy_dst[FW * FH * 2];

static double now_us(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec * 1e6 + t.tv_nsec / 1e3;
}

struct src { size_t size, off; int ox, oy; };

static size_t in_cb(JDEC *jd, uint8_t *buf, size_t n)
{
	struct src *s = jd->device;
	if (n > s->size - s->off)
		n = s->size - s->off;
	if (buf)
		memcpy(buf, jpeg + s->off, n);
	s->off += n;
	return n;
}

/* TJpgDec (LVGL kopyasi) B,G,R verir -> RGB565 (R yuksek bitlerde), satir isaretcisiyle */
static int out_cb(JDEC *jd, void *bitmap, JRECT *r)
{
	struct src *s = jd->device;
	const uint8_t *p = bitmap;
	unsigned w = r->right - r->left + 1;
	for (unsigned y = r->top; y <= r->bottom; y++) {
		uint16_t *d = frame + (size_t)(s->oy + y) * FW + s->ox + r->left;
		for (unsigned x = 0; x < w; x++, p += 3)
			d[x] = (uint16_t)(((p[2] & 0xf8) << 8) | ((p[1] & 0xfc) << 3) | (p[0] >> 3));
	}
	return 1;
}

static int decode(size_t n, int ox, int oy, unsigned *w, unsigned *h)
{
	struct src s = { .size = n, .ox = ox, .oy = oy };
	JDEC jd;
	int r = jd_prepare(&jd, in_cb, pool, sizeof pool, &s);
	if (r)
		return r;
	*w = jd.width;
	*h = jd.height;
	if (ox + jd.width > FW || oy + jd.height > FH)
		return 100;
	return jd_decomp(&jd, out_cb, 0);
}

static inline uint32_t c565(uint16_t p)
{
	uint32_t r = (p >> 11) & 31, g = (p >> 5) & 63, b = p & 31;
	return 0xFF000000u | ((r << 3 | r >> 2) << 16) | ((g << 2 | g >> 4) << 8) | (b << 3 | b >> 2);
}

/* mantiksal (x,y) -> fiziksel satir x, sutun (599-y); panel flush ile ayni yon */
static void blit_naive(void)
{
	for (int x = 0; x < FW; x++) {
		uint32_t *d = fake_fb + (size_t)x * FH;
		for (int y = 0; y < FH; y++)
			d[FH - 1 - y] = c565(frame[(size_t)y * FW + x]);
	}
}

static void blit_block(int x1, int y1, int x2, int y2)
{
	for (int bx = x1; bx <= x2; bx += 16)
		for (int by = y1; by <= y2; by += 64) {
			int ex = bx + 15 > x2 ? x2 : bx + 15, ey = by + 63 > y2 ? y2 : by + 63;
			for (int x = bx; x <= ex; x++) {
				uint32_t *d = fake_fb + (size_t)x * FH + (FH - 1);
				const uint16_t *s = frame + x;
				for (int y = by; y <= ey; y++)
					d[-y] = c565(s[(size_t)y * FW]);
			}
		}
}

static void flush_like_panel(void)
{
	for (int band = 0; band < FH; band += 60)
		for (int x = 0; x < FW; x++) {
			uint32_t *dst = fake_fb + (size_t)x * FH;
			const uint32_t *src = strip + x;
			for (int y = band; y < band + 60; y++)
				dst[FH - 1 - y] = src[(size_t)(y - band) * FW] | 0xFF000000u;
		}
}

#define TIME(label, reps, stmt) do { double t0 = now_us(), best = 1e18, sum = 0; \
	for (int i_ = 0; i_ < (reps); i_++) { double a = now_us(); stmt; double b = now_us() - a; sum += b; if (b < best) best = b; } \
	(void)t0; printf("%-34s en_iyi %8.0f us  ort %8.0f us\n", label, best, sum / (reps)); } while (0)

int main(int argc, char **argv)
{
	printf("ekran-bench v1 JD_FASTDECODE=%d JD_SZBUF=%d\n", JD_FASTDECODE, JD_SZBUF);
	for (int i = 1; i < argc; i++) {
		FILE *f = fopen(argv[i], "rb");
		if (!f) { perror(argv[i]); return 2; }
		size_t n = fread(jpeg, 1, sizeof jpeg, f);
		fclose(f);
		unsigned w = 0, h = 0;
		int r = decode(n, 0, 0, &w, &h);
		if (r) { printf("%s: hata %d\n", argv[i], r); continue; }
		char lab[96];
		snprintf(lab, sizeof lab, "coz %ux%u %zuKB", w, h, n / 1024);
		TIME(lab, 3, decode(n, 0, 0, &w, &h));
		printf("   ilk piksel 565=%04x  (%s)\n", frame[0], argv[i]);
	}
	TIME("blit naif 1600x600 565->fb", 3, blit_naive());
	TIME("blit bloklu 1600x600 565->fb", 3, blit_block(0, 0, FW - 1, FH - 1));
	TIME("blit bloklu 256x128 parca", 10, blit_block(100, 100, 355, 227));
	TIME("panel flush yolu (10 serit)", 3, flush_like_panel());
	TIME("memcpy 1.92 MB", 5, memcpy(copy_dst, frame, sizeof copy_dst));
	uint32_t hsh = 0;
	TIME("FNV 1.92 MB", 2, { uint32_t h = 2166136261u; const uint8_t *p = (const uint8_t *)frame;
		for (size_t k = 0; k < sizeof frame; k++) h = (h ^ p[k]) * 16777619u; hsh ^= h; });
	printf("ozet=%08x %08x\n", hsh, fake_fb[12345] ^ copy_dst[777]);
	return 0;
}
