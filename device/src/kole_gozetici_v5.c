/* kole_gozetici_v5.c (29.10.2026, Claude; scripts/673): v4 + acilis satiri "kole-gozetici v5" (v4 "v3" yaziyordu).
 * --- v4 ---
 * kole_gozetici_v4.c (26.10.2026, Claude; scripts/666, §186): v3 + kosul "!yol" = dosya VARKEN baslatilmaz
 * (ikinci ekran acikken pano). Calisan is durdurulmaz; yalniz yeni baslatma bekler.
 * --- v3 ---
 * kole_gozetici_v3.c (24.10.2026, Claude; denetim-2026-10-24-v3 A3, scripts/630): v2 + OOM ayari cocukta
 * execv'den once yazilir ("x" = 0). v2 kendini -1000 yapip fork ediyordu; "x" cocuklar -1000 miras aliyordu.
 * --- v2 ---
 * kole_gozetici_v2.c (23.10.2026, Claude; denetim-2026-10-23-v2 bulgu 8, scripts/611): v1 + gunluk YERINDE kirpilir
 * (ftruncate + kuyrugu O_APPEND ile geri yazma) ve 60 sn'de bir calisan isler icin de (v1 yalniz cocuk cikinca, .k +
 * rename ile: haftalarca acik hizmetlerin gunlukleri /tmp'de buyuyordu).
 * --- v1 ---
 * kole_gozetici_v1.c (23.10.2026, Claude; Sira 18 madde 136, scripts/578): autorun'un kabuk dongulerinin yerine tek surec.
 * Yapilandirma satiri: ad|bekle_sn|oom|gunluk|azami_kb|saat|kosul|bayrak|env|komut arg...
 *   bekle_sn: cikistan sonra yeniden baslatma bekleme (bayrak kipinde denetim araligi)
 *   oom: sayi -> /proc/<pid>/oom_score_adj; "x" dokunma
 *   azami_kb: >0 ise gunluk bu boyutu asarsa (cocuk cikinca ve 60 sn'de bir) son azami/4 KB kalir (v2: yerinde)
 *   saat: 0 yok; N>0 ilk baslatmadan once yil >= 2026 ya da N sn; -1 her baslatmadan once yil >= 2026
 *   kosul: bossa yok; dosya yoksa baslatilmaz (v4: "!yol" -> dosya varsa baslatilmaz). bayrak: bossa yok; dosya varken calisir, kalkinca SIGTERM
 *   env: "-" ya da "K=V;K=V" (ortama eklenir). komut: bosluklarla ayrilmis (tirnak yok)
 * Calistirma: kole-gozetici-v5 <conf> | --self-test */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define AZAMI_IS 16
#define AZAMI_ARG 16

struct is {
	char ad[32], gunluk[160], kosul[160], bayrak[160], satir[512];
	char *argv[AZAMI_ARG + 1], *env[8];
	int bekle, oom_var, oom, azami_kb, saat, nenv;
	pid_t pid;
	double sonraki, bas;
	int baslatma;
};

static struct is isler[AZAMI_IS];
static int nis;
static volatile sig_atomic_t dur;
static int test_kipi;	/* self-test: yil denetimi atlanir */
static void isle(int s)
{
	(void)s;
	dur = 1;
}

static double mono(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec / 1e9;
}

static void kopya(char *o, size_t n, const char *k)	/* sinirli kopya (snprintf kesme uyarisi yerine) */
{
	size_t m = strlen(k);
	if (m >= n)
		m = n - 1;
	memcpy(o, k, m);
	o[m] = 0;
}

static int var(const char *y)
{
	struct stat st;
	return y[0] && !stat(y, &st);
}

static int saat_dogru(void)
{
	if (test_kipi)
		return 1;
	time_t t = time(NULL);
	struct tm tm;
	localtime_r(&t, &tm);
	return tm.tm_year + 1900 >= 2026;
}

static void gunluk_yaz(struct is *s, const char *m)
{
	FILE *f = fopen(s->gunluk, "a");
	if (!f)
		return;
	time_t t = time(NULL);
	struct tm tm;
	char z[16];
	localtime_r(&t, &tm);
	strftime(z, sizeof z, "%H:%M:%S", &tm);
	fprintf(f, "%s %s %s\n", z, s->ad, m);
	fclose(f);
}

