/* ses_akis_v4.h (24.10.2026, Claude; scripts/624, denetim-2026-10-24-v3 A5, A18a): v2 + "Hey Kole birak" bayraginin
 * sahibi: /tmp/asistan-mik-birak O_EXCL ile "kopru <pid>\n" acilir (varsa dokunulmaz: TTS "tts <pid>" ya da baska bir
 * dinleme), her cikista yalniz bizimse silinir; /tmp/mik-aktif close'dan once ve yalniz bizimse silinir; olduren taraf
 * (RTSP, Merkez) mik_cocuk_temizle(pid). Mesgul mikrofon perror'u yalniz bir kez (son denemeden sonra). v3 adi kurulmamis
 * taslakta. --- v2 ---
 */
/* ses_akis_v2.h (15.10.2026, Claude): v1 + Hey Kole mikrofon paylasimi (scripts/178): mic_akis once
 * /tmp/asistan-mik-birak koyar, mesgulse 1.5 sn yeniden dener, bitince siler. v1 korumasini da tanimlar.
 * --- v1 ---
 * ses_akis_v1.h (13.10.2026, Claude): kole-kopru v8 (ve ileride cihaz-ici web merkezi) ses yollari.
 * MIC/PLAY kodu kole-kopru v7 (= ses-kopru v2) ile AYNI: dahili codec /dev/snd/pcmC0D0{c,p}, 48 kHz S16, FIR 48k<->16k.
 * ALSA yonlendirmesi DEGISTIRILMEZ: yalniz calma sirasinda "Headphone Switch" 1 yapilir ve eski degere dondurulur
 * (v7 PLAY ile ayni). MCU'ya cerceve gonderilmez.
 *   mic_akis(s, sn, baslik): 16 kHz mono s16le akis. Suresince /tmp/mik-aktif (pid) panel gostergesi.
 *       baslik=1 (MIC_AKIS): once "MIC_AKIS 16000 1 s16le\n" ya da "HATA ..." satiri. Gizlilik kalkani -> HATA / durur.
 *   cal_akis(s, bayt)       : v7 PLAY <bayt> (sabit uzunluk).
 *   play_akis(s, sn)        : PLAY_AKIS: "PLAY_AKIS HAZIR 16000\n"; [u16 LE n][n bayt PCM16k] cerceveleri, n=0 son;
 *                             150 ms on tampon (ag titremesi), yetersizlikte yeniden tamponlar; en fazla sn (<=120).
 *                             Bitiste "OK <ornek> <xrun>\n". Suresince /tmp/hoparlor-aktif.
 * ses_sahte=1 (self-test / rtsp-test): ses aygiti ACILMAZ; mikrofon 1 kHz sentetik ton (gercek zamanli),
 * hoparlor yazimlari yalniz sayilir. Ham ses hicbir yerde diske yazilmaz. */
#ifndef SES_AKIS_V1_H
#define SES_AKIS_V1_H
#define SES_AKIS_V2_H	/* v1 yerine: sonraki v1 dahil etmeleri bos gecer */
#define SES_AKIS_V4_H
#define ASISTAN_BIRAK "/tmp/asistan-mik-birak"	/* kole-asistan bu bayrakta mikrofonu birakir */
#include <math.h>
#include <sys/ioctl.h>
#include <sound/asound.h>
#include "kopru_ortak_v1.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define TAP 31
static float fir[TAP];
static int ses_sahte = 0;
static unsigned long ses_sahte_kare = 0;	/* sahte hoparlore yazilan 48k kare */
static const char *asistan_birak_yol = ASISTAN_BIRAK;	/* v4: self-test / KOLE_TEST yonlendirir */
static int pcm_mesgul_sessiz = 0;	/* v4: yeniden denemelerde EBUSY perror'u basilmaz */

/* v4: dosya tam olarak "<on><pid>" ise sil (sahibi baskasiysa dokunulmaz) */
static void sahipliyse_sil(const char *yol, const char *on, pid_t p)
{
	char b[48], bek[48];
	snprintf(bek, sizeof bek, "%s%d", on, (int)p);
	if (dosya_oku(yol, b, sizeof b) > 0 && !strcmp(b, bek)) unlink(yol);
}

/* v4: SIGKILL ile olen mikrofon cocugunun bayraklari (RTSP, Merkez): yalniz o cocugunsa */
static void mik_cocuk_temizle(pid_t p)
{
	sahipliyse_sil(mik_bayrak, "", p);
	sahipliyse_sil(asistan_birak_yol, "kopru ", p);
}

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

