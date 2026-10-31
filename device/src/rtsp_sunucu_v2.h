/* rtsp_sunucu_v2.h (24.10.2026, Claude; scripts/624, denetim-2026-10-24-v3 A14, A18a-c): v1 +
 *   - hata halkasi IP + zaman; 10 dk'da 10 hata yalniz o IP'yi 10 dk kilitler (v1 herkesi kilitliyordu: eski parolali
 *     tek istemci dogru parolayi da reddettiriyordu). Dogru ozet + suresi gecmis nonce: 401 stale=TRUE, hata sayilmaz.
 *   - nonce MAC'inde istemci IP'si; Digest uri yolu istek yoluyla ayni olmali (yakalanan baslik baska yerde gecmez).
 *   - SIGKILL temizligi mik_cocuk_temizle (ses_akis_v4.h: /tmp/mik-aktif + "kopru <pid>" birak bayragi).
 *   - rtsp_sunucu basinda artik "rtsp" kamera bayragi ve /tmp/rtsp-aktif silinir; dongude dakikada bir
 *     RTSP_GUNLUK_KIRP() (kole_kopru: gunluk_kirp(1)). --- v1 ---
 */
/* rtsp_sunucu_v1.h (13.10.2026, Claude): kole-kopru v8 asgari RTSP sunucusu (IP kamera gibi: VLC, iOS RTSP
 * uygulamalari, ffmpeg). Tek surec, poll() dongusu; ileride cihaz-ici web merkezine gomulebilir (rtsp_adim).
 *   - VARSAYILAN KAPALI: /mnt/sd/velinux/rtsp-acik yoksa port bile dinlenmez. Gizlilik kalkani varsa kapanir.
 *   - Parola: /data/rtsp-parola (DEPODA YOK): "kullanici <HA1 hex>" (HA1 = MD5(kullanici:VeLinux-KOLE:parola))
 *     ya da "kullanici:parola" (en az 12 karakter). Dosya yoksa sunucu dinlemez. Yalniz Digest (Basic yok).
 *   - Yalniz yerel ag / Tailscale / CGNAT adresleri (10/8, 172.16/12, 192.168/16, 100.64/10, 127/8) kabul edilir.
 *   - Tasima yalniz RTP/AVP/TCP interleaved (RTSP-over-TCP; UDP istegi -> 461). En fazla 4 baglanti, 2 izleyici.
 *   - /kamera = RTP/JPEG (RFC 2435, PT 26) + PCMU 8 kHz (PT 0) ; /video = yalniz goruntu. RTCP SR 2 sn'de bir.
 *   - Kaynaklar AYRI cocuk sureclerde, var olan kod yollariyla: goruntu = kamera_akis() (yalniz READ(10) ya da
 *     kamera-kayit hizmeti), ses = mic_akis() (MIC_AKIS). Izleyici yokken ikisi de kapali.
 *   - Gizlilik: oynatma surerken /tmp/kamera-aktif (yoksa "rtsp" icerikle olusturulur, bitince yalniz bizimkiyse
 *     silinir), /tmp/mik-aktif (mic_akis), /tmp/rtsp-aktif (izleyici sayisi). Durum: /tmp/rtsp-durum.json.
 *   - Kaba kuvvet: hatali Digest 1 sn bekletilir, baglanti basina 3 hata -> kapanir, 10 dk'da 10 hata -> 10 dk kilit.
 * Goruntu/ses hicbir yere yazilmaz; yalniz bellekte paketlenip gonderilir. */
#ifndef RTSP_SUNUCU_V1_H
#define RTSP_SUNUCU_V1_H
#define RTSP_SUNUCU_V2_H	/* v1 yerine */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <stddef.h>
#include <signal.h>
#include <sys/wait.h>
#include "kopru_ortak_v1.h"
#include "ses_akis_v4.h"
#include "kamera_akis_v2.h"
#include "rtsp_v1.h"

static const char *rtsp_bayrak = "/mnt/sd/velinux/rtsp-acik";
static const char *rtsp_parola_yol = "/data/rtsp-parola";
static const char *rtsp_durum_yol = "/tmp/rtsp-durum.json";
static const char *rtsp_aktif_yol = "/tmp/rtsp-aktif";
static int rtsp_her_zaman_acik = 0;	/* rtsp-test: bayrak dosyasi aranmaz */
static uint32_t rtsp_bag_ip = INADDR_ANY;	/* rtsp-test: yalniz 127.0.0.1 */

#define R_BAGLANTI 4
#define R_OYNATAN 2
#define R_FPS 5
#define R_CIKIS (512 * 1024)
#define R_KARE_SINIR (256 * 1024)
#define R_MTU 1400
#define R_VBUF (300000 + 8)
#define R_RET 32	/* v2: hata halkasi (IP + zaman) */
#define R_KILIT 8	/* v2: ayni anda kilitli IP */
#ifndef RTSP_GUNLUK_KIRP
#define RTSP_GUNLUK_KIRP() ((void)0)	/* v2: gomen dosya verir (kole_kopru: gunluk_kirp(1)) */
#endif

struct istemci {
	int fd;
	uint32_t ip;
	char gelen[8192];
	size_t gn, atla;
	uint8_t cikis[R_CIKIS];
	size_t cb, cs;
	int yetkili, ret, kapat_sonra, oynuyor, vk, sk;
	double tut_t, son_istek, son_ilerleme, sr_t, bag_t;
	char oturum[20];
	uint16_t vseq, aseq;
	uint32_t vssrc, assrc, vpk, vokt, apk, aokt;
	unsigned atlanan;
};
static struct istemci R_ist[R_BAGLANTI];

