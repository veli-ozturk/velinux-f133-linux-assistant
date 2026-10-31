/* CF prova v1: kullanıcı 17.10.2026 explicit authorization.
 * Based on v3 heartbeat behavior. UART owner and SD baseline remain separate.
 * MCU_CF_PROVA=1 permits exactly ONE successful transmission of the recorded
 * 8-byte Melis challenge fa241b2c2e5f313e. No arbitrary CF or firmware updates.
 * Repeated or altered challenges are rejected. Default CF remains blocked.
 */
/* v3: non-BT B2 payload diagnostics (first/change only); v2 heartbeat/guards unchanged. SV is NOT a numeric volume field. */
/* mcu-servis v2 (12.10.2026, Claude): v1 canli: her B0 cevabi aninda yeni B2 getirdi (~500/sn ping-pong).
 * v2: ayni B2 koduna en sik MCU_KALP_ARALIK sn (varsayilan 2) cevap; 10 sn'de bir B2 kod ozeti yazilir.
 * --- v1 ---
 * mcu-servis v1 (12.10.2026, Claude): MCU UART'inin (ttyS4, 115200) TEK sahibi.
 *  1) KALP ATISI: MCU her "02 81 B2 n 00 XX.." olayina Melis gibi "02 80 B0 (n-1) XX.." ile cevaplanir
 *     (Melis gunlugu: Rx b2 06 00 ce 00 00 00 00 -> Tx b0 05 ce 00 00 00 00 fb, ~10 sn'de bir).
 *     12.10.2026 olcumu: A7 ile uyandirilan MCU, B2'lere cevap gelmeyince ~1 dk icinde karti yeniden baslatti;
 *     11.10'daki "el sikisma gucu kesti" olayinin da muhtemel nedeni budur.
 *  2) mcu-izle v1 gibi yalniz BT olay KODLARI (IA/IB/MA/MB...) tutulur, /tmp/bt-durum.json yazilir (panel uyumlu);
 *     telefon adi/adresi/sarki metni tutulmaz/yazilmaz.
 *  3) Komut FIFO'su /tmp/mcu-komut: her satir "ID HEX.." (ornek "A7 0f 00") -> cerceve gonderilir.
 *     C4/C6/CF/START_UPD/AT#UP YASAK; A4/A0/B7/C3/BA yalniz MCU_ELSIKISMA=1 ortaminda.
 * Kilit /tmp/mcu-izle.lock (mcu-izle ile ayni anda calismaz).
 *   mcu-servis [--seconds N] [--tty /dev/ttyS4] [--baslat "A7 0f 00;AA 14 00 .."] | --self-test */
#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define FIFO "/tmp/mcu-komut"

static volatile sig_atomic_t stop;
static int ufd = -1;
static uint8_t test_tx[64];
static size_t test_tx_n;
static struct {
	int connection, music;
	char music_code[3], last_code[3];
	unsigned long frames, bad_xor, events, tx, heartbeats, rejected;
	double last_event, last_music, last_heartbeat;
} state;
static uint8_t rx[512];
static size_t rx_n, line_n;
static char prefix[2];
static int line_overflow;

static double mono(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec / 1e9;
}

static void reset(void)
{
	memset(&state, 0, sizeof state);
	state.connection = state.music = -1;
	memcpy(state.music_code, "??", 3);
	memcpy(state.last_code, "??", 3);
	rx_n = line_n = 0;
	line_overflow = 0;
}

static unsigned cf_sent;

static int yasak(uint8_t id, const uint8_t *d, size_t n)
{
	if (id == 0xC4 || id == 0xC6)
		return 1;
	if (id == 0xCF) {
		static const uint8_t challenge[8] = {0xfa,0x24,0x1b,0x2c,0x2e,0x5f,0x31,0x3e};
		const char *permit = getenv("MCU_CF_PROVA");
		if (!permit || strcmp(permit, "1") || cf_sent || n != sizeof challenge ||
		    memcmp(d, challenge, sizeof challenge))
			return 1;
	}
	if ((id == 0xA4 || id == 0xA0 || id == 0xB7 || id == 0xC3 || id == 0xBA) &&
	    (!getenv("MCU_ELSIKISMA") || strcmp(getenv("MCU_ELSIKISMA"), "1")))
		return 1;
	if (memmem(d, n, "START_UPD", 9) || memmem(d, n, "AT#UP", 5))
		return 1;
	return 0;
}

