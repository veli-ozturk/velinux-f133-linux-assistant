/* fb_alfa_say_v1 (15.10.2026, Claude): fb0'da alfa != 0xFF piksel sayisi (panel 0.38 G2D yolu ham kopyalar; CPU yolu
 * 0xFF ile OR'luyordu). Yalniz okur. */
#include <fcntl.h>
#include <linux/fb.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
int main(void)
{
	int fd = open("/dev/fb0", O_RDONLY);
	struct fb_fix_screeninfo f;
	struct fb_var_screeninfo v;
	if (fd < 0 || ioctl(fd, FBIOGET_FSCREENINFO, &f) || ioctl(fd, FBIOGET_VSCREENINFO, &v)) return 1;
	size_t n = (size_t)f.line_length * v.yres;
	const uint32_t *m = mmap(NULL, n, PROT_READ, MAP_SHARED, fd, 0);
	if (m == MAP_FAILED) return 1;
	static uint32_t satir[4096];
	unsigned long sifir = 0, ara = 0, tam = 0;
	for (unsigned y = 0; y < v.yres; y++) {
		for (unsigned x = 0; x < v.xres; x++) satir[x] = m[(size_t)y * (f.line_length / 4) + x];
		for (unsigned x = 0; x < v.xres; x++) {
			unsigned a = satir[x] >> 24;
			if (a == 0xff) tam++; else if (a == 0) sifir++; else ara++;
		}
	}
	printf("alfa: 0xFF %lu, 0x00 %lu, ara %lu piksel\n", tam, sifir, ara);
	return 0;
}
