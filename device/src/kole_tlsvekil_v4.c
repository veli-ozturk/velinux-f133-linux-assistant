/* kole-tlsvekil v2 (25.10.2026, Claude; kullanıcı: "443'u de BearSSL vekiline cevir; vekil Merkez'e guvenli sekilde https ve
 * istemci bilgisini iletsin"). v1 + Tailscale serve --tcp --proxy-protocol 1: TLS'ten once PROXY v1 satiri okunur (yoksa
 * baglanti reddedilir); kaynak adres Tailscale yerel API whois ile kullanici adina cevrilir (1 sa onbellek). Hedef Merkez'in
 * vekil dinleyicisi (127.0.0.2: X-Forwarded-* basliklarina guvenir); bu yuzden istemcinin gonderdigi tum vekil basliklari
 * (X-Forwarded-*, Forwarded, X-Real-IP, Tailscale-User-*) her istekte silinir ve dogrulari eklenir:
 * X-Forwarded-For <PROXY kaynagi>, X-Forwarded-Proto https, Tailscale-User-Login <whois>. Keep-alive: govde Content-Length ile
 * sayilir; chunked istek govdesi ve katlanmis baslik reddedilir; Upgrade sonrasi ham aktarim.
 *   kole-tlsvekil-v4 <dinle_port> <hedef_ip> <hedef_port> <sertifika.pem> <anahtar.pem>
 * v3 (25.10.2026): telefondan giris "Merkeze ulasilamadi" verdi, v2 sessizdi -> baglanti basina tek satir stderr gunlugu
 *   (zaman, kaynak IP, kullanici var/yok, istek sayisi, BearSSL son hata, kapanma nedeni; baslik degeri yazilmaz).
 * v4 (26.10.2026): Merkez bosta keep-alive'i 30 sn'de kapatir; v3 istemci baglantisini da kapatiyordu (telefon POST'u
 *   kapanmis baglantiya gidebilir). Hedef, istek arasinda ve >=5 sn sessizken kapanirsa istemci acik kalir; sonraki
 *   istekte Merkez'e yeniden baglanilir. Bosta zaman asimi 30 -> 90 sn, en cok cocuk 8 -> 12.
 * Dinleme yalniz 127.0.0.1 (yalniz yerel tailscaled baglanir; PROXY satirina bu yuzden guvenilir). Anahtar icerigi yazilmaz. */
#define _GNU_SOURCE
#include "bearssl.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define AZAMI_ZINCIR 4
#define AZAMI_COCUK 12

static br_x509_certificate zincir[AZAMI_ZINCIR];
static size_t zincir_n;
static br_ec_private_key ec_anahtar;
static br_rsa_private_key rsa_anahtar;
static int anahtar_tur;	/* BR_KEYTYPE_EC / BR_KEYTYPE_RSA */
static unsigned char anahtar_kopya[4096];	/* skey cozucusunun gosterdigi baytlarin kalici kopyasi */

static unsigned char *dosya_oku(const char *yol, size_t *n)
{
	FILE *f = fopen(yol, "rb");
	if (!f) return NULL;
	unsigned char *b = malloc(65536);
	*n = b ? fread(b, 1, 65535, f) : 0;
	fclose(f);
	return b;
}

struct pem_nesne { char ad[128]; unsigned char *veri; size_t n, kap; };

static void pem_ekle(void *ctx, const void *b, size_t n)
{
	struct pem_nesne *o = ctx;
	if (o->n + n > o->kap) {
		size_t k = (o->n + n) * 2;
		unsigned char *y = realloc(o->veri, k);
		if (!y) return;
		o->veri = y;
		o->kap = k;
	}
	memcpy(o->veri + o->n, b, n);
	o->n += n;
}

