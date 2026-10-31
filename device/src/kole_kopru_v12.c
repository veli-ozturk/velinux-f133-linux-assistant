/* kole_kopru v12 (24.10.2026, Claude; docs/denetim-2026-10-24-v3.md A5, A14, A18; scripts/624): v11 + ses_akis_v4.h
 * (birak bayragi "kopru <pid>" sahipli), rtsp_sunucu_v2.h (IP basina kilit, stale nonce, nonce IP'ye bagli, uri yolu,
 * artik bayrak temizligi, dongude gunluk kirpma), kamera_akis_v2.h (kayitci donunce USB birakilir); komut satiri toplam
 * 5 sn (v11 bayt basina). */
/* kole_kopru v11 (23.10.2026, Claude; docs/denetim-2026-10-23-v2.md bulgu 6, 8; scripts/604): v9 + izinli_ip_v1.h:
 * kopru 8767 ve RTSP 8554 baglantisi hicbir sey okunmadan once denetlenir (127.0.0.1 cihaz-ici asistan, Tailscale,
 * /data/izinli-ip); stdout gunlugu (>> O_APPEND) 256 KB'i asinca bastan yazilir (her kabulde denetim). v10 taslagi
 * (yarim ses isi) kurulmadi, bu v9 tabanli. */
/* kole_kopru v9 (15.10.2026, Claude): v8 + ses_akis_v2.h (Hey Kole mikrofonu birakir; scripts/178). */
/* kole_kopru v8 (13.10.2026, Claude): v7 + canli ses (dinleme + bas-konus) + asgari RTSP sunucusu (IP kamera).
 * Moduler: ortak yardimcilar kopru_ortak_v1.h, ses ses_akis_v1.h, kamera kaynagi kamera_akis_v1.h (YALNIZ READ(10)),
 * SD kayitlari kayit_sd_v1.h (salt okuma), RTSP saf yardimcilar rtsp_v1.h + sunucu rtsp_sunucu_v1.h. Ileride
 * cihaz-ici (PC'siz) web merkezi ayni basliklari icerecek.
 * Yeni komutlar (anahtarli TCP 8767, v7 komutlari aynen):
 *   MIC_AKIS [sn]        : "MIC_AKIS 16000 1 s16le\n" + 16 kHz mono PCM (en fazla sn, varsayilan 1800, <=7200) ya da
 *                          "HATA kalkan acik|mik mesgul|mik yok". Suresince /tmp/mik-aktif. Kalkan gelince durur.
 *   PLAY_AKIS [sn]       : bas-konus. "PLAY_AKIS HAZIR 16000\n", sonra [u16 LE n][PCM16k] cerceveleri, n=0 son,
 *                          "OK <ornek> <xrun>\n". Yalniz "Headphone Switch" (v7 PLAY ile ayni); MCU cercevesi yok.
 *   RTSP_DURUM           : {"bayrak","parola_dosyasi","kalkan","durum":rtsp-durum.json|null,"durum_yas_ms"}
 *   RTSP 1|0             : /mnt/sd/velinux/rtsp-acik bayragi (kalkan acikken 1 reddedilir)
 *   RTSP_PAROLA <k> <ha1>: /data/rtsp-parola = "k ha1" (0600, atomik). "RTSP_PAROLA -" siler. Parola KOPRUDEN GECMEZ.
 *   DURUM                : v7 + hoparlor (yok|dogrulanmadi|dogrulandi), hoparlor_aktif, rtsp, yeni yetenekler.
 *   v7 MIC artik gizlilik kalkaninda calismaz (ayni ses yolu, 120 sn).
 * Ek kipler: "rtsp [port]" (RTSP sunucusu, varsayilan 8554), "rtsp-test <port> <jpeg> <parola> <dizin> [sn]"
 * (host sinamasi: sahte kamera/ses, bayraklar <dizin>'de), "jpeg-denetle <dosya>", "--ornek-jpeg <dosya> [dri]",
 * "--ulaw-tablo" (65536 bayt), "--self-test", "--durum-ornek".
 * --- v7 ve oncesi: kole_kopru_v7.c basindaki aciklamalar. */
#define _GNU_SOURCE
#include <sys/statvfs.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <dirent.h>
#include <linux/fb.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "kopru_ortak_v1.h"
#include "ses_akis_v4.h"	/* v12: birak bayragi sahipli (denetim v3 A5) */
#include "kamera_akis_v2.h"	/* v12: kayitci donunce USB birakilir (A18e) */
#include "kayit_sd_v1.h"
#include "izinli_ip_v1.h"	/* v11 */

/* v11: stdout duzenli dosyaysa ve 256 KB'i astiysa bosalt (kamera_kopru_sd yalniz yeniden baslatmada kirpiyordu).
 * 1: kirpildi */
static int gunluk_kirp(int fd)
{
	struct stat st;
	if (fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size <= 256 * 1024)
		return 0;
	if (ftruncate(fd, 0))
		return 0;
	lseek(fd, 0, SEEK_SET);	/* O_APPEND degilse de bastan */
	return 1;
}

