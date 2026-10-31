/*
 * VeLinux KOLE acilis ekrani: /usr/share/velinux/acilis.rle (VLR2) -> /dev/fb0 (statik, bagimsiz).
 * Bicim (scripts/32_gen_logo_v4.py):
 *   "VLR2" | u16 W,H (mantiksal yatay 1600x600) | u16 BW,BH | BW*BH*3 zemin RGB
 *   | u16 cx,cy,cw,ch (logo kutusu) | 256*3 palet RGB | (sayi u8, indeks u8)* kutu icin satir sirali RLE
 * Zemin cift dogrusal buyutulur (Python ile ayni tamsayi formulu), logo kutusu ustune yazilir.
 * Mantiksal (x,y) -> fiziksel fb (xres-1-y, x) (fbcon=rotate:1 yonu, panel ile ayni). 32 bpp XRGB.
 * tty1 grafik kipine alinir: fbcon imlec/yazi cizip gorseli bozmasin (panel ayni seyi yapar).
 */
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/kd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static unsigned u16(const uint8_t *p) { return p[0] | p[1] << 8; }

static void axis(unsigned i, unsigned ns, unsigned nb, unsigned *i0, unsigned *i1, unsigned *f)
{
	long num = (long)((2 * i + 1) * ns * 256) / (long)(2 * nb) - 128;
	if (num < 0)
		num = 0;
	*i0 = num >> 8;
	*f = num & 255;
	if (*i0 >= ns - 1) {
		*i0 = ns - 1;
		*f = 0;
	}
	*i1 = *i0 + 1 < ns ? *i0 + 1 : ns - 1;
}

static uint8_t *fb;
static unsigned fb_line, fb_xres, fb_yres;

static void put(unsigned x, unsigned y, uint32_t c)
{
	/* mantiksal (x,y) -> fiziksel (xres-1-y, x) */
	if (y >= fb_xres || x >= fb_yres)
		return;
	((uint32_t *)(fb + (size_t)x * fb_line))[fb_xres - 1 - y] = c;
}

int main(int argc, char **argv)
{
	const char *path = argc > 1 ? argv[1] : "/usr/share/velinux/acilis.rle";
	int in = open(path, O_RDONLY);
	struct stat st;
	if (in < 0 || fstat(in, &st) || st.st_size < 16) {
		perror(path);
		return 1;
	}
	uint8_t *d = malloc(st.st_size);
	if (!d || read(in, d, st.st_size) != st.st_size || memcmp(d, "VLR2", 4)) {
		fprintf(stderr, "acilis: gecersiz dosya\n");
		return 1;
	}
	close(in);
	unsigned W = u16(d + 4), H = u16(d + 6), BW = u16(d + 8), BH = u16(d + 10);
	const uint8_t *bg = d + 12;
	const uint8_t *q = bg + BW * BH * 3;
	unsigned cx = u16(q), cy = u16(q + 2), cw = u16(q + 4), ch = u16(q + 6);
	const uint8_t *pal = q + 8, *p = pal + 768, *e = d + st.st_size;
	if (p > e) {
		fprintf(stderr, "acilis: kisa dosya\n");
		return 1;
	}

	int tty = open("/dev/tty1", O_RDWR);
	if (tty >= 0) {
		ioctl(tty, KDSETMODE, KD_GRAPHICS);
		close(tty);
	}
	int fd = open("/dev/fb0", O_RDWR);
	struct fb_var_screeninfo v;
	struct fb_fix_screeninfo f;
	if (fd < 0 || ioctl(fd, FBIOGET_VSCREENINFO, &v) || ioctl(fd, FBIOGET_FSCREENINFO, &f) || v.bits_per_pixel != 32) {
		fprintf(stderr, "acilis: fb0 uygun degil\n");
		return 1;
	}
	fb_line = f.line_length;
	fb_xres = v.xres;
	fb_yres = v.yres;
	fb = mmap(NULL, f.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fb == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	if (v.yoffset) {
		v.yoffset = 0;
		ioctl(fd, FBIOPAN_DISPLAY, &v);
	}

	/* 1) zemin: cift dogrusal */
	unsigned *xa = malloc(W * 3 * sizeof(unsigned));
	for (unsigned x = 0; x < W; x++)
		axis(x, BW, W, &xa[x * 3], &xa[x * 3 + 1], &xa[x * 3 + 2]);
	for (unsigned y = 0; y < H; y++) {
		unsigned y0, y1, fy;
		axis(y, BH, H, &y0, &y1, &fy);
		for (unsigned x = 0; x < W; x++) {
			unsigned x0 = xa[x * 3], x1 = xa[x * 3 + 1], fx = xa[x * 3 + 2];
			const uint8_t *c00 = bg + (y0 * BW + x0) * 3, *c10 = bg + (y0 * BW + x1) * 3;
			const uint8_t *c01 = bg + (y1 * BW + x0) * 3, *c11 = bg + (y1 * BW + x1) * 3;
			uint32_t c = 0xFF000000u;
			for (int k = 0; k < 3; k++) {
				unsigned t = ((c00[k] * (256 - fx) + c10[k] * fx) * (256 - fy) +
					      (c01[k] * (256 - fx) + c11[k] * fx) * fy) >> 16;
				c |= t << (16 - 8 * k);
			}
			put(x, y, c);
		}
	}
	/* 2) logo kutusu: paletli RLE */
	uint32_t pc[256];
	for (int i = 0; i < 256; i++)
		pc[i] = 0xFF000000u | pal[i * 3] << 16 | pal[i * 3 + 1] << 8 | pal[i * 3 + 2];
	unsigned x = 0, y = 0;
	while (p + 1 < e && y < ch) {
		unsigned n = p[0];
		uint32_t c = pc[p[1]];
		p += 2;
		while (n-- && y < ch) {
			put(cx + x, cy + y, c);
			if (++x == cw) {
				x = 0;
				y++;
			}
		}
	}
	munmap(fb, f.smem_len);
	close(fd);
	return 0;
}
