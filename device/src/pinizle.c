/*
 * pinizle <sn> [maske]: PE portunu (varsayilan PE4|PE5) PASIF izler: pinleri GIRIS yapar (cekme yok), DAT yazmacini
 * sikica okur, her degisimi 24 MHz zaman sayaciyla (rdtime) kaydeder. Hicbir pin surulmez. Sonda eski ayarlar geri yuklenir.
 * Cikti: kenar listesi "t_us maske_degeri" (stdout) + ozet (stderr). UART cozumu PC'de yapilir.
 */
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

static inline uint64_t rdtime(void)
{
	uint64_t t;
	asm volatile("rdtime %0" : "=r"(t));
	return t;
}

int main(int argc, char **argv)
{
	double sn = argc > 1 ? atof(argv[1]) : 5;
	uint32_t mask = argc > 2 ? strtoul(argv[2], 0, 0) : 0x30;
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("/dev/mem");
		return 1;
	}
	volatile uint8_t *pio = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x02000000);
	if (pio == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	volatile uint32_t *cfg0 = (volatile uint32_t *)(pio + 0xC0), *dat = (volatile uint32_t *)(pio + 0xD0);
	volatile uint32_t *pull0 = (volatile uint32_t *)(pio + 0xE4);
	uint32_t c0 = *cfg0, p0 = *pull0, c = c0, p = p0;
	for (int i = 0; i < 8; i++)
		if (mask & (1u << i)) {
			c &= ~(0xFu << (i * 4));	/* 0 = giris */
			p &= ~(3u << (i * 2));		/* cekme yok */
		}
	*cfg0 = c;
	*pull0 = p;
	usleep(1000);

	enum { MAX = 200000 };
	static uint64_t te[MAX];
	static uint32_t ve[MAX];
	int n = 0;
	uint64_t t0 = rdtime(), son = t0 + (uint64_t)(sn * 24e6);
	uint32_t v = *dat & mask;
	te[n] = t0;
	ve[n++] = v;
	uint64_t ornek = 0;
	for (;;) {
		uint32_t w = *dat & mask;
		ornek++;
		if (w != v) {
			if (n < MAX) {
				te[n] = rdtime();
				ve[n++] = w;
			}
			v = w;
		}
		if ((ornek & 255) == 0 && rdtime() > son)
			break;
	}
	uint64_t t1 = rdtime();
	*cfg0 = c0;
	*pull0 = p0;
	for (int i = 0; i < n; i++)
		printf("%.3f %x\n", (te[i] - t0) / 24.0, ve[i]);
	fprintf(stderr, "pinizle: %.1f sn, %llu ornek (%.2f Mornek/s, ~%.2f us/ornek), %d kenar, ilk deger 0x%x\n",
		(t1 - t0) / 24e6, (unsigned long long)ornek, ornek / ((t1 - t0) / 24.0), (t1 - t0) / 24.0 / ornek, n - 1, ve[0]);
	return 0;
}
