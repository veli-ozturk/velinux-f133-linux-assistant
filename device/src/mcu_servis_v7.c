/* v7 (24.10.2026, Claude; docs/denetim-2026-10-24-v3.md A7/A8/A9/A20, scripts/628):
 *  - A7: bastaki yarim cerceve 0.5 sn'de tamamlanmazsa 1 bayt atilir, yeniden cozulur (bozuk LEN baytinda v6 251
 *    bayt bekliyordu; arkadaki B2'ler cevapsiz kalip MCU karti sifirliyordu). Yalniz ana dongude denetlenir.
 *  - A8: setpriority(-10) + mlockall(MCL_CURRENT|MCL_FUTURE|MCL_ONFAULT) (kole_wdt_v2 gibi; hata yalniz gunluge).
 *  - A9: 511 bayti asan FIFO satiri satir sonuna kadar atilir (v6 kalanini yeni komut sayiyordu).
 *  - A20: D4 (BT tuneli) yalniz TEK tam "AT#XY..\r\n" komutu; harfler buyuk/kucuk duyarsiz (AT#UP yasak).
 *  Kalp cevabi, UART ayari, A3/A4/AA/B8 filtreleri ve bt-durum.json bicimi v6 ile ayni.
 * --- v6 ---
 * v6 (19.10.2026, Claude): A4 KORUMASI. AB5301_WIFI B8 isleyicisi (0x1001C776) alt komut DATA0=3'te DATA2 bit7'yi
 * kalici A4 anahtarina yazar (0x10015732). A4 != 0 iken timer 0x10002C86 PF3'u (amfi) surekli LOW tutar.
 * Melis "FM Transmit" = B8 03 82 80 .. -> A4=0x80 = hoparlor kalici sessiz; "Local horn" = B8 03 82 00 .. -> A4=0.
 * v6, B8 03 cercevesinde DATA2 bit7=1 (veya DATA2'si olmayan) olanlari reddeder; yalniz MCU_B8_A4=1 ortaminda gecirir.
 * Ayrinti docs/ab5301-A3-geri-donus-yontemi-20261019.md §4. Geri kalan her sey v5 ile ayni.
 * --- v5 ---
 * v5 (19.10.2026, Claude): A3 KORUMASI. Kartin AB5301_WIFI kodunda AA DATA7 bit0 kalici A3 anahtarina yazilir;
 * A3=0 iken timer 0x10002C86 PF3'u (amfi) surekli LOW tutar = hoparlor kalici sessiz
 * (docs/ab5301-wifi-A3-PF3-kapisi-20261019.md). v5, DATA7 bit0=0 (veya DATA7'si olmayan) AA cercevelerini
 * reddeder; yalniz MCU_AA_A3_SIFIR=1 ortaminda bilincli gecirir. Geri kalan her sey v3 ile ayni.
 * --- v3 ---
 * v3: non-BT B2 payload diagnostics (first/change only); v2 heartbeat/guards unchanged. SV is NOT a numeric volume field. */
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
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#define FIFO "/tmp/mcu-komut"
#ifndef MCL_ONFAULT
#define MCL_ONFAULT 4	/* Linux 4.4+ (v7) */
#endif

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
static double rx_bekle;	/* v7 (A7): bastaki yarim cercevenin ilk goruldugu an (0 = yok) */
static char kbuf[512];	/* v7 (A9): komut FIFO satir tamponu (v6'da main icindeydi) */
static size_t kn;
static int atla;	/* v7 (A9): tasan satirin sonuna kadar at */

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
	rx_bekle = 0;	/* v7 */
	kn = 0;
	atla = 0;
}

