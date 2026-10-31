/* kole-telgraf-v7 (30.10.2026, Claude; scripts/674, kullanıcı: esi de bota): v6 + /data/asistan/telegram-izinli (satir basina
 * bir sohbet kimligi): izinli sohbetin guncellemeleri de Go'ya gider; izinsiz OZEL sohbetin kimligi gunluge yazilir
 * (eklemek icin). Kuyruk: "sohbet:<id>" satirlari varsa yalniz onlara, yoksa sahip + izinlilere; bir alici agdan
 * kalirsa dosya yalniz kalan alicilarla yeniden yazilir (gidenlere tekrar gitmez). /foto cevabi yalniz sorana.
 * --- v6 ---
 * kole-telgraf-v6 (29.10.2026, Claude; scripts/673): v5 + gunluk/kullanim etiketi v6 (v5 "v4" yaziyordu).
 * --- v5 ---
 * kole-telgraf-v5 (29.10.2026, Claude; madde 135 A2, scripts/669): v4 + sahibin "/foto" ya da "📷 Foto"
 * mesaji C'de: Go acilmaz (tepe ~8 MB), /tmp/kamera-foto-iste yazilir, cevap /tmp/bildirim-kuyruk'a. Kalkan varken
 * kamera istenmez. Dugme (callback_query) ve diger komutlar Go'da.
 * --- v4 ---
 * kole-telgraf-v4 (23.10.2026, Claude; denetim-2026-10-23-v2 R3 + R4, scripts/611): v3 +
 *  - sahip kaydi YAZILDIKTAN sonra izin dosyasi silinir; yazilamazsa izin durur (yeniden baslatmada yeniden ogrenilir),
 *    sahip bu surecte bellekte gecerli (v3 once siliyordu: /data yazilamazsa bot kilitli kaliyordu).
 *  - kole_https_v2.h: DNS tum nameserver'lari dener.
 * --- v3 ---
 * kole-telgraf-v3 (23.10.2026, Claude; denetim-2026-10-23 bulgulari 2, 6, 7, 8 + 429, scripts/594): v2 +
 *  - sahip sohbet YALNIZ yerel izinle ogrenilir: /data/asistan/telegram-sahip-ogren varken ve ozel sohbetten (kimlik > 0)
 *    gelen ilk mesaj; sonra izin dosyasi silinir (v2: dosya yoksa ilk yazan herkes sahip oluyordu).
 *  - offset bellekte (dosya yalniz acilista okunur: /data yazilamazsa ayni komut tekrar islenmesin)
 *  - cozulemeyen getUpdates paketi (HTTP 200, ok:true): ilk update_id atlanir (yoksa Telegram ayni paketi 24 sa yollar)
 *  - kuyrukta Telegram 400 alan ogenin /tmp fotosu da silinir; getUpdates 429 -> retry_after kadar beklenir
 * --- v2 ---
 * kole-telgraf-v2 (23.10.2026, Claude; Sira 18 madde 135 A3, scripts/591): v1 + zamanli Go tek atisi YALNIZ GEREKINCE.
 * v1 dakikada bir `--tg-zamanli` aciyordu (Go ~0.5 sn, tepe ~8.7 MB: MemAvailable 23.2 -> 18.4 MB dususleri, docs bolum 138).
 * v2: C dakikada bir ucuz denetim (bellek < esik+4 MB, oom_kill artisi, gorulmus bir hizmetin yoklugu, sabah ozeti
 * vakti, bayat mikrofon/TTS bayragi); tetik simdi ya da bir onceki denetimde varsa Go her dakika (Go'nun ardisik
 * sayaclari ayni isler), yoksa 5 dk'da bir. Mesaj mantigi Go'da kalir (tek kaynak). Gerisi v1 ile ayni.
 * --- v1 ---
 * kole-telgraf-v1 (23.10.2026, Claude; Sira 18 madde 135 A1, scripts/588): Telegram'in SUREKLI dongusu C'de (Go
 * kole-gemini --telegram RSS 5.9 MB + takas 1.8 MB; C + BearSSL ~0.7 MB, docs/kole-telgraf-tasarim-v1.md). Is geldikce
 * Go TEK ATIS: kole-gemini-v41 --tg-isle / --tg-zamanli.
 *   kole-telgraf-v1 --go <kole-gemini-v41> [--sure-sn N]   |   kole-telgraf-v1 --self-test
 * Tur:
 *  1) bitmis cocuklar toplanir (zamanli 120 sn'de bitmezse SIGTERM)
 *  2) bildirim kuyrugu /tmp/bildirim-kuyruk icindeki .txt dosyalari (ada gore sirali): "foto:<yol>" satiri -> sendPhoto (multipart, baslik
 *     en cok 1000 bayt), yoksa sendMessage (en cok 4000 bayt). Basarili ya da Telegram 400 (bozuk oge) -> .txt (ve /tmp/
 *     altindaki foto) silinir; ag/429/5xx -> bu tur birakilir. Foto okunamazsa yalniz metin gider (kuyruk takilmaz).
 *  3) son zamanlidan >= 60 sn ve calisan yoksa `<go> --tg-zamanli` (arka planda)
 *  4) getUpdates (offset+1, timeout 20, limit 20, message + callback_query). Her guncelleme: offset dosyaya, sahibin degilse
 *     atlanir (sohbet dosyasi yoksa ilk mesajin sohbeti yazilir), ham JSON /tmp/tg-guncelleme-<id>.json, `<go> --tg-isle`
 *     KOLE_TG_HAZIR_FD=3 borusuyla; "hazir" ya da boru kapanana kadar en cok 150 sn beklenir (sira korunur), surec
 *     sonra arka planda kalabilir (/canli izleme, ses akisi).
 *  Ag hatasi: 5 -> 60 sn artan bekleme; 409 (baska getUpdates istemcisi): 30 sn; 429: retry_after. Jeton yoksa 60 sn.
 * Jeton hicbir gunluge yazilmaz (istek yolu kh_hata'ya girmez). Durum /tmp/kole-telgraf.durum (her tur). */
#include "kole_https_v2.h"
#include "json_mini_v1.h"
#include <assert.h>
#include <dirent.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TG_AD "api.telegram.org"
static const char *JETON = "/data/asistan/telegram-jeton", *SOHBET = "/data/asistan/telegram-sohbet",
		  *OFSET = "/data/asistan/telegram-offset", *SAHIP_OGREN = "/data/asistan/telegram-sahip-ogren", *KUYRUK = "/tmp/bildirim-kuyruk", *DURUM = "/tmp/kole-telgraf.durum",
		  *FOTO_ISTE = "/tmp/kamera-foto-iste", *KALKAN = "/tmp/gizlilik-kalkani";	/* v5: Go telegram.go ile ayni */
static char jeton[128], sahip[32], go_yol[256] = "/mnt/sd/velinux/asistan/kole-gemini-v41";
static struct kh baglanti;
static char cevap[512 * 1024];
static struct jtok tk[8192];
static volatile sig_atomic_t dur;

static double simdi(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (double)t.tv_sec + (double)t.tv_nsec / 1e9;
}

static void gunluk(const char *b, ...) __attribute__((format(printf, 1, 2)));
static void gunluk(const char *b, ...)
{
	char z[32];
	time_t t = time(NULL);
	struct tm tm;
	strftime(z, sizeof z, "%H:%M:%S", localtime_r(&t, &tm));
	printf("[%s] ", z);
	va_list a;
	va_start(a, b);
	vprintf(b, a);
	va_end(a);
	putchar('\n');
	fflush(stdout);
}

static int dosya_oku(const char *yol, char *o, size_t n)	/* bosluklari kirpilmis ilk satir; 0 tamam */
{
	FILE *f = fopen(yol, "r");
	if (!f)
		return -1;
	o[0] = 0;
	if (!fgets(o, (int)n, f))
		o[0] = 0;
	fclose(f);
	size_t l = strlen(o);
	while (l && (o[l - 1] == '\n' || o[l - 1] == '\r' || o[l - 1] == ' ' || o[l - 1] == '\t'))
		o[--l] = 0;
	return o[0] ? 0 : -1;
}