/* azami_kb asildiysa YERINDE son azami/4 KB kalir (ilk tam satirdan). v2: calisan cocuk ayni inode'a O_APPEND ile
 * yazmayi surdurur; kuyruk da O_APPEND ile yazilir. shortcut: pread ile ftruncate arasinda gelen satir kaybolur, ftruncate
 * ile geri yazma arasindaki satir kuyruktan once durur (yalniz gunluk; kilit gerekirse cocuklar gunluge kendisi yazsin). */
static void gunluk_kirp(struct is *s)
{
	struct stat st;
	if (s->azami_kb <= 0 || stat(s->gunluk, &st) || !S_ISREG(st.st_mode) || st.st_size <= (off_t)s->azami_kb * 1024)
		return;
	long kal = (long)s->azami_kb * 256;
	int fd = open(s->gunluk, O_RDWR | O_APPEND | O_CLOEXEC);
	if (fd < 0)
		return;
	char *b = malloc((size_t)kal);
	ssize_t n = b ? pread(fd, b, (size_t)kal, st.st_size - kal) : -1;
	if (n > 0) {
		char *p = memchr(b, '\n', (size_t)n);
		size_t bas = p && p + 1 < b + n ? (size_t)(p + 1 - b) : 0;
		if (!ftruncate(fd, 0)) {
			ssize_t w = write(fd, b + bas, (size_t)n - bas);
			(void)w;
		}
	}
	free(b);
	close(fd);
}

/* satir -> is; 0 tamam */
static int satir_coz(const char *satir, struct is *s)
{
	memset(s, 0, sizeof *s);
	s->pid = -1;
	kopya(s->satir, sizeof s->satir, satir);
	char *alan[10], *p = s->satir;
	for (int i = 0; i < 10; i++) {
		alan[i] = p;
		if (i < 9) {
			p = strchr(p, '|');
			if (!p)
				return -1;
			*p++ = 0;
		}
	}
	alan[9][strcspn(alan[9], "\r\n")] = 0;
	kopya(s->ad, sizeof s->ad, alan[0]);
	s->bekle = atoi(alan[1]);
	s->oom_var = strcmp(alan[2], "x") != 0;
	s->oom = atoi(alan[2]);
	kopya(s->gunluk, sizeof s->gunluk, alan[3]);
	s->azami_kb = atoi(alan[4]);
	s->saat = atoi(alan[5]);
	kopya(s->kosul, sizeof s->kosul, alan[6]);
	kopya(s->bayrak, sizeof s->bayrak, alan[7]);
	if (strcmp(alan[8], "-")) {
		for (char *e = strtok(alan[8], ";"); e && s->nenv < 7; e = strtok(NULL, ";"))
			s->env[s->nenv++] = e;
	}
	int n = 0;
	for (char *a = strtok(alan[9], " "); a && n < AZAMI_ARG; a = strtok(NULL, " "))
		s->argv[n++] = a;
	s->argv[n] = NULL;
	if (!s->ad[0] || !n || s->bekle < 1 || !s->gunluk[0])
		return -1;
	return 0;
}

static void baslat(struct is *s)
{
	pid_t c = fork();
	if (c < 0) {
		s->sonraki = mono() + s->bekle;
		return;
	}
	if (c == 0) {
		int g = open(s->gunluk, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644), z = open("/dev/null", O_RDONLY);
		if (z >= 0)
			dup2(z, 0);
		if (g >= 0) {
			dup2(g, 1);
			dup2(g, 2);
		}
		for (int i = 0; i < s->nenv; i++)
			putenv(s->env[i]);
		signal(SIGTERM, SIG_DFL);
		int o = open("/proc/self/oom_score_adj", O_WRONLY | O_CLOEXEC);	/* v3: gozeticinin -1000'i miras kalmasin */
		if (o >= 0) {
			char ob[16];
			int on = snprintf(ob, sizeof ob, "%d\n", s->oom_var ? s->oom : 0);
			ssize_t w = write(o, ob, (size_t)on);
			(void)w;
			close(o);
		}
		execv(s->argv[0], s->argv);
		_exit(127);
	}
	s->pid = c;
	s->bas = mono();
	s->baslatma++;
}