/* tek cerceve gonder; test modunda (ufd<0) test_tx'e yazar */
static int gonder(uint8_t id, const uint8_t *d, size_t n, int kalp)
{
	if (n > 250)
		return -1;
	if (!kalp && yasak(id, d, n)) {
		state.rejected++;
		printf("RED %02x (yasak)\n", id);
		return -1;
	}
	uint8_t b[260];
	size_t k = 0;
	b[k++] = 0x02;
	b[k++] = 0x80;
	b[k++] = id;
	b[k++] = (uint8_t)n;
	memcpy(b + k, d, n);
	k += n;
	uint8_t x = 0;
	for (size_t i = 1; i < k; i++)
		x ^= b[i];
	b[k++] = x;
	if (ufd < 0) {
		memcpy(test_tx, b, k < sizeof test_tx ? k : sizeof test_tx);
		test_tx_n = k;
	} else if (write(ufd, b, k) != (ssize_t)k) {
		perror("uart write");
		return -1;
	}
	state.tx++;
	if (id == 0xCF) cf_sent++;
	if (!kalp || getenv("MCU_KALP_YAZ")) {
		printf("TX %.3f", mono());
		for (size_t i = 0; i < k && i < 24; i++)
			printf(" %02x", b[i]);
		printf("\n");
		fflush(stdout);
	}
	return 0;
}

static void event(const char code[3])
{
	double now = mono();
	memcpy(state.last_code, code, 3);
	state.last_event = now;
	state.events++;
	if (!strcmp(code, "IA")) {
		state.connection = 0;
		state.music = -1;
		state.last_music = 0;
		memcpy(state.music_code, "??", 3);
	} else if (!strcmp(code, "IB")) {
		state.connection = 1;
	} else if (!strcmp(code, "MA") || !strcmp(code, "MC") || !strcmp(code, "MB") || !strcmp(code, "MD") ||
		   !strcmp(code, "MY")) {
		state.music = !strcmp(code, "MB") ? 3 : (!strcmp(code, "MA") || !strcmp(code, "MC")) ? 2 : 0;
		memcpy(state.music_code, code, 3);
		state.last_music = now;
	}
	printf("BT %.3f %s\n", now, code);	/* satirin geri kalani asla yazilmaz */
	fflush(stdout);
}

static void bt_byte(uint8_t b)
{
	if (b == '\r' || b == '\n') {
		if (!line_overflow && line_n >= 2 && prefix[0] >= 'A' && prefix[0] <= 'Z' && prefix[1] >= 'A' &&
		    prefix[1] <= 'Z') {
			char code[3] = { prefix[0], prefix[1], 0 };
			event(code);
		}
		line_n = 0;
		line_overflow = 0;
	} else {
		if (line_n < 2)
			prefix[line_n] = (char)b;
		if (line_n < 512)
			line_n++;
		else
			line_overflow = 1;
	}
}

static void frame(const uint8_t *f, size_t n)
{
	uint8_t yon = f[1], id = f[2];
	const uint8_t *d = f + 4;
	if (id == 0xD4) {
		for (size_t i = 0; i < n; i++)
			bt_byte(d[i]);
		return;
	}
	if (yon == 0x81 && id == 0xB2 && n >= 2) {	/* kalp atisi sorusu -> B0 cevabi (kod basina sinirli) */
		static double son_cevap[256];
		static unsigned sayac[256];
		static double ozet_t;
		double now = mono(), aralik = getenv("MCU_KALP_ARALIK") ? atof(getenv("MCU_KALP_ARALIK")) : 2.0;
		sayac[d[1]]++;
		state.heartbeats++;
        /* Numeric MCU payload only: never BT text/phone/address/song data.
         * First payload or change per code, max16 bytes; unchanged heartbeats stay summarized. */
        static uint8_t diag[256][16], diag_n[256];
        size_t dn = n < 16 ? n : 16;
        unsigned code = d[1];
        if (diag_n[code] != dn || memcmp(diag[code], d, dn)) {
            diag_n[code] = (uint8_t)dn;
            memcpy(diag[code], d, dn);
            printf("B2 DATA %.3f code=%02x n=%zu:", now, code, n);
            for (size_t j = 0; j < dn; j++) printf(" %02x", d[j]);
            printf("\n"); fflush(stdout);
        }
		if (now - son_cevap[d[1]] >= aralik) {
			son_cevap[d[1]] = now;
			state.last_heartbeat = now;
			gonder(0xB0, d + 1, n - 1, 1);
		}
		if (now - ozet_t >= 10) {
			ozet_t = now;
			printf("B2 ozet %.1f:", now);
			for (int i = 0; i < 256; i++)
				if (sayac[i]) {
					printf(" %02x=%u", i, sayac[i]);
					sayac[i] = 0;
				}
			printf(" (tx=%lu)\n", state.tx);
			fflush(stdout);
		}
		return;
	}
	printf("RX %.3f %s %02x:", mono(), yon == 0x81 ? "olay" : "cevap", id);
	for (size_t i = 0; i < n && i < 16; i++)
		printf(" %02x", d[i]);
	printf("\n");
	fflush(stdout);
}

