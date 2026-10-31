/* kole_asistan v19 (23.10.2026, Claude; scripts/597, denetim-2026-10-23 bulgu 5): v18 + soru kaydi sirasinda kalkan / kapali /
 * uzaktan dinleme gelirse soru IPTAL (D_DINLE): v18 durumu D_SORU birakiyordu, mikrofon yeniden acilinca kayit uyandirma
 * sozcugu olmadan suruyor ve sonrasi Gemini'ye gidiyordu. Gerisi v18 ile ayni.
 * kole_asistan v18 (23.10.2026, Claude; scripts/566): v17 + Hey Kole sorusu once kole-gemini v40 soru soketine
 * (/tmp/kole-gemini.sock, gemini_soket_v1.h): Go sureci acilmaz; soket yoksa v17 gibi fork + exec. Kararlar ayni.
 * kole_asistan v17 (21.10.2026, Claude; scripts/492): v16 + SURE KAPISI: sablon uzunluklarinin [0.7 x en kisa,
 * 1.4 x en uzun] disindaki parca karsilastirilmaz (yanlis uyanmalarin hepsi ~2 kat uzun parcalardi). Aralik ici
 * kararlar v16 ile ayni; /data/asistan/sure-kapisi-kapali ile kapanir.
 * kole_asistan v16 (21.10.2026, Claude; scripts/483): v15 + azalt_v2.h (48 -> 16 kHz FIR: 4 cikis ic ice, her cikis
 * v1 ile ayni sirada; host 1.99x, cikti bit bit ayni). Davranis degismez.
 * kole_asistan v15 (21.10.2026, Claude): v14 + Hey Kole ayni icerikli kole-gemini-v19'u calistirir (Telegram
 * sureciyle ortak sayfa onbellegi; scripts/474). Baska degisiklik yok.
 * kole_asistan v14 (21.10.2026, Claude): v13 + uyandirma_v6.h (tembel kepstrum: sessizlikte FFT yok; kararlar bit bit
 * ayni, scripts/472). Baska degisiklik yok.
 * kole_asistan v13 (21.10.2026, Claude): v12 + CPU (olcum %7-8 bosta): uyandirma_v5.h (pencere halka tamponu, seyrek mel),
 * azalt_v1.h (FIR halka tamponu), mikrofon poll() (5 ms yoklama yerine). Davranis ve ozellikler bit bit ayni.
 * Cihazda "kole-asistan-v8" adiyla. scripts/430.
 * kole_asistan v12 (20.10.2026, Claude): v11 + uyarlanir konusma sonu (soru_algi: taban = son 3 sn 100 ms ortalamalarinin
 * en kucugu; U.gurultu mikrofon yeniden acilinca -1 oluyor, esik 22 dB kaliyordu -> her soru 10 sn), kayit enerjisi
 * gunlugu (tek satir ozet), bostayken kendi dosyasi degisince cikis (autorun yeni surumu baslatir). Cihazda
 * "kole-asistan-v8" adiyla. scripts/398.
 * kole_asistan v11 (20.10.2026, Claude): v10 + cevap olayinda "soru" (kole-gemini'nin anladigi soru; panel 0.50
 * "Anladigim: ..."), 690 bayt ustunde soru dusurulur (panel 0.49 uyumu). Cihazda "kole-asistan-v8" adiyla. scripts/369.
 * kole_asistan v10 (20.10.2026, Claude): v9 + sabit cumleler sesli (kole-gemini v15 hazir sesleri, SD
 * /mnt/sd/velinux/asistan/sesler/<fnv1a32>.pcm; kilit /tmp/tts-caliyor + /tmp/tts-sure), karakter karsilamasi
 * hazir sesle (yoksa selam.pcm). Cihazda "kole-asistan-v8" adiyla. scripts/359.
 * kole_asistan v9 (20.10.2026, Claude): v8 + uyaninca sesli karsilama (/data/asistan/selam.pcm, kole-gemini v14 uretir),
 * kopru PLAY, soru kaydi karsilamadan sonra. Cihazda "kole-asistan-v8" adiyla (autorun sabit adi). scripts/351.
 * kole_asistan v8 (15.10.2026, Claude): v7 + en yuksek kole-gemini-vN (scripts/213).
 * kole_asistan v7 (15.10.2026, Claude): v6 + Gemini istemcisi en yeni surum (v4 > v3 > v2) (scripts/204).
 * kole_asistan v6 (15.10.2026, Claude): v5 + GEMINI soru-cevap (scripts/191).
 * kole_asistan v5 (15.10.2026, Claude): v4 + karakter (yuz.conf) cevap uslubu (scripts/184).
 * kole_asistan v4 (14.10.2026, Claude): v3 + esik = sablon en yakin komsu ortalamasi x 1.2 (dosya: /data/asistan/esik-carpan,
 * 1.0..3.0), gunluk satirlarinda saat (scripts/174).
 * kole_asistan v3 (14.10.2026, Claude): v2 + OLCUM: /tmp/asistan-ozellik-kaydet varken her parcanin ham kepstrumu
 * /tmp/asistan-parca/<ogren|parca>-<sira>.bin (int n + n*12 float; ses degil, RAM, en cok 80 dosya) (scripts/172).
 * kole_asistan v2 (14.10.2026, Claude): v1 + uyandirma_v2.h, tutarli ornek secimi (en fazla 8 deneme), esik her
 * yuklemede sablonlardan, SIGTERM temizligi (scripts/170). Ilk cihaz ogrenmesinde v1 esigi 1.6e8 cikip her sese cevap verdi.
 * --- v1 ---
 * kole_asistan v1 (14.10.2026, Claude): "Hey Köle" uyandirma dinleyicisi (cihazda, PC'siz). kullanıcı: "hey köle diyince
 * cevap versin, buyurun efendim gibi; benimle hep efendi seklinde konussun, o bir köle oldugunu bilsin".
 * Mikrofon: dahili codec /dev/snd/pcmC0D0c 48 kHz mono S16 (kole-kopru v8 ses_akis_v1.h ile ayni ayar, kazanc 4),
 *   FIR ile 16 kHz -> uyandirma_v1.h (MFCC + konusma parcasi + DTW sablon).
 * Mikrofonu BIRAKIR (kapatir) ve beklemeye gecer:
 *   /data/asistan/kapali (panel: asistan kapali) | /tmp/gizlilik-kalkani (panel kalkani) |
 *   /tmp/asistan-mik-birak (kole-kopru v9: uzaktan Dinle/RTSP mikrofonu istiyor) | sablon yok ve ogrenme yok.
 * Dinlerken /tmp/asistan-dinliyor bayragi (panel gostergesi). /tmp/mik-aktif'e DOKUNMAZ (o uzaktan dinleme demek).
 * Komutlar /tmp/asistan-komut (tek satir, okununca silinir): ogren | iptal | sil | test (panel yuz animasyonu denemesi)
 * Cikti: /tmp/asistan-durum.json (saniyede bir), /tmp/asistan-olay.json {"seq","tur","metin"} (panel okur).
 * Sablonlar /data/asistan/sablon.bin: yalniz kepstrum dizileri (HAM SES SAKLANMAZ, diske ses yazilmaz).
 * Ayar /data/asistan/esik-carpan (istege bagli, ornek 1.6). Gunluk satirlari stdout (autorun: /tmp/asistan.log).
 * Calistirma: kole-asistan [--self-test] */
#define _GNU_SOURCE
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sound/asound.h>
#include <sys/socket.h>	/* v9 */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>	/* v13 */
#include "uyandirma_v6.h"	/* v14 (v13: v5) */
#include "gemini_soket_v1.h"	/* v18 */

#define KOK "/data/asistan"
#define SABLON KOK "/sablon.bin"
#define KAPALI KOK "/kapali"
#define CARPAN KOK "/esik-carpan"
#define SURE_KAPALI KOK "/sure-kapisi-kapali"	/* v17 */
#define KALKAN "/tmp/gizlilik-kalkani"
#define BIRAK "/tmp/asistan-mik-birak"
#define DINLIYOR "/tmp/asistan-dinliyor"
#define KOMUT "/tmp/asistan-komut"
/* v9 karsilama sesi */
static const char *selam_pcm = KOK "/selam.pcm", *selam_txt = KOK "/selam.txt", *tts_kapali = KOK "/tts-kapali",
		  *kopru_anahtar = "/data/ses-anahtar", *tts_caliyor = "/tmp/tts-caliyor";
static int kopru_port = 8767;
static char selam_metin[128];
/* v10: hazir sesler (kole-gemini v15 uretir) */
static const char *hazir_dizin = "/mnt/sd/velinux/asistan/sesler", *tts_sure = "/tmp/tts-sure";
static char bekleyen[200];	/* ana dongunun calacagi PCM */
static int bekleyen_soru;	/* calindiktan sonra soru kaydi (karsilama) */
#define DURUM "/tmp/asistan-durum.json"
#define OLAY "/tmp/asistan-olay.json"
#define OGREN_N 5		/* hedef tutarli ornek */
#define OGREN_AZAMI 8		/* en fazla deneme */
#define PER 960
#define TAP 31
#include "azalt_v2.h"	/* v16: dorder cikis (v13: azalt_v1.h halka) */

/* v5: karakter (panel Ayarlar > KOLE) -> cevap kumesi; hepsinde "efendim" */
static const char *const CEVAP_SADIK[] = { "Buyurun efendim.", "Emredin efendim.", "Sizi dinliyorum efendim.",
					   "Buyurun efendim, emrinizdeyim.", "Sadık köleniz burada efendim." };