/* v7 pcm_ac ile ayni parametreler; *mesgul = EBUSY ise 1 */
static int pcm_ac(int kayit, unsigned kanal, unsigned per, int *mesgul)
{
	char yol[128];
	if (mesgul) *mesgul = 0;
	if (ses_sahte) return open("/dev/null", O_RDWR | O_CLOEXEC);
	snprintf(yol, sizeof yol, "%s/%s", snd_kok, kayit ? "pcmC0D0c" : "pcmC0D0p");
	int fd = open(yol, O_RDWR | O_CLOEXEC | (kayit ? O_NONBLOCK : 0));
	if (fd < 0) {	/* v4: mesgul yeniden denemesinde sessiz */
		int e = errno;
		if (mesgul && e == EBUSY) *mesgul = 1;
		if (!(pcm_mesgul_sessiz && e == EBUSY)) perror(yol);
		return -1;
	}
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
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &hp)) {
		int e = errno;
		if (mesgul && e == EBUSY) *mesgul = 1;
		if (!(pcm_mesgul_sessiz && e == EBUSY)) perror("HW_PARAMS");
		close(fd);
		return -1;
	}
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

/* bool mikser kontrolu: eski degeri dondurur (-1 hata). Sahte kipte dokunulmaz. */
static int bool_ayar(const char *ad, int yeni)
{
	if (ses_sahte) return -1;
	char yol[128];
	snprintf(yol, sizeof yol, "%s/controlC0", snd_kok);
	int fd = open(yol, O_RDWR | O_CLOEXEC);
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

/* sn: en fazla sure (v7 MIC: 120). baslik: MIC_AKIS satiri. */
static int mic_akis(int s, double sn, int baslik)
{
	unsigned per = 960;
	if (dosya_var(kalkan_yol)) { if (baslik) hata(s, "kalkan acik"); return 2; }
	int mesgul = 0;
	pid_t ben = getpid();
	{	/* v4: Hey Kole dinleyicisi biraksin; bayrak varsa (TTS "tts <pid>" / baska dinleme) dokunulmaz */
		char bm[32];
		int bn = snprintf(bm, sizeof bm, "kopru %d\n", (int)ben);
		int bf = open(asistan_birak_yol, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);
		if (bf >= 0) { tam_yaz(bf, bm, (size_t)bn); close(bf); }
	}
	pcm_mesgul_sessiz = 1;
	int fd = pcm_ac(1, 1, per, &mesgul);
	for (double t0 = mono(); fd < 0 && mesgul && mono() - t0 < 1.5;) {
		struct timespec bk = { .tv_nsec = 100000000 };
		nanosleep(&bk, NULL);
		fd = pcm_ac(1, 1, per, &mesgul);
	}
	pcm_mesgul_sessiz = 0;
	if (fd < 0 && mesgul) fprintf(stderr, "%s/pcmC0D0c: mikrofon mesgul (1.5 sn denendi)\n", snd_kok);	/* v4: tek satir */
	if (fd < 0) { sahipliyse_sil(asistan_birak_yol, "kopru ", ben); if (baslik) hata(s, mesgul ? "mik mesgul" : "mik yok"); return 1; }
	if (!ses_sahte && ioctl(fd, SNDRV_PCM_IOCTL_START) && errno != EBADFD) {
		perror("START"); close(fd); sahipliyse_sil(asistan_birak_yol, "kopru ", ben); if (baslik) hata(s, "mik baslamadi"); return 1;
	}
	if (baslik && tam_yaz(s, "MIC_AKIS 16000 1 s16le\n", 23)) {	/* v4: v2 birak bayragini silmeden donuyordu */
		close(fd);
		sahipliyse_sil(asistan_birak_yol, "kopru ", ben);
		return 1;
	}
	char pidm[24];
	snprintf(pidm, sizeof pidm, "%d\n", getpid());
	bayrak_yaz_icerik(mik_bayrak, 1, pidm);
	float g = getenv("MIK_KAZANC") ? (float)atof(getenv("MIK_KAZANC")) : 4.0f;
	static int16_t in[960], out[960];
	struct azalt a = {0};
	double bas = mono(), bitis = bas + (sn > 0 && sn <= 7200 ? sn : 120), kalkan_t = 0;
	unsigned long gonderilen = 0, sahte_n = 0;
	unsigned xrun = 0;
	int rc = 0;
	while (mono() < bitis) {
		if (mono() - kalkan_t > 0.5) {
			kalkan_t = mono();
			if (dosya_var(kalkan_yol)) { rc = 2; break; }	/* gizlilik kalkani: hemen dur */
		}
		struct snd_xferi x = { .buf = in, .frames = per };
		int err = 0;
		if (ses_sahte) {
			double hedef = bas + (sahte_n + per) / 48000.0;
			double d = hedef - mono();
			if (d > 0) { struct timespec ts = { 0, (long)(d * 1e9) }; nanosleep(&ts, NULL); }
			for (unsigned i = 0; i < per; i++, sahte_n++)
				in[i] = (int16_t)(2000 * sin(2 * M_PI * 1000.0 * sahte_n / 48000.0));
			x.result = per;
		} else if (ioctl(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &x)) err = errno;
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
		size_t k = azalt(&a, in, (size_t)x.result, out, ses_sahte ? 1.0f : g);
		if (tam_yaz(s, out, k * 2)) break;	/* istemci kapatti */
		gonderilen += k;
	}
	memset(in, 0, sizeof in);
	memset(out, 0, sizeof out);
	if (!ses_sahte) ioctl(fd, SNDRV_PCM_IOCTL_DROP);
	sahipliyse_sil(mik_bayrak, "", ben);	/* v4: close'dan once (sonraki sahibinki silinmesin), yalniz bizimse */
	close(fd);
	sahipliyse_sil(asistan_birak_yol, "kopru ", ben);	/* v4: yalniz bizimse; Hey Kole yeniden dinleyebilir */
	printf("MIC bitti: %lu ornek (%.1f sn) xrun=%u rc=%d\n", gonderilen, gonderilen / 16000.0, xrun, rc);
	return rc;
}

/* 48k stereo kareleri yaz; EPIPE -> 1 (yetersizlik), diger hata -1 */
static int pcm_yaz(int fd, int16_t *o, size_t k)
{
	if (ses_sahte) { ses_sahte_kare += k; return 0; }
	size_t yazilan = 0;
	while (yazilan < k) {
		struct snd_xferi x = { .buf = o + 2 * yazilan, .frames = k - yazilan };
		if (ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &x)) {
			if (errno == EINTR) continue;
			if (errno == EPIPE) { ioctl(fd, SNDRV_PCM_IOCTL_PREPARE); return 1; }
			perror("WRITEI");
			return -1;
		}
		yazilan += (size_t)x.result;
	}
	return 0;
}

