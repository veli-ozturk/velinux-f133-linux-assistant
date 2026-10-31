/* ses_kart_v2.h (20.10.2026, Claude): v1 + zamanlayici ZILI (sk_zil_baslat: 1 sn sessiz on tampon + 880/660/990 Hz uc
 * yumusak "ding", %10 genlik), ornek uretici sk_ornek(tur, j) / sk_uzunluk(tur) (tur 0 = v1 test tonu BIREBIR),
 * sk_son_tur. Panel 0.52 (scripts/388). --- v1 ---
 * ses_kart_v1.h (19.10.2026, Claude): panel 0.39 SES karti (yol haritasi v2 S5). YALNIZ F133 dahili codec'i:
 * ALSA /dev/snd/controlC0 (mikser) ve /dev/snd/pcmC0D0p (test tonu). MCU'ya (AB5301) HICBIR cerceve gondermez:
 * ses duzeyi AA'ya, kaynak A7'ye, A3/A4 kapilarina dokunulmaz (docs/ab5301-A3-geri-donus-yontemi-20261019.md).
 *   sk_yuzde_seviye(p)     : panel %0..100 -> 'Headphone volume' 0..7 (yuvarlama).
 *   sk_uygula(p, sessiz)   : degisiklik varsa 'Headphone volume' yazar. Sessizde uc cikis anahtarini 0 yapar ve
 *                            /tmp/ses-sessiz bayragini koyar (diger ses yollari icin); sessiz degilse bayragi siler.
 *                            Bosta anahtarlar ACILMAZ (S8: bosta amfi hisirtisi); calma yollari kendi acar/kapatir.
 *   sk_durum_metni(b,n,pk) : codec okumasi (duzey, anahtarlar, PCM durumu) tek satir; pk = /proc onu (test).
 *   sk_test_baslat(p, s)   : cocuk surecte 440 Hz, 1.2 sn, %3 genlik ton (ses-aux-kisa-prova-v2 ile ayni genlik);
 *                            uc anahtar calma boyunca 1, sonra eski degerlerine doner. Sessizde ya da test surerken
 *                            reddeder. Sonuc sk_test_yokla() ile sk_son_test'e: 0 tamam, 2 mesgul, 3 aygit yok,
 *                            4 hata, 5 sessiz (reddedildi).
 *   PANEL_SND_KOK=<dizin>  : test kancasi (/dev/snd yerine; host provasinda "aygit yok" yolu). */
#ifndef SES_KART_V2_H
#define SES_KART_V2_H
#define SES_KART_V1_H
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sound/asound.h>

#define SK_SESSIZ_BAYRAK "/tmp/ses-sessiz"
#define SK_HZ 48000
#define SK_PER 960
#define SK_TEST_FREK 440.0
#define SK_TEST_MS 1200
#define SK_TEST_GENLIK 983	/* %3 */
#define SK_YUMUSAK 960		/* 20 ms giris/cikis rampasi (pat sesi olmasin) */

static const char *const sk_anahtar[3] = {"Headphone Switch", "HpSpeaker Switch", "LINEOUT Switch"};
static int sk_son_duzey = -1, sk_son_sessiz = -1;
static pid_t sk_pid = -1;
static int sk_son_test = -1;	/* -1 henuz yok / suruyor */
static int sk_son_tur;	/* v2: 0 test tonu, 1 zil */

static const char *sk_kok(void)
{
	const char *k = getenv("PANEL_SND_KOK");
	return k && *k ? k : "/dev/snd";
}

static int sk_yuzde_seviye(int p)
{
	if (p < 0)
		p = 0;
	if (p > 100)
		p = 100;
	return (p * 7 + 50) / 100;
}

