/* v4: actual WRITEI result, partial offsets, strict low tone bounds, deadline/drain status. */
/* v3 numerical spectrum only; tone path retained fromv2, bounded by external timeout. */
/* v2: per-channel levels, actual READI result, nonblocking deadline, channels1..3. */
/* ses v1 (12.10.2026, Claude): ALSA ioctl ile en kucuk calma/kayit/mikser araci (tinyalsa yok, statik).
 *   ses kontrol [kart]                       : mikser kontrollerini listele (ad, tur, deger)
 *   ses ayar <kart> "<ad>" <deger>[,<deger2>] : tamsayi/bool/enum (sira no) kontrolu yaz
 *   ses ton <kart> <cihaz> <hz> <sn> [genlik%] : 48 kHz S16 stereo sinus cal
 *   ses kayit <kart> <cihaz> <sn> [kanal]    : 48 kHz S16 kayit; her 200 ms RMS/tepe (dBFS) yazar, SES KAYDEDILMEZ
 * Gizlilik: kayit modu yalniz duzey hesaplar; ham ses diske/aga yazilmaz. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <sound/asound.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static void maske_ayarla(struct snd_pcm_hw_params *p, int ad, unsigned v)
{
	struct snd_mask *m = &p->masks[ad - SNDRV_PCM_HW_PARAM_FIRST_MASK];
	memset(m, 0, sizeof *m);
	m->bits[v >> 5] |= 1u << (v & 31);
}

static void aralik_ayarla(struct snd_pcm_hw_params *p, int ad, unsigned v)
{
	struct snd_interval *i = &p->intervals[ad - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
	memset(i, 0, sizeof *i);
	i->min = i->max = v;
	i->integer = 1;
}

static void aralik_serbest(struct snd_pcm_hw_params *p, int ad)
{
	struct snd_interval *i = &p->intervals[ad - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
	memset(i, 0, sizeof *i);
	i->max = ~0u;
}

static int pcm_ac(int kart, int cihaz, int kayit, unsigned kanal, unsigned hiz, unsigned *periyot)
{
	char yol[64];
	snprintf(yol, sizeof yol, "/dev/snd/pcmC%dD%d%c", kart, cihaz, kayit ? 'c' : 'p');
	int fd = open(yol, O_RDWR | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0) { perror(yol); return -1; }
	struct snd_pcm_hw_params hp;
	memset(&hp, 0, sizeof hp);
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_MASK; i <= SNDRV_PCM_HW_PARAM_LAST_MASK; i++)
		memset(hp.masks[i - SNDRV_PCM_HW_PARAM_FIRST_MASK].bits, 0xff, sizeof hp.masks[0].bits);
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL; i <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; i++)
		aralik_serbest(&hp, i);
	maske_ayarla(&hp, SNDRV_PCM_HW_PARAM_ACCESS, SNDRV_PCM_ACCESS_RW_INTERLEAVED);
	maske_ayarla(&hp, SNDRV_PCM_HW_PARAM_FORMAT, SNDRV_PCM_FORMAT_S16_LE);
	maske_ayarla(&hp, SNDRV_PCM_HW_PARAM_SUBFORMAT, SNDRV_PCM_SUBFORMAT_STD);
	aralik_ayarla(&hp, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, 16);
	aralik_ayarla(&hp, SNDRV_PCM_HW_PARAM_FRAME_BITS, 16 * kanal);
	aralik_ayarla(&hp, SNDRV_PCM_HW_PARAM_CHANNELS, kanal);
	aralik_ayarla(&hp, SNDRV_PCM_HW_PARAM_RATE, hiz);
	aralik_ayarla(&hp, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, *periyot);
	aralik_ayarla(&hp, SNDRV_PCM_HW_PARAM_PERIODS, 4);
	hp.rmask = ~0u;
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &hp)) { perror("HW_PARAMS"); close(fd); return -1; }
	struct snd_pcm_sw_params sp;
	memset(&sp, 0, sizeof sp);
	sp.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
	sp.period_step = 1;
	sp.avail_min = *periyot;
	sp.start_threshold = kayit ? 1 : *periyot * 2;
	sp.stop_threshold = *periyot * 4;
	sp.boundary = *periyot * 4;
	while (sp.boundary * 2 <= 0x7fffffffu - *periyot * 4)
		sp.boundary *= 2;
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sp)) { perror("SW_PARAMS"); close(fd); return -1; }
	if (ioctl(fd, SNDRV_PCM_IOCTL_PREPARE)) { perror("PREPARE"); close(fd); return -1; }
	printf("%s: %u kanal %u Hz S16, periyot %u\n", yol, kanal, hiz, *periyot);
	return fd;
}


static double mono_sn(void);

static void ton_bekle(void)
{
    const struct timespec delay = {.tv_nsec = 5000000};
    nanosleep(&delay, NULL);
}

static int ton(int kart, int cihaz, double hz, double sn, double genlik)
{
    unsigned per = 1024, hiz = 48000;
    int fd = pcm_ac(kart, cihaz, 0, 2, hiz, &per);
    if (fd < 0) return 1;
    int16_t buf[1024 * 2];
    unsigned long hedef = (unsigned long)(sn * hiz), done = 0;
    unsigned short_writes = 0;
    double deadline = mono_sn() + sn + 6;
    int rc = 0;
    while (done < hedef) {
        unsigned count = hedef - done < per ? (unsigned)(hedef - done) : per;
        /* Absolute sample phase survives partial writes without discontinuity. */
        for (unsigned i = 0; i < count; i++) {
            int16_t v = (int16_t)(sin(2 * M_PI * hz * (done + i) / hiz)
                                 * 32767.0 * genlik);
            buf[2*i] = buf[2*i+1] = v;
        }
        unsigned offset = 0;
        while (offset < count) {
            if (mono_sn() > deadline) { rc = 1; fputs("WRITEI deadline\n", stderr); break; }
            unsigned want = count - offset;
            struct snd_xferi x = {.buf = buf + 2*offset, .frames = want};
            int err = 0;
            if (ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &x)) err = errno;
            else if (x.result < 0) err = (int)-x.result;
            if (err == EAGAIN || err == EINTR || (!err && !x.result)) {
                ton_bekle(); continue;
            }
            /* An underrun is a failed continuity measurement, never success. */
            if (err || (unsigned long)x.result > want) {
                fprintf(stderr, "WRITEI error=%d result=%ld want=%u\n", err, (long)x.result, want);
                rc = 1; break;
            }
            if ((unsigned long)x.result < want) short_writes++;
            offset += (unsigned)x.result;
        }
        done += offset;
        if (rc) break;
    }
    if (!rc) {
        if (ioctl(fd, SNDRV_PCM_IOCTL_DRAIN) && errno != EAGAIN && errno != EINTR) {
            perror("DRAIN"); rc = 1;
        }
        /* Nonblocking DRAIN starts draining; STATUS confirms completion. */
        double drain_deadline = mono_sn() + 5;
        while (!rc) {
            struct snd_pcm_status status;
            memset(&status, 0, sizeof status);
            if (ioctl(fd, SNDRV_PCM_IOCTL_STATUS, &status)) { perror("STATUS"); rc = 1; break; }
            if (status.state == SNDRV_PCM_STATE_SETUP) break;
            if (status.state != SNDRV_PCM_STATE_DRAINING || mono_sn() > drain_deadline) {
                fprintf(stderr, "DRAIN state=%d deadline/error\n", status.state);
                rc = 1; break;
            }
            ton_bekle();
        }
    }
    if (rc) ioctl(fd, SNDRV_PCM_IOCTL_DROP);
    memset(buf, 0, sizeof buf);
    close(fd);
    printf("TON BITTI accepted_frames=%lu target_frames=%lu short_writes=%u rc=%d\n",
           done, hedef, short_writes, rc);
    return rc;
}