static const char *const CEVAP_USAK[] = { "Buyurunuz efendim, hizmetinizdeyim.", "Emirlerinizi bekliyorum efendim.",
					  "Sizi dinliyorum, efendim.", "Emredersiniz efendim." };
static const char *const CEVAP_NESELI[] = { "Bip bop! Buyurun efendim!", "Kölenizi çağırdınız mı efendim? Buradayım!",
					    "Emret efendim, hemen hallederim!", "Hazır ve nazırım efendim!" };
static int karakter_oku(void)
{
	FILE *f = fopen("/data/asistan/yuz.conf", "r");
	int k = 0, v;
	char a[32];
	if (!f) return 0;
	while (fscanf(f, " %31[^=]=%d", a, &v) == 2)
		if (!strcmp(a, "karakter") && v >= 0 && v <= 2) k = v;
	fclose(f);
	return k;
}
static const char *cevap_sec(unsigned long n)
{
	switch (karakter_oku()) {
	case 1: return CEVAP_USAK[n % (sizeof CEVAP_USAK / sizeof *CEVAP_USAK)];
	case 2: return CEVAP_NESELI[n % (sizeof CEVAP_NESELI / sizeof *CEVAP_NESELI)];
	default: return CEVAP_SADIK[n % (sizeof CEVAP_SADIK / sizeof *CEVAP_SADIK)];
	}
}

static struct uy U;
static struct uy_sablon S[UY_SABLON_N];
static int sn;
static float esik, ort_uzak, son_skor = -1;
static int ogren = -1;	/* -1 kapali, 0..OGREN_N-1 toplanan */
static struct uy_sablon og[OGREN_AZAMI];
static volatile sig_atomic_t dur;
static void dur_isle(int s) { (void)s; dur = 1; }
static unsigned olay_seq;
static int mod_kodu;	/* 0 dinliyor, 1 ogren, 2 bekliyor */
static const char *bekleme_nedeni = "";
static unsigned long uyandi_say, parca_say;
static float gurultu_db = 0;

static double mono(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec / 1e9;
}

static int var(const char *y)
{
	struct stat st;
	return !stat(y, &st);
}

static void atomik_yaz(const char *yol, const char *icerik)
{
	char t[160];
	snprintf(t, sizeof t, "%s.part", yol);
	FILE *f = fopen(t, "w");
	if (!f) return;
	fputs(icerik, f);
	if (fclose(f) == 0) rename(t, yol);
	else unlink(t);
}

/* v6: JSON kacisi (v11: ayri islev) */
static void json_kacis(const char *m, char *k, size_t kn)
{
	size_t j = 0;
	for (const unsigned char *p = (const unsigned char *)m; *p && j < kn - 8; p++) {
		if (*p == '"' || *p == '\\') { k[j++] = '\\'; k[j++] = (char)*p; }
		else if (*p < 0x20) k[j++] = ' ';
		else k[j++] = (char)*p;
	}
	k[j] = 0;
}

static const char *olay_yol = OLAY;	/* v11: self-test gecici dosyaya yazar */

/* v11: soru (bos olabilir) panel 0.50'de "Anladigim: ..."; panel 0.49 ilk 699 bayti okur -> 690 ustunde soru dusurulur */
static void olay_ek(const char *tur, const char *metin, const char *soru)
{
	char k[1100], q[400], b[1700];
	int n = -1, sorulu = 0;
	json_kacis(metin, k, sizeof k);
	if (soru && *soru) {
		json_kacis(soru, q, sizeof q);
		n = snprintf(b, sizeof b, "{\"seq\":%u,\"tur\":\"%s\",\"metin\":\"%s\",\"soru\":\"%s\"}\n", olay_seq + 1, tur, k, q);
		sorulu = n > 0 && n <= 690;
	}
	if (!sorulu)
		snprintf(b, sizeof b, "{\"seq\":%u,\"tur\":\"%s\",\"metin\":\"%s\"}\n", olay_seq + 1, tur, k);
	olay_seq++;
	atomik_yaz(olay_yol, b);
	printf("olay %u %s (%zu bayt%s)\n", olay_seq, tur, strlen(metin), sorulu ? ", soru" : "");	/* v6: metin gunluge yazilmaz */
	fflush(stdout);
}

static void olay(const char *tur, const char *metin)
{
	olay_ek(tur, metin, NULL);
}

static void durum_yaz(void)
{
	static const char *MOD[] = { "dinliyor", "ogren", "bekliyor" };
	char b[512];
	snprintf(b, sizeof b, "{\"surum\":\"kole-asistan-v8\",\"mod\":\"%s\",\"neden\":\"%s\",\"sablon\":%d,"
		 "\"ogren_n\":%d,\"ogren_toplam\":%d,\"esik\":%.2f,\"ort\":%.2f,\"son_skor\":%.2f,\"uyandi\":%lu,"
		 "\"parca\":%lu,\"gurultu_db\":%.1f,\"seq\":%u}\n",
		 MOD[mod_kodu], bekleme_nedeni, sn, ogren < 0 ? 0 : ogren, OGREN_N, esik, ort_uzak, son_skor, uyandi_say,
		 parca_say, gurultu_db, olay_seq);
	atomik_yaz(DURUM, b);
}

/* ---------------------------------------------------------------- sablon dosyasi */
static float carpan_oku(void)
{
	FILE *f = fopen(CARPAN, "r");
	float c = UY_NN_CARPAN;
	if (f) {
		if (fscanf(f, "%f", &c) != 1 || c < 1.0f || c > 3.0f) c = UY_NN_CARPAN;
		fclose(f);
	}
	return c;
}

static int sablon_yukle(void)
{
	FILE *f = fopen(SABLON, "rb");
	sn = 0;
	if (!f) return 0;
	char m[8];
	int n = 0;
	if (fread(m, 1, 8, f) == 8 && !memcmp(m, "KOLEUY1", 8) && fread(&n, sizeof n, 1, f) == 1 && n >= 2 &&
	    n <= UY_SABLON_N && fread(&esik, sizeof esik, 1, f) == 1 && fread(&ort_uzak, sizeof ort_uzak, 1, f) == 1) {
		int i;
		for (i = 0; i < n; i++) {
			if (fread(&S[i].n, sizeof S[i].n, 1, f) != 1 || S[i].n < 2 || S[i].n > UY_EN_UZUN + 2 * UY_ON_KARE ||
			    fread(S[i].k, sizeof(float) * UY_BOY, (size_t)S[i].n, f) != (size_t)S[i].n)
				break;
		}
		if (i == n) sn = n;
	}
	fclose(f);
	if (sn) {	/* v2: esik kayittan degil, sablonlardan yeniden (bozuk v1 kaydi etkisiz); v4: en yakin komsu */
		uy_esik_nn(S, sn, 1.0f, &ort_uzak);
		esik = ort_uzak * carpan_oku();
		if (ort_uzak <= 0 || ort_uzak > 40) {
			printf("sablon esigi gecersiz (ort %.2f): sablon yok sayildi, yeniden ogrenin\n", ort_uzak);
			sn = 0;
			esik = ort_uzak = 0;
		}
	}
	return sn;
}

static int sablon_kaydet(void)
{
	mkdir(KOK, 0700);
	FILE *f = fopen(SABLON ".part", "wb");
	if (!f) return -1;
	int ok = fwrite("KOLEUY1", 1, 8, f) == 8 && fwrite(&sn, sizeof sn, 1, f) == 1 &&
		 fwrite(&esik, sizeof esik, 1, f) == 1 && fwrite(&ort_uzak, sizeof ort_uzak, 1, f) == 1;
	for (int i = 0; ok && i < sn; i++)
		ok = fwrite(&S[i].n, sizeof S[i].n, 1, f) == 1 &&
		     fwrite(S[i].k, sizeof(float) * UY_BOY, (size_t)S[i].n, f) == (size_t)S[i].n;
	if (fclose(f) || !ok) { unlink(SABLON ".part"); return -1; }
	chmod(SABLON ".part", 0600);
	return rename(SABLON ".part", SABLON);
}

/* ---------------------------------------------------------------- ses (ses_akis_v1.h ile ayni ayar) */
/* v13: FIR azaltici azalt_v1.h'de (halka tampon) */

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

static int mik_ac(int *mesgul)
{
	*mesgul = 0;
	int fd = open("/dev/snd/pcmC0D0c", O_RDWR | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0) { *mesgul = errno == EBUSY; return -1; }
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
	aralik(&hp, SNDRV_PCM_HW_PARAM_FRAME_BITS, 16);
	aralik(&hp, SNDRV_PCM_HW_PARAM_CHANNELS, 1);
	aralik(&hp, SNDRV_PCM_HW_PARAM_RATE, 48000);
	aralik(&hp, SNDRV_PCM_HW_PARAM_PERIOD_SIZE, PER);
	aralik(&hp, SNDRV_PCM_HW_PARAM_PERIODS, 4);
	hp.rmask = ~0u;
	if (ioctl(fd, SNDRV_PCM_IOCTL_HW_PARAMS, &hp)) { *mesgul = errno == EBUSY; close(fd); return -1; }
	struct snd_pcm_sw_params sp;
	memset(&sp, 0, sizeof sp);
	sp.tstamp_mode = SNDRV_PCM_TSTAMP_NONE;
	sp.period_step = 1;
	sp.avail_min = PER;
	sp.start_threshold = 1;
	sp.stop_threshold = PER * 4;
	sp.boundary = PER * 4;
	while (sp.boundary * 2 <= 0x7fffffffu - PER * 4) sp.boundary *= 2;
	if (ioctl(fd, SNDRV_PCM_IOCTL_SW_PARAMS, &sp) || ioctl(fd, SNDRV_PCM_IOCTL_PREPARE) ||
	    (ioctl(fd, SNDRV_PCM_IOCTL_START) && errno != EBADFD)) {
		close(fd);
		return -1;
	}
	return fd;
}