static int dosya_yaz(const char *yol, const void *v, size_t n, mode_t kip)	/* .part + rename */
{
	char t[300];
	if ((size_t)snprintf(t, sizeof t, "%s.part", yol) >= sizeof t)
		return -1;
	int fd = open(t, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, kip);
	if (fd < 0)
		return -1;
	int ok = write(fd, v, n) == (ssize_t)n;
	ok = !close(fd) && ok;
	if (!ok || rename(t, yol)) {
		unlink(t);
		return -1;
	}
	return 0;
}

/* ---- saf yardimcilar (self-test) ---- */
static size_t url_kodla(const char *s, char *o, size_t n)	/* form alani; sigmazsa 0 */
{
	static const char hx[] = "0123456789ABCDEF";
	size_t k = 0;
	for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
		if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || strchr("-_.~", *p)) {
			if (k + 1 >= n)
				return 0;
			o[k++] = (char)*p;
		} else {
			if (k + 3 >= n)
				return 0;
			o[k++] = '%';
			o[k++] = hx[*p >> 4];
			o[k++] = hx[*p & 15];
		}
	}
	o[k] = 0;
	return k;
}

static size_t utf8_kirp(char *s, size_t azami)	/* azami bayta, kod noktasini bolmeden */
{
	size_t n = strlen(s);
	if (n <= azami)
		return n;
	n = azami;
	while (n > 0 && ((unsigned char)s[n] & 0xC0) == 0x80)
		n--;
	s[n] = 0;
	return n;
}

#define ALICI_AZAMI 6	/* v7: sahip + izinliler */
static const char *IZINLI = "/data/asistan/telegram-izinli";
static char izinli[ALICI_AZAMI - 1][32];
static int nizinli;

static void kopya32(char *d, const char *s)	/* v7: 32 baytlik kimlik alanina (uzunluk sohbet_gecerli ile < 32) */
{
	size_t n = strnlen(s, 31);
	memcpy(d, s, n);
	d[n] = 0;
}

static int sohbet_gecerli(const char *s)	/* v7: isaretli tamsayi */
{
	size_t i = *s == '-';
	if (!s[i] || strlen(s) >= 32)
		return 0;
	for (; s[i]; i++)
		if (s[i] < '0' || s[i] > '9')
			return 0;
	return 1;
}

static void izinli_oku(void)	/* v7: her kullanimda (dosya kucuk; kullanıcı ekleyince yeniden baslatma gerekmez) */
{
	char b[512], *p, *k;
	FILE *f = fopen(IZINLI, "r");
	nizinli = 0;
	if (!f)
		return;
	size_t n = fread(b, 1, sizeof b - 1, f);
	fclose(f);
	b[n] = 0;
	for (p = strtok_r(b, " \t\r\n", &k); p && nizinli < ALICI_AZAMI - 1; p = strtok_r(NULL, " \t\r\n", &k))
		if (sohbet_gecerli(p) && strcmp(p, sahip))
			kopya32(izinli[nizinli++], p);
}

static int yetkili(const char *s)	/* v7 */
{
	if (sahip[0] && !strcmp(s, sahip))
		return 1;
	for (int i = 0; i < nizinli; i++)
		if (!strcmp(s, izinli[i]))
			return 1;
	return 0;
}

/* kuyruk dosyasi: "foto:<yol>" satiri foto, "sohbet:<id>" alici (v7, en cok ALICI_AZAMI), digerleri (bos olmayan) metin */
static void kuyruk_coz(const char *v, char *metin, size_t mn, char *foto, size_t fn, char hedef[][32], int *nh)
{
	size_t m = 0;
	metin[0] = foto[0] = 0;
	*nh = 0;
	while (*v) {
		const char *son = strchr(v, '\n');
		size_t l = son ? (size_t)(son - v) : strlen(v);
		if (l && v[l - 1] == '\r')
			l--;
		if (l >= 7 && !memcmp(v, "sohbet:", 7)) {	/* v7 */
			char s[40];
			size_t bl = l - 7 < sizeof s - 1 ? l - 7 : sizeof s - 1;
			memcpy(s, v + 7, bl);
			s[bl] = 0;
			if (sohbet_gecerli(s) && *nh < ALICI_AZAMI)
				kopya32(hedef[(*nh)++], s);
		} else if (l >= 5 && !memcmp(v, "foto:", 5)) {
			const char *b = v + 5;
			size_t bl = l - 5;
			while (bl && *b == ' ')
				b++, bl--;
			while (bl && b[bl - 1] == ' ')
				bl--;
			if (bl < fn) {
				memcpy(foto, b, bl);
				foto[bl] = 0;
			}
		} else {
			size_t i = 0;
			while (i < l && (v[i] == ' ' || v[i] == '\t'))
				i++;
			if (i < l && m + l + 2 < mn) {
				if (m)
					metin[m++] = '\n';
				memcpy(metin + m, v, l);
				m += l;
				metin[m] = 0;
			}
		}
		if (!son)
			break;
		v = son + 1;
	}
}

/* multipart/form-data govdesi (sendPhoto). Donen: malloc'lu govde (uz), sinir *sinir'e. */
static char *cok_parcali(const char *sinir, const char *sohbet, const char *baslik, const void *foto, size_t fn, size_t *uz)
{
	char ust[2400];
	int a = snprintf(ust, sizeof ust,
			 "--%s\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n%s\r\n", sinir, sohbet);
	if (a < 0 || (size_t)a >= sizeof ust)
		return NULL;
	if (baslik && *baslik) {
		int b = snprintf(ust + a, sizeof ust - (size_t)a,
				 "--%s\r\nContent-Disposition: form-data; name=\"caption\"\r\n\r\n%s\r\n", sinir, baslik);
		if (b < 0 || (size_t)b >= sizeof ust - (size_t)a)
			return NULL;
		a += b;
	}
	int b = snprintf(ust + a, sizeof ust - (size_t)a,
			 "--%s\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"kamera.jpg\"\r\n"
			 "Content-Type: image/jpeg\r\n\r\n", sinir);
	if (b < 0 || (size_t)b >= sizeof ust - (size_t)a)
		return NULL;
	a += b;
	char alt[96];
	int c = snprintf(alt, sizeof alt, "\r\n--%s--\r\n", sinir);
	if (c < 0 || (size_t)c >= sizeof alt)
		return NULL;
	size_t n = (size_t)a + fn + (size_t)c;
	char *g = malloc(n);
	if (!g)
		return NULL;
	memcpy(g, ust, (size_t)a);
	memcpy(g + a, foto, fn);
	memcpy(g + a + fn, alt, (size_t)c);
	*uz = n;
	return g;
}

/* Telegram cevabi: ok -> 1; degilse 0 ve hata kodu / retry_after / aciklama */
static int tg_ok(const char *s, size_t n, int *ekod, int *bekle, char *acik, size_t an)
{
	*ekod = *bekle = 0;
	if (acik && an)
		acik[0] = 0;
	int t = json_ayristir(s, (int)n, tk, (int)(sizeof tk / sizeof tk[0]));
	if (t <= 0)
		return 0;
	int i = j_al(s, tk, t, 0, "ok");
	if (i >= 0 && j_bool(s, &tk[i]) == 1)
		return 1;
	long long v;
	i = j_al(s, tk, t, 0, "error_code");
	if (i >= 0 && !j_tamsayi(s, &tk[i], &v))
		*ekod = (int)v;
	i = j_al(s, tk, t, j_al(s, tk, t, 0, "parameters"), "retry_after");
	if (i >= 0 && !j_tamsayi(s, &tk[i], &v))
		*bekle = (int)v;
	i = j_al(s, tk, t, 0, "description");
	if (acik && i >= 0 && j_dizge_al(s, &tk[i], acik, an))
		acik[0] = 0;
	return 0;
}

