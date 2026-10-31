/* ses_kopru v2 (12.10.2026, Claude): v1 + PLAY suresince "Headphone Switch" 1, sonra eski degere doner
 * (cikislar kapaliyken DAPM DAC'i acmiyor, WRITEI tukenmiyordu).
 * --- v1 ---
 * ses_kopru v1 (12.10.2026, Claude): sesli asistan icin cihaz <-> PC ses koprusu (statik, ALSA ioctl, tinyalsa yok).
 *   ses-kopru sunucu [port]   : TCP dinler (varsayilan 8767), tek istemci. Ilk satir: "<anahtar> MIC\n" veya
 *                               "<anahtar> PLAY <bayt>\n". Anahtar /data/ses-anahtar (yoksa /dev/urandom ile uretilir, 600).
 *       MIC  : MIC3 48 kHz kayit -> 31 tap FIR (7 kHz) -> 16 kHz mono S16LE akis; istemci kapatinca ya da 120 sn'de biter.
 *       PLAY : 16 kHz mono S16LE alir -> x3 FIR ara degerleme -> 48 kHz stereo HPOUT; en fazla 60 sn.
 *   ses-kopru --self-test     : FIR/ornek donusumu sinamasi (aygit acilmaz)
 * Gizlilik: ses yalniz RAM'de ve yalniz dogru anahtari veren istemciye akar; diske yazilmaz. Akis surerken
 * /tmp/mik-aktif dosyasi vardir (panel gostergesi icin). Yalniz LAN; cihaz internete port acmaz. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <sound/asound.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define ANAHTAR_DOSYA "/data/ses-anahtar"
#define TAP 31
static float fir[TAP];

static void fir_hazirla(void)
{
	double top = 0, fc = 7000.0 / 48000.0;
	for (int i = 0; i < TAP; i++) {
		int m = i - TAP / 2;
		double s = m ? sin(2 * M_PI * fc * m) / (M_PI * m) : 2 * fc;
		double w = 0.54 - 0.46 * cos(2 * M_PI * i / (TAP - 1));
		fir[i] = (float)(s * w);
		top += fir[i];
	}
	for (int i = 0; i < TAP; i++)
		fir[i] = (float)(fir[i] / top);
}

static int16_t kirp(float v)
{
	return v > 32767 ? 32767 : v < -32768 ? -32768 : (int16_t)lrintf(v);
}

/* 48k -> 16k: gecmis tamponlu FIR + 3'te 1 secim. g: kazanc. */
struct azalt { float h[TAP]; int faz; };
static size_t azalt(struct azalt *a, const int16_t *in, size_t n, int16_t *out, float g)
{
	size_t k = 0;
	for (size_t i = 0; i < n; i++) {
		memmove(a->h, a->h + 1, (TAP - 1) * sizeof(float));
		a->h[TAP - 1] = in[i];
		if (++a->faz == 3) {
			a->faz = 0;
			float s = 0;
			for (int t = 0; t < TAP; t++)
				s += a->h[t] * fir[t];
			out[k++] = kirp(s * g);
		}
	}
	return k;
}

/* 16k -> 48k stereo: sifir ekle + FIR (x3 kazanc). */
struct arttir { float h[TAP]; };
static size_t arttir(struct arttir *a, const int16_t *in, size_t n, int16_t *out)
{
	size_t k = 0;
	for (size_t i = 0; i < n; i++)
		for (int f = 0; f < 3; f++) {
			memmove(a->h, a->h + 1, (TAP - 1) * sizeof(float));
			a->h[TAP - 1] = f ? 0 : in[i];
			float s = 0;
			for (int t = 0; t < TAP; t++)
				s += a->h[t] * fir[t];
			out[2 * k] = out[2 * k + 1] = kirp(s * 3);
			k++;
		}
	return k;
}

static double mono(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec / 1e9;
}

static void maske(struct snd_pcm_hw_params *p, int ad, unsigned v)
{
	struct snd_mask *m = &p->masks[ad - SNDRV_PCM_HW_PARAM_FIRST_MASK];
	memset(m, 0, sizeof *m);
	m->bits[v >> 5] |= 1u << (v & 31);
}

