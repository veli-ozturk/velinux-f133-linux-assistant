/*
 * fbtest: /dev/fb0'a test deseni cizer.
 *   fbtest            -> 8 renk cubugu + beyaz cerceve + kose isaretleri (yon kontrolu)
 *   fbtest R G B      -> tum ekran tek renk (0-255)
 * Kose isaretleri: sol-ust KIRMIZI, sag-ust YESIL, sol-alt MAVI, sag-alt BEYAZ kare.
 */
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static struct fb_var_screeninfo v;
static struct fb_fix_screeninfo f;
static uint8_t *mem;

static void px(unsigned x, unsigned y, unsigned r, unsigned g, unsigned b)
{
	uint8_t *p;

	if (x >= v.xres || y >= v.yres)
		return;
	p = mem + (y + v.yoffset) * f.line_length + x * (v.bits_per_pixel / 8);
	if (v.bits_per_pixel == 32) {
		uint32_t c = (0xffu << 24) | (r << v.red.offset) | (g << v.green.offset) | (b << v.blue.offset);
		*(uint32_t *)p = c;
	} else if (v.bits_per_pixel == 16) {
		uint16_t c = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
		*(uint16_t *)p = c;
	}
}

static void rect(unsigned x0, unsigned y0, unsigned w, unsigned h, unsigned r, unsigned g, unsigned b)
{
	for (unsigned y = y0; y < y0 + h; y++)
		for (unsigned x = x0; x < x0 + w; x++)
			px(x, y, r, g, b);
}

int main(int argc, char **argv)
{
	static const unsigned bars[8][3] = {
		{255, 255, 255}, {255, 255, 0}, {0, 255, 255}, {0, 255, 0},
		{255, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 0, 0},
	};
	int fd = open("/dev/fb0", O_RDWR);

	if (fd < 0) {
		perror("/dev/fb0");
		return 1;
	}
	ioctl(fd, FBIOGET_VSCREENINFO, &v);
	ioctl(fd, FBIOGET_FSCREENINFO, &f);
	printf("fb0: %ux%u (sanal %ux%u) %u bpp, satir %u bayt, R%u G%u B%u, bellek %u bayt\n",
	       v.xres, v.yres, v.xres_virtual, v.yres_virtual, v.bits_per_pixel, f.line_length,
	       v.red.offset, v.green.offset, v.blue.offset, f.smem_len);
	mem = mmap(NULL, f.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (mem == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	if (argc == 4) {
		rect(0, 0, v.xres, v.yres, atoi(argv[1]), atoi(argv[2]), atoi(argv[3]));
		return 0;
	}
	/* Panel dikey (600x1600): cubuklar uzun kenar boyunca */
	for (unsigned i = 0; i < 8; i++) {
		unsigned y0 = i * v.yres / 8, y1 = (i + 1) * v.yres / 8;
		rect(0, y0, v.xres, y1 - y0, bars[i][0], bars[i][1], bars[i][2]);
	}
	rect(0, 0, v.xres, 4, 255, 255, 255);
	rect(0, v.yres - 4, v.xres, 4, 255, 255, 255);
	rect(0, 0, 4, v.yres, 255, 255, 255);
	rect(v.xres - 4, 0, 4, v.yres, 255, 255, 255);
	rect(10, 10, 60, 60, 255, 0, 0);
	rect(v.xres - 70, 10, 60, 60, 0, 255, 0);
	rect(10, v.yres - 70, 60, 60, 0, 0, 255);
	rect(v.xres - 70, v.yres - 70, 60, 60, 255, 255, 255);
	return 0;
}