static void mik_kapat(int *fd)
{
	if (*fd >= 0) {
		ioctl(*fd, SNDRV_PCM_IOCTL_DROP);
		close(*fd);
		*fd = -1;
	}
	unlink(DINLIYOR);
}

/* v3 olcum dokumu: yalniz bayrak varken; ses degil, kepstrum sayilari */
static void ozellik_dok(const char *tur)
{
	static unsigned sira;
	if (!var("/tmp/asistan-ozellik-kaydet")) return;
	mkdir("/tmp/asistan-parca", 0700);
	char y[96];
	snprintf(y, sizeof y, "/tmp/asistan-parca/%s-%03u.bin", tur, sira % 80);
	sira++;
	FILE *f = fopen(y, "wb");
	if (!f) return;
	fwrite(&U.parca_ham.n, sizeof U.parca_ham.n, 1, f);
	fwrite(U.parca_ham.k, sizeof(float) * UY_BOY, (size_t)U.parca_ham.n, f);
	fclose(f);
	printf("dokum %s (%d kare)\n", y, U.parca_ham.n);
}

/* ---------------------------------------------------------------- v6 Gemini soru-cevap */
/* v8: /mnt/sd/velinux/asistan/kole-gemini-vN icinden calistirilabilir EN YUKSEK N (yeni surumde kod degismesin) */
static char gemini_yol[300] = "/mnt/sd/velinux/asistan/kole-gemini-v4";
static const char *gemini_dizin(void)	/* v15: KOLE_GEMINI_DIZIN yalniz sinama */
{
	const char *e = getenv("KOLE_GEMINI_DIZIN");
	return e && *e ? e : "/mnt/sd/velinux/asistan";
}

/* v15: iki dosya bayt bayt ayni mi (64 KB parcalarla) */
static int dosya_ayni(const char *a, const char *b)
{
	static unsigned char ba[65536], bb[65536];
	FILE *fa = fopen(a, "rb"), *fb = fopen(b, "rb");
	int ayni = fa && fb;
	while (ayni) {
		size_t na = fread(ba, 1, sizeof ba, fa), nb = fread(bb, 1, sizeof bb, fb);
		if (na != nb || memcmp(ba, bb, na))
			ayni = 0;
		if (na < sizeof ba)
			break;
	}
	if (fa)
		fclose(fa);
	if (fb)
		fclose(fb);
	return ayni;
}

/* v15: autorun Telegram/Pano dongusu kole-gemini-v19'u calistirir; en yuksek vN onunla bayt bayt ayniysa v19 (ortak sayfa
 * onbellegi: kod SD'den bir kez okunur, bellekte bir kez durur). Karar (yol, boyut, mtime) degisene kadar saklanir. */
static struct { char yol[300]; long long boy, boy19; long mt, mt19; int ayni; } gem_kiyas;
static char gemini_v19[300];

static const char *gemini_ortak(const char *en_yol)
{
	struct stat sa, sb;
	snprintf(gemini_v19, sizeof gemini_v19, "%s/kole-gemini-v19", gemini_dizin());
	if (!strcmp(en_yol, gemini_v19) || stat(en_yol, &sa) || stat(gemini_v19, &sb) || access(gemini_v19, X_OK) ||
	    sa.st_size != sb.st_size)
		return en_yol;
	if (strcmp(gem_kiyas.yol, en_yol) || gem_kiyas.boy != (long long)sa.st_size || gem_kiyas.mt != (long)sa.st_mtime ||
	    gem_kiyas.boy19 != (long long)sb.st_size || gem_kiyas.mt19 != (long)sb.st_mtime) {
		snprintf(gem_kiyas.yol, sizeof gem_kiyas.yol, "%s", en_yol);
		gem_kiyas.boy = sa.st_size;
		gem_kiyas.mt = sa.st_mtime;
		gem_kiyas.boy19 = sb.st_size;
		gem_kiyas.mt19 = sb.st_mtime;
		gem_kiyas.ayni = dosya_ayni(en_yol, gemini_v19);
		printf("gemini ikili: %s %s v19 -> %s\n", strrchr(en_yol, '/') + 1, gem_kiyas.ayni ? "=" : "!=",
		       gem_kiyas.ayni ? "v19 (ortak sayfa onbellegi)" : "en yuksek");
	}
	return gem_kiyas.ayni ? gemini_v19 : en_yol;
}

static const char *gemini_ikili(void)
{
	DIR *dd = opendir(gemini_dizin());
	int en = -1;
	if (dd) {
		struct dirent *e;
		while ((e = readdir(dd))) {
			int n;
			char son;
			if (sscanf(e->d_name, "kole-gemini-v%d%c", &n, &son) == 1 && n > en) {
				char y[300];
				snprintf(y, sizeof y, "%s/%s", gemini_dizin(), e->d_name);
				if (access(y, X_OK) == 0) {
					en = n;
					snprintf(gemini_yol, sizeof gemini_yol, "%s", y);
				}
			}
		}
		closedir(dd);
	}
	return gemini_ortak(gemini_yol);	/* v15 */
}
#define GEMINI_IKILI gemini_ikili()
#define GEMINI_ANAHTAR KOK "/gemini-anahtar"
#define GEMINI_KAPALI KOK "/gemini-kapali"
#define SORU_WAV "/tmp/asistan-soru.wav"
#define SORU_SOKET "/tmp/kole-gemini.sock"	/* v18: kole-gemini v40 */
#define SORU_AZAMI (16000 * 10)
#define ON_TAMPON 4800	/* 0.3 sn */
enum { D_DINLE, D_SORU, D_BEKLE };
static int durum = D_DINLE;
static int16_t soru[SORU_AZAMI], on[ON_TAMPON];
static int soru_n, on_i, on_dolu, soru_basladi, soru_sessiz, soru_cerceve;
static double soru_t0, bekle_t0;
static float soru_eacc;

/* v12: soru sonu algilama (saf; self-test sentetik enerjilerle sinar). guc = 10 ms karenin ortalama karesi */
#define SA_HALKA 30		/* 30 x 100 ms = 3 sn kayan taban penceresi */
#define SA_SESSIZ_KARE 80	/* 0.8 sn sessizlik = son */
#define SA_EN_AZ_KARE 50	/* baslangictan once bitmez (0.5 sn) */
#define SA_YOK_KARE 600		/* 6 sn baslamazsa: konusma yok */
enum { SA_DEVAM, SA_BASLADI, SA_SESSIZLIK, SA_YOK };
struct soru_algi {
	int kare, basladi, bas_kare, sessiz, konusma, halka_n, halka_i, blok_n, yum_i, yum_n, sn_n, sn_say;
	float tepe, taban, esik, halka[SA_HALKA], sn_db[10];
	double blok_acc, sn_acc, yum[5];
};
static struct soru_algi SA;

static void sa_baslat(struct soru_algi *a)
{
	memset(a, 0, sizeof *a);
	a->taban = 1e9f;
}

static int sa_kare(struct soru_algi *a, float guc)
{
	a->kare++;
	float e = 10.0f * log10f(guc + 1.0f);
	a->sn_acc += guc;
	if (++a->sn_n == 100) {
		if (a->sn_say < 10) a->sn_db[a->sn_say++] = 10.0f * log10f((float)(a->sn_acc / 100) + 1.0f);
		a->sn_acc = 0;
		a->sn_n = 0;
	}
	a->blok_acc += guc;
	if (++a->blok_n == 10) {
		a->halka[a->halka_i] = 10.0f * log10f((float)(a->blok_acc / 10) + 1.0f);
		a->halka_i = (a->halka_i + 1) % SA_HALKA;
		if (a->halka_n < SA_HALKA) a->halka_n++;
		a->blok_acc = 0;
		a->blok_n = 0;
		float m = 1e9f;
		for (int k = 0; k < a->halka_n; k++)
			if (a->halka[k] < m) m = a->halka[k];
		a->taban = m;
	}
	a->yum[a->yum_i] = guc;
	a->yum_i = (a->yum_i + 1) % 5;
	if (a->yum_n < 5) a->yum_n++;
	double ys = 0;
	for (int k = 0; k < a->yum_n; k++) ys += a->yum[k];
	float ey = 10.0f * log10f((float)(ys / a->yum_n) + 1.0f);
	if (!a->basladi) {
		int bas = a->halka_n < 2 ? e > 40.0f : (e > a->taban + 10.0f && e > 22.0f);
		if (bas) {
			a->basladi = 1;
			a->bas_kare = a->kare;
			a->tepe = ey;
			return SA_BASLADI;
		}
		return a->kare >= SA_YOK_KARE ? SA_YOK : SA_DEVAM;
	}
	if (ey > a->tepe) a->tepe = ey;
	float fark = a->tepe - a->taban;
	a->esik = fark < 16.0f ? a->taban + fark / 2 : a->taban + 8.0f;
	if (fark < 10.0f) {	/* henuz gercek taban gorulmedi (konusma ilk 200 ms'de basladi): sessizlik sayilmaz */
		a->sessiz = 0;
		a->konusma++;
	} else if (ey > a->esik) {
		a->sessiz = 0;
		a->konusma++;
	} else {
		a->sessiz++;
	}
	if (a->sessiz >= SA_SESSIZ_KARE && a->kare - a->bas_kare >= SA_EN_AZ_KARE) return SA_SESSIZLIK;
	return SA_DEVAM;
}

