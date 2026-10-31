/* kole_https_v1.h (23.10.2026, Claude; Sira 18 madde 135): kucuk HTTPS istemcisi. kole-gemini'nin Telegram dongusunu
 * Go'dan C'ye tasimanin temeli (Go sureci RSS 5.9 MB + takas 1.8 MB). TLS: BearSSL 0.6 (bearssl.org, scripts/584),
 * kok sertifikalari derlemede gomulu (ta_kokler_v1.h, `brssl ta` ile Mozilla paketinden; .rodata = dosya destekli,
 * anonim RSS degil). TLS 1.2 (BearSSL 1.3 bilmez; Telegram/Google 1.2 kabul eder).
 *  - DNS: UDP, /etc/resolv.conf ilk nameserver (KH_RESOLV ile baska dosya), A kaydi, CNAME atlanir, 2 deneme.
 *  - TCP: engellemesiz connect + poll zaman asimi; okuma/yazma geri cagrilari da poll'lu (k->ms).
 *  - HTTP/1.1 tek istek (Connection: close); cevap Content-Length / chunked / kapanisa kadar; govde cagiranin tamponuna.
 * Bellek: baglanti basina struct kh (~40 KB: BR_SSL_BUFSIZE_BIDI tampon + baglam). Hata metni kh_hata[]. */
#ifndef KOLE_HTTPS_V1_H
#define KOLE_HTTPS_V1_H
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include "bearssl.h"
#include "ta_kokler_v1.h"

static char kh_hata[160];

static void kh_hata_yaz(const char *b, ...)
{
	va_list a;
	va_start(a, b);
	vsnprintf(kh_hata, sizeof kh_hata, b, a);
	va_end(a);
}

/* ---- DNS ---- */
static int kh_dns_sorgu(uint8_t *q, size_t n, const char *ad, uint16_t id)
{
	size_t k = 12, al = strlen(ad);
	if (al == 0 || al > 253 || n < 12 + al + 2 + 4)
		return -1;
	memset(q, 0, 12);
	q[0] = (uint8_t)(id >> 8);
	q[1] = (uint8_t)id;
	q[2] = 0x01;	/* RD */
	q[5] = 1;	/* QDCOUNT */
	const char *p = ad;
	while (*p) {
		const char *nok = strchr(p, '.');
		size_t l = nok ? (size_t)(nok - p) : strlen(p);
		if (l == 0 || l > 63)
			return -1;
		q[k++] = (uint8_t)l;
		memcpy(q + k, p, l);
		k += l;
		p += l;
		if (*p == '.')
			p++;
	}
	q[k++] = 0;
	q[k++] = 0;
	q[k++] = 1;	/* A */
	q[k++] = 0;
	q[k++] = 1;	/* IN */
	return (int)k;
}

static int kh_ad_atla(const uint8_t *r, size_t n, size_t *k)
{
	while (*k < n) {
		uint8_t l = r[*k];
		if (l == 0) {
			(*k)++;
			return 0;
		}
		if ((l & 0xC0) == 0xC0) {
			if (*k + 2 > n)
				return -1;
			*k += 2;
			return 0;
		}
		if (l & 0xC0)
			return -1;
		*k += 1 + (size_t)l;
	}
	return -1;
}

static int kh_dns_coz(const uint8_t *r, size_t n, uint16_t id, struct in_addr *o)
{
	if (n < 12 || ((r[0] << 8) | r[1]) != id || !(r[2] & 0x80) || (r[3] & 0x0F))
		return -1;
	unsigned qd = (unsigned)((r[4] << 8) | r[5]), an = (unsigned)((r[6] << 8) | r[7]);
	size_t k = 12;
	for (unsigned i = 0; i < qd; i++) {
		if (kh_ad_atla(r, n, &k) || k + 4 > n)
			return -1;
		k += 4;
	}
	for (unsigned i = 0; i < an; i++) {
		if (kh_ad_atla(r, n, &k) || k + 10 > n)
			return -1;
		unsigned tip = (unsigned)((r[k] << 8) | r[k + 1]), sinif = (unsigned)((r[k + 2] << 8) | r[k + 3]);
		size_t rl = (size_t)((r[k + 8] << 8) | r[k + 9]);
		k += 10;
		if (k + rl > n)
			return -1;
		if (tip == 1 && sinif == 1 && rl == 4) {
			memcpy(&o->s_addr, r + k, 4);
			return 0;
		}
		k += rl;
	}
	return -1;
}