/* PEM dosyasindaki nesneleri sirayla cagir: (ad, DER, uzunluk). 0 tamam */
static int pem_coz(const unsigned char *b, size_t n, void (*fn)(const char *, unsigned char *, size_t))
{
	br_pem_decoder_context pc;
	br_pem_decoder_init(&pc);
	struct pem_nesne o = { "", NULL, 0, 0 };
	while (n) {
		size_t k = br_pem_decoder_push(&pc, b, n);
		b += k;
		n -= k;
		switch (br_pem_decoder_event(&pc)) {
		case BR_PEM_BEGIN_OBJ:
			snprintf(o.ad, sizeof o.ad, "%s", br_pem_decoder_name(&pc));
			o.n = 0;
			br_pem_decoder_setdest(&pc, pem_ekle, &o);
			break;
		case BR_PEM_END_OBJ:
			if (o.n) {
				unsigned char *k2 = malloc(o.n);
				if (!k2) return -1;
				memcpy(k2, o.veri, o.n);
				fn(o.ad, k2, o.n);
			}
			break;
		case BR_PEM_ERROR:
			if (o.veri) memset(o.veri, 0, o.kap);
			free(o.veri);
			return -1;
		}
	}
	if (o.veri) memset(o.veri, 0, o.kap);	/* anahtar DER'i ara tamponda kalmasin */
	free(o.veri);
	return 0;
}

static void sertifika_al(const char *ad, unsigned char *d, size_t n)
{
	if (!strcmp(ad, "CERTIFICATE") && zincir_n < AZAMI_ZINCIR) {
		zincir[zincir_n].data = d;
		zincir[zincir_n].data_len = n;
		zincir_n++;
	} else free(d);
}

static void anahtar_al(const char *ad, unsigned char *d, size_t n)
{
	if (anahtar_tur || (strcmp(ad, "EC PRIVATE KEY") && strcmp(ad, "PRIVATE KEY") && strcmp(ad, "RSA PRIVATE KEY"))) {
		free(d);
		return;
	}
	br_skey_decoder_context sk;
	br_skey_decoder_init(&sk);
	br_skey_decoder_push(&sk, d, n);
	int t = br_skey_decoder_key_type(&sk);
	if (t == BR_KEYTYPE_EC) {
		const br_ec_private_key *k = br_skey_decoder_get_ec(&sk);
		if (k->xlen <= sizeof anahtar_kopya) {
			memcpy(anahtar_kopya, k->x, k->xlen);
			ec_anahtar = (br_ec_private_key){ k->curve, anahtar_kopya, k->xlen };
			anahtar_tur = BR_KEYTYPE_EC;
		}
	} else if (t == BR_KEYTYPE_RSA) {
		const br_rsa_private_key *k = br_skey_decoder_get_rsa(&sk);
		size_t top = k->plen + k->qlen + k->dplen + k->dqlen + k->iqlen;
		if (top <= sizeof anahtar_kopya) {
			unsigned char *p = anahtar_kopya;
			rsa_anahtar.n_bitlen = k->n_bitlen;
			memcpy(p, k->p, k->plen); rsa_anahtar.p = p; rsa_anahtar.plen = k->plen; p += k->plen;
			memcpy(p, k->q, k->qlen); rsa_anahtar.q = p; rsa_anahtar.qlen = k->qlen; p += k->qlen;
			memcpy(p, k->dp, k->dplen); rsa_anahtar.dp = p; rsa_anahtar.dplen = k->dplen; p += k->dplen;
			memcpy(p, k->dq, k->dqlen); rsa_anahtar.dq = p; rsa_anahtar.dqlen = k->dqlen; p += k->dqlen;
			memcpy(p, k->iq, k->iqlen); rsa_anahtar.iq = p; rsa_anahtar.iqlen = k->iqlen;
			anahtar_tur = BR_KEYTYPE_RSA;
		}
	}
	memset(d, 0, n);	/* anahtar baytlari bellekte birakilmaz */
	free(d);
}

static int tam_yaz(int fd, const unsigned char *b, size_t n)
{
	while (n) {
		ssize_t w = write(fd, b, n);
		if (w < 0 && errno == EINTR) continue;
		if (w <= 0) return -1;
		b += w;
		n -= (size_t)w;
	}
	return 0;
}

/* ---------------------------------------------------------------- v2: PROXY v1, whois, HTTP basligi yeniden yazimi */
#include <sys/stat.h>
#include <sys/un.h>
#include <time.h>

static char kaynak_ip[64], kullanici[128];
static const char *neden = "-";	/* v3: baglanti sonu gunluk satiri */
static unsigned istek_n;	/* PROXY satirindan / whois'ten */