static struct {
	int lfd, port, dinleme_yok, acik, parola_var, bizim_kamera, sbaslik, ses_canli, son_oynatan, ret_i, dht_uyari;
	char gizli[33], kul[64], ha1[33], vhata[96], shata[64], jpeg_uyum[96];
	double t0, kontrol_t, durum_t, ret_t[R_RET], kilit_t[R_KILIT], vyeniden_t, syeniden_t, sessiz_t, ses_son_t, ats_mono;
	uint32_t ret_ip[R_RET], kilit_ip[R_KILIT];	/* v2: hata ve kilit kaynak IP basina */
	pid_t vpid, spid, olen[8];
	double olen_t[8];
	int vfd, sfd;
	uint8_t vbuf[R_VBUF];
	size_t vn, sn_;
	uint8_t sbuf[8192];
	struct az8 az;
	uint8_t ubuf[160];
	size_t un;
	uint32_t ats, vbase;
	unsigned long kareler, red, baglanti;
	volatile sig_atomic_t dur;
} R;

static uint32_t rastgele32(void)
{
	uint32_t v = 0;
	int u = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	if (u < 0 || read(u, &v, 4) != 4) v = (uint32_t)(mono() * 1e6) ^ (uint32_t)getpid() * 2654435761u;
	if (u >= 0) close(u);
	return v;
}

static int ozel_ip(uint32_t ip)
{
	return (ip >> 24) == 10 || (ip >> 24) == 127 || (ip >> 20) == 0xAC1 || (ip >> 16) == 0xC0A8 || (ip >> 22) == 0x191;
}