static int yasak(uint8_t id, const uint8_t *d, size_t n)
{
	if (id == 0xC4 || id == 0xC6 || id == 0xCF)
		return 1;
	if ((id == 0xA4 || id == 0xA0 || id == 0xB7 || id == 0xC3 || id == 0xBA) &&
	    (!getenv("MCU_ELSIKISMA") || strcmp(getenv("MCU_ELSIKISMA"), "1")))
		return 1;
	if (memmem(d, n, "START_UPD", 9) || memmem(d, n, "AT#UP", 5))
		return 1;
	/* v7 (A20): D4 = BT AT tuneli. Yalniz TEK tam komut "AT#XY..\r\n" (bolunmus ya da birlesik komut yok); komut
	 * harfleri buyuk/kucuk duyarsiz yasak listesiyle (UP = BT DFU) karsilastirilir. */
	if (id == 0xD4 && (n < 7 || memcmp(d, "AT#", 3) || d[n - 2] != '\r' || d[n - 1] != '\n' ||
			   memchr(d, '\r', n - 2) || memchr(d, '\n', n - 2) ||
			   (toupper(d[3]) == 'U' && toupper(d[4]) == 'P')))
		return 4;
	/* v5: AA DATA7 bit0 = A3 (amfi kapisi, kalici). 0 yazmak hoparloru kalici susturur. */
	if (id == 0xAA && (n < 8 || !(d[7] & 1)) &&
	    (!getenv("MCU_AA_A3_SIFIR") || strcmp(getenv("MCU_AA_A3_SIFIR"), "1")))
		return 2;
	/* v6: B8 03 DATA2 bit7 = A4 (FM Transmit, kalici). 1 yazmak hoparloru kalici susturur. */
	if (id == 0xB8 && n >= 1 && d[0] == 3 && (n < 3 || (d[2] & 0x80)) &&
	    (!getenv("MCU_B8_A4") || strcmp(getenv("MCU_B8_A4"), "1")))
		return 3;
	return 0;
}

/* tek cerceve gonder; test modunda (ufd<0) test_tx'e yazar */
static int gonder(uint8_t id, const uint8_t *d, size_t n, int kalp)
{
	if (n > 250)
		return -1;
	int y = kalp ? 0 : yasak(id, d, n);
	if (y) {
		state.rejected++;
		printf(y == 2 ? "RED %02x (A3 korumasi: DATA7 bit0=0 hoparloru kalici susturur)\n" :
		       y == 3 ? "RED %02x (A4 korumasi: B8 03 DATA2 bit7=1 FM Transmit hoparloru kalici susturur)\n" :
		       y == 4 ? "RED %02x (D4: tek tam AT# komutu degil ya da yasak AT#UP)\n" :
		       "RED %02x (yasak)\n", id);
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
			rx_bekle = 0;
			continue;
		}
		size_t n = rx_n >= 5 ? rx[3] : 0, total = n + 5;
		if (rx_n < 5 || rx_n < total) {
			if (!rx_bekle)	/* v7 (A7): yarim cerceve; zaman asimi yarim_zaman_asimi() */
				rx_bekle = mono();
			return;
		}
		rx_bekle = 0;
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
			rx_bekle = 0;	/* v7 */
		}
		rx[rx_n++] = data[i];
		parse();
	}
}

/* v7 (A7): bastaki yarim cerceve 0.5 sn'dir tamamlanmadiysa bozuktur (115200'de tam cerceve <25 ms): bir bayt at,
 * yeniden coz. Yalniz ana dongude, okunan parti islendikten sonra cagrilir (gecikmeli okumada saglam cerceve atilmasin). */