/* v11: rtsp_sunucu_v1.h'deki tek accept4 (RTSP 8554) baslik degismeden izinli_ip'den gecer */
static int izinli_accept4(int l, struct sockaddr *a, socklen_t *n, int bayrak)
{
	gunluk_kirp(1);
	int s = accept4(l, a, n, bayrak);
	if (s >= 0 && !izinli_soket(s, NULL)) {
		uint32_t ip = ntohl(((struct sockaddr_in *)a)->sin_addr.s_addr);
		printf("RTSP %u.%u.%u.%u RED (izinli-ip)\n", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
		close(s);
		errno = EAGAIN;
		return -1;
	}
	return s;
}
#define accept4 izinli_accept4
#define RTSP_GUNLUK_KIRP() gunluk_kirp(1)	/* v12: RTSP dongusunde dakikada bir (v11 yalniz kabulde) */
#include "rtsp_sunucu_v2.h"
#undef accept4

#define ANAHTAR_DOSYA "/data/ses-anahtar"
static const char *hoparlor_dogrulandi_yol = "/data/hoparlor-dogrulandi";	/* kullanıcı ses sorununu cozunce olusturur */
#define YETENEKLER "KAYIT_OKU_ARALIK KAYIT_IDX KAYIT_KARE KAYIT_OLAYLAR KALKAN SAGLIK MIC_AKIS PLAY_AKIS RTSP"

static int anahtar_oku(char *k, size_t n)
{
	FILE *f = fopen(ANAHTAR_DOSYA, "r");
	if (!f) {
		unsigned char r[16];
		int u = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
		if (u < 0 || read(u, r, sizeof r) != (ssize_t)sizeof r) { if (u >= 0) close(u); return -1; }
		close(u);
		int w = open(ANAHTAR_DOSYA, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0600);
		if (w < 0) return -1;
		for (int i = 0; i < 16; i++)
			dprintf(w, "%02x", r[i]);
		dprintf(w, "\n");
		fsync(w);
		close(w);
		f = fopen(ANAHTAR_DOSYA, "r");
		if (!f) return -1;
		puts("yeni anahtar uretildi: " ANAHTAR_DOSYA);
	}
	if (!fgets(k, (int)n, f)) { fclose(f); return -1; }
	fclose(f);
	k[strcspn(k, "\r\n")] = 0;
	return strlen(k) >= 16 ? 0 : -1;
}

/* sabit zamanli karsilastirma */
static int esit(const char *a, const char *b)
{
	size_t na = strlen(a), nb = strlen(b);
	unsigned d = (unsigned)(na ^ nb);
	for (size_t i = 0; i < na && i < nb; i++)
		d |= (unsigned char)(a[i] ^ b[i]);
	return d == 0;
}

static int pid_bul(const char *ad)
{
	DIR *d = opendir("/proc");
	struct dirent *e;
	int bulunan = 0;
	char yol[64], c[64];
	while (d && (e = readdir(d))) {
		if (e->d_name[0] < '1' || e->d_name[0] > '9') continue;
		snprintf(yol, sizeof yol, "/proc/%s/comm", e->d_name);
		if (dosya_oku(yol, c, sizeof c) > 0 && !strcmp(c, ad)) { bulunan = atoi(e->d_name); break; }
	}
	if (d) closedir(d);
	return bulunan;
}

/* dosyanin degisme zamanindan bu yana gecen ms; yoksa -1 */
static long long yas_ms(const char *p)
{
	struct stat st;
	struct timespec t;
	if (stat(p, &st)) return -1;
	clock_gettime(CLOCK_REALTIME, &t);
	long long d = ((long long)t.tv_sec - st.st_mtim.tv_sec) * 1000 + (t.tv_nsec - st.st_mtim.tv_nsec) / 1000000;
	return d < 0 ? 0 : d;
}

static const char *sayi_veya_null(char *b, size_t n, long long v)
{
	if (v < 0) return "null";
	snprintf(b, n, "%lld", v);
	return b;
}

static const char *hoparlor_durum(void)
{
	char yol[160];
	snprintf(yol, sizeof yol, "%s/pcmC0D0p", snd_kok);
	if (!dosya_var(yol)) return "yok";
	return dosya_var(hoparlor_dogrulandi_yol) ? "dogrulandi" : "dogrulanmadi";
}

/* rtsp-durum.json tazeyse (<=10 sn) icerigi, degilse "null" */
static const char *rtsp_ozet(char *b, size_t n, long long *yas)
{
	*yas = yas_ms(rtsp_durum_yol);
	if (*yas < 0 || *yas > 10000 || dosya_oku(rtsp_durum_yol, b, n) <= 0 || b[0] != '{') return "null";
	return b;
}

static int durum(int s)
{
	char up[64], la[64], bt[1024], tmp[64], buf[512], mem[16] = "0", wl[64] = "null", rb[1024];
	dosya_oku("/proc/uptime", up, sizeof up);
	dosya_oku("/proc/loadavg", la, sizeof la);
	if (dosya_oku("/tmp/bt-durum.json", bt, sizeof bt) <= 0 || bt[0] != '{') strcpy(bt, "null");
	if (dosya_oku("/sys/class/thermal/thermal_zone0/temp", tmp, sizeof tmp) <= 0) strcpy(tmp, "null");
	FILE *f = fopen("/proc/meminfo", "r");
	while (f && fgets(buf, sizeof buf, f))
		if (sscanf(buf, "MemAvailable: %15s", mem) == 1) break;
	if (f) fclose(f);
	f = fopen("/proc/net/wireless", "r");
	while (f && fgets(buf, sizeof buf, f)) {
		float link, level;
		if (strstr(buf, "wlan0:") && sscanf(strstr(buf, ":") + 1, "%*s %f %f", &link, &level) == 2) {
			snprintf(wl, sizeof wl, "{\"link\":%.0f,\"dbm\":%.0f}", link, level);
			break;
		}
	}
	if (f) fclose(f);
	int sd = 0;
	f = fopen("/proc/mounts", "r");
	while (f && fgets(buf, sizeof buf, f))
		if (strstr(buf, " /mnt/sd ")) sd = strstr(buf, " rw") ? 2 : 1;
	if (f) fclose(f);
	long long sd_bos = -1, sd_top = -1, ryas;
	struct statvfs v;
	if (sd && !statvfs(sd_kok, &v)) {
		sd_bos = (long long)v.f_bavail * (long long)v.f_frsize / 1048576;
		sd_top = (long long)v.f_blocks * (long long)v.f_frsize / 1048576;
	}
	char kam[32], s1[24], s2[24], s3[24], s4[24], snd[160];
	snprintf(snd, sizeof snd, "%s/pcmC0D0p", snd_kok);
	int kamera_usb = dosya_oku("/sys/bus/usb/devices/1-1/idVendor", kam, sizeof kam) > 0;
	char *u1 = strtok(up, " "), *l = la;
	const char *rtsp = rtsp_ozet(rb, sizeof rb, &ryas);
	static char j[4096];
	int n = snprintf(j, sizeof j,
		"{\"surum\":\"kole-kopru-v8\",\"uptime\":%s,\"loadavg\":\"%.14s\",\"mem_kb\":%s,\"wifi\":%s,"
		"\"sicaklik_mC\":%s,\"sd\":%d,\"sd_bos_mb\":%s,\"sd_toplam_mb\":%s,\"kamera_usb\":%s,\"kamera_aktif\":%s,"
		"\"mik_aktif\":%s,\"hoparlor_aktif\":%s,\"kalkan\":%s,\"ses_karti\":%s,\"hoparlor\":\"%s\","
		"\"rtsp_bayrak\":%s,\"rtsp\":%s,\"pid\":{\"panel\":%d,\"wifi_ekran\":%d,\"mcu\":%d},"
		"\"kamera_saglik\":{\"hizmet\":%s,\"son_kare_yas_ms\":%s,\"durum_yas_ms\":%s},"
		"\"yetenek\":[\"KAYIT_OKU_ARALIK\",\"KAYIT_IDX\",\"KAYIT_KARE\",\"KAYIT_OLAYLAR\",\"KALKAN\",\"SAGLIK\","
		"\"MIC_AKIS\",\"PLAY_AKIS\",\"RTSP\"],\"bt\":%s}\n",
		u1 ? u1 : "0", l, mem, wl, tmp, sd, sayi_veya_null(s1, sizeof s1, sd_bos), sayi_veya_null(s2, sizeof s2, sd_top),
		kamera_usb ? "true" : "false", dosya_var(kamera_bayrak) ? "true" : "false",
		dosya_var(mik_bayrak) ? "true" : "false", dosya_var(hoparlor_bayrak) ? "true" : "false",
		dosya_var(kalkan_yol) ? "true" : "false", dosya_var(snd) ? "true" : "false", hoparlor_durum(),
		dosya_var(rtsp_bayrak) ? "true" : "false", rtsp,
		pid_bul("panel"), pid_bul("wifi-ekran-v16"), pid_bul("mcu-servis-v3"),
		hizmet_var() ? "true" : "false", sayi_veya_null(s3, sizeof s3, yas_ms(kamera_son)),
		sayi_veya_null(s4, sizeof s4, yas_ms("/tmp/kamera-kayit.json")), bt);
	if (n >= (int)sizeof j) n = (int)sizeof j - 1;
	return tam_yaz(s, j, (size_t)n);
}

static int rtsp_durum_kmt(int s)
{
	char rb[1024], y[24], o[1400];
	long long yas;
	const char *d = rtsp_ozet(rb, sizeof rb, &yas);
	int n = snprintf(o, sizeof o, "{\"bayrak\":%s,\"parola_dosyasi\":%s,\"kalkan\":%s,\"durum\":%s,\"durum_yas_ms\":%s}\n",
			 dosya_var(rtsp_bayrak) ? "true" : "false", dosya_var(rtsp_parola_yol) ? "true" : "false",
			 dosya_var(kalkan_yol) ? "true" : "false", d, sayi_veya_null(y, sizeof y, yas));
	if (n >= (int)sizeof o) n = (int)sizeof o - 1;
	return tam_yaz(s, o, (size_t)n);
}

/* RTSP_PAROLA <kullanici> <ha1>: parola degil HA1 gelir (MD5(k:VeLinux-KOLE:p)); "-" dosyayi siler */
static int rtsp_parola_kmt(int s, const char *k, const char *h)
{
	if (k && !strcmp(k, "-") && !h) {
		unlink(rtsp_parola_yol);
		sync();
		return tam_yaz(s, "RTSP_PAROLA OK\n", 15);
	}
	if (!k || !h || !kullanici_gecerli(k) || !hex32_mi(h)) return hata(s, "parola bicimi");
	char tmp[200];
	snprintf(tmp, sizeof tmp, "%s.tmp", rtsp_parola_yol);
	int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
	if (fd < 0) return hata(s, "yazilamadi");
	fchmod(fd, 0600);
	char satir[128];
	int n = snprintf(satir, sizeof satir, "%s ", k);
	for (int i = 0; i < 32; i++) satir[n++] = (char)tolower((unsigned char)h[i]);
	satir[n++] = '\n';
	int ok = !tam_yaz(fd, satir, (size_t)n) && !fsync(fd);
	close(fd);
	memset(satir, 0, sizeof satir);
	if (!ok || rename(tmp, rtsp_parola_yol)) { unlink(tmp); return hata(s, "yazilamadi"); }
	sync();
	return tam_yaz(s, "RTSP_PAROLA OK\n", 15);
}

static int ekran(int s, int adim)
{
	if (adim < 2 || adim > 8) adim = 4;
	int fd = open("/dev/fb0", O_RDONLY | O_CLOEXEC);
	if (fd < 0) return -1;
	struct fb_var_screeninfo v;
	struct fb_fix_screeninfo x;
	if (ioctl(fd, FBIOGET_VSCREENINFO, &v) || ioctl(fd, FBIOGET_FSCREENINFO, &x) || v.bits_per_pixel != 32) {
		close(fd);
		return -1;
	}
	size_t boy = (size_t)x.line_length * v.yres_virtual;
	uint8_t *m = mmap(NULL, boy, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	if (m == MAP_FAILED) return -1;
	unsigned g = v.xres / adim, y = v.yres / adim;
	char h[48];
	int hn = snprintf(h, sizeof h, "EKRAN %u %u\n", g, y);
	int rc = tam_yaz(s, h, (size_t)hn);
	static uint8_t satir[4096 * 3];
	for (unsigned r = 0; !rc && r < y; r++) {
		const uint8_t *p = m + (size_t)(v.yoffset + r * adim) * x.line_length + (size_t)v.xoffset * 4;
		for (unsigned c = 0; c < g; c++) {
			uint32_t px = *(const volatile uint32_t *)(p + (size_t)c * adim * 4);
			satir[3 * c] = (uint8_t)(px >> v.red.offset);
			satir[3 * c + 1] = (uint8_t)(px >> v.green.offset);
			satir[3 * c + 2] = (uint8_t)(px >> v.blue.offset);
		}
		rc = tam_yaz(s, satir, (size_t)g * 3);
	}
	munmap(m, boy);
	return rc;
}

static void zaman_asimi(int s, int opt, int sn)
{
	struct timeval t = { .tv_sec = sn };
	setsockopt(s, SOL_SOCKET, opt, &t, sizeof t);
}

/* tek komut; a1..a3 bos olabilir. Sunucu ve self-test ayni yolu kullanir. */
static void komut_isle(int s, const char *kmt, char *a1, char *a2, char *a3)
{
	if (!strcmp(kmt, "DURUM")) {
		durum(s);
	} else if (!strcmp(kmt, "YETENEK")) {
		tam_yaz(s, "YETENEK 8 " YETENEKLER "\n", strlen("YETENEK 8 " YETENEKLER "\n"));
	} else if (!strcmp(kmt, "EKRAN")) {
		ekran(s, a1 ? atoi(a1) : 4);
	} else if (!strcmp(kmt, "KAYIT") && a1) {
		int ac = a1[0] == '1';
		if (ac && dosya_var(kalkan_yol)) { hata(s, "kalkan acik"); return; }
		char yol[600];
		mkdir(kok, 0755);
		snprintf(yol, sizeof yol, "%s/kayit-acik", kok);
		bayrak_yaz(yol, ac);
		sync();
		char c[24];
		int m = snprintf(c, sizeof c, "KAYIT %c OK\n", ac ? '1' : '0');
		tam_yaz(s, c, (size_t)m);
	} else if (!strcmp(kmt, "KAYIT_LISTE")) {
		kayit_liste(s);
	} else if (!strcmp(kmt, "KAYIT_OKU") && a1) {
		zaman_asimi(s, SO_SNDTIMEO, 20);
		kayit_oku(s, a1, a2, a3);
	} else if (!strcmp(kmt, "KAYIT_IDX") && a1) {
		kayit_idx(s, a1);
	} else if (!strcmp(kmt, "KAYIT_KARE") && a1) {
		kayit_kare(s, a1, a2);
	} else if (!strcmp(kmt, "KAYIT_OLAYLAR")) {
		kayit_olaylar(s, a1, a2);
	} else if (!strcmp(kmt, "KAYIT_DURUM")) {
		char kd[800];
		if (dosya_oku("/tmp/kamera-kayit.json", kd, sizeof kd) <= 0) strcpy(kd, "{}");
		tam_yaz(s, kd, strlen(kd));
		tam_yaz(s, "\n", 1);
	} else if (!strcmp(kmt, "KAMERA_AKIS")) {
		zaman_asimi(s, SO_SNDTIMEO, 10);
		kamera_akis(s, a1 ? atof(a1) : 600, a2 ? atof(a2) : 5);
	} else if (!strcmp(kmt, "KAMERA") && a1) {
		int ac = a1[0] == '1';
		if (ac && dosya_var(kalkan_yol)) { hata(s, "kalkan acik"); return; }
		bayrak_yaz(kamera_bayrak, ac);
		char c[24];
		int m = snprintf(c, sizeof c, "KAMERA %c OK\n", ac ? '1' : '0');
		tam_yaz(s, c, (size_t)m);
	} else if (!strcmp(kmt, "MIC")) {
		mic_akis(s, 120, 0);
	} else if (!strcmp(kmt, "MIC_AKIS")) {
		zaman_asimi(s, SO_SNDTIMEO, 10);
		mic_akis(s, a1 ? atof(a1) : 1800, 1);
	} else if (!strcmp(kmt, "PLAY") && a1) {
		zaman_asimi(s, SO_RCVTIMEO, 10);
		int rc = cal_akis(s, atol(a1));
		char c[32];
		int m = snprintf(c, sizeof c, "OK %d\n", rc);
		tam_yaz(s, c, (size_t)m);
	} else if (!strcmp(kmt, "PLAY_AKIS")) {
		zaman_asimi(s, SO_RCVTIMEO, 3);
		play_akis(s, a1 ? atof(a1) : 60);
	} else if (!strcmp(kmt, "RTSP_DURUM")) {
		rtsp_durum_kmt(s);
	} else if (!strcmp(kmt, "RTSP") && a1) {
		int ac = a1[0] == '1';
		if (ac && dosya_var(kalkan_yol)) { hata(s, "kalkan acik"); return; }
		bayrak_yaz(rtsp_bayrak, ac);
		sync();
		char c[24];
		int m = snprintf(c, sizeof c, "RTSP %c OK\n", ac ? '1' : '0');
		tam_yaz(s, c, (size_t)m);
	} else if (!strcmp(kmt, "RTSP_PAROLA")) {
		rtsp_parola_kmt(s, a1, a2);
	}
}

static int sessiz(const char *k)
{
	static const char *l[] = { "DURUM", "EKRAN", "KAYIT_DURUM", "KAYIT_LISTE", "KAYIT_IDX", "KAYIT_KARE", "KAYIT_OLAYLAR",
		"YETENEK", "RTSP_DURUM", NULL };
	for (int i = 0; l[i]; i++)
		if (!strcmp(k, l[i])) return 1;
	return 0;
}

/* v12: komut satiri; tum satir icin TOPLAM sn (v11 bayt basina 5 sn: 4 yuva bayt bayt tutulabiliyordu). Donen: uzunluk */
static size_t satir_oku(int s, char *satir, size_t n, double sn)
{
	size_t k = 0;
	double son = mono() + sn;
	while (k < n - 1) {
		int ms = (int)((son - mono()) * 1000);
		struct pollfd p = { s, POLLIN, 0 };
		if (ms <= 0 || poll(&p, 1, ms) <= 0) break;
		ssize_t r = recv(s, satir + k, 1, 0);
		if (r <= 0 || satir[k] == '\n') break;
		k++;
	}
	satir[k] = 0;
	return k;
}

static int sunucu(int port)
{
	char anahtar[80];
	if (anahtar_oku(anahtar, sizeof anahtar)) { fprintf(stderr, "anahtar okunamadi\n"); return 1; }
	int l = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
	int bir = 1;
	setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &bir, sizeof bir);
	struct sockaddr_in ad = { .sin_family = AF_INET, .sin_port = htons((uint16_t)port) };
	if (bind(l, (struct sockaddr *)&ad, sizeof ad) || listen(l, 1)) { perror("bind/listen"); return 1; }
	printf("kole-kopru v8: TCP %d dinleniyor\n", port);
	fflush(stdout);
	int cocuk = 0;
	for (;;) {
		while (cocuk > 0 && waitpid(-1, NULL, WNOHANG) > 0)
			cocuk--;
		if (cocuk >= 4) {	/* en fazla 4 eszamanli baglanti */
			struct timespec d = { .tv_nsec = 50000000 };
			nanosleep(&d, NULL);
			continue;
		}
		gunluk_kirp(1);	/* v11: cocuklar ayni dosyaya yazar; buyume baglantilarla olur */
		struct sockaddr_in ia;
		socklen_t il = sizeof ia;
		int s = accept(l, (struct sockaddr *)&ia, &il);
		if (s < 0) { if (errno == EINTR) continue; perror("accept"); sleep(1); continue; }
		if (!izinli_soket(s, NULL)) {	/* v11: anahtar okunmadan once */
			uint32_t ip = ntohl(ia.sin_addr.s_addr);
			printf("RED %u.%u.%u.%u (izinli-ip)\n", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
			fflush(stdout);
			close(s);
			continue;
		}
		fflush(stdout);
		pid_t pid = fork();
		if (pid < 0) { perror("fork"); close(s); sleep(1); continue; }
		if (pid > 0) {	/* ebeveyn: bir sonraki baglantiya */
			cocuk++;
			close(s);
			continue;
		}
		close(l);
		struct timeval to = { .tv_sec = 5 };
		setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
		setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);
		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &bir, sizeof bir);
		char satir[160];
		satir_oku(s, satir, sizeof satir, 5);	/* v12: satirin tamami 5 sn */
		char *k = strtok(satir, " "), *kmt = strtok(NULL, " ");
		char *a1 = strtok(NULL, " "), *a2 = strtok(NULL, " "), *a3 = strtok(NULL, " ");
		uint32_t ip = ntohl(ia.sin_addr.s_addr);
		if (!k || !kmt || !esit(k, anahtar)) {
			printf("RED %u.%u.%u.%u (anahtar)\n", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255);
			fflush(stdout);
			sleep(1);	/* kaba kuvvet yavaslatma (4 cocuk siniriyla birlikte) */
			close(s);
			_exit(0);
		}
		if (!sessiz(kmt))
			printf("istemci %u.%u.%u.%u %s\n", ip >> 24, (ip >> 16) & 255, (ip >> 8) & 255, ip & 255, kmt);
		fflush(stdout);
		komut_isle(s, kmt, a1, a2, a3);
		fflush(stdout);
		close(s);
		_exit(0);
	}
}

