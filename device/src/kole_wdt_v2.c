/* kole_wdt_v2.c (23.10.2026, Claude; denetim-2026-10-23-v2 bulgu 13, scripts/611): v1 + mlockall(MCL_CURRENT |
 *   MCL_FUTURE | MCL_ONFAULT): ikili SD'den calisiyor; bellek sikisinca besleme dongusunun kod sayfasi atilip SD okumasi
 *   12+ sn takilirsa saglikli sistem resetlenirdi. ONFAULT: yalniz dokunulan sayfalar (RAM bedeli bugunku RSS kadar).
 * --- v1 ---
 * kole_wdt_v1.c (23.10.2026, Claude; Sira 18 madde 134, scripts/573): watchdog besleyici. /init'in
 *   nice -n -10 sh -c 'while true; do devmem 0x020500B0 32 0x14af; sleep 4; done'
 * dongusu 4 sn'de bir alt surec aciyordu (her fork order-2 cekirdek yigini). Ayni yazmaca ayni degeri /dev/mem mmap ile,
 * surec acmadan yazar. nice -10 ve oom_score_adj -1000. /tmp/kole-wdt.durum her beslemede "besleme N mono S".
 * 75 beslemede (5 dk) /data/kole-wdt-deneme silinir (autorun_v38 acilis dongusu korumasi). SIGTERM/SIGINT: cikar;
 * init dongusu durdurulmussa besleme durur ve 16 sn'de reset gelir (donma bekcisi davranisi bilerek korunur).
 * Calistirma: kole-wdt-v2 [--self-test] */
#define _GNU_SOURCE
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define WDT_TABAN 0x02050000UL
#define WDT_BESLE 0xB0
#define WDT_DEGER 0x14afU
#define DURUM "/tmp/kole-wdt.durum"
#define DENEME "/data/kole-wdt-deneme"
#define DENEME_TUR 75
#ifndef MCL_ONFAULT
#define MCL_ONFAULT 4	/* Linux 4.4+ */
#endif

static volatile sig_atomic_t dur;
static void isle(int s)
{
	(void)s;
	dur = 1;
}

/* tur < 0: sonsuz. Her turda once besle, sonra durum yaz, deneme_tur'da deneme dosyasini sil, sonra uyu */
static long calis(volatile uint32_t *taban, long tur, long aralik_ms, const char *durum, const char *deneme, long deneme_tur)
{
	struct timespec a = {aralik_ms / 1000, (aralik_ms % 1000) * 1000000L};
	long n = 0;
	while (!dur && (tur < 0 || n < tur)) {
		taban[WDT_BESLE / 4] = WDT_DEGER;
		n++;
		FILE *f = fopen(durum, "w");
		if (f) {
			struct timespec m;
			clock_gettime(CLOCK_MONOTONIC, &m);
			fprintf(f, "besleme %ld mono %ld\n", n, (long)m.tv_sec);
			fclose(f);
		}
		if (n == deneme_tur)
			unlink(deneme);
		nanosleep(&a, NULL);
	}
	return n;
}

static int self_test(void)
{
	static uint32_t sahte[1024];
	const char *dz = "/tmp/kole-wdt-test.durum", *dn = "/tmp/kole-wdt-test.deneme";
	FILE *f = fopen(dn, "w");
	if (f)
		fclose(f);
	long n = calis(sahte, 3, 10, dz, dn, 2);
	char b[64] = {0};
	f = fopen(dz, "r");
	if (f) {
		if (!fgets(b, sizeof b, f))
			b[0] = 0;
		fclose(f);
	}
	struct stat st;
	int ok = n == 3 && sahte[WDT_BESLE / 4] == WDT_DEGER && !strncmp(b, "besleme 3 mono ", 15) && stat(dn, &st) != 0;
	for (int i = 0; i < 1024; i++)
		if (i != WDT_BESLE / 4 && sahte[i])
			ok = 0;	/* yalniz besleme yazmaci */
	dur = 1;
	ok = ok && calis(sahte, 5, 10, dz, dn, 99) == 0;	/* sinyal sonrasi besleme yok */
	unlink(dz);
	printf("self-test: kole-wdt v2 %s (tur %ld, deger %#x, durum '%.20s')\n", ok ? "OK" : "HATA", n, (unsigned)sahte[WDT_BESLE / 4], b);
	return ok ? 0 : 1;
}

int main(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "--self-test"))
		return self_test();
	int fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC);
	if (fd < 0) {
		perror("kole-wdt: /dev/mem");
		return 1;
	}
	void *m = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, WDT_TABAN);
	if (m == MAP_FAILED) {
		perror("kole-wdt: mmap");
		return 1;
	}
	if (setpriority(PRIO_PROCESS, 0, -10))
		perror("kole-wdt: setpriority");
	if (mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT))	/* v2: sayfalar SD'ye geri atilmaz */
		perror("kole-wdt: mlockall");
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
	printf("kole-wdt v2: %#lx+%#x <- %#x, 4 sn, bellege kilitli\n", WDT_TABAN, WDT_BESLE, WDT_DEGER);
	fflush(stdout);
	long n = calis((volatile uint32_t *)m, -1, 4000, DURUM, DENEME, DENEME_TUR);
	printf("kole-wdt: durdu (sinyal), %ld besleme\n", n);
	return 0;
}