static void aralik(struct snd_pcm_hw_params *p, int ad, unsigned v)
{
	struct snd_interval *i = &p->intervals[ad - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
	memset(i, 0, sizeof *i);
	i->min = i->max = v;
	i->integer = 1;
}

static int pcm_ac(int kayit, unsigned kanal, unsigned per)
{
	const char *yol = kayit ? "/dev/snd/pcmC0D0c" : "/dev/snd/pcmC0D0p";
	int fd = open(yol, O_RDWR | O_CLOEXEC | (kayit ? O_NONBLOCK : 0));
	if (fd < 0) { perror(yol); return -1; }
	struct snd_pcm_hw_params hp;
	memset(&hp, 0, sizeof hp);
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_MASK; i <= SNDRV_PCM_HW_PARAM_LAST_MASK; i++)
		memset(hp.masks[i - SNDRV_PCM_HW_PARAM_FIRST_MASK].bits, 0xff, sizeof hp.masks[0].bits);
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL; i <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; i++) {
		struct snd_interval *v = &hp.intervals[i - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
		memset(v, 0, sizeof *v);
		v->max = ~0u;
	}
	maske(&hp, SNDRV_PCM_HW_PARAM_ACCESS, SNDRV_PCM_ACCESS_RW_INTERLEAVED);
	maske(&hp, SNDRV_PCM_HW_PARAM_FORMAT, SNDRV_PCM_FORMAT_S16_LE);
	maske(&hp, SNDRV_PCM_HW_PARAM_SUBFORMAT, SNDRV_PCM_SUBFORMAT_STD);
	aralik(&hp, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, 16);
	aralik(&hp, SNDRV_PCM_HW_PARAM_FRAME_BITS, 16 * kanal);
	aralik(&hp, SNDRV_PCM_HW_PARAM_CHANNELS, kanal);
	aralik(&hp, SNDRV_PCM_HW_PARAM_RATE, 48000);
	aralik(&hp, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, per);
	aralik(&hp, SNDRV_PCM_HW_PARAM_PERIODS, 4);
	hp.rmask = ~0u;
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &hp)) { perror("HW_PARAMS"); close(fd); return -1; }
	struct snd_pcm_sw_params sp;
	memset(&sp, 0, sizeof sp);
	sp.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
	sp.period_step = 1;
	sp.avail_min = per;
	sp.start_threshold = kayit ? 1 : per * 2;
	sp.stop_threshold = per * 4;
	sp.boundary = per * 4;
	while (sp.boundary * 2 <= 0x7fffffffu - per * 4)
		sp.boundary *= 2;
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sp)) { perror("SW_PARAMS"); close(fd); return -1; }
	if (ioctl(fd, SNDRV_PCM_IOCTL_PREPARE)) { perror("PREPARE"); close(fd); return -1; }
	return fd;
}

static int tam_yaz(int s, const void *b, size_t n)
{
	const char *p = b;
	while (n) {
		ssize_t w = send(s, p, n, MSG_NOSIGNAL);
		if (w <= 0) {
			if (w < 0 && errno == EINTR)
				continue;
			return -1;
		}
		p += w;
		n -= (size_t)w;
	}
	return 0;
}

static int mic_akis(int s)
{
	unsigned per = 960;
	int fd = pcm_ac(1, 1, per);
	if (fd < 0) return 1;
	if (ioctl(fd, SNDRV_PCM_IOCTL_START) && errno != EBADFD) { perror("START"); close(fd); return 1; }
	int af = open("/tmp/mik-aktif", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (af >= 0) { dprintf(af, "%d\n", getpid()); close(af); }
	float g = getenv("MIK_KAZANC") ? (float)atof(getenv("MIK_KAZANC")) : 4.0f;
	static int16_t in[960], out[960];
	struct azalt a = {0};
	double bitis = mono() + 120;
	unsigned long gonderilen = 0;
	unsigned xrun = 0;
	int rc = 0;
	while (mono() < bitis) {
		struct snd_xferi x = { .buf = in, .frames = per };
		int err = 0;
		if (ioctl(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &x)) err = errno;
		if (err == EAGAIN || (!err && !x.result)) {
			struct timespec d = { .tv_nsec = 5000000 };
			nanosleep(&d, NULL);
			continue;
		}
		if (err == EPIPE && ++xrun < 50) {
			ioctl(fd, SNDRV_PCM_IOCTL_PREPARE);
			ioctl(fd, SNDRV_PCM_IOCTL_START);
			continue;
		}
		if (err || x.result < 0 || x.result > (long)per) { fprintf(stderr, "READI %d\n", err); rc = 1; break; }
		size_t k = azalt(&a, in, (size_t)x.result, out, g);
		if (tam_yaz(s, out, k * 2)) break;	/* istemci kapatti */
		gonderilen += k;
	}
	memset(in, 0, sizeof in);
	memset(out, 0, sizeof out);
	ioctl(fd, SNDRV_PCM_IOCTL_DROP);
	close(fd);
	unlink("/tmp/mik-aktif");
	printf("MIC bitti: %lu ornek (%.1f sn) xrun=%u rc=%d\n", gonderilen, gonderilen / 16000.0, xrun, rc);
	return rc;
}

/* bool mikser kontrolu: eski degeri dondurur (-1 hata) */
static int bool_ayar(const char *ad, int yeni)
{
	int fd = open("/dev/snd/controlC0", O_RDWR | O_CLOEXEC);
	if (fd < 0) return -1;
	struct snd_ctl_elem_value v;
	memset(&v, 0, sizeof v);
	v.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	snprintf((char *)v.id.name, sizeof v.id.name, "%s", ad);
	int eski = -1;
	if (!ioctl(fd, SNDRV_CTL_IOCTL_ELEM_READ, &v)) {
		eski = (int)v.value.integer.value[0];
		v.value.integer.value[0] = yeni;
		if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &v)) eski = -1;
	}
	close(fd);
	return eski;
}