static uint16_t kh_rastgele16(void)
{
	uint16_t v = 0;
	int f = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	if (f < 0 || read(f, &v, sizeof v) != (ssize_t)sizeof v)
		v = (uint16_t)(getpid() ^ (unsigned)time(NULL));
	if (f >= 0)
		close(f);
	return v;
}

static int kh_dns(const char *ad, struct in_addr *o, int ms)
{
	if (inet_aton(ad, o))
		return 0;
	char ns[64] = "", s[256];
	const char *rc = getenv("KH_RESOLV") ? getenv("KH_RESOLV") : "/etc/resolv.conf";
	FILE *f = fopen(rc, "r");
	if (f) {
		while (fgets(s, sizeof s, f))
			if (sscanf(s, "nameserver %63s", ns) == 1)
				break;
		fclose(f);
	}
	struct sockaddr_in sa = { .sin_family = AF_INET, .sin_port = htons(53) };
	if (!ns[0] || !inet_aton(ns, &sa.sin_addr)) {
		kh_hata_yaz("dns: %s icinde nameserver yok", rc);
		return -1;
	}
	uint8_t q[300], r[1500];
	uint16_t id = kh_rastgele16();
	int qn = kh_dns_sorgu(q, sizeof q, ad, id);
	if (qn < 0) {
		kh_hata_yaz("dns: gecersiz ad");
		return -1;
	}
	int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (fd < 0) {
		kh_hata_yaz("dns: socket %s", strerror(errno));
		return -1;
	}
	for (int dene = 0; dene < 2; dene++) {
		if (sendto(fd, q, (size_t)qn, 0, (struct sockaddr *)&sa, sizeof sa) != qn)
			continue;
		struct pollfd p = { fd, POLLIN, 0 };
		while (poll(&p, 1, ms / 2) == 1) {
			ssize_t m = recv(fd, r, sizeof r, 0);
			if (m > 0 && !kh_dns_coz(r, (size_t)m, id, o)) {
				close(fd);
				return 0;
			}
			if (m <= 0)
				break;
		}
	}
	close(fd);
	kh_hata_yaz("dns: %s cozulemedi (%s)", ad, ns);
	return -1;
}

/* ---- TCP + TLS ---- */
struct kh {
	int fd, ms, zaman_asimi;
	br_ssl_client_context sc;
	br_x509_minimal_context xc;
	br_sslio_context io;
	unsigned char tampon[BR_SSL_BUFSIZE_BIDI];
};

static int kh_tcp(struct in_addr ip, int port, int ms)
{
	int fd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (fd < 0)
		return -1;
	struct sockaddr_in sa = { .sin_family = AF_INET, .sin_port = htons((uint16_t)port), .sin_addr = ip };
	if (connect(fd, (struct sockaddr *)&sa, sizeof sa) && errno != EINPROGRESS) {
		kh_hata_yaz("tcp: connect %s", strerror(errno));
		close(fd);
		return -1;
	}
	struct pollfd p = { fd, POLLOUT, 0 };
	int e = 0;
	socklen_t el = sizeof e;
	if (poll(&p, 1, ms) != 1 || getsockopt(fd, SOL_SOCKET, SO_ERROR, &e, &el) || e) {
		kh_hata_yaz("tcp: %s:%d %s", inet_ntoa(ip), port, e ? strerror(e) : "zaman asimi");
		close(fd);
		return -1;
	}
	return fd;
}