static int ton_args(int argc, char **argv)
{
    if (argc != 6 && argc != 7) return 2;
    char *end;
    errno = 0;
    long kart = strtol(argv[2], &end, 10);
    if (errno || !*argv[2] || *end || kart < 0 || kart > 31) return 2;
    long cihaz = strtol(argv[3], &end, 10);
    if (errno || !*argv[3] || *end || cihaz < 0 || cihaz > 31) return 2;
    double hz = strtod(argv[4], &end);
    if (errno || !*argv[4] || *end || !isfinite(hz) || hz < 20 || hz > 20000) return 2;
    double sn = strtod(argv[5], &end);
    if (errno || !*argv[5] || *end || !isfinite(sn) || sn < 0.1 || sn > 30) return 2;
    double percent = argc == 7 ? strtod(argv[6], &end) : 3;
    if (errno || !isfinite(percent) || percent < 0.1 || percent > 10 ||
        (argc == 7 && (!*argv[6] || *end))) return 2;
    return ton((int)kart, (int)cihaz, hz, sn, percent/100);
}

/* Inserted by 91_prepare_ses_v2.py. PCM exists only in RAM; numeric levels only. */
struct duzey {
    double kare[3];
    double g1[3][2], g2[3][2];
    int tepe[3];
    unsigned kirpilma[3];
    unsigned long kare_sayisi;
};