/* "PROXY TCP4 <src> <dst> <sport> <dport>\r\n" (en cok 107 bayt, TLS'ten once). 0 tamam (ip/port doldurulur) */
static int proxy_oku(int fd, char *ip, size_t in, unsigned *port)
{
	char s[110];
	size_t n = 0;
	struct pollfd p = { fd, POLLIN, 0 };
	while (n < sizeof s - 1) {
		if (poll(&p, 1, 5000) <= 0) return -1;
		ssize_t k = read(fd, s + n, 1);
		if (k <= 0) return -1;
		if (s[n++] == '\n') break;
	}
	s[n] = 0;
	if (n < 8 || strncmp(s, "PROXY ", 6) || s[n - 1] != '\n' || s[n - 2] != '\r') return -1;
	char ag[8], src[64], dst[64];
	unsigned sp = 0, dp = 0;
	if (sscanf(s, "PROXY %7s %63s %63s %u %u", ag, src, dst, &sp, &dp) != 5 || (strcmp(ag, "TCP4") && strcmp(ag, "TCP6"))) return -1;
	if (strspn(src, "0123456789abcdefABCDEF.:") != strlen(src) || sp > 65535) return -1;
	struct in6_addr t6;
	struct in_addr t4;
	if (inet_pton(!strcmp(ag, "TCP4") ? AF_INET : AF_INET6, src, !strcmp(ag, "TCP4") ? (void *)&t4 : (void *)&t6) != 1) return -1;
	snprintf(ip, in, "%s", src);
	*port = sp;
	return 0;
}