struct guncelleme {
	long long id, sohbet;
	int bas, son, mesaj;	/* mesaj: 1 message, 0 callback_query */
	int metin;	/* v5: message.text jetonu (tk icinde, ayni tur); yoksa -1 */
};

/* getUpdates sonucu: en cok gn oge; -1 bicim hatasi */
static int guncellemeler(const char *s, size_t n, struct guncelleme *g, int gn)
{
	int t = json_ayristir(s, (int)n, tk, (int)(sizeof tk / sizeof tk[0]));
	if (t <= 0)
		return -1;
	int ok = j_al(s, tk, t, 0, "ok"), r = j_al(s, tk, t, 0, "result");
	if (ok < 0 || j_bool(s, &tk[ok]) != 1 || r < 0 || tk[r].tip != J_DIZI)
		return -1;
	int k = 0;
	for (int i = 0; i < tk[r].boyut && k < gn; i++) {
		int u = j_oge(tk, t, r, i), x;
		if (u < 0 || tk[u].tip != J_NESNE)
			continue;
		struct guncelleme e = { 0, 0, tk[u].bas, tk[u].son, 0, -1 };
		if ((x = j_al(s, tk, t, u, "update_id")) < 0 || j_tamsayi(s, &tk[x], &e.id))
			continue;
		int m = j_al(s, tk, t, u, "message"), cq = j_al(s, tk, t, u, "callback_query");
		if (m >= 0) {
			e.mesaj = 1;
			e.metin = j_al(s, tk, t, m, "text");	/* v5 */
			x = j_al(s, tk, t, j_al(s, tk, t, m, "chat"), "id");
		} else if (cq >= 0) {
			x = j_al(s, tk, t, j_al(s, tk, t, j_al(s, tk, t, cq, "message"), "chat"), "id");
			if (x < 0)
				x = j_al(s, tk, t, j_al(s, tk, t, cq, "from"), "id");
		} else {
			x = -1;
		}
		if (x < 0 || j_tamsayi(s, &tk[x], &e.sohbet))
			e.sohbet = 0;
		g[k++] = e;
	}
	return k;
}

/* ---- Telegram cagrisi ---- */
static int tg_cagri(const char *metod, const char *tip, const void *govde, size_t gn, int ms, size_t *uz)
{
	char yol[200], ek[200];
	if ((size_t)snprintf(yol, sizeof yol, "/bot%s/%s", jeton, metod) >= sizeof yol ||
	    (size_t)snprintf(ek, sizeof ek, "Content-Type: %s\r\n", tip) >= sizeof ek)
		return -1;
	int kod = kh_tek(&baglanti, "POST", TG_AD, yol, ek, govde, gn, cevap, sizeof cevap, uz, ms);
	memset(yol, 0, sizeof yol);
	return kod;
}

static int tg_form(const char *metod, const char *form, int ms, size_t *uz)
{
	return tg_cagri(metod, "application/x-www-form-urlencoded", form, strlen(form), ms, uz);
}

/* ---- cocuklar ---- */
struct cocuk {
	pid_t pid;
	double bas;
	int zamanli;
};
static struct cocuk cocuklar[16];
static double son_zamanli = -1e9;
static unsigned long zamanli_sayi;	/* v2: acilan --tg-zamanli */

static void cocuk_ekle(pid_t p, int zamanli)
{
	for (size_t i = 0; i < sizeof cocuklar / sizeof cocuklar[0]; i++)
		if (!cocuklar[i].pid) {
			cocuklar[i] = (struct cocuk){ p, simdi(), zamanli };
			return;
		}
	gunluk("UYARI: cocuk tablosu dolu (pid %d izlenmiyor)", (int)p);
}

static int cocuk_say(int zamanli)
{
	int n = 0;
	for (size_t i = 0; i < sizeof cocuklar / sizeof cocuklar[0]; i++)
		if (cocuklar[i].pid && (zamanli < 0 || cocuklar[i].zamanli == zamanli))
			n++;
	return n;
}

static void cocuk_topla(void)
{
	int st;
	pid_t p;
	while ((p = waitpid(-1, &st, WNOHANG)) > 0) {
		for (size_t i = 0; i < sizeof cocuklar / sizeof cocuklar[0]; i++) {
			if (cocuklar[i].pid != p)
				continue;
			int rc = WIFEXITED(st) ? WEXITSTATUS(st) : 128 + (WIFSIGNALED(st) ? WTERMSIG(st) : 0);
			if (rc)
				gunluk("%s bitti: rc %d (%.0f sn)", cocuklar[i].zamanli ? "zamanli" : "tg-isle", rc, simdi() - cocuklar[i].bas);
			cocuklar[i].pid = 0;
		}
	}
	for (size_t i = 0; i < sizeof cocuklar / sizeof cocuklar[0]; i++)
		if (cocuklar[i].pid && cocuklar[i].zamanli && simdi() - cocuklar[i].bas > 120) {
			gunluk("zamanli 120 sn'de bitmedi: SIGTERM %d", (int)cocuklar[i].pid);
			kill(cocuklar[i].pid, SIGTERM);
			cocuklar[i].bas = simdi();	/* bir sonraki SIGTERM 120 sn sonra */
		}
}

static pid_t go_calistir(const char *kip, const char *arg, int hazir_fd)
{
	pid_t p = fork();
	if (p < 0) {
		gunluk("fork: %s", strerror(errno));
		return -1;
	}
	if (p == 0) {
		signal(SIGPIPE, SIG_DFL);
		if (hazir_fd >= 0) {
			if (dup2(hazir_fd, 3) < 0)
				_exit(127);
			setenv("KOLE_TG_HAZIR_FD", "3", 1);
		}
		char *av[] = { go_yol, (char *)kip, (char *)arg, NULL };
		execv(go_yol, av);
		_exit(127);
	}
	return p;
}

/* ---- bildirim kuyrugu ---- */
static int ad_kiyas(const void *a, const void *b)
{
	return strcmp(*(char *const *)a, *(char *const *)b);
}

/* v7: tek aliciya gonder; 0 gitti ya da atildi (Telegram 400), -1 sonra yeniden. metin kirpilabilir (kopya ver). */
static int alici_gonder(const char *alici, char *metin, const char *foto)
{
	char hata[160];
	size_t uz = 0;
	int kod = -1, ekod = 0, bekle = 0, foto_var = 0;
	if (foto[0]) {
		struct stat st;
		int ff = open(foto, O_RDONLY | O_CLOEXEC);
		char *jpg = NULL;
		if (ff >= 0 && !fstat(ff, &st) && st.st_size > 0 && st.st_size <= 5 << 20 && (jpg = malloc((size_t)st.st_size)) &&
		    read(ff, jpg, (size_t)st.st_size) == st.st_size) {
			char sinir[48], tip[96];
			size_t gu;
			snprintf(sinir, sizeof sinir, "koleTelgraf%08x%04x", (unsigned)time(NULL), kh_rastgele16());
			snprintf(tip, sizeof tip, "multipart/form-data; boundary=%s", sinir);
			utf8_kirp(metin, 1000);
			char *g = cok_parcali(sinir, alici, metin, jpg, (size_t)st.st_size, &gu);
			if (g) {
				foto_var = 1;
				kod = tg_cagri("sendPhoto", tip, g, gu, 30000, &uz);
				free(g);
			}
		} else {
			gunluk("kuyruk: foto okunamadi, yalniz metin gidecek");
		}
		free(jpg);
		if (ff >= 0)
			close(ff);
	}
	if (!foto_var) {
		static char form[13000];
		char kod_m[12600];
		if (!metin[0])
			return 0;	/* bos oge */
		utf8_kirp(metin, 4000);
		if (!url_kodla(metin, kod_m, sizeof kod_m))
			return 0;
		snprintf(form, sizeof form, "chat_id=%s&text=%s", alici, kod_m);
		kod = tg_form("sendMessage", form, 20000, &uz);
	}
	if (kod < 0) {
		gunluk("kuyruk: %s", kh_hata);
		return -1;
	}
	if (!tg_ok(cevap, uz, &ekod, &bekle, hata, sizeof hata)) {
		gunluk("kuyruk: Telegram %d %s", ekod ? ekod : kod, hata);
		return ekod == 400 ? 0 : -1;	/* bozuk oge ya da bu alici (engelledi vb.): at */
	}
	return 0;
}