static void duzey_ekle(struct duzey *d, const int16_t *buf, unsigned frames, unsigned channels)
{
    for (unsigned f = 0; f < frames; f++) {
        for (unsigned c = 0; c < channels; c++) {
            int v = buf[f * channels + c];
            d->kare[c] += (double)v * v;
            const double freqs[2] = {440.0,1000.0};
            for (unsigned b=0;b<2;b++) {
                double coeff=2*cos(2*M_PI*freqs[b]/48000.0);
                double next=v+coeff*d->g1[c][b]-d->g2[c][b];
                d->g2[c][b]=d->g1[c][b]; d->g1[c][b]=next;
            }
            if (abs(v) > d->tepe[c]) d->tepe[c] = abs(v);
            if (v == -32768 || v == 32767) d->kirpilma[c]++;
        }
    }
    d->kare_sayisi += frames;
}

static double mono_sn(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

static double bin_power(const struct duzey *d,unsigned c,unsigned b)
{
    const double freqs[2]={440.0,1000.0};
    if (!d->kare_sayisi) return 0;
    double coeff=2*cos(2*M_PI*freqs[b]/48000.0);
    double p=d->g1[c][b]*d->g1[c][b]+d->g2[c][b]*d->g2[c][b]-coeff*d->g1[c][b]*d->g2[c][b];
    return fmax(0,2*p/((double)d->kare_sayisi*d->kare_sayisi));
}

static void duzey_yaz(struct duzey *d, unsigned channels, unsigned long total, unsigned rate)
{
    if (!d->kare_sayisi) return;
    printf("%.3f sn frames=%lu", (double)total / rate, d->kare_sayisi);
    for (unsigned c = 0; c < channels; c++) {
        double rms = sqrt(d->kare[c] / d->kare_sayisi);
        printf(" | ch%u rms=%.1f peak=%.1f dBFS clip=%u", c + 1,
               rms ? 20 * log10(rms / 32768.0) : -120.0,
               d->tepe[c] ? 20 * log10(d->tepe[c] / 32768.0) : -120.0,
               d->kirpilma[c]);
        for (unsigned b=0;b<2;b++) {
            double r=sqrt(bin_power(d,c,b));
            printf(" tone%u=%.1f dBFS",b?1000:440,r>0?20*log10(r/32768.0):-120.0);
        }
    }
    putchar('\n');
    fflush(stdout);
    memset(d, 0, sizeof *d);
}

static int duzey_test(void)
{
    /* Last two frames simulate stale bytes after a short READI result. */
    const int16_t b[] = {0, 16384, -32768, 100, -16384, 0,
                        32000, 32000, 32000, 32000, 32000, 32000};
    struct duzey d = {0};
    duzey_ekle(&d, b, 2, 3);
    if (d.kare_sayisi != 2 || d.kare[0] != 10000.0 ||
        d.kare[1] != 536870912.0 || d.kare[2] != 1073741824.0 ||
        d.tepe[0] != 100 || d.tepe[1] != 16384 || d.tepe[2] != 32768 ||
        d.kirpilma[0] || d.kirpilma[1] || d.kirpilma[2] != 1)
        return 1;
    struct duzey tone={0};
    int16_t chunk[9600];
    for (unsigned i=0;i<9600;i++) chunk[i]=(int16_t)(8192*sin(2*M_PI*1000*i/48000.0));
    duzey_ekle(&tone,chunk,9600,1);
    double measured=sqrt(bin_power(&tone,0,1));
    if (fabs(measured-8192/sqrt(2))>2 || sqrt(bin_power(&tone,0,0))>2) return 1;
    memset(chunk,0,sizeof chunk); memset(&tone,0,sizeof tone);
    duzey_ekle(&tone,chunk,9600,1);
    if (bin_power(&tone,0,0)!=0 || bin_power(&tone,0,1)!=0) return 1;
    puts("SELF-TEST PASS: channel/short-read/clip,1000Hz magnitude and440Hz isolation,silence; no audio device opened");
    return 0;
}

static int kayit(int kart, int cihaz, double sn, unsigned kanal)
{
    if (kart < 0 || kart > 31 || cihaz < 0 || cihaz > 31 ||
        !isfinite(sn) || sn < 0.2 || sn > 60 || kanal < 1 || kanal > 3) {
        fputs("capture bounds: card/device0..31, seconds0.2..60, channels1..3\n", stderr);
        return 2;
    }
    unsigned per = 1920, hiz = 48000;
    int fd = pcm_ac(kart, cihaz, 1, kanal, hiz, &per);
    if (fd < 0) return 1;
    if (ioctl(fd, SNDRV_PCM_IOCTL_START) && errno != EBADFD) {
        perror("START"); close(fd); return 1;
    }
    int16_t buf[1920 * 3];
    struct duzey d = {0};
    unsigned long hedef = (unsigned long)(sn * hiz), done = 0;
    unsigned xruns = 0;
    int rc = 0;
    double deadline = mono_sn() + sn + 5;
    while (done < hedef) {
        if (mono_sn() > deadline) { fputs("capture deadline exceeded\n", stderr); rc = 1; break; }
        unsigned want = hedef - done < per ? (unsigned)(hedef - done) : per;
        struct snd_xferi x = {.buf = buf, .frames = want};
        int err = 0;
        if (ioctl(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &x)) err = errno;
        else if (x.result < 0) err = (int)-x.result;
        if (err == EAGAIN || err == EINTR || (!err && !x.result)) {
            struct timespec delay = {.tv_nsec = 5000000};
            nanosleep(&delay, NULL);
            continue;
        }
        if (err == EPIPE && ++xruns <= 8) {
            if (!ioctl(fd, SNDRV_PCM_IOCTL_PREPARE) && !ioctl(fd, SNDRV_PCM_IOCTL_START)) continue;
            err = errno;
        }
        if (err || (unsigned long)x.result > want) {
            fprintf(stderr, "READI error=%d result=%ld want=%u xruns=%u\n",
                    err, (long)x.result, want, xruns);
            rc = 1; break;
        }
        /* Only frames actually returned by ALSA are valid. */
        duzey_ekle(&d, buf, (unsigned)x.result, kanal);
        done += (unsigned long)x.result;
        if (d.kare_sayisi >= hiz / 5) duzey_yaz(&d, kanal, done, hiz);
    }
    duzey_yaz(&d, kanal, done, hiz);
    memset(buf, 0, sizeof buf);
    ioctl(fd, SNDRV_PCM_IOCTL_DROP);
    close(fd);
    printf("KAYIT BITTI frames=%lu xruns=%u rc=%d (raw audio not saved or sent)\n", done, xruns, rc);
    return rc;
}

static int kayit_args(int argc, char **argv)
{
    if (argc < 5 || argc > 6) return 2;
    char *end;
    errno = 0;
    long kart = strtol(argv[2], &end, 10);
    if (errno || !*argv[2] || *end || kart < 0 || kart > 31) return 2;
    long cihaz = strtol(argv[3], &end, 10);
    if (errno || !*argv[3] || *end || cihaz < 0 || cihaz > 31) return 2;
    double sn = strtod(argv[4], &end);
    if (errno || !*argv[4] || *end || !isfinite(sn) || sn < 0.2 || sn > 60) return 2;
    long kanal = argc == 6 ? strtol(argv[5], &end, 10) : 1;
    if (errno || kanal < 1 || kanal > 3 || (argc == 6 && (!*argv[5] || *end))) return 2;
    return kayit((int)kart, (int)cihaz, sn, (unsigned)kanal);
}

static int kontrol_ac(int kart)
{
	char yol[48];
	snprintf(yol, sizeof yol, "/dev/snd/controlC%d", kart);
	int fd = open(yol, O_RDWR | O_CLOEXEC);
	if (fd < 0) perror(yol);
	return fd;
}

static int kontrol_liste(int kart)
{
	int fd = kontrol_ac(kart);
	if (fd < 0) return 1;
	struct snd_ctl_elem_list l;
	memset(&l, 0, sizeof l);
	if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &l)) { perror("ELEM_LIST"); return 1; }
	unsigned n = l.count;
	struct snd_ctl_elem_id *ids = calloc(n, sizeof *ids);
	l.space = n;
	l.pids = ids;
	if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_LIST, &l)) { perror("ELEM_LIST2"); return 1; }
	for (unsigned i = 0; i < l.used; i++) {
		struct snd_ctl_elem_info inf;
		memset(&inf, 0, sizeof inf);
		inf.id = ids[i];
		if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, &inf)) continue;
		struct snd_ctl_elem_value v;
		memset(&v, 0, sizeof v);
		v.id = ids[i];
		ioctl(fd, SNDRV_CTL_IOCTL_ELEM_READ, &v);
		printf("%3u '%s' ", ids[i].numid, ids[i].name);
		if (inf.type == SNDRV_CTL_ELEM_TYPE_INTEGER)
			printf("int %ld..%ld =", inf.value.integer.min, inf.value.integer.max);
		else if (inf.type == SNDRV_CTL_ELEM_TYPE_BOOLEAN)
			printf("bool =");
		else if (inf.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED)
			printf("enum(%u) =", inf.value.enumerated.items);
		else
			printf("tur%d =", inf.type);
		for (unsigned c = 0; c < inf.count && c < 4; c++)
			printf(" %ld", inf.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED ? (long)v.value.enumerated.item[c] : v.value.integer.value[c]);
		if (inf.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED) {
			printf("  [");
			for (unsigned e = 0; e < inf.value.enumerated.items; e++) {
				inf.value.enumerated.item = e;
				if (!ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, &inf))
					printf("%s%s", e ? "|" : "", inf.value.enumerated.name);
			}
			printf("]");
		}
		putchar('\n');
	}
	close(fd);
	return 0;
}