static void ip_metin(uint32_t ip, char *o, size_t n)
{
	snprintf(o, n, "%u.%u.%u.%u", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
}

static int kullanici_gecerli(const char *k)
{
	size_t n = strlen(k);
	return n >= 1 && n <= 32 && strspn(k, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.-") == n;
}

/* /data/rtsp-parola -> R.kul, R.ha1 */
static int parola_yukle(void)
{
	char b[256];
	R.parola_var = 0;
	if (dosya_oku(rtsp_parola_yol, b, sizeof b) <= 0) return 0;
	b[strcspn(b, "\r\n")] = 0;
	char *sp = strchr(b, ' '), *ik = strchr(b, ':');
	if (sp && (!ik || sp < ik)) {
		*sp = 0;
		char *h = sp + 1;
		while (*h == ' ') h++;
		h[strcspn(h, " \t")] = 0;
		if (!kullanici_gecerli(b) || !hex32_mi(h)) return 0;
		snprintf(R.kul, sizeof R.kul, "%s", b);
		for (int i = 0; i < 32; i++) R.ha1[i] = (char)tolower((unsigned char)h[i]);
		R.ha1[32] = 0;
	} else if (ik) {
		*ik = 0;
		if (!kullanici_gecerli(b) || strlen(ik + 1) < 12) return 0;
		const char *p[] = { b, RTSP_REALM, ik + 1, NULL };
		md5_hex_parcalar(R.ha1, p);
		snprintf(R.kul, sizeof R.kul, "%s", b);
	} else {
		return 0;
	}
	memset(b, 0, sizeof b);
	R.parola_var = 1;
	return 1;
}

/* v2: nonce = zaman (8 hex) + MD5(gizli, zaman, istemci IP)[:24]: yakalanan Digest basligi baska IP'den gecmez */
static void nonce_yap(char o[33], uint32_t t, uint32_t ip)
{
	char ts[9], is[9], h[33];
	snprintf(ts, sizeof ts, "%08x", t);
	snprintf(is, sizeof is, "%08x", ip);
	const char *p[] = { R.gizli, ts, is, NULL };
	md5_hex_parcalar(h, p);
	snprintf(o, 33, "%s%.24s", ts, h);
}

static void nonce_uret(char o[33], uint32_t ip) { nonce_yap(o, (uint32_t)mono(), ip); }

/* v2: 1 gecerli, 2 bizim ama suresi gecmis (stale), 0 gecersiz */
static int nonce_gecerli(const char *n, uint32_t ip)
{
	if (strlen(n) != 32) return 0;
	char t[9], bek[33];
	memcpy(t, n, 8);
	t[8] = 0;
	unsigned long v = strtoul(t, NULL, 16);
	nonce_yap(bek, (uint32_t)v, ip);
	if (!ct_esit(n, bek)) return 0;
	double yas = mono() - (double)v;
	return yas < -1 ? 0 : yas <= 300 ? 1 : 2;
}

static size_t cikis_bos(const struct istemci *c) { return c->cs - c->cb; }

static int cikis_yer(struct istemci *c, size_t n)
{
	if (c->cs + n > R_CIKIS && c->cb) {
		memmove(c->cikis, c->cikis + c->cb, c->cs - c->cb);
		c->cs -= c->cb;
		c->cb = 0;
	}
	return c->cs + n <= R_CIKIS;
}

static int cikis_ekle(struct istemci *c, const void *v, size_t n)
{
	if (!cikis_yer(c, n)) return -1;
	memcpy(c->cikis + c->cs, v, n);
	c->cs += n;
	return 0;
}

static int cikis_kanal(struct istemci *c, int kanal, const uint8_t *v, size_t n)
{
	if (n > 65535 || !cikis_yer(c, n + 4)) return -1;
	uint8_t h[4] = { '$', (uint8_t)kanal, (uint8_t)(n >> 8), (uint8_t)n };
	cikis_ekle(c, h, 4);
	return cikis_ekle(c, v, n);
}

static void cevap(struct istemci *c, int kod, const char *metin, int cseq, const char *ek, const char *govde)
{
	char b[3072];
	size_t gn = govde ? strlen(govde) : 0;
	int n = snprintf(b, sizeof b, "RTSP/1.0 %d %s\r\nCSeq: %d\r\nServer: VeLinux-KOLE/v8\r\n%s", kod, metin, cseq,
			 ek ? ek : "");
	if (gn) n += snprintf(b + n, sizeof b - (size_t)n, "Content-Length: %zu\r\n", gn);
	n += snprintf(b + n, sizeof b - (size_t)n, "\r\n");
	if (n >= (int)sizeof b) n = (int)sizeof b - 1;
	cikis_ekle(c, b, (size_t)n);
	if (gn) cikis_ekle(c, govde, gn);
}

static int oynatan_sayi(void)
{
	int n = 0;
	for (int i = 0; i < R_BAGLANTI; i++) n += R_ist[i].fd >= 0 && R_ist[i].oynuyor;
	return n;
}

static void istemci_kapat(struct istemci *c, const char *neden)
{
	if (c->fd < 0) return;
	char ip[20];
	ip_metin(c->ip, ip, sizeof ip);
	printf("RTSP %s kapandi (%s, %.0f sn%s)\n", ip, neden, mono() - c->bag_t, c->oynuyor ? ", izliyordu" : "");
	fflush(stdout);
	close(c->fd);
	c->fd = -1;
	c->oynuyor = 0;
	c->gn = c->atla = c->cb = c->cs = 0;
}

/* v2: kilit IP basina */
static int ip_kilitli(uint32_t ip, double t)
{
	for (int i = 0; i < R_KILIT; i++) if (R.kilit_ip[i] == ip && t < R.kilit_t[i]) return 1;
	return 0;
}

static int kilit_var(double t)
{
	for (int i = 0; i < R_KILIT; i++) if (t < R.kilit_t[i]) return 1;
	return 0;
}

/* v2: Digest uri'sinin yolu istek yoluyla ayni mi (sondaki '/' yok sayilir) */
static int uri_uyar(const char *uri, const char *url)
{
	char a[256], b[256];
	url_yol(uri, a, sizeof a);
	url_yol(url, b, sizeof b);
	size_t na = strlen(a), nb = strlen(b);
	if (na > 1 && a[na - 1] == '/') a[na - 1] = 0;
	if (nb > 1 && b[nb - 1] == '/') b[nb - 1] = 0;
	return !strcmp(a, b);
}

/* -2: dogru ozet ama nonce suresi gecmis (stale; hata sayilmaz), -1: deneme yok (yalniz meydan okuma), 0: yanlis,
 * 1: dogru. v2: kilit ve hata sayaci kaynak IP basina, nonce IP'ye bagli, uri yolu istekle ayni */
static int yetki_denetle(struct istemci *c, const struct rtsp_istek *r)
{
	if (c->yetkili) return 1;
	if (!r->yetki[0]) return -1;
	double t = mono();
	struct ozet o;
	int ozet_ok = !ip_kilitli(c->ip, t) && R.parola_var && !ozet_ayristir(r->yetki, &o) &&
		      ozet_dogrula(&o, r->yontem, R.kul, R.ha1) && uri_uyar(o.uri, r->url);
	int ng = ozet_ok ? nonce_gecerli(o.nonce, c->ip) : 0;
	char ip[20];
	ip_metin(c->ip, ip, sizeof ip);
	if (ng == 1) {
		c->yetkili = 1;
		printf("RTSP %s giris OK\n", ip);
		return 1;
	}
	if (ng == 2) {
		printf("RTSP %s nonce suresi doldu (stale=TRUE)\n", ip);
		return -2;
	}
	R.red++;
	c->ret++;
	R.ret_t[R.ret_i] = t;
	R.ret_ip[R.ret_i] = c->ip;
	R.ret_i = (R.ret_i + 1) % R_RET;
	int son10 = 0;
	for (int i = 0; i < R_RET; i++) son10 += R.ret_ip[i] == c->ip && R.ret_t[i] > 0 && t - R.ret_t[i] < 600;
	if (son10 >= 10 && !ip_kilitli(c->ip, t)) {
		int k = 0;
		for (int i = 1; i < R_KILIT; i++) if (R.kilit_t[i] < R.kilit_t[k]) k = i;	/* bos / en eski yuva */
		R.kilit_ip[k] = c->ip;
		R.kilit_t[k] = t + 600;
		printf("RTSP KILIT %s: 10 dk icinde 10 hatali giris, bu IP 10 dk giremez\n", ip);
	}
	printf("RTSP %s giris REDDEDILDI (%d)\n", ip, c->ret);
	return 0;
}

static int yol_on(const char *yol, const char *on)
{
	size_t n = strlen(on);
	return !strncmp(yol, on, n) && (yol[n] == 0 || yol[n] == '/');
}

static uint32_t video_ts(double t) { return R.vbase + (uint32_t)(int64_t)((t - R.t0) * 90000.0); }
static uint32_t ses_ts(double t)
{
	return R.ats_mono > 0 ? R.ats + (uint32_t)(int64_t)((t - R.ats_mono) * 8000.0) : R.ats;
}

static void istek_isle(struct istemci *c, const struct rtsp_istek *r)
{
	char yol[256], ek[1024], govde[1024];
	url_yol(r->url, yol, sizeof yol);
	c->son_istek = mono();
	if (!strcmp(r->yontem, "OPTIONS")) {
		cevap(c, 200, "OK", r->cseq,
		      "Public: OPTIONS, DESCRIBE, SETUP, PLAY, PAUSE, TEARDOWN, GET_PARAMETER, SET_PARAMETER\r\n", NULL);
		return;
	}
	int y = yetki_denetle(c, r);
	if (y <= 0) {
		char n[33];
		nonce_uret(n, c->ip);
		snprintf(ek, sizeof ek, "WWW-Authenticate: Digest realm=\"" RTSP_REALM "\", nonce=\"%s\", algorithm=MD5%s\r\n", n,
			 y == -2 ? ", stale=TRUE" : "");	/* v2 */
		cevap(c, 401, "Unauthorized", r->cseq, ek, NULL);
		if (!y) {
			c->tut_t = mono() + 1.0;	/* hatali deneme: yanit 1 sn gecikir */
			if (c->ret >= 3) c->kapat_sonra = 1;
		}
		return;
	}
	int kamera = yol_on(yol, "/kamera") || !strncmp(yol, "/kamera/", 8);
	int video = yol_on(yol, "/video") || !strncmp(yol, "/video/", 7);
	if (!kamera && !video) { cevap(c, 404, "Not Found", r->cseq, NULL, NULL); return; }
	if (r->oturum[0] && (!c->oturum[0] || strcmp(r->oturum, c->oturum))) {
		cevap(c, 454, "Session Not Found", r->cseq, NULL, NULL);
		return;
	}
	char oturum_h[64] = "";
	if (c->oturum[0]) snprintf(oturum_h, sizeof oturum_h, "Session: %s;timeout=60\r\n", c->oturum);
	char taban[300];
	snprintf(taban, sizeof taban, "%s%s", r->url, r->url[0] && r->url[strlen(r->url) - 1] == '/' ? "" : "/");
	if (!strcmp(r->yontem, "DESCRIBE")) {
		snprintf(govde, sizeof govde,
			 "v=0\r\no=- %u 1 IN IP4 0.0.0.0\r\ns=VeLinux KOLE kamera\r\nc=IN IP4 0.0.0.0\r\nt=0 0\r\n"
			 "a=control:*\r\na=range:npt=0-\r\nm=video 0 RTP/AVP 26\r\na=control:trackID=0\r\na=framerate:%d\r\n%s",
			 R.vbase, R_FPS, kamera ? "m=audio 0 RTP/AVP 0\r\na=rtpmap:0 PCMU/8000\r\na=control:trackID=1\r\n" : "");
		snprintf(ek, sizeof ek, "Content-Base: %s\r\nContent-Type: application/sdp\r\n", taban);
		cevap(c, 200, "OK", r->cseq, ek, govde);
	} else if (!strcmp(r->yontem, "SETUP")) {
		int ses = strstr(yol, "trackID=1") != NULL;
		if (ses && !kamera) { cevap(c, 404, "Not Found", r->cseq, NULL, NULL); return; }
		int a, b;
		if (!tasima_tcp(r->tasima, &a, &b)) {
			cevap(c, 461, "Unsupported Transport", r->cseq, NULL, NULL);
			return;
		}
		if (a < 0) a = ses ? 2 : 0;
		if ((ses && c->vk == a) || (!ses && c->sk == a)) a = ses ? 2 : 0;
		if (!c->oturum[0]) {
			snprintf(c->oturum, sizeof c->oturum, "%08X%08X", rastgele32(), rastgele32());
			snprintf(oturum_h, sizeof oturum_h, "Session: %s;timeout=60\r\n", c->oturum);
		}
		if (ses) c->sk = a; else c->vk = a;
		snprintf(ek, sizeof ek, "Transport: RTP/AVP/TCP;unicast;interleaved=%d-%d;ssrc=%08X\r\n%s", a, a + 1,
			 ses ? c->assrc : c->vssrc, oturum_h);
		cevap(c, 200, "OK", r->cseq, ek, NULL);
	} else if (!strcmp(r->yontem, "PLAY")) {
		if (!c->oturum[0] || (c->vk < 0 && c->sk < 0)) { cevap(c, 455, "Method Not Valid in This State", r->cseq, NULL, NULL); return; }
		if (!c->oynuyor && oynatan_sayi() >= R_OYNATAN) { cevap(c, 453, "Not Enough Bandwidth", r->cseq, NULL, NULL); return; }
		double t = mono();
		char v[160] = "", s[160] = "";
		if (c->vk >= 0) snprintf(v, sizeof v, "url=%strackID=0;seq=%u;rtptime=%u", taban, c->vseq, video_ts(t));
		if (c->sk >= 0) snprintf(s, sizeof s, "%surl=%strackID=1;seq=%u;rtptime=%u", v[0] ? "," : "", taban, c->aseq, ses_ts(t));
		snprintf(ek, sizeof ek, "Range: npt=0.000-\r\nRTP-Info: %s%s\r\n%s", v, s, oturum_h);
		cevap(c, 200, "OK", r->cseq, ek, NULL);
		if (!c->oynuyor) {
			char ip[20];
			ip_metin(c->ip, ip, sizeof ip);
			printf("RTSP %s IZLIYOR (%s%s)\n", ip, c->vk >= 0 ? "goruntu" : "", c->sk >= 0 ? "+ses" : "");
		}
		c->oynuyor = 1;
		c->sr_t = 0;
	} else if (!strcmp(r->yontem, "PAUSE")) {
		c->oynuyor = 0;
		cevap(c, 200, "OK", r->cseq, oturum_h, NULL);
	} else if (!strcmp(r->yontem, "TEARDOWN")) {
		c->oynuyor = 0;
		c->kapat_sonra = 1;
		cevap(c, 200, "OK", r->cseq, oturum_h, NULL);
	} else if (!strcmp(r->yontem, "GET_PARAMETER") || !strcmp(r->yontem, "SET_PARAMETER")) {
		cevap(c, 200, "OK", r->cseq, oturum_h, NULL);
	} else {
		cevap(c, 501, "Not Implemented", r->cseq, NULL, NULL);
	}
}

/* gelen tamponu isle: istekler + istemcinin interleaved (RTCP RR) cerceveleri atlanir */
static void gelen_isle(struct istemci *c)
{
	while (c->gn && c->fd >= 0 && !c->kapat_sonra) {
		if (c->atla) {
			size_t k = c->atla < c->gn ? c->atla : c->gn;
			memmove(c->gelen, c->gelen + k, c->gn - k);
			c->gn -= k;
			c->atla -= k;
			continue;
		}
		if (c->gelen[0] == '$') {
			if (c->gn < 4) return;
			c->atla = (size_t)((uint8_t)c->gelen[2] << 8 | (uint8_t)c->gelen[3]) + 4;
			continue;
		}
		char *son = memmem(c->gelen, c->gn, "\r\n\r\n", 4);
		if (!son) {
			if (c->gn >= sizeof c->gelen - 1) { cevap(c, 400, "Bad Request", 0, NULL, NULL); c->kapat_sonra = 1; c->gn = 0; }
			return;
		}
		size_t hn = (size_t)(son - c->gelen) + 4;
		struct rtsp_istek r;
		if (rtsp_ayristir(c->gelen, hn, &r)) {
			cevap(c, 400, "Bad Request", r.cseq > 0 ? r.cseq : 0, NULL, NULL);
			c->kapat_sonra = 1;
			c->gn = 0;
			return;
		}
		if (c->gn < hn + (size_t)r.govde) return;
		istek_isle(c, &r);
		size_t k = hn + (size_t)r.govde;
		memmove(c->gelen, c->gelen + k, c->gn - k);
		c->gn -= k;
		memset(&r, 0, sizeof r);
	}
}

static int rtsp_istemci_ekle(int fd, uint32_t ip)
{
	for (int i = 0; i < R_BAGLANTI; i++) {
		struct istemci *c = &R_ist[i];
		if (c->fd >= 0) continue;
		memset(c, 0, offsetof(struct istemci, cikis));
		c->cb = c->cs = 0;
		c->fd = fd;
		c->ip = ip;
		c->vk = c->sk = -1;
		c->yetkili = c->ret = c->kapat_sonra = c->oynuyor = 0;
		c->tut_t = c->sr_t = 0;
		c->oturum[0] = 0;
		c->bag_t = c->son_istek = c->son_ilerleme = mono();
		c->vseq = (uint16_t)rastgele32();
		c->aseq = (uint16_t)rastgele32();
		c->vssrc = rastgele32();
		c->assrc = rastgele32();
		c->vpk = c->vokt = c->apk = c->aokt = 0;
		c->atlanan = 0;
		int f = fcntl(fd, F_GETFL);
		fcntl(fd, F_SETFL, f | O_NONBLOCK);
		R.baglanti++;
		return i;
	}
	return -1;
}

/* ---------------- kaynak cocuk surecleri ---------------- */
static pid_t cocuk_baslat(int video, int *fd)
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sv)) return -1;
	fflush(stdout);
	pid_t p = fork();
	if (p < 0) { close(sv[0]); close(sv[1]); return -1; }
	if (!p) {
		for (int f = 3; f < 1024; f++)
			if (f != sv[1]) close(f);
		signal(SIGTERM, SIG_DFL);
		signal(SIGINT, SIG_DFL);
		int rc = video ? kamera_akis(sv[1], 7200, R_FPS) : mic_akis(sv[1], 7200, 1);
		fflush(stdout);
		_exit(rc & 255);
	}
	close(sv[1]);
	int f = fcntl(sv[0], F_GETFL);
	fcntl(sv[0], F_SETFL, f | O_NONBLOCK);
	*fd = sv[0];
	return p;
}