/* Tailscale yerel API whois: kullanici adi (LoginName). Onbellek /tmp/tlsvekil-whois/<ip> 1 saat. Basarisizsa bos. */
static void whois(const char *ip, unsigned port, char *o, size_t on)
{
	o[0] = 0;
	char y[160];
	snprintf(y, sizeof y, "/tmp/tlsvekil-whois/%s", ip);
	struct stat st;
	if (!stat(y, &st) && time(NULL) - st.st_mtime < 3600) {
		FILE *f = fopen(y, "r");
		if (f) {
			if (!fgets(o, (int)on, f)) o[0] = 0;
			fclose(f);
			o[strcspn(o, "\r\n")] = 0;
			return;
		}
	}
	int s = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
	struct sockaddr_un a = { .sun_family = AF_UNIX };
	snprintf(a.sun_path, sizeof a.sun_path, "/tmp/tailscaled.sock");
	struct timeval to = { 1, 500000 };
	if (s < 0) return;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);
	if (connect(s, (struct sockaddr *)&a, sizeof a)) { close(s); return; }
	char q[256];
	int qn = snprintf(q, sizeof q, "GET /localapi/v0/whois?addr=%s%s%s:%u HTTP/1.0\r\nHost: local-tailscaled.sock\r\n\r\n",
			  strchr(ip, ':') ? "[" : "", ip, strchr(ip, ':') ? "]" : "", port);
	static char b[16384];
	size_t n = 0;
	if (write(s, q, (size_t)qn) == qn) {
		ssize_t k;
		while (n < sizeof b - 1 && (k = read(s, b + n, sizeof b - 1 - n)) > 0) n += (size_t)k;
	}
	close(s);
	b[n] = 0;
	const char *u = strstr(b, "\"UserProfile\"");
	const char *l = u ? strstr(u, "\"LoginName\"") : NULL;
	if (!l) return;
	l += 11;
	l += strspn(l, " \t");
	if (*l++ != ':') return;
	l += strspn(l, " \t");
	if (*l++ != '"') return;
	size_t k = strcspn(l, "\"");
	if (!k || k >= on || strspn(l, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@._+-") < k) return;
	memcpy(o, l, k);
	o[k] = 0;
	mkdir("/tmp/tlsvekil-whois", 0700);
	char g[180];
	snprintf(g, sizeof g, "%s.%d", y, (int)getpid());
	FILE *f = fopen(g, "w");
	if (f) {
		fprintf(f, "%s\n", o);
		if (!fclose(f)) rename(g, y);
		else unlink(g);
	}
}

/* istemci -> Merkez filtresi: her istegin basliklari yeniden yazilir (taklit edilebilir vekil basliklari silinir, dogrulari
 * eklenir), govde Content-Length ile sayilir; Upgrade sonrasi ham. Chunked istek govdesi ve katlanmis baslik reddedilir. */
enum { F_BASLIK, F_GOVDE, F_HAM };
struct filtre { int mod; unsigned long long kalan; size_t hn; char hb[16384]; };

static int ad_esit(const char *s, size_t n, const char *ad)
{
	size_t m = strlen(ad);
	return n == m && !strncasecmp(s, ad, m);
}

static const char *const yasak[] = { "X-Forwarded-For", "X-Forwarded-Proto", "X-Forwarded-Host", "X-Forwarded-Port",
	"X-Forwarded-Server", "Forwarded", "X-Real-IP", "Tailscale-User-Login", "Tailscale-User-Name",
	"Tailscale-User-Profile-Pic", "Tailscale-Headers-Info", NULL };

/* tam baslik blogu hb[0..bn) (\r\n\r\n dahil) -> bfd'ye yeniden yazilmis olarak. 0 tamam, -1 reddet */
static int baslik_yaz(struct filtre *f, size_t bn, int bfd)
{
	static char o[20480];
	size_t on = 0;
	char *s = f->hb, *son = f->hb + bn - 2;	/* son bos satirin \r\n'si haric */
	int ilk = 1, upgrade = 0;
	unsigned long long cl = 0;
	int cl_var = 0;
	while (s < son) {
		char *e = memchr(s, '\n', (size_t)(son - s));
		if (!e) { neden = "satir"; return -1; }
		size_t sn = (size_t)(e - s) + 1;	/* satir \n dahil */
		if (sn < 2 || s[sn - 2] != '\r') { neden = "CR yok"; return -1; }
		if (!ilk) {
			if (s[0] == ' ' || s[0] == '\t') { neden = "katlanmis"; return -1; }
			char *ik = memchr(s, ':', sn);
			if (!ik) { neden = "iki nokta yok"; return -1; }
			size_t an = (size_t)(ik - s);
			int atla = 0;
			for (int i = 0; yasak[i]; i++) if (ad_esit(s, an, yasak[i])) atla = 1;
			char *v = ik + 1;
			while (*v == ' ' || *v == '\t') v++;
			size_t vn = (size_t)(s + sn - 2 - v);
			if (ad_esit(s, an, "Content-Length")) {
				if (!vn || vn > 15 || strspn(v, "0123456789") < vn || cl_var) { neden = "Content-Length"; return -1; }
				cl = strtoull(v, NULL, 10);
				cl_var = 1;
			}
			if (ad_esit(s, an, "Transfer-Encoding")) { neden = "Transfer-Encoding"; return -1; }	/* chunked istek govdesi desteklenmez */
			if (ad_esit(s, an, "Upgrade")) upgrade = 1;
			if (atla) { s += sn; continue; }
		}
		ilk = 0;
		if (on + sn > sizeof o - 512) { neden = "baslik buyuk"; return -1; }
		memcpy(o + on, s, sn);
		on += sn;
		s += sn;
	}
	int k = snprintf(o + on, sizeof o - on, "X-Forwarded-For: %s\r\nX-Forwarded-Proto: https\r\n%s%s%s\r\n", kaynak_ip,
			 kullanici[0] ? "Tailscale-User-Login: " : "", kullanici, kullanici[0] ? "\r\n" : "");
	if (k <= 0 || (size_t)k >= sizeof o - on) return -1;
	on += (size_t)k;
	if (tam_yaz(bfd, (unsigned char *)o, on)) return -1;
	istek_n++;
	f->mod = upgrade ? F_HAM : cl ? F_GOVDE : F_BASLIK;
	f->kalan = cl;
	return 0;
}

/* cozulmus istemci baytlari; 0 tamam, -1 baglantiyi kapat */
static int filtre_besle(struct filtre *f, const unsigned char *b, size_t n, int bfd)
{
	while (n) {
		if (f->mod == F_HAM) return tam_yaz(bfd, b, n);
		if (f->mod == F_GOVDE) {
			size_t k = n < f->kalan ? n : (size_t)f->kalan;
			if (tam_yaz(bfd, b, k)) return -1;
			b += k;
			n -= k;
			f->kalan -= k;
			if (!f->kalan) f->mod = F_BASLIK;
			continue;
		}
		size_t k = sizeof f->hb - f->hn < n ? sizeof f->hb - f->hn : n;
		memcpy(f->hb + f->hn, b, k);
		size_t eski = f->hn;
		f->hn += k;
		char *e = memmem(f->hb, f->hn, "\r\n\r\n", 4);
		if (!e) {
			if (f->hn == sizeof f->hb) { neden = "16 KB"; return -1; }	/* 16 KB'ta baslik bitmedi */
			b += k;
			n -= k;
			continue;
		}
		size_t bn = (size_t)(e - f->hb) + 4;
		if (baslik_yaz(f, bn, bfd)) return -1;
		size_t tuk = bn - eski;	/* bu parcadan baslik icin tuketilen */
		f->hn = 0;
		b += tuk;
		n -= tuk;
	}
	return 0;
}

/* bir istemci: PROXY satiri, TLS motoru <-> Merkez (vekil dinleyicisi), poll ile; 30 sn bosta kapanir */
/* v4: Merkez'e baglan; -1 basarisiz */
static int hedef_bag(const char *hip, int hport)
{
	int bfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0), bir = 1;
	struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons((uint16_t)hport) };
	if (bfd < 0 || inet_pton(AF_INET, hip, &a.sin_addr) != 1 || connect(bfd, (struct sockaddr *)&a, sizeof a)) {
		if (bfd >= 0) close(bfd);
		return -1;
	}
	setsockopt(bfd, IPPROTO_TCP, TCP_NODELAY, &bir, sizeof bir);
	return bfd;
}