static void sa_ozet(const struct soru_algi *a, const char *neden)
{
	char sn[96] = "";
	size_t o = 0;
	for (int k = 0; k < a->sn_say && o < sizeof sn - 8; k++) o += (size_t)snprintf(sn + o, sizeof sn - o, " %.0f", a->sn_db[k]);
	int sure = a->basladi ? a->kare - a->bas_kare : 0;
	printf("soru bitti (%s): %.1f sn, taban %.1f tepe %.1f esik %.1f dB, konusma %%%d, sn:%s\n", neden, a->kare / 100.0,
	       a->taban, a->tepe, a->esik, sure > 0 ? 100 * a->konusma / sure : 0, sn);
}

/* v12: kendi dosyasi degisti mi (OTA .part -> rename: inode degisir). Yarim/silinmis dosyada 0 */
static const char *oz_yol;
static struct stat oz_st;
static int oz_degisti(const char *yol, const struct stat *eski)
{
	struct stat y;
	if (stat(yol, &y) || access(yol, X_OK)) return 0;
	return y.st_ino != eski->st_ino || y.st_mtime != eski->st_mtime || y.st_size != eski->st_size;
}
static pid_t g_pid = -1;
static int g_fd = -1;
static int g_soket;	/* v18: g_fd soket (surec yok) */
static char g_cikti[2048];
static size_t g_n;

static int var(const char *y);
static double mono(void);

/* v9: karsilama calinabilir mi (metni selam_metin'e) */
static int selam_var(void)
{
	struct stat st;
	if (var(tts_kapali) || var(tts_caliyor) || stat(selam_pcm, &st) || st.st_size < 3200 || st.st_size > 400000 ||
	    st.st_size % 2)
		return 0;
	FILE *f = fopen(selam_txt, "r");
	if (!f) return 0;
	if (!fgets(selam_metin, sizeof selam_metin, f)) selam_metin[0] = 0;
	fclose(f);
	selam_metin[strcspn(selam_metin, "\r\n")] = 0;
	return selam_metin[0] != 0;
}

/* v9: PCM'i kopru PLAY ile cal (bloklar, ses suresi kadar). 0: "OK 0". v10: genel (selam + hazir sesler) */
static int pcm_cal(const char *yol)
{
	int sonuc = -1, s = -1;
	char k[128] = "", bas[200], y[64] = "";
	FILE *f = fopen(kopru_anahtar, "r");
	if (f) {
		if (!fgets(k, sizeof k, f)) k[0] = 0;
		fclose(f);
	}
	k[strcspn(k, "\r\n ")] = 0;
	int fd = open(yol, O_RDONLY | O_CLOEXEC);
	struct stat st;
	char *b = NULL;
	if (!k[0] || fd < 0 || fstat(fd, &st) || st.st_size < 3200 || st.st_size > 1200000) goto bitti;
	b = malloc((size_t)st.st_size);
	if (!b || read(fd, b, (size_t)st.st_size) != st.st_size) goto bitti;
	s = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (s < 0) goto bitti;
	struct timeval tv = { 15, 0 };
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof tv);
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
	struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons((uint16_t)kopru_port), .sin_addr.s_addr = htonl(INADDR_LOOPBACK) };
	if (connect(s, (struct sockaddr *)&a, sizeof a)) goto bitti;
	int n = snprintf(bas, sizeof bas, "%s PLAY %ld\n", k, (long)st.st_size);
	if (n <= 0 || write(s, bas, (size_t)n) != n) goto bitti;
	for (off_t o = 0; o < st.st_size;) {
		ssize_t w = write(s, b + o, (size_t)(st.st_size - o));
		if (w <= 0) goto bitti;
		o += w;
	}
	size_t yn = 0;
	while (yn < sizeof y - 1) {
		ssize_t r = read(s, y + yn, 1);
		if (r <= 0 || y[yn] == '\n') break;
		yn++;
	}
	y[yn] = 0;
	sonuc = strcmp(y, "OK 0") ? -2 : 0;
bitti:
	memset(k, 0, sizeof k);
	memset(bas, 0, sizeof bas);
	free(b);
	if (fd >= 0) close(fd);
	if (s >= 0) close(s);
	return sonuc;
}

/* v10: FNV-1a 32 (kole-gemini v15 hazirYol ile ayni) */
static uint32_t fnv1a32(const char *s)
{
	uint32_t h = 2166136261u;
	for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
		h ^= *p;
		h *= 16777619u;
	}
	return h;
}

/* v10: PCM'i calmaya ayir: kilit /tmp/tts-caliyor (kole-gemini v15 cevap sesi bekler) + /tmp/tts-sure (panel 0.49
 * "cevap" altyazisi). Calma ana dongude, mikrofon kapaliyken. 1: ayrildi */
static int ses_ayir(const char *yol)
{
	struct stat st;
	if (bekleyen[0] || var(tts_kapali) || strlen(yol) >= sizeof bekleyen || stat(yol, &st) || st.st_size < 3200 ||
	    st.st_size > 1200000 || st.st_size % 2)
		return 0;
	int fd = open(tts_caliyor, O_CREAT | O_EXCL | O_WRONLY | O_CLOEXEC, 0644);
	if (fd < 0) return 0;	/* baska konusma suruyor: yalniz yazi */
	dprintf(fd, "asistan %d\n", (int)getpid());
	close(fd);
	char b[64];
	long ms = ((long)st.st_size / 2 - 16000 - 3200) * 1000 / 16000;	/* 1 sn on + 0.2 sn son sessizlik haric */
	snprintf(b, sizeof b, "on=1000 sure=%ld\n", ms > 100 ? ms : 100);
	atomik_yaz(tts_sure, b);
	snprintf(bekleyen, sizeof bekleyen, "%s", yol);
	return 1;
}

static int metin_sesi_ayir(const char *metin)
{
	char y[200];
	snprintf(y, sizeof y, "%s/%08x.pcm", hazir_dizin, (unsigned)fnv1a32(metin));
	return ses_ayir(y);
}

/* v10: sabit cumle: hazir sesi varsa calinir (yazi ile birlikte) */
static void olay_sesli(const char *tur, const char *metin)
{
	metin_sesi_ayir(metin);
	olay(tur, metin);
}

/* v10: ayrilan sesi cal (bloklar), kilidi birak; karsilamadan sonra soru kaydi. 0: calindi */
static void soru_basla(void);
static int bekleyen_cal(int *fd)
{
	mik_kapat(fd);
	double t0 = mono();
	int r = pcm_cal(bekleyen);
	const char *ad = strrchr(bekleyen, '/');
	printf("ses: %s %s (%.1f sn)\n", ad ? ad + 1 : bekleyen, r ? "calinamadi" : "calindi", mono() - t0);
	unlink(tts_sure);
	unlink(tts_caliyor);
	bekleyen[0] = 0;
	if (bekleyen_soru) {
		bekleyen_soru = 0;
		soru_basla();
	}
	return r;
}

static int gemini_var(void)
{
	return access(GEMINI_IKILI, X_OK) == 0 && var(GEMINI_ANAHTAR) && !var(GEMINI_KAPALI);
}

static void soru_basla(void)
{
	durum = D_SORU;
	soru_n = on_i = on_dolu = soru_basladi = soru_sessiz = soru_cerceve = 0;
	soru_eacc = 0;
	sa_baslat(&SA);	/* v12 */
	soru_t0 = mono();
	printf("soru dinleniyor (gurultu %.1f dB)\n", U.gurultu);
}

static int wav_yaz(const char *yol, const int16_t *x, int n)
{
	FILE *f = fopen(yol, "wb");
	if (!f) return -1;
	uint32_t veri = (uint32_t)n * 2, u32;
	uint16_t u16;
	fwrite("RIFF", 1, 4, f); u32 = 36 + veri; fwrite(&u32, 4, 1, f); fwrite("WAVEfmt ", 1, 8, f);
	u32 = 16; fwrite(&u32, 4, 1, f); u16 = 1; fwrite(&u16, 2, 1, f); u16 = 1; fwrite(&u16, 2, 1, f);
	u32 = 16000; fwrite(&u32, 4, 1, f); u32 = 32000; fwrite(&u32, 4, 1, f); u16 = 2; fwrite(&u16, 2, 1, f);
	u16 = 16; fwrite(&u16, 2, 1, f); fwrite("data", 1, 4, f); fwrite(&veri, 4, 1, f);
	fwrite(x, 2, (size_t)n, f);
	chmod(yol, 0600);
	return fclose(f);
}

static void gemini_baslat(int *fd_mik)
{
	if (wav_yaz(SORU_WAV, soru, soru_n)) {
		olay_sesli("hata", "Sorunuzu kaydedemedim efendim.");
		durum = D_DINLE;
		return;
	}
	memset(soru, 0, sizeof(int16_t) * (size_t)soru_n);
	olay_sesli("dusunuyor", "Bir saniye efendim, düşünüyorum...");
	mik_kapat(fd_mik);	/* beklerken mikrofon acik kalmasin (bellek/CPU) */
	int sk = gs_ac(SORU_SOKET, SORU_WAV);	/* v18: surekli kole-gemini surecine sor (Go sureci acilmaz) */
	if (sk >= 0) {
		g_soket = 1;
		g_pid = -1;
		g_fd = sk;
		g_n = 0;
		bekle_t0 = mono();
		durum = D_BEKLE;
		printf("gemini: %d ornek (%.1f sn) soket ile gonderiliyor\n", soru_n, soru_n / 16000.0);
		return;
	}
	int p[2];
	if (pipe(p)) { unlink(SORU_WAV); olay_sesli("hata", "Şu an cevap veremiyorum efendim."); durum = D_DINLE; return; }
	pid_t c = fork();
	if (c == 0) {
		dup2(p[1], 1);
		close(p[0]);
		close(p[1]);
		setenv("SSL_CERT_FILE", "/mnt/sd/velinux/tailscale/ca-certificates.crt", 1);
		setenv("GOMEMLIMIT", "12MiB", 1);
		setenv("GOGC", "50", 1);
		execl(GEMINI_IKILI, "kole-gemini", "--ses", SORU_WAV, (char *)NULL);
		_exit(127);
	}
	close(p[1]);
	if (c < 0) { close(p[0]); unlink(SORU_WAV); olay_sesli("hata", "Şu an cevap veremiyorum efendim."); durum = D_DINLE; return; }
	fcntl(p[0], F_SETFL, O_NONBLOCK);
	g_pid = c;
	g_fd = p[0];
	g_n = 0;
	bekle_t0 = mono();
	durum = D_BEKLE;
	printf("gemini: %d ornek (%.1f sn) gonderiliyor\n", soru_n, soru_n / 16000.0);
}