static void olen_ekle(pid_t p)
{
	for (int i = 0; i < 8; i++)
		if (!R.olen[i]) { R.olen[i] = p; R.olen_t[i] = mono(); return; }
	kill(p, SIGKILL);
	waitpid(p, NULL, 0);
}

static void kaynak_durdur(int video, double yeniden)
{
	pid_t *p = video ? &R.vpid : &R.spid;
	int *fd = video ? &R.vfd : &R.sfd;
	if (*fd >= 0) close(*fd);	/* cocuk bir sonraki yazimda EPIPE alir, bayraklarini kendisi temizler */
	*fd = -1;
	if (*p > 0) olen_ekle(*p);
	*p = 0;
	if (video) { R.vn = 0; R.vyeniden_t = mono() + yeniden; }
	else { R.sn_ = 0; R.sbaslik = 0; R.ses_canli = 0; R.syeniden_t = mono() + yeniden; }
}

static void olenleri_topla(int hepsi)
{
	for (int i = 0; i < 8; i++) {
		if (!R.olen[i]) continue;
		pid_t r = waitpid(R.olen[i], NULL, WNOHANG);
		if (r == R.olen[i] || (r < 0 && errno == ECHILD)) { R.olen[i] = 0; continue; }
		if (hepsi || mono() - R.olen_t[i] > 3) {
			kill(R.olen[i], SIGKILL);
			waitpid(R.olen[i], NULL, 0);
			mik_cocuk_temizle(R.olen[i]);	/* v2: /tmp/mik-aktif + "kopru <pid>" birak bayragi, yalniz onunsa */
			R.olen[i] = 0;
		}
	}
}