static void hizmet(int cfd, const char *hip, int hport)
{
	unsigned sport = 0;
	if (proxy_oku(cfd, kaynak_ip, sizeof kaynak_ip, &sport)) { fprintf(stderr, "%ld - PROXY yok/gecersiz\n", (long)time(NULL)); return; }	/* PROXY yoksa istemci bilgisi yok: reddet */
	whois(kaynak_ip, sport, kullanici, sizeof kullanici);
	static br_ssl_server_context sc;
	static unsigned char iobuf[BR_SSL_BUFSIZE_BIDI];
	static struct filtre f;
	f.mod = F_BASLIK;
	f.hn = 0;
	if (anahtar_tur == BR_KEYTYPE_EC)
		br_ssl_server_init_full_ec(&sc, zincir, zincir_n, BR_KEYTYPE_EC, &ec_anahtar);
	else
		br_ssl_server_init_full_rsa(&sc, zincir, zincir_n, &rsa_anahtar);
	br_ssl_engine_set_buffer(&sc.eng, iobuf, sizeof iobuf, 1);
	if (!br_ssl_server_reset(&sc)) return;
	int bfd = hedef_bag(hip, hport);
	if (bfd < 0) { fprintf(stderr, "%ld %s hedef baglanamadi\n", (long)time(NULL), kaynak_ip); return; }
	int bir = 1;
	setsockopt(cfd, IPPROTO_TCP, TCP_NODELAY, &bir, sizeof bir);
	time_t son_b = time(NULL), son_c = son_b;	/* v4: hedeften / istemciden son veri */
	unsigned yeniden = 0;
	br_ssl_engine_context *e = &sc.eng;
	int hedef_kapali = 0;
	for (;;) {
		unsigned st = br_ssl_engine_current_state(e);
		if (st & BR_SSL_CLOSED) { if (*neden == '-') neden = "tls kapandi"; break; }
		size_t n;
		if (st & BR_SSL_SENDREC) {
			unsigned char *b = br_ssl_engine_sendrec_buf(e, &n);
			if (tam_yaz(cfd, b, n)) { neden = "istemci yazma"; break; }
			br_ssl_engine_sendrec_ack(e, n);
			continue;
		}
		if (st & BR_SSL_RECVAPP) {
			unsigned char *b = br_ssl_engine_recvapp_buf(e, &n);
			if (bfd < 0) {	/* v4: Merkez bostayken kapatmisti */
				if ((bfd = hedef_bag(hip, hport)) < 0) { neden = "hedef yeniden baglanamadi"; break; }
				yeniden++;
			}
			son_c = time(NULL);
			if (filtre_besle(&f, b, n, bfd)) { if (*neden == '-') neden = "hedef yazma"; break; }	/* v2: basliklar yeniden yazilir */
			br_ssl_engine_recvapp_ack(e, n);
			continue;
		}
		struct pollfd p[2] = { { cfd, (st & BR_SSL_RECVREC) ? POLLIN : 0, 0 },
				       { bfd, (st & BR_SSL_SENDAPP) && !hedef_kapali ? POLLIN : 0, 0 } };
		int r = poll(p, 2, 90000);
		if (r == 0) { neden = "90 sn bosta"; break; }
		if (r < 0) { if (errno == EINTR) continue; break; }
		if (p[0].revents & (POLLIN | POLLHUP | POLLERR)) {
			unsigned char *b = br_ssl_engine_recvrec_buf(e, &n);
			ssize_t k = read(cfd, b, n);
			if (k <= 0) { neden = "istemci kapatti"; break; }
			br_ssl_engine_recvrec_ack(e, (size_t)k);
		}
		if (p[1].revents & (POLLIN | POLLHUP | POLLERR)) {
			unsigned char *b = br_ssl_engine_sendapp_buf(e, &n);
			ssize_t k = b ? read(bfd, b, n) : -1;
			time_t t = time(NULL);
			if (k <= 0 && f.mod == F_BASLIK && !f.hn && t - son_b >= 5 && t - son_c >= 5) {
				close(bfd);	/* v4: istek arasi bosta kapanma: istemci acik kalir, sonraki istekte yeniden baglan */
				bfd = -1;
			} else if (k <= 0) {
				hedef_kapali = 1;
				br_ssl_engine_close(e);	/* close_notify; kalan kayitlar gonderilip kapanir */
			} else {
				son_b = t;
				br_ssl_engine_sendapp_ack(e, (size_t)k);
				br_ssl_engine_flush(e, 0);
			}
		}
	}
	if (bfd >= 0) close(bfd);
	fprintf(stderr, "%ld %s %s istek %u yeniden %u tls_hata %d neden %s\n", (long)time(NULL), kaynak_ip, kullanici[0] ? "k+" : "k-",
		istek_n, yeniden, br_ssl_engine_last_error(e), neden);
}