/* bir tur: cikanlari topla, kosullara gore baslat/durdur */
static void tur(double bas)
{
	int st;
	pid_t w;
	while ((w = waitpid(-1, &st, WNOHANG)) > 0) {
		for (int i = 0; i < nis; i++) {
			struct is *s = &isler[i];
			if (s->pid != w)
				continue;
			char m[96];
			int rc = WIFEXITED(st) ? WEXITSTATUS(st) : 128 + WTERMSIG(st);
			snprintf(m, sizeof m, "cikti (%d), %d sn sonra yeniden", rc, s->bekle);
			if (!s->bayrak[0])
				gunluk_yaz(s, m);
			gunluk_kirp(s);
			s->pid = -1;
			s->sonraki = mono() + s->bekle;
		}
	}
	double now = mono();
	static double son_kirp;
	if (now - son_kirp >= 60) {	/* v2: calisan islerin gunlukleri de */
		son_kirp = now;
		for (int i = 0; i < nis; i++)
			gunluk_kirp(&isler[i]);
	}
	for (int i = 0; i < nis; i++) {
		struct is *s = &isler[i];
		if (s->bayrak[0]) {
			int b = var(s->bayrak);
			if (s->pid > 0 && !b) {
				kill(s->pid, SIGTERM);
				gunluk_yaz(s, "durduruldu (bayrak yok)");
			} else if (s->pid < 0 && b && now >= s->sonraki) {
				baslat(s);
				gunluk_yaz(s, "basladi (bayrak var)");
			}
			continue;
		}
		if (s->pid > 0 || now < s->sonraki)
			continue;
		if (s->kosul[0] && (s->kosul[0] == '!' ? var(s->kosul + 1) : !var(s->kosul))) {	/* v4: ! ters */
			s->sonraki = now + s->bekle;
			continue;
		}
		if ((s->saat < 0 || (s->saat > 0 && !s->baslatma && now - bas < s->saat)) && !saat_dogru()) {
			s->sonraki = now + (s->saat < 0 ? s->bekle : 2);
			continue;
		}
		baslat(s);
	}
}

static int yukle(const char *conf)
{
	FILE *f = fopen(conf, "r");
	if (!f)
		return -1;
	char b[512];
	nis = 0;
	while (fgets(b, sizeof b, f) && nis < AZAMI_IS) {
		if (b[0] == '#' || b[0] == '\n')
			continue;
		if (satir_coz(b, &isler[nis]))
			fprintf(stderr, "kole-gozetici: gecersiz satir: %s", b);
		else
			nis++;
	}
	fclose(f);
	return nis ? 0 : -1;
}