static int bizim_bayrak(const char *yol)
{
	char b[16];
	return dosya_oku(yol, b, sizeof b) > 0 && !strncmp(b, "rtsp", 4);
}

static void kaynaklari_yonet(int vgerek, int sgerek)
{
	double t = mono();
	if (vgerek) {
		if (!dosya_var(kamera_bayrak)) {	/* gizlilik gostergesi: izleyici varken kamera AKTIF gorunur */
			bayrak_yaz_icerik(kamera_bayrak, 1, "rtsp\n");
			R.bizim_kamera = 1;
		}
		if (!R.vpid && t >= R.vyeniden_t) {
			R.vpid = cocuk_baslat(1, &R.vfd);
			if (R.vpid < 0) { R.vpid = 0; R.vyeniden_t = t + 3; }
			R.vn = 0;
		}
	} else {
		if (R.vpid) kaynak_durdur(1, 0);
		if (R.bizim_kamera) {
			if (bizim_bayrak(kamera_bayrak)) unlink(kamera_bayrak);
			R.bizim_kamera = 0;
		}
	}
	if (sgerek) {
		if (!R.spid && t >= R.syeniden_t) {
			R.spid = cocuk_baslat(0, &R.sfd);
			if (R.spid < 0) { R.spid = 0; R.syeniden_t = t + 10; }
			R.sn_ = 0;
			R.sbaslik = 0;
		}
	} else if (R.spid) {
		kaynak_durdur(0, 0);
	}
}

struct paket_ctx { struct istemci *c; };
static int paket_istemciye(void *v, const uint8_t *p, size_t n)
{
	struct paket_ctx *x = v;
	x->c->vpk++;
	x->c->vokt += (uint32_t)(n - 12);
	return cikis_kanal(x->c, x->c->vk, p, n);
}

