/* kole-tlstest-v1 (23.10.2026, Claude; Sira 18 madde 135): kole_https_v1.h denemesi. Cihazda C + BearSSL HTTPS'in
 * calistigini ve bellek maliyetini olcer (Go Telegram sureci RSS 5.9 MB ile kiyas icin).
 *   kole-tlstest-v1 <ad> [yol]   -> GET https://<ad><yol>: DURUM, govde boyu, sureler, VmHWM/VmRSS
 *   kole-tlstest-v1 --self-test  -> ag yok: DNS sorgu/cevap cozumu, HTTP cevap ayristirma (Content-Length, chunked,
 *                                    kapanisa kadar, tasma, kesik), 1 baytlik parcalarla
 * Govde YAZDIRILMAZ (anahtar/jeton icerebilecek yollar icin de guvenli); yalniz boyu ve ilk satirin ilk 60 karakteri
 * --ilk ile. */
#include "kole_https_v1.h"
#include <assert.h>

static double simdi_ms(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (double)t.tv_sec * 1000.0 + (double)t.tv_nsec / 1e6;
}

struct bellek_okur {
	const char *v;
	size_t n, k, parca;
};

static int bellek_oku(void *c, unsigned char *b, size_t n)
{
	struct bellek_okur *o = c;
	if (o->k >= o->n)
		return 0;
	size_t m = o->n - o->k;
	if (m > n)
		m = n;
	if (o->parca && m > o->parca)
		m = o->parca;
	memcpy(b, o->v + o->k, m);
	o->k += m;
	return (int)m;
}

static int http_dene(const char *v, size_t parca, char *g, size_t max, size_t *uz)
{
	struct bellek_okur o = { v, strlen(v), 0, parca };
	return kh_http_cevap(bellek_oku, &o, g, max, uz);
}

static int self_test(void)
{
	/* DNS sorgusu */
	uint8_t q[300];
	int qn = kh_dns_sorgu(q, sizeof q, "api.telegram.org", 0x1234);
	static const uint8_t bek[] = { 0x12, 0x34, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3, 'a', 'p', 'i', 8, 't', 'e', 'l', 'e', 'g',
				       'r', 'a', 'm', 3, 'o', 'r', 'g', 0, 0, 1, 0, 1 };
	assert(qn == (int)sizeof bek && !memcmp(q, bek, sizeof bek));
	assert(kh_dns_sorgu(q, sizeof q, "a..b", 1) < 0 && kh_dns_sorgu(q, sizeof q, "", 1) < 0);
	/* cevap: soru + CNAME (sikistirilmis ad) + A 149.154.167.220 */
	uint8_t r[200];
	memcpy(r, bek, sizeof bek);
	r[2] = 0x81;
	r[3] = 0x80;
	r[7] = 2;	/* ANCOUNT */
	size_t k = sizeof bek;
	static const uint8_t cname[] = { 0xC0, 0x0C, 0, 5, 0, 1, 0, 0, 0, 60, 0, 2, 0xC0, 0x0C };
	static const uint8_t a[] = { 0xC0, 0x0C, 0, 1, 0, 1, 0, 0, 0, 60, 0, 4, 149, 154, 167, 220 };
	memcpy(r + k, cname, sizeof cname);
	k += sizeof cname;
	memcpy(r + k, a, sizeof a);
	k += sizeof a;
	struct in_addr ip;
	assert(!kh_dns_coz(r, k, 0x1234, &ip) && !strcmp(inet_ntoa(ip), "149.154.167.220"));
	assert(kh_dns_coz(r, k, 0x1235, &ip) < 0);	/* yanlis kimlik */
	assert(kh_dns_coz(r, k - 3, 0x1234, &ip) < 0);	/* kesik */
	r[3] = 0x83;	/* NXDOMAIN */
	assert(kh_dns_coz(r, k, 0x1234, &ip) < 0);
	assert(!kh_dns("10.1.2.3", &ip, 10) && !strcmp(inet_ntoa(ip), "10.1.2.3"));
	puts("self-test DNS: OK (sorgu baytlari, CNAME+A, kimlik, kesik, NXDOMAIN, sayisal)");

	/* HTTP */
	char g[64];
	size_t uz;
	for (size_t parca = 0; parca <= 1; parca++) {
		assert(http_dene("HTTP/1.1 200 OK\r\nContent-Type: x\r\ncontent-length: 5\r\n\r\nhello", parca, g, sizeof g, &uz) == 200 &&
		       uz == 5 && !strcmp(g, "hello"));
		assert(http_dene("HTTP/1.1 302 Found\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6;x=1\r\n world\r\n0\r\nT: 1\r\n\r\n",
				 parca, g, sizeof g, &uz) == 302 && uz == 11 && !strcmp(g, "hello world"));
		assert(http_dene("HTTP/1.0 404 Not Found\r\n\r\nyok burada", parca, g, sizeof g, &uz) == 404 && !strcmp(g, "yok burada"));
		assert(http_dene("HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\nkisa", parca, g, sizeof g, &uz) == -1);
		assert(http_dene("HTTP/1.1 200 OK\r\nContent-Length: 64\r\n\r\n", parca, g, sizeof g, &uz) == -2);
		assert(http_dene("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n40\r\n", parca, g, 16, &uz) == -1 ||
		       http_dene("HTTP/1.1 200 OK\r\nTransfer-Encoding: chunked\r\n\r\n40\r\n", parca, g, 16, &uz) == -2);
		assert(http_dene("SMTP hazir\r\n\r\n", parca, g, sizeof g, &uz) == -1);
		assert(http_dene("HTTP/1.1 200 OK\r\nContent-Length: 2\r\n", parca, g, sizeof g, &uz) == -1);
	}
	puts("self-test HTTP: OK (Content-Length, chunked+uzanti+trailer, kapanisa kadar, kesik, tasma, bozuk; 1 baytlik parca)");
	printf("self-test: OK (gomulu kok sertifika %d)\n", (int)TAs_NUM);
	return 0;
}