static void yarim_zaman_asimi(double now)
{
	if (!rx_n || !rx_bekle || now - rx_bekle <= 0.5)
		return;
	printf("UYARI %.3f yarim cerceve 0.5 sn'de tamamlanmadi (%zu bayt): 1 bayt atildi\n", now, rx_n);
	fflush(stdout);
	rx_bekle = 0;
	line_n = 0;
	line_overflow = 0;
	memmove(rx, rx + 1, --rx_n);
	parse();
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

/* FIFO'dan kbuf+kn'ye okunan n bayti isle. v7 (A9): 511 bayti asan satir satir sonuna kadar atilir (v6 kalanini yeni
 * komut sayiyordu: 100 harflik BT adi ID 0x05 + cop gonderdi). */
static void komut_veri(size_t n)
{
	kn += n;
	kbuf[kn] = 0;
	char *nl;
	while ((nl = strchr(kbuf, '\n'))) {
		*nl = 0;
		if (atla)
			atla = 0;	/* v7: tasan satirin sonu, calistirilmaz */
		else if (komut_satiri(kbuf))
			printf("komut reddedildi/gecersiz: %.40s\n", kbuf);
		size_t used = (size_t)(nl - kbuf) + 1;
		memmove(kbuf, nl + 1, kn - used + 1);
		kn -= used;
	}
	if (kn >= sizeof kbuf - 1) {
		if (!atla)
			printf("komut satiri %zu bayti asti, satir sonuna kadar atiliyor: %.40s\n", sizeof kbuf - 1, kbuf);
		atla = 1;
		kn = 0;
	}
}

static int format_state(char *out, size_t n, int running)
{
	return snprintf(out, n,
		"{\"schema\":1,\"running\":%s,\"pid\":%ld,\"heartbeat_mono\":%.3f,"
		"\"connection\":%d,\"music_state\":%d,\"music_code\":\"%s\","
		"\"last_code\":\"%s\",\"last_event_mono\":%.3f,\"last_music_mono\":%.3f,"
		"\"rx_frames\":%lu,\"bad_xor\":%lu,\"events\":%lu,\"tx_frames\":%lu,"
		"\"mcu_heartbeats\":%lu,\"last_mcu_heartbeat\":%.3f,\"rejected\":%lu,\"service\":\"mcu-servis-v3\"}\n",
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

/* v7 test: FIFO'ya s yazilmis gibi (read en cok sizeof kbuf - 1 - kn bayt verir) */
static void test_fifo(const char *s)
{
	size_t n = strlen(s);
	while (n) {
		size_t k = sizeof kbuf - 1 - kn < n ? sizeof kbuf - 1 - kn : n;
		memcpy(kbuf + kn, s, k);
		komut_veri(k);
		s += k;
		n -= k;
	}
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
	/* satir 388: aa 0a 00 00 07 07 00 00 14 00 14 00 20 -> v5: DATA7=0, A3'u sifirlar -> RED */
	unsetenv("MCU_AA_A3_SIFIR");
	unsigned long red0 = state.rejected, tx0 = state.tx;
	assert(komut_satiri("AA 00 00 07 07 00 00 14 00 14 00") < 0 && state.rejected == red0 + 1 && state.tx == tx0);
	assert(komut_satiri("AA 14 00 07 07 00 00 16") < 0);		/* DATA7 yok */
	assert(komut_satiri("AA 14 00 07 07 00 00 16 00 14 00") < 0);	/* 17.10 pozitif AA: yine DATA7=0 */
	/* DATA7 bit0=1: A3 korunur -> gecer */
	assert(komut_satiri("AA 14 00 07 07 00 00 16 01 14 00") == 0 && test_tx_n == 15 && test_tx[2] == 0xaa &&
	       test_tx[11] == 0x01);
	/* bilincli gecis: Melis satiri v3 ile ayni XOR'u uretir */
	setenv("MCU_AA_A3_SIFIR", "1", 1);
	assert(komut_satiri("AA 00 00 07 07 00 00 14 00 14 00") == 0 && test_tx[test_tx_n - 1] == 0x20);
	unsetenv("MCU_AA_A3_SIFIR");
	unsetenv("MCU_ELSIKISMA");
	/* v6: B8 03 -> A4. Melis/17.10 "Local horn" gecer (02 80 b8 07 03 82 00 00 04 00 01 bb) */
	unsetenv("MCU_B8_A4");
	assert(komut_satiri("B8 03 82 00 00 04 00 01") == 0 && test_tx_n == 12 && test_tx[2] == 0xb8 &&
	       test_tx[3] == 0x07 && test_tx[11] == 0xbb);
	red0 = state.rejected;
	tx0 = state.tx;
	assert(komut_satiri("B8 03 82 80 00 00 00 01") < 0 && state.rejected == red0 + 1 && state.tx == tx0); /* FM Transmit */
	assert(komut_satiri("B8 03 00 ff") < 0);		/* DATA2 bit7 */
	assert(komut_satiri("B8 03 82") < 0);		/* DATA2 yok */
	assert(komut_satiri("B8 03") < 0);
	assert(komut_satiri("B8 07 00 40") == 0);		/* baska alt komut: A4'e dokunmaz */
	assert(komut_satiri("B8 01 00 80") == 0);		/* DATA0!=3: bit7 serbest */
	/* bilincli gecis: Melis FM Transmit satiri (02 80 b8 07 03 82 80 00 00 00 01 3f) */
	setenv("MCU_B8_A4", "1", 1);
	assert(komut_satiri("B8 03 82 80 00 00 00 01") == 0 && test_tx_n == 12 && test_tx[11] == 0x3f);
	unsetenv("MCU_B8_A4");
	/* B8 korumasi A3 korumasini gevsetmez */
	assert(komut_satiri("AA 14 00 07 07 00 00 16 00 14 00") < 0);
	assert(komut_satiri("C4 00") < 0 && komut_satiri("A4") < 0 && komut_satiri("CF 00") < 0);
	assert(komut_satiri("zz") < 0);
	/* v7 (A20): D4 yalniz tek tam AT# komutu; AT#UP buyuk/kucuk harf duyarsiz yasak */
	red0 = state.rejected;
	tx0 = state.tx;
	assert(komut_satiri("D4 41 54 23 55") < 0);				/* "AT#U" (bolunmus: CR LF yok) */
	assert(komut_satiri("D4 50 0d 0a") < 0);				/* "P\r\n" (AT# yok) */
	assert(komut_satiri("D4 61 74 23 75 70 0d 0a") < 0);			/* "at#up\r\n" */
	assert(komut_satiri("D4 41 54 23 75 50 0d 0a") < 0);			/* "AT#uP\r\n" */
	assert(komut_satiri("D4 41 54 23 55 50 0d 0a") < 0);			/* "AT#UP\r\n" */
	assert(komut_satiri("D4 41 54 23 55 50 31 0d 0a") < 0);			/* "AT#UP1\r\n" */
	assert(komut_satiri("D4 41 54 23 43 41 0d 0a 41 54 23 55 50 0d 0a") < 0);	/* iki komut tek cercevede */
	assert(komut_satiri("D4 41 54 23 4d 4d 41 0d 42 0d 0a") < 0);		/* ic CR */
	assert(komut_satiri("D4 41 54 23 0d 0a") < 0 && komut_satiri("D4 41 54 23 4d 0d 0a") < 0);	/* harf yok / tek harf */
	assert(komut_satiri("D4 41 54 23 4d 4d 0a 0d") < 0);			/* LF CR sirasi */
	assert(komut_satiri("D4") < 0);
	assert(state.rejected == red0 + 12 && state.tx == tx0);
	/* docs/mcu-bt-protokol.md §7: AT#MMCarKit -> 02 80 d4 0d 41 54 23 4d 4d 43 61 72 4b 69 74 0d 0a 6e */
	assert(komut_satiri("D4 41 54 23 4d 4d 43 61 72 4b 69 74 0d 0a") == 0);
	const uint8_t carkit[] = { 0x02, 0x80, 0xd4, 0x0d, 0x41, 0x54, 0x23, 0x4d, 0x4d, 0x43, 0x61,
				   0x72, 0x4b, 0x69, 0x74, 0x0d, 0x0a, 0x6e };
	assert(test_tx_n == sizeof carkit && !memcmp(test_tx, carkit, sizeof carkit));
	/* yazicilar: amfi-bekci ad satiri (VeLinux KOLE, UTF-8), panel AT#CA/CC/CD/CB, 'mcu ad' (ayni bicim) */
	assert(komut_satiri("D4 41 54 23 4d 4d 56 65 4c 69 6e 75 78 20 4b c3 96 4c 45 0d 0a") == 0);
	assert(komut_satiri("D4 41 54 23 43 41 0d 0a") == 0 && komut_satiri("D4 41 54 23 43 43 0d 0a") == 0);
	assert(komut_satiri("D4 41 54 23 43 44 0d 0a") == 0 && komut_satiri("D4 41 54 23 43 42 0d 0a") == 0);
	assert(komut_satiri("D4 41 54 23 75 51 0d 0a") == 0);			/* "AT#uQ": yalniz UP yasak */
	assert(state.tx == tx0 + 7);
	/* v7 (A9): tasan FIFO satiri (100 harflik ad, 624 bayt) hic cerceve gondermez; sonraki satir calisir */
	char uzun[700];
	int k = snprintf(uzun, sizeof uzun, "D4 41 54 23 4d 4d");
	for (int i = 0; i < 100; i++)
		k += snprintf(uzun + k, sizeof uzun - (size_t)k, " c5 9e");
	snprintf(uzun + k, sizeof uzun - (size_t)k, " 0d 0a\n");
	assert(strlen(uzun) == 624);
	tx0 = state.tx;
	red0 = state.rejected;
	test_fifo(uzun);
	assert(state.tx == tx0 && state.rejected == red0 && !atla && kn == 0);
	test_fifo("A7 0f 00\n");
	assert(state.tx == tx0 + 1 && test_tx_n == sizeof a7 && !memcmp(test_tx, a7, sizeof a7));
	/* sinir: 510 karakter + LF calisir, 511 karakter + LF atilir; parcali yazim */
	char sinir[600];
	k = snprintf(sinir, sizeof sinir, "B8 01");
	for (int i = 0; i < 168; i++)
		k += snprintf(sinir + k, sizeof sinir - (size_t)k, " 00");
	assert(k == 509);
	strcpy(sinir + k, " \n");
	test_fifo(sinir);
	assert(state.tx == tx0 + 2 && test_tx[2] == 0xb8 && test_tx[3] == 169);
	strcpy(sinir + k, "  \n");
	test_fifo(sinir);
	assert(state.tx == tx0 + 2 && !atla && kn == 0);
	test_fifo("A7 0f");
	test_fifo(" 00\nA7 0f 00\n");
	assert(state.tx == tx0 + 4 && kn == 0);
	/* v7 (A7): bozuk LEN (06 -> f6) yarim cerceve; arkasindaki B2 0.5 sn sonra cevaplanir (v6: 251 bayt bekliyordu) */
	uint8_t k2[11] = { 0x02, 0x81, 0xb2, 0x06, 0x00, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00 };
	for (size_t i = 1; i < 10; i++)
		k2[10] ^= k2[i];
	uint8_t bozuk[11];
	memcpy(bozuk, k2, sizeof bozuk);
	bozuk[3] = 0xf6;
	unsigned long hb0 = state.heartbeats;
	tx0 = state.tx;
	test_bytes(bozuk, sizeof bozuk);
	test_bytes(k2, sizeof k2);
	assert(state.heartbeats == hb0 && state.tx == tx0 && rx_n == 22 && rx_bekle > 0);
	double t = mono();
	yarim_zaman_asimi(t + 0.4);
	assert(state.heartbeats == hb0 && rx_n == 22);		/* 0.5 sn dolmadan atilmaz */
	yarim_zaman_asimi(t + 0.6);
	assert(state.heartbeats == hb0 + 1 && state.tx == tx0 + 1 && rx_n == 0 && !rx_bekle);
	assert(test_tx[2] == 0xb0 && test_tx[3] == 0x05 && test_tx[4] == 0x5a);
	/* saglam cerceve parca parca gelir, arada dongu gecikir: zaman asimi atmaz (denetim parti islendikten sonra) */
	k2[5] = 0x5b;
	k2[10] = 0;
	for (size_t i = 1; i < 10; i++)
		k2[10] ^= k2[i];
	test_bytes(k2, 4);
	assert(rx_n == 4 && rx_bekle > 0);
	test_bytes(k2 + 4, sizeof k2 - 4);
	yarim_zaman_asimi(mono() + 5);
	assert(state.heartbeats == hb0 + 2 && state.tx == tx0 + 2 && rx_n == 0 && state.bad_xor == 0);
	puts("SELF-TEST OK (v7): B2->B0 kalp, A7/AA/B8 cerceve, yasaklar, A3 (AA DATA7 bit0) + A4 (B8 03 DATA2 bit7) korumasi;"
	     " D4 tek AT# + AT#UP harf duyarsiz; tasan FIFO satiri atilir; bozuk LEN 0.5 sn; UART acilmadi");
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
	/* v7 (A8): kalp butcesi ~10 sn; SD'den calisan kod sayfasi bellek sikisinca atilip geri okunurken takilmasin */
	if (setpriority(PRIO_PROCESS, 0, -10))
		perror("mcu-servis: setpriority");
	if (mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT))
		perror("mcu-servis: mlockall");
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
	printf("mcu-servis v7 (A3+A4 korumasi, D4 tek AT#, yarim cerceve 0.5 sn, mlock): %s O_RDWR 115200; B2->B0 kalp cevabi (kod basina sinirli); komut FIFO %s\n", tty, FIFO);
	fflush(stdout);
	double started = mono(), saved = -1;
	if (baslat) {
		char kop[1024];
		snprintf(kop, sizeof kop, "%s", baslat);
		for (char *p = strtok(kop, ";"); p; p = strtok(NULL, ";"))
			if (komut_satiri(p))
				printf("baslat komutu reddedildi: %s\n", p);
	}
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
		yarim_zaman_asimi(mono());	/* v7 (A7): veri gelmese de (poll en cok 200 ms) */
		if (r > 0 && kfd >= 0 && (p[1].revents & POLLIN)) {
			ssize_t n = read(kfd, kbuf + kn, sizeof kbuf - 1 - kn);
			if (n > 0)
				komut_veri((size_t)n);	/* v7 (A9) */
		}
	}
	save_state(0);
	close(ufd);
	unlink(FIFO);
	close(lock);
	printf("Bitti: rx=%lu kalp=%lu tx=%lu red=%lu\n", state.frames, state.heartbeats, state.tx, state.rejected);
	return result;
}