static int cal_akis_ic(int s, long bayt);
static int cal_akis(int s, long bayt)
{
	int eski = bool_ayar("Headphone Switch", 1);
	int rc = cal_akis_ic(s, bayt);
	if (eski >= 0) bool_ayar("Headphone Switch", eski);
	return rc;
}

static int cal_akis_ic(int s, long bayt)
{
	if (bayt <= 0 || bayt > 60L * 16000 * 2) { fprintf(stderr, "PLAY boyut gecersiz\n"); return 2; }
	unsigned per = 960;
	int fd = pcm_ac(0, 2, per);
	if (fd < 0) return 1;
	static int16_t in[320], out[960 * 2];
	struct arttir a = {0};
	long alinan = 0;
	unsigned xrun = 0;
	int tek = 0, tekb = 0;	/* tek bayt kalintisi */
	while (alinan < bayt) {
		char *p = (char *)in;
		size_t dolu = 0;
		if (tek) { p[0] = (char)tekb; dolu = 1; tek = 0; }
		long kalan = bayt - alinan;
		size_t iste = sizeof in - dolu;
		if ((long)iste > kalan) iste = (size_t)kalan;
		ssize_t r = recv(s, p + dolu, iste, 0);
		if (r <= 0) break;
		alinan += r;
		dolu += (size_t)r;
		if (dolu & 1) { tek = 1; tekb = p[dolu - 1]; dolu--; }
		size_t k = arttir(&a, in, dolu / 2, out);
		size_t yazilan = 0;
		while (yazilan < k) {
			struct snd_xferi x = { .buf = out + 2 * yazilan, .frames = k - yazilan };
			if (ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &x)) {
				if (errno == EPIPE && ++xrun < 50) { ioctl(fd, SNDRV_PCM_IOCTL_PREPARE); continue; }
				if (errno == EINTR) continue;
				perror("WRITEI");
				close(fd);
				return 1;
			}
			yazilan += (size_t)x.result;
		}
	}
	/* kuyrugu bosalt: FIR gecikmesi + bir periyot sessizlik */
	memset(out, 0, sizeof out);
	struct snd_xferi x = { .buf = out, .frames = per };
	ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &x);
	ioctl(fd, SNDRV_PCM_IOCTL_DRAIN);
	close(fd);
	printf("PLAY bitti: %ld bayt (%.2f sn) xrun=%u\n", alinan, alinan / 32000.0, xrun);
	return alinan == bayt ? 0 : 1;
}