static void pcm_bitir(int fd, unsigned per)
{
	static int16_t sus[960 * 2];
	memset(sus, 0, sizeof sus);
	pcm_yaz(fd, sus, per);	/* FIR gecikmesi + bir periyot sessizlik */
	if (!ses_sahte) ioctl(fd, SNDRV_PCM_IOCTL_DRAIN);
	close(fd);
}

static int cal_akis_ic(int s, long bayt)
{
	if (bayt <= 0 || bayt > 60L * 16000 * 2) { fprintf(stderr, "PLAY boyut gecersiz\n"); return 2; }
	unsigned per = 960;
	int fd = pcm_ac(0, 2, per, NULL);
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
		if (r < 0 && errno == ENOTSOCK) r = read(s, p + dolu, iste);
		if (r <= 0) break;
		alinan += r;
		dolu += (size_t)r;
		if (dolu & 1) { tek = 1; tekb = p[dolu - 1]; dolu--; }
		size_t k = arttir(&a, in, dolu / 2, out);
		int w = pcm_yaz(fd, out, k);
		if (w > 0 && ++xrun < 50) continue;
		if (w) { close(fd); return 1; }
	}
	pcm_bitir(fd, per);
	printf("PLAY bitti: %ld bayt (%.2f sn) xrun=%u\n", alinan, alinan / 32000.0, xrun);
	return alinan == bayt ? 0 : 1;
}

static int cal_akis(int s, long bayt)
{
	int eski = bool_ayar("Headphone Switch", 1);
	int rc = cal_akis_ic(s, bayt);
	if (eski >= 0) bool_ayar("Headphone Switch", eski);
	return rc;
}

#define PLAY_ON_TAMPON 2400	/* 150 ms @16k */
#define PLAY_CERCEVE 8192	/* bayt; 256 ms */