static int kh_dusuk_oku(void *c, unsigned char *b, size_t n)
{
	struct kh *k = c;
	for (;;) {
		ssize_t r = read(k->fd, b, n);
		if (r > 0)
			return (int)r;
		if (r == 0)
			return -1;
		if (errno == EINTR)
			continue;
		if (errno != EAGAIN)
			return -1;
		struct pollfd p = { k->fd, POLLIN, 0 };
		if (poll(&p, 1, k->ms) != 1) {
			k->zaman_asimi = 1;
			return -1;
		}
	}
}

static int kh_dusuk_yaz(void *c, const unsigned char *b, size_t n)
{
	struct kh *k = c;
	for (;;) {
		ssize_t r = write(k->fd, b, n);
		if (r > 0)
			return (int)r;
		if (r < 0 && errno == EINTR)
			continue;
		if (r < 0 && errno != EAGAIN)
			return -1;
		struct pollfd p = { k->fd, POLLOUT, 0 };
		if (poll(&p, 1, k->ms) != 1) {
			k->zaman_asimi = 1;
			return -1;
		}
	}
}

/* ad:port'a TLS baglantisi hazirlar (el sikisma ilk yazmada). 0 tamam. */
static int kh_ac(struct kh *k, const char *ad, int port, int ms)
{
	struct in_addr ip;
	k->fd = -1;
	k->ms = ms;
	k->zaman_asimi = 0;
	if (kh_dns(ad, &ip, ms))
		return -1;
	if ((k->fd = kh_tcp(ip, port, ms)) < 0)
		return -1;
	br_ssl_client_init_full(&k->sc, &k->xc, TAs, TAs_NUM);
	br_ssl_engine_set_buffer(&k->sc.eng, k->tampon, sizeof k->tampon, 1);
	if (!br_ssl_client_reset(&k->sc, ad, 0)) {
		kh_hata_yaz("tls: reset hata %d", br_ssl_engine_last_error(&k->sc.eng));
		close(k->fd);
		k->fd = -1;
		return -1;
	}
	br_sslio_init(&k->io, &k->sc.eng, kh_dusuk_oku, k, kh_dusuk_yaz, k);
	return 0;
}

static void kh_tls_hatasi(struct kh *k, const char *nerede)
{
	int e = br_ssl_engine_last_error(&k->sc.eng);
	kh_hata_yaz("%s: tls hata %d%s", nerede, e, k->zaman_asimi ? " (zaman asimi)" : "");
}

/* tek HTTP/1.1 istegi; ek: "Ad: deger\r\n" satirlari ya da NULL. 0 tamam. */
static int kh_istek(struct kh *k, const char *yontem, const char *ad, const char *yol, const char *ek,
		    const void *govde, size_t n)
{
	char b[2048];
	int l = snprintf(b, sizeof b, "%s %s HTTP/1.1\r\nHost: %s\r\nUser-Agent: kole-https/1\r\nConnection: close\r\n%s",
			 yontem, yol, ad, ek ? ek : "");
	if (l < 0 || (size_t)l >= sizeof b - 40) {
		kh_hata_yaz("istek: baslik cok uzun");
		return -1;
	}
	if (govde)
		l += snprintf(b + l, sizeof b - (size_t)l, "Content-Length: %zu\r\n", n);
	l += snprintf(b + l, sizeof b - (size_t)l, "\r\n");
	if (br_sslio_write_all(&k->io, b, (size_t)l) || (govde && n && br_sslio_write_all(&k->io, govde, n)) ||
	    br_sslio_flush(&k->io)) {
		kh_tls_hatasi(k, "istek");
		return -1;
	}
	return 0;
}

static void kh_kapat(struct kh *k)	/* close_notify beklenmez (Connection: close; cevap uzunlugu zaten denetli) */
{
	if (k->fd >= 0) {
		br_ssl_engine_close(&k->sc.eng);
		close(k->fd);
	}
	k->fd = -1;
}

/* ---- HTTP cevabi (okuyucudan bagimsiz: host testi bellekten okur) ---- */
typedef int (*kh_okuyucu)(void *c, unsigned char *b, size_t n);	/* >0 bayt; <=0 son/hata */

struct kh_akis {
	kh_okuyucu f;
	void *c;
	unsigned char t[2048];
	size_t bas, son;
	int bitti;
};