static void bellek_yaz(void)
{
	FILE *f = fopen("/proc/self/status", "r");
	char s[256];
	if (!f)
		return;
	while (fgets(s, sizeof s, f))
		if (!strncmp(s, "VmHWM:", 6) || !strncmp(s, "VmRSS:", 6) || !strncmp(s, "RssAnon:", 8))
			fputs(s, stdout);
	fclose(f);
}

int main(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "--self-test"))
		return self_test();
	if (argc < 2 || argv[1][0] == '-') {
		fprintf(stderr, "kullanim: kole-tlstest-v1 <ad> [yol] [--ilk] | --self-test\n");
		return 2;
	}
	const char *ad = argv[1], *yol = argc > 2 && argv[2][0] == '/' ? argv[2] : "/";
	int ilk = argc > 2 && !strcmp(argv[argc - 1], "--ilk");
	static struct kh k;
	static char govde[128 * 1024];
	size_t uz;
	double t0 = simdi_ms();
	struct in_addr ip;
	if (kh_dns(ad, &ip, 5000)) {
		printf("HATA %s\n", kh_hata);
		return 1;
	}
	double t1 = simdi_ms();
	int kod = kh_tek(&k, "GET", ad, yol, "Accept: */*\r\n", NULL, 0, govde, sizeof govde, &uz, 10000);
	double t2 = simdi_ms();
	if (kod < 0)
		printf("HATA %s\n", kh_hata);
	else
		printf("DURUM %d govde %zu bayt; dns %.0f ms (%s), dns+tcp+tls+http %.0f ms\n", kod, uz, t1 - t0, inet_ntoa(ip), t2 - t1);
	if (kod > 0 && ilk) {
		char *nl = strchr(govde, '\n');
		if (nl)
			*nl = 0;
		printf("ILK: %.60s\n", govde);
	}
	bellek_yaz();
	return kod > 0 ? 0 : 1;
}