static void parse(void)
{
	while (rx_n) {
		if (rx[0] != 2 || (rx_n >= 2 && rx[1] != 0x80 && rx[1] != 0x81)) {
			memmove(rx, rx + 1, --rx_n);
			continue;
		}
		if (rx_n < 5)
			return;
		size_t n = rx[3], total = n + 5;
		if (rx_n < total)
			return;
		uint8_t sum = 0;
		for (size_t i = 1; i < n + 4; i++)
			sum ^= rx[i];
		if (sum != rx[n + 4]) {
			state.bad_xor++;
			line_n = 0;
			line_overflow = 0;
			memmove(rx, rx + 1, --rx_n);
			continue;
		}
		state.frames++;
		frame(rx, n);
		rx_n -= total;
		memmove(rx, rx + total, rx_n);
	}
}

static void feed(const uint8_t *data, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (rx_n == sizeof rx) {
			rx_n = line_n = 0;
			line_overflow = 0;
		}
		rx[rx_n++] = data[i];
		parse();
	}
}

/* "A7 0f 00" -> cerceve; 0 tamam */
static int komut_satiri(const char *s)
{
	uint8_t d[250];
	size_t n = 0;
	char *e;
	while (*s == ' ')
		s++;
	unsigned long id = strtoul(s, &e, 16);
	if (e == s || id > 0xFF)
		return -1;
	s = e;
	while (*s && n < sizeof d) {
		while (*s == ' ' || *s == ',' || *s == ':')
			s++;
		if (!s[0] || s[0] == '\n' || s[0] == '\r')
			break;
		if (!s[1])
			return -1;
		char t[3] = { s[0], s[1], 0 };
		d[n++] = (uint8_t)strtoul(t, &e, 16);
		if (*e)
			return -1;
		s += 2;
	}
	return gonder((uint8_t)id, d, n, 0);
}

static int format_state(char *out, size_t n, int running)
{
	return snprintf(out, n,
		"{\"schema\":1,\"running\":%s,\"pid\":%ld,\"heartbeat_mono\":%.3f,"
		"\"connection\":%d,\"music_state\":%d,\"music_code\":\"%s\","
		"\"last_code\":\"%s\",\"last_event_mono\":%.3f,\"last_music_mono\":%.3f,"
		"\"rx_frames\":%lu,\"bad_xor\":%lu,\"events\":%lu,\"tx_frames\":%lu,"
		"\"mcu_heartbeats\":%lu,\"last_mcu_heartbeat\":%.3f,\"rejected\":%lu,\"service\":\"mcu-servis-cf-v1\"}\n",
		running ? "true" : "false", (long)getpid(), mono(), state.connection, state.music, state.music_code,
		state.last_code, state.last_event, state.last_music, state.frames, state.bad_xor, state.events, state.tx,
		state.heartbeats, state.last_heartbeat, state.rejected);
}

static int save_state(int running)
{
	char data[900], temp[128];
	int n = format_state(data, sizeof data, running);
	if (n < 0 || (size_t)n >= sizeof data)
		return -1;
	snprintf(temp, sizeof temp, "/tmp/.bt-durum.%ld.tmp", (long)getpid());
	FILE *f = fopen(temp, "w");
	if (!f)
		return -1;
	int ok = fwrite(data, 1, n, f) == (size_t)n;
	if (fclose(f))
		ok = 0;
	if (ok && !rename(temp, "/tmp/bt-durum.json"))
		return 0;
	unlink(temp);
	return -1;
}

static void test_bytes(const uint8_t *b, size_t n)
{
	for (size_t i = 0; i < n; i++)
		feed(b + i, 1);
}