/* ---------------- self-test ---------------- */
static int hata_say;
#define DENE(x, ...) do { if (!(x)) { printf("FAIL %s:%d: ", __func__, __LINE__); printf(__VA_ARGS__); puts(""); hata_say++; } } while (0)

/* komutu ayri surecte socketpair uzerinden calistir, yaniti topla */
static size_t istek(const char *kmt, const char *a1, const char *a2, const char *a3, uint8_t *o, size_t on)
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) return 0;
	fflush(stdout);
	pid_t p = fork();
	if (!p) {
		close(sv[0]);
		char b1[128] = "", b2[64] = "", b3[64] = "";
		if (a1) snprintf(b1, sizeof b1, "%s", a1);
		if (a2) snprintf(b2, sizeof b2, "%s", a2);
		if (a3) snprintf(b3, sizeof b3, "%s", a3);
		komut_isle(sv[1], kmt, a1 ? b1 : NULL, a2 ? b2 : NULL, a3 ? b3 : NULL);
		fflush(stdout);
		_exit(0);
	}
	close(sv[1]);
	size_t n = 0;
	ssize_t r;
	while (n < on - 1 && (r = read(sv[0], o + n, on - 1 - n)) > 0) n += (size_t)r;
	o[n] = 0;
	close(sv[0]);
	waitpid(p, NULL, 0);
	return n;
}

static size_t sahte_jpeg(uint8_t *o, size_t govde, uint8_t tohum)
{
	size_t i = 0;
	o[i++] = 0xff; o[i++] = 0xd8;
	o[i++] = 0xff; o[i++] = 0xe1; o[i++] = 0; o[i++] = 6; o[i++] = 0xff; o[i++] = 0xd9; o[i++] = 'x'; o[i++] = tohum;
	o[i++] = 0xff; o[i++] = 0xda; o[i++] = 0; o[i++] = 4; o[i++] = 1; o[i++] = 2;
	for (size_t k = 0; k < govde; k++) {
		uint8_t v = (uint8_t)(k * 13 + tohum);
		o[i++] = v;
		if (v == 0xff) o[i++] = 0;
	}
	o[i++] = 0xff; o[i++] = 0xd9;
	return i;
}

static uint8_t kareler_[6][8000];
static size_t kn_[6];

/* parca dosyasi: verilen kareler; idx=1 ise yan dizin de yazilir */
static unsigned long long parca_yaz_test(const char *ad, const int *kl, int n, int idx, uint32_t *ofs_o)
{
	char yol[600];
	snprintf(yol, sizeof yol, "%s/%.8s", kok, ad);
	mkdir(yol, 0755);
	snprintf(yol, sizeof yol, "%s/%s", kok, ad);
	FILE *f = fopen(yol, "wb");
	FILE *g = NULL;
	if (idx) {
		char iy[600];
		snprintf(iy, sizeof iy, "%s/%.13s.idx", kok, ad);
		g = fopen(iy, "w");
		if (g) fputs("KIDX1\n", g);
	}
	unsigned long long o = 0;
	for (int i = 0; f && i < n; i++) {
		fwrite(kareler_[kl[i]], 1, kn_[kl[i]], f);
		if (ofs_o) ofs_o[i] = (uint32_t)o;
		if (g) fprintf(g, "%llu %zu %d\n", o, kn_[kl[i]], i * 500 + 100);
		o += kn_[kl[i]];
	}
	if (f) fclose(f);
	if (g) { fputs("999999999 10 0\n", g); fclose(g); }	/* bozuk satir: atlanmali */
	return o;
}