static void kare_dagit(const uint8_t *j, size_t n)
{
	struct rjpeg r;
	const char *e = rjpeg_ayristir(j, n, &r);
	if (e) {
		if (strcmp(R.jpeg_uyum, e)) printf("RTSP: kare RFC 2435'e uygun degil: %s\n", e);
		snprintf(R.jpeg_uyum, sizeof R.jpeg_uyum, "%s", e);
		return;
	}
	R.jpeg_uyum[0] = 0;
	if (!r.dht_standart && !R.dht_uyari) {
		R.dht_uyari = 1;
		printf("RTSP UYARI: kamera standart olmayan Huffman tablosu kullaniyor; RFC 2435 alicisinda goruntu bozuk cikabilir\n");
	}
	R.kareler++;
	uint32_t ts = video_ts(mono());
	size_t gerek = r.tarama_n + (r.tarama_n / (R_MTU - 40) + 2) * 40 + 200;
	for (int i = 0; i < R_BAGLANTI; i++) {
		struct istemci *c = &R_ist[i];
		if (c->fd < 0 || !c->oynuyor || c->vk < 0) continue;
		if (cikis_bos(c) > R_KARE_SINIR || !cikis_yer(c, gerek)) { c->atlanan++; continue; }	/* yavas istemci: kare atla */
		struct paket_ctx x = { c };
		rtp_jpeg_paketle(&r, ts, c->vssrc, &c->vseq, R_MTU, paket_istemciye, &x);
	}
}

static void ses_paket(const uint8_t *u)
{
	double t = mono();
	uint8_t p[12 + 160];
	for (int i = 0; i < R_BAGLANTI; i++) {
		struct istemci *c = &R_ist[i];
		if (c->fd < 0 || !c->oynuyor || c->sk < 0) continue;
		rtp_baslik(p, 0, 0, c->aseq, R.ats, c->assrc);
		memcpy(p + 12, u, 160);
		if (!cikis_kanal(c, c->sk, p, sizeof p)) { c->aseq++; c->apk++; c->aokt += 160; }
	}
	R.ats += 160;
	R.ats_mono = t;
}

static void video_oku(void)
{
	for (;;) {
		if (R.vn >= R_VBUF) { kaynak_durdur(1, 3); return; }
		ssize_t r = read(R.vfd, R.vbuf + R.vn, R_VBUF - R.vn);
		if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
		if (r <= 0) { kaynak_durdur(1, 3); return; }
		R.vn += (size_t)r;
		while (R.vn >= 4) {
			if (!memcmp(R.vbuf, "HATA", 4)) {
				uint8_t *nl = memchr(R.vbuf, '\n', R.vn);
				if (!nl && R.vn < 96) break;
				size_t k = nl ? (size_t)(nl - R.vbuf) : 95;
				snprintf(R.vhata, sizeof R.vhata, "%.*s", (int)(k > 5 ? k - 5 : 0), (char *)R.vbuf + 5);
				printf("RTSP goruntu kaynagi: %s\n", R.vhata);
				kaynak_durdur(1, 3);
				return;
			}
			uint32_t n = R.vbuf[0] | R.vbuf[1] << 8 | R.vbuf[2] << 16 | (uint32_t)R.vbuf[3] << 24;
			if (n < 4 || n > 300000) { snprintf(R.vhata, sizeof R.vhata, "kare boyu gecersiz"); kaynak_durdur(1, 3); return; }
			if (R.vn < 4 + (size_t)n) break;
			R.vhata[0] = 0;
			kare_dagit(R.vbuf + 4, n);
			memmove(R.vbuf, R.vbuf + 4 + n, R.vn - 4 - n);
			R.vn -= 4 + (size_t)n;
		}
	}
}

static void ses_oku(void)
{
	for (;;) {
		ssize_t r = read(R.sfd, R.sbuf + R.sn_, sizeof R.sbuf - R.sn_);
		if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
		if (r <= 0) { kaynak_durdur(0, 10); return; }
		R.sn_ += (size_t)r;
		if (!R.sbaslik) {
			uint8_t *nl = memchr(R.sbuf, '\n', R.sn_);
			if (!nl) { if (R.sn_ >= 64) kaynak_durdur(0, 10); return; }
			size_t k = (size_t)(nl - R.sbuf);
			if (strncmp((char *)R.sbuf, "MIC_AKIS 16000", 14)) {
				snprintf(R.shata, sizeof R.shata, "%.*s", (int)(k > 5 ? k - 5 : 0), (char *)R.sbuf + 5);
				printf("RTSP ses kaynagi: %s (sessizlik gonderilir)\n", R.shata);
				kaynak_durdur(0, 10);
				return;
			}
			R.shata[0] = 0;
			R.sbaslik = 1;
			memmove(R.sbuf, nl + 1, R.sn_ - k - 1);
			R.sn_ -= k + 1;
		}
		size_t ornek = R.sn_ / 2;
		static uint8_t u[4096];
		static int16_t hiza[4096];	/* hizali kopya (RISC-V'de hizasiz erisimden kacin) */
		memcpy(hiza, R.sbuf, ornek * 2);
		size_t m = az8_ulaw(&R.az, hiza, ornek, u);
		memmove(R.sbuf, R.sbuf + ornek * 2, R.sn_ - ornek * 2);
		R.sn_ -= ornek * 2;
		for (size_t i = 0; i < m; i++) {
			R.ubuf[R.un++] = u[i];
			if (R.un == 160) { ses_paket(R.ubuf); R.un = 0; }
		}
		R.ses_canli = 1;
		R.ses_son_t = mono();
	}
}

static void hepsini_kapat(const char *neden)
{
	for (int i = 0; i < R_BAGLANTI; i++) istemci_kapat(&R_ist[i], neden);
}