/* "<alan>":"..." dizgesini cikar (Go json: \" \\ \n \u00xx kacislari). v11: alan parametresi */
static int alan_cikar(const char *j, const char *alan, char *o, size_t on)
{
	char kal[40];
	snprintf(kal, sizeof kal, "\"%s\":\"", alan);
	const char *p = strstr(j, kal);
	if (!p) return -1;
	p += strlen(kal);
	size_t k = 0;
	while (*p && *p != '"' && k + 4 < on) {
		if (*p == '\\' && p[1]) {
			p++;
			if (*p == 'n' || *p == 't' || *p == 'r') o[k++] = ' ';
			else if (*p == 'u' && p[1] && p[2] && p[3] && p[4]) {
				unsigned v = (unsigned)strtoul((char[]){p[1], p[2], p[3], p[4], 0}, NULL, 16);
				if (v < 0x80) o[k++] = (char)v;
				else if (v < 0x800) { o[k++] = (char)(0xC0 | v >> 6); o[k++] = (char)(0x80 | (v & 63)); }
				else { o[k++] = (char)(0xE0 | v >> 12); o[k++] = (char)(0x80 | (v >> 6 & 63)); o[k++] = (char)(0x80 | (v & 63)); }
				p += 4;
			} else o[k++] = *p;
			p++;
		} else o[k++] = *p++;
	}
	o[k] = 0;
	return k ? 0 : -1;
}

static int cevap_cikar(const char *j, char *o, size_t on)
{
	return alan_cikar(j, "cevap", o, on);
}

static void gemini_bekle(void)
{
	if (g_soket) {	/* v18: soket kipi (surec yok, waitpid yapilmaz) */
		int r = gs_oku(g_fd, g_cikti, sizeof g_cikti, &g_n);
		if (r == 0) {
			if (mono() - bekle_t0 > 30) {
				printf("gemini (soket): 30 sn zaman asimi\n");
				olay_sesli("hata", "Üzgünüm efendim, cevap çok gecikti. Bir daha sorar mısınız?");
				goto bitti;
			}
			return;
		}
		char c[1100];
		int rc = r < 0 ? 99 : gs_rc(g_cikti);
		printf("gemini (soket): rc %d, %.1f sn, %zu bayt\n", rc, mono() - bekle_t0, g_n);
		if (rc == 0 && !cevap_cikar(g_cikti, c, sizeof c)) {
			char q[300];
			if (alan_cikar(g_cikti, "soru", q, sizeof q)) q[0] = 0;
			olay_ek("cevap", c, q);
		} else olay_sesli("hata", "Üzgünüm efendim, şu an bağlantı kuramıyorum.");
		goto bitti;
	}
	if (g_fd >= 0) {
		ssize_t r;
		while (g_n < sizeof g_cikti - 1 && (r = read(g_fd, g_cikti + g_n, sizeof g_cikti - 1 - g_n)) > 0) g_n += (size_t)r;
		g_cikti[g_n] = 0;
	}
	int st = 0;
	pid_t w = waitpid(g_pid, &st, WNOHANG);
	if (w == 0) {
		if (mono() - bekle_t0 > 30) {
			kill(g_pid, SIGKILL);
			waitpid(g_pid, &st, 0);
			printf("gemini: 30 sn zaman asimi\n");
			olay_sesli("hata", "Üzgünüm efendim, cevap çok gecikti. Bir daha sorar mısınız?");
			goto bitti;
		}
		return;
	}
	{
		ssize_t r;
		while (g_fd >= 0 && g_n < sizeof g_cikti - 1 && (r = read(g_fd, g_cikti + g_n, sizeof g_cikti - 1 - g_n)) > 0) g_n += (size_t)r;
		g_cikti[g_n] = 0;
		char c[1100];
		int rc = WIFEXITED(st) ? WEXITSTATUS(st) : 99;
		printf("gemini: rc %d, %.1f sn, %zu bayt\n", rc, mono() - bekle_t0, g_n);
		if (rc == 0 && !cevap_cikar(g_cikti, c, sizeof c)) {
			char q[300];
			if (alan_cikar(g_cikti, "soru", q, sizeof q)) q[0] = 0;
			olay_ek("cevap", c, q);	/* v11: anlasilan soru panelde */
		} else olay_sesli("hata", "Üzgünüm efendim, şu an bağlantı kuramıyorum.");
	}
bitti:
	if (g_fd >= 0) close(g_fd);
	g_fd = -1;
	g_pid = -1;
	g_soket = 0;	/* v18 */
	unlink(SORU_WAV);
	memset(g_cikti, 0, sizeof g_cikti);
	durum = D_DINLE;
	uy_baslat(&U);
}

/* 16 kHz ornekler (soru durumunda): 10 ms kareler, enerji esigi uyandirma ile ayni (gurultu + 9 dB) */
static void soru_besle(const int16_t *x, int n, int *fd_mik)
{
	for (int i = 0; i < n && durum == D_SORU; i++) {
		if (!soru_basladi) {
			on[on_i] = x[i];
			on_i = (on_i + 1) % ON_TAMPON;
			if (on_dolu < ON_TAMPON) on_dolu++;
		} else if (soru_n < SORU_AZAMI) {
			soru[soru_n++] = x[i];
		}
		soru_eacc += (float)x[i] * x[i];
		if (++soru_cerceve < 160) continue;
		float guc = soru_eacc / 160;
		soru_cerceve = 0;
		soru_eacc = 0;
		int r = sa_kare(&SA, guc);	/* v12: uyarlanir esik (U.gurultu burada -1'di) */
		if (!soru_basladi) {
			if (r == SA_BASLADI) {	/* on-tampon (0.3 sn) dahil basla */
				soru_basladi = 1;
				printf("soru basladi: +%.2f sn, taban %.1f dB\n", SA.kare / 100.0, SA.taban < 1e8f ? SA.taban : -1.0f);
				for (int k = 0; k < on_dolu; k++) soru[soru_n++] = on[(on_i - on_dolu + k + ON_TAMPON) % ON_TAMPON];
			} else if (r == SA_YOK) {
				sa_ozet(&SA, "konusma yok");
				printf("soru: 6 sn konusma yok\n");
				olay_sesli("bilgi", "Bir şey duyamadım efendim. Gerekirse yine seslenin.");
				durum = D_DINLE;
				uy_baslat(&U);
			}
			continue;
		}
		if (r == SA_SESSIZLIK || soru_n >= SORU_AZAMI) {
			sa_ozet(&SA, r == SA_SESSIZLIK ? "sessizlik" : "sinir");
			gemini_baslat(fd_mik);
		}
	}
}

/* ---------------------------------------------------------------- olaylar */
/* v17: sablon uzunluk araligi [0.7 x en kisa, 1.4 x en uzun] (kare; parca.n ile ayni olcu, on/arka kareler dahil) */
static void sure_araligi(const struct uy_sablon *s, int n, int *lo, int *hi)
{
	int a = 1 << 30, b = 0;
	for (int i = 0; i < n; i++) {
		if (s[i].n < a) a = s[i].n;
		if (s[i].n > b) b = s[i].n;
	}
	*lo = n ? a * 7 / 10 : 0;
	*hi = n ? b * 14 / 10 : 1 << 30;
}

static int sure_uygun(int pn, const struct uy_sablon *s, int n, int *lo, int *hi)
{
	sure_araligi(s, n, lo, hi);
	return pn >= *lo && pn <= *hi;
}

