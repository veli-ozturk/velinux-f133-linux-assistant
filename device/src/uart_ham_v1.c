/* uart_ham v1 (12.10.2026, Claude): F133 UART'ini /dev/mem ile dogrudan surer (DTS'te etkin olmayan UART5 icin).
 * Amac: alınan.bin Melis'inin (sesin calistigi surum) MCU trafigini gonderdigi UART5 = PB4 (TX) / PB5 (RX) hattini sinamak.
 *   uart-ham yokla                     : PB5'i giris yapip pull-down/pull-up ile okur (karsi taraf suruyor mu?)
 *   uart-ham dinle <sn>                : UART5 115200 8N1 ac, yalniz dinle, gelen baytlari hex yaz
 *   uart-ham gonder <sn> <hex> [hex..] : her arguman bir cerceve govdesi "ID DATA.." (ornek "a4" "a7 0f 00");
 *                                        02 80 ID LEN DATA XOR olarak gonderilir, araya 0.6 sn; sonra <sn> dinler
 * UART5 taban 0x02501400, CCU UART_BGR 0x0200190C (bit5 kapi, bit21 reset), PB4/PB5 mux 7. Yalniz bu pinlere dokunur.
 * Yasak ID'ler (C4/C6/START_UPD/AT#UP) gonderilmez. */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

static volatile uint32_t *pio, *ccu, *u;
#define R(b, o) (*(volatile uint32_t *)((volatile uint8_t *)(b) + (o)))

static double mono(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec / 1e9;
}

static void *esle(int fd, unsigned long adr)
{
	void *p = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, adr & ~0xfffUL);
	if (p == MAP_FAILED) { perror("mmap"); exit(1); }
	return (uint8_t *)p + (adr & 0xfff);
}

static void pin_mux(int pin, unsigned mux)	/* PB pin */
{
	unsigned reg = 0x30 + (pin / 8) * 4, sh = (pin % 8) * 4;
	R(pio, reg) = (R(pio, reg) & ~(0xfu << sh)) | (mux << sh);
}

static void pin_pull(int pin, unsigned pull)
{
	unsigned reg = 0x54 + (pin / 16) * 4, sh = (pin % 16) * 2;
	R(pio, reg) = (R(pio, reg) & ~(3u << sh)) | (pull << sh);
}

static int pin_oku(int pin)
{
	return (R(pio, 0x40) >> pin) & 1;
}

static void uart_ac(void)
{
	R(ccu, 0) |= (1u << 5) | (1u << 21);	/* UART5 kapi + reset birak */
	usleep(1000);
	pin_mux(4, 7);
	pin_mux(5, 7);
	pin_pull(5, 1);
	R(u, 0x0C) = 0x83;	/* LCR: DLAB, 8N1 */
	R(u, 0x00) = 13;	/* 24 MHz / (16*13) = 115384 */
	R(u, 0x04) = 0;
	R(u, 0x0C) = 0x03;
	R(u, 0x08) = 0x07;	/* FIFO ac + temizle */
	R(u, 0x04) = 0;	/* IER kapali: yoklama */
	usleep(2000);
	while (R(u, 0x14) & 1)
		(void)R(u, 0x00);
}

static void dinle(double sn)
{
	double son = mono() + sn, t0 = mono();
	int n = 0;
	while (mono() < son) {
		if (R(u, 0x14) & 1) {
			if (n % 16 == 0) printf("\n[%6.3f]", mono() - t0);
			printf(" %02x", R(u, 0x00) & 0xff);
			n++;
		} else
			usleep(200);
	}
	printf("\nRX toplam %d bayt\n", n);
}

static void yaz_bayt(uint8_t b)
{
	while (!(R(u, 0x14) & 0x20))	/* THRE */
		;
	R(u, 0x00) = b;
}

static int gonder(const char *s)
{
	uint8_t d[64];
	int n = 0;
	while (*s && n < 64) {
		while (*s == ' ') s++;
		if (!s[0] || !s[1]) break;
		char t[3] = { s[0], s[1], 0 };
		d[n++] = (uint8_t)strtoul(t, NULL, 16);
		s += 2;
	}
	if (!n || d[0] == 0xC4 || d[0] == 0xC6) { fprintf(stderr, "gecersiz/yasak\n"); return -1; }
	uint8_t f[70] = { 0x02, 0x80, d[0], (uint8_t)(n - 1) };
	memcpy(f + 4, d + 1, (size_t)(n - 1));
	uint8_t x = 0;
	for (int i = 1; i < n + 3; i++) x ^= f[i];
	f[n + 3] = x;
	printf("\nTX");
	for (int i = 0; i < n + 4; i++) { printf(" %02x", f[i]); yaz_bayt(f[i]); }
	fflush(stdout);
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) { fputs("kullanim: uart-ham yokla | dinle <sn> | gonder <sn> <hex>...\n", stderr); return 2; }
	int fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) { perror("/dev/mem"); return 1; }
	pio = esle(fd, 0x02000000);
	ccu = esle(fd, 0x0200190C);
	u = esle(fd, 0x02501400);
	setvbuf(stdout, NULL, _IONBF, 0);
	if (!strcmp(argv[1], "yokla")) {
		uint32_t c0 = R(pio, 0x30), p0 = R(pio, 0x54);
		pin_mux(5, 0);
		pin_pull(5, 2);
		usleep(5000);
		int asagi = pin_oku(5);
		pin_pull(5, 1);
		usleep(5000);
		int yukari = pin_oku(5);
		pin_mux(4, 0);
		pin_pull(4, 2);
		usleep(5000);
		int pb4a = pin_oku(4);
		pin_pull(4, 1);
		usleep(5000);
		int pb4y = pin_oku(4);
		R(pio, 0x30) = c0;
		R(pio, 0x54) = p0;
		printf("PB5 pull-down=%d pull-up=%d -> %s\n", asagi, yukari,
		       asagi && yukari ? "KARSI TARAF YUKSEK SURUYOR (UART TX bosta olabilir)" :
		       !asagi && !yukari ? "karsi taraf dusuk suruyor" : "bos/surulmuyor");
		printf("PB4 pull-down=%d pull-up=%d -> %s\n", pb4a, pb4y,
		       pb4a && pb4y ? "yuksek suruluyor" : !pb4a && !pb4y ? "dusuk suruluyor" : "bos/surulmuyor");
		return 0;
	}
	uart_ac();
	if (!strcmp(argv[1], "dinle") && argc > 2) {
		dinle(atof(argv[2]));
	} else if (!strcmp(argv[1], "gonder") && argc > 3) {
		for (int i = 3; i < argc; i++) {
			gonder(argv[i]);
			dinle(0.6);
		}
		dinle(atof(argv[2]));
	}
	return 0;
}