static void durum_dosyasi(int calisiyor)
{
	char tmp[160];
	snprintf(tmp, sizeof tmp, "%s.tmp", rtsp_durum_yol);
	FILE *f = fopen(tmp, "w");
	if (!f) return;
	int ist = 0;
	for (int i = 0; i < R_BAGLANTI; i++) ist += R_ist[i].fd >= 0;
	double t = mono();
	const char *ses = !R.spid && !R.shata[0] ? "kapali" : R.ses_canli && t - R.ses_son_t < 1 ? "aktif" : R.shata[0] ? "mesgul" : "bekliyor";
	const char *video = !R.vpid && !R.vhata[0] ? "kapali" : R.vhata[0] ? "hata" : "aktif";
	fprintf(f, "{\"surum\":\"kole-kopru-v8-rtsp\",\"calisiyor\":%s,\"acik\":%s,\"dinliyor\":%s,\"parola\":%s,\"port\":%d,"
		"\"istemci\":%d,\"oynatan\":%d,\"azami_oynatan\":%d,\"video\":\"%s\",\"ses\":\"%s\",\"kare\":%lu,\"red\":%lu,"
		"\"kilitli\":%s,\"video_hata\":\"%s\",\"ses_hata\":\"%s\",\"jpeg_uyum\":\"%s\",\"dht_standart\":%s,\"fps\":%d,"
		"\"tasima\":\"RTP/AVP/TCP\",\"yollar\":[\"/kamera\",\"/video\"]}\n",
		calisiyor ? "true" : "false", R.acik ? "true" : "false", R.lfd >= 0 ? "true" : "false",
		R.parola_var ? "true" : "false", R.port, ist, oynatan_sayi(), R_OYNATAN, video, ses, R.kareler, R.red,
		kilit_var(t) ? "true" : "false", R.vhata, R.shata, R.jpeg_uyum, R.dht_uyari ? "false" : "true", R_FPS);
	fclose(f);
	rename(tmp, rtsp_durum_yol);
}

static void dinle(void)
{
	int l = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	int bir = 1;
	setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &bir, sizeof bir);
	struct sockaddr_in ad = { .sin_family = AF_INET, .sin_port = htons((uint16_t)R.port),
				  .sin_addr.s_addr = htonl(rtsp_bag_ip) };
	if (l < 0 || bind(l, (struct sockaddr *)&ad, sizeof ad) || listen(l, 4)) {
		perror("RTSP bind/listen");
		if (l >= 0) close(l);
		return;
	}
	R.lfd = l;
	printf("RTSP: TCP %d dinleniyor (kullanici %s)\n", R.port, R.kul);
}

static void rtsp_hazirla(int port, int dinleme_yok)
{
	memset(&R, 0, sizeof R);
	for (int i = 0; i < R_BAGLANTI; i++) R_ist[i].fd = -1;
	R.lfd = R.vfd = R.sfd = -1;
	R.port = port;
	R.dinleme_yok = dinleme_yok;
	R.t0 = mono();
	R.vbase = rastgele32();
	R.ats = rastgele32();
	for (int i = 0; i < 4; i++) snprintf(R.gizli + 8 * i, 9, "%08x", rastgele32());
	az8_hazirla();
	R.kontrol_t = -10;
}