static void parca_isle(void)
{
	parca_say++;
	ozellik_dok(ogren >= 0 ? "ogren" : "parca");
	if (ogren >= 0) {
		og[ogren] = U.parca;
		printf("ogren %d: %d kare\n", ogren + 1, U.parca.n);
		ogren++;
		struct uy_sablon sec[OGREN_AZAMI];
		int k = ogren >= OGREN_N ? uy_sablon_sec(og, ogren, sec) : 0;
		if (k >= 4 || (ogren >= OGREN_AZAMI && k >= 3)) {
			memcpy(S, sec, sizeof(struct uy_sablon) * (size_t)k);
			sn = k;
			ort_uzak = 0;
			uy_esik_nn(S, sn, 1.0f, &ort_uzak);
			esik = ort_uzak * carpan_oku();
			printf("ogrenme bitti: %d/%d ornek tutarli, ortalama %.2f esik %.2f\n", k, ogren, ort_uzak, esik);
			if (ort_uzak <= 0 || ort_uzak > 40) {
				sn = 0;
				olay("hata", "Sesinizi tam anlayamadım efendim, lütfen yeniden öğretin.");
			} else if (sablon_kaydet()) {
				olay("hata", "Sesinizi kaydedemedim efendim, depolama hatası.");
			} else {
				olay("ogrendi", "Teşekkürler efendim, sesinizi öğrendim. Artık Hey Köle dediğinizde buradayım.");
			}
			ogren = -1;
		} else if (ogren >= OGREN_AZAMI) {
			printf("ogrenme basarisiz: %d denemede yalniz %d tutarli\n", ogren, k);
			olay("hata", "Söyleyişler birbirine benzemedi efendim, lütfen sessiz bir yerde yeniden öğretin.");
			ogren = -1;
			sablon_yukle();
		} else if (ogren >= OGREN_N) {
			olay("ogren", "Bir kez daha lütfen efendim.");
		}
		durum_yaz();
		return;
	}
	if (!sn) return;
	char saat[16];
	time_t tt = time(NULL);
	strftime(saat, sizeof saat, "%H:%M:%S", localtime(&tt));
	int lo, hi;
	if (!sure_uygun(U.parca.n, S, sn, &lo, &hi) && !var(SURE_KAPALI)) {	/* v17: sure kapisi */
		printf("%s parca %d kare: sure disi (sablon %d-%d kare), karsilastirilmadi\n", saat, U.parca.n, lo, hi);
		return;
	}
	son_skor = uy_en_yakin(&U.parca, S, sn);
	printf("%s parca %d kare: uzaklik %.2f esik %.2f (oran %.2f) gurultu %.1f dB\n", saat, U.parca.n, son_skor, esik,
	       ort_uzak > 0 ? son_skor / ort_uzak : 0, gurultu_db);
	static double son_uyanma;
	if (son_skor < esik && mono() - son_uyanma > 2.0) {
		son_uyanma = mono();
		uyandi_say++;
		const char *m = cevap_sec(uyandi_say);
		if (gemini_var()) {	/* v6: cevap + soruyu dinle */
			/* v10: karakterin karsilamasi hazir sesle; yoksa v9 selam.pcm; soru kaydi calindiktan sonra (ana dongu) */
			int c = metin_sesi_ayir(m);
			if (!c && selam_var() && (c = ses_ayir(selam_pcm)))
				m = selam_metin;
			bekleyen_soru = c;
			olay("dinliyor", m);
			if (!c)
				soru_basla();
		} else {
			olay_sesli("cevap", m);
		}
	}
	fflush(stdout);
}

static void komut_oku(void)
{
	char b[64] = {0};
	FILE *f = fopen(KOMUT, "r");
	if (!f) return;
	if (!fgets(b, sizeof b, f)) b[0] = 0;
	fclose(f);
	unlink(KOMUT);
	b[strcspn(b, "\r\n ")] = 0;
	printf("komut: %s\n", b);
	if (!strcmp(b, "ogren")) {
		ogren = 0;
		olay("ogren", "Lütfen beş kez, her seferinde kısa bir ara vererek Hey Köle deyin efendim.");
	} else if (!strcmp(b, "iptal")) {
		if (ogren >= 0) olay("bilgi", "Öğrenme iptal edildi efendim.");
		ogren = -1;
	} else if (!strcmp(b, "sil")) {
		unlink(SABLON);
		sn = 0;
		olay("bilgi", "Ses kaydınız silindi efendim.");
	} else if (!strcmp(b, "test")) {
		olay_sesli("cevap", cevap_sec((unsigned long)rand()));
	} else if (!strcmp(b, "yenile")) {
		sablon_yukle();
	}
	durum_yaz();
}

/* ---------------------------------------------------------------- ana dongu */
/* v9 self-test: sahte kopru sunucusu (cocuk surec) karsilamayi alir, "OK 0" doner */
static int selam_testi(void)
{
	char kok[] = "/tmp/asistan-v9-testXXXXXX";
	if (!mkdtemp(kok)) return 1;
	static char p1[96], p2[96], p3[96], p4[96], p5[96];
	snprintf(p1, sizeof p1, "%s/selam.pcm", kok);
	snprintf(p2, sizeof p2, "%s/selam.txt", kok);
	snprintf(p3, sizeof p3, "%s/anahtar", kok);
	snprintf(p4, sizeof p4, "%s/kilit", kok);
	snprintf(p5, sizeof p5, "%s/sure", kok);
	selam_pcm = p1;
	selam_txt = p2;
	kopru_anahtar = p3;
	tts_kapali = "/nonexistent/tts-kapali";
	tts_caliyor = p4;
	tts_sure = p5;
	FILE *f = fopen(p1, "wb");
	for (int i = 0; f && i < 8000; i++) { int16_t v = (int16_t)(i % 100); fwrite(&v, 2, 1, f); }
	if (f) fclose(f);
	f = fopen(p2, "w");
	if (f) { fputs("Buyurun efendim.\n", f); fclose(f); }
	f = fopen(p3, "w");
	if (f) { fputs("GIZLI99\n", f); fclose(f); }
	if (!selam_var() || strcmp(selam_metin, "Buyurun efendim.")) { printf("self-test: selam_var hatali\n"); return 1; }
	int l = socket(AF_INET, SOCK_STREAM, 0), bir = 1;
	setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &bir, sizeof bir);
	struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = 0, .sin_addr.s_addr = htonl(INADDR_LOOPBACK) };
	socklen_t al = sizeof a;
	if (l < 0 || bind(l, (struct sockaddr *)&a, sizeof a) || listen(l, 1) || getsockname(l, (struct sockaddr *)&a, &al)) return 1;
	kopru_port = ntohs(a.sin_port);
	pid_t c = fork();
	if (c == 0) {
		int s = accept(l, NULL, NULL);
		char bas[128];
		size_t n = 0;
		while (s >= 0 && n < sizeof bas - 1 && read(s, bas + n, 1) == 1 && bas[n] != '\n') n++;
		bas[n] = 0;
		long tamam = 0;
		char buf[4096];
		ssize_t r;
		while (tamam < 16000 && (r = read(s, buf, sizeof buf)) > 0) tamam += r;
		int ok = !strcmp(bas, "GIZLI99 PLAY 16000") && tamam == 16000;
		if (write(s, ok ? "OK 0\n" : "HATA\n", 5) != 5) _exit(3);
		close(s);
		_exit(ok ? 0 : 2);
	}
	close(l);
	int fdm = -1, st = 0, r = ses_ayir(selam_pcm) ? 0 : -9;
	char sb[64] = "";
	f = fopen(p5, "r");
	if (f) {
		if (!fgets(sb, sizeof sb, f)) sb[0] = 0;
		fclose(f);
	}
	if (r == 0 && (strcmp(sb, "on=1000 sure=100\n") || !var(p4))) r = -8;
	if (r == 0) r = bekleyen_cal(&fdm);
	else kill(c, SIGKILL);
	if (!r && (var(p4) || var(p5) || bekleyen[0])) r = -7;	/* kilit ve sure birakilmali */
	waitpid(c, &st, 0);
	unlink(p4);
	unlink(p5);
	unlink(p1);
	unlink(p2);
	unlink(p3);
	rmdir(kok);
	printf("self-test: karsilama calma %s (sunucu %d)\n", r == 0 ? "OK" : "HATA", WIFEXITED(st) ? WEXITSTATUS(st) : -1);
	return r != 0 || !WIFEXITED(st) || WEXITSTATUS(st) != 0;
}

/* v10 self-test: FNV vektorleri, hazir ses ayirma (kilit + tts-sure), dolu kilit ve tts-kapali reddi */
static int hazir_testi(void)
{
	static const char *DUS = "Bir saniye efendim, düşünüyorum...";
	if (fnv1a32("") != 0x811c9dc5u || fnv1a32("a") != 0xe40c292cu) { printf("self-test: fnv hatali\n"); return 1; }
	printf("self-test: fnv dusunuyor %08x\n", (unsigned)fnv1a32(DUS));
	char kok[] = "/tmp/asistan-v10-testXXXXXX";
	if (!mkdtemp(kok)) return 1;
	static char kl[96], sr[96], pc[160], dz[64];
	snprintf(dz, sizeof dz, "%s", kok);
	snprintf(kl, sizeof kl, "%s/kilit", kok);
	snprintf(sr, sizeof sr, "%s/sure", kok);
	snprintf(pc, sizeof pc, "%s/%08x.pcm", kok, (unsigned)fnv1a32(DUS));
	hazir_dizin = dz;
	tts_caliyor = kl;
	tts_sure = sr;
	tts_kapali = "/nonexistent/tts-kapali";
	FILE *f = fopen(pc, "wb");
	for (int i = 0; f && i < 16000 + 32000 + 3200; i++) { int16_t v = 0; fwrite(&v, 2, 1, f); }	/* 2 sn ses */
	if (f) fclose(f);
	bekleyen[0] = 0;
	int ok = !metin_sesi_ayir("Yok boyle bir cumle.") && metin_sesi_ayir(DUS) && !strcmp(bekleyen, pc);
	char b[64] = "";
	f = fopen(sr, "r");
	if (f) {
		if (!fgets(b, sizeof b, f)) b[0] = 0;
		fclose(f);
	}
	ok = ok && !strcmp(b, "on=1000 sure=2000\n") && var(kl);
	bekleyen[0] = 0;
	ok = ok && !metin_sesi_ayir(DUS);	/* kilit dolu (baska konusma): yalniz yazi */
	unlink(kl);
	unlink(sr);
	tts_kapali = pc;	/* var olan dosya: sesli konusma kapali */
	ok = ok && !metin_sesi_ayir(DUS) && !var(kl);
	unlink(pc);
	rmdir(kok);
	printf("self-test: hazir ses ayirma %s\n", ok ? "OK" : "HATA");
	return !ok;
}

