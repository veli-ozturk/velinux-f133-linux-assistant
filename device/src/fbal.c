/* fbal: /dev/fb0'i mmap ile (panelin kullandigi write-combine eslemesiyle, ekran motorunun gordugu DRAM) okuyup
 * stdout'a yazar. 'cat /dev/fb0' cekirdegin ayri eslemesinden okudugu icin D-cache acikken yaniltici olabilir. */
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
int main(void)
{
	int fd = open("/dev/fb0", O_RDONLY);
	struct fb_fix_screeninfo f;
	struct fb_var_screeninfo v;
	if (fd < 0 || ioctl(fd, FBIOGET_FSCREENINFO, &f) || ioctl(fd, FBIOGET_VSCREENINFO, &v))
		return 1;
	size_t n = (size_t)f.line_length * v.yres;
	unsigned char *p = mmap(NULL, f.smem_len, PROT_READ, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED)
		return 2;
	static unsigned char buf[65536];
	for (size_t o = 0; o < n; o += sizeof buf) {
		size_t k = n - o < sizeof buf ? n - o : sizeof buf;
		for (size_t i = 0; i < k; i++)
			buf[i] = p[o + i];
		if (write(1, buf, k) != (ssize_t)k)
			return 3;
	}
	return 0;
}
