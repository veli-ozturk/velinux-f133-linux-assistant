/*
 * mcu: arac MCU'su / Bluetrum BT yongasi ile UART1 (ttyS1, 115200 8N1) konusur. Protokol: linux-build/docs/mcu-bt-protokol.md
 *   cerceve: 02 | yon (80 istek/cevap, 81 MCU olayi) | ID | LEN | DATA[LEN] | XOR(yon..DATA)
 *   BT AT# komutlari: 02 80 D4 LEN "AT#..\r\n" ; BT cevaplari 02 81 D4 icinde ASCII, \r\n ile bolunur.
 * Port: varsayilan /dev/ttyS4 (uart4 PE4 TX / PE5 RX; uart1/uart3 bu kartta bos). MCU_TTY ile degisir.
 * El sikisma (A4/A0/B7/C3/BA) VARSAYILAN OLARAK GONDERILMEZ: 11.10.2026'da kartin gucunu kestirdi. D4 (AT#) icin gerekmiyor.
 * MCU_ELSIKISMA=1 ile zorlanir.
 * Kullanim:
 *   mcu dinle [sn]            yalniz dinle, cerceveleri yazdir (hicbir sey gondermez)
 *   mcu baslat [sn]           el sikisma (Melis gunlugundeki guvenli sira: A4, A0, B7 02, C3, BA) + dinle
 *   mcu ad <ad> [sn]          el sikisma, BT "IS" (hazir) gelince / yoksa 3 sn sonra AT#MM<ad>; "MM<ad>" cevabini bekle
 *   mcu at <AT#..> [sn]       el sikisma + tek AT# komutu (yalniz bilinen guvenli komutlar: MM, MN, CY, ... ; UP YASAK)
 * YASAK (gonderilmez): C4/C6 ve "START_UPD" (MCU firmware), AT#UP (BT DFU), CF (kimlik dogrulama).
 */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

static int fd;
static double t0;

