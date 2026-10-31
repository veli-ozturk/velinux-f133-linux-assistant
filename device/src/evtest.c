/*
 * evtest (kucuk): bir input aygitinin adini, ABS araliklarini ve olaylarini yazdirir.
 *   evtest                    -> /dev/input/event* listesi
 *   evtest /dev/input/event0  -> olaylari izle (Ctrl-C ile cik)
 * Dokunmalar ekrana da cizilir (fb0 varsa): ham koordinat panel cozunurlugune olceklenir.
 */
#include <dirent.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

static const char *absname(int c)
{
	switch (c) {
	case ABS_X: return "ABS_X";
	case ABS_Y: return "ABS_Y";
	case ABS_MT_SLOT: return "MT_SLOT";
	case ABS_MT_POSITION_X: return "MT_X";
	case ABS_MT_POSITION_Y: return "MT_Y";
	case ABS_MT_TRACKING_ID: return "MT_ID";
	case ABS_MT_TOUCH_MAJOR: return "MT_MAJOR";
	case ABS_MT_WIDTH_MAJOR: return "MT_WIDTH";
	default: return NULL;
	}
}

static void list(void)
{
	DIR *d = opendir("/dev/input");
	struct dirent *e;
	char path[300], name[256];

	if (!d) {
		perror("/dev/input");
		return;
	}
	while ((e = readdir(d))) {
		int fd;

		if (strncmp(e->d_name, "event", 5))
			continue;
		snprintf(path, sizeof(path), "/dev/input/%s", e->d_name);
		fd = open(path, O_RDONLY);
		if (fd < 0)
			continue;
		name[0] = 0;
		ioctl(fd, EVIOCGNAME(sizeof(name)), name);
		printf("%s: %s\n", path, name);
		close(fd);
	}
	closedir(d);
}

int main(int argc, char **argv)
{
	struct input_event ev;
	struct input_absinfo ai;
	struct fb_var_screeninfo v;
	struct fb_fix_screeninfo f;
	uint8_t *fbm = NULL;
	int fd, fb, x = -1, y = -1, xmax = 1, ymax = 1;
	char name[256] = "";

	if (argc < 2) {
		list();
		return 0;
	}
	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror(argv[1]);
		return 1;
	}
	ioctl(fd, EVIOCGNAME(sizeof(name)), name);
	printf("Aygit: %s\n", name);
	for (int c = 0; c < ABS_CNT; c++) {
		if (!absname(c) || ioctl(fd, EVIOCGABS(c), &ai) < 0)
			continue;
		printf("  %-9s min %d max %d\n", absname(c), ai.minimum, ai.maximum);
		if (c == ABS_MT_POSITION_X)
			xmax = ai.maximum;
		if (c == ABS_MT_POSITION_Y)
			ymax = ai.maximum;
	}
	fb = open("/dev/fb0", O_RDWR);
	if (fb >= 0 && !ioctl(fb, FBIOGET_VSCREENINFO, &v) && !ioctl(fb, FBIOGET_FSCREENINFO, &f)) {
		fbm = mmap(NULL, f.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
		if (fbm == MAP_FAILED)
			fbm = NULL;
	}
	while (read(fd, &ev, sizeof(ev)) == sizeof(ev)) {
		if (ev.type == EV_ABS) {
			const char *n = absname(ev.code);

			printf("ABS %s %d\n", n ? n : "?", ev.value);
			if (ev.code == ABS_MT_POSITION_X || ev.code == ABS_X)
				x = ev.value;
			if (ev.code == ABS_MT_POSITION_Y || ev.code == ABS_Y)
				y = ev.value;
		} else if (ev.type == EV_KEY) {
			printf("KEY %d %d\n", ev.code, ev.value);
		} else if (ev.type == EV_SYN && x >= 0 && y >= 0) {
			printf("--- dokunma ham (%d,%d) / max (%d,%d)\n", x, y, xmax, ymax);
			if (fbm && v.bits_per_pixel == 32) {
				/* Ham araligi YATAY ekrana (fbcon=rotate:1) olcekle, 15x15 sari nokta.
				 * fb dikey (600x1600) ise mantiksal yatay nokta (LX,LY) -> fiziksel (xres-1-LY, LX). */
				int px, py;

				if (v.yres > v.xres) {
					int lx = (long)x * (v.yres - 1) / (xmax ? xmax : 1);
					int ly = (long)y * (v.xres - 1) / (ymax ? ymax : 1);

					px = v.xres - 1 - ly;
					py = lx;
				} else {
					px = (long)x * (v.xres - 1) / (xmax ? xmax : 1);
					py = (long)y * (v.yres - 1) / (ymax ? ymax : 1);
				}
				for (int dy = -7; dy <= 7; dy++)
					for (int dx = -7; dx <= 7; dx++) {
						int X = px + dx, Y = py + dy;

						if (X >= 0 && Y >= 0 && X < (int)v.xres && Y < (int)v.yres)
							*(uint32_t *)(fbm + Y * f.line_length + X * 4) = 0xffffff00;
					}
			}
		}
		fflush(stdout);
	}
	return 0;
}