/* tek dongu adimi (en fazla ms bekler). Sunucu ve self-test ayni yolu kullanir */
static void rtsp_adim(int ms)
{
	double t = mono();
	if (t - R.kontrol_t >= 1.0) {
		R.kontrol_t = t;
		int kalkan = dosya_var(kalkan_yol);
		R.acik = (rtsp_her_zaman_acik || dosya_var(rtsp_bayrak)) && !kalkan;
		parola_yukle();
		if (!R.acik || !R.parola_var) {
			int vardi = R.lfd >= 0;
			for (int i = 0; i < R_BAGLANTI; i++) vardi |= R_ist[i].fd >= 0;
			if (vardi) {
				hepsini_kapat(kalkan ? "gizlilik kalkani" : !R.acik ? "RTSP kapatildi" : "parola dosyasi yok");
				if (R.lfd >= 0) { close(R.lfd); R.lfd = -1; printf("RTSP: dinleme durdu\n"); }
			}
		} else if (R.lfd < 0 && !R.dinleme_yok) {
			dinle();
		}
	}
	int vgerek = 0, sgerek = 0;
	for (int i = 0; i < R_BAGLANTI; i++) {
		struct istemci *c = &R_ist[i];
		if (c->fd >= 0 && c->oynuyor) { vgerek |= c->vk >= 0; sgerek |= c->sk >= 0; }
	}
	kaynaklari_yonet(vgerek, sgerek);
	int oyn = oynatan_sayi();
	if (oyn != R.son_oynatan) {
		char b[16];
		snprintf(b, sizeof b, "%d\n", oyn);
		bayrak_yaz_icerik(rtsp_aktif_yol, oyn > 0, b);
		R.son_oynatan = oyn;
	}

	struct pollfd p[R_BAGLANTI + 3];
	int idx[R_BAGLANTI + 3], n = 0;
	if (R.lfd >= 0) { p[n] = (struct pollfd){ R.lfd, POLLIN, 0 }; idx[n++] = -1; }
	if (R.vfd >= 0) { p[n] = (struct pollfd){ R.vfd, POLLIN, 0 }; idx[n++] = -2; }
	if (R.sfd >= 0) { p[n] = (struct pollfd){ R.sfd, POLLIN, 0 }; idx[n++] = -3; }
	for (int i = 0; i < R_BAGLANTI; i++) {
		struct istemci *c = &R_ist[i];
		if (c->fd < 0) continue;
		short e = POLLIN;
		if (cikis_bos(c) && t >= c->tut_t) e |= POLLOUT;
		p[n] = (struct pollfd){ c->fd, e, 0 };
		idx[n++] = i;
	}
	int r = poll(p, (nfds_t)n, ms);
	t = mono();
	for (int k = 0; r > 0 && k < n; k++) {
		if (!p[k].revents) continue;
		if (idx[k] == -1) {
			struct sockaddr_in ia;
			socklen_t il = sizeof ia;
			int s = accept4(R.lfd, (struct sockaddr *)&ia, &il, SOCK_CLOEXEC);
			if (s < 0) continue;
			uint32_t ip = ntohl(ia.sin_addr.s_addr);
			char ipm[20];
			ip_metin(ip, ipm, sizeof ipm);
			if (!ozel_ip(ip)) { printf("RTSP %s RED (yerel/Tailscale disi)\n", ipm); close(s); continue; }
			int bir = 1;
			setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &bir, sizeof bir);
			if (rtsp_istemci_ekle(s, ip) < 0) { printf("RTSP %s RED (en fazla %d baglanti)\n", ipm, R_BAGLANTI); close(s); continue; }
			printf("RTSP %s baglandi\n", ipm);
			continue;
		}
		if (idx[k] == -2) { if (R.vfd >= 0) video_oku(); continue; }
		if (idx[k] == -3) { if (R.sfd >= 0) ses_oku(); continue; }
		struct istemci *c = &R_ist[idx[k]];
		if (c->fd < 0) continue;
		if (p[k].revents & (POLLERR | POLLNVAL)) { istemci_kapat(c, "soket hatasi"); continue; }
		if (p[k].revents & POLLOUT) {
			ssize_t w = send(c->fd, c->cikis + c->cb, cikis_bos(c), MSG_NOSIGNAL | MSG_DONTWAIT);
			if (w < 0 && errno == ENOTSOCK) w = write(c->fd, c->cikis + c->cb, cikis_bos(c));
			if (w > 0) { c->cb += (size_t)w; c->son_ilerleme = t; if (c->cb == c->cs) c->cb = c->cs = 0; }
			else if (w < 0 && errno != EAGAIN && errno != EINTR) { istemci_kapat(c, "gonderim hatasi"); continue; }
		}
		if (p[k].revents & (POLLIN | POLLHUP)) {
			ssize_t g = recv(c->fd, c->gelen + c->gn, sizeof c->gelen - 1 - c->gn, MSG_DONTWAIT);
			if (g < 0 && errno == ENOTSOCK) g = read(c->fd, c->gelen + c->gn, sizeof c->gelen - 1 - c->gn);
			if (g == 0 || (g < 0 && errno != EAGAIN && errno != EINTR)) { istemci_kapat(c, "istemci kapatti"); continue; }
			if (g > 0) { c->gn += (size_t)g; gelen_isle(c); }
		}
	}
	/* ses yoksa (mikrofon mesgul / ses karti yok) 20 ms'de bir sessizlik: oynaticilar ses izini beklemesin */
	if (sgerek && !(R.ses_canli && t - R.ses_son_t < 0.5)) {
		if (R.sessiz_t <= 0 || t - R.sessiz_t > 0.5) R.sessiz_t = t;
		uint8_t sus[160];
		memset(sus, 0xff, sizeof sus);
		while (t - R.sessiz_t >= 0.02) { ses_paket(sus); R.sessiz_t += 0.02; }
	} else {
		R.sessiz_t = 0;
	}
	for (int i = 0; i < R_BAGLANTI; i++) {
		struct istemci *c = &R_ist[i];
		if (c->fd < 0) continue;
		if (c->oynuyor && t - c->sr_t >= 2.0) {
			c->sr_t = t;
			struct timespec rt;
			clock_gettime(CLOCK_REALTIME, &rt);
			uint64_t ntp = ((uint64_t)(rt.tv_sec + 2208988800u) << 32) | (uint64_t)((double)rt.tv_nsec * 4.294967296);
			uint8_t sr[64];
			if (c->vk >= 0) cikis_kanal(c, c->vk + 1, sr, rtcp_sr(sr, c->vssrc, ntp, video_ts(t), c->vpk, c->vokt));
			if (c->sk >= 0) cikis_kanal(c, c->sk + 1, sr, rtcp_sr(sr, c->assrc, ntp, ses_ts(t), c->apk, c->aokt));
		}
		if (c->kapat_sonra && !cikis_bos(c) && t >= c->tut_t) istemci_kapat(c, "bitti");
		else if (!c->yetkili && t - c->bag_t > 20) istemci_kapat(c, "giris zaman asimi");
		else if (!c->oynuyor && t - c->son_istek > 60) istemci_kapat(c, "bosta");
		else if (cikis_bos(c) && t - c->son_ilerleme > 20) istemci_kapat(c, "istemci okumuyor");
	}
	olenleri_topla(0);
	if (t - R.durum_t >= 2.0) { R.durum_t = t; durum_dosyasi(1); }
}

static void rtsp_bitir(void)
{
	hepsini_kapat("sunucu duruyor");
	if (R.lfd >= 0) close(R.lfd);
	R.lfd = -1;
	kaynaklari_yonet(0, 0);
	double t = mono();
	while (mono() - t < 3) {
		int kalan = 0;
		for (int i = 0; i < 8; i++) kalan += R.olen[i] != 0;
		if (!kalan) break;
		olenleri_topla(0);
		struct timespec d = { 0, 50000000 };
		nanosleep(&d, NULL);
	}
	olenleri_topla(1);
	unlink(rtsp_aktif_yol);
	durum_dosyasi(0);
}

static void rtsp_sinyal(int s) { (void)s; R.dur = 1; }

/* v2: oldurulmus onceki RTSP surecinin "rtsp" kamera bayragi ve /tmp/rtsp-aktif (baskasinin bayragina dokunulmaz) */
static void rtsp_artik_temizle(void)
{
	if (bizim_bayrak(kamera_bayrak)) unlink(kamera_bayrak);
	unlink(rtsp_aktif_yol);
}

static int rtsp_sunucu(int port)
{
	rtsp_hazirla(port, 0);
	rtsp_artik_temizle();	/* v2 */
	signal(SIGTERM, rtsp_sinyal);
	signal(SIGINT, rtsp_sinyal);
	printf("kole-kopru v8 RTSP: bayrak %s, parola %s (bayrak yoksa dinlemez)\n", rtsp_bayrak, rtsp_parola_yol);
	fflush(stdout);
	double kirp_t = mono();
	while (!R.dur) {
		rtsp_adim(20);
		fflush(stdout);
		if (mono() - kirp_t >= 60) { kirp_t = mono(); RTSP_GUNLUK_KIRP(); }	/* v2: kalici izleyicide de */
	}
	rtsp_bitir();
	printf("RTSP: durdu\n");
	return 0;
}
#endif