static int self_test(void)
{
	reset();
	/* Melis gunlugu satir 519/521: Rx 02 81 b2 06 00 ce 00 00 00 00 fb -> Tx 02 80 b0 05 ce 00 00 00 00 fb */
	const uint8_t b2[] = { 0x02, 0x81, 0xb2, 0x06, 0x00, 0xce, 0x00, 0x00, 0x00, 0x00, 0xfb };
	test_bytes(b2, sizeof b2);
	const uint8_t beklenen[] = { 0x02, 0x80, 0xb0, 0x05, 0xce, 0x00, 0x00, 0x00, 0x00, 0xfb };
	assert(state.heartbeats == 1 && test_tx_n == sizeof beklenen && !memcmp(test_tx, beklenen, sizeof beklenen));
    /* Identical immediate B2 must not start v1's 500/s ping-pong. */
    unsigned long once_tx = state.tx;
    test_bytes(b2, sizeof b2);
    assert(state.heartbeats == 2 && state.tx == once_tx);
	/* satir 408: b0 05 fe 00 00 05 00 ce  <- b2 06 00 fe 00 00 05 00 */
	const uint8_t b2b[] = { 0x02, 0x81, 0xb2, 0x06, 0x00, 0xfe, 0x00, 0x00, 0x05, 0x00, 0x00 };
	uint8_t c[sizeof b2b];
	memcpy(c, b2b, sizeof c);
	uint8_t x = 0;
	for (size_t i = 1; i < sizeof c - 1; i++)
		x ^= c[i];
	c[sizeof c - 1] = x;
	test_bytes(c, sizeof c);
	const uint8_t bek2[] = { 0x02, 0x80, 0xb0, 0x05, 0xfe, 0x00, 0x00, 0x05, 0x00, 0xce };
	assert(state.heartbeats == 3 && !memcmp(test_tx, bek2, sizeof bek2));
	/* Melis satir 338: Tx 02 80 a7 02 0f 00 2a */
	assert(komut_satiri("A7 0f 00") == 0);
	const uint8_t a7[] = { 0x02, 0x80, 0xa7, 0x02, 0x0f, 0x00, 0x2a };
	assert(test_tx_n == sizeof a7 && !memcmp(test_tx, a7, sizeof a7));
	/* satir 388: aa 0a 00 00 07 07 00 00 14 00 14 00 20 */
	assert(komut_satiri("AA 00 00 07 07 00 00 14 00 14 00") == 0 && test_tx[test_tx_n - 1] == 0x20);
	unsetenv("MCU_ELSIKISMA");
	assert(komut_satiri("C4 00") < 0 && komut_satiri("A4") < 0 && komut_satiri("CF 00") < 0);
	assert(komut_satiri("zz") < 0);
	unsetenv("MCU_CF_PROVA");
	assert(komut_satiri("CF fa 24 1b 2c 2e 5f 31 3e") < 0);
	setenv("MCU_CF_PROVA", "1", 1);
	assert(komut_satiri("CF fa 24") < 0);
	assert(komut_satiri("CF 00 24 1b 2c 2e 5f 31 3e") < 0);
	assert(komut_satiri("CF fa 24 1b 2c 2e 5f 31 3e") == 0);
	const uint8_t cf_expected[] = {2,0x80,0xcf,8,0xfa,0x24,0x1b,0x2c,0x2e,0x5f,0x31,0x3e,0xd0};
	assert(cf_sent == 1 && test_tx_n == sizeof cf_expected && !memcmp(test_tx, cf_expected, sizeof cf_expected));
	assert(komut_satiri("CF fa 24 1b 2c 2e 5f 31 3e") < 0);
	assert(komut_satiri("C4 00") < 0 && komut_satiri("C6 00") < 0);
	assert(komut_satiri("D4 41 54 23 55 50") < 0);
	unsetenv("MCU_CF_PROVA");
	puts("CF GATES PASS: default deny, exact payload/length, one shot, firmware deny; no UART opened");
	puts("SELF-TEST OK: B2->B0 kalp (Melis satir 519/521, 408), A7/AA cerceve, yasaklar; UART acilmadi");
	return 0;
}

static void on_signal(int sig)
{
	(void)sig;
	stop = 1;
}