int main(int argc, char **argv)
{
	if (argc != 6) {
		fprintf(stderr, "kullanim: kole-tlsvekil-v4 <dinle_port> <hedef_ip> <hedef_port> <sertifika.pem> <anahtar.pem>\n");
		return 2;
	}
	int lport = atoi(argv[1]), hport = atoi(argv[3]);
	const char *hip = argv[2];
	size_t n;
	unsigned char *b = dosya_oku(argv[4], &n);
	if (!b || pem_coz(b, n, sertifika_al) || !zincir_n) { fprintf(stderr, "sertifika okunamadi\n"); return 3; }
	free(b);
	b = dosya_oku(argv[5], &n);
	if (!b || pem_coz(b, n, anahtar_al) || !anahtar_tur) { fprintf(stderr, "anahtar okunamadi (EC/RSA)\n"); return 3; }
	memset(b, 0, n);
	free(b);
	signal(SIGPIPE, SIG_IGN);
	struct sigaction sa = { .sa_handler = SIG_DFL, .sa_flags = SA_NOCLDWAIT };
	sigaction(SIGCHLD, &sa, NULL);
	int l = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0), bir = 1;
	setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &bir, sizeof bir);
	struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons((uint16_t)lport), .sin_addr.s_addr = htonl(INADDR_LOOPBACK) };
	if (l < 0 || bind(l, (struct sockaddr *)&a, sizeof a) || listen(l, 16)) { perror("dinle"); return 4; }
	printf("kole-tlsvekil-v4: 127.0.0.1:%d (PROXY v1) -> %s:%d, zincir %zu, anahtar %s\n", lport, hip, hport, zincir_n,
	       anahtar_tur == BR_KEYTYPE_EC ? "EC" : "RSA");
	fflush(stdout);
	static pid_t cocuk[AZAMI_COCUK];
	for (;;) {
		int c = accept4(l, NULL, NULL, SOCK_CLOEXEC);
		if (c < 0) { if (errno == EINTR) continue; perror("accept"); continue; }
		int bos = -1;
		for (int i = 0; i < AZAMI_COCUK; i++)
			if (!cocuk[i] || kill(cocuk[i], 0)) { bos = i; break; }	/* SA_NOCLDWAIT: olen cocuk kendiliginden toplanir */
		if (bos < 0) { close(c); continue; }
		pid_t p = fork();
		if (p == 0) {
			close(l);
			hizmet(c, hip, hport);
			_exit(0);
		}
		if (p > 0) cocuk[bos] = p;
		close(c);
	}
}