static int self_test_kayit(void)
{
	char sab[] = "/tmp/kole-kopru-v8-kayitXXXXXX";
	if (!mkdtemp(sab)) { perror("mkdtemp"); return 1; }
	static char k1[256], k2[256], k3[256];
	snprintf(k1, sizeof k1, "%s/kamera", sab); kok = k1;
	sd_kok = sab;
	snprintf(k2, sizeof k2, "%s/kalkan", sab); kalkan_yol = k2;
	snprintf(k3, sizeof k3, "%s/kamera-aktif", sab); kamera_bayrak = k3;
	mkdir(kok, 0755);
	for (int i = 0; i < 6; i++) kn_[i] = sahte_jpeg(kareler_[i], 1000 + (size_t)i * 900, (uint8_t)(i * 29 + 3));
	int a[4] = { 0, 1, 5, 1 }, b3[3] = { 2, 2, 3 }, c[3] = { 0, 0, 4 }, d1[1] = { 3 };
	uint32_t oa[4], ob[3];
	unsigned long long ba = parca_yaz_test("20261012/2255.mjpg", a, 4, 1, oa);
	unsigned long long bb = parca_yaz_test("20261012/2256.mjpg", b3, 3, 0, ob);
	parca_yaz_test("20261012/2257.mjpg", c, 3, 1, NULL);
	parca_yaz_test("20261013/0101.mjpg", d1, 1, 1, NULL);
	char yol[600];
	snprintf(yol, sizeof yol, "%s/20261012/olay.csv", kok);
	FILE *f = fopen(yol, "w");
	if (f) { fputs("2255,1,4,9,0,0\nbozuk satir\n2255,777,4,123,2,1100\n", f); fclose(f); }
	snprintf(yol, sizeof yol, "%s/20261013/olay.csv", kok);
	f = fopen(yol, "w");
	if (f) { fputs("0101,900,1,55,0,100\n", f); fclose(f); }
	snprintf(yol, sizeof yol, "%s/notlar.txt", kok);
	f = fopen(yol, "w");
	if (f) { fputs("gizli", f); fclose(f); }
	static uint8_t o[1 << 20];
	size_t n;

	/* LISTE: yalniz .mjpg */
	n = istek("KAYIT_LISTE", NULL, NULL, NULL, o, sizeof o);
	DENE(strstr((char *)o, "20261012/2255.mjpg ") && strstr((char *)o, "20261013/0101.mjpg ") && !strstr((char *)o, "idx") &&
	     !strstr((char *)o, "csv") && !strstr((char *)o, "notlar") && n > 4 && !strcmp((char *)o + n - 4, "SON\n"), "liste %s", o);

	/* OKU tam / aralik / ofs */
	n = istek("KAYIT_OKU", "20261012/2255.mjpg", NULL, NULL, o, sizeof o);
	uint32_t L = o[0] | o[1] << 8 | o[2] << 16 | (uint32_t)o[3] << 24;
	DENE(L == ba && n == ba + 4 && !memcmp(o + 4, kareler_[0], kn_[0]), "oku tam L=%u n=%zu", L, n);
	char ofs[24], len[24];
	snprintf(ofs, sizeof ofs, "%u", oa[2]);
	snprintf(len, sizeof len, "%zu", kn_[5]);
	n = istek("KAYIT_OKU", "20261012/2255.mjpg", ofs, len, o, sizeof o);
	L = o[0] | o[1] << 8 | o[2] << 16 | (uint32_t)o[3] << 24;
	DENE(L == kn_[5] && n == kn_[5] + 4 && !memcmp(o + 4, kareler_[5], kn_[5]), "oku aralik L=%u", L);
	n = istek("KAYIT_OKU", "20261012/2255.mjpg", ofs, NULL, o, sizeof o);
	L = o[0] | o[1] << 8 | o[2] << 16 | (uint32_t)o[3] << 24;
	DENE(L == ba - oa[2] && n == L + 4, "oku ofs->son");
	n = istek("KAYIT_OKU", "20261012/2255.mjpg", ofs, "0", o, sizeof o);
	DENE(n == ba - oa[2] + 4, "len 0 = sona kadar");
	snprintf(ofs, sizeof ofs, "%llu", ba + 1);
	istek("KAYIT_OKU", "20261012/2255.mjpg", ofs, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA aralik\n"), "ofs > boyut");
	istek("KAYIT_OKU", "20261012/2255.mjpg", "12a", NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA aralik\n"), "ofs rakam degil");
	istek("KAYIT_OKU", "20261012/2255.mjpg", "-1", NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA aralik\n"), "ofs negatif");
	const char *kotu[] = { "../velinux/autorun.sh", "20261012/2255.mjpg.", "20261312/2255.mjpg", "20261012/2460.mjpg",
		"20261012\\2255.mjpg", "2026101/22555.mjpg", "20261012/../../x.mjp", "20261000/2255.mjpg", "x", NULL };
	for (int i = 0; kotu[i]; i++) {
		istek("KAYIT_OKU", kotu[i], NULL, NULL, o, sizeof o);
		DENE(!strcmp((char *)o, "HATA ad\n"), "kotu ad %s", kotu[i]);
		istek("KAYIT_IDX", kotu[i], NULL, NULL, o, sizeof o);
		DENE(!strcmp((char *)o, "HATA ad\n"), "idx kotu ad %s", kotu[i]);
		istek("KAYIT_KARE", kotu[i], "t", NULL, o, sizeof o);
		DENE(!strcmp((char *)o, "HATA ad\n"), "kare kotu ad %s", kotu[i]);
	}
	istek("KAYIT_OKU", "20261012/2259.mjpg", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA yok\n"), "olmayan dosya");

	/* IDX: yan dizin ve tarama */
	istek("KAYIT_IDX", "20261012/2255.mjpg", NULL, NULL, o, sizeof o);
	char bek[200];
	snprintf(bek, sizeof bek, "IDX 4 %llu idx\n0 %zu 100\n", ba, kn_[0]);
	DENE(!strncmp((char *)o, bek, strlen(bek)) && strstr((char *)o, "SON\n") && !strstr((char *)o, "999999999"), "idx %s", o);
	istek("KAYIT_IDX", "20261012/2256.mjpg", NULL, NULL, o, sizeof o);
	snprintf(bek, sizeof bek, "IDX 3 %llu tarama\n0 %zu -1\n%u %zu -1\n%u %zu -1\nSON\n", bb, kn_[2], ob[1], kn_[2], ob[2], kn_[3]);
	DENE(!strcmp((char *)o, bek), "tarama idx '%s' beklenen '%s'", o, bek);

	/* KARE */
	n = istek("KAYIT_KARE", "20261012/2255.mjpg", "1", NULL, o, sizeof o);
	DENE(n == kn_[1] + 4 && !memcmp(o + 4, kareler_[1], kn_[1]), "kare 1");
	n = istek("KAYIT_KARE", "20261012/2256.mjpg", "2", NULL, o, sizeof o);
	DENE(n == kn_[3] + 4 && !memcmp(o + 4, kareler_[3], kn_[3]), "taranan kare 2");
	n = istek("KAYIT_KARE", "20261012/2255.mjpg", "t", NULL, o, sizeof o);
	DENE(n == kn_[5] + 4 && !memcmp(o + 4, kareler_[5], kn_[5]), "temsilci = en buyuk sicrama (kare 2)");
	istek("KAYIT_KARE", "20261012/2255.mjpg", "9", NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA kare\n"), "kare sinir disi");
	istek("KAYIT_KARE", "20261012/2255.mjpg", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA kare\n"), "kare secimi yok");

	/* OLAYLAR: csv (son satir), idx'ten hesaplanan; taranacak (idx'siz) parca puansiz */
	istek("KAYIT_OLAYLAR", "20261012", "0", NULL, o, sizeof o);
	uint32_t lc[3] = { (uint32_t)kn_[0], (uint32_t)kn_[0], (uint32_t)kn_[4] };
	int tp;
	int sk = olay_skor(lc, 3, &tp);
	snprintf(bek, sizeof bek, "20261012/2255.mjpg 777 4 123 2 1100\n20261012/2257.mjpg %d 3 %zu %d %d\nSON\n", sk,
		kn_[0] * 2 + kn_[4], tp, tp * 500 + 100);
	DENE(!strcmp((char *)o, bek), "olaylar '%s' beklenen '%s'", o, bek);
	istek("KAYIT_OLAYLAR", "*", "800", NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "20261013/0101.mjpg 900 1 55 0 100\nSON\n"), "olaylar * esik 800: %s", o);
	istek("KAYIT_OLAYLAR", "../x", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA gun\n"), "olaylar kotu gun");
	istek("KAYIT_OLAYLAR", "20261012", "5000", NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA esik\n"), "olaylar kotu esik");
	istek("KAYIT_OLAYLAR", NULL, NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA gun\n"), "olaylar gun yok");

	/* kalkan */
	f = fopen(kalkan_yol, "w");
	if (f) fclose(f);
	istek("KAMERA", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA kalkan acik\n") && !dosya_var(kamera_bayrak), "kalkan KAMERA 1");
	istek("KAYIT", "1", NULL, NULL, o, sizeof o);
	snprintf(yol, sizeof yol, "%s/kayit-acik", kok);
	DENE(!strcmp((char *)o, "HATA kalkan acik\n") && !dosya_var(yol), "kalkan KAYIT 1");
	istek("KAMERA_AKIS", "5", "5", NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA kalkan acik\n"), "kalkan KAMERA_AKIS");
	istek("KAMERA", "0", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "KAMERA 0 OK\n"), "kalkan varken kapatma serbest");
	unlink(kalkan_yol);
	istek("KAMERA", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "KAMERA 1 OK\n") && dosya_var(kamera_bayrak), "kalkansiz KAMERA 1");
	istek("KAYIT", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "KAYIT 1 OK\n") && dosya_var(yol), "kalkansiz KAYIT 1");
	istek("KAYIT", "0", NULL, NULL, o, sizeof o);
	DENE(!dosya_var(yol), "KAYIT 0");

	/* YETENEK + DURUM */
	istek("YETENEK", NULL, NULL, NULL, o, sizeof o);
	DENE(!strncmp((char *)o, "YETENEK 8 KAYIT_OKU_ARALIK", 26), "yetenek %s", o);
	istek("DURUM", NULL, NULL, NULL, o, sizeof o);
	DENE(strstr((char *)o, "\"surum\":\"kole-kopru-v8\"") && strstr((char *)o, "\"kamera_saglik\":{") &&
	     strstr((char *)o, "\"kalkan\":false") && strstr((char *)o, "\"kamera_aktif\":true"), "durum %s", o);

	char sil[400];
	snprintf(sil, sizeof sil, "rm -rf '%s'", sab);
	if (system(sil)) puts("uyari: gecici klasor silinemedi");
	return hata_say;
}

/* ---------------- v8 self-test: ses, RTSP saf yardimcilar, RTSP oturumu (socketpair; ag/USB/ses aygiti yok) -------- */
static char T[256];	/* v8 testlerinin gecici klasoru */

static void yol_ata(const char **hedef, char *depo, size_t n, const char *ad)
{
	snprintf(depo, n, "%s/%s", T, ad);
	*hedef = depo;
}

/* komutu ayri surecte calistir; once gonder[] yazilir, sonra yanit toplanir */
static size_t istek_gonder(const char *kmt, const char *a1, const uint8_t *g, size_t gn, uint8_t *o, size_t on)
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) return 0;
	fflush(stdout);
	pid_t p = fork();
	if (!p) {
		close(sv[0]);
		char b1[64] = "";
		if (a1) snprintf(b1, sizeof b1, "%s", a1);
		komut_isle(sv[1], kmt, a1 ? b1 : NULL, NULL, NULL);
		fflush(stdout);
		_exit(0);
	}
	close(sv[1]);
	if (gn) tam_yaz(sv[0], g, gn);
	size_t n = 0;
	ssize_t r;
	while (n < on - 1 && (r = read(sv[0], o + n, on - 1 - n)) > 0) n += (size_t)r;
	o[n] = 0;
	close(sv[0]);
	waitpid(p, NULL, 0);
	return n;
}

static size_t cerceve_ekle(uint8_t *o, size_t n, unsigned ornek, int tek_bayt)
{
	unsigned b = ornek * 2 + (tek_bayt ? 1 : 0);
	o[n] = (uint8_t)b;
	o[n + 1] = (uint8_t)(b >> 8);
	for (unsigned i = 0; i < b; i++) o[n + 2 + i] = (uint8_t)(i * 7);
	return n + 2 + b;
}

static int self_test_ses(void)
{
	static uint8_t o[1 << 20], g[65536];
	size_t n;
	/* ses aygiti yok (snd_kok var olmayan klasor) */
	ses_sahte = 0;
	istek("MIC_AKIS", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA mik yok\n"), "aygitsiz MIC_AKIS '%s'", o);
	istek("PLAY_AKIS", "5", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA ses cikisi yok\n"), "aygitsiz PLAY_AKIS '%s'", o);
	n = istek("MIC", NULL, NULL, NULL, o, sizeof o);
	DENE(n == 0, "v7 MIC aygitsiz: yanitsiz kapanir");
	/* kalkan */
	bayrak_yaz(kalkan_yol, 1);
	istek("MIC_AKIS", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA kalkan acik\n"), "kalkan MIC_AKIS");
	istek("PLAY_AKIS", "5", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA kalkan acik\n"), "kalkan PLAY_AKIS");
	istek("RTSP", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA kalkan acik\n") && !dosya_var(rtsp_bayrak), "kalkan RTSP 1");
	bayrak_yaz(kalkan_yol, 0);
	/* sahte ses: MIC_AKIS 0.3 sn -> baslik + ~4800 ornek, bayrak sonra silinir */
	ses_sahte = 1;
	double t = mono();
	n = istek("MIC_AKIS", "0.3", NULL, NULL, o, sizeof o);
	double sure = mono() - t;
	DENE(n > 23 && !memcmp(o, "MIC_AKIS 16000 1 s16le\n", 23), "MIC_AKIS baslik");
	size_t pcm = n - 23;
	DENE(pcm >= 2 * 4000 && pcm <= 2 * 5500 && !(pcm & 1), "MIC_AKIS 0.3 sn ~4800 ornek (%zu bayt)", pcm);
	DENE(sure > 0.25 && sure < 1.5, "MIC_AKIS gercek zamanli (%.2f sn)", sure);
	double e = 0;
	for (size_t i = 23 + 400; i + 1 < n; i += 2) { int16_t v; memcpy(&v, o + i, 2); e += (double)v * v; }
	DENE(sqrt(e / ((pcm - 400) / 2.0)) > 800, "MIC_AKIS sentetik ton duzeyi");
	DENE(!dosya_var(mik_bayrak), "MIC_AKIS bitince /tmp/mik-aktif silinir");
	DENE(!dosya_var(asistan_birak_yol), "v12: MIC_AKIS bitince birak bayragi silinir");
	/* PLAY_AKIS: 3 cerceve x 800 ornek + son */
	size_t gn = 0;
	for (int i = 0; i < 3; i++) gn = cerceve_ekle(g, gn, 800, 0);
	g[gn++] = 0; g[gn++] = 0;
	istek_gonder("PLAY_AKIS", "5", g, gn, o, sizeof o);
	DENE(!strcmp((char *)o, "PLAY_AKIS HAZIR 16000\nOK 2400 0\n"), "PLAY_AKIS akis '%s'", o);
	DENE(!dosya_var(hoparlor_bayrak), "PLAY_AKIS bitince hoparlor bayragi silinir");
	/* kisa konusma (on tampon dolmadan) da biter */
	gn = cerceve_ekle(g, 0, 300, 0);
	g[gn++] = 0; g[gn++] = 0;
	istek_gonder("PLAY_AKIS", NULL, g, gn, o, sizeof o);
	DENE(!strcmp((char *)o, "PLAY_AKIS HAZIR 16000\nOK 300 0\n"), "kisa konusma '%s'", o);
	/* tek baytli cerceve -> HATA cerceve */
	gn = cerceve_ekle(g, 0, 100, 1);
	istek_gonder("PLAY_AKIS", "5", g, gn, o, sizeof o);
	DENE(strstr((char *)o, "HATA cerceve\n") != NULL, "tek bayt cerceve '%s'", o);
	/* sure siniri: 0.1 sn = 1600 ornek; 2400 gonder */
	gn = 0;
	for (int i = 0; i < 3; i++) gn = cerceve_ekle(g, gn, 800, 0);
	g[gn++] = 0; g[gn++] = 0;
	istek_gonder("PLAY_AKIS", "0.1", g, gn, o, sizeof o);
	DENE(strstr((char *)o, "HATA sure\n") != NULL, "sure siniri '%s'", o);
	/* istemci erken kapatir (son cercevesi yok) -> yine OK ile biter */
	gn = cerceve_ekle(g, 0, 800, 0);
	istek_gonder("PLAY_AKIS", "5", g, gn, o, sizeof o);
	DENE(strstr((char *)o, "OK 800 0\n") != NULL, "erken kapanma '%s'", o);
	ses_sahte = 0;
	/* RTSP bayrak ve parola komutlari */
	istek("RTSP", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "RTSP 1 OK\n") && dosya_var(rtsp_bayrak), "RTSP 1");
	istek("RTSP_PAROLA", "kole", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA parola bicimi\n"), "RTSP_PAROLA eksik");
	n = istek("RTSP_DURUM", NULL, NULL, NULL, o, sizeof o);
	DENE(strstr((char *)o, "\"bayrak\":true") && strstr((char *)o, "\"parola_dosyasi\":false") && strstr((char *)o, "\"durum\":null"),
	     "RTSP_DURUM %s", o);
	istek("RTSP", "0", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "RTSP 0 OK\n") && !dosya_var(rtsp_bayrak), "RTSP 0");
	return hata_say;
}

/* sentetik baseline JPEG: tip 0 (2x1) / 1 (2x2), dri, sof (0xC0/0xC2), DHT bozuk mu, Cb ornekleme */
static size_t ornek_jpeg(uint8_t *o, int tip, unsigned dri, int sof, int dht_bozuk, int cb_hv, size_t tarama, uint32_t tohum)
{
	size_t i = 0;
	o[i++] = 0xff; o[i++] = 0xd8;
	static const uint8_t app0[] = { 0xff, 0xe0, 0, 16, 'J', 'F', 'I', 'F', 0, 1, 1, 0, 0, 1, 0, 1, 0, 0 };
	memcpy(o + i, app0, sizeof app0); i += sizeof app0;
	o[i++] = 0xff; o[i++] = 0xdb; o[i++] = 0; o[i++] = 132;
	o[i++] = 0x00; for (int k = 0; k < 64; k++) o[i++] = (uint8_t)(k + 1);
	o[i++] = 0x01; for (int k = 0; k < 64; k++) o[i++] = (uint8_t)(64 - k);
	o[i++] = 0xff; o[i++] = (uint8_t)sof; o[i++] = 0; o[i++] = 17; o[i++] = 8;
	o[i++] = 720 >> 8; o[i++] = 720 & 255; o[i++] = 1280 >> 8; o[i++] = 1280 & 255; o[i++] = 3;
	o[i++] = 1; o[i++] = tip ? 0x22 : 0x21; o[i++] = 0;
	o[i++] = 2; o[i++] = (uint8_t)cb_hv; o[i++] = 1;
	o[i++] = 3; o[i++] = 0x11; o[i++] = 1;
	o[i++] = 0xff; o[i++] = 0xc4; o[i++] = 418 >> 8; o[i++] = 418 & 255;
	o[i++] = 0x00; memcpy(o + i, HUF_DC_Y_BITS, 16); i += 16; memcpy(o + i, HUF_DC_VAL, 12); i += 12;
	o[i++] = 0x01; memcpy(o + i, HUF_DC_C_BITS, 16); i += 16; memcpy(o + i, HUF_DC_VAL, 12); i += 12;
	o[i++] = 0x10; memcpy(o + i, HUF_AC_Y_BITS, 16); i += 16; memcpy(o + i, HUF_AC_Y_VAL, 162); i += 162;
	o[i++] = 0x11; memcpy(o + i, HUF_AC_C_BITS, 16); i += 16; memcpy(o + i, HUF_AC_C_VAL, 162);
	if (dht_bozuk) o[i + 5] ^= 1;
	i += 162;
	if (dri) { o[i++] = 0xff; o[i++] = 0xdd; o[i++] = 0; o[i++] = 4; o[i++] = (uint8_t)(dri >> 8); o[i++] = (uint8_t)dri; }
	static const uint8_t sos[] = { 0xff, 0xda, 0, 12, 3, 1, 0x00, 2, 0x11, 3, 0x11, 0, 63, 0 };
	memcpy(o + i, sos, sizeof sos); i += sizeof sos;
	uint32_t x = tohum | 1;
	for (size_t k = 0; k < tarama; k++) {
		x ^= x << 13; x ^= x >> 17; x ^= x << 5;
		uint8_t v = (uint8_t)x;
		if (k % 97 == 0) v = 0xff;		/* bol FF -> FF 00 doldurma */
		o[i++] = v;
		if (v == 0xff) o[i++] = 0;
		if (dri && k && k % 700 == 0) { o[i++] = 0xff; o[i++] = (uint8_t)(0xd0 + (k / 700) % 8); }
	}
	o[i++] = 0xff; o[i++] = 0xd9;
	return i;
}

struct toplayici { uint8_t p[512][RTP_AZAMI]; size_t n[512]; int sayi; };
static int topla_cb(void *v, const uint8_t *p, size_t n)
{
	struct toplayici *t = v;
	if (t->sayi >= 512) return -1;
	memcpy(t->p[t->sayi], p, n);
	t->n[t->sayi++] = n;
	return 0;
}

/* RFC 2435 alicisi gibi coz: parcalari birlestir, basliklari denetle. 0 ok */
static int rtp_jpeg_coz(const struct toplayici *t, const struct rjpeg *r, size_t mtu, uint8_t *birlesik, size_t *bn)
{
	size_t ofs = 0;
	for (int i = 0; i < t->sayi; i++) {
		const uint8_t *p = t->p[i];
		size_t n = t->n[i];
		if (n > mtu || p[0] != 0x80 || (p[1] & 0x7f) != 26) return 1;
		if (((p[1] & 0x80) != 0) != (i == t->sayi - 1)) return 2;	/* isaret yalniz son pakette */
		if (i && be16(p + 2) != (uint16_t)(be16(t->p[i - 1] + 2) + 1)) return 3;
		if (memcmp(p + 4, t->p[0] + 4, 8)) return 4;			/* ts + ssrc ayni */
		const uint8_t *J = p + 12;
		size_t fo = (size_t)J[1] << 16 | (size_t)J[2] << 8 | J[3];
		if (fo != ofs || J[5] != 255 || J[6] != r->g8 || J[7] != r->y8) return 5;
		if (J[4] != r->tip + (r->dri ? 64 : 0)) return 6;
		size_t h = 20;
		if (r->dri) {
			if (be16(p + h) != r->dri || be16(p + h + 2) != 0xffff) return 7;
			h += 4;
		}
		if (!fo) {
			if (p[h] || p[h + 1] || be16(p + h + 2) != 128 || memcmp(p + h + 4, r->q, 128)) return 8;
			h += 132;
		}
		memcpy(birlesik + ofs, p + h, n - h);
		ofs += n - h;
	}
	*bn = ofs;
	return 0;
}

static int self_test_rtsp_saf(void)
{
	/* MD5 (RFC 1321 Ek A.5) */
	struct { const char *g, *h; } md[] = { { "", "d41d8cd98f00b204e9800998ecf8427e" }, { "abc", "900150983cd24fb0d6963f7d28e17f72" },
		{ "The quick brown fox jumps over the lazy dog", "9e107d9d372bb6826bd81d3542a419d6" },
		{ "12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a" } };
	for (size_t i = 0; i < 4; i++) {
		char h[33];
		const char *p[] = { md[i].g, NULL };
		md5_hex_parcalar(h, p);
		DENE(!strcmp(h, md[i].h), "md5 '%s' -> %s", md[i].g, h);
	}
	/* RFC 2617 3.5 ornegi (qop=auth) */
	struct ozet o;
	const char *ah = "Digest username=\"Mufasa\", realm=\"testrealm@host.com\", nonce=\"dcd98b7102dd2f0e8b11d0f600bfb0c093\", "
			 "uri=\"/dir/index.html\", qop=auth, nc=00000001, cnonce=\"0a4f113b\", "
			 "response=\"6629fae49393a05397450978507c4ef1\", opaque=\"5ccc069c403ebaf9f0171e9517f40e41\"";
	DENE(!ozet_ayristir(ah, &o), "rfc2617 ayristir");
	char ha1[33], bek[33];
	const char *p1[] = { "Mufasa", "testrealm@host.com", "Circle Of Life", NULL };
	md5_hex_parcalar(ha1, p1);
	ozet_beklenen(&o, "GET", ha1, bek);
	DENE(!strcmp(bek, "6629fae49393a05397450978507c4ef1"), "rfc2617 yanit %s", bek);
	DENE(!ozet_dogrula(&o, "GET", "Mufasa", ha1), "realm farkli -> red");
	const char *kotu[] = { "Basic dXNlcjpwYXNz", "Digest username=\"a\"", "Digest username=\"a\", nonce=\"n\", uri=\"/\", response=\"kisa\"",
		"Digest username=\"a\", nonce=\"n\", uri=\"/\", response=\"6629fae49393a05397450978507c4ef1\", algorithm=SHA-256",
		"Digest username=\"a, nonce=\"n\"", "Digest username=\"a\", nonce=\"n\", uri=\"/\", qop=auth, "
		"response=\"6629fae49393a05397450978507c4ef1\"", NULL };
	for (int i = 0; kotu[i]; i++) DENE(ozet_ayristir(kotu[i], &o), "kotu digest %d kabul edildi", i);
	/* mu-law (Sun g711 / audioop ile ayni; tam tablo 134'te audioop ile karsilastirilir) */
	DENE(ulaw_kodla(0) == 0xff && ulaw_kodla(32767) == 0x80 && ulaw_kodla(-32768) == 0x00 && ulaw_kodla(-1) == 0x7e,
	     "ulaw sabitleri");
	int tek = 1;
	for (int v = -32768; v <= 32767; v += 7) {
		int c = ulaw_coz(ulaw_kodla((int16_t)v));
		int hata_ = abs(c - v);
		if (hata_ > abs(v) / 15 + 40) { tek = 0; break; }
	}
	DENE(tek, "ulaw gidis-donus hatasi sinirda");
	/* 16k -> 8k: 1 kHz gecer, 6 kHz bastirilir */
	az8_hazirla();
	for (int f = 0; f < 2; f++) {
		static int16_t in[3200];
		static uint8_t u[1600];
		for (int i = 0; i < 3200; i++) in[i] = (int16_t)(10000 * sin(2 * M_PI * (f ? 6000.0 : 1000.0) * i / 16000));
		struct az8 a = {0};
		size_t k = az8_ulaw(&a, in, 3200, u);
		double e = 0;
		for (size_t i = 100; i < k; i++) { double v = ulaw_coz(u[i]); e += v * v; }
		double rms = sqrt(e / (double)(k - 100));
		DENE(k == 1600 && ((f == 0 && fabs(rms - 7071) < 600) || (f == 1 && rms < 500)), "az8 %d Hz rms=%.0f", f ? 6000 : 1000, rms);
	}
	/* RTSP ayristirici */
	struct rtsp_istek r;
	const char *ok1 = "DESCRIBE rtsp://10.0.0.5:8554/kamera RTSP/1.0\r\nCSeq: 3\r\nAuthorization: Digest username=\"k\"\r\n"
			  "User-Agent: test\r\nSession: ABCD;timeout=60\r\n\r\n";
	DENE(!rtsp_ayristir(ok1, strlen(ok1), &r) && r.cseq == 3 && !strcmp(r.yontem, "DESCRIBE") && !strcmp(r.oturum, "ABCD") &&
	     !strncmp(r.yetki, "Digest", 6), "rtsp ayristir");
	char yol[64];
	url_yol(r.url, yol, sizeof yol);
	DENE(!strcmp(yol, "/kamera"), "url yol %s", yol);
	url_yol("rtsp://h/video/trackID=0?x=1", yol, sizeof yol);
	DENE(!strcmp(yol, "/video/trackID=0"), "url yol 2 %s", yol);
	url_yol("rtsp://h:8554", yol, sizeof yol);
	DENE(!strcmp(yol, "/"), "url yol bos");
	const char *bozuk[] = { "OPTIONS * RTSP/1.0\r\n\r\n", "OPTIONS * HTTP/1.1\r\nCSeq: 1\r\n\r\n", "options * RTSP/1.0\r\nCSeq: 1\r\n\r\n",
		"OPTIONS * RTSP/1.0\r\nCSeq: x\r\n\r\n", "OPTIONS * RTSP/1.0\r\nCSeq: 1\r\nContent-Length: 99999\r\n\r\n",
		"OPTIONS * RTSP/1.0\r\nCSeq: 1\r\nbaslik-sutunsuz\r\n\r\n", NULL };
	for (int i = 0; bozuk[i]; i++) DENE(rtsp_ayristir(bozuk[i], strlen(bozuk[i]), &r), "bozuk istek %d kabul", i);
	static char uzun[2000];
	snprintf(uzun, sizeof uzun, "OPTIONS * RTSP/1.0\r\nCSeq: 1\r\nX: %01500d\r\n\r\n", 0);
	DENE(rtsp_ayristir(uzun, strlen(uzun), &r), "uzun satir");
	int a, b;
	DENE(tasima_tcp("RTP/AVP/TCP;unicast;interleaved=4-5", &a, &b) && a == 4 && b == 5, "tasima 4-5");
	DENE(tasima_tcp("RTP/AVP/TCP;unicast", &a, &b) && a == -1, "tasima atanacak");
	DENE(!tasima_tcp("RTP/AVP;unicast;client_port=5000-5001", &a, &b), "UDP tasima");
	/* RTP/JPEG paketleyici: sentetik baseline JPEG'ler */
	static uint8_t j[300000], birlesik[300000];
	static struct toplayici t;
	struct { int tip; unsigned dri; size_t mtu; } dz[] = { { 0, 0, 1400 }, { 1, 0, 400 }, { 0, 16, 1400 }, { 1, 80, 300 } };
	for (size_t i = 0; i < 4; i++) {
		size_t n = ornek_jpeg(j, dz[i].tip, dz[i].dri, 0xc0, 0, 0x11, 60000, (uint32_t)(i + 7));
		struct rjpeg rj;
		const char *e = rjpeg_ayristir(j, n, &rj);
		DENE(!e, "rjpeg %zu: %s", i, e ? e : "");
		if (e) continue;
		DENE(rj.tip == dz[i].tip && rj.dri == dz[i].dri && rj.g8 == 160 && rj.y8 == 90 && rj.dht_standart, "rjpeg alanlari %zu", i);
		DENE(rj.q[0] == 1 && rj.q[63] == 64 && rj.q[64] == 64 && rj.q[127] == 1, "Q tablolari %zu", i);
		DENE(rj.tarama + rj.tarama_n == j + n - 2, "tarama EOI'ye kadar %zu", i);
		t.sayi = 0;
		uint16_t seq = 65530;	/* sarma */
		int ps = rtp_jpeg_paketle(&rj, 0x12345678, 0xCAFEBABE, &seq, dz[i].mtu, topla_cb, &t);
		size_t bn = 0;
		int c = rtp_jpeg_coz(&t, &rj, dz[i].mtu, birlesik, &bn);
		DENE(ps > 1 && ps == t.sayi && !c && bn == rj.tarama_n && !memcmp(birlesik, rj.tarama, bn),
		     "paketle/coz %zu: ps=%d c=%d bn=%zu", i, ps, c, bn);
	}
	size_t n = ornek_jpeg(j, 0, 0, 0xc0, 1, 0x11, 5000, 3);
	struct rjpeg rj;
	DENE(!rjpeg_ayristir(j, n, &rj) && !rj.dht_standart, "standart olmayan DHT bildirilir");
	n = ornek_jpeg(j, 0, 0, 0xc2, 0, 0x11, 5000, 3);
	DENE(rjpeg_ayristir(j, n, &rj) != NULL, "progressive reddedilir");
	n = ornek_jpeg(j, 0, 0, 0xc0, 0, 0x21, 5000, 3);
	DENE(rjpeg_ayristir(j, n, &rj) != NULL, "Cb 2x1 reddedilir");
	n = ornek_jpeg(j, 0, 0, 0xc0, 0, 0x11, 5000, 3);
	DENE(rjpeg_ayristir(j, n - 2, &rj) != NULL, "EOI'siz reddedilir");
	j[20 + 4 + 0] = 0x10;	/* DQT: 16 bit */
	DENE(rjpeg_ayristir(j, n, &rj) != NULL, "16 bit DQT reddedilir");
	DENE(rjpeg_ayristir((const uint8_t *)"ABCD", 4, &rj) != NULL, "JPEG degil");
	uint8_t sr[64];
	size_t sn = rtcp_sr(sr, 1, 2, 3, 4, 5);
	DENE(sn == 48 && sr[1] == 200 && be16(sr + 2) == 6 && sr[29] == 202 && be16(sr + 30) == 4, "RTCP SR+SDES");
	return hata_say;
}

/* ------ RTSP oturumu: socketpair istemciler, rtsp_adim ile surulur ------ */
struct rt_ist { int fd; uint8_t b[1 << 20]; size_t n; };
static struct rt_ist RI[3];

static void rt_pompala(double sn)
{
	double t = mono();
	do {
		rtsp_adim(5);
		for (int i = 0; i < 3; i++) {
			if (RI[i].fd < 0) continue;
			for (;;) {
				if (RI[i].n >= sizeof RI[i].b) RI[i].n = 0;	/* test tamponu tasarsa bastan (yalniz sayilar korunur) */
				ssize_t r = recv(RI[i].fd, RI[i].b + RI[i].n, sizeof RI[i].b - RI[i].n, MSG_DONTWAIT);
				if (r <= 0) break;
				RI[i].n += (size_t)r;
			}
		}
	} while (mono() - t < sn);
}

/* tampondaki ilk RTSP yanitini al (onceki interleaved cerceveleri atla); bulunamazsa 0 */
static int rt_yanit(struct rt_ist *c, char *o, size_t on, double bekle)
{
	double t = mono();
	for (;;) {
		size_t i = 0;
		while (i + 4 <= c->n && c->b[i] == '$') i += 4 + be16(c->b + i + 2);
		if (i < c->n) {
			char *e = memmem(c->b + i, c->n - i, "\r\n\r\n", 4);
			if (e) {
				size_t hn = (size_t)(e - (char *)c->b) + 4 - i;
				char *cl = memmem(c->b + i, hn, "Content-Length: ", 16);
				size_t gl = cl ? (size_t)atoi(cl + 16) : 0;
				if (c->n >= i + hn + gl) {
					size_t k = hn + gl < on - 1 ? hn + gl : on - 1;
					memcpy(o, c->b + i, k);
					o[k] = 0;
					memmove(c->b + i, c->b + i + hn + gl, c->n - i - hn - gl);
					c->n -= hn + gl;
					return 1;
				}
			}
		}
		if (mono() - t > bekle) { o[0] = 0; return 0; }
		rt_pompala(0.01);
	}
}

static int rt_istek(int k, const char *m, char *o, size_t on, double bekle)
{
	tam_yaz(RI[k].fd, m, strlen(m));
	return rt_yanit(&RI[k], o, on, bekle);
}

static void yetki_yaz(char *o, size_t n, const char *yontem, const char *uri, const char *nonce, const char *parola, int qop)
{
	struct ozet z;
	memset(&z, 0, sizeof z);
	snprintf(z.kullanici, sizeof z.kullanici, "kole");
	snprintf(z.realm, sizeof z.realm, RTSP_REALM);
	snprintf(z.nonce, sizeof z.nonce, "%s", nonce);
	snprintf(z.uri, sizeof z.uri, "%s", uri);
	if (qop) { strcpy(z.qop, "auth"); strcpy(z.nc, "00000001"); strcpy(z.cnonce, "abcdef12"); }
	char ha1[33], y[33];
	const char *p[] = { "kole", RTSP_REALM, parola, NULL };
	md5_hex_parcalar(ha1, p);
	ozet_beklenen(&z, yontem, ha1, y);
	if (qop)
		snprintf(o, n, "Authorization: Digest username=\"kole\", realm=\"" RTSP_REALM "\", nonce=\"%s\", uri=\"%s\", "
			 "qop=auth, nc=00000001, cnonce=\"abcdef12\", response=\"%s\"\r\n", nonce, uri, y);
	else
		snprintf(o, n, "Authorization: Digest username=\"kole\", realm=\"" RTSP_REALM "\", nonce=\"%s\", uri=\"%s\", "
			 "response=\"%s\", algorithm=MD5\r\n", nonce, uri, y);
}

static void nonce_al(const char *yanit, char *o)
{
	const char *p = strstr(yanit, "nonce=\"");
	o[0] = 0;
	if (p) snprintf(o, 33, "%.32s", p + 7);
}

/* bir istemciyi PLAY'e kadar goturur; 0: ok */
static int rt_oynat(int k, const char *parola, int qop, int ses, char *son)
{
	char y[4096], ya[600], m[1600], nonce[40];
	rt_istek(k, "DESCRIBE rtsp://127.0.0.1/kamera RTSP/1.0\r\nCSeq: 1\r\n\r\n", y, sizeof y, 2);
	if (strncmp(y, "RTSP/1.0 401", 12)) return 1;
	nonce_al(y, nonce);
	yetki_yaz(ya, sizeof ya, "DESCRIBE", "rtsp://127.0.0.1/kamera", nonce, parola, qop);
	snprintf(m, sizeof m, "DESCRIBE rtsp://127.0.0.1/kamera RTSP/1.0\r\nCSeq: 2\r\n%s\r\n", ya);
	rt_istek(k, m, y, sizeof y, 3);
	if (strncmp(y, "RTSP/1.0 200", 12)) { snprintf(son, 200, "%s", y); return 2; }
	snprintf(m, sizeof m, "SETUP rtsp://127.0.0.1/kamera/trackID=0 RTSP/1.0\r\nCSeq: 3\r\n%sTransport: RTP/AVP/TCP;unicast;interleaved=0-1\r\n\r\n", ya);
	rt_istek(k, m, y, sizeof y, 2);
	if (strncmp(y, "RTSP/1.0 200", 12)) return 3;
	char *se = strstr(y, "Session: ");
	char oturum[40] = "";
	if (se) sscanf(se + 9, "%32[0-9A-F]", oturum);
	if (ses) {
		snprintf(m, sizeof m, "SETUP rtsp://127.0.0.1/kamera/trackID=1 RTSP/1.0\r\nCSeq: 4\r\n%sSession: %s\r\n"
			 "Transport: RTP/AVP/TCP;unicast;interleaved=2-3\r\n\r\n", ya, oturum);
		rt_istek(k, m, y, sizeof y, 2);
		if (strncmp(y, "RTSP/1.0 200", 12) || !strstr(y, "interleaved=2-3")) return 4;
	}
	snprintf(m, sizeof m, "PLAY rtsp://127.0.0.1/kamera/ RTSP/1.0\r\nCSeq: 5\r\n%sSession: %s\r\nRange: npt=0.000-\r\n\r\n", ya, oturum);
	rt_istek(k, m, y, sizeof y, 2);
	snprintf(son, 200, "%s", y);
	return strncmp(y, "RTSP/1.0 200", 12) ? 5 : 0;
}

static int self_test_rtsp_oturum(const char *jpeg_yol, const uint8_t *jpeg, size_t jn)
{
	static char y[8192], son[256], m[1024];
	char pyol[300];
	snprintf(pyol, sizeof pyol, "%s/rtsp-parola", T);
	FILE *f = fopen(pyol, "w");
	if (f) { fputs("kole:dogru-parola-uzun-1\n", f); fclose(f); }	/* duz bicim */
	rtsp_parola_yol = strdup(pyol);
	rtsp_her_zaman_acik = 1;
	ses_sahte = 1;
	kamera_sahte_jpeg = jpeg_yol;
	rtsp_hazirla(0, 1);
	for (int i = 0; i < 3; i++) {
		int sv[2];
		socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
		RI[i].fd = sv[1];
		RI[i].n = 0;
		int fl = fcntl(sv[1], F_GETFL);
		fcntl(sv[1], F_SETFL, fl | O_NONBLOCK);
		rtsp_istemci_ekle(sv[0], 0x7f000001);
	}
	rt_pompala(0.05);
	rt_istek(0, "OPTIONS rtsp://127.0.0.1/kamera RTSP/1.0\r\nCSeq: 1\r\n\r\n", y, sizeof y, 2);
	DENE(!strncmp(y, "RTSP/1.0 200", 12) && strstr(y, "Public: OPTIONS, DESCRIBE"), "OPTIONS yetkisiz serbest: %s", y);
	/* yanlis parola: 401 en az 1 sn gecikir, 3 hata baglantiyi kapatir (istemci 2) */
	rt_istek(2, "DESCRIBE rtsp://127.0.0.1/kamera RTSP/1.0\r\nCSeq: 1\r\n\r\n", y, sizeof y, 2);
	char nonce[40], ya[600];
	nonce_al(y, nonce);
	DENE(!strncmp(y, "RTSP/1.0 401", 12) && strlen(nonce) == 32 && strstr(y, "realm=\"VeLinux-KOLE\""), "401 meydan okuma");
	for (int i = 0; i < 3; i++) {
		yetki_yaz(ya, sizeof ya, "DESCRIBE", "rtsp://127.0.0.1/kamera", nonce, "yanlis-parola-000", 0);
		snprintf(m, sizeof m, "DESCRIBE rtsp://127.0.0.1/kamera RTSP/1.0\r\nCSeq: %d\r\n%s\r\n", i + 2, ya);
		double t = mono();
		rt_istek(2, m, y, sizeof y, 3);
		DENE(!strncmp(y, "RTSP/1.0 401", 12) && mono() - t >= 0.95, "yanlis parola %d: 401 + 1 sn (%.2f)", i, mono() - t);
	}
	rt_pompala(0.1);
	DENE(R_ist[2].fd < 0 && R.red == 3, "3 hatali deneme baglantiyi kapatir (red=%lu)", R.red);
	/* bozuk nonce reddedilir */
	yetki_yaz(ya, sizeof ya, "DESCRIBE", "rtsp://127.0.0.1/kamera", "00000000aaaaaaaaaaaaaaaaaaaaaaaa", "dogru-parola-uzun-1", 0);
	snprintf(m, sizeof m, "DESCRIBE rtsp://127.0.0.1/kamera RTSP/1.0\r\nCSeq: 7\r\n%s\r\n", ya);
	rt_istek(1, m, y, sizeof y, 3);
	DENE(!strncmp(y, "RTSP/1.0 401", 12), "sahte nonce reddedilir");
	/* istemci 0: tam akis (qopsuz), SDP */
	int rc = rt_oynat(0, "dogru-parola-uzun-1", 0, 1, son);
	DENE(!rc && strstr(son, "RTP-Info: url=rtsp://127.0.0.1/kamera/trackID=0;seq="), "oynat 0 rc=%d %s", rc, son);
	rt_pompala(1.6);
	/* interleaved cerceveleri ayikla */
	struct rt_ist *c = &RI[0];
	int v_paket = 0, a_paket = 0, sr_v = 0, sr_a = 0, tam_kare = 0, kare_ok = 0;
	static uint8_t kare[300000];
	size_t kn = 0, i = 0;
	struct rjpeg ref;
	rjpeg_ayristir(jpeg, jn, &ref);
	double ae = 0;
	int an = 0;
	while (i + 4 <= c->n && c->b[i] == '$') {
		size_t L = be16(c->b + i + 2);
		if (i + 4 + L > c->n) break;
		const uint8_t *p = c->b + i + 4;
		int ch = c->b[i + 1];
		if (ch == 0) {
			v_paket++;
			const uint8_t *J = p + 12;
			size_t fo = (size_t)J[1] << 16 | (size_t)J[2] << 8 | J[3], h = 20 + (fo ? 0 : 132);
			if (!fo) kn = 0;
			if (fo == kn && kn + L - h <= sizeof kare) { memcpy(kare + kn, p + h, L - h); kn += L - h; }
			if (p[1] & 0x80) { tam_kare++; if (kn == ref.tarama_n && !memcmp(kare, ref.tarama, kn)) kare_ok++; kn = 0; }
		} else if (ch == 2) {
			a_paket++;
			if (L == 172 && (p[1] & 0x7f) == 0)
				for (int k = 0; k < 160; k++) { double v = ulaw_coz(p[12 + k]); ae += v * v; an++; }
		} else if (ch == 1 && p[1] == 200) sr_v++;
		else if (ch == 3 && p[1] == 200) sr_a++;
		i += 4 + L;
	}
	DENE(tam_kare >= 4 && kare_ok == tam_kare, "RTP/JPEG kareler %d/%d (5 fps, 1.6 sn)", kare_ok, tam_kare);
	DENE(a_paket >= 50 && a_paket <= 110, "PCMU paket sayisi %d (~80)", a_paket);
	DENE(an && sqrt(ae / an) > 300, "ses duzeyi (sentetik ton) rms=%.0f", an ? sqrt(ae / an) : 0);
	DENE(sr_v >= 1 && sr_a >= 1, "RTCP SR video %d ses %d", sr_v, sr_a);
	DENE(v_paket > tam_kare, "kare basina birden cok paket");
	DENE(bizim_bayrak(kamera_bayrak) && dosya_var(mik_bayrak) && dosya_var(rtsp_aktif_yol), "oynatma sirasinda gizlilik bayraklari");
	{	/* v12: ses cocugu birak bayragini "kopru <kendi pid>" ile tutar */
		char bb[48], bek[48];
		snprintf(bek, sizeof bek, "kopru %d", (int)R.spid);
		DENE(dosya_oku(asistan_birak_yol, bb, sizeof bb) > 0 && !strcmp(bb, bek), "v12: RTSP ses cocugu birak bayragi '%s' (%s)", bb, bek);
	}
	c->n = 0;
	/* istemci 1: qop=auth, yalniz goruntu */
	rc = rt_oynat(1, "dogru-parola-uzun-1", 1, 0, son);
	DENE(!rc, "oynat 1 (qop) rc=%d %s", rc, son);
	DENE(oynatan_sayi() == 2, "iki izleyici");
	/* ucuncu izleyici: yeni baglanti -> 453 */
	int sv[2];
	socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
	RI[2].fd = sv[1];
	RI[2].n = 0;
	fcntl(sv[1], F_SETFL, fcntl(sv[1], F_GETFL) | O_NONBLOCK);
	rtsp_istemci_ekle(sv[0], 0x0a000002);
	rc = rt_oynat(2, "dogru-parola-uzun-1", 0, 0, son);
	DENE(rc == 5 && !strncmp(son, "RTSP/1.0 453", 12), "ucuncu izleyici 453: rc=%d %s", rc, son);
	/* UDP tasima -> 461 ; bilinmeyen yol -> 404 ; bilinmeyen yontem -> 501 */
	yetki_yaz(ya, sizeof ya, "SETUP", "rtsp://127.0.0.1/kamera/trackID=0", nonce, "x", 0);
	rt_istek(2, "SETUP rtsp://127.0.0.1/kamera/trackID=0 RTSP/1.0\r\nCSeq: 9\r\nTransport: RTP/AVP;unicast;client_port=5000-5001\r\n\r\n",
		 y, sizeof y, 2);
	DENE(!strncmp(y, "RTSP/1.0 461", 12), "UDP 461: %.40s", y);
	rt_istek(2, "DESCRIBE rtsp://127.0.0.1/gizli RTSP/1.0\r\nCSeq: 10\r\n\r\n", y, sizeof y, 2);
	DENE(!strncmp(y, "RTSP/1.0 404", 12), "yol 404: %.40s", y);
	rt_istek(2, "RECORD rtsp://127.0.0.1/kamera RTSP/1.0\r\nCSeq: 11\r\n\r\n", y, sizeof y, 2);
	DENE(!strncmp(y, "RTSP/1.0 501", 12), "RECORD 501: %.40s", y);
	rt_istek(2, "GET / HTTP/1.1\r\nHost: x\r\n\r\n", y, sizeof y, 2);
	DENE(!strncmp(y, "RTSP/1.0 400", 12), "HTTP istegi 400: %.40s", y);
	rt_pompala(0.1);
	DENE(R_ist[2].fd < 0, "bozuk istek baglantiyi kapatir");
	/* durum dosyasi */
	rt_pompala(2.1);
	char d[1500];
	dosya_oku(rtsp_durum_yol, d, sizeof d);
	DENE(strstr(d, "\"oynatan\":2") && strstr(d, "\"video\":\"aktif\"") && strstr(d, "\"ses\":\"aktif\"") && strstr(d, "\"parola\":true"),
	     "rtsp-durum %s", d);
	/* gizlilik kalkani: tum istemciler kapanir, bayraklar temizlenir */
	bayrak_yaz(kalkan_yol, 1);
	rt_pompala(1.2);
	DENE(R_ist[0].fd < 0 && R_ist[1].fd < 0, "kalkan istemcileri kapatir");
	rt_pompala(0.5);
	double t = mono();
	while (mono() - t < 4 && (dosya_var(mik_bayrak) || dosya_var(asistan_birak_yol) || R.olen[0] || R.olen[1])) rt_pompala(0.1);
	DENE(!dosya_var(kamera_bayrak) && !dosya_var(mik_bayrak) && !dosya_var(rtsp_aktif_yol) && !dosya_var(asistan_birak_yol),
	     "kalkan sonrasi bayraklar temiz");
	bayrak_yaz(kalkan_yol, 0);
	/* hub KAMERA 1 ile acilmis bayrak (bos icerik) RTSP'ce silinmez */
	bayrak_yaz(kamera_bayrak, 1);
	DENE(!bizim_bayrak(kamera_bayrak), "baskasinin bayragi bizim sayilmaz");
	bayrak_yaz(kamera_bayrak, 0);
	/* HA1 bicimli parola dosyasi */
	char ha1[33];
	const char *pp[] = { "kole", RTSP_REALM, "ikinci-parola-uzun", NULL };
	md5_hex_parcalar(ha1, pp);
	f = fopen(pyol, "w");
	if (f) { fprintf(f, "kole %s\n", ha1); fclose(f); }
	DENE(parola_yukle() && !strcmp(R.ha1, ha1), "HA1 bicimi");
	f = fopen(pyol, "w");
	if (f) { fputs("kole:kisa\n", f); fclose(f); }
	DENE(!parola_yukle(), "12 karakterden kisa parola reddedilir");
	rtsp_bitir();
	for (int k = 0; k < 3; k++) if (RI[k].fd >= 0) { close(RI[k].fd); RI[k].fd = -1; }
	ses_sahte = 0;
	kamera_sahte_jpeg = NULL;
	rtsp_her_zaman_acik = 0;
	DENE(dosya_var(rtsp_durum_yol), "bitiste durum dosyasi");
	return hata_say;
}

/* v12: test istemcisini k yuvasina ip ile bagla */
static void rt_bagla(int k, uint32_t ip)
{
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) return;
	if (RI[k].fd >= 0) close(RI[k].fd);
	RI[k].fd = sv[1];
	RI[k].n = 0;
	fcntl(sv[1], F_SETFL, fcntl(sv[1], F_GETFL) | O_NONBLOCK);
	if (rtsp_istemci_ekle(sv[0], ip) < 0) close(sv[0]);
}

static void rt_describe(char *m, size_t mn, const char *url, const char *uri, const char *nonce, const char *parola, int cseq)
{
	char ya[600];
	yetki_yaz(ya, sizeof ya, "DESCRIBE", uri, nonce, parola, 0);
	snprintf(m, mn, "DESCRIBE %s RTSP/1.0\r\nCSeq: %d\r\n%s\r\n", url, cseq, ya);
}

/* v12 (denetim v3 A5, A14, A18): birak bayragi sahipligi, olen cocuk, artik bayraklar, IP basina kilit, stale nonce,
 * nonce IP, uri yolu, satir zaman siniri */
static int self_test_v12(void)
{
	static uint8_t o[65536];
	static char y[8192], m[1024];
	char b[64];
	/* A5: aygitsiz cikista bayrak birakilir; baskasinin (TTS) bayragina dokunulmaz */
	ses_sahte = 0;
	istek("MIC_AKIS", "1", NULL, NULL, o, sizeof o);
	DENE(!strcmp((char *)o, "HATA mik yok\n") && !dosya_var(asistan_birak_yol), "v12: mik yok -> birak bayragi kalmaz '%s'", o);
	bayrak_yaz_icerik(asistan_birak_yol, 1, "tts 77\n");
	istek("MIC_AKIS", "1", NULL, NULL, o, sizeof o);
	ses_sahte = 1;
	istek("MIC_AKIS", "0.2", NULL, NULL, o, sizeof o);
	DENE(dosya_oku(asistan_birak_yol, b, sizeof b) > 0 && !strcmp(b, "tts 77") && !dosya_var(mik_bayrak), "v12: TTS bayragi korunur '%s'", b);
	unlink(asistan_birak_yol);
	/* baslik yazilamadan istemci kapandi (v11 bayragi birakmadan donuyordu) */
	int sv[2];
	DENE(!socketpair(AF_UNIX, SOCK_STREAM, 0, sv), "socketpair");
	close(sv[0]);
	int rc = mic_akis(sv[1], 1, 1);
	close(sv[1]);
	DENE(rc == 1 && !dosya_var(asistan_birak_yol) && !dosya_var(mik_bayrak), "v12: baslik hatasinda bayraklar silinir (rc=%d)", rc);
	ses_sahte = 0;
	/* A5: SIGKILL ile olen cocugun bayraklari yalniz onunsa silinir */
	for (int tur = 0; tur < 2; tur++) {
		fflush(stdout);
		pid_t p = fork();
		if (!p) for (;;) pause();
		snprintf(b, sizeof b, "%d\n", tur ? 1 : (int)p);
		bayrak_yaz_icerik(mik_bayrak, 1, b);
		snprintf(b, sizeof b, "%s %d\n", tur ? "tts" : "kopru", (int)p);
		bayrak_yaz_icerik(asistan_birak_yol, 1, b);
		R.olen[0] = p;
		R.olen_t[0] = mono() - 10;
		olenleri_topla(0);
		DENE(!R.olen[0] && dosya_var(mik_bayrak) == tur && dosya_var(asistan_birak_yol) == tur, "v12: olen cocuk bayraklari (tur %d)", tur);
		unlink(mik_bayrak);
		unlink(asistan_birak_yol);
	}
	/* A18b: onceki RTSP'nin artik bayraklari; baskasininki kalir */
	bayrak_yaz_icerik(kamera_bayrak, 1, "rtsp\n");
	bayrak_yaz_icerik(rtsp_aktif_yol, 1, "1\n");
	rtsp_artik_temizle();
	DENE(!dosya_var(kamera_bayrak) && !dosya_var(rtsp_aktif_yol), "v12: artik rtsp bayraklari silinir");
	bayrak_yaz_icerik(kamera_bayrak, 1, "merkez 5\n");
	rtsp_artik_temizle();
	DENE(dosya_var(kamera_bayrak), "v12: baskasinin kamera bayragi kalir");
	unlink(kamera_bayrak);
	/* A14 + A18c: IP basina kilit, stale nonce, nonce IP'ye bagli, uri yolu */
	char nonce[40];
	FILE *f = fopen(rtsp_parola_yol, "w");	/* oturum testinin <T>/rtsp-parola'si */
	if (f) { fputs("kole:dogru-parola-uzun-1\n", f); fclose(f); }
	rtsp_her_zaman_acik = 1;
	rtsp_hazirla(0, 1);
	for (int i = 0; i < 3; i++) RI[i].fd = -1;
	const uint32_t A = 0x0a000005, Bi = 0x0a000006, C = 0x0a000007;
	const char *U = "rtsp://127.0.0.1/kamera";
	nonce_uret(nonce, A);
	rt_describe(m, sizeof m, U, U, nonce, "yanlis-parola-000", 2);
	for (int i = 0; i < 3; i++) rt_bagla(i, A);
	rt_pompala(0.02);
	for (int tur = 0; tur < 3; tur++) {	/* 3 baglanti x 3 hata (her yanit 1 sn gecikir, paralel) */
		for (int i = 0; i < 3; i++) tam_yaz(RI[i].fd, m, strlen(m));
		for (int i = 0; i < 3; i++) rt_yanit(&RI[i], y, sizeof y, 3);
	}
	rt_pompala(0.1);
	DENE(R.red == 9 && !ip_kilitli(A, mono()), "v12: 9 hata kilitlemez (red=%lu)", R.red);
	rt_bagla(0, A);
	rt_istek(0, m, y, sizeof y, 3);
	DENE(R.red == 10 && ip_kilitli(A, mono()) && !ip_kilitli(Bi, mono()) && kilit_var(mono()), "v12: 10. hata yalniz o IP'yi kilitler (red=%lu)", R.red);
	nonce_uret(nonce, A);
	rt_describe(m, sizeof m, U, U, nonce, "dogru-parola-uzun-1", 3);
	rt_istek(0, m, y, sizeof y, 3);
	DENE(!strncmp(y, "RTSP/1.0 401", 12), "v12: kilitli IP dogru parolayla da 401: %.40s", y);
	rt_bagla(1, Bi);
	nonce_uret(nonce, Bi);
	rt_describe(m, sizeof m, "rtsp://127.0.0.1/kamera/", U, nonce, "dogru-parola-uzun-1", 4);
	rt_istek(1, m, y, sizeof y, 3);
	DENE(!strncmp(y, "RTSP/1.0 200", 12), "v12: baska IP kilitten etkilenmez (sondaki / farki kabul): %.40s", y);
	rt_bagla(2, C);
	rt_describe(m, sizeof m, U, U, nonce, "dogru-parola-uzun-1", 5);	/* Bi'nin nonce'u C'den */
	unsigned long red = R.red;
	rt_istek(2, m, y, sizeof y, 3);
	DENE(!strncmp(y, "RTSP/1.0 401", 12) && !strstr(y, "stale") && R.red == red + 1, "v12: baska IP'nin nonce'u gecmez: %.60s", y);
	if (mono() > 400) {
		char eski[33];
		nonce_yap(eski, (uint32_t)mono() - 400, C);
		red = R.red;
		double t0 = mono();
		rt_describe(m, sizeof m, U, U, eski, "dogru-parola-uzun-1", 6);
		rt_istek(2, m, y, sizeof y, 3);
		DENE(!strncmp(y, "RTSP/1.0 401", 12) && strstr(y, "stale=TRUE") && R.red == red && mono() - t0 < 0.9,
		     "v12: eski nonce + dogru ozet: 401 stale=TRUE, hata sayilmaz, gecikmesiz (%.2f sn)", mono() - t0);
		rt_describe(m, sizeof m, U, U, eski, "yanlis-parola-000", 7);
		rt_istek(2, m, y, sizeof y, 3);
		DENE(!strncmp(y, "RTSP/1.0 401", 12) && !strstr(y, "stale") && R.red == red + 1, "v12: eski nonce + yanlis ozet hata sayilir");
	} else {
		puts("uyari: mono() < 400 sn, stale denemesi atlandi");
	}
	nonce_uret(nonce, C);
	rt_describe(m, sizeof m, U, "rtsp://127.0.0.1/video", nonce, "dogru-parola-uzun-1", 8);
	rt_istek(2, m, y, sizeof y, 3);
	DENE(!strncmp(y, "RTSP/1.0 401", 12), "v12: Digest uri yolu istek yolundan farkli -> 401: %.40s", y);
	rtsp_bitir();
	for (int k = 0; k < 3; k++) if (RI[k].fd >= 0) { close(RI[k].fd); RI[k].fd = -1; }
	rtsp_her_zaman_acik = 0;
	/* A18d: komut satiri toplam zaman siniri (testte 0.6 sn): bayt bayt gonderen istemci uzatamaz */
	DENE(!socketpair(AF_UNIX, SOCK_STREAM, 0, sv), "socketpair 2");
	fflush(stdout);
	pid_t p = fork();
	if (!p) {
		close(sv[0]);
		for (int i = 0; i < 30; i++) {
			if (write(sv[1], "a", 1) != 1) break;
			struct timespec d = { 0, 100000000 };
			nanosleep(&d, NULL);
		}
		_exit(0);
	}
	close(sv[1]);
	char st[160];
	double t0 = mono();
	size_t n = satir_oku(sv[0], st, sizeof st, 0.6);
	double sure = mono() - t0;
	close(sv[0]);
	kill(p, SIGKILL);
	waitpid(p, NULL, 0);
	DENE(sure >= 0.5 && sure < 1.0 && n >= 3 && n <= 8, "v12: satir toplam zaman siniri (%.2f sn, %zu bayt)", sure, n);
	DENE(!socketpair(AF_UNIX, SOCK_STREAM, 0, sv), "socketpair 3");
	tam_yaz(sv[1], "anahtar KMT 1\nx", 15);
	n = satir_oku(sv[0], st, sizeof st, 5);
	DENE(n == 13 && !strcmp(st, "anahtar KMT 1"), "v12: tam satir '%s'", st);
	close(sv[0]);
	close(sv[1]);
	return hata_say;
}

static int self_test_v8(void)
{
	static char y1[300], y2[300], y3[300], y4[300], y5[300], y6[300], y7[300], y8[300], y9[300], jy[300];
	snprintf(T, sizeof T, "/tmp/kole-kopru-v8-testXXXXXX");
	if (!mkdtemp(T)) { perror("mkdtemp"); return 1; }
	yol_ata(&kalkan_yol, y1, sizeof y1, "kalkan");
	yol_ata(&kamera_bayrak, y2, sizeof y2, "kamera-aktif");
	yol_ata(&mik_bayrak, y3, sizeof y3, "mik-aktif");
	yol_ata(&hoparlor_bayrak, y4, sizeof y4, "hoparlor-aktif");
	yol_ata(&rtsp_bayrak, y5, sizeof y5, "rtsp-acik");
	yol_ata(&rtsp_durum_yol, y6, sizeof y6, "rtsp-durum.json");
	yol_ata(&rtsp_aktif_yol, y7, sizeof y7, "rtsp-aktif");
	yol_ata(&rtsp_parola_yol, y8, sizeof y8, "rtsp-parola-yok");
	yol_ata(&snd_kok, y9, sizeof y9, "snd-yok");
	static char y10[300];
	yol_ata(&asistan_birak_yol, y10, sizeof y10, "asistan-mik-birak");	/* v12 */
	if (self_test_ses()) return 1;
	puts("v8 ses: MIC_AKIS/PLAY_AKIS/kalkan/RTSP komutlari PASS");
	if (self_test_rtsp_saf()) return 1;
	puts("v8 RTSP saf: MD5/Digest(RFC 2617)/mu-law/8k/ayristirici/RFC 2435/RTCP PASS");
	static uint8_t j[300000];
	size_t jn = ornek_jpeg(j, 0, 0, 0xc0, 0, 0x11, 40000, 11);
	snprintf(jy, sizeof jy, "%s/ornek.jpg", T);
	FILE *f = fopen(jy, "wb");
	if (f) { fwrite(j, 1, jn, f); fclose(f); }
	if (self_test_rtsp_oturum(jy, j, jn)) return 1;
	puts("v8 RTSP oturum: Digest/401 gecikme/3 hata/453/461/404/501/400/RTP-JPEG/PCMU/SR/kalkan/bayraklar PASS");
	if (self_test_v12()) return 1;
	puts("v12: birak bayragi sahipli/olen cocuk/artik bayraklar/IP basina kilit/stale nonce/nonce IP/uri yolu/satir 5 sn PASS");
	char sil[400];
	snprintf(sil, sizeof sil, "rm -rf '%s'", T);
	if (system(sil)) puts("uyari: gecici klasor silinemedi");
	return 0;
}

static int self_test(void)
{
	fir_hazirla();
	/* 1 kHz 48k -> 16k: genlik korunmali; 12 kHz bastirilmali */
	static int16_t in[4800], out[1600], ust[4800 * 2];
	for (int f = 0; f < 2; f++) {
		double hz = f ? 12000 : 1000;
		for (int i = 0; i < 4800; i++)
			in[i] = (int16_t)(10000 * sin(2 * M_PI * hz * i / 48000));
		struct azalt a = {0};
		size_t k = azalt(&a, in, 4800, out, 1.0f);
		double e = 0;
		for (size_t i = 200; i < k; i++)
			e += (double)out[i] * out[i];
		double rms = sqrt(e / (k - 200));
		printf("azalt %5.0f Hz: %zu ornek rms=%.0f\n", hz, k, rms);
		if (k != 1600 || (f == 0 && fabs(rms - 7071) > 400) || (f == 1 && rms > 300)) return 1;
	}
	for (int i = 0; i < 1600; i++)
		out[i] = (int16_t)(10000 * sin(2 * M_PI * 1000.0 * i / 16000));
	struct arttir b = {0};
	size_t k = arttir(&b, out, 1600, ust);
	double e = 0;
	for (size_t i = 300; i < k; i++)
		e += (double)ust[2 * i] * ust[2 * i];
	double rms = sqrt(e / (k - 300));
	printf("arttir 1000 Hz: %zu kare rms=%.0f\n", k, rms);
	if (k != 4800 || fabs(rms - 7071) > 400) return 1;
	static char sy[300], my[300], hy[300];
	snprintf(sy, sizeof sy, "/tmp/kole-kopru-v8-snd-yok-%d", getpid());
	snd_kok = sy;
	snprintf(my, sizeof my, "/tmp/kole-kopru-v8-mik-%d", getpid());
	mik_bayrak = my;
	static char ay[300];	/* v12: gercek /tmp/asistan-mik-birak'a dokunulmaz */
	snprintf(ay, sizeof ay, "/tmp/kole-kopru-v12-birak-%d", getpid());
	asistan_birak_yol = ay;
	snprintf(hy, sizeof hy, "/tmp/kole-kopru-v8-hop-%d", getpid());
	hoparlor_bayrak = hy;
	if (self_test_kayit()) { printf("SELF-TEST FAIL (%d)\n", hata_say); return 1; }
	puts("v7 kayit testleri PASS");
	if (self_test_v8()) { printf("SELF-TEST FAIL (%d)\n", hata_say); return 1; }
	{	/* v11 */
		char y[80];
		snprintf(y, sizeof y, "/tmp/kole-kopru-v11-izinli-%d", getpid());
		int r = izinli_ip_test(y);
		DENE(r == 0, "izinli_ip_test %d", r);
		snprintf(y, sizeof y, "/tmp/kole-kopru-v11-gunluk-%d", getpid());
		int g = open(y, O_RDWR | O_CREAT | O_TRUNC | O_APPEND | O_CLOEXEC, 0600);
		struct stat st;
		DENE(g >= 0 && !ftruncate(g, 256 * 1024) && !gunluk_kirp(g) && !fstat(g, &st) && st.st_size == 256 * 1024, "kirpma esik alti");
		DENE(g >= 0 && !ftruncate(g, 300 * 1024) && gunluk_kirp(g) && write(g, "x\n", 2) == 2 && !fstat(g, &st) && st.st_size == 2,
		     "kirpma 300 KB");
		int bos[2], pr = pipe(bos);
		DENE(!pr && !gunluk_kirp(bos[0]), "boru kirpilmaz");
		if (!pr) { close(bos[0]); close(bos[1]); }
		if (g >= 0) close(g);
		unlink(y);
		if (hata_say) { printf("SELF-TEST FAIL (%d)\n", hata_say); return 1; }
		puts("v11 izinli_ip + gunluk kirpma PASS");
	}
	puts("SELF-TEST PASS: FIR; v7 KAYIT_*/kalkan/YETENEK/DURUM; v8 MIC_AKIS/PLAY_AKIS/RTSP komutlari; MD5/Digest; mu-law; "
	     "RFC 2435; RTSP oturumu (socketpair); v11 izinli_ip/gunluk kirpma; v12 bayrak sahipligi/RTSP IP kilidi/stale/"
	     "satir siniri. Ag/USB/ses aygiti acilmadi");
	return 0;
}

/* ---------------- rtsp-test: host'ta gercek TCP sinamasi (sahte kamera + sahte ses) ---------------- */
static int rtsp_test_kipi(int port, const char *jpeg, const char *parola, const char *dizin, double sn)
{
	static char y[8][300];
	snprintf(T, sizeof T, "%s", dizin);
	yol_ata(&kalkan_yol, y[0], sizeof y[0], "kalkan");
	yol_ata(&kamera_bayrak, y[1], sizeof y[1], "kamera-aktif");
	yol_ata(&mik_bayrak, y[2], sizeof y[2], "mik-aktif");
	yol_ata(&rtsp_durum_yol, y[3], sizeof y[3], "rtsp-durum.json");
	yol_ata(&rtsp_aktif_yol, y[4], sizeof y[4], "rtsp-aktif");
	yol_ata(&snd_kok, y[5], sizeof y[5], "snd-yok");
	yol_ata(&asistan_birak_yol, y[6], sizeof y[6], "asistan-mik-birak");	/* v12 */
	rtsp_parola_yol = parola;
	rtsp_her_zaman_acik = 1;
	rtsp_bag_ip = INADDR_LOOPBACK;
	ses_sahte = 1;
	kamera_sahte_jpeg = jpeg;
	rtsp_hazirla(port, 0);
	signal(SIGTERM, rtsp_sinyal);
	signal(SIGINT, rtsp_sinyal);
	printf("rtsp-test: port %d, sahte kamera %s, sahte ses, bayraklar %s\n", port, jpeg, dizin);
	fflush(stdout);
	double t = mono();
	while (!R.dur && (sn <= 0 || mono() - t < sn)) {
		rtsp_adim(20);
		fflush(stdout);
	}
	rtsp_bitir();
	return 0;
}

static int jpeg_denetle(const char *yol)
{
	static uint8_t j[1 << 20];
	int fd = open(yol, O_RDONLY | O_CLOEXEC);
	if (fd < 0) { perror(yol); return 2; }
	ssize_t n = read(fd, j, sizeof j);
	close(fd);
	struct rjpeg r;
	const char *e = rjpeg_ayristir(j, n > 0 ? (size_t)n : 0, &r);
	if (e) { printf("RFC 2435 UYGUN DEGIL: %s\n", e); return 1; }
	printf("RFC 2435 UYGUN: tip=%d (%s) %dx%d dri=%u dht_standart=%d tarama=%zu bayt\n", r.tip, r.tip ? "4:2:0" : "4:2:2",
	       r.genislik, r.yukseklik, r.dri, r.dht_standart, r.tarama_n);
	return r.dht_standart ? 0 : 3;
}

int main(int argc, char **argv)
{
	signal(SIGPIPE, SIG_IGN);
	if (argc == 2 && !strcmp(argv[1], "--self-test"))
		return self_test();
	if (argc == 2 && !strcmp(argv[1], "--durum-ornek")) {	/* yalniz derleme denetimi: DURUM JSON'u stdout'a */
		static uint8_t o[8192];
		size_t n = istek("DURUM", NULL, NULL, NULL, o, sizeof o);
		fwrite(o, 1, n, stdout);
		return 0;
	}
	if (argc == 2 && !strcmp(argv[1], "--ulaw-tablo")) {	/* -32768..32767 icin 65536 bayt (134: audioop karsilastirmasi) */
		for (int v = -32768; v <= 32767; v++) putchar(ulaw_kodla((int16_t)v));
		return 0;
	}
	if (argc >= 3 && !strcmp(argv[1], "--ornek-jpeg")) {
		static uint8_t j[300000];
		size_t n = ornek_jpeg(j, 0, argc > 3 ? (unsigned)atoi(argv[3]) : 0, 0xc0, 0, 0x11, 50000, 5);
		FILE *f = fopen(argv[2], "wb");
		if (!f || fwrite(j, 1, n, f) != n) return 1;
		fclose(f);
		return 0;
	}
	if (argc == 3 && !strcmp(argv[1], "jpeg-denetle"))
		return jpeg_denetle(argv[2]);
	if (argc >= 2 && !strcmp(argv[1], "sunucu")) {
		fir_hazirla();
		return sunucu(argc > 2 ? atoi(argv[2]) : 8767);
	}
	if (argc >= 2 && !strcmp(argv[1], "rtsp")) {
		fir_hazirla();
		return rtsp_sunucu(argc > 2 ? atoi(argv[2]) : 8554);
	}
	if (argc >= 6 && !strcmp(argv[1], "rtsp-test")) {
		fir_hazirla();
		return rtsp_test_kipi(atoi(argv[2]), argv[3], argv[4], argv[5], argc > 6 ? atof(argv[6]) : 0);
	}
	fputs("kullanim: kole-kopru sunucu [port] | rtsp [port] | rtsp-test <port> <jpeg> <parola> <dizin> [sn] |\n"
	      "          jpeg-denetle <dosya> | --ornek-jpeg <dosya> [dri] | --ulaw-tablo | --self-test | --durum-ornek\n", stderr);
	return 2;
}