int main(int argc, char **argv)
{
	const char *tty = "/dev/ttyS4", *baslat = NULL;
	double seconds = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--self-test"))
			return self_test();
		if (!strcmp(argv[i], "--seconds") && i + 1 < argc) {
			char *end;
			seconds = strtod(argv[++i], &end);
			if (*end || seconds < 0 || seconds > 864000)
				return 2;
		} else if (!strcmp(argv[i], "--tty") && i + 1 < argc) {
			tty = argv[++i];
		} else if (!strcmp(argv[i], "--baslat") && i + 1 < argc) {
			baslat = argv[++i];
		} else {
			fprintf(stderr, "kullanim: mcu-servis [--seconds N] [--tty /dev/ttyS4] [--baslat \"A7 0f 00;AA ..\"] | --self-test\n");
			return 2;
		}
	}
	int lock = open("/tmp/mcu-izle.lock", O_CREAT | O_RDWR, 0600);
	if (lock < 0 || flock(lock, LOCK_EX | LOCK_NB)) {
		perror("mcu kilidi (mcu-izle calisiyor mu?)");
		return 1;
	}
	ufd = open(tty, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (ufd < 0) {
		perror(tty);
		return 1;
	}
	struct termios t;
	if (tcgetattr(ufd, &t)) {
		perror("tcgetattr");
		return 1;
	}
	cfmakeraw(&t);
	cfsetispeed(&t, B115200);
	cfsetospeed(&t, B115200);
	t.c_cflag &= ~(CRTSCTS | CSTOPB | PARENB | CSIZE);
	t.c_cflag |= CLOCAL | CREAD | CS8;
	t.c_cc[VMIN] = t.c_cc[VTIME] = 0;
	if (tcsetattr(ufd, TCSANOW, &t)) {
		perror("tcsetattr");
		return 1;
	}
	unlink(FIFO);
	if (mkfifo(FIFO, 0620)) {
		perror(FIFO);
		return 1;
	}
	int kfd = open(FIFO, O_RDWR | O_NONBLOCK | O_CLOEXEC);
	reset();
	signal(SIGINT, on_signal);
	signal(SIGTERM, on_signal);
	signal(SIGPIPE, SIG_IGN);
	printf("mcu-servis CF prova v1: %s O_RDWR 115200; B2->B0 kalp cevabi (kod basina sinirli); komut FIFO %s\n", tty, FIFO);
	fflush(stdout);
	double started = mono(), saved = -1;
	if (baslat) {
		char kop[1024];
		snprintf(kop, sizeof kop, "%s", baslat);
		for (char *p = strtok(kop, ";"); p; p = strtok(NULL, ";"))
			if (komut_satiri(p))
				printf("baslat komutu reddedildi: %s\n", p);
	}
	char kbuf[512];
	size_t kn = 0;
	int result = 0;
	while (!stop && (!seconds || mono() - started < seconds)) {
		if (mono() - saved >= 1) {
			save_state(1);
			saved = mono();
		}
		struct pollfd p[2] = { { ufd, POLLIN, 0 }, { kfd, POLLIN, 0 } };
		int r = poll(p, kfd >= 0 ? 2 : 1, 200);
		if (r < 0 && errno != EINTR) {
			perror("poll");
			result = 1;
			break;
		}
		if (r > 0 && (p[0].revents & POLLIN)) {
			uint8_t b[256];
			ssize_t n = read(ufd, b, sizeof b);
			if (n > 0)
				feed(b, (size_t)n);
		}
		if (r > 0 && kfd >= 0 && (p[1].revents & POLLIN)) {
			ssize_t n = read(kfd, kbuf + kn, sizeof kbuf - 1 - kn);
			if (n > 0) {
				kn += (size_t)n;
				kbuf[kn] = 0;
				char *nl;
				while ((nl = strchr(kbuf, '\n'))) {
					*nl = 0;
					if (komut_satiri(kbuf))
						printf("komut reddedildi/gecersiz: %.40s\n", kbuf);
					size_t used = (size_t)(nl - kbuf) + 1;
					memmove(kbuf, nl + 1, kn - used + 1);
					kn -= used;
				}
				if (kn >= sizeof kbuf - 1)
					kn = 0;
			}
		}
	}
	save_state(0);
	close(ufd);
	unlink(FIFO);
	close(lock);
	printf("Bitti: rx=%lu kalp=%lu tx=%lu red=%lu\n", state.frames, state.heartbeats, state.tx, state.rejected);
	return result;
}