static int kh_bayt(struct kh_akis *a)
{
	if (a->bas == a->son) {
		if (a->bitti)
			return -1;
		int r = a->f(a->c, a->t, sizeof a->t);
		if (r <= 0) {
			a->bitti = 1;
			return -1;
		}
		a->bas = 0;
		a->son = (size_t)r;
	}
	return a->t[a->bas++];
}

static int kh_satir(struct kh_akis *a, char *s, size_t n)	/* CRLF'siz satir uzunlugu; -1 hata */
{
	size_t k = 0;
	for (;;) {
		int c = kh_bayt(a);
		if (c < 0)
			return -1;
		if (c == '\n')
			break;
		if (k + 1 >= n)
			return -1;
		s[k++] = (char)c;
	}
	if (k && s[k - 1] == '\r')
		k--;
	s[k] = 0;
	return (int)k;
}

/* HTTP durum kodu; -1 bicim/baglanti hatasi, -2 govde tampona sigmadi. govde NUL ile biter. */
static int kh_http_cevap(kh_okuyucu f, void *c, char *govde, size_t max, size_t *uz)
{
	struct kh_akis a = { .f = f, .c = c };
	char s[1024];
	int kod, l;
	*uz = 0;
	if (max == 0 || kh_satir(&a, s, sizeof s) < 0 || sscanf(s, "HTTP/1.%*d %d", &kod) != 1)
		return -1;
	long cl = -1;
	int parca = 0;
	while ((l = kh_satir(&a, s, sizeof s)) > 0) {
		if (!strncasecmp(s, "content-length:", 15))
			cl = strtol(s + 15, NULL, 10);
		else if (!strncasecmp(s, "transfer-encoding:", 18) && strcasestr(s + 18, "chunked"))
			parca = 1;
	}
	if (l < 0)
		return -1;
	size_t k = 0;
	if (parca) {
		for (;;) {
			char *e;
			if (kh_satir(&a, s, sizeof s) < 0)
				return -1;
			unsigned long n = strtoul(s, &e, 16);
			if (e == s)
				return -1;
			if (n == 0) {
				while ((l = kh_satir(&a, s, sizeof s)) > 0)
					;
				break;
			}
			for (unsigned long i = 0; i < n; i++) {
				int b = kh_bayt(&a);
				if (b < 0)
					return -1;
				if (k + 1 >= max)
					return -2;
				govde[k++] = (char)b;
			}
			if (kh_satir(&a, s, sizeof s) != 0)
				return -1;
		}
	} else if (cl >= 0) {
		if ((size_t)cl + 1 > max)
			return -2;
		for (long i = 0; i < cl; i++) {
			int b = kh_bayt(&a);
			if (b < 0)
				return -1;
			govde[k++] = (char)b;
		}
	} else {
		int b;
		while ((b = kh_bayt(&a)) >= 0) {
			if (k + 1 >= max)
				return -2;
			govde[k++] = (char)b;
		}
	}
	govde[k] = 0;
	*uz = k;
	return kod;
}

static int kh_tls_oku(void *c, unsigned char *b, size_t n)
{
	struct kh *k = c;
	return br_sslio_read(&k->io, b, n);
}

/* ac + istek + cevap + kapat. HTTP kodu ya da <0 (kh_hata). */
static int kh_tek(struct kh *k, const char *yontem, const char *ad, const char *yol, const char *ek, const void *ig,
		  size_t in, char *govde, size_t max, size_t *uz, int ms)
{
	*uz = 0;
	if (kh_ac(k, ad, 443, ms))
		return -1;
	int kod = -1;
	if (!kh_istek(k, yontem, ad, yol, ek, ig, in)) {
		kod = kh_http_cevap(kh_tls_oku, k, govde, max, uz);
		if (kod == -1)
			kh_tls_hatasi(k, "cevap");
		else if (kod == -2)
			kh_hata_yaz("cevap: govde %zu bayt tampona sigmadi", max);
	}
	kh_kapat(k);
	return kod;
}
#endif