static int anahtar_oku(char *k, size_t n)
{
	FILE *f = fopen(ANAHTAR_DOSYA, "r");
	if (!f) {
		unsigned char r[16];
		int u = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
		if (u < 0 || read(u, r, sizeof r) != (ssize_t)sizeof r) { if (u >= 0) close(u); return -1; }
		close(u);
		int w = open(ANAHTAR_DOSYA, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
		if (w < 0) return -1;
		for (int i = 0; i < 16; i++)
			dprintf(w, "%02x", r[i]);
		dprintf(w, "\n");
		fsync(w);
		close(w);
		f = fopen(ANAHTAR_DOSYA, "r");
		if (!f) return -1;
		puts("yeni anahtar uretildi: " ANAHTAR_DOSYA);
	}
	if (!fgets(k, (int)n, f)) { fclose(f); return -1; }
	fclose(f);
	k[strcspn(k, "\r\n")] = 0;
	return strlen(k) >= 16 ? 0 : -1;
}

/* sabit zamanli karsilastirma */
static int esit(const char *a, const char *b)
{
	size_t na = strlen(a), nb = strlen(b);
	unsigned d = (unsigned)(na ^ nb);
	for (size_t i = 0; i < na && i < nb; i++)
		d |= (unsigned char)(a[i] ^ b[i]);
	return d == 0;
}

static int sunucu(int port)
{
	char anahtar[80];
	if (anahtar_oku(anahtar, sizeof anahtar)) { fprintf(stderr, "anahtar okunamadi\n"); return 1; }
	int l = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	int bir = 1;
	setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &bir, sizeof bir);
	struct sockaddr_in ad = { .sin_family = AF_INET, .sin_port = htons((uint16_t)port) };
	if (bind(l, (struct sockaddr *)&ad, sizeof ad) || listen(l, 1)) { perror("bind/listen"); return 1; }
	printf("ses-kopru v2: TCP %d dinleniyor\n", port);
	fflush(stdout);
	for (;;) {
		struct sockaddr_in ia;
		socklen_t il = sizeof ia;
		int s = accept(l, (struct sockaddr *)&ia, &il);
		if (s < 0) { if (errno == EINTR) continue; perror("accept"); sleep(1); continue; }
		struct timeval to = { .tv_sec = 5 };
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
		setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &bir, sizeof bir);
		char satir[160];
		size_t n = 0;
		while (n < sizeof satir - 1) {
			ssize_t r = recv(s, satir + n, 1, 0);
			if (r <= 0) break;
			if (satir[n] == '\n') break;
			n++;
		}
		satir[n] = 0;
		char *k = strtok(satir, " "), *kmt = strtok(NULL, " "), *arg = strtok(NULL, " ");
		uint32_t ip = ntohl(ia.sin_addr.s_addr);
		if (!k || !kmt || !esit(k, anahtar)) {
			printf("RED %u.%u.%u.%u (anahtar)\n", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
			fflush(stdout);
			close(s);
			sleep(1);	/* kaba kuvvet yavaslatma */
			continue;
		}
		printf("istemci %u.%u.%u.%u %s\n", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255, kmt);
		fflush(stdout);
		if (!strcmp(kmt, "MIC")) {
			mic_akis(s);
		} else if (!strcmp(kmt, "PLAY") && arg) {
			struct timeval to2 = { .tv_sec = 10 };
			setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &to2, sizeof to2);
			int rc = cal_akis(s, atol(arg));
			char c[32];
			int m = snprintf(c, sizeof c, "OK %d\n", rc);
			tam_yaz(s, c, (size_t)m);
		}
		fflush(stdout);
		close(s);
	}
}

static int self_test(void)
{
	fir_hazirla();
	/* 1 kHz 48k -> 16k: genlik korunmali; 12 kHz bastirilmali */
	static int16_t in[4800], out[1600], ust[4800 * 2];
	for (int f = 0; f < 2; f++) {
		double hz = f ? 12000 : 1000;
		for (int i = 0; i < 4800; i++)
			in[i] = (int16_t)(10000 * sin(2 * M_PI * hz * i / 48000));
		struct azalt a = {0};
		size_t k = azalt(&a, in, 4800, out, 1.0f);
		double e = 0;
		for (size_t i = 200; i < k; i++)
			e += (double)out[i] * out[i];
		double rms = sqrt(e / (k - 200));
		printf("azalt %5.0f Hz: %zu ornek rms=%.0f\n", hz, k, rms);
		if (k != 1600 || (f == 0 && fabs(rms - 7071) > 400) || (f == 1 && rms > 300)) return 1;
	}
	for (int i = 0; i < 1600; i++)
		out[i] = (int16_t)(10000 * sin(2 * M_PI * 1000.0 * i / 16000));
	struct arttir b = {0};
	size_t k = arttir(&b, out, 1600, ust);
	double e = 0;
	for (size_t i = 300; i < k; i++)
		e += (double)ust[2 * i] * ust[2 * i];
	double rms = sqrt(e / (k - 300));
	printf("arttir 1000 Hz: %zu kare rms=%.0f\n", k, rms);
	if (k != 4800 || fabs(rms - 7071) > 400) return 1;
	puts("SELF-TEST PASS: 48k->16k, 16k->48k FIR; aygit/ag acilmadi");
	return 0;
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	if (argc == 2 && !strcmp(argv[1], "--self-test"))
		return self_test();
	if (argc >= 2 && !strcmp(argv[1], "sunucu")) {
		fir_hazirla();
		return sunucu(argc > 2 ? atoi(argv[2]) : 8767);
	}
	fputs("kullanim: ses-kopru sunucu [port] | --self-test\n", stderr);
	return 2;
}