static int kuyruk_tek(const char *yol)	/* 0 gonderildi/atildi, -1 sonra yeniden */
{
	char v[8192], metin[4200], foto[256], kopya[4200], hedef[ALICI_AZAMI][32];
	int nh;
	int fd = open(yol, O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return 0;
	ssize_t n = read(fd, v, sizeof v - 1);
	close(fd);
	if (n < 0)
		return 0;
	v[n] = 0;
	kuyruk_coz(v, metin, sizeof metin, foto, sizeof foto, hedef, &nh);
	if (!nh) {	/* v7: alici yazilmamis bildirim -> sahip + izinliler */
		izinli_oku();
		if (sahip[0])
			snprintf(hedef[nh++], 32, "%s", sahip);
		for (int i = 0; i < nizinli && nh < ALICI_AZAMI; i++)
			kopya32(hedef[nh++], izinli[i]);
	}
	char kalan[ALICI_AZAMI * 40 + 4300 + 300];
	size_t kl = 0;
	for (int i = 0; i < nh; i++) {
		memcpy(kopya, metin, sizeof kopya);
		if (alici_gonder(hedef[i], kopya, foto))
			kl += (size_t)snprintf(kalan + kl, sizeof kalan - kl, "sohbet:%s\n", hedef[i]);
	}
	if (kl) {	/* v7: yalniz gidemeyenler icin yeniden yaz (gidenlere tekrar gitmesin) */
		kl += (size_t)snprintf(kalan + kl, sizeof kalan - kl, "%s\n", metin);
		if (foto[0])
			kl += (size_t)snprintf(kalan + kl, sizeof kalan - kl, "foto:%s\n", foto);
		if (dosya_yaz(yol, kalan, kl < sizeof kalan ? kl : sizeof kalan - 1, 0600))
			gunluk("kuyruk: kalan alicilar yazilamadi");
		return -1;
	}
	if (foto[0] && !strncmp(foto, "/tmp/", 5))	/* v3: /tmp fotosu RAM'de birikmesin */
		unlink(foto);
	return 0;
}

static int kuyruk_gonder(void)	/* gonderilen sayisi; hata -> -1 */
{
	DIR *d = opendir(KUYRUK);
	if (!d)
		return 0;
	char *adlar[64];
	int n = 0;
	struct dirent *e;
	while ((e = readdir(d)) && n < 64) {
		size_t l = strlen(e->d_name);
		if (l > 4 && !strcmp(e->d_name + l - 4, ".txt"))
			adlar[n++] = strdup(e->d_name);
	}
	closedir(d);
	qsort(adlar, (size_t)n, sizeof adlar[0], ad_kiyas);
	int gitti = 0, sonuc = 0;
	for (int i = 0; i < n; i++) {
		char y[400];
		if (!sonuc && adlar[i]) {
			snprintf(y, sizeof y, "%s/%s", KUYRUK, adlar[i]);
			if (kuyruk_tek(y) == 0) {
				unlink(y);
				gitti++;
			} else {
				sonuc = -1;
			}
		}
		free(adlar[i]);
	}
	return sonuc ? -1 : gitti;
}

/* ---- guncellemeler ---- */
static long long ofset_oku(void)
{
	char s[32];
	return dosya_oku(OFSET, s, sizeof s) ? 0 : atoll(s);
}

static void ofset_yaz(long long o)
{
	char s[32];
	int n = snprintf(s, sizeof s, "%lld\n", o);
	if (dosya_yaz(OFSET, s, (size_t)n, 0600))
		gunluk("UYARI: offset yazilamadi");
}

static int sahip_yazilmadi;	/* v4 (R3): sahip ogrenildi ama sohbet dosyasi henuz yazilamadi */

/* v5 (135 A2): basit komut C'de; 1 = islendi (Go acilmaz). cevap/tk guncellemeler()'in bu turdaki ciktisi. */
static int hizli_komut(const struct guncelleme *g)
{
	char m[64];
	if (g->metin < 0 || j_dizge_al(cevap, &tk[g->metin], m, sizeof m) < 0)
		return 0;
	if (strcmp(m, "/foto") && strncmp(m, "/foto@", 6) && strcmp(m, "📷 Foto"))
		return 0;
	const char *y = "Gizlilik kalkanı açık efendim, kamera kullanılamaz.";
	if (access(KALKAN, F_OK)) {
		if (dosya_yaz(FOTO_ISTE, "telegram\n", 9, 0644))
			return 0;	/* yazilamadi: Go denesin */
		y = "📸 Çekiyorum efendim, birazdan gelir.";
	}
	char k[200], c[160];
	int cn = snprintf(c, sizeof c, "sohbet:%lld\n%s", g->sohbet, y);	/* v7: cevap yalniz sorana */
	snprintf(k, sizeof k, "%s/%lld-hizli.txt", KUYRUK, g->id);
	if (dosya_yaz(k, c, (size_t)cn, 0600))
		gunluk("guncelleme %lld: cevap kuyruga yazilamadi", g->id);
	gunluk("guncelleme %lld (mesaj): /foto C'de", g->id);
	return 1;
}

static void guncelleme_isle(const struct guncelleme *g)
{
	char s[32];
	snprintf(s, sizeof s, "%lld", g->sohbet);
	if (!sahip[0] && g->mesaj && g->sohbet > 0 && !access(SAHIP_OGREN, F_OK)) {	/* v3: yerel izin + ozel sohbet */
		snprintf(sahip, sizeof sahip, "%s", s);
		sahip_yazilmadi = 1;
		gunluk("sahip sohbet ilk mesajdan ogrenildi");
	}
	if (sahip_yazilmadi && !strcmp(s, sahip)) {	/* v4: izin yalniz kayit yazilinca silinir; yazilamazsa sahibin sonraki mesajinda */
		char t[40];
		int n = snprintf(t, sizeof t, "%s\n", sahip);
		if (dosya_yaz(SOHBET, t, (size_t)n, 0600)) {
			gunluk("UYARI: sohbet dosyasi yazilamadi; izin dosyasi korundu, sonraki mesajda yeniden");
		} else {
			unlink(SAHIP_OGREN);
			sahip_yazilmadi = 0;
			gunluk("sahip sohbet kaydedildi");
		}
	}
	izinli_oku();	/* v7 */
	if (!sahip[0] || !yetkili(s)) {	/* yalniz sahip + izinliler */
		if (sahip[0] && g->mesaj && g->sohbet > 0)
			gunluk("guncelleme %lld: izinsiz ozel sohbet %s (eklemek icin %s)", g->id, s, IZINLI);
		return;
	}
	if (hizli_komut(g))	/* v5 */
		return;
	char y[96];
	snprintf(y, sizeof y, "/tmp/tg-guncelleme-%lld.json", g->id);
	if (dosya_yaz(y, cevap + g->bas, (size_t)(g->son - g->bas), 0600)) {
		gunluk("guncelleme %lld yazilamadi", g->id);
		return;
	}
	int b[2];
	if (pipe2(b, O_CLOEXEC)) {
		gunluk("pipe: %s", strerror(errno));
		return;
	}
	double t0 = simdi();
	pid_t p = go_calistir("--tg-isle", y, b[1]);
	close(b[1]);
	if (p < 0) {
		close(b[0]);
		unlink(y);
		return;
	}
	cocuk_ekle(p, 0);
	struct pollfd pf = { b[0], POLLIN, 0 };
	char h[16];
	int r = poll(&pf, 1, 150000);
	ssize_t hn = r == 1 ? read(b[0], h, sizeof h) : -1;
	close(b[0]);
	gunluk("guncelleme %lld (%s): %s %.1f sn", g->id, g->mesaj ? "mesaj" : "dugme",
	       hn > 0 ? "hazir" : hn == 0 ? "surec bitti" : "150 sn'de hazir gelmedi", simdi() - t0);
}

static int son_retry;	/* v3: getUpdates 429 retry_after */

/* v3: cozulemeyen pakette ilk update_id (yoksa -1) */
static long long ilk_update_id(const char *s)
{
	const char *p = strstr(s, "\"update_id\":");
	return p ? strtoll(p + 12, NULL, 10) : -1;
}

static int guncelleme_turu(int bekle_sn)	/* islenen sayisi; ag hatasi -1, 409 -2, 429 -3 (son_retry) */
{
	static struct guncelleme g[20];
	char form[200];
	static long long o = -1;	/* v3: bellekte; dosya yalniz acilista */
	if (o < 0)
		o = ofset_oku();
	snprintf(form, sizeof form, "offset=%lld&timeout=%d&limit=20&allowed_updates=%%5B%%22message%%22%%2C%%22callback_query%%22%%5D",
		 o + 1, bekle_sn);
	size_t uz;
	int kod = tg_form("getUpdates", form, (bekle_sn + 15) * 1000, &uz);
	if (kod < 0) {
		gunluk("getUpdates: %s", kh_hata);
		return -1;
	}
	int ekod, rb;
	char hata[160];
	int n = guncellemeler(cevap, uz, g, 20);
	if (n < 0) {
		long long ilk = ilk_update_id(cevap);
		if (kod == 200 && strstr(cevap, "\"ok\":true") && ilk > o) {	/* v3: cozulemeyen paket: ilk ogeyi atla */
			o = ilk;
			ofset_yaz(o);
			gunluk("getUpdates: paket cozulemedi (%zu bayt), guncelleme %lld atlandi", uz, ilk);
			return 0;
		}
		tg_ok(cevap, uz, &ekod, &rb, hata, sizeof hata);
		gunluk("getUpdates: HTTP %d Telegram %d %s", kod, ekod, hata);
		son_retry = ekod == 429 ? rb : 0;
		return ekod == 409 ? -2 : ekod == 429 ? -3 : -1;
	}
	for (int i = 0; i < n; i++) {
		if (g[i].id <= o)
			continue;
		o = g[i].id;
		ofset_yaz(o);	/* once kaydet: islem cokse de ayni guncelleme tekrar gelmez (Go ile ayni) */
		guncelleme_isle(&g[i]);
		cocuk_topla();
	}
	return n;
}

static void durum_yaz(unsigned long tur, unsigned long islenen, unsigned long gonderilen, const char *son)
{
	char s[256];
	int n = snprintf(s, sizeof s, "tur %lu\nguncelleme %lu\nbildirim %lu\ncocuk %d\nzamanli %lu\nson %s\nzaman %ld\n", tur, islenen,
			 gonderilen, cocuk_say(-1), zamanli_sayi, son, (long)time(NULL));
	if (n > 0)
		dosya_yaz(DURUM, s, (size_t)n, 0644);
}

/* ---- v2: zamanli Go tek atisi yalniz gerekince ---- */
static const char *proc_kok = "/proc";
/* shortcut: liste Go saglik.go hizmetler ile elle esit tutulur; Go'ya hizmet eklenirse buraya da ekle */
static const char *const HIZMETLER[] = { "panel", "kamera-kayit-v", "kole-kopru-v", "kole-cihaz-merk", "tailscaled",
					 "kole-asistan-v", "mcu-servis-v" };
#define HZ_ASISTAN 5

static long long proc_sayi(const char *dosya, const char *anahtar)	/* "anahtar N" ya da "anahtar: N kB"; yoksa -1 */
{
	char y[200], s[256];
	snprintf(y, sizeof y, "%s/%s", proc_kok, dosya);
	FILE *f = fopen(y, "r");
	if (!f)
		return -1;
	size_t n = strlen(anahtar);
	long long v = -1;
	while (fgets(s, sizeof s, f))
		if (!strncmp(s, anahtar, n) && (s[n] == ' ' || s[n] == ':' || s[n] == '\t')) {
			v = atoll(s + n + 1);
			break;
		}
	fclose(f);
	return v;
}

static unsigned hizmet_maske(void)	/* bit i: HIZMETLER[i] calisiyor (comm onekiyle; panel tam ad) */
{
	unsigned m = 0;
	DIR *d = opendir(proc_kok);
	if (!d)
		return 0;
	struct dirent *e;
	while ((e = readdir(d))) {
		if (e->d_name[0] < '1' || e->d_name[0] > '9')
			continue;
		char y[320], c[32];
		snprintf(y, sizeof y, "%s/%s/comm", proc_kok, e->d_name);
		if (dosya_oku(y, c, sizeof c))
			continue;
		for (size_t i = 0; i < sizeof HIZMETLER / sizeof HIZMETLER[0]; i++)
			if (i == 0 ? !strcmp(c, HIZMETLER[0]) : !strncmp(c, HIZMETLER[i], strlen(HIZMETLER[i])))
				m |= 1u << i;
	}
	closedir(d);
	return m;
}

/* Go sabahGonderilsinMi: TR saati (UTC+3, yaz saati yok) [saat, saat+3 sa) ve bugun gonderilmediyse */
static int sabah_vakti(time_t utc, const char *saat, const char *son_gun)
{
	int sa = 8, dk = 0;
	if (!saat || sscanf(saat, "%d:%d", &sa, &dk) != 2 || sa < 0 || sa > 23 || dk < 0 || dk > 59)
		sa = 8, dk = 0;
	time_t tr = utc + 3 * 3600;
	struct tm t;
	char gun[16];
	gmtime_r(&tr, &t);
	strftime(gun, sizeof gun, "%Y-%m-%d", &t);
	int dakika = t.tm_hour * 60 + t.tm_min, bas = sa * 60 + dk;
	return dakika >= bas && dakika < bas + 180 && strcmp(son_gun ? son_gun : "", gun);
}

static int bayat(const char *yol, int sn)
{
	struct stat st;
	return !stat(yol, &st) && time(NULL) - st.st_mtime > sn;
}

static void neden_ekle(char *o, size_t n, const char *s)
{
	size_t l = strlen(o);
	snprintf(o + l, n - l, "%s%s", l ? ", " : "", s);
}

/* Go'nun zamanli islerinde mesaj cikabilir mi? (ust kume; karar Go'da) */
static int zamanli_tetik(time_t utc, char *neden, size_t nn)
{
	static long long son_oom = -1;
	static unsigned goruldu;
	char e[16], saat[16], son[16];
	neden[0] = 0;
	long long esik = dosya_oku("/data/asistan/saglik-bellek-esik-mb", e, sizeof e) ? 12 : atoll(e);
	if (esik <= 0)
		esik = 12;
	long long mem = proc_sayi("meminfo", "MemAvailable");
	if (mem >= 0 && mem < (esik + 4) * 1024)	/* Go: uyari < esik, iyilesme >= esik+4 */
		neden_ekle(neden, nn, "bellek");
	long long oom = proc_sayi("vmstat", "oom_kill");
	if (oom >= 0 && son_oom >= 0 && oom != son_oom)
		neden_ekle(neden, nn, "oom");
	if (oom >= 0)
		son_oom = oom;
	unsigned m = hizmet_maske();
	if (!access("/data/asistan/kapali", F_OK)) {	/* Hey Kole kapali: Go da izlemez */
		m &= ~(1u << HZ_ASISTAN);
		goruldu &= ~(1u << HZ_ASISTAN);
	}
	goruldu |= m;
	if (goruldu & ~m)
		neden_ekle(neden, nn, "hizmet");
	if (access("/data/asistan/sabah-ozeti-kapali", F_OK) &&
	    sabah_vakti(utc, dosya_oku("/data/asistan/sabah-ozeti-saat", saat, sizeof saat) ? NULL : saat,
			dosya_oku("/data/asistan/sabah-ozeti-son", son, sizeof son) ? NULL : son))
		neden_ekle(neden, nn, "sabah");
	if (bayat("/tmp/asistan-mik-birak", 120) || bayat("/tmp/tts-caliyor", 120))
		neden_ekle(neden, nn, "mikrofon");
	return neden[0] != 0;
}

static void sinyal(int s)
{
	(void)s;
	dur = 1;
}

static int self_test(void)
{
	char o[64];
	assert(url_kodla("a b&ç=1", o, sizeof o) && !strcmp(o, "a%20b%26%C3%A7%3D1"));
	assert(!url_kodla("abcdef", o, 4));
	char k[16] = "ççç";	/* 6 bayt */
	assert(utf8_kirp(k, 5) == 4 && !strcmp(k, "çç"));
	char metin[64], foto[32], hd[ALICI_AZAMI][32];
	int nh;
	kuyruk_coz("Hareket var\r\nfoto: /tmp/a.jpg \n\n  \nikinci satir", metin, sizeof metin, foto, sizeof foto, hd, &nh);
	assert(!strcmp(metin, "Hareket var\nikinci satir") && !strcmp(foto, "/tmp/a.jpg") && nh == 0);
	kuyruk_coz("yalniz metin\n", metin, sizeof metin, foto, sizeof foto, hd, &nh);
	assert(!strcmp(metin, "yalniz metin") && !foto[0]);
	kuyruk_coz("sohbet:42\r\nsohbet:-77\nsohbet:x1\nmerhaba\n", metin, sizeof metin, foto, sizeof foto, hd, &nh);	/* v7 */
	assert(nh == 2 && !strcmp(hd[0], "42") && !strcmp(hd[1], "-77") && !strcmp(metin, "merhaba"));
	assert(sohbet_gecerli("123") && sohbet_gecerli("-5") && !sohbet_gecerli("-") && !sohbet_gecerli("12a") && !sohbet_gecerli(""));
	size_t uz;
	char *g = cok_parcali("SNR", "42", "baş", "\xff\xd8xy", 4, &uz);
	const char *bek = "--SNR\r\nContent-Disposition: form-data; name=\"chat_id\"\r\n\r\n42\r\n"
			  "--SNR\r\nContent-Disposition: form-data; name=\"caption\"\r\n\r\nbaş\r\n"
			  "--SNR\r\nContent-Disposition: form-data; name=\"photo\"; filename=\"kamera.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n"
			  "\xff\xd8xy\r\n--SNR--\r\n";
	assert(g && uz == strlen(bek) && !memcmp(g, bek, uz));
	free(g);
	int ekod, bekle;
	char acik[80];
	const char *h = "{\"ok\":false,\"error_code\":429,\"description\":\"Too Many Requests: retry after 7\",\"parameters\":{\"retry_after\":7}}";
	assert(!tg_ok(h, strlen(h), &ekod, &bekle, acik, sizeof acik) && ekod == 429 && bekle == 7 && strstr(acik, "retry after 7"));
	assert(tg_ok("{\"ok\":true,\"result\":true}", 25, &ekod, &bekle, acik, sizeof acik) == 1);
	const char *u = "{\"ok\":true,\"result\":[{\"update_id\":10,\"message\":{\"message_id\":1,\"chat\":{\"id\":42,\"type\":\"private\"},"
			"\"text\":\"/yardim \\u00e7\"}},{\"update_id\":11,\"callback_query\":{\"id\":\"c\",\"from\":{\"id\":42},"
			"\"message\":{\"chat\":{\"id\":42}},\"data\":\"k:/foto\"}},{\"update_id\":12,\"message\":{\"chat\":{\"id\":-43}}},"
			"{\"update_id\":13,\"callback_query\":{\"id\":\"d\",\"from\":{\"id\":42}}}]}";
	struct guncelleme gg[8];
	int n = guncellemeler(u, strlen(u), gg, 8);
	assert(n == 4 && gg[0].id == 10 && gg[0].sohbet == 42 && gg[0].mesaj && gg[1].id == 11 && gg[1].sohbet == 42 &&
	       !gg[1].mesaj && gg[2].sohbet == -43 && gg[3].sohbet == 42 && !gg[3].mesaj);
	assert(!strncmp(u + gg[0].bas, "{\"update_id\":10,", 16) && u[gg[0].son - 1] == '}' && u[gg[0].son] == ',');
	assert(guncellemeler(u, strlen(u), gg, 2) == 2);
	assert(guncellemeler("{\"ok\":false,\"error_code\":409}", 29, gg, 8) == -1);
	assert(guncellemeler("{\"ok\":true,\"result\":[", 21, gg, 8) == -1);
	assert(gg[0].metin >= 0 && gg[1].metin < 0 && gg[2].metin < 0);	/* v5 */
	{	/* v5: /foto C'de (gecici dizin; sahip 42; Go acilmamali) */
		char kd[] = "/tmp/kt-hizli-XXXXXX", fi[96], ka[96], ky[128], b[256];
		assert(mkdtemp(kd));
		snprintf(fi, sizeof fi, "%s/foto-iste", kd);
		snprintf(ka, sizeof ka, "%s/kalkan", kd);
		KUYRUK = kd, FOTO_ISTE = fi, KALKAN = ka;
		snprintf(sahip, sizeof sahip, "42");
		const char *js = "{\"ok\":true,\"result\":[{\"update_id\":5,\"message\":{\"chat\":{\"id\":42},\"text\":\"\\ud83d\\udcf7 Foto\"}},"
				 "{\"update_id\":6,\"message\":{\"chat\":{\"id\":42},\"text\":\"/foto\"}}]}";
		size_t jn = strlen(js);
		memcpy(cevap, js, jn + 1);
		struct guncelleme g3[4];
		assert(guncellemeler(cevap, jn, g3, 4) == 2);
		guncelleme_isle(&g3[0]);
		assert(cocuk_say(0) == 0 && !access(fi, F_OK));
		snprintf(ky, sizeof ky, "%s/5-hizli.txt", kd);
		int f = open(ky, O_RDONLY);
		assert(f >= 0);
		ssize_t bn = read(f, b, sizeof b - 1);
		close(f);
		assert(bn > 0 && (b[bn] = 0, strstr(b, "Çekiyorum")) && !strncmp(b, "sohbet:42\n", 10));	/* v7: yalniz sorana */
		unlink(fi);
		assert(!dosya_yaz(ka, "1", 1, 0644));
		guncelleme_isle(&g3[1]);	/* kalkan: kamera istenmez */
		snprintf(ky, sizeof ky, "%s/6-hizli.txt", kd);
		f = open(ky, O_RDONLY);
		assert(f >= 0);
		bn = read(f, b, sizeof b - 1);
		close(f);
		assert(bn > 0 && (b[bn] = 0, strstr(b, "kalkanı")) && access(fi, F_OK) && cocuk_say(0) == 0);
		unlink(ka);
		unlink(ky);
		snprintf(ky, sizeof ky, "%s/5-hizli.txt", kd);
		unlink(ky);
		{	/* v7: izinli liste (esi 77 izinli, 99 degil; sahip tekrari ve bozuk satir atilir) */
			char iz[128];
			snprintf(iz, sizeof iz, "%s/izinli", kd);
			IZINLI = iz;
			assert(!dosya_yaz(iz, "77\n42\nabc\n -88 \n", 16, 0600));
			izinli_oku();
			assert(nizinli == 2 && yetkili("42") && yetkili("77") && yetkili("-88") && !yetkili("99"));
			const char *j2 = "{\"ok\":true,\"result\":[{\"update_id\":7,\"message\":{\"chat\":{\"id\":77},\"text\":\"/foto\"}},"
					 "{\"update_id\":8,\"message\":{\"chat\":{\"id\":99},\"text\":\"/foto\"}}]}";
			size_t j2n = strlen(j2);
			memcpy(cevap, j2, j2n + 1);
			assert(guncellemeler(cevap, j2n, g3, 4) == 2);
			guncelleme_isle(&g3[0]);
			snprintf(ky, sizeof ky, "%s/7-hizli.txt", kd);
			f = open(ky, O_RDONLY);
			assert(f >= 0);
			bn = read(f, b, sizeof b - 1);
			close(f);
			assert(bn > 0 && (b[bn] = 0, !strncmp(b, "sohbet:77\n", 10)) && !access(fi, F_OK));
			unlink(ky);
			unlink(fi);
			guncelleme_isle(&g3[1]);	/* izinsiz: hicbir sey yazilmaz */
			snprintf(ky, sizeof ky, "%s/8-hizli.txt", kd);
			assert(access(ky, F_OK) && access(fi, F_OK) && cocuk_say(0) == 0);
			unlink(iz);
			IZINLI = "/data/asistan/telegram-izinli";
			nizinli = 0;
		}
		rmdir(kd);
		KUYRUK = "/tmp/bildirim-kuyruk", FOTO_ISTE = "/tmp/kamera-foto-iste", KALKAN = "/tmp/gizlilik-kalkani";
		sahip[0] = 0;
		puts("self-test: /foto C'de OK (dugme metni + komut, kalkan); v7 izinli liste OK (izinliye cevap, izinsiz atlanir)");
	}
	puts("self-test: OK (url kodlama, utf8 kirpma, kuyruk satirlari, multipart, Telegram hata/429, getUpdates 4 oge)");
	/* KT_SAHTE_GO: sahte Go betigiyle fork/exec + hazir borusu (ag yok). Betik guncelleme dosyasini KT_SAHTE_CIKTI'ya
	 * kopyalar, fd 3'e "hazir" yazar, 2 sn daha yasar: guncelleme_isle 1 sn'den kisa donmeli, cocuk sonra toplanmali. */
	const char *sg = getenv("KT_SAHTE_GO"), *sc = getenv("KT_SAHTE_CIKTI");
	if (sg && sc) {
		snprintf(go_yol, sizeof go_yol, "%s", sg);
		snprintf(sahip, sizeof sahip, "42");
		const char *js = "{\"ok\":true,\"result\":[{\"update_id\":77,\"message\":{\"chat\":{\"id\":42},\"text\":\"x\"}},"
				 "{\"update_id\":78,\"message\":{\"chat\":{\"id\":9},\"text\":\"yabanci\"}}]}";
		size_t jn = strlen(js);
		memcpy(cevap, js, jn + 1);
		struct guncelleme g2[4];
		assert(guncellemeler(cevap, jn, g2, 4) == 2);
		unlink(sc);
		double t0 = simdi();
		guncelleme_isle(&g2[0]);
		double dt = simdi() - t0;
		assert(dt < 1.0 && cocuk_say(0) == 1);
		guncelleme_isle(&g2[1]);	/* yabanci: cocuk acilmaz */
		assert(cocuk_say(0) == 1);
		char b[256] = "";
		int f = open(sc, O_RDONLY);
		assert(f >= 0);
		ssize_t bn = read(f, b, sizeof b - 1);
		close(f);
		assert(bn > 0);
		b[bn] = 0;
		assert(!strcmp(b, "{\"update_id\":77,\"message\":{\"chat\":{\"id\":42},\"text\":\"x\"}}"));
		for (int i = 0; i < 40 && cocuk_say(0); i++) {
			usleep(100000);
			cocuk_topla();
		}
		assert(cocuk_say(0) == 0);
		printf("self-test sahte Go: OK (hazir %.2f sn, cocuk sonra toplandi, yabanci sohbet atlandi)\n", dt);
	}
	/* v2: zamanli tetikleri (sahte /proc; /data yok varsayilir: esik 12 MB, sabah 08:00) */
	struct tm st = { .tm_year = 126, .tm_mon = 9, .tm_mday = 23, .tm_hour = 5 };	/* 2026-10-23 05:00 UTC = 08:00 TR */
	time_t sb = timegm(&st);
	assert(sabah_vakti(sb, NULL, "2026-10-22") && !sabah_vakti(sb, NULL, "2026-10-23") && !sabah_vakti(sb - 60, NULL, NULL));
	assert(sabah_vakti(sb + 179 * 60, "08:00", NULL) && !sabah_vakti(sb + 180 * 60, "08:00", NULL));
	assert(!sabah_vakti(sb, "07:30", "2026-10-23") && sabah_vakti(sb, "25:00", NULL) && !sabah_vakti(sb, "09:00", NULL));
	char kok[] = "/tmp/kt-proc-XXXXXX", y[96];
	assert(mkdtemp(kok));
	proc_kok = kok;
	const char *dosyalar[][2] = { { "meminfo", "MemTotal: 56372 kB\nMemAvailable:   15000 kB\n" }, { "vmstat", "pgfault 9\noom_kill 3\n" },
				      { "1/comm", "panel\n" }, { "2/comm", "tailscaled-kucu\n" }, { "3/comm", "panelx\n" },
				      { "12/comm", "kole-asistan-v8\n" } };
	for (size_t i = 0; i < sizeof dosyalar / sizeof dosyalar[0]; i++) {
		if (strchr(dosyalar[i][0], '/')) {
			snprintf(y, sizeof y, "%s/%.*s", kok, (int)(strchr(dosyalar[i][0], '/') - dosyalar[i][0]), dosyalar[i][0]);
			mkdir(y, 0700);
		}
		snprintf(y, sizeof y, "%s/%s", kok, dosyalar[i][0]);
		assert(!dosya_yaz(y, dosyalar[i][1], strlen(dosyalar[i][1]), 0600));
	}
	assert(proc_sayi("meminfo", "MemAvailable") == 15000 && proc_sayi("vmstat", "oom_kill") == 3 && proc_sayi("vmstat", "yok") == -1);
	assert(hizmet_maske() == ((1u << 0) | (1u << 4) | (1u << HZ_ASISTAN)));
	time_t ogle = sb + 7 * 3600;	/* 15:00 TR: sabah disi */
	char nd[64];
	assert(zamanli_tetik(ogle, nd, sizeof nd) && !strcmp(nd, "bellek"));	/* 15000 < (12+4)*1024 */
	snprintf(y, sizeof y, "%s/meminfo", kok);
	assert(!dosya_yaz(y, "MemAvailable: 30000 kB\n", 23, 0600));
	assert(!zamanli_tetik(ogle, nd, sizeof nd) && !nd[0]);
	snprintf(y, sizeof y, "%s/2/comm", kok);
	unlink(y);	/* tailscaled gitti */
	assert(zamanli_tetik(ogle, nd, sizeof nd) && !strcmp(nd, "hizmet"));
	snprintf(y, sizeof y, "%s/vmstat", kok);
	assert(!dosya_yaz(y, "oom_kill 4\n", 11, 0600));
	assert(zamanli_tetik(sb, nd, sizeof nd) && !strcmp(nd, "oom, hizmet, sabah"));
	for (size_t i = 0; i < sizeof dosyalar / sizeof dosyalar[0]; i++) {
		snprintf(y, sizeof y, "%s/%s", kok, dosyalar[i][0]);
		unlink(y);
		char *b = strrchr(y, '/');
		if (strchr(dosyalar[i][0], '/') && b) {
			*b = 0;
			rmdir(y);
		}
	}
	rmdir(kok);
	proc_kok = "/proc";
	puts("self-test zamanli tetik: OK (sabah penceresi 7 durum, /proc okuma, hizmet maskesi, bellek/hizmet/oom/sabah)");
	/* v3: ilk update_id; sahip ogrenme yalniz izin dosyasi + ozel sohbetle */
	assert(ilk_update_id("{\"ok\":true,\"result\":[{\"update_id\":512,\"message\":{}}]}") == 512 && ilk_update_id("{}") == -1);
	char sk[] = "/tmp/kt-sahip-XXXXXX", izin[64], sohbet_y[64];
	assert(mkdtemp(sk));
	snprintf(izin, sizeof izin, "%s/izin", sk);
	snprintf(sohbet_y, sizeof sohbet_y, "%s/sohbet", sk);
	SAHIP_OGREN = izin;
	SOHBET = sohbet_y;
	snprintf(go_yol, sizeof go_yol, "/bin/true");
	sahip[0] = 0;
	const char *bir = "{\"update_id\":1,\"message\":{\"chat\":{\"id\":55}}}";
	memcpy(cevap, bir, strlen(bir) + 1);
	struct guncelleme ga = { 1, 55, 0, (int)strlen(bir), 1, -1 };
	guncelleme_isle(&ga);	/* izin yok: ogrenilmez */
	assert(!sahip[0] && access(sohbet_y, F_OK));
	assert(!dosya_yaz(izin, "1", 1, 0600));
	struct guncelleme gb = { 2, -77, 0, (int)strlen(bir), 1, -1 };
	guncelleme_isle(&gb);	/* grup (negatif): ogrenilmez, izin durur */
	assert(!sahip[0] && !access(izin, F_OK));
	guncelleme_isle(&ga);	/* izin + ozel: ogrenilir, izin silinir */
	char ss[32] = "";
	assert(!strcmp(sahip, "55") && !dosya_oku(sohbet_y, ss, sizeof ss) && !strcmp(ss, "55") && access(izin, F_OK));
	for (int i = 0; i < 20 && cocuk_say(0); i++) {
		usleep(50000);
		cocuk_topla();
	}
	unlink(sohbet_y);
	/* v4 R3: sohbet dosyasi yazilamazsa izin durur, sahip bellekte */
	char yok[96];
	snprintf(yok, sizeof yok, "%s/yok/sohbet", sk);
	SOHBET = yok;
	sahip[0] = 0;
	assert(!dosya_yaz(izin, "1", 1, 0600));
	guncelleme_isle(&ga);
	assert(!strcmp(sahip, "55") && !access(izin, F_OK));
	SOHBET = sohbet_y;	/* yazilabilir oldu: sahibin sonraki mesajinda kaydedilir, izin silinir */
	guncelleme_isle(&ga);
	assert(!dosya_oku(sohbet_y, ss, sizeof ss) && !strcmp(ss, "55") && access(izin, F_OK));
	unlink(sohbet_y);
	for (int i = 0; i < 20 && cocuk_say(0); i++) {
		usleep(50000);
		cocuk_topla();
	}
	unlink(izin);
	unlink("/tmp/tg-guncelleme-1.json");	/* /bin/true dosyayi silmez */
	/* v4 R4: nameserver listesi (ayni satirda iki adres, yorum, bozuk satir) */
	char rc[64];
	snprintf(rc, sizeof rc, "%s/resolv", sk);
	const char *rcm = "# udhcpc\nnameserver 1.1.1.1 1.0.0.1\nnameserverx 9.9.9.9\nsearch lan\nnameserver\t8.8.8.8\n";
	assert(!dosya_yaz(rc, rcm, strlen(rcm), 0600));
	struct in_addr nsl[4];
	int nn = kh_ns_oku(rc, nsl, 4);
	assert(nn == 3 && nsl[0].s_addr == inet_addr("1.1.1.1") && nsl[1].s_addr == inet_addr("1.0.0.1") && nsl[2].s_addr == inet_addr("8.8.8.8"));
	assert(kh_ns_oku(rc, nsl, 2) == 2 && kh_ns_oku("/yok/resolv", nsl, 4) == 0);
	unlink(rc);
	rmdir(sk);
	puts("self-test v3: OK (ilk update_id, sahip ogrenme: izin yok / grup / izin + ozel)");
	puts("self-test v4: OK (sohbet yazilamazsa izin korunur, sonraki mesajda kaydedilir; nameserver listesi 3/2/0)");
	return 0;
}

int main(int argc, char **argv)
{
	int sure = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--self-test"))
			return self_test();
		if (!strcmp(argv[i], "--go") && i + 1 < argc)
			snprintf(go_yol, sizeof go_yol, "%s", argv[++i]);
		else if (!strcmp(argv[i], "--sure-sn") && i + 1 < argc)
			sure = atoi(argv[++i]);
		else {
			fprintf(stderr, "kullanim: kole-telgraf-v7 --go <kole-gemini-v41> [--sure-sn N] | --self-test\n");
			return 2;
		}
	}
	if (access(go_yol, X_OK)) {
		gunluk("HATA: Go ikilisi yok: %s", go_yol);
		return 3;
	}
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, sinyal);
	signal(SIGINT, sinyal);
	mkdir(KUYRUK, 0700);
	gunluk("kole-telgraf-v7 basladi (go %s)", go_yol);
	double bas = simdi(), uyku = 0;
	unsigned long tur = 0, islenen = 0, gonderilen = 0;
	while (!dur && (!sure || simdi() - bas < sure)) {
		if (uyku > 0) {
			durum_yaz(tur, islenen, gonderilen, "bekleme");
			for (double t = simdi() + uyku; !dur && simdi() < t;) {
				usleep(200000);
				cocuk_topla();
			}
			uyku = 0;
			continue;
		}
		tur++;
		cocuk_topla();
		struct stat gs;	/* gozetici gunlugu yalniz cocuk cikinca kirpar; bu surec cikmaz: 256 KB'ta sifirla */
		if (!fstat(1, &gs) && S_ISREG(gs.st_mode) && gs.st_size > 256 * 1024 && !ftruncate(1, 0))
			gunluk("gunluk 256 KB'i asti, sifirlandi");
		if (dosya_oku(JETON, jeton, sizeof jeton)) {
			gunluk("jeton yok: 60 sn");
			uyku = 60;
			continue;
		}
		if (!sahip[0])
			dosya_oku(SOHBET, sahip, sizeof sahip);
		static double kuyruk_sonraki;	/* gonderim hatasindan sonra 30 sn kuyruga bakilmaz (el sikisma firtinasi olmasin) */
		int kuyruk_vakti = sahip[0] && simdi() >= kuyruk_sonraki;
		if (kuyruk_vakti) {
			int k = kuyruk_gonder();
			if (k > 0)
				gonderilen += (unsigned long)k;
			if (k < 0) {
				kuyruk_sonraki = simdi() + 30;
				kuyruk_vakti = 0;
			}
		}
		static double son_denetim = -1e9;
		static int onceki_tetik;
		if (sahip[0] && simdi() - son_denetim >= 60 && !cocuk_say(1)) {	/* v2 */
			char neden[64];
			int t = zamanli_tetik(time(NULL), neden, sizeof neden);
			son_denetim = simdi();
			if (t || onceki_tetik || simdi() - son_zamanli >= 300) {
				pid_t p = go_calistir("--tg-zamanli", NULL, -1);
				if (p > 0) {
					cocuk_ekle(p, 1);
					zamanli_sayi++;
				}
				son_zamanli = simdi();
				if (t || onceki_tetik)
					gunluk("zamanli: %s", t ? neden : "onceki tetik");
			}
			onceki_tetik = t;
		}
		DIR *qd = opendir(KUYRUK);	/* kuyrukta bekleyen varsa uzun sorgu kisa */
		int bekleyen = 0;
		struct dirent *e;
		while (kuyruk_vakti && qd && (e = readdir(qd)))
			bekleyen |= strstr(e->d_name, ".txt") != NULL;
		if (qd)
			closedir(qd);
		int r = guncelleme_turu(bekleyen ? 2 : 20);
		static int hata;
		if (r >= 0) {
			islenen += (unsigned long)r;
			hata = 0;
			durum_yaz(tur, islenen, gonderilen, "tamam");
		} else {
			hata++;
			uyku = r == -2 ? 30 : r == -3 ? (son_retry > 0 && son_retry < 300 ? son_retry : 10) : (hata < 4 ? 5 << (hata - 1) : 60);
			durum_yaz(tur, islenen, gonderilen, r == -2 ? "409" : r == -3 ? "429" : "ag hatasi");
		}
	}
	gunluk("kole-telgraf-v7 cikiyor (%lu tur, %lu guncelleme, %lu bildirim)", tur, islenen, gonderilen);
	return 0;
}