/* PLAY_AKIS: canli bas-konus. Donen 0 ok, 1 hata, 2 reddedildi */
static int play_akis(int s, double sn)
{
	char yol[160];
	if (sn <= 0 || sn > 120) sn = 60;
	if (dosya_var(kalkan_yol)) return hata(s, "kalkan acik");
	snprintf(yol, sizeof yol, "%s/pcmC0D0p", snd_kok);
	if (!ses_sahte && !dosya_var(yol)) return hata(s, "ses cikisi yok");
	int eski = bool_ayar("Headphone Switch", 1);
	unsigned per = 960;
	int mesgul = 0;
	int fd = pcm_ac(0, 2, per, &mesgul);
	if (fd < 0) {
		if (eski >= 0) bool_ayar("Headphone Switch", eski);
		return hata(s, mesgul ? "hoparlor mesgul" : "hoparlor acilamadi");
	}
	char pidm[24];
	snprintf(pidm, sizeof pidm, "%d\n", getpid());
	bayrak_yaz_icerik(hoparlor_bayrak, 1, pidm);
	int rc = 0;
	if (tam_yaz(s, "PLAY_AKIS HAZIR 16000\n", 22)) rc = 1;
	static int16_t tampon[PLAY_ON_TAMPON + PLAY_CERCEVE / 2], cerceve[PLAY_CERCEVE / 2], out[PLAY_CERCEVE / 2 * 3 * 2];
	struct arttir a = {0};
	size_t bek = 0;		/* on tampondaki ornek */
	int doluyor = 1;
	unsigned long alinan = 0, azami = (unsigned long)(sn * 16000);
	unsigned xrun = 0;
	double son_t = mono() + sn + 10;
	while (!rc && mono() < son_t) {
		uint8_t h[2];
		if (tam_oku_fd(s, h, 2)) break;			/* istemci kapatti / 3 sn sessiz */
		unsigned n = h[0] | (unsigned)h[1] << 8;
		if (!n) break;					/* duzgun son */
		if (n > PLAY_CERCEVE || (n & 1)) { rc = 2; break; }
		if (tam_oku_fd(s, cerceve, n)) break;
		size_t ornek = n / 2;
		if (alinan + ornek > azami) { rc = 3; break; }	/* sure siniri */
		alinan += ornek;
		if (doluyor) {
			memcpy(tampon + bek, cerceve, ornek * 2);
			bek += ornek;
			if (bek < PLAY_ON_TAMPON) continue;
			doluyor = 0;
			for (size_t o = 0; o < bek && !rc; o += PLAY_CERCEVE / 2) {
				size_t k = bek - o < PLAY_CERCEVE / 2 ? bek - o : PLAY_CERCEVE / 2;
				size_t m = arttir(&a, tampon + o, k, out);
				int w = pcm_yaz(fd, out, m);
				if (w < 0) rc = 1;
				else if (w > 0) { xrun++; doluyor = 1; bek = 0; break; }
			}
			if (!doluyor) bek = 0;
			continue;
		}
		size_t m = arttir(&a, cerceve, ornek, out);
		int w = pcm_yaz(fd, out, m);
		if (w < 0) rc = 1;
		else if (w > 0) { xrun++; doluyor = 1; bek = 0; }	/* ag yetersiz kaldi: yeniden 150 ms tamponla */
	}
	if (!rc && doluyor && bek) {	/* kisa konusma: on tampon dolmadan bitti -> yine de cal */
		size_t m = arttir(&a, tampon, bek, out);
		pcm_yaz(fd, out, m);
	}
	pcm_bitir(fd, per);
	memset(tampon, 0, sizeof tampon);
	memset(cerceve, 0, sizeof cerceve);
	unlink(hoparlor_bayrak);
	if (eski >= 0) bool_ayar("Headphone Switch", eski);
	char c[64];
	int m = rc == 2 ? snprintf(c, sizeof c, "HATA cerceve\n") : rc == 3 ? snprintf(c, sizeof c, "HATA sure\n")
		: snprintf(c, sizeof c, "OK %lu %u\n", alinan, xrun);
	tam_yaz(s, c, (size_t)m);
	printf("PLAY_AKIS bitti: %lu ornek (%.2f sn) xrun=%u rc=%d\n", alinan, alinan / 16000.0, xrun, rc);
	return rc;
}
#endif