/* v11 self-test: soru/cevap cikarma (kacislar, \u00xx), olayda soru, 690 bayt ustunde soru dusmesi */
static int soru_testi(void)
{
	char kok[] = "/tmp/asistan-v11-testXXXXXX";
	if (!mkdtemp(kok)) return 1;
	static char oy[96];
	snprintf(oy, sizeof oy, "%s/olay.json", kok);
	olay_yol = oy;
	const char *j = "{\"soru\":\"Saat ka\\u00e7 \\\"acaba\\\"?\",\"cevap\":\"Saat on efendim.\",\"eylemler\":[]}";
	char q[300], c[300], b[2000];
	int ok = !alan_cikar(j, "soru", q, sizeof q) && !strcmp(q, "Saat kaç \"acaba\"?") && !cevap_cikar(j, c, sizeof c) &&
		 !strcmp(c, "Saat on efendim.") && alan_cikar(j, "yok", q + 200, 50);
	size_t n;
	FILE *f;
	olay_ek("cevap", c, q);
	f = fopen(oy, "r");
	n = f ? fread(b, 1, sizeof b - 1, f) : 0;
	if (f) fclose(f);
	b[n] = 0;
	ok = ok && strstr(b, "\"metin\":\"Saat on efendim.\",\"soru\":\"Saat kaç \\\"acaba\\\"?\"") && n <= 699;
	char uzun[700];
	memset(uzun, 'a', 650);
	uzun[650] = 0;
	olay_ek("cevap", uzun, q);	/* panel 0.49 siniri: soru dusmeli */
	f = fopen(oy, "r");
	n = f ? fread(b, 1, sizeof b - 1, f) : 0;
	if (f) fclose(f);
	b[n] = 0;
	ok = ok && !strstr(b, "\"soru\"") && strstr(b, "\"metin\":\"aaaa") && n <= 699;
	olay("bilgi", "Deneme.");
	f = fopen(oy, "r");
	n = f ? fread(b, 1, sizeof b - 1, f) : 0;
	if (f) fclose(f);
	b[n] = 0;
	ok = ok && !strstr(b, "\"soru\"") && strstr(b, "\"tur\":\"bilgi\"");
	unlink(oy);
	rmdir(kok);
	olay_yol = OLAY;
	printf("self-test: soru olayi %s\n", ok ? "OK" : "HATA");
	return !ok;
}

/* v12 self-test: soru sonu algilama sentetik senaryolarla (dB + deterministik +-1.5 dB oynama) */
static unsigned sa_lcg = 12345;
static float sa_oyna(void)
{
	sa_lcg = sa_lcg * 1103515245u + 12345u;
	return (float)((sa_lcg >> 16) % 300) / 100.0f - 1.5f;
}

static void sa_senaryo(float taban_db, float ses_db, float bas, float son, int bosluk, float *t_bas, float *t_son, int *olay)
{
	struct soru_algi a;
	sa_baslat(&a);
	*t_bas = *t_son = -1;
	*olay = SA_DEVAM;
	for (int k = 0; k < 1000; k++) {
		float t = k / 100.0f;
		int konus = t >= bas && t < son && !(bosluk && fmodf(t - bas, 0.4f) >= 0.25f);
		float db = (konus ? ses_db : taban_db) + sa_oyna();
		int r = sa_kare(&a, powf(10.0f, db / 10.0f) - 1.0f);
		if (r == SA_BASLADI) *t_bas = t;
		if (r == SA_SESSIZLIK || r == SA_YOK) {
			*t_son = t;
			*olay = r;
			return;
		}
	}
}

static int sa_testi(void)
{
	static const struct { const char *ad; float taban, ses, bas, son; int bosluk, olay; float b0, b1, s0, s1; } T[] = {
		{"hisirti 28 dB + konusma", 28, 50, 0.5f, 2.5f, 0, SA_SESSIZLIK, 0.45f, 0.7f, 3.2f, 3.5f},
		{"sessiz oda + kelime bosluklari", 15, 45, 0.6f, 3.0f, 1, SA_SESSIZLIK, 0.55f, 0.75f, 3.55f, 3.9f},
		{"yalniz hisirti", 28, 0, 0, 0, 0, SA_YOK, -1, -1, 5.98f, 6.01f},
		{"sessiz konusma (taban+13)", 20, 33, 0.5f, 2.0f, 0, SA_SESSIZLIK, 0.45f, 0.75f, 2.7f, 3.05f},
		{"ilk 200 ms'de baslayan konusma", 20, 48, 0.05f, 2.0f, 0, SA_SESSIZLIK, 0.0f, 0.2f, 2.8f, 3.1f},
	};
	int hata = 0;
	for (size_t i = 0; i < sizeof T / sizeof *T; i++) {
		float tb, ts;
		int o;
		sa_senaryo(T[i].taban, T[i].ses, T[i].bas, T[i].son, T[i].bosluk, &tb, &ts, &o);
		int ok = o == T[i].olay && ts >= T[i].s0 && ts <= T[i].s1 && (T[i].b0 < 0 ? tb < 0 : tb >= T[i].b0 && tb <= T[i].b1);
		printf("self-test: soru sonu %-32s basla %5.2f son %5.2f %s\n", T[i].ad, tb, ts, ok ? "OK" : "HATA");
		hata |= !ok;
	}
	/* eski algoritma (sabit 22 dB) ayni hisirtida bitmezdi: belge niteliginde */
	return hata;
}

static int oz_testi(void)
{
	char kok[] = "/tmp/asistan-oz-XXXXXX", a[64], b[64];
	if (!mkdtemp(kok)) return 1;
	snprintf(a, sizeof a, "%s/ikili", kok);
	snprintf(b, sizeof b, "%s/ikili.part", kok);
	FILE *f = fopen(a, "w");
	if (!f) return 1;
	fputs("v11", f);
	fclose(f);
	chmod(a, 0755);
	struct stat st;
	int ok = !stat(a, &st) && !oz_degisti(a, &st);
	f = fopen(b, "w");
	if (!f) return 1;
	fputs("v12 yeni", f);
	fclose(f);
	ok = ok && !oz_degisti(a, &st);		/* .part yazilirken degismedi */
	chmod(b, 0755);
	ok = ok && !rename(b, a) && oz_degisti(a, &st);
	unlink(a);
	rmdir(kok);
	printf("self-test: kendini guncelleme algisi %s\n", ok ? "OK" : "HATA");
	return !ok;
}

/* v15: ortak ikili secimi (gecici dizinde sahte ikililer) */
/* v17: sure kapisi araligi (sablonlar 49..60 kare -> 34..84) */
static int sure_testi(void)
{
	static struct uy_sablon t[5];
	const int n[5] = { 50, 56, 60, 49, 50 };
	for (int i = 0; i < 5; i++) t[i].n = n[i];
	int lo, hi, hata = 0;
	const int pn[] = { 33, 34, 41, 57, 61, 84, 85, 93, 108 }, bek[] = { 0, 1, 1, 1, 1, 1, 0, 0, 0 };
	for (unsigned i = 0; i < sizeof pn / sizeof pn[0]; i++)
		if (sure_uygun(pn[i], t, 5, &lo, &hi) != bek[i]) {
			printf("self-test: sure kapisi %d kare -> %d (beklenen %d), aralik %d-%d HATA\n", pn[i], !bek[i], bek[i], lo, hi);
			hata = 1;
		}
	if (lo != 34 || hi != 84) {
		printf("self-test: sure araligi %d-%d (beklenen 34-84) HATA\n", lo, hi);
		hata = 1;
	}
	if (!hata) printf("self-test: sure kapisi %d-%d OK\n", lo, hi);
	return hata;
}

static int ortak_testi(void)
{
	char d[] = "/tmp/kole-ortak-XXXXXX", y19[300], y27[300], y20[300];
	if (!mkdtemp(d))
		return 1;
	setenv("KOLE_GEMINI_DIZIN", d, 1);
	snprintf(y19, sizeof y19, "%s/kole-gemini-v19", d);
	snprintf(y27, sizeof y27, "%s/kole-gemini-v27", d);
	snprintf(y20, sizeof y20, "%s/kole-gemini-v20", d);
	int hata = 0;
	FILE *f;
#define YAZ(yol, ic) do { f = fopen(yol, "wb"); if (f) { fputs(ic, f); fclose(f); } chmod(yol, 0755); } while (0)
	YAZ(y19, "AYNI-ICERIK-1234");
	YAZ(y27, "AYNI-ICERIK-1234");
	if (strcmp(gemini_ikili(), y19)) { printf("self-test: ortak ikili ayni -> %s (beklenen v19)\n", gemini_ikili()); hata++; }
	YAZ(y27, "AYNI-ICERIK-1235");	/* ayni boy, farkli bayt */
	struct timespec ts[2] = {{0, UTIME_OMIT}, {123456, 0}};
	utimensat(AT_FDCWD, y27, ts, 0);	/* mtime degisti -> yeniden kiyas */
	if (strcmp(gemini_ikili(), y27)) { printf("self-test: farkli icerik -> %s (beklenen v27)\n", gemini_ikili()); hata++; }
	unlink(y19);
	if (strcmp(gemini_ikili(), y27)) { printf("self-test: v19 yok -> %s\n", gemini_ikili()); hata++; }
	unlink(y27);
	YAZ(y19, "x");
	if (strcmp(gemini_ikili(), y19)) { printf("self-test: yalniz v19 -> %s\n", gemini_ikili()); hata++; }
	YAZ(y20, "UZUN-FARKLI-BOY");	/* boy farkli: kiyas yok, en yuksek */
	if (strcmp(gemini_ikili(), y20)) { printf("self-test: farkli boy -> %s\n", gemini_ikili()); hata++; }
#undef YAZ
	unlink(y19);
	unlink(y20);
	rmdir(d);
	unsetenv("KOLE_GEMINI_DIZIN");
	memset(&gem_kiyas, 0, sizeof gem_kiyas);
	printf("self-test: ortak ikili secimi %s\n", hata ? "HATA" : "OK");
	return hata != 0;
}