static int self_test(void)
{
	char d[] = "/tmp/kole-gozetici-test-XXXXXX";
	if (!mkdtemp(d))
		return 1;
	char conf[128], g1[128], g2[128], g3[128], bay[128], kos[128];
	snprintf(conf, sizeof conf, "%s/c", d);
	snprintf(g1, sizeof g1, "%s/g1", d);
	snprintf(g2, sizeof g2, "%s/g2", d);
	snprintf(g3, sizeof g3, "%s/g3", d);
	snprintf(bay, sizeof bay, "%s/bayrak", d);
	snprintf(kos, sizeof kos, "%s/kosul", d);
	char g4t[200];	/* v4: ters kosullu isin gunlugu */
	snprintf(g4t, sizeof g4t, "%s/g4t", d);
	FILE *f = fopen(conf, "w");
	fprintf(f, "# test\nyeniden|1|x|%s|1|0|||TEST_K=deger|/bin/sh -c echo_$TEST_K\n", g1);
	fprintf(f, "bayrakli|1|x|%s|0|0||%s|-|/bin/sleep 30\n", g2, bay);
	fprintf(f, "kosullu|1|x|%s|0|0|%s||-|/bin/echo kosul_tamam\n", g3, kos);
	fprintf(f, "tersli|1|x|%s|0|0|!%s||-|/bin/echo ters_tamam\n", g4t, kos);	/* v4 */
	fprintf(f, "bozuk|satir\n");
	fclose(f);
	test_kipi = 1;
	int hata = 0;
	if (yukle(conf) || nis != 4) {	/* v4: + tersli */
		printf("self-test: yukleme %d is (beklenen 4)\n", nis);
		return 1;
	}
	/* sh -c 'echo_$TEST_K' ozel: komut bosluklarla bolunur; ortam degiskeni sh icinde acilir */
	double b0 = mono();
	FILE *fb = NULL;
	while (mono() - b0 < 4.5) {
		tur(b0);
		if (mono() - b0 > 1.5 && !fb) {
			fb = fopen(bay, "w");	/* 1.5 sn: bayrak -> bayrakli baslar */
			if (fb)
				fclose(fb);
		}
		if (mono() - b0 > 3.0)
			unlink(bay);	/* 3 sn: bayrak kalkar -> durdurulur */
		usleep(100000);
	}
	tur(b0);
	usleep(200000);
	tur(b0);
	struct is *y = &isler[0], *ba = &isler[1], *ko = &isler[2], *te = &isler[3];
	if (te->baslatma < 1) {	/* v4: kosul dosyasi yokken ters kosullu baslamali */
		printf("self-test: ters kosullu dosya yokken baslamadi\n");
		hata++;
	}
	char b[4096] = {0};
	f = fopen(g1, "r");
	size_t n = f ? fread(b, 1, sizeof b - 1, f) : 0;
	if (f)
		fclose(f);
	b[n] = 0;
	if (y->baslatma < 3 || !strstr(b, "cikti (127)")) {	/* echo_deger komutu yok -> sh 127 */
		printf("self-test: yeniden baslatma %d, gunluk '%.80s'\n", y->baslatma, b);
		hata++;
	}
	if (!strstr(b, "echo_deger")) {
		printf("self-test: ortam degiskeni gecmedi\n");
		hata++;
	}
	if (ba->baslatma != 1 || ba->pid > 0) {
		printf("self-test: bayrakli baslatma %d pid %d\n", ba->baslatma, (int)ba->pid);
		hata++;
	}
	if (ko->baslatma) {
		printf("self-test: kosulsuz baslamamaliydi\n");
		hata++;
	}
	fb = fopen(kos, "w");
	if (fb)
		fclose(fb);
	ko->sonraki = 0;
	usleep(200000);
	tur(b0);	/* tersli (echo) cikti, toplanir */
	int te_once = te->baslatma;
	te->sonraki = 0;
	tur(b0);
	if (te->baslatma != te_once) {	/* v4: dosya varken baslamamali */
		printf("self-test: ters kosullu dosya varken basladi\n");
		hata++;
	}
	if (ko->baslatma != 1) {
		printf("self-test: kosul gelince baslamadi\n");
		hata++;
	}
	/* gunluk kirpma: 1 KB sinirli gunluk buyuyunce son 256 bayt kalir */
	f = fopen(g1, "a");
	for (int i = 0; i < 200; i++)
		fprintf(f, "doldurma satiri %03d\n", i);
	fclose(f);
	gunluk_kirp(y);
	struct stat st;
	if (stat(g1, &st) || st.st_size > 256) {
		printf("self-test: kirpma %ld bayt\n", (long)st.st_size);
		hata++;
	}
	/* v2: yerinde kirpma; acik O_APPEND yazici sonra yeni sona yazar (seyrek dosya/NUL yok), ilk tam satirdan baslar */
	char g4[128];
	snprintf(g4, sizeof g4, "%s/g4", d);
	int af = open(g4, O_WRONLY | O_CREAT | O_APPEND, 0644);
	for (int i = 0; af >= 0 && i < 200; i++)
		dprintf(af, "yazici satiri %03d\n", i);
	struct is ya = { .azami_kb = 1 };
	kopya(ya.gunluk, sizeof ya.gunluk, g4);
	gunluk_kirp(&ya);
	if (af >= 0)
		dprintf(af, "SON\n");
	char b4[600] = {0};
	f = fopen(g4, "r");
	size_t n4 = f ? fread(b4, 1, sizeof b4 - 1, f) : 0;
	if (f)
		fclose(f);
	if (af < 0 || n4 < 4 || n4 > 260 || strlen(b4) != n4 || strncmp(b4, "yazici satiri ", 14) || strcmp(b4 + n4 - 4, "SON\n")) {
		printf("self-test: yerinde kirpma %zu bayt '%.40s'\n", n4, b4);
		hata++;
	}
	if (af >= 0)
		close(af);
	/* v3: OOM ayari cocukta: gozetici 300 iken "x" cocuk 0, "250" cocuk 250 */
	FILE *oo = fopen("/proc/self/oom_score_adj", "w");
	int ust = oo && fputs("300\n", oo) >= 0;
	if (oo && fclose(oo))
		ust = 0;
	static const char *const oom_ist[2] = {"x", "250"}, *const oom_bek[2] = {"0", "250"};
	for (int i = 0; ust && i < 2; i++) {
		char go[128], so[300], ob[32] = "";
		snprintf(go, sizeof go, "%s/oom%d", d, i);
		snprintf(so, sizeof so, "oom%d|1|%s|%s|0|0|||-|/bin/cat /proc/self/oom_score_adj", i, oom_ist[i], go);
		struct is oi;
		if (satir_coz(so, &oi)) {
			hata++;
			continue;
		}
		baslat(&oi);
		if (oi.pid > 0)
			waitpid(oi.pid, NULL, 0);
		FILE *fo = fopen(go, "r");
		if (!fo || !fgets(ob, sizeof ob, fo))
			ob[0] = 0;
		if (fo)
			fclose(fo);
		ob[strcspn(ob, "\n")] = 0;
		if (strcmp(ob, oom_bek[i])) {
			printf("self-test: v3 oom '%s' cocugu %s (beklenen %s)\n", oom_ist[i], ob, oom_bek[i]);
			hata++;
		}
	}
	oo = fopen("/proc/self/oom_score_adj", "w");
	if (oo) {
		fputs("0\n", oo);
		fclose(oo);
	}
	if (!ust)
		printf("self-test: v3 oom testi atlandi (oom_score_adj yazilamadi)\n");
	struct is t;
	if (!satir_coz("a|0|x|/g|0|0|||-|/bin/x", &t) || !satir_coz("a|1|x||0|0|||-|/bin/x", &t) || satir_coz("m|3|-500|/g|0|0|||-|/bin/m --p 1", &t) ||
	    t.oom != -500 || !t.oom_var || strcmp(t.argv[2], "1") || t.argv[3]) {
		printf("self-test: satir_coz sinirlari\n");
		hata++;
	}
	for (int i = 0; i < nis; i++)
		if (isler[i].pid > 0)
			kill(isler[i].pid, SIGKILL);
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	char k[200];
	snprintf(k, sizeof k, "rm -rf %s", d);
	if (system(k))
		hata++;
	printf("self-test: kole-gozetici v4 %s (ters kosul, yeniden %d, bayrakli %d, kosullu %d, yerinde kirpma, cocuk oom)\n", hata ? "HATA" : "OK", y->baslatma, ba->baslatma,
	       ko->baslatma);
	return hata ? 1 : 0;
}

int main(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "--self-test"))
		return self_test();
	if (argc < 2 || yukle(argv[1])) {
		fprintf(stderr, "kullanim: kole-gozetici-v3 <conf> (gecerli satir yok)\n");
		return 2;
	}
	FILE *o = fopen("/proc/self/oom_score_adj", "w");
	if (o) {
		fputs("-1000\n", o);
		fclose(o);
	}
	struct sigaction sa;
	memset(&sa, 0, sizeof sa);
	sa.sa_handler = isle;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);
	setvbuf(stdout, NULL, _IOLBF, 0);
	printf("kole-gozetici v5: %d is\n", nis);
	for (int i = 0; i < nis; i++)
		printf("  %s: %s (bekle %d)\n", isler[i].ad, isler[i].argv[0], isler[i].bekle);
	double bas = mono();
	while (!dur) {
		tur(bas);
		struct timespec a = {1, 0};
		nanosleep(&a, NULL);
	}
	printf("kole-gozetici: durdu (sinyal; cocuklar suruyor)\n");
	return 0;
}