static double now(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int ac(const char *dev)
{
	fd = open(dev, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror(dev);
		return -1;
	}
	struct termios t;
	tcgetattr(fd, &t);
	cfmakeraw(&t);
	cfsetispeed(&t, B115200);
	cfsetospeed(&t, B115200);
	t.c_cflag |= CLOCAL | CREAD;
	t.c_cflag &= ~(CRTSCTS | CSTOPB | PARENB);
	t.c_cc[VMIN] = 0;
	t.c_cc[VTIME] = 0;
	tcsetattr(fd, TCSANOW, &t);
	tcflush(fd, TCIOFLUSH);
	return 0;
}

static void gonder(uint8_t id, const uint8_t *d, int n)
{
	uint8_t b[300];
	int k = 0;
	b[k++] = 0x02;
	b[k++] = 0x80;
	b[k++] = id;
	b[k++] = n;
	memcpy(b + k, d, n);
	k += n;
	uint8_t x = 0;
	for (int i = 1; i < k; i++)
		x ^= b[i];
	b[k++] = x;
	printf("[%6.2f] TX", now() - t0);
	for (int i = 0; i < k; i++)
		printf(" %02x", b[i]);
	printf("\n");
	fflush(stdout);
	if (write(fd, b, k) != k)
		perror("write");
}

static void at(const char *cmd)
{
	char s[260];
	int n = snprintf(s, sizeof s, "%s\r\n", cmd);
	if (!strncmp(cmd, "AT#UP", 5)) {
		fprintf(stderr, "AT#UP YASAK (BT DFU)\n");
		return;
	}
	gonder(0xD4, (const uint8_t *)s, n);
}

/* --- alici: cerceve ayristirici + BT satir toplayici --- */
static uint8_t rb[512];
static int rn;
static char btline[512];
static int btn;
static char son_bt[64][128];
static int son_n;

static void bt_bayt(uint8_t c)
{
	if (c == '\r' || c == '\n') {
		if (btn) {
			btline[btn] = 0;
			printf("[%6.2f] BT  \"%s\"\n", now() - t0, btline);
			if (son_n < 64)
				snprintf(son_bt[son_n++], 128, "%s", btline);
			btn = 0;
		}
		return;
	}
	if (btn < (int)sizeof btline - 1)
		btline[btn++] = c;
}

static void cerceve(const uint8_t *f, int len)
{
	uint8_t yon = f[1], id = f[2], n = f[3];
	printf("[%6.2f] RX %s id=%02x len=%u:", now() - t0, yon == 0x81 ? "olay " : "cevap", id, n);
	for (int i = 0; i < n; i++)
		printf(" %02x", f[4 + i]);
	int yaz = 1;
	for (int i = 0; i < n; i++)
		if (f[4 + i] < 0x20 && f[4 + i] != '\r' && f[4 + i] != '\n')
			yaz = 0;
	if (yaz && n) {
		printf("  \"");
		for (int i = 0; i < n; i++)
			putchar(f[4 + i] >= 0x20 ? f[4 + i] : '.');
		printf("\"");
	}
	printf("\n");
	if (id == 0xD4)
		for (int i = 0; i < n; i++)
			bt_bayt(f[4 + i]);
	fflush(stdout);
	(void)len;
}

static void isle(void)
{
	for (;;) {
		int s = 0;
		while (s < rn && rb[s] != 0x02)
			s++;
		if (s) {
			printf("[%6.2f] ?? %d bayt cerceve disi:", now() - t0, s);
			for (int i = 0; i < s && i < 32; i++)
				printf(" %02x", rb[i]);
			printf("\n");
			memmove(rb, rb + s, rn - s);
			rn -= s;
		}
		if (rn < 5)
			return;
		int n = rb[3], L = 5 + n;
		if (rn < L)
			return;
		uint8_t x = 0;
		for (int i = 1; i < 4 + n; i++)
			x ^= rb[i];
		if (x != rb[4 + n]) {
			printf("[%6.2f] !! XOR hatali (beklenen %02x, gelen %02x), 1 bayt atlaniyor\n", now() - t0, x, rb[4 + n]);
			memmove(rb, rb + 1, rn - 1);
			rn -= 1;
			continue;
		}
		cerceve(rb, L);
		memmove(rb, rb + L, rn - L);
		rn -= L;
	}
}

static void bekle(double sn)
{
	double son = now() + sn;
	while (now() < son) {
		struct pollfd p = {.fd = fd, .events = POLLIN};
		int ms = (int)((son - now()) * 1000);
		if (ms < 1)
			ms = 1;
		if (poll(&p, 1, ms) > 0) {
			int k = read(fd, rb + rn, sizeof rb - rn);
			if (k > 0) {
				rn += k;
				isle();
			}
			if (rn >= (int)sizeof rb)
				rn = 0;
		}
	}
}

static int bt_var(const char *pfx)
{
	for (int i = 0; i < son_n; i++)
		if (!strncmp(son_bt[i], pfx, strlen(pfx)))
			return 1;
	return 0;
}

static void el_sikis(void)
{
	if (!getenv("MCU_ELSIKISMA") || strcmp(getenv("MCU_ELSIKISMA"), "1")) {	/* varsayilan: ATLA */
		printf("(el sikisma atlandi; MCU_ELSIKISMA=1 ile gonderilir - DIKKAT: kartin gucunu kestirdi)\n");
		return;
	}
	static const uint8_t b7[] = {0x02};
	gonder(0xA4, NULL, 0);
	bekle(0.3);
	gonder(0xA0, NULL, 0);
	bekle(0.3);
	gonder(0xB7, b7, 1);
	bekle(0.3);
	gonder(0xC3, NULL, 0);
	bekle(0.3);
	gonder(0xBA, NULL, 0);
	bekle(0.3);
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "kullanim: mcu dinle|baslat [sn] | mcu ad <ad> [sn] | mcu at <AT#..> [sn]\n");
		return 2;
	}
	t0 = now();
	const char *tty = getenv("MCU_TTY") ? getenv("MCU_TTY") : "/dev/ttyS4";	/* uart4 = PE4/PE5 (DTS); MCU_TTY ile baska port */
	if (ac(tty))
		return 1;
	const char *k = argv[1];
	if (!strcmp(k, "dinle")) {
		bekle(argc > 2 ? atof(argv[2]) : 10);
	} else if (!strcmp(k, "baslat")) {
		el_sikis();
		bekle(argc > 2 ? atof(argv[2]) : 8);
	} else if (!strcmp(k, "ad") && argc > 2) {
		el_sikis();
		double son = now() + 3;
		while (now() < son && !bt_var("IS"))
			bekle(0.2);
		char c[200];
		snprintf(c, sizeof c, "AT#MM%s", argv[2]);
		at(c);
		son = now() + (argc > 3 ? atof(argv[3]) : 4);
		char beklenen[200];
		snprintf(beklenen, sizeof beklenen, "MM%s", argv[2]);
		while (now() < son && !bt_var(beklenen))
			bekle(0.2);
		printf(bt_var(beklenen) ? "SONUC: BT adi '%s' kabul edildi\n" : "SONUC: '%s' icin MM cevabi gelmedi\n", argv[2]);
		return bt_var(beklenen) ? 0 : 3;
	} else if (!strcmp(k, "at") && argc > 2) {
		el_sikis();
		at(argv[2]);
		bekle(argc > 3 ? atof(argv[3]) : 4);
	} else {
		fprintf(stderr, "bilinmeyen komut\n");
		return 2;
	}
	return 0;
}