/* mikser denetimi (tamsayi ya da bool): *eski <- okunan ilk kanal; yaz ise tum kanallara deger. 0 tamam, -1 yok/hata */
static int sk_ctl(const char *ad, int yaz, long deger, long *eski)
{
	char yol[192];
	snprintf(yol, sizeof yol, "%s/controlC0", sk_kok());
	int fd = open(yol, O_RDWR | O_CLOEXEC);
	if (fd < 0)
		return -1;
	struct snd_ctl_elem_value v;
	memset(&v, 0, sizeof v);
	v.id.iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	snprintf((char *)v.id.name, sizeof v.id.name, "%s", ad);
	int r = -1;
	if (!ioctl(fd, SNDRV_CTL_IOCTL_ELEM_READ, &v)) {
		if (eski)
			*eski = v.value.integer.value[0];
		r = 0;
		if (yaz) {
			v.value.integer.value[0] = v.value.integer.value[1] = deger;
			if (ioctl(fd, SNDRV_CTL_IOCTL_ELEM_WRITE, &v))
				r = -1;
		}
	}
	close(fd);
	return r;
}

static int sk_uygula(int yuzde, int sessiz)
{
	int d = sk_yuzde_seviye(yuzde), r = 0;
	if (d != sk_son_duzey) {
		if (sk_ctl("Headphone volume", 1, d, NULL))
			r = -1;
		else
			sk_son_duzey = d;
	}
	if (sessiz != sk_son_sessiz) {
		if (sessiz) {
			int fd = open(SK_SESSIZ_BAYRAK, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
			if (fd >= 0)
				close(fd);
			for (int i = 0; i < 3; i++)
				if (sk_ctl(sk_anahtar[i], 1, 0, NULL))
					r = -1;
		} else
			unlink(SK_SESSIZ_BAYRAK);
		sk_son_sessiz = sessiz;
	}
	return r;
}

static const char *sk_ac_kapa(long v)
{
	return v < 0 ? "?" : v ? "açık" : "kapalı";
}

static void sk_durum_metni(char *b, size_t n, const char *proc_onek)
{
	long d = -1, s[3] = {-1, -1, -1};
	if (sk_ctl("Headphone volume", 0, 0, &d)) {
		snprintf(b, n, "Dahili codec bulunamadı (%s/controlC0).", sk_kok());
		return;
	}
	for (int i = 0; i < 3; i++)
		sk_ctl(sk_anahtar[i], 0, 0, &s[i]);
	char yol[192], st[64] = "?";
	snprintf(yol, sizeof yol, "%s/proc/asound/card0/pcm0p/sub0/status", proc_onek ? proc_onek : "");
	int fd = open(yol, O_RDONLY | O_CLOEXEC);
	if (fd >= 0) {
		ssize_t k = read(fd, st, sizeof st - 1);
		st[k > 0 ? k : 0] = 0;
		st[strcspn(st, "\n")] = 0;
		close(fd);
		if (!strncmp(st, "state: ", 7))
			memmove(st, st + 7, strlen(st + 7) + 1);
		if (!st[0])
			strcpy(st, "?");
	}
	snprintf(b, n, "Codec düzeyi %ld/7 · kulaklık %s · hoparlör %s · hat %s · PCM %s", d, sk_ac_kapa(s[0]),
		 sk_ac_kapa(s[1]), sk_ac_kapa(s[2]), st);
}

static void sk_maske(struct snd_pcm_hw_params *p, int ad, unsigned v)
{
	struct snd_mask *m = &p->masks[ad - SNDRV_PCM_HW_PARAM_FIRST_MASK];
	memset(m, 0, sizeof *m);
	m->bits[v >> 5] |= 1u << (v & 31);
}

static void sk_aralik(struct snd_pcm_hw_params *p, int ad, unsigned v)
{
	struct snd_interval *i = &p->intervals[ad - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL];
	memset(i, 0, sizeof *i);
	i->min = i->max = v;
	i->integer = 1;
}

/* v2: tur 0 = test tonu (v1 ile birebir), tur 1 = zamanlayici zili: 1 sn sessiz on tampon (amfi acilisi), sonra
 * 880 / 660 / 990 Hz uc yumusak "ding" (5 ms atak, ustel sonum 0.22 sn, sonda 20 ms rampa), %10 genlik. */
#define SK_ZIL_ON 48000
#define SK_ZIL_GENLIK 3277	/* %10 */
static const struct { double frek, sn; } sk_zil_not[3] = { { 880.0, 0.30 }, { 660.0, 0.30 }, { 990.0, 0.55 } };

static long sk_uzunluk(int tur)
{
	if (!tur)
		return (long)SK_HZ * SK_TEST_MS / 1000;
	long n = SK_ZIL_ON;
	for (int i = 0; i < 3; i++)
		n += (long)(sk_zil_not[i].sn * SK_HZ);
	return n;
}

static double sk_ornek(int tur, long j)
{
	if (!tur) {
		const long top = (long)SK_HZ * SK_TEST_MS / 1000;
		if (j < 0 || j >= top)
			return 0;
		double r = j < SK_YUMUSAK ? (double)j / SK_YUMUSAK :
			   top - j < SK_YUMUSAK ? (double)(top - j) / SK_YUMUSAK : 1.0;
		return SK_TEST_GENLIK * r * sin(2 * M_PI * SK_TEST_FREK * (double)j / SK_HZ);
	}
	if (j < SK_ZIL_ON)
		return 0;
	long k = j - SK_ZIL_ON;
	for (int i = 0; i < 3; i++) {
		long n = (long)(sk_zil_not[i].sn * SK_HZ);
		if (k < n) {
			double t = (double)k / SK_HZ;
			double atak = k < 240 ? (double)k / 240 : 1.0;
			double son = n - k < SK_YUMUSAK ? (double)(n - k) / SK_YUMUSAK : 1.0;
			return SK_ZIL_GENLIK * atak * son * exp(-t / 0.22) * sin(2 * M_PI * sk_zil_not[i].frek * t);
		}
		k -= n;
	}
	return 0;
}

/* yalniz cocuk surecte: 0 tamam, 2 mesgul, 3 aygit yok, 4 hata */
static int sk_ton_cal(int tur)
{
	char yol[192];
	snprintf(yol, sizeof yol, "%s/pcmC0D0p", sk_kok());
	int fd = open(yol, O_RDWR | O_CLOEXEC | O_NONBLOCK);	/* mesgulse beklemeden EBUSY */
	if (fd < 0)
		return errno == EBUSY ? 2 : 3;
	fcntl(fd, F_SETFL, 0);
	struct snd_pcm_hw_params hp;
	memset(&hp, 0, sizeof hp);
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_MASK; i <= SNDRV_PCM_HW_PARAM_LAST_MASK; i++)
		memset(hp.masks[i - SNDRV_PCM_HW_PARAM_FIRST_MASK].bits, 0xff, sizeof hp.masks[0].bits);
	for (int i = SNDRV_PCM_HW_PARAM_FIRST_INTERVAL; i <= SNDRV_PCM_HW_PARAM_LAST_INTERVAL; i++)
		hp.intervals[i - SNDRV_PCM_HW_PARAM_FIRST_INTERVAL].max = ~0u;
	sk_maske(&hp, SNDRV_PCM_HW_PARAM_ACCESS, SNDRV_PCM_ACCESS_RW_INTERLEAVED);
	sk_maske(&hp, SNDRV_PCM_HW_PARAM_FORMAT, SNDRV_PCM_FORMAT_S16_LE);
	sk_maske(&hp, SNDRV_PCM_HW_PARAM_SUBFORMAT, SNDRV_PCM_SUBFORMAT_STD);
	sk_aralik(&hp, SNDRV_PCM_HW_PARAM_SAMPLE_BITS, 16);
	sk_aralik(&hp, SNDRV_PCM_HW_PARAM_FRAME_BITS, 32);
	sk_aralik(&hp, SNDRV_PCM_HW_PARAM_CHANNELS, 2);
	sk_aralik(&hp, SNDRV_PCM_HW_PARAM_RATE, SK_HZ);
	sk_aralik(&hp, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, SK_PER);
	sk_aralik(&hp, SNDRV_PCM_HW_PARAM_PERIODS, 4);
	hp.rmask = ~0u;
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &hp)) {
		int m = errno == EBUSY;
		close(fd);
		return m ? 2 : 4;
	}
	struct snd_pcm_sw_params sp;
	memset(&sp, 0, sizeof sp);
	sp.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
	sp.period_step = 1;
	sp.avail_min = SK_PER;
	sp.start_threshold = SK_PER * 2;
	sp.stop_threshold = SK_PER * 4;
	sp.boundary = SK_PER * 4;
	while (sp.boundary * 2 <= 0x7fffffffu - SK_PER * 4)
		sp.boundary *= 2;
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sp) || ioctl(fd, SNDRV_PCM_IOCTL_PREPARE)) {
		close(fd);
		return 4;
	}
	long eski[3] = {0, 0, 0};
	int alindi[3];
	for (int i = 0; i < 3; i++)
		alindi[i] = !sk_ctl(sk_anahtar[i], 1, 1, &eski[i]);
	static int16_t o[SK_PER * 2];
	const long top = sk_uzunluk(tur);
	int rc = 0;
	for (long t = 0; t < top + SK_PER && !rc; t += SK_PER) {
		for (int k = 0; k < SK_PER; k++) {
			long j = t + k;
			o[2 * k] = o[2 * k + 1] = (int16_t)lrint(j < top ? sk_ornek(tur, j) : 0.0);
		}
		size_t yaz = 0;
		while (yaz < SK_PER) {
			struct snd_xferi x = {.buf = o + 2 * yaz, .frames = SK_PER - yaz};
			if (ioctl(fd, SNDRV_PCM_IOCTL_WRITEI_FRAMES, &x)) {
				if (errno == EINTR)
					continue;
				if (errno == EPIPE) {
					ioctl(fd, SNDRV_PCM_IOCTL_PREPARE);
					continue;
				}
				rc = 4;
				break;
			}
			yaz += (size_t)x.result;
		}
	}
	if (!rc)
		ioctl(fd, SNDRV_PCM_IOCTL_DRAIN);
	close(fd);
	for (int i = 0; i < 3; i++)
		if (alindi[i])
			sk_ctl(sk_anahtar[i], 1, eski[i], NULL);
	return rc;
}