/* v18 self-test: soru soketi (sahte kole-gemini sunucusu cocuk surecte), yok soket -1, rc ayristirma */
static int soket_testi(void)
{
	const char *yol = "/tmp/kole-asistan-selftest.sock";
	int hata = 0;
	unlink(yol);
	if (gs_ac(yol, "/tmp/x.wav") != -1) { printf("self-test: soket yokken -1 bekleniyordu\n"); hata++; }
	if (gs_rc("{\"soru\":\"a\",\"cevap\":\"b\"}") != 0 || gs_rc("{\"rc\":6}") != 6 || gs_rc("") != 5 || gs_rc("{\"rc\":0}") != 5) {
		printf("self-test: gs_rc hatali\n");
		hata++;
	}
	int l = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un a;
	memset(&a, 0, sizeof a);
	a.sun_family = AF_UNIX;
	memcpy(a.sun_path, yol, strlen(yol) + 1);
	if (l < 0 || bind(l, (struct sockaddr *)&a, sizeof a) || listen(l, 1)) { printf("self-test: soket kurulamadi\n"); return 1; }
	pid_t c = fork();
	if (c == 0) {
		int s = accept(l, NULL, NULL);
		char b[256] = {0};
		ssize_t n = read(s, b, sizeof b - 1);
		const char *cv = "{\"soru\":\"saat kac\",\"cevap\":\"Saat \\u00fc\\u00e7 efendim.\"}\n";
		usleep(200000);	/* cevap gecikmesi: okuyucu once 0 (suruyor) gormeli */
		_exit(n > 0 && strstr(b, "{\"ses\":\"/tmp/x.wav\"}\n") && write(s, cv, strlen(cv)) == (ssize_t)strlen(cv) ? 0 : 3);
	}
	close(l);
	int fd = gs_ac(yol, "/tmp/x.wav");
	char buf[512];
	size_t n = 0;
	int r = 0, sur = 0;
	double t0 = mono();
	while (fd >= 0 && (r = gs_oku(fd, buf, sizeof buf, &n)) == 0 && mono() - t0 < 5) { sur++; usleep(20000); }
	int st = 0;
	waitpid(c, &st, 0);
	char cv[200], q[100];
	if (fd < 0 || r != 1 || !sur || gs_rc(buf) != 0 || cevap_cikar(buf, cv, sizeof cv) || strcmp(cv, "Saat üç efendim.") ||
	    alan_cikar(buf, "soru", q, sizeof q) || strcmp(q, "saat kac") || !WIFEXITED(st) || WEXITSTATUS(st)) {
		printf("self-test: soket sor/oku HATA (fd %d r %d sur %d cevap '%s')\n", fd, r, sur, n ? buf : "");
		hata++;
	}
	if (fd >= 0) close(fd);
	unlink(yol);
	if (gs_ac(yol, "/tmp/a\"b.wav") != -1) { printf("self-test: tirnakli yol reddedilmeli\n"); hata++; }
	if (!hata) printf("self-test: soru soketi OK (sahte sunucu, %d bekleme turu)\n", sur);
	return hata;
}

static int self_test(void)
{
	if (selam_testi() || hazir_testi() || soru_testi() || sa_testi() || oz_testi() || ortak_testi() || sure_testi() ||
	    soket_testi())	/* v12: sa + oz; v15; v18 soket */
		return 1;
	/* ses aygiti acilmaz: ozellik zinciri + sablon dosyasi gidis-donus (gecici kok) */
	uy_baslat(&U);
	int16_t x[1600] = {0};
	int t, n = 0;
	for (int i = 0; i < 50; i++) n += uy_besle(&U, x, 1600, &t);
	printf("self-test: sessizlikte parca %d (beklenen 0), kare %ld\n", n, U.kare_no);
	return n != 0;
}

int main(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "--self-test")) return self_test();
	if (argc > 0 && argv[0][0] == '/' && !stat(argv[0], &oz_st)) oz_yol = argv[0];	/* v12: kendini guncelleme */
	setvbuf(stdout, NULL, _IOLBF, 0);
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = dur_isle;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	fir_hazirla();
	uy_baslat(&U);
	sablon_yukle();
	unlink(DINLIYOR);
	{	/* v10: onceki calismadan kalan kendi kilidimiz */
		char b[32] = "";
		FILE *f = fopen(tts_caliyor, "r");
		if (f) {
			if (!fgets(b, sizeof b, f)) b[0] = 0;
			fclose(f);
			if (!strncmp(b, "asistan ", 8)) {
				unlink(tts_caliyor);
				unlink(tts_sure);
				printf("bayat ses kilidi silindi\n");
			}
		}
	}
	srand((unsigned)time(NULL));
	olay_seq = (unsigned)time(NULL) % 100000 * 10;	/* yeniden baslayinca panel eski olayi tekrar gostermesin */
	printf("kole-asistan v13: sablon %d, esik %.2f (ort %.2f x %.2f)\n", sn, esik, ort_uzak, carpan_oku());
	int fd = -1;
	static int16_t in[PER], out[PER];
	struct azalt a = {0};
	double son_kontrol = 0, son_durum = 0, bekle_t = 0, son_oz = 0;
	int yeni_surum = 0;
	while (!dur) {
		double now = mono();
		if (now - son_kontrol > 0.1) {
			son_kontrol = now;
			if (var(KOMUT)) komut_oku();
			const char *neden = durum == D_BEKLE ? "gemini bekleniyor" : var(KAPALI) ? "kapali" : var(KALKAN) ? "kalkan" : var(BIRAK) ? "uzaktan dinleme"
					   : (!sn && ogren < 0) ? "sablon yok" : NULL;
			if (neden) {
				if (durum == D_SORU) {	/* v19: yarim soru surmesin */
					durum = D_DINLE;
					uy_baslat(&U);
					olay("bilgi", "Dinlemeyi bıraktım efendim.");
					printf("soru iptal: %s\n", neden);
				}
				if (fd >= 0) printf("mikrofon birakildi: %s\n", neden);
				mik_kapat(&fd);
				mod_kodu = 2;
				bekleme_nedeni = neden;
			} else if (fd < 0 && now >= bekle_t) {
				int mesgul;
				fd = mik_ac(&mesgul);
				if (fd < 0) {
					bekle_t = now + 1.0;
					mod_kodu = 2;
					bekleme_nedeni = mesgul ? "mikrofon mesgul" : "mikrofon acilamadi";
				} else {
					uy_baslat(&U);
					memset(&a, 0, sizeof a);
					FILE *f = fopen(DINLIYOR, "w");
					if (f) fclose(f);
					printf("mikrofon acildi (%s)\n", ogren >= 0 ? "ogren" : "dinle");
				}
			}
			if (fd >= 0) { mod_kodu = ogren >= 0 ? 1 : 0; bekleme_nedeni = ""; }
		}
		if (now - son_durum > 1.0) { son_durum = now; gurultu_db = U.gurultu; durum_yaz(); }
		if (oz_yol && now - son_oz > 3.0) {	/* v12: bostayken yeni surum -> cik, autorun 5 sn sonra baslatir */
			son_oz = now;
			if (durum == D_DINLE && !bekleyen[0] && ogren < 0 && g_pid < 0 && oz_degisti(oz_yol, &oz_st)) {
				printf("yeni surum algilandi (%s): cikiliyor, autorun yeniden baslatir\n", oz_yol);
				yeni_surum = 1;
				break;
			}
		}
		if (bekleyen[0]) {	/* v10: hazir ses / karsilama (mikrofon kapatilir) */
			bekleyen_cal(&fd);
			bekle_t = 0;
			continue;
		}
		if (durum == D_BEKLE) {	/* v6: Gemini cevabi bekleniyor (mikrofon kapali) */
			gemini_bekle();
			struct timespec d = { 0, 100000000 };
			nanosleep(&d, NULL);
			continue;
		}
		if (fd < 0) {
			struct timespec d = { 0, 100000000 };
			nanosleep(&d, NULL);
			continue;
		}
		struct snd_xferi x = { .buf = in, .frames = PER };
		if (ioctl(fd, SNDRV_PCM_IOCTL_READI_FRAMES, &x)) {
			if (errno == EAGAIN) {	/* v13: veri gelene kadar uyu (5 ms yoklama yerine; 100 ms'de bir denetimler) */
				struct pollfd p = { .fd = fd, .events = POLLIN };
				poll(&p, 1, 100);
				continue;
			}
			if (errno == EPIPE) { ioctl(fd, SNDRV_PCM_IOCTL_PREPARE); ioctl(fd, SNDRV_PCM_IOCTL_START); continue; }
			printf("READI hata %d: mikrofon yeniden acilacak\n", errno);
			mik_kapat(&fd);
			bekle_t = mono() + 1.0;
			continue;
		}
		if (x.result <= 0) continue;
		int k = azalt(&a, in, (int)x.result, out, 4.0f);
		if (durum == D_SORU) {	/* v6: uyandirma yerine soru kaydi */
			soru_besle(out, k, &fd);
			continue;
		}
		int i = 0;
		while (i < k && durum == D_DINLE && !bekleyen[0]) {
			int t;
			if (uy_besle(&U, out + i, k - i, &t)) parca_isle();
			i += t;
		}
	}
	mik_kapat(&fd);
	unlink(DINLIYOR);
	printf("kole-asistan durdu (%s)\n", yeni_surum ? "yeni surum" : "sinyal");
	return 0;
}