static int kontrol_yaz(int kart, const char *ad, const char *deg)
{
	int fd = kontrol_ac(kart);
	if (fd < 0) return 1;
	struct snd_ctl_elem_info inf;
	memset(&inf, 0, sizeof inf);
	inf.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	snprintf((char *)inf.id.name, sizeof inf.id.name, "%s", ad);
	if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_INFO, &inf)) { fprintf(stderr, "kontrol yok: %s\n", ad); return 1; }
	struct snd_ctl_elem_value v;
	memset(&v, 0, sizeof v);
	v.id = inf.id;
	char kop[64];
	snprintf(kop, sizeof kop, "%s", deg);
	char *p = strtok(kop, ",");
	long son = 0;
	for (unsigned c = 0; c < inf.count; c++) {
		if (p) { son = strtol(p, NULL, 0); p = strtok(NULL, ","); }
		if (inf.type == SNDRV_CTL_ELEM_TYPE_ENUMERATED)
			v.value.enumerated.item[c] = (unsigned)son;
		else
			v.value.integer.value[c] = son;
	}
	if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &v)) { perror("ELEM_WRITE"); return 1; }
	printf("YAZILDI '%s' = %s\n", ad, deg);
	close(fd);
	return 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "--self-test")) return duzey_test();
	if (argc >= 2 && !strcmp(argv[1], "kontrol"))
		return kontrol_liste(argc > 2 ? atoi(argv[2]) : 0);
	if (argc == 5 && !strcmp(argv[1], "ayar"))
		return kontrol_yaz(atoi(argv[2]), argv[3], argv[4]);
	if (argc >= 2 && !strcmp(argv[1], "ton")) return ton_args(argc, argv);
	if (argc >= 2 && !strcmp(argv[1], "kayit"))
		return kayit_args(argc, argv);
	fputs("kullanim: ses kontrol [kart] | ayar <kart> <ad> <deger> | ton <kart> <cihaz> <hz> <sn> [genlik%] | kayit <kart> <cihaz> <sn> [kanal]\n", stderr);
	return 2;
}