/* 0 basladi, -1 zaten suruyor, -2 sessiz, -3 fork hatasi */
static int sk_baslat(int tur, int yuzde, int sessiz)
{
	if (sk_pid > 0)
		return -1;
	if (sessiz) {
		sk_son_test = 5;
		return -2;
	}
	sk_uygula(yuzde, sessiz);
	pid_t p = fork();
	if (p < 0) {
		sk_son_test = 4;
		return -3;
	}
	if (!p)
		_exit(sk_ton_cal(tur));
	sk_pid = p;
	sk_son_test = -1;
	sk_son_tur = tur;
	return 0;
}

static int sk_test_baslat(int yuzde, int sessiz)
{
	return sk_baslat(0, yuzde, sessiz);
}

/* v2: zamanlayici zili (ayni cocuk/yoklama yolu; test ile ayni anda calmaz) */
static int sk_zil_baslat(int yuzde, int sessiz)
{
	return sk_baslat(1, yuzde, sessiz);
}

/* test bittiyse 1 (sonuc sk_son_test'te) */
static int sk_test_yokla(void)
{
	if (sk_pid <= 0)
		return 0;
	int st = 0;
	pid_t r = waitpid(sk_pid, &st, WNOHANG);
	if (!r)
		return 0;
	sk_pid = -1;
	sk_son_test = r > 0 && WIFEXITED(st) ? WEXITSTATUS(st) : 4;
	return 1;
}

static const char *sk_test_metni(void)
{
	switch (sk_son_test) {
	case 0: return sk_son_tur ? "Zamanlayıcı zili çalındı." :
			"Test tonu çalındı (440 Hz, 1.2 sn). Duymadıysan hoparlör amfisi MCU tarafında kapalı olabilir (A3).";
	case 2: return "Ses çıkışı meşgul (başka bir çalma sürüyor).";
	case 3: return "Dahili codec bulunamadı; test çalınamadı.";
	case 5: return "Sessiz açık: önce sessizi kapat.";
	case 4: return "Test sırasında hata oluştu.";
	default: return sk_pid > 0 ? "Test tonu çalıyor..." : "";
	}
}
#endif
