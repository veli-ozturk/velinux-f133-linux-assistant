/* kole_cihaz_merkez v18 (25.10.2026, Claude; scripts/642, kullanıcı 25.10): v17 + asistana sor, panelde söyle ve hatırlatıcı
 *   cihazda (kole-gemini çocuğu, panel eylem dosyası); panel ekranı/komutları için 501 metni anlaşılır. */
/* kole_cihaz_merkez v17 (24.10.2026, Claude; OTONOM 149, denetim v3 A20): v16 + 127.0.0.2 dinleyicisi; vekil basliklari
 *   (X-Forwarded-For/-Proto/-Host, Tailscale-User-Login) yalniz 127.0.0.2'ye gelen baglantida (Tailscale serve oraya
 *   yonlendirilir). Userspace tailscaled dogrudan tailnet baglantilarini 127.0.0.1'den getirir: basliklari taklit edebiliyordu. */
/* kole_cihaz_merkez v16 (24.10.2026, Claude; OTONOM 150, inceleme B7): v15 + foto listesi klasor basina en cok
 *   FOTO_TARAMA_AZAMI girdi stat eder. */
/* kole_cihaz_merkez v15 (24.10.2026, Claude; OTONOM 152, inceleme B5): v14 + plan penceresinde kayit-acik baskasi tarafindan
 *   (kopru KAYIT 0, panel, sesli komut) silinirse plan_adim kayit-elle-kapali yazar; elle yeniden acilinca siler. */
/* kole_cihaz_merkez v14 (24.10.2026, Claude; OTONOM 151): v13 + yalniz SURUM_TAM "kole-cihaz-merkez-v14" (v13 "v11" yaziyordu). */
/* kole_cihaz_merkez v13 (24.10.2026, Claude; scripts/626, denetim-2026-10-24-v3 A4/A5/A16/A19 + inceleme B1/B2/B5/B6): v12 +
 *   okumayan dinleyici mikrofonu acik tutamaz (kapat_sonra sayilmaz, 60/10 sn ilerlemesiz cikis kapanir); birak bayragi
 *   "kopru <pid>" sahipli (ses_akis_v4.h) ve SIGKILL'de temizlenir; canli cocuklar sabit ust sinirla (kamera 30 dk,
 *   mikrofon 2 sa); yeniden baslatma MCU kalbine hizali betikle; kayit plani bayragi "plan" icerikli, acilista elle
 *   acilmis kayda ve elle kapatma isaretine dokunmaz, gecersiz saatte karar vermez; oynatma hizi/atlamasi idx ms'den.
 *   Kurulum adi yine kole-cihaz-merkez-v9. */
/* kole_cihaz_merkez v12 (24.10.2026, Claude; scripts/616, denetim-2026-10-23-v1 bulgu 12-15, 20 + OTONOM 144): v11 +
 *   kayit plani acilista o anki hali bir kez uygular; tum oturumlari kapatirken jeton iptali yazilamazsa geri alinir (502);
 *   denetim tekrar ayiklamasi son 24 anahtarlik halka; foto listesi doluyken en yeniler kalir; /api/cihaz /proc'u bir kez
 *   tarar; hizmet adi 15 karakterde kesilmis comm yerine cmdline'dan. Kurulum adi yine kole-cihaz-merkez-v9. */
/* kole_cihaz_merkez v11 (22.10.2026, Claude): v10 + time-lapse 1..3600 sn, olay tespiti kipi (POST /api/zaman/mod "olay",
 *   <kamera>/zaman-olay), GET /api/zaman/olay (kamera-kayit v9 zaman/<gun>-olay.csv: dakikalar + esik ustu olaylar, ofs ile
 *   atlama), liste doluluk (zaman_mb/sinir/dolu) ve gun basina olay. scripts/542. Kurulum adi yine kole-cihaz-merkez-v9. */
/* kole_cihaz_merkez v10 (22.10.2026, Claude): v9 + time-lapse (GET /api/zaman/liste, CEK bicimi /api/zaman/kare, POST
 *   /api/zaman/mod), fotograflar (/api/foto/liste, /api/foto/al), /api/giris "kullanici" (varsa "efendi"), ozellik.zaman.
 *   scripts/513. Kurulum adi kole-cihaz-merkez-v9 (autorun v32 v9..v3 arar). */
/* kole_cihaz_merkez v9 (20.10.2026, Claude): v8 + POST /api/yeniden-baslat (iki kipte parola, OTA surerken 409, 3 sn
 *   sonra sync + reboot(RB_AUTOBOOT); FEL yok) + OTA adi wifi-ekran-vN. scripts/385. 387: yeniden baslatmadan once
 *   /data/.wifi-deneme silinir (Wi-Fi guc korumasi bilerek yeniden baslatmada tetiklenmesin). */
/* kole_cihaz_merkez v8 (20.10.2026, Claude): v6 + CEK bicimli canli kare GET /api/kamera/kare?son=<sira> (tek JPEG ya
 *   da 204; yolda en cok bir kare -> Tailscale aktaricisinda gecikme birikmez), gomulu /canli sayfasi
 *   (merkez_canli_v1.h), ceken istemci 5 sn izleyici sayilir. v7 (Codex, ortak on tampon) bu surumde YOK. scripts/383. */
/* kole_cihaz_merkez v6 (19.10.2026, Claude): v5 + /api/cihaz pid.mcu "mcu-servis-v" onekiyle (mcu-servis-v6). scripts/303. */
/* kole_cihaz_merkez v5 (15.10.2026, Claude): v4 + /api/cihaz hizmetler/surumler/ozellik (PC Merkez), wifi-ekran pid onekle
 *   (v4 "wifi-ekran-v16" sabit adla ariyordu). Sirlarin icerigi okunmaz. scripts/260. */
/* kole_cihaz_merkez v4 (15.10.2026, Claude): v3 + KABLOSUZ GUNCELLEME: POST /api/ota/baslat (parola + izinli ad + boyut +
 *   SHA-256 -> tek kullanimlik jeton) ve POST /api/ota/yukle?j= (govde SD'ye akar, SHA esitse yedekleyip yerine koyar).
 *   Ayrinti scripts/240. */
/* kole_cihaz_merkez v3 (15.10.2026, Claude): v2 + ses_akis_v2.h (Hey Kole mikrofonu birakir; scripts/178). */
/* kole-cihaz-merkez v2 (13.10.2026, Claude): v1 + "tek dogrulama" guvenlik modu (KOLE Merkez v7 PC ile AYNI sozlesme).
 *   Ayar "yeniden_dogrulama": "tek_giris" (VARSAYILAN, anahtar yoksa) ya da "siki" (= v1 davranisi), "oturum_bosta_saat"
 *   1..72 (varsayilan 12). tek_giris'te acik oturum; kamera acma/uzatma, SD kayit acma, kayit izni/acma, plan, denetim,
 *   ses dinle/konus izni icin parolayi YENIDEN istemez (sifre alani yok sayilir; IP/genel hata kilidi surerse 429);
 *   her islem yine denetime yazilir (ek " (tek dogrulama)"). Kayit izni gereken giris noktalari (oynat/kucuk/indir/
 *   anlik kayit) ve GET /api/denetim izni yoksa verir, varsa tazeler. Iki modda da parola: /api/parola, RTSP acma ve
 *   RTSP parolasi, /api/jeton (artik parolali) + yeni /api/jeton/iptal, baska oturumlari kapatma, tek_giris'e gecis,
 *   bosta suresini uzatma. Yeni GET/POST /api/guvenlik (yalniz oturum; Bearer 401). Oturum: mutlak 7 gun, bosta N saat.
 * --- v1 aciklamasi ---
 * kole-cihaz-merkez v1 (13.10.2026, Claude): PC'SIZ cihaz-ici KOLE Merkez. KOLE Merkez v6 (PC) ile AYNI JSON API'si ve
 * ayni PWA (SD: /mnt/sd/velinux/merkez/). Tek surec, poll() dongusu; fork/is parcacigi yalniz agir isler icin:
 *   - canli kamera kaynagi  : cocuk surecte kamera_akis() (kamera_akis_v1.h; kamera-kayit hizmeti varsa YALNIZ
 *                             /tmp/kamera-son.jpg okunur, yoksa USB YALNIZ READ(10)) -> socketpair -> bellekte son kare
 *   - canli dinleme         : cocukta mic_akis() (ses_akis_v1.h), tek ust akis, en fazla 2 dinleyici (WebSocket)
 *   - bas-konus             : cocukta play_akis() (ses_akis_v1.h), ayni anda tek konusan, en fazla 60 sn/basis
 *   - PBKDF2 dogrulama      : cocukta (dongu tikanmaz; ayni anda tek)
 *   - SD kayitlari          : SURECTE, salt okuma (kayit_sd_v1.h: kayit_liste/kayit_olaylar/idx_yukle/temsilci)
 * Dinleme: 127.0.0.1:8780 (Tailscale `serve` HTTPS'i sonlandirir ve buraya aktarir); --lan ile 0.0.0.0 (salt HTTP).
 * Guvenlik (PC v6 ile ayni ilkeler, ayrinti linux-build/docs/cihaz-merkez-tailscale-v1.md):
 *   parola PBKDF2-HMAC-SHA256 (/data/kole-web.json, 0600; ilk kurulum tek kullanimlik /data/kole-web-kurulum anahtari
 *   ya da `parola-ayarla` stdin), oturum cerezi HttpOnly + SameSite=Strict (+ Secure HTTPS arkasinda), POST'larda
 *   X-Kole: 1 + Origin denetimi, WebSocket'te Origin = Host, Host izin listesi (DNS rebinding), IP basina 15 dk'da 5 hata
 *   + genel 1 saatte 20 hata -> 30 dk kilit, kamera/kayit/dinleme/konusma/denetim icin AYRI parola + sure, denetim
 *   gunlugu SD'de (256 KB'ta doner, 2 eski kopya), gizlilik kalkani her seyi reddeder, panel gostergeleri
 *   (/tmp/kamera-aktif, /tmp/mik-aktif, /tmp/hoparlor-aktif). Goruntu/ses DISKE YAZILMAZ (yalniz kamera-kayit SD kaydi).
 * Bellek: en fazla 8 baglanti, 3 canli izleyici, 2 dinleyici, 1 konusan, 1 kayit oynatma, 1 indirme; kare tamponlari
 * yalniz kamera acikken ayrilir. Kipler: [--port N] [--lan] [--host AD]... [--pbkdf2-tur N (>=20000)] | --self-test |
 * --pbkdf2-olc | parola-ayarla (stdin'den bir satir). KOLE_TEST host surumunde ek: --test-kok DIZIN, /api/_test/saat. */
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>
#include <sys/reboot.h>	/* v9 */
#include <time.h>
#include <unistd.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-variable"
#include "kopru_ortak_v1.h"
#include "ses_akis_v4.h"	/* v13: birak bayragi sahipli (denetim v3 A5) */
#include "kamera_akis_v2.h"	/* v13: kayitci donunce USB birakilir (A18e) */
#include "kayit_sd_v1.h"
#include "rtsp_v1.h"
#pragma GCC diagnostic pop
#include "kripto_v1.h"
#include "http_ws_v1.h"
#include "json_mini_v1.h"

#define SURUM "cihaz-v2"
#define SURUM_TAM "kole-cihaz-merkez-v18"
#define VARSAYILAN_PORT 8780
#define AZAMI_BAG 8
#define AZAMI_IZLEYICI 3
#define KARE_YUVA (AZAMI_IZLEYICI + 2)
#define AZAMI_DINLEYICI 2
#define GELEN_AZAMI 16384
#define GOVDE_AZAMI 8192
#define CIKIS_AZAMI (2u * 1024 * 1024)
#define KARE_AZAMI 300000u
#define KAYIT_AZAMI (64u * 1024 * 1024)
#define WS_AZAMI 16384
#define OTURUM_AZAMI 16
#define OTURUM_OMUR (7 * 86400.0)
#define BOSTA_VARSAYILAN_SAAT 12	/* v2: ayar oturum_bosta_saat (1..72); v1'de sabit 24 sa */
#define BOSTA_ALT 1
#define BOSTA_UST 72
#define TEK_EK " (tek dogrulama)"	/* tek_giris'te parolasiz islem eki ("giris" kelimesi YOK: denetim suzgeci) */
#define KAYIT_IZIN_SN 600
#define DENETIM_IZIN_SN 300
#define KAMERA_IZLEYICISIZ_SN 180
#define KAYIT_FPS 2
#define KAMERA_COCUK_SN (30 * 60 + 5)	/* v13 (A16): canli cocuklarin sabit ust siniri; durdurmayi ebeveyn yapar */
#define MIC_COCUK_SN 7200
#define SAAT_GECERLI 1767225600	/* v13 (B6): 2026-01-01; oncesi saat ayarlanmamis */
#define YB_BETIK "/mnt/sd/velinux/yeniden_baslat_sd_v1.sh"	/* v13 (A19): MCU kalbine hizali yeniden baslatma */
#define PLAN_AZAMI_KURAL 8
#define PLAN_ARALIK_SN 20
#define KONUSMA_AZAMI_SN 60
#define OLAY_ESIK 80
#define AKTARIM_HIZ (1024.0 * 1024)
#define DENETIM_DONME (256 * 1024)
#define PBKDF2_VARSAYILAN 100000u
#define DINLE_PARCA 1920
#define DINLE_KUYRUK_AZAMI (64 * 1024)
#define RTSP_KULLANICI "kole"
#define RTSP_PORT 8554

/* ------------------------------------------------------------------ yollar (self-test / KOLE_TEST yonlendirir) */
static const char *web_kok = "/mnt/sd/velinux/merkez";
static const char *ayar_yol = "/data/kole-web.json";
static const char *kurulum_yol = "/data/kole-web-kurulum";
static const char *denetim_sd = "/mnt/sd/velinux/denetim.log";
static const char *denetim_ram = "/tmp/kole-merkez-denetim.log";
static const char *kayit_durum_yol = "/tmp/kamera-kayit.json";
static const char *bt_durum_yol = "/tmp/bt-durum.json";
static const char *ts_durum_yol = "/tmp/tailscale-durum.json";
static const char *ts_serve_yol = "/tmp/tailscale-serve.json";
static const char *ts_ikili_yol = "/mnt/sd/velinux/tailscale/tailscaled";
static const char *ts_bayrak_yol = "/mnt/sd/velinux/tailscale-acik";
static const char *merkez_durum_yol = "/tmp/kole-cihaz-merkez.json";
static const char *hop_dogrulandi_yol = "/data/hoparlor-dogrulandi";
/* rtsp_sunucu_v1.h ile AYNI (138 derleme betigi esitligi denetler) */
static const char *rtsp_bayrak_m = "/mnt/sd/velinux/rtsp-acik";
static const char *rtsp_parola_m = "/data/rtsp-parola";
static const char *rtsp_durum_m = "/tmp/rtsp-durum.json";
static const char *proc_kok = "/proc";
static const char *mounts_yol = "/proc/mounts";
static const char *ota_kok = "/mnt/sd/velinux";	/* v4 */
static const char *gemini_dizin = "/mnt/sd/velinux/asistan";	/* v18: en yuksek kole-gemini-vN */
static const char *hatirlat_yol = "/data/asistan/hatirlatici.txt";	/* v18: panel 0.35 yazar "unix|tur|metin" */
static const char *eylem_yol = "/tmp/asistan-eylem.json";	/* v18: panel 0.32 okur {"seq","ui":[...]} */
/* v5: yalniz VARLIK/yas/sayac okunur (sir dosyalarinin icerigi ASLA) */
static const char *panel_durum_yol = "/tmp/pc-ekran-durum.json";
static const char *tg_jeton_yol = "/data/asistan/telegram-jeton";
static const char *tg_ofs_yol = "/data/asistan/telegram-offset";
static const char *gemini_yol = "/data/asistan/gemini-anahtar";
static const char *asistan_kapali_yol = "/data/asistan/kapali";
static const char *cerceve_bayrak_yol = "/data/panel-cerceve";
static const char *foto_dizin = "/mnt/sd/fotograf";
static const char *yb_isaret_yol = "/tmp/yeniden-baslat-istendi";	/* v9 */
static const char *wifi_isaret_yol = "/data/.wifi-deneme";	/* v9 (387): wifi-baslat guc korumasi isareti */
static int test_kamera_dosya = 0;	/* KOLE_TEST: kamera-kayit yerine dogrudan /tmp/kamera-son.jpg izlenir */
static uint32_t pbkdf2_tur_yeni = PBKDF2_VARSAYILAN;

/* ------------------------------------------------------------------ zaman */
static double saat_kayma = 0;	/* yalniz self-test / KOLE_TEST */
static double simdi(void) { return mono() + saat_kayma; }
static double simdi_unix(void)
{
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return t.tv_sec + t.tv_nsec / 1e9 + saat_kayma;
}

static void rastgele(void *b, size_t n)
{
	int u = open("/dev/urandom", O_RDONLY | O_CLOEXEC);
	size_t k = 0;
	while (u >= 0 && k < n) {
		ssize_t r = read(u, (char *)b + k, n - k);
		if (r <= 0) break;
		k += (size_t)r;
	}
	if (u >= 0) close(u);
	if (k < n) { fprintf(stderr, "urandom okunamadi\n"); abort(); }
}

/* ------------------------------------------------------------------ durum yapilari */
enum { B_BOS, B_ISTEK, B_YANIT, B_PBKDF2, B_BEKLE, B_CANLI, B_KAYIT, B_INDIR, B_WS_DINLE, B_WS_KONUS, B_OTA };

struct oynatma { char ad[20]; int hiz, bas, kullanildi; uint32_t belirtec; };

struct oturum {
	int var;
	uint8_t ozet[32];		/* SHA-256(sid); sid yalniz cerezde */
	char kimlik[13];
	char ip[48], cihaz[48];
	double baslangic_u, son_u, son_m, bitis_m;
	long surum;
	double kayit_bitis;
	int oyn_var;
	struct oynatma oyn;
	char kucuk[24][12];
	int kucuk_n;
	double denetim_bitis, dinle_bitis, konus_bitis;
};

struct kare { uint8_t *b; uint32_t n, seq; int ref; double t; };

struct bag {
	int fd, tur, peer_lo, vekil;	/* v17 (A20): vekil = 127.0.0.2'ye gelen (Tailscale serve) */
	char ip[48];
	uint8_t gelen[GELEN_AZAMI];
	size_t gn;
	struct http_istek r;
	size_t govde_n;
	int https, oturum;		/* oturum: dizin ya da -1 */
	/* cikis */
	uint8_t *cb_;
	size_t cbas, cson, ccap;
	struct kare *kref;
	size_t kofs;
	int dosya_fd;
	int kad_yanlis;	/* v10: /api/giris kullanici adi uymadi */
	unsigned long long dosya_kalan, giden;
	double hiz, hiz_t0, bekle_t;
	int kapat_sonra, keepalive, head;
	double son_t, t0;
	double cikis_t;	/* v13 (A4): son gonderim ilerlemesi (mono) */
	/* PBKDF2 devam */
	void (*devam)(struct bag *, int, const uint8_t *);
	uint8_t yeni_tuz[16];
	char olay[48];
	double gecikme_t;
	/* canli */
	uint32_t son_seq;
	/* kayit oynatma */
	struct kidx *oyn_k;
	int oyn_n, oyn_i, oyn_hiz, oyn_fd, oyn_oturum;
	uint32_t oyn_belirtec;
	unsigned long long oyn_boyut;
	uint8_t *oyn_tampon;
	double oyn_sonraki;
	/* websocket */
	uint8_t *wsb;
	size_t wsn;
	int ws_op;
	uint32_t ws_seq;
	unsigned long long ses_bayt, ses_atilan;
	unsigned long ornek;
	double ws_son_veri;
	int kon_bitiyor;
	char neden[64];
	/* v4 OTA */
	int ota_fd;
	unsigned long long ota_kalan;
	struct sha256 ota_sha;
};

struct kural { uint8_t gun; int bas, bit; };

static struct {
	int lfd, vfd, port, lan;	/* v17: vfd 127.0.0.2 */
	pid_t sor_pid, soyle_pid;	/* v18 */
	int sor_fd, sor_bag, sor_bag_fd, sor_sesli;
	size_t sor_n;
	double sor_bitis, soyle_bitis;
	char sor_cikti[8192];
	char izinli_host[4][128];
	int izinli_n;
	char ana_ad[64];
	/* ayar */
	int ayar_var;
	uint32_t tur;
	uint8_t tuz[16], sifre[32];
	long surum;
	int jeton_var;
	uint8_t jeton[32];
	int plan_acik, plan_n;
	struct kural plan[PLAN_AZAMI_KURAL];
	int siki;		/* v2: 0 = tek_giris (varsayilan), 1 = siki (v1 davranisi) */
	int bosta_saat;		/* v2: oturum bosta kalma siniri (saat, 1..72) */
	struct timespec ayar_mtime;
	double ayar_kontrol_t;
	int kurulum_var;
	char kurulum[40];
	/* PBKDF2 iscisi */
	pid_t pb_pid;
	int pb_fd, pb_bag, pb_iki;
	uint8_t pb_sonuc[64];
	size_t pb_n;
	/* kamera */
	int kamera_acik, kamera_nesil;
	double kamera_bitis, kamera_bos_t, kam_yeniden_t;
	double kare_istek_t;	/* v8: son CEK bicimli kare istegi */
	double kamera_bitis_u;
	char kamera_hata[96];
	pid_t kam_pid;
	int kam_fd;
	uint8_t kam_bas[4];
	size_t kam_bn, kam_al;
	uint32_t kam_n;
	int kam_yuva;
	struct kare kare[KARE_YUVA];
	int son_kare;
	uint32_t kare_seq;
	double kare_t[40];
	int kare_tn;
	/* mikrofon */
	pid_t mic_pid;
	int mic_fd, mic_durum;
	char mic_hata[128], mic_satir[128];
	size_t mic_sn;
	uint32_t mic_seq;
	uint8_t mic_t[DINLE_PARCA * 2];
	size_t mic_tn;
	double mic_yeniden_t;
	/* konusma */
	pid_t kon_pid;
	int kon_fd, kon_bag;
	char kon_satir[96];
	size_t kon_sn;
	uint8_t kon_cikis[32768];
	size_t kon_cn;
	int kon_hazir;
	/* kayit */
	int oyn_bag, yan_bag;
	/* plan */
	int plan_onceki;
	char plan_son[32], plan_hata[96];
	int plan_bildirildi;
	double plan_t;
	/* genel */
	double kilit_bitis, genel_hata[20];
	int genel_n;
	double baslangic, durum_t, tick_t;
	time_t baslangic_u;
	int kalkan_onceki;
	unsigned long istek_sayi;
	volatile sig_atomic_t dur;
} M;

static struct bag B[AZAMI_BAG];
static struct oturum O[OTURUM_AZAMI];
struct ipk { char ip[48]; double t[5]; int n; double son; };
static struct ipk IPK[32];

static void bag_kapat(struct bag *c, const char *neden);
static void denetim(const char *ip, const char *f, ...) __attribute__((format(printf, 2, 3)));

/* ------------------------------------------------------------------ kucuk yardimcilar */
static int kalkan(void) { return dosya_var(kalkan_yol); }

static void tek_satir(char *s)
{
	for (; *s; s++) if ((unsigned char)*s < 32 || *s == 127) *s = ' ';
}

static int sd_yazilabilir(void)
{
	char buf[512];
	int sd = 0;
	FILE *f = fopen(mounts_yol, "r");
	while (f && fgets(buf, sizeof buf, f)) {
		char ad[256];
		snprintf(ad, sizeof ad, " %s ", sd_kok);
		if (strstr(buf, ad)) sd = strstr(buf, " rw") ? 2 : 1;
	}
	if (f) fclose(f);
	return sd;
}

static long rss_kb(void)
{
	char b[128];
	long sayfa = 0, rss = 0;
	if (dosya_oku("/proc/self/statm", b, sizeof b) <= 0 || sscanf(b, "%ld %ld", &sayfa, &rss) != 2) return -1;
	return rss * (sysconf(_SC_PAGESIZE) / 1024);
}

static long long yas_ms_m(const char *p)
{
	struct stat st;
	if (stat(p, &st)) return -1;
	double d = simdi_unix() - (st.st_mtim.tv_sec + st.st_mtim.tv_nsec / 1e9);
	return d < 0 ? 0 : (long long)(d * 1000);
}

/* ------------------------------------------------------------------ denetim gunlugu (SD, donen) */
static const char *denetim_yolu(void)
{
	char d[300];
	snprintf(d, sizeof d, "%s/velinux", sd_kok);
	struct stat st;
	return sd_yazilabilir() == 2 && !stat(d, &st) && S_ISDIR(st.st_mode) ? denetim_sd : denetim_ram;
}

static void denetim(const char *ip, const char *f, ...)
{
	char olay[320], satir[420], z[32];
	va_list a;
	va_start(a, f);
	vsnprintf(olay, sizeof olay, f, a);
	va_end(a);
	tek_satir(olay);
	olay[300] = 0;
	time_t t = (time_t)simdi_unix();
	struct tm tm;
	localtime_r(&t, &tm);
	strftime(z, sizeof z, "%Y-%m-%dT%H:%M:%S", &tm);
	char ipt[48];
	snprintf(ipt, sizeof ipt, "%s", ip && *ip ? ip : "-");
	for (char *p = ipt; *p; p++) if (*p == ' ') *p = '_';
	int n = snprintf(satir, sizeof satir, "%s %s %s\n", z, ipt, olay);
	const char *yol = denetim_yolu();
	struct stat st;
	size_t sinir = yol == denetim_sd ? DENETIM_DONME : 64 * 1024;
	if (!stat(yol, &st) && (size_t)st.st_size > sinir) {
		char y1[320], y2[320];
		snprintf(y1, sizeof y1, "%s.1", yol);
		snprintf(y2, sizeof y2, "%s.2", yol);
		rename(y1, y2);
		rename(yol, y1);
	}
	int fd = open(yol, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC, 0600);
	if (fd >= 0) {
		if (write(fd, satir, (size_t)n) != n) { /* SD dolu: sessiz */ }
		close(fd);
	}
}

/* ------------------------------------------------------------------ ayar (/data/kole-web.json) */
static void kural_metni(const struct kural *k, char *o, size_t n)
{
	static const char *G[] = { "Pzt", "Sal", "Çar", "Per", "Cum", "Cmt", "Paz" };
	size_t w = 0;
	o[0] = 0;
	for (int g = 0; g < 7; g++)
		if (k->gun & (1 << g)) w += (size_t)snprintf(o + w, n > w ? n - w : 0, "%s%s", w ? "," : "", G[g]);
	if (w < n) snprintf(o + w, n - w, " %02d:%02d-%02d:%02d", k->bas / 60, k->bas % 60, k->bit / 60, k->bit % 60);
}

static void plan_json(struct jy *y)
{
	jy_ham(y, "[");
	for (int i = 0; i < M.plan_n; i++) {
		const struct kural *k = &M.plan[i];
		jy_ham(y, i ? ",{\"gunler\":[" : "{\"gunler\":[");
		int ilk = 1;
		for (int g = 0; g < 7; g++)
			if (k->gun & (1 << g)) { jy_bicim(y, "%s%d", ilk ? "" : ",", g); ilk = 0; }
		jy_bicim(y, "],\"bas\":\"%02d:%02d\",\"bit\":\"%02d:%02d\"}", k->bas / 60, k->bas % 60, k->bit / 60, k->bit % 60);
	}
	jy_ham(y, "]");
}

static int ayar_yaz(void)
{
	struct jy y = { .azami = 16384 };
	char h[80];
	jy_bicim(&y, "{\"bicim\":1,\"tur\":%u,\"tuz\":\"", M.tur);
	hex_yaz(M.tuz, 16, h);
	jy_ham(&y, h);
	jy_ham(&y, "\",\"sifre\":\"");
	hex_yaz(M.sifre, 32, h);
	jy_ham(&y, h);
	jy_bicim(&y, "\",\"surum\":%ld,\"jeton\":", M.surum);
	if (M.jeton_var) { hex_yaz(M.jeton, 32, h); jy_dizge(&y, h); } else jy_ham(&y, "null");
	jy_bicim(&y, ",\"plan\":{\"acik\":%s,\"kurallar\":", M.plan_acik ? "true" : "false");
	plan_json(&y);
	jy_bicim(&y, "},\"yeniden_dogrulama\":\"%s\",\"oturum_bosta_saat\":%d}\n", M.siki ? "siki" : "tek_giris", M.bosta_saat);
	memset(h, 0, sizeof h);
	if (y.hata) { jy_bitir(&y); return -1; }
	char tmp[300];
	snprintf(tmp, sizeof tmp, "%s.tmp", ayar_yol);
	int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
	int ok = fd >= 0 && !fchmod(fd, 0600) && !tam_yaz(fd, y.b, y.n) && !fsync(fd);
	if (fd >= 0) close(fd);
	memset(y.b, 0, y.n);
	jy_bitir(&y);
	if (!ok || rename(tmp, ayar_yol)) { unlink(tmp); return -1; }
	struct stat st;
	if (!stat(ayar_yol, &st)) M.ayar_mtime = st.st_mtim;
	return 0;
}

static int ss_dd(const char *s, int *dk)
{
	if (strlen(s) != 5 || s[2] != ':' || !isdigit((unsigned char)s[0]) || !isdigit((unsigned char)s[1]) ||
	    !isdigit((unsigned char)s[3]) || !isdigit((unsigned char)s[4])) return -1;
	int h = (s[0] - '0') * 10 + s[1] - '0', m = (s[3] - '0') * 10 + s[4] - '0';
	if (h > 23 || m > 59) return -1;
	*dk = h * 60 + m;
	return 0;
}

/* kurallar dizisini dogrula (PC plan_dogrula ile ayni kurallar); hata metni e */
static int plan_ayristir(const char *s, const struct jtok *tk, int t, int a, struct kural *o, int *on, const char **e)
{
	*e = "Kural biçimi geçersiz";
	if (a < 0 || tk[a].tip != J_DIZI) return -1;
	if (tk[a].boyut > PLAN_AZAMI_KURAL) { *e = "En fazla 8 kural"; return -1; }
	int n = 0;
	for (int i = 0; i < tk[a].boyut; i++) {
		int k = j_oge(tk, t, a, i);
		if (k < 0 || tk[k].tip != J_NESNE) return -1;
		int g = j_al(s, tk, t, k, "gunler"), b = j_al(s, tk, t, k, "bas"), bt = j_al(s, tk, t, k, "bit");
		if (g < 0 || tk[g].tip != J_DIZI || !tk[g].boyut || tk[g].boyut > 7) { *e = "Gün listesi 0 (Pzt) .. 6 (Paz) olmalı"; return -1; }
		uint8_t m = 0;
		for (int j = 0; j < tk[g].boyut; j++) {
			long long v;
			int x = j_oge(tk, t, g, j);
			if (x < 0 || j_tamsayi(s, &tk[x], &v) || v < 0 || v > 6) { *e = "Gün listesi 0 (Pzt) .. 6 (Paz) olmalı"; return -1; }
			m |= (uint8_t)(1 << v);
		}
		char bs[8], bts[8];
		int bd, btd;
		if (b < 0 || bt < 0 || j_dizge_al(s, &tk[b], bs, sizeof bs) || j_dizge_al(s, &tk[bt], bts, sizeof bts) ||
		    ss_dd(bs, &bd) || ss_dd(bts, &btd)) { *e = "Saat SS:DD olmalı"; return -1; }
		if (bd == btd) { *e = "Başlangıç ve bitiş aynı olamaz"; return -1; }
		o[n++] = (struct kural){ m, bd, btd };
	}
	*on = n;
	return 0;
}

static int ayar_oku(void)
{
	static char b[16384];
	static struct jtok tk[512];
	M.ayar_var = 0;
	M.siki = 0;	/* v2: anahtarlar yoksa (v1 dosyasi) varsayilanlar */
	M.bosta_saat = BOSTA_VARSAYILAN_SAAT;
	if (dosya_oku(ayar_yol, b, sizeof b) <= 0) return -1;
	int t = json_ayristir(b, (int)strlen(b), tk, 512);
	char h[80];
	long long v;
	int i;
	if (t < 1 || tk[0].tip != J_NESNE) goto bozuk;
	i = j_al(b, tk, t, 0, "tur");
	if (i < 0 || j_tamsayi(b, &tk[i], &v) || v < 1000 || v > 10000000) goto bozuk;
	M.tur = (uint32_t)v;
	i = j_al(b, tk, t, 0, "tuz");
	if (i < 0 || j_dizge_al(b, &tk[i], h, sizeof h) || hex_coz(h, M.tuz, 16)) goto bozuk;
	i = j_al(b, tk, t, 0, "sifre");
	if (i < 0 || j_dizge_al(b, &tk[i], h, sizeof h) || hex_coz(h, M.sifre, 32)) goto bozuk;
	i = j_al(b, tk, t, 0, "surum");
	M.surum = i >= 0 && !j_tamsayi(b, &tk[i], &v) ? (long)v : 0;
	i = j_al(b, tk, t, 0, "jeton");
	M.jeton_var = i >= 0 && !j_dizge_al(b, &tk[i], h, sizeof h) && !hex_coz(h, M.jeton, 32);
	M.plan_acik = 0;
	M.plan_n = 0;
	i = j_al(b, tk, t, 0, "plan");
	if (i >= 0) {
		int a = j_al(b, tk, t, i, "acik"), k = j_al(b, tk, t, i, "kurallar");
		const char *e;
		struct kural kk[PLAN_AZAMI_KURAL];
		int kn = 0;
		if (k >= 0 && !plan_ayristir(b, tk, t, k, kk, &kn, &e)) {
			memcpy(M.plan, kk, sizeof kk);
			M.plan_n = kn;
			M.plan_acik = a >= 0 && j_bool(b, &tk[a]) == 1 && kn > 0;
		}
	}
	/* v2: guvenlik modu ("siki" disindaki her deger tek_giris; PC guvenlik_modu() ile ayni) ve bosta suresi */
	i = j_al(b, tk, t, 0, "yeniden_dogrulama");
	M.siki = i >= 0 && !j_dizge_al(b, &tk[i], h, sizeof h) && !strcmp(h, "siki");
	i = j_al(b, tk, t, 0, "oturum_bosta_saat");
	if (i >= 0 && !j_tamsayi(b, &tk[i], &v) && v >= BOSTA_ALT && v <= BOSTA_UST) M.bosta_saat = (int)v;
	memset(b, 0, sizeof b);
	M.ayar_var = 1;
	struct stat st;
	if (!stat(ayar_yol, &st)) M.ayar_mtime = st.st_mtim;
	return 0;
bozuk:
	memset(b, 0, sizeof b);
	fprintf(stderr, "ayar dosyasi bozuk: %s (kurulum gerekiyor)\n", ayar_yol);
	return -1;
}

/* ayar yoksa tek kullanimlik kurulum anahtari (/data, 0600). Anahtar gunluge YAZILMAZ. */
static void kurulum_hazirla(void)
{
	M.kurulum_var = 0;
	if (M.ayar_var) return;
	char b[64];
	if (dosya_oku(kurulum_yol, b, sizeof b) >= 20) {
		b[strcspn(b, " \r\n")] = 0;
		snprintf(M.kurulum, sizeof M.kurulum, "%s", b);
		M.kurulum_var = strlen(M.kurulum) >= 20;
		memset(b, 0, sizeof b);
		if (M.kurulum_var) return;
	}
	static const char abc[] = "abcdefghjkmnpqrstuvwxyz23456789";
	uint8_t r[20];
	rastgele(r, sizeof r);
	int k = 0;
	for (int i = 0; i < 20; i++) {
		if (i && i % 5 == 0) M.kurulum[k++] = '-';
		M.kurulum[k++] = abc[r[i] % 31];
	}
	M.kurulum[k] = 0;
	int fd = open(kurulum_yol, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
	if (fd >= 0) {
		fchmod(fd, 0600);
		dprintf(fd, "%s\n", M.kurulum);
		fsync(fd);
		close(fd);
		M.kurulum_var = 1;
		printf("ilk kurulum: tek kullanimlik anahtar %s dosyasinda (cihazda: cat %s)\n", kurulum_yol, kurulum_yol);
	} else {
		fprintf(stderr, "kurulum anahtari yazilamadi (%s): parola-ayarla kipini kullanin\n", kurulum_yol);
	}
	memset(r, 0, sizeof r);
}

/* ------------------------------------------------------------------ oturumlar */
static void oturum_temizle(int i)
{
	memset(&O[i], 0, sizeof O[i]);
}

/* v2: bosta kalma siniri ayardan (saniye) */
static double bosta_sn(void)
{
	int h = M.bosta_saat >= BOSTA_ALT && M.bosta_saat <= BOSTA_UST ? M.bosta_saat : BOSTA_VARSAYILAN_SAAT;
	return h * 3600.0;
}

static int oturum_gecerli(int i)
{
	if (i < 0 || i >= OTURUM_AZAMI || !O[i].var) return 0;
	double t = simdi();
	return O[i].surum == M.surum && O[i].bitis_m > t && t - O[i].son_m < bosta_sn();
}

static int oturum_sayisi(void)
{
	int n = 0;
	for (int i = 0; i < OTURUM_AZAMI; i++) n += oturum_gecerli(i);
	return n;
}

static void cihaz_ozet(const char *ua, char *o, size_t n)
{
	static const char *C[][2] = { { "iPhone", "iPhone" }, { "iPad", "iPad" }, { "Android", "Android" },
		{ "Windows", "Windows" }, { "Macintosh", "Mac" }, { "Linux", "Linux" } };
	static const char *T[][2] = { { "Edg/", "Edge" }, { "CriOS", "Chrome" }, { "Chrome/", "Chrome" }, { "FxiOS", "Firefox" },
		{ "Firefox/", "Firefox" }, { "Safari/", "Safari" } };
	const char *c = "Bilinmeyen cihaz", *t = "";
	for (size_t i = 0; i < sizeof C / sizeof C[0]; i++) if (strstr(ua, C[i][0])) { c = C[i][1]; break; }
	for (size_t i = 0; i < sizeof T / sizeof T[0]; i++) if (strstr(ua, T[i][0])) { t = T[i][1]; break; }
	snprintf(o, n, "%s%s%s", c, *t ? " · " : "", t);
}

/* yeni oturum: sid_hex (49) doldurulur; dizin ya da -1 */
static int oturum_ac(const char *ip, const char *ua, char sid_hex[49])
{
	int bos = -1;
	double t = simdi(), en_eski = 1e300;
	int eski = 0;
	for (int i = 0; i < OTURUM_AZAMI; i++) {
		if (O[i].var && !oturum_gecerli(i)) oturum_temizle(i);
		if (!O[i].var && bos < 0) bos = i;
		if (O[i].var && O[i].son_m < en_eski) { en_eski = O[i].son_m; eski = i; }
	}
	if (bos < 0) { bos = eski; oturum_temizle(bos); }	/* dolu: en uzun suredir bosta olani kapat */
	uint8_t sid[24];
	rastgele(sid, sizeof sid);
	hex_yaz(sid, 24, sid_hex);
	struct oturum *o = &O[bos];
	memset(o, 0, sizeof *o);
	o->var = 1;
	sha256_tek(sid, sizeof sid, o->ozet);
	char k[80];
	uint8_t h[32];
	snprintf(k, sizeof k, "oturum:%s", sid_hex);
	sha256_tek(k, strlen(k), h);
	hex_yaz(h, 6, o->kimlik);
	snprintf(o->ip, sizeof o->ip, "%s", ip);
	cihaz_ozet(ua ? ua : "", o->cihaz, sizeof o->cihaz);
	o->baslangic_u = o->son_u = simdi_unix();
	o->son_m = t;
	o->bitis_m = t + OTURUM_OMUR;
	o->surum = M.surum;
	memset(sid, 0, sizeof sid);
	return bos;
}

static int oturum_bul(const char *cerez)
{
	char v[64];
	uint8_t sid[24], h[32];
	if (!cerez_al(cerez, "kole_cm", v, sizeof v) || hex_coz(v, sid, 24)) return -1;
	sha256_tek(sid, sizeof sid, h);
	int bulunan = -1;
	for (int i = 0; i < OTURUM_AZAMI; i++)
		if (O[i].var && sabit_esit(O[i].ozet, h, 32)) bulunan = i;
	memset(sid, 0, sizeof sid);
	return oturum_gecerli(bulunan) ? bulunan : -1;
}

static int izin_aktif(int o, double bitis) { return oturum_gecerli(o) && bitis > simdi(); }
static int izin_kalan(int o, double bitis) { return izin_aktif(o, bitis) ? (int)(bitis - simdi()) : 0; }

/* ------------------------------------------------------------------ kaba kuvvet sinirlari */
static struct ipk *ipk_al(const char *ip, int olustur)
{
	struct ipk *bos = NULL, *eski = &IPK[0];
	for (size_t i = 0; i < sizeof IPK / sizeof IPK[0]; i++) {
		if (IPK[i].ip[0] && !strcmp(IPK[i].ip, ip)) return &IPK[i];
		if (!IPK[i].ip[0] && !bos) bos = &IPK[i];
		if (IPK[i].son < eski->son) eski = &IPK[i];
	}
	if (!olustur) return NULL;
	struct ipk *k = bos ? bos : eski;
	memset(k, 0, sizeof *k);
	snprintf(k->ip, sizeof k->ip, "%s", ip);
	return k;
}

static int hata_sayisi(const char *ip)
{
	struct ipk *k = ipk_al(ip, 0);
	if (!k) return 0;
	int n = 0;
	for (int i = 0; i < k->n; i++) n += simdi() - k->t[i] < 900;
	return n;
}

/* 0: denenebilir, 1: IP siniri (429), 2: genel kilit (429) */
static int deneme_engeli(const char *ip)
{
	if (M.kilit_bitis > simdi()) return 2;
	return hata_sayisi(ip) >= 5;
}

static void hata_ekle(const char *ip)
{
	struct ipk *k = ipk_al(ip, 1);
	double t = simdi();
	int n = 0;
	for (int i = 0; i < k->n; i++) if (t - k->t[i] < 900) k->t[n++] = k->t[i];
	if (n == 5) { memmove(k->t, k->t + 1, 4 * sizeof(double)); n = 4; }
	k->t[n++] = t;
	k->n = n;
	k->son = t;
	/* genel: 1 saatte 20 hata -> 30 dk kilit */
	int g = 0;
	for (int i = 0; i < M.genel_n; i++) if (t - M.genel_hata[i] < 3600) M.genel_hata[g++] = M.genel_hata[i];
	if (g == 20) { memmove(M.genel_hata, M.genel_hata + 1, 19 * sizeof(double)); g = 19; }
	M.genel_hata[g++] = t;
	M.genel_n = g;
	if (g >= 20 && M.kilit_bitis <= t) {
		M.kilit_bitis = t + 1800;
		denetim(ip, "GENEL KILIT: 1 saatte 20 hatali parola, 30 dk tum parola girisleri kapali");
	}
}

/* ------------------------------------------------------------------ cikis tamponu */
static int cikis_ekle(struct bag *c, const void *v, size_t n)
{
	if (c->cson + n > c->ccap) {
		if (c->cbas) {
			memmove(c->cb_, c->cb_ + c->cbas, c->cson - c->cbas);
			c->cson -= c->cbas;
			c->cbas = 0;
		}
		if (c->cson + n > c->ccap) {
			size_t yeni = c->ccap ? c->ccap : 4096;
			while (yeni < c->cson + n) yeni *= 2;
			if (yeni > CIKIS_AZAMI) return -1;
			uint8_t *nb = realloc(c->cb_, yeni);
			if (!nb) return -1;
			c->cb_ = nb;
			c->ccap = yeni;
		}
	}
	memcpy(c->cb_ + c->cson, v, n);
	c->cson += n;
	return 0;
}

static size_t cikis_bekleyen(const struct bag *c) { return c->cson - c->cbas; }

static void cikis_serbest(struct bag *c)
{
	if (c->ccap > 65536 || !cikis_bekleyen(c)) {
		free(c->cb_);
		c->cb_ = NULL;
		c->ccap = c->cbas = c->cson = 0;
	}
}

static const char *durum_metni(int k)
{
	switch (k) {
	case 101: return "Switching Protocols";
	case 200: return "OK";
	case 400: return "Bad Request";
	case 401: return "Unauthorized";
	case 403: return "Forbidden";
	case 404: return "Not Found";
	case 405: return "Method Not Allowed";
	case 409: return "Conflict";
	case 413: return "Payload Too Large";
	case 421: return "Misdirected Request";
	case 423: return "Locked";
	case 429: return "Too Many Requests";
	case 431: return "Request Header Fields Too Large";
	case 501: return "Not Implemented";
	case 502: return "Bad Gateway";
	case 503: return "Service Unavailable";
	default: return "Error";
	}
}

static void guvenlik_basliklari(struct bag *c, struct jy *y)
{
	char ws[300] = "";
	const char *h = c->r.host;
	if (*h && strlen(h) < 128 && strspn(h, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.:[]-") == strlen(h))
		snprintf(ws, sizeof ws, " ws://%s wss://%s", h, h);
	jy_ham(y, "X-Content-Type-Options: nosniff\r\nX-Frame-Options: DENY\r\nReferrer-Policy: no-referrer\r\n"
		   "Permissions-Policy: camera=(), microphone=(self), geolocation=(), payment=(), usb=()\r\n"
		   "Cross-Origin-Opener-Policy: same-origin\r\nCross-Origin-Resource-Policy: same-origin\r\n"
		   "Content-Security-Policy: default-src 'self'; img-src 'self' blob: data:; style-src 'self'; script-src 'self'; "
		   "connect-src 'self'");
	jy_ham(y, ws);
	jy_ham(y, "; worker-src 'self'; manifest-src 'self'; object-src 'none'; base-uri 'none'; form-action 'self'; "
		   "frame-ancestors 'none'\r\n");
}

/* baslik + govde (govde NULL olabilir: akis yanitlari) */
static void yanit_bas(struct bag *c, int kod, const char *tur, long long uzunluk, const char *ek)
{
	struct jy y = { .azami = 8192 };
	jy_bicim(&y, "HTTP/1.1 %d %s\r\n", kod, durum_metni(kod));
	if (tur) jy_bicim(&y, "Content-Type: %s\r\n", tur);
	if (uzunluk >= 0) jy_bicim(&y, "Content-Length: %lld\r\n", uzunluk);
	guvenlik_basliklari(c, &y);
	if (!strncmp(c->r.yol, "/api/", 5) || uzunluk < 0) jy_ham(&y, "Cache-Control: no-store\r\n");
	else if (!strcmp(c->r.yol, "/") || !strcmp(c->r.yol, "/index.html") || !strcmp(c->r.yol, "/sw.js"))
		jy_ham(&y, "Cache-Control: no-cache\r\n");
	if (ek) jy_ham(&y, ek);
	if (uzunluk < 0) c->keepalive = 0;
	jy_ham(&y, c->keepalive ? "Connection: keep-alive\r\n\r\n" : "Connection: close\r\n\r\n");
	if (y.hata || cikis_ekle(c, y.b, y.n)) c->kapat_sonra = 1;
	jy_bitir(&y);
}

static void yanit(struct bag *c, int kod, const char *tur, const void *g, size_t n, const char *ek)
{
	yanit_bas(c, kod, tur, (long long)n, ek);
	if (n && !c->head && cikis_ekle(c, g, n)) c->kapat_sonra = 1;
	c->tur = B_YANIT;
}

static void yanit_json(struct bag *c, int kod, struct jy *y, const char *ek)
{
	if (y->hata) {
		static const char e[] = "{\"hata\":\"Yanıt çok büyük\"}";
		yanit(c, 500, "application/json", e, sizeof e - 1, NULL);
	} else {
		yanit(c, kod, "application/json", y->b ? y->b : "{}", y->b ? y->n : 2, ek);
	}
	jy_bitir(y);
}

static const char *turkce(int kod)
{
	switch (kod) {
	case 400: return "Geçersiz istek";
	case 401: return "Oturum gerekli";
	case 403: return "İzin yok";
	case 404: return "Bulunamadı";
	case 409: return "Başka bir kayıt oynatılıyor; birkaç saniye sonra deneyin";
	case 413: return "İstek çok büyük";
	case 423: return "Kilitli: önce parolayla izin alın";
	case 429: return "Çok fazla hatalı deneme, 15 dk bekleyin";
	case 503: return "Cihaz meşgul; birkaç saniye sonra yeniden deneyin";
	default: return "Hata";
	}
}

static void hata_yanit(struct bag *c, int kod, const char *metin)
{
	struct jy y = { .azami = 4096 };
	jy_ham(&y, "{\"hata\":");
	jy_dizge(&y, metin ? metin : turkce(kod));
	jy_ham(&y, "}");
	yanit_json(c, kod, &y, NULL);
}

static void ok_yanit(struct bag *c)
{
	static const char e[] = "{\"ok\":true}";
	yanit(c, 200, "application/json", e, sizeof e - 1, NULL);
}

/* ------------------------------------------------------------------ istek govdesi (JSON) */
struct govde { const char *s; int n, t; struct jtok tk[256]; };
static struct govde G;

static int govde_ayristir(struct bag *c)
{
	G.s = (const char *)c->gelen + c->r.baslik_n;
	G.n = (int)c->govde_n;
	G.t = c->govde_n ? json_ayristir(G.s, G.n, G.tk, 256) : -1;
	if (G.t < 1 || G.tk[0].tip != J_NESNE) G.t = 0;
	return G.t;
}

static int g_al(const char *ad) { return G.t ? j_al(G.s, G.tk, G.t, 0, ad) : -1; }
static int g_bool(const char *ad) { int i = g_al(ad); return i < 0 ? -1 : j_bool(G.s, &G.tk[i]); }
static int g_int(const char *ad, long long *v) { int i = g_al(ad); return i < 0 ? -2 : j_tamsayi(G.s, &G.tk[i], v); }
/* 0 ok, -1 yok/tip yanlis, -2 cok uzun */
static int g_str(const char *ad, char *o, size_t n)
{
	int i = g_al(ad);
	if (i < 0 || G.tk[i].tip != J_DIZGE) return -1;
	return j_dizge_al(G.s, &G.tk[i], o, n) ? -2 : 0;
}

/* ------------------------------------------------------------------ PBKDF2 iscisi (cocuk surec) */
static void cocuk_hazirla(int tut)
{
	signal(SIGCHLD, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGPIPE, SIG_IGN);
	int azami = (int)sysconf(_SC_OPEN_MAX);
	if (azami < 0 || azami > 4096) azami = 4096;
	for (int f = 3; f < azami; f++) if (f != tut) close(f);
}

static char pb_sifre[1100], pb_yeni[1100];
static int govde_ayristir(struct bag *c);

/* sifre(+yeni) PBKDF2'si cocukta; bitince c->devam(c, ok, h2). 0 basladi, -1 mesgul */
static int pbkdf2_baslat(struct bag *c, int iki, void (*devam)(struct bag *, int, const uint8_t *))
{
	if (M.pb_pid > 0) return -1;
	int p[2];
	if (pipe2(p, O_CLOEXEC)) return -1;
	fflush(stdout);
	pid_t pid = fork();
	if (pid < 0) { close(p[0]); close(p[1]); return -1; }
	if (!pid) {
		cocuk_hazirla(p[1]);
		uint8_t h[64];
		pbkdf2_sha256(pb_sifre, strlen(pb_sifre), M.tuz, 16, M.tur, h, 32);
		if (iki) pbkdf2_sha256(pb_yeni, strlen(pb_yeni), c->yeni_tuz, 16, pbkdf2_tur_yeni, h + 32, 32);
		tam_yaz(p[1], h, iki ? 64 : 32);
		_exit(0);
	}
	close(p[1]);
	memset(pb_sifre, 0, sizeof pb_sifre);
	memset(pb_yeni, 0, sizeof pb_yeni);
	fcntl(p[0], F_SETFL, O_NONBLOCK);
	M.pb_pid = pid;
	M.pb_fd = p[0];
	M.pb_bag = (int)(c - B);
	M.pb_iki = iki;
	M.pb_n = 0;
	c->devam = devam;
	c->tur = B_PBKDF2;
	return 0;
}

static void pbkdf2_oku(void)
{
	ssize_t r = read(M.pb_fd, M.pb_sonuc + M.pb_n, sizeof M.pb_sonuc - M.pb_n);
	if (r > 0) { M.pb_n += (size_t)r; if (M.pb_n < (M.pb_iki ? 64u : 32u)) return; }
	else if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
	close(M.pb_fd);
	M.pb_fd = -1;
	M.pb_pid = 0;	/* SIGCHLD toplayicisi bekler */
	struct bag *c = M.pb_bag >= 0 ? &B[M.pb_bag] : NULL;
	M.pb_bag = -1;
	int tamam = M.pb_n >= (M.pb_iki ? 64u : 32u);
	if (c && c->fd >= 0 && c->tur == B_PBKDF2 && c->devam) {
		void (*d)(struct bag *, int, const uint8_t *) = c->devam;
		c->devam = NULL;
		c->tur = B_YANIT;
		govde_ayristir(c);	/* G baska istekle degismis olabilir: bu istegin govdesi yeniden */
		if (!tamam) hata_yanit(c, 503, NULL);
		else if (c->oturum >= 0 && !oturum_gecerli(c->oturum)) hata_yanit(c, 401, NULL);
		else d(c, sabit_esit(M.pb_sonuc, M.sifre, 32), M.pb_sonuc + 32);
	}
	memset(M.pb_sonuc, 0, sizeof M.pb_sonuc);
}

/* basarisiz: 1 sn sonra gonderilecek yanit */
static void geciktir(struct bag *c)
{
	c->gecikme_t = simdi() + 1.0;
}

/* Yeniden dogrulama girisi: izin + sinir + "sifre" alani. 0: PBKDF2 basladi, -1: yanit gonderildi */
static int yeniden_parola(struct bag *c, const char *olay, void (*devam)(struct bag *, int, const uint8_t *))
{
	int e = deneme_engeli(c->ip);
	if (e) { hata_yanit(c, 429, e == 2 ? "Çok fazla hatalı deneme: 30 dk kilit" : NULL); return -1; }
	if (!M.ayar_var) { hata_yanit(c, 409, "Önce kurulum gerekli"); return -1; }
	int r = g_str("sifre", pb_sifre, sizeof pb_sifre);
	if (r == -2 || (r == 0 && strlen(pb_sifre) > 1024)) { memset(pb_sifre, 0, sizeof pb_sifre); hata_yanit(c, 400, NULL); return -1; }
	if (r == -1) pb_sifre[0] = 0;
	snprintf(c->olay, sizeof c->olay, "%s", olay);
	if (pbkdf2_baslat(c, 0, devam)) { memset(pb_sifre, 0, sizeof pb_sifre); hata_yanit(c, 503, NULL); return -1; }
	return 0;
}

/* devam islevlerinin ortak basi: parola yanlissa 403 (1 sn gecikmeli) ve 1 doner */
static int parola_red(struct bag *c, int ok)
{
	if (ok) return 0;
	hata_ekle(c->ip);
	denetim(c->ip, "%s REDDEDILDI (parola)", c->olay);
	hata_yanit(c, 403, "Parola yanlış");
	geciktir(c);
	return 1;
}

/* v2: denetim satiri eki. Yalniz tek_giris'te parolasiz yapilan (devam islevi dogrudan cagrilan) islemlerde TEK_EK */
static const char *dog_ek = "";

/* v2: modun gevsettigi hassas islemler (kamera/kayit/plan/denetim/ses izni). tek_giris'te acik oturum yeter: "sifre"
 * alani yok sayilir ve devam(c, 1) hemen cagrilir (IP ya da genel hata kilidi suruyorsa 429, PC v7 ile ayni);
 * siki'de v1 yeniden_parola aynen. 0: islendi/basladi, -1: yanit gonderildi */
static int hassas(struct bag *c, const char *olay, void (*devam)(struct bag *, int, const uint8_t *))
{
	if (M.siki || c->oturum < 0) return yeniden_parola(c, olay, devam);
	int e = deneme_engeli(c->ip);
	if (e) { hata_yanit(c, 429, e == 2 ? "Çok fazla hatalı deneme: 30 dk kilit" : NULL); return -1; }
	snprintf(c->olay, sizeof c->olay, "%s", olay);
	dog_ek = TEK_EK;
	devam(c, 1, NULL);
	dog_ek = "";
	return 0;
}

/* ------------------------------------------------------------------ cihaz durumu (kole-kopru v8 DURUM ile ayni alanlar) */

/* v5: tek /proc taramasi; comm onekiyle (cekirdek comm'u 15 karakterde keser) */
#define HZ_N 8
static const char *const HZ_AD[HZ_N] = { "panel", "wifi_ekran", "kamera_kayit", "kole_kopru", "asistan", "tailscale", "mcu", "merkez" };
static const char *const HZ_ONEK[HZ_N] = { "panel", "wifi-ekran-v", "kamera-kayit-v", "kole-kopru-v", "kole-asistan-v", "tailscaled",
					     "mcu-servis-v", "kole-cihaz-merk" };
struct hizmet { int pid; char ad[24]; };	/* v12: kole-cihaz-merkez-v9 20 karakter */
static void hizmet_tara(struct hizmet *h)
{
	DIR *d = opendir(proc_kok);
	struct dirent *e;
	char yol[300], c[64];
	memset(h, 0, sizeof(struct hizmet) * HZ_N);
	while (d && (e = readdir(d))) {
		if (e->d_name[0] < '1' || e->d_name[0] > '9') continue;
		snprintf(yol, sizeof yol, "%s/%s/comm", proc_kok, e->d_name);
		if (dosya_oku(yol, c, sizeof c) <= 0) continue;
		for (int i = 0; i < HZ_N; i++) {
			size_t n = strlen(HZ_ONEK[i]);
			int esit = i == 0 ? !strcmp(c, HZ_ONEK[i]) : !strncmp(c, HZ_ONEK[i], n);
			if (esit && !h[i].pid) {
				h[i].pid = atoi(e->d_name);
				snprintf(h[i].ad, sizeof h[i].ad, "%.23s", c);
				char a[300];	/* v12: comm 15 karakterde kesilir (kamera-kayit-v10 -> -v1): argv[0]'in dosya adi */
				snprintf(yol, sizeof yol, "%s/%s/cmdline", proc_kok, e->d_name);
				if (strlen(c) >= 15 && dosya_oku(yol, a, sizeof a) > 0) {
					const char *b = strrchr(a, '/');
					snprintf(h[i].ad, sizeof h[i].ad, "%.23s", b ? b + 1 : a);
				}
			}
		}
	}
	if (d) closedir(d);
}

static const char *hoparlor_durumu(void)
{
	char yol[300];
	snprintf(yol, sizeof yol, "%s/pcmC0D0p", snd_kok);
	if (!dosya_var(yol) && !ses_sahte) return "yok";
	return dosya_var(hop_dogrulandi_yol) ? "dogrulandi" : "dogrulanmadi";
}

struct cdurum {
	char uptime[32], loadavg[64], mem[16], wifi[64], sicak[24], bt[1024], rtsp[1024];
	int sd, kamera_usb, ses_karti, hizmet;
	long long sd_bos, sd_top, son_kare_yas, durum_yas;
	int p_panel, p_wifi, p_mcu;
	struct hizmet hz[HZ_N];	/* v12: tek /proc taramasi (/api/cihaz da kullanir) */
};

static void cihaz_durumu(struct cdurum *d)
{
	char up[64], buf[512];
	memset(d, 0, sizeof *d);
	dosya_oku("/proc/uptime", up, sizeof up);
	snprintf(d->uptime, sizeof d->uptime, "%.*s", (int)strcspn(up, " "), up[0] ? up : "0");
	if (!d->uptime[0]) strcpy(d->uptime, "0");
	dosya_oku("/proc/loadavg", buf, sizeof buf);
	snprintf(d->loadavg, sizeof d->loadavg, "%.14s", buf);
	strcpy(d->mem, "0");
	FILE *f = fopen("/proc/meminfo", "r");
	while (f && fgets(buf, sizeof buf, f))
		if (sscanf(buf, "MemAvailable: %15s", d->mem) == 1) break;
	if (f) fclose(f);
	strcpy(d->wifi, "null");
	f = fopen("/proc/net/wireless", "r");
	while (f && fgets(buf, sizeof buf, f)) {
		float link, level;
		if (strstr(buf, "wlan0:") && sscanf(strstr(buf, ":") + 1, "%*s %f %f", &link, &level) == 2) {
			snprintf(d->wifi, sizeof d->wifi, "{\"link\":%.0f,\"dbm\":%.0f}", link, level);
			break;
		}
	}
	if (f) fclose(f);
	if (dosya_oku("/sys/class/thermal/thermal_zone0/temp", d->sicak, sizeof d->sicak) <= 0 ||
	    strspn(d->sicak, "-0123456789") != strlen(d->sicak)) strcpy(d->sicak, "null");
	if (dosya_oku(bt_durum_yol, d->bt, sizeof d->bt) <= 0 || d->bt[0] != '{') strcpy(d->bt, "null");
	else {
		static struct jtok tk[128];
		if (json_ayristir(d->bt, (int)strlen(d->bt), tk, 128) < 1) strcpy(d->bt, "null");
	}
	d->sd = sd_yazilabilir();
	d->sd_bos = d->sd_top = -1;
	struct statvfs v;
	if (d->sd && !statvfs(sd_kok, &v)) {
		d->sd_bos = (long long)v.f_bavail * (long long)v.f_frsize / 1048576;
		d->sd_top = (long long)v.f_blocks * (long long)v.f_frsize / 1048576;
	}
	char kam[32], snd[300];
	d->kamera_usb = dosya_oku("/sys/bus/usb/devices/1-1/idVendor", kam, sizeof kam) > 0;
	snprintf(snd, sizeof snd, "%s/pcmC0D0p", snd_kok);
	d->ses_karti = dosya_var(snd);
	long long ry = yas_ms_m(rtsp_durum_m);
	strcpy(d->rtsp, "null");
	if (ry >= 0 && ry <= 10000 && dosya_oku(rtsp_durum_m, d->rtsp, sizeof d->rtsp) > 0 && d->rtsp[0] == '{') {
		static struct jtok tk[128];
		if (json_ayristir(d->rtsp, (int)strlen(d->rtsp), tk, 128) < 1) strcpy(d->rtsp, "null");
	} else strcpy(d->rtsp, "null");
	{	/* v5: onek (v4: "wifi-ekran-v16" sabit); v12: tek tarama, panel de buradan */
		struct hizmet *hz = d->hz;
		hizmet_tara(hz);
		d->p_panel = hz[0].pid;
		d->p_wifi = hz[1].pid;
		d->p_mcu = hz[6].pid;	/* v6: onek "mcu-servis-v" (v5: sabit "mcu-servis-v3") */
	}
	d->hizmet = hizmet_var();
	d->son_kare_yas = yas_ms_m(kamera_son);
	d->durum_yas = yas_ms_m(kayit_durum_yol);
}

static void sayi_null(struct jy *y, long long v)
{
	if (v < 0) jy_ham(y, "null"); else jy_bicim(y, "%lld", v);
}

/* tam: kamera_saglik/rtsp/rtsp_bayrak dahil (yalniz oturumla, /api/cihaz) */
static void durum_alanlari(struct jy *y, const struct cdurum *d, int tam)
{
	jy_bicim(y, "\"uptime\":%s,\"loadavg\":", d->uptime);
	jy_dizge(y, d->loadavg);
	jy_bicim(y, ",\"mem_kb\":%s,\"wifi\":%s,\"sicaklik_mC\":%s,\"sd\":%d,\"sd_bos_mb\":", d->mem, d->wifi, d->sicak, d->sd);
	sayi_null(y, d->sd_bos);
	jy_ham(y, ",\"sd_toplam_mb\":");
	sayi_null(y, d->sd_top);
	jy_bicim(y, ",\"kamera_usb\":%s,\"kamera_aktif\":%s,\"mik_aktif\":%s,\"hoparlor_aktif\":%s,\"kalkan\":%s,"
		 "\"ses_karti\":%s,\"hoparlor\":\"%s\"",
		 d->kamera_usb ? "true" : "false", dosya_var(kamera_bayrak) ? "true" : "false",
		 dosya_var(mik_bayrak) ? "true" : "false", dosya_var(hoparlor_bayrak) ? "true" : "false",
		 kalkan() ? "true" : "false", d->ses_karti ? "true" : "false", hoparlor_durumu());
	if (tam) {
		jy_bicim(y, ",\"rtsp_bayrak\":%s,\"rtsp\":%s", dosya_var(rtsp_bayrak_m) ? "true" : "false", d->rtsp);
		jy_bicim(y, ",\"kamera_saglik\":{\"hizmet\":%s,\"son_kare_yas_ms\":", d->hizmet ? "true" : "false");
		sayi_null(y, d->son_kare_yas);
		jy_ham(y, ",\"durum_yas_ms\":");
		sayi_null(y, d->durum_yas);
		jy_ham(y, "}");
	}
	jy_bicim(y, ",\"pid\":{\"panel\":%d,\"wifi_ekran\":%d,\"mcu\":%d}", d->p_panel, d->p_wifi, d->p_mcu);
	jy_ham(y, ",\"yetenek\":[\"KAYIT_OKU_ARALIK\",\"KAYIT_IDX\",\"KAYIT_KARE\",\"KAYIT_OLAYLAR\",\"KALKAN\",\"SAGLIK\","
		  "\"MIC_AKIS\",\"PLAY_AKIS\",\"RTSP\",\"CIHAZ_MERKEZI\"]");
	jy_bicim(y, ",\"bt\":%s", d->bt);
}

/* ------------------------------------------------------------------ kamera (canli) */
static int izleyici_sayisi(void)
{
	int n = 0;
	for (int i = 0; i < AZAMI_BAG; i++) n += B[i].fd >= 0 && B[i].tur == B_CANLI;
	n += M.kare_istek_t > 0 && simdi() - M.kare_istek_t < 5.0;	/* v8: cek bicimli izleyici */
	return n;
}

/* Cocuklar SIGTERM ile OLDURULMEZ: soket kapatilir, cocuk bir sonraki yazma/okumada kendi temizligini yapar
 * (/tmp/mik-aktif, /tmp/hoparlor-aktif silinir, "Headphone Switch" eski degerine doner, USB arayuzu birakilir).
 * 3 sn icinde cikmazsa SIGKILL + yalniz o cocugun pid'ini tasiyan bayrak silinir. */
struct olen { pid_t pid; double son; const char *bayrak; };
static struct olen OLEN[8];

static void cocuk_birak(pid_t pid, const char *bayrak)
{
	if (pid <= 0) return;
	for (int i = 0; i < 8; i++)
		if (!OLEN[i].pid) { OLEN[i] = (struct olen){ pid, simdi() + 3, bayrak }; return; }
	kill(pid, SIGKILL);	/* liste dolu: olamaz (en fazla 3 cocuk) */
}

static void olenleri_denetle(void)
{
	for (int i = 0; i < 8; i++) {
		if (!OLEN[i].pid || simdi() < OLEN[i].son) continue;
		kill(OLEN[i].pid, SIGKILL);
		if (OLEN[i].bayrak) {
			char b[32];
			if (dosya_oku(OLEN[i].bayrak, b, sizeof b) > 0 && atoi(b) == OLEN[i].pid) unlink(OLEN[i].bayrak);
		}
		sahipliyse_sil(asistan_birak_yol, "kopru ", OLEN[i].pid);	/* v13 (A5): mikrofon cocugunun birak bayragi */
		OLEN[i].son = simdi() + 3600;	/* toplanana kadar bekle */
	}
}

static void kare_birak(struct bag *c)
{
	if (c->kref) { c->kref->ref--; c->kref = NULL; }
	c->kofs = 0;
}

static void kareler_serbest(void)
{
	for (int i = 0; i < KARE_YUVA; i++)
		if (M.kare[i].b && !M.kare[i].ref) { free(M.kare[i].b); M.kare[i] = (struct kare){ 0 }; }
	M.son_kare = -1;
}

static void kamera_cocuk_dur(void)
{
	if (M.kam_fd >= 0) close(M.kam_fd);
	cocuk_birak(M.kam_pid, NULL);
	M.kam_pid = 0;
	M.kam_fd = -1;
	M.kam_bn = M.kam_al = 0;
	M.kam_yuva = -1;
}

static void kamera_kapat(const char *neden, const char *ip)
{
	int acikti = M.kamera_acik;
	M.kamera_acik = 0;
	M.kamera_bitis = 0;
	M.kamera_nesil++;
	kamera_cocuk_dur();
	for (int i = 0; i < AZAMI_BAG; i++)
		if (B[i].fd >= 0 && B[i].tur == B_CANLI) bag_kapat(&B[i], "kamera kapandi");
	kareler_serbest();
	M.kare_tn = 0;
	/* bayrak yalniz bizimse silinir (RTSP izleyicisinin bayragina dokunulmaz) */
	char b[64];
	if (dosya_oku(kamera_bayrak, b, sizeof b) >= 0 && !strncmp(b, "merkez", 6)) unlink(kamera_bayrak);
	if (acikti && neden) denetim(ip, "%s", neden);
}

static int kamera_ac(int dk)
{
	if (kalkan()) return -2;
	char b[48];
	snprintf(b, sizeof b, "merkez %d\n", getpid());
	char eski[64];
	if (dosya_oku(kamera_bayrak, eski, sizeof eski) < 0 || !strncmp(eski, "merkez", 6)) bayrak_yaz_icerik(kamera_bayrak, 1, b);
	if (!dosya_var(kamera_bayrak)) return -1;
	if (!M.kamera_acik) {
		M.kamera_nesil++;
		M.kamera_bos_t = simdi();
		M.kam_yeniden_t = 0;
		M.kamera_hata[0] = 0;
	}
	M.kamera_acik = 1;
	M.kamera_bitis = simdi() + dk * 60.0;
	M.kamera_bitis_u = simdi_unix() + dk * 60.0;
	return 0;
}

static void kamera_cocuk_baslat(void)
{
	int sp[2];
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sp)) return;
	fflush(stdout);
	pid_t p = fork();
	if (p < 0) { close(sp[0]); close(sp[1]); return; }
	if (!p) {
		cocuk_hazirla(sp[1]);
		struct timeval to = { .tv_sec = 10 };
		setsockopt(sp[1], SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);
		if (test_kamera_dosya) {
			if (kalkan()) _exit(hata(sp[1], "kalkan acik"));
			kamera_akis_dosya(sp[1], KAMERA_COCUK_SN, 5);
		} else {
			kamera_akis(sp[1], KAMERA_COCUK_SN, 5);	/* v13 (A16): v12 kalan sureyle baslatiyordu, uzatma yansimiyordu */
		}
		_exit(0);
	}
	close(sp[1]);
	fcntl(sp[0], F_SETFL, O_NONBLOCK);
	M.kam_pid = p;
	M.kam_fd = sp[0];
	M.kam_bn = M.kam_al = 0;
	M.kam_yuva = -1;
}

static double kamera_fps(void)
{
	double t = simdi(), z[40];
	int n = 0;
	for (int i = 0; i < M.kare_tn; i++) if (t - M.kare_t[i] < 5) z[n++] = M.kare_t[i];
	return n >= 2 && z[n - 1] > z[0] ? (n - 1) / (z[n - 1] - z[0]) : 0.0;
}

static void kare_bolum_gonder(struct bag *c);

static void kare_yayinla(void)
{
	for (int i = 0; i < AZAMI_BAG; i++)
		if (B[i].fd >= 0 && B[i].tur == B_CANLI && !B[i].kref && !cikis_bekleyen(&B[i])) kare_bolum_gonder(&B[i]);
}

static void kamera_oku(void)
{
	for (int dongu = 0; dongu < 16; dongu++) {
		if (M.kam_bn < 4) {
			ssize_t r = read(M.kam_fd, M.kam_bas + M.kam_bn, 4 - M.kam_bn);
			if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
			if (r <= 0) goto bitti;
			M.kam_bn += (size_t)r;
			if (M.kam_bn < 4) return;
			if (!memcmp(M.kam_bas, "HATA", 4)) {
				char s[96] = "";
				ssize_t k = read(M.kam_fd, s, sizeof s - 1);
				if (k > 0) s[k] = 0;
				s[strcspn(s, "\n")] = 0;
				snprintf(M.kamera_hata, sizeof M.kamera_hata, "Cihaz kamerası:%s", s);
				goto bitti;
			}
			M.kam_n = M.kam_bas[0] | M.kam_bas[1] << 8 | M.kam_bas[2] << 16 | (uint32_t)M.kam_bas[3] << 24;
			if (M.kam_n < 1000 || M.kam_n > KARE_AZAMI) {
				snprintf(M.kamera_hata, sizeof M.kamera_hata, "Canlı kare boyutu sınır dışında");
				goto bitti;
			}
			M.kam_yuva = -1;
			for (int i = 0; i < KARE_YUVA; i++)
				if (i != M.son_kare && !M.kare[i].ref) { M.kam_yuva = i; break; }
			if (M.kam_yuva < 0) goto bitti;	/* olamaz: yuva sayisi izleyici+2 */
			struct kare *k = &M.kare[M.kam_yuva];
			if (!k->b && !(k->b = malloc(KARE_AZAMI))) goto bitti;
			M.kam_al = 0;
		}
		struct kare *k = &M.kare[M.kam_yuva];
		ssize_t r = read(M.kam_fd, k->b + M.kam_al, M.kam_n - M.kam_al);
		if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
		if (r <= 0) goto bitti;
		M.kam_al += (size_t)r;
		if (M.kam_al < M.kam_n) continue;
		M.kam_bn = 0;
		if (k->b[0] != 0xff || k->b[1] != 0xd8 || k->b[M.kam_n - 2] != 0xff || k->b[M.kam_n - 1] != 0xd9) {
			snprintf(M.kamera_hata, sizeof M.kamera_hata, "Canlı JPEG biçimi geçersiz");
			continue;
		}
		k->n = M.kam_n;
		k->seq = ++M.kare_seq;
		k->t = simdi();
		M.son_kare = M.kam_yuva;
		M.kamera_hata[0] = 0;
		if (M.kare_tn == 40) { memmove(M.kare_t, M.kare_t + 1, 39 * sizeof(double)); M.kare_tn = 39; }
		M.kare_t[M.kare_tn++] = k->t;
		kare_yayinla();
	}
	return;
bitti:
	if (!M.kamera_hata[0]) snprintf(M.kamera_hata, sizeof M.kamera_hata, "Kamera akışı kesildi");
	kamera_cocuk_dur();
	M.kam_yeniden_t = simdi() + 3;
}

static void kare_bolum_gonder(struct bag *c)
{
	if (M.son_kare < 0) return;
	struct kare *k = &M.kare[M.son_kare];
	if (!k->b || k->seq <= c->son_seq) return;
	char h[256];
	int kalan = M.kamera_bitis > simdi() ? (int)(M.kamera_bitis - simdi()) : 0;
	int n = snprintf(h, sizeof h, "--kare\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\nX-Seq: %u\r\nX-Yas-Ms: %d\r\n"
			 "X-Kalan-Sn: %d\r\n\r\n", k->n, k->seq, (int)((simdi() - k->t) * 1000), kalan);
	if (cikis_ekle(c, h, (size_t)n)) { c->kapat_sonra = 1; return; }
	c->kref = k;
	k->ref++;
	c->kofs = 0;
	c->son_seq = k->seq;
}

/* ------------------------------------------------------------------ mikrofon (tek ust akis, cok dinleyici) */
static const char *ses_hata_metni(const char *s)
{
	if (!strncmp(s, "HATA ", 5)) {
		const char *h = s + 5;
		if (!strcmp(h, "kalkan acik")) return "Panelde gizlilik kalkanı açık: mikrofon/hoparlör uzaktan kullanılamaz";
		if (!strcmp(h, "mik mesgul")) return "Mikrofon başka bir işte (sesli asistan ya da RTSP izleyicisi)";
		if (!strcmp(h, "mik yok")) return "Cihazda mikrofon/ses kartı yok";
		if (!strcmp(h, "ses cikisi yok")) return "Cihazda ses çıkışı yok (ses kartı açılmamış)";
		if (!strcmp(h, "hoparlor mesgul")) return "Hoparlör başka bir işte (anons/PLAY sürüyor)";
		if (!strcmp(h, "hoparlor acilamadi")) return "Hoparlör açılamadı";
		return "Cihaz: ses hatası";
	}
	return *s ? "Beklenmeyen yanıt" : "Ses süreci yanıt vermedi";
}

static const char *mic_durum_metni(void)
{
	static const char *D[] = { "kapalı", "bağlanıyor", "dinliyor", "hata" };
	return D[M.mic_durum & 3];
}

static int dinleyici_sayisi(void)
{
	int n = 0;
	for (int i = 0; i < AZAMI_BAG; i++) n += B[i].fd >= 0 && B[i].tur == B_WS_DINLE && !B[i].kapat_sonra;	/* v13 (A4) */
	return n;
}

static void mic_dur(void)
{
	if (M.mic_fd >= 0) close(M.mic_fd);
	cocuk_birak(M.mic_pid, mik_bayrak);
	M.mic_pid = 0;
	M.mic_fd = -1;
	M.mic_sn = M.mic_tn = 0;
	memset(M.mic_t, 0, sizeof M.mic_t);
}

static void mic_hata(const char *m)
{
	snprintf(M.mic_hata, sizeof M.mic_hata, "%s", m);
	M.mic_durum = 3;
	M.mic_seq++;
}

static void mic_baslat(void)
{
	int sp[2];
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sp)) return;
	fflush(stdout);
	pid_t p = fork();
	if (p < 0) { close(sp[0]); close(sp[1]); return; }
	if (!p) {
		cocuk_hazirla(sp[1]);
		struct timeval to = { .tv_sec = 10 };
		setsockopt(sp[1], SOL_SOCKET, SO_SNDTIMEO, &to, sizeof to);
		fir_hazirla();
		mic_akis(sp[1], MIC_COCUK_SN, 1);	/* v13 (A16): izin uzatilinca akis kopmaz */
		_exit(0);
	}
	close(sp[1]);
	fcntl(sp[0], F_SETFL, O_NONBLOCK);
	M.mic_pid = p;
	M.mic_fd = sp[0];
	M.mic_sn = M.mic_tn = 0;
	M.mic_durum = 1;
}

static void ws_gonder(struct bag *c, int op, const void *v, size_t n)
{
	uint8_t h[10];
	size_t hn = ws_baslik(h, op, n);
	if (cikis_ekle(c, h, hn) || cikis_ekle(c, v, n)) c->kapat_sonra = 1;
}

static void ws_metin(struct bag *c, struct jy *y)
{
	if (!y->hata) ws_gonder(c, 1, y->b, y->n);
	jy_bitir(y);
}

static void mic_dagit(const uint8_t *p, size_t n)
{
	for (int i = 0; i < AZAMI_BAG; i++) {
		struct bag *c = &B[i];
		if (c->fd < 0 || c->tur != B_WS_DINLE || c->kapat_sonra) continue;
		if (cikis_bekleyen(c) > DINLE_KUYRUK_AZAMI) { c->ses_atilan += n; continue; }	/* yavas dinleyici: eski ses birikmez */
		ws_gonder(c, 2, p, n);
		c->ses_bayt += n;
	}
}

static void mic_oku(void)
{
	uint8_t b[4096];
	for (;;) {
		ssize_t r = read(M.mic_fd, b, sizeof b);
		if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
		if (r <= 0) {
			if (M.mic_durum != 3) mic_hata("Mikrofon akışı kesildi");
			mic_dur();
			M.mic_yeniden_t = simdi() + 3;
			return;
		}
		size_t i = 0;
		if (M.mic_durum == 1) {
			while (i < (size_t)r && M.mic_sn < sizeof M.mic_satir - 1) {
				char ch = (char)b[i++];
				if (ch == '\n') break;
				M.mic_satir[M.mic_sn++] = ch;
			}
			M.mic_satir[M.mic_sn] = 0;
			if (i == (size_t)r && b[r - 1] != '\n' && M.mic_sn < sizeof M.mic_satir - 1) continue;
			if (!strncmp(M.mic_satir, "MIC_AKIS 16000 1", 16)) {
				M.mic_durum = 2;
				M.mic_hata[0] = 0;
				M.mic_seq++;
			} else {
				mic_hata(ses_hata_metni(M.mic_satir));
				mic_dur();
				M.mic_yeniden_t = simdi() + 3;
				return;
			}
		}
		while (i < (size_t)r) {
			size_t k = sizeof M.mic_t - M.mic_tn < (size_t)r - i ? sizeof M.mic_t - M.mic_tn : (size_t)r - i;
			memcpy(M.mic_t + M.mic_tn, b + i, k);
			M.mic_tn += k;
			i += k;
			if (M.mic_tn >= DINLE_PARCA) {
				size_t g = M.mic_tn & ~(size_t)1;
				mic_dagit(M.mic_t, g);
				memmove(M.mic_t, M.mic_t + g, M.mic_tn - g);
				M.mic_tn -= g;
			}
		}
		memset(b, 0, sizeof b);
	}
}

/* ------------------------------------------------------------------ bas-konus */
static void kon_dur(void)
{
	if (M.kon_fd >= 0) close(M.kon_fd);
	cocuk_birak(M.kon_pid, hoparlor_bayrak);
	M.kon_pid = 0;
	M.kon_fd = -1;
	M.kon_bag = -1;
	M.kon_sn = M.kon_cn = 0;
	M.kon_hazir = 0;
	memset(M.kon_cikis, 0, sizeof M.kon_cikis);
}

static int kon_baslat(void)
{
	int sp[2];
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0, sp)) return -1;
	fflush(stdout);
	pid_t p = fork();
	if (p < 0) { close(sp[0]); close(sp[1]); return -1; }
	if (!p) {
		cocuk_hazirla(sp[1]);
		struct timeval to = { .tv_sec = 3 };
		setsockopt(sp[1], SOL_SOCKET, SO_RCVTIMEO, &to, sizeof to);
		fir_hazirla();
		play_akis(sp[1], KONUSMA_AZAMI_SN);
		_exit(0);
	}
	close(sp[1]);
	fcntl(sp[0], F_SETFL, O_NONBLOCK);
	M.kon_pid = p;
	M.kon_fd = sp[0];
	M.kon_sn = M.kon_cn = 0;
	M.kon_hazir = 0;
	return 0;
}

static void kon_yaz(void)
{
	while (M.kon_cn && M.kon_fd >= 0) {
		ssize_t w = send(M.kon_fd, M.kon_cikis, M.kon_cn, MSG_NOSIGNAL | MSG_DONTWAIT);
		if (w < 0 && (errno == EAGAIN || errno == EINTR)) return;
		if (w <= 0) { M.kon_cn = 0; return; }
		memmove(M.kon_cikis, M.kon_cikis + w, M.kon_cn - (size_t)w);
		M.kon_cn -= (size_t)w;
	}
}

static void konusma_bitti(struct bag *c, const char *neden, const char *cihaz)
{
	struct jy y = { .azami = 1024 };
	jy_bicim(&y, "{\"t\":\"tamam\",\"sn\":%.1f,\"neden\":", c->ornek / 16000.0);
	jy_dizge(&y, neden);
	jy_ham(&y, ",\"cihaz\":");
	jy_dizge(&y, cihaz);
	jy_ham(&y, "}");
	ws_metin(c, &y);
	snprintf(c->neden, sizeof c->neden, "%s", neden);
	uint8_t kp[2] = { 0x03, 0xe8 };
	ws_gonder(c, 8, kp, 2);
	c->kapat_sonra = 1;
}

static void kon_oku(void)
{
	char b[256];
	ssize_t r = read(M.kon_fd, b, sizeof b);
	if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
	struct bag *c = M.kon_bag >= 0 ? &B[M.kon_bag] : NULL;
	if (r <= 0) {
		if (c && c->fd >= 0 && c->tur == B_WS_KONUS && !c->kapat_sonra) {
			if (!M.kon_hazir) {
				struct jy y = { .azami = 512 };
				jy_ham(&y, "{\"t\":\"hata\",\"metin\":");
				jy_dizge(&y, ses_hata_metni(M.kon_satir));
				jy_ham(&y, "}");
				ws_metin(c, &y);
				snprintf(c->neden, sizeof c->neden, "%s", "ses süreci açılamadı");
				c->kapat_sonra = 1;
			} else konusma_bitti(c, c->kon_bitiyor ? (c->neden[0] ? c->neden : "bitti") : "ses süreci kapandı", M.kon_satir);
		}
		kon_dur();
		return;
	}
	for (ssize_t i = 0; i < r; i++) {
		if (b[i] != '\n') {
			if (M.kon_sn < sizeof M.kon_satir - 1) M.kon_satir[M.kon_sn++] = b[i];
			continue;
		}
		M.kon_satir[M.kon_sn] = 0;
		if (!M.kon_hazir && !strncmp(M.kon_satir, "PLAY_AKIS HAZIR", 15)) {
			M.kon_hazir = 1;
			if (c && c->fd >= 0) {
				const char *h = hoparlor_durumu();
				const char *m = !strcmp(h, "dogrulanmadi") ? "Panel hoparlörü henüz çalışmıyor (ses sorunu araştırılıyor): "
					"konuşmanız cihaza iletilir ama duyulmayabilir." : !strcmp(h, "yok") ? "Cihazda ses çıkışı yok: konuşma gönderilemez." : "";
				struct jy y = { .azami = 1024 };
				jy_bicim(&y, "{\"t\":\"hazir\",\"azami_sn\":%d,\"hoparlor_metin\":", KONUSMA_AZAMI_SN);
				jy_dizge(&y, m);
				jy_ham(&y, "}");
				ws_metin(c, &y);
				c->ws_son_veri = simdi();
				c->t0 = simdi();
			}
		} else if (!M.kon_hazir) {
			if (c && c->fd >= 0) {
				struct jy y = { .azami = 512 };
				jy_ham(&y, "{\"t\":\"hata\",\"metin\":");
				jy_dizge(&y, ses_hata_metni(M.kon_satir));
				jy_ham(&y, "}");
				ws_metin(c, &y);
				snprintf(c->neden, sizeof c->neden, "%s", ses_hata_metni(M.kon_satir));
				c->kapat_sonra = 1;
			}
			kon_dur();
			return;
		} else if (c && c->fd >= 0) {
			konusma_bitti(c, c->neden[0] ? c->neden : "bitti", M.kon_satir);
			kon_dur();
			return;
		}
		M.kon_sn = 0;
	}
}

/* ------------------------------------------------------------------ istek yardimcilari */
static int ip_yerel(const char *ip) { return !strcmp(ip, "127.0.0.1") || !strcmp(ip, "::1"); }

static int host_uygun(const char *h)
{
	char ad[128];
	if (!*h) return 0;
	if (h[0] == '[') return strchr(h, ']') != NULL;
	snprintf(ad, sizeof ad, "%s", h);
	char *p = strrchr(ad, ':');
	if (p) {
		if (strspn(p + 1, "0123456789") != strlen(p + 1)) return 0;
		*p = 0;
	}
	for (char *q = ad; *q; q++) *q = (char)tolower((unsigned char)*q);
	if (!*ad) return 0;
	if (strspn(ad, "0123456789.") == strlen(ad)) return 1;	/* IPv4 */
	if (!strcmp(ad, "localhost")) return 1;
	size_t n = strlen(ad);
	if (n > 7 && !strcmp(ad + n - 7, ".ts.net")) return 1;
	if (M.ana_ad[0] && (!strcmp(ad, M.ana_ad) || (!strncmp(ad, M.ana_ad, strlen(M.ana_ad)) && !strcmp(ad + strlen(M.ana_ad), ".local"))))
		return 1;
	for (int i = 0; i < M.izinli_n; i++) if (!strcmp(ad, M.izinli_host[i])) return 1;
	return 0;
}

/* Origin varsa Host ya da X-Forwarded-Host ile ayni olmali */
static int koken_uygun(struct bag *c, int zorunlu)
{
	if (!c->r.origin[0]) return !zorunlu;
	char n[192], h[128];
	if (koken_netloc(c->r.origin, n, sizeof n)) return 0;
	snprintf(h, sizeof h, "%s", c->r.host);
	for (char *q = h; *q; q++) *q = (char)tolower((unsigned char)*q);
	if (!strcmp(n, h)) return 1;
	if (c->vekil && c->r.xfhost[0]) {	/* v17 (A20) */
		snprintf(h, sizeof h, "%.*s", (int)strcspn(c->r.xfhost, ", "), c->r.xfhost);
		for (char *q = h; *q; q++) *q = (char)tolower((unsigned char)*q);
		if (!strcmp(n, h)) return 1;
	}
	/* Tailscale serve Host'u degistirirse: --host ile verilen ad(lar) vekil baglantida Origin olarak kabul edilir (v17) */
	if (c->vekil)
		for (int i = 0; i < M.izinli_n; i++) if (!strcmp(n, M.izinli_host[i])) return 1;
	return 0;
}

static int jeton_gecerli(struct bag *c)
{
	if (strncmp(c->r.auth, "Bearer ", 7) || !M.jeton_var) return 0;
	uint8_t h[32];
	sha256_tek(c->r.auth + 7, strlen(c->r.auth + 7), h);
	return sabit_esit(h, M.jeton, 32);	/* M.jeton = SHA-256(jeton); dosyada hex */
}

/* oturum (ya da jeton_olur ise Bearer) zorunlu; POST'ta X-Kole + Origin. 0 ok, -1 yanit gonderildi */
static int izin(struct bag *c, int jeton_olur)
{
	if (c->oturum >= 0) {
		struct oturum *o = &O[c->oturum];
		o->son_m = simdi();
		o->son_u = simdi_unix();
		snprintf(o->ip, sizeof o->ip, "%s", c->ip);
		if (!strcmp(c->r.yontem, "POST") && (strcmp(c->r.xkole, "1") || !koken_uygun(c, 0))) { hata_yanit(c, 403, NULL); return -1; }
		return 0;
	}
	if (jeton_olur && jeton_gecerli(c)) return 0;
	hata_yanit(c, 401, NULL);
	return -1;
}

static void cerez_basligi(struct bag *c, const char *sid, char *o, size_t n)
{
	if (sid)
		snprintf(o, n, "Set-Cookie: kole_cm=%s; Path=/; Max-Age=%d; HttpOnly; SameSite=Strict%s\r\n", sid, (int)OTURUM_OMUR,
			 c->https ? "; Secure" : "");
	else
		snprintf(o, n, "Set-Cookie: kole_cm=; Path=/; Max-Age=0; HttpOnly; SameSite=Strict%s\r\n", c->https ? "; Secure" : "");
}

/* ------------------------------------------------------------------ giris / kurulum / oturumlar */
static void giris_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (!ok || c->kad_yanlis) {	/* v10 */
		hata_ekle(c->ip);
		denetim(c->ip, "GIRIS BASARISIZ%s", c->kad_yanlis ? " (kullanici adi)" : "");
		hata_yanit(c, 401, c->kad_yanlis ? "Kullanıcı adı ya da parola yanlış" : "Parola yanlış");
		geciktir(c);
		return;
	}
	char sid[49], ck[200];
	int i = oturum_ac(c->ip, c->r.ua, sid);
	cerez_basligi(c, sid, ck, sizeof ck);
	memset(sid, 0, sizeof sid);
	denetim(c->ip, "giris %s", O[i].kimlik);
	yanit(c, 200, "application/json", "{\"ok\":true}", 11, ck);
	memset(ck, 0, sizeof ck);
}

/* v10: giris adi (kullanıcı: "kullanıcı adı efendi olsun şifre aynı kalsın"). Bas/son bosluk ve ASCII buyuk/kucuk harf onemsiz. */
static int kullanici_uygun(const char *k)
{
	static const char beklenen[] = "efendi";
	while (*k == ' ') k++;
	size_t n = strlen(k);
	while (n && k[n - 1] == ' ') n--;
	if (n != sizeof beklenen - 1) return 0;
	for (size_t i = 0; i < n; i++) {
		char ch = k[i];
		if (ch >= 'A' && ch <= 'Z') ch = (char)(ch - 'A' + 'a');
		if (ch != beklenen[i]) return 0;
	}
	return 1;
}

static void api_giris(struct bag *c)
{
	int e = deneme_engeli(c->ip);
	if (e) { hata_yanit(c, 429, e == 2 ? "Çok fazla hatalı deneme: 30 dk kilit" : "Çok fazla deneme, 15 dk bekleyin"); return; }
	if (strcmp(c->r.xkole, "1") || !koken_uygun(c, 0)) { hata_yanit(c, 403, NULL); return; }
	if (!M.ayar_var) {
		static const char k[] = "{\"hata\":\"Önce ilk kurulum: kurulum anahtarı ve yeni parola\",\"kurulum\":true}";
		yanit(c, 409, "application/json", k, sizeof k - 1, NULL);
		return;
	}
	int r = g_str("sifre", pb_sifre, sizeof pb_sifre);
	if (r || strlen(pb_sifre) > 1024) { memset(pb_sifre, 0, sizeof pb_sifre); hata_yanit(c, 400, NULL); return; }
	char kad[64];	/* v10: alan yoksa (-1) eski istemci gecer; varsa "efendi"; cok uzun (-2) yanlis */
	int rk = g_str("kullanici", kad, sizeof kad);
	c->kad_yanlis = rk == 0 ? !kullanici_uygun(kad) : rk == -2;
	if (pbkdf2_baslat(c, 0, giris_devam)) { memset(pb_sifre, 0, sizeof pb_sifre); hata_yanit(c, 503, NULL); }
}

static void kurulum_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)ok;
	if (M.ayar_var) { hata_yanit(c, 409, "Kurulum zaten tamamlandı"); return; }
	memcpy(M.tuz, c->yeni_tuz, 16);
	memcpy(M.sifre, h2, 32);
	M.tur = pbkdf2_tur_yeni;
	M.surum = 1;
	M.jeton_var = 0;
	M.plan_acik = 0;
	M.plan_n = 0;
	if (ayar_yaz()) { hata_yanit(c, 502, "Ayar /data'ya yazılamadı"); return; }
	M.ayar_var = 1;
	M.kurulum_var = 0;
	memset(M.kurulum, 0, sizeof M.kurulum);
	unlink(kurulum_yol);
	sync();
	char sid[49], ck[200];
	int i = oturum_ac(c->ip, c->r.ua, sid);
	cerez_basligi(c, sid, ck, sizeof ck);
	memset(sid, 0, sizeof sid);
	denetim(c->ip, "ILK KURULUM: parola ayarlandi, kurulum anahtari silindi; giris %s", O[i].kimlik);
	yanit(c, 200, "application/json", "{\"ok\":true}", 11, ck);
}

static void api_kurulum(struct bag *c)
{
	if (M.ayar_var) { hata_yanit(c, 409, "Kurulum zaten tamamlandı"); return; }
	int e = deneme_engeli(c->ip);
	if (e) { hata_yanit(c, 429, NULL); return; }
	if (strcmp(c->r.xkole, "1") || !koken_uygun(c, 0)) { hata_yanit(c, 403, NULL); return; }
	char anahtar[64];
	if (g_str("anahtar", anahtar, sizeof anahtar) || !M.kurulum_var || strlen(anahtar) != strlen(M.kurulum) ||
	    !sabit_esit(anahtar, M.kurulum, strlen(M.kurulum))) {
		hata_ekle(c->ip);
		denetim(c->ip, "ILK KURULUM REDDEDILDI (anahtar)");
		hata_yanit(c, 403, "Kurulum anahtarı yanlış");
		geciktir(c);
		return;
	}
	int r = g_str("yeni", pb_yeni, sizeof pb_yeni);
	size_t L = r ? 0 : strlen(pb_yeni);
	if (r || L < 10 || L > 1024) { memset(pb_yeni, 0, sizeof pb_yeni); hata_yanit(c, 400, "Yeni parola en az 10 karakter olmalı"); return; }
	snprintf(pb_sifre, sizeof pb_sifre, "-");
	rastgele(c->yeni_tuz, 16);
	if (pbkdf2_baslat(c, 1, kurulum_devam)) { memset(pb_yeni, 0, sizeof pb_yeni); hata_yanit(c, 503, NULL); }
}

static void api_cikis(struct bag *c)
{
	if (izin(c, 0)) return;
	oturum_temizle(c->oturum);
	c->oturum = -1;
	char ck[160];
	cerez_basligi(c, NULL, ck, sizeof ck);
	yanit(c, 200, "application/json", "{\"ok\":true}", 11, ck);
}

static void tum_izinleri_sil(void)
{
	for (int i = 0; i < OTURUM_AZAMI; i++) {
		O[i].kayit_bitis = O[i].denetim_bitis = O[i].dinle_bitis = O[i].konus_bitis = 0;
		O[i].oyn_var = 0;
	}
}

static void parola_devam(struct bag *c, int ok, const uint8_t *h2)
{
	if (!ok) {
		hata_ekle(c->ip);
		denetim(c->ip, "parola degisimi REDDEDILDI");
		hata_yanit(c, 403, "Eski parola yanlış");
		geciktir(c);
		return;
	}
	int bu = c->oturum;
	if (!oturum_gecerli(bu)) { hata_yanit(c, 401, NULL); return; }
	uint8_t eski_tuz[16], eski_sifre[32];
	uint32_t eski_tur = M.tur;
	memcpy(eski_tuz, M.tuz, 16);
	memcpy(eski_sifre, M.sifre, 32);
	memcpy(M.tuz, c->yeni_tuz, 16);
	memcpy(M.sifre, h2, 32);
	M.tur = pbkdf2_tur_yeni;
	M.surum++;
	if (ayar_yaz()) {
		memcpy(M.tuz, eski_tuz, 16);
		memcpy(M.sifre, eski_sifre, 32);
		M.tur = eski_tur;
		M.surum--;
		hata_yanit(c, 502, "Ayar /data'ya yazılamadı");
		return;
	}
	for (int i = 0; i < OTURUM_AZAMI; i++) if (i != bu) oturum_temizle(i);
	tum_izinleri_sil();
	O[bu].surum = M.surum;
	kamera_kapat(NULL, c->ip);
	unlink(kurulum_yol);
	denetim(c->ip, "parola degisti (diger oturumlar kapandi)");
	ok_yanit(c);
}

static void api_parola(struct bag *c)
{
	if (izin(c, 0)) return;
	int e = deneme_engeli(c->ip);
	if (e) { hata_yanit(c, 429, NULL); return; }
	int r1 = g_str("eski", pb_sifre, sizeof pb_sifre);
	if (r1 == -1) pb_sifre[0] = 0;
	int r2 = g_str("yeni", pb_yeni, sizeof pb_yeni);
	size_t L = r2 ? 0 : strlen(pb_yeni);
	if (r1 == -2 || strlen(pb_sifre) > 1024 || r2 || L < 10 || L > 1024) {
		memset(pb_sifre, 0, sizeof pb_sifre);
		memset(pb_yeni, 0, sizeof pb_yeni);
		hata_yanit(c, 400, "Yeni parola en az 10 karakter olmalı");
		return;
	}
	rastgele(c->yeni_tuz, 16);
	if (pbkdf2_baslat(c, 1, parola_devam)) { memset(pb_yeni, 0, sizeof pb_yeni); hata_yanit(c, 503, NULL); }
}

/* v2: Kisayollar jetonu iki modda da parolayla (PC v7 ile ayni) */
static void jeton_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	uint8_t r[32];
	char j[64];
	rastgele(r, sizeof r);
	/* token_urlsafe(32) benzeri */
	size_t n = b64_kodla(r, 32, j, sizeof j);
	for (size_t i = 0; i < n; i++) { if (j[i] == '+') j[i] = '-'; else if (j[i] == '/') j[i] = '_'; }
	while (n && j[n - 1] == '=') j[--n] = 0;
	sha256_tek(j, n, M.jeton);
	M.jeton_var = 1;
	if (ayar_yaz()) { M.jeton_var = 0; hata_yanit(c, 502, "Ayar yazılamadı"); return; }
	denetim(c->ip, "kisayol jetonu yenilendi");
	struct jy y = { .azami = 256 };
	jy_ham(&y, "{\"jeton\":");
	jy_dizge(&y, j);
	jy_ham(&y, "}");
	yanit_json(c, 200, &y, NULL);
	memset(j, 0, sizeof j);
	memset(r, 0, sizeof r);
}

static void api_jeton(struct bag *c)
{
	if (izin(c, 0)) return;
	yeniden_parola(c, "KISAYOL JETONU", jeton_devam);
}

/* v2: jetonu parolayla iptal (iki modda da) */
static void jeton_iptal_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	int eski = M.jeton_var;
	uint8_t yedek[32];
	memcpy(yedek, M.jeton, 32);
	M.jeton_var = 0;
	memset(M.jeton, 0, 32);
	if (ayar_yaz()) { M.jeton_var = eski; memcpy(M.jeton, yedek, 32); hata_yanit(c, 502, "Ayar yazılamadı"); return; }
	memset(yedek, 0, sizeof yedek);
	denetim(c->ip, "kisayol jetonu iptal edildi");
	static const char e[] = "{\"ok\":true,\"jeton_var\":false}";
	yanit(c, 200, "application/json", e, sizeof e - 1, NULL);
}

static void api_jeton_iptal(struct bag *c)
{
	if (izin(c, 0)) return;
	yeniden_parola(c, "KISAYOL JETONU IPTALI", jeton_iptal_devam);
}

static void api_oturumlar(struct bag *c)
{
	if (izin(c, 0)) return;
	struct jy y = { .azami = 65536 };
	jy_ham(&y, "{\"liste\":[");
	int ilk = 1;
	/* once bu oturum, sonra son etkinlige gore */
	int sira[OTURUM_AZAMI], n = 0;
	for (int i = 0; i < OTURUM_AZAMI; i++) if (oturum_gecerli(i)) sira[n++] = i;
	for (int a = 0; a < n; a++)
		for (int b2 = a + 1; b2 < n; b2++) {
			int x = sira[a], z = sira[b2];
			int xb = x == c->oturum, zb = z == c->oturum;
			if (zb > xb || (zb == xb && O[z].son_u > O[x].son_u)) { sira[a] = z; sira[b2] = x; }
		}
	for (int k = 0; k < n; k++) {
		struct oturum *o = &O[sira[k]];
		jy_ham(&y, ilk ? "{" : ",{");
		ilk = 0;
		jy_ham(&y, "\"kimlik\":");
		jy_dizge(&y, o->kimlik);
		jy_ham(&y, ",\"ip\":");
		jy_dizge(&y, o->ip);
		jy_ham(&y, ",\"cihaz\":");
		jy_dizge(&y, o->cihaz);
		double bk = o->son_m + bosta_sn() - simdi();
		jy_bicim(&y, ",\"baslangic\":%.3f,\"son\":%.3f,\"bu\":%s,\"bosta_kalan_sn\":%d}", o->baslangic_u, o->son_u,
			 sira[k] == c->oturum ? "true" : "false", bk > 0 ? (int)bk : 0);
	}
	jy_bicim(&y, "],\"jeton_var\":%s}", M.jeton_var ? "true" : "false");
	yanit_json(c, 200, &y, NULL);
}

static void oturum_kapat_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	char id[32];
	if (g_str("id", id, sizeof id) || strlen(id) != 12 || strspn(id, "0123456789abcdef") != 12) { hata_yanit(c, 400, NULL); return; }
	if (!strcmp(id, O[c->oturum].kimlik)) { hata_yanit(c, 400, "Bu oturum için Çıkış düğmesini kullanın"); return; }
	for (int i = 0; i < OTURUM_AZAMI; i++)
		if (oturum_gecerli(i) && !strcmp(O[i].kimlik, id)) {
			oturum_temizle(i);
			denetim(c->ip, "oturum kapatildi %s", id);
			ok_yanit(c);
			return;
		}
	hata_yanit(c, 404, "Oturum bulunamadı (zaten kapanmış olabilir)");
}

static void oturum_hepsi_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	int n = 0;
	for (int i = 0; i < OTURUM_AZAMI; i++) if (i != c->oturum && O[i].var) { n += oturum_gecerli(i); oturum_temizle(i); }
	int iptal = g_bool("jeton_iptal") == 1;
	if (iptal && M.jeton_var) {	/* v12: jeton_iptal_devam gibi; yazilamazsa geri al (yeniden baslatmada eski jeton donmesin) */
		uint8_t yedek[32];
		memcpy(yedek, M.jeton, 32);
		M.jeton_var = 0;
		memset(M.jeton, 0, 32);
		if (ayar_yaz()) {
			M.jeton_var = 1;
			memcpy(M.jeton, yedek, 32);
			memset(yedek, 0, sizeof yedek);
			denetim(c->ip, "diger %d oturum kapatildi; kisayol jetonu iptali YAZILAMADI", n);
			hata_yanit(c, 502, "Oturumlar kapandı ama kısayol jetonu iptali yazılamadı");
			return;
		}
		memset(yedek, 0, sizeof yedek);
	}
	denetim(c->ip, "diger %d oturum kapatildi%s", n, iptal ? " + kisayol jetonu iptal" : "");
	struct jy y = { .azami = 256 };
	jy_bicim(&y, "{\"ok\":true,\"kapanan\":%d,\"jeton_iptal\":%s}", n, iptal ? "true" : "false");
	yanit_json(c, 200, &y, NULL);
}

/* ------------------------------------------------------------------ durum / cihaz */
static void kamera_ozet_json(struct jy *y)
{
	int kalan = M.kamera_acik && M.kamera_bitis > simdi() ? (int)(M.kamera_bitis - simdi()) : 0;
	jy_bicim(y, "{\"acik\":%s,\"kalan_sn\":%d,\"kaynak\":\"ELENE · cihaz merkezi\",\"hata\":", M.kamera_acik ? "true" : "false", kalan);
	jy_dizge(y, M.kamera_hata);
	jy_bicim(y, ",\"izleyicisiz_kapanma_sn\":%d,\"fps\":%.1f,\"son_kare_ms\":", KAMERA_IZLEYICISIZ_SN, kamera_fps());
	if (M.son_kare >= 0 && M.kare[M.son_kare].b) jy_bicim(y, "%d", (int)((simdi() - M.kare[M.son_kare].t) * 1000));
	else jy_ham(y, "null");
	jy_bicim(y, ",\"izleyen\":%d}", izleyici_sayisi());
}

static void api_durum(struct bag *c)
{
	if (izin(c, 1)) return;
	struct cdurum d;
	cihaz_durumu(&d);
	struct jy y = { .azami = 65536 };
	jy_ham(&y, "{");
	durum_alanlari(&y, &d, 0);
	int kalan = M.kamera_acik && M.kamera_bitis > simdi() ? (int)(M.kamera_bitis - simdi()) : 0;
	jy_bicim(&y, ",\"bagli\":true,\"gecikme_ms\":0,\"kamera\":{\"acik\":%s,\"kalan_sn\":%d,\"kaynak\":\"ELENE · cihaz merkezi\",\"hata\":",
		 M.kamera_acik ? "true" : "false", kalan);
	jy_dizge(&y, M.kamera_hata);
	jy_ham(&y, "},\"makrolar\":[],\"istemler\":[],\"sekmeler\":[],\"merkez\":\"cihaz\",\"surum\":\"" SURUM "\"}");
	yanit_json(c, 200, &y, NULL);
}

struct kdurum { int var; char json[2048]; int kayit; long long bos_mb; char hata[256]; int son_kare_yas_gecerli; };

/* /tmp/kamera-kayit.json -> PC cihaz_kayit_durum ile ayni normallestirme */
static void kayit_durum_json(struct jy *y, struct kdurum *kd)
{
	static char b[2048];
	static struct jtok tk[256];
	memset(kd, 0, sizeof *kd);
	int t = 0;
	if (dosya_oku(kayit_durum_yol, b, sizeof b) > 0) t = json_ayristir(b, (int)strlen(b), tk, 256);
	if (t < 1 || tk[0].tip != J_NESNE) t = 0;
	int bos = !t || !tk[0].boyut;
	kd->var = !bos;
	jy_bicim(y, "{\"hizmet\":%s", bos ? "false" : "true");
	static const struct { const char *ad; char tip; } A[] = {
		{ "kayit", 'b' }, { "canli", 'b' }, { "fps", 'f' }, { "kareler", 'i' }, { "parca_sayi", 'i' }, { "toplam_mb", 'f' },
		{ "bos_mb", 'i' }, { "azami_parca", 'i' }, { "kayit_fps", 'f' }, { "kalkan", 'b' }, { "son_skor", 'i' }, { "parca_kare", 'i' } };
	double parca_sayi = -1, azami = -1, toplam = -1;
	for (size_t i = 0; t && i < sizeof A / sizeof A[0]; i++) {
		int k = j_al(b, tk, t, 0, A[i].ad);
		if (k < 0) continue;
		int bv = j_bool(b, &tk[k]);
		double v;
		if (A[i].tip == 'b') {
			if (bv >= 0) jy_bicim(y, ",\"%s\":%s", A[i].ad, bv ? "true" : "false");
			else if (!j_sayi(b, &tk[k], &v)) jy_bicim(y, ",\"%s\":%s", A[i].ad, v != 0 ? "true" : "false");
			if (!strcmp(A[i].ad, "kayit")) kd->kayit = bv == 1;
			continue;
		}
		if (bv >= 0 || j_sayi(b, &tk[k], &v)) continue;
		if (A[i].tip == 'i') jy_bicim(y, ",\"%s\":%lld", A[i].ad, (long long)v);
		else jy_bicim(y, ",\"%s\":%g", A[i].ad, v);
		if (!strcmp(A[i].ad, "parca_sayi")) parca_sayi = (long long)v;
		if (!strcmp(A[i].ad, "azami_parca")) azami = (long long)v;
		if (!strcmp(A[i].ad, "toplam_mb")) toplam = v;
		if (!strcmp(A[i].ad, "bos_mb")) kd->bos_mb = (long long)v;
	}
	char p[32] = "", h[256] = "", s[40] = "";
	int k;
	if (t && (k = j_al(b, tk, t, 0, "parca")) >= 0 && !j_dizge_al(b, &tk[k], p, sizeof p) &&
	    !(strlen(p) == 18 && strspn(p, "0123456789") == 8 && p[8] == '/' && strspn(p + 9, "0123456789") == 4 && !strcmp(p + 13, ".mjpg")))
		p[0] = 0;
	if (t && (k = j_al(b, tk, t, 0, "hata")) >= 0) {
		if (j_dizge_al(b, &tk[k], h, sizeof h)) h[0] = 0;
		h[200] = 0;
	}
	if (t && (k = j_al(b, tk, t, 0, "surum")) >= 0) j_dizge_al(b, &tk[k], s, sizeof s);
	snprintf(kd->hata, sizeof kd->hata, "%s", h);
	jy_ham(y, ",\"parca\":");
	jy_dizge(y, p);
	jy_ham(y, ",\"hata\":");
	jy_dizge(y, h);
	jy_ham(y, ",\"hizmet_surum\":");
	jy_ham(y, !strcmp(s, "kamera-kayit-v2") ? "\"v2\"" : !bos ? "\"v1\"" : "null");
	jy_ham(y, ",\"doluluk\":");
	if (azami > 0 && parca_sayi >= 0) jy_bicim(y, "%.3f", parca_sayi / azami < 1 ? parca_sayi / azami : 1.0); else jy_ham(y, "null");
	jy_ham(y, ",\"tahmini_sinir_mb\":");
	if (parca_sayi > 0 && azami > 0 && toplam >= 0) jy_bicim(y, "%.1f", toplam / parca_sayi * azami); else jy_ham(y, "null");
	jy_ham(y, ",\"saklama_saat\":");
	if (azami > 0) jy_bicim(y, "%.1f", azami / 60); else jy_ham(y, "null");
	jy_ham(y, "}");
}

static void api_kayit_durum(struct bag *c)
{
	if (izin(c, 0)) return;
	struct kdurum kd;
	struct jy y = { .azami = 8192 };
	kayit_durum_json(&y, &kd);
	yanit_json(c, 200, &y, NULL);
}

static void uyari(struct jy *y, int *ilk, const char *sev, const char *m)
{
	jy_ham(y, *ilk ? "{\"seviye\":" : ",{\"seviye\":");
	*ilk = 0;
	jy_dizge(y, sev);
	jy_ham(y, ",\"metin\":");
	jy_dizge(y, m);
	jy_ham(y, "}");
}

static void api_cihaz(struct bag *c)
{
	if (izin(c, 0)) return;
	struct cdurum d;
	cihaz_durumu(&d);
	struct jy y = { .azami = 131072 };
	jy_ham(&y, "{\"cihaz\":{");
	durum_alanlari(&y, &d, 1);
	{	/* v5: hizmetler, surumler, ozellik; v12: cihaz_durumu'nun taramasi */
		const struct hizmet *hz = d.hz;
		jy_ham(&y, ",\"hizmetler\":{");
		for (int i = 0; i < HZ_N; i++) {
			jy_bicim(&y, "%s\"%s\":{\"pid\":%d,\"ad\":", i ? "," : "", HZ_AD[i], hz[i].pid);
			jy_dizge(&y, hz[i].ad);
			jy_ham(&y, "}");
		}
		char pb[2048], ver[64] = "";
		static struct jtok ptk[256];
		long pn = dosya_oku(panel_durum_yol, pb, sizeof pb);
		if (pn > 0) {
			int t = json_ayristir(pb, (int)pn, ptk, 256);
			int i = t > 0 ? j_al(pb, ptk, t, 0, "ver") : -1;
			if (i < 0 || j_dizge_al(pb, &ptk[i], ver, sizeof ver)) ver[0] = 0;
		}
		jy_ham(&y, "},\"surumler\":{\"panel\":");
		if (ver[0]) jy_dizge(&y, ver); else jy_ham(&y, "null");
		jy_ham(&y, ",\"merkez\":\"" SURUM_TAM "\"},\"ozellik\":{\"zaman\":true,\"foto\":true,\"telegram\":{");	/* v10 */
		long long ofs = yas_ms_m(tg_ofs_yol);
		jy_bicim(&y, "\"jeton\":%s,\"son_mesaj_sn\":", dosya_var(tg_jeton_yol) ? "true" : "false");
		sayi_null(&y, ofs >= 0 ? ofs / 1000 : -1);
		int foto = 0;
		DIR *fd = opendir(foto_dizin);
		struct dirent *fe;
		while (fd && (fe = readdir(fd)) && foto < 9999) {
			size_t n = strlen(fe->d_name);
			if (fe->d_name[0] != '.' && n > 4 && (!strcasecmp(fe->d_name + n - 4, ".jpg") || (n > 5 && !strcasecmp(fe->d_name + n - 5, ".jpeg"))))
				foto++;
		}
		if (fd) closedir(fd);
		jy_bicim(&y, "},\"gemini_anahtar\":%s,\"asistan_kapali\":%s,\"cerceve\":{\"acik\":%s,\"foto\":%d},\"kamera_kayit\":",
			 dosya_var(gemini_yol) ? "true" : "false", dosya_var(asistan_kapali_yol) ? "true" : "false",
			 dosya_var(cerceve_bayrak_yol) ? "true" : "false", foto);
		char kb[2048];
		static struct jtok ktk[256];
		long kn = dosya_oku(kayit_durum_yol, kb, sizeof kb);
		while (kn > 0 && (kb[kn - 1] == '\n' || kb[kn - 1] == ' ')) kb[--kn] = 0;
		if (kn > 0 && kb[0] == '{' && json_ayristir(kb, (int)kn, ktk, 256) > 0) jy_ham(&y, kb);
		else jy_ham(&y, "null");
		jy_ham(&y, "}");
	}
	jy_ham(&y, ",\"surum\":\"" SURUM_TAM "\",\"bagli\":true},\"gecikme_ms\":0,\"kayit\":");
	struct kdurum kd;
	kayit_durum_json(&y, &kd);
	jy_ham(&y, ",\"kopru\":{\"surum\":\"" SURUM_TAM "\",\"yetenek\":[\"CIHAZ_MERKEZI\",\"KALKAN\",\"KAYIT_IDX\",\"KAYIT_KARE\","
		   "\"KAYIT_OKU_ARALIK\",\"KAYIT_OLAYLAR\",\"MIC_AKIS\",\"PLAY_AKIS\",\"RTSP\",\"SAGLIK\"],\"v7\":true,\"v8\":true,"
		   "\"gecikme\":{\"son\":0,\"ort\":0,\"p95\":0,\"kayip\":0,\"gecmis\":[]}}");
	jy_bicim(&y, ",\"merkez\":{\"surum\":\"" SURUM "\",\"calisma_sn\":%d,\"python\":\"C (cihaz içi)\",\"oturum\":%d,\"kamera\":",
		 (int)(simdi() - M.baslangic), oturum_sayisi());
	kamera_ozet_json(&y);
	jy_bicim(&y, ",\"plan_acik\":%s,\"rss_kb\":%ld,\"baglanti\":%d}", M.plan_acik ? "true" : "false", rss_kb(),
		 (int)({ int n = 0; for (int i = 0; i < AZAMI_BAG; i++) n += B[i].fd >= 0; n; }));
	jy_ham(&y, ",\"uyarilar\":[");
	int ilk = 1;
	char m[160];
	if (!strcmp(hoparlor_durumu(), "dogrulanmadi")) uyari(&y, &ilk, "bilgi", "Panel hoparlörü henüz doğrulanmadı: bas-konuş duyulmayabilir");
	if (kalkan()) uyari(&y, &ilk, "bilgi", "Panelde gizlilik kalkanı açık: kamera, kayıt ve ses kapalı");
	long long bos = d.sd_bos >= 0 ? d.sd_bos : kd.bos_mb;
	if ((d.sd_bos >= 0 || kd.var) && bos >= 0 && bos < 1024) {
		snprintf(m, sizeof m, "SD kartta %lld MB boş: eski kayıtlar daha hızlı silinir", bos);
		uyari(&y, &ilk, "orta", m);
	}
	if (d.sd == 1) uyari(&y, &ilk, "kotu", "SD kart salt okunur bağlı: kayıt yazılamaz");
	else if (d.sd == 0) uyari(&y, &ilk, "orta", "SD kart bağlı değil");
	long mem = atol(d.mem);
	if (mem > 0 && mem < 8192) { snprintf(m, sizeof m, "Boş bellek az (%ld MB)", mem / 1024); uyari(&y, &ilk, "orta", m); }
	if (strcmp(d.sicak, "null") && atol(d.sicak) > 75000) uyari(&y, &ilk, "orta", "Cihaz sıcak (75 °C üstü)");
	if (!kd.var) uyari(&y, &ilk, "orta", "Kamera-kayıt hizmeti durum yazmıyor");
	if ((M.kamera_acik || kd.kayit) && d.son_kare_yas > 5000) {
		snprintf(m, sizeof m, "Son kamera karesi %lld sn önce", d.son_kare_yas / 1000);
		uyari(&y, &ilk, "orta", m);
	}
	if (kd.hata[0]) { snprintf(m, sizeof m, "Kamera hizmeti: %.120s", kd.hata); uyari(&y, &ilk, "orta", m); }
	jy_ham(&y, "]}");
	yanit_json(c, 200, &y, NULL);
}

/* ------------------------------------------------------------------ kamera API */
static void kamera_ac_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	long long dk = 5;
	int r = g_int("dakika", &dk);
	if (r == -1 || dk < 1 || dk > 30) { hata_yanit(c, 400, NULL); return; }
	int uzatma = M.kamera_acik;
	int k = kamera_ac((int)dk);
	if (k == -2) {
		denetim(c->ip, "KAMERA ACMA ENGELLENDI (panelde gizlilik kalkani acik)");
		hata_yanit(c, 409, "Panelde gizlilik kalkanı açık; kamera uzaktan açılamaz");
		return;
	}
	if (k) { hata_yanit(c, 502, "Kamera bayrağı yazılamadı"); return; }
	denetim(c->ip, "KAMERA %s %lld dk%s", uzatma ? "SURESI UZATILDI" : "ACILDI", dk, dog_ek);
	struct jy y = { .azami = 128 };
	jy_bicim(&y, "{\"ok\":true,\"kalan_sn\":%lld}", dk * 60);
	yanit_json(c, 200, &y, NULL);
}

static void api_kamera_ac(struct bag *c)
{
	if (izin(c, 0)) return;
	long long dk = 5;
	if (g_int("dakika", &dk) == -1 || dk < 1 || dk > 30) { hata_yanit(c, 400, NULL); return; }
	if (kalkan()) {
		denetim(c->ip, "KAMERA ACMA ENGELLENDI (panelde gizlilik kalkani acik)");
		hata_yanit(c, 409, "Panelde gizlilik kalkanı açık; kamera uzaktan açılamaz");
		return;
	}
	hassas(c, "KAMERA ACMA", kamera_ac_devam);
}

static void api_kamera_kapat(struct bag *c)
{
	if (izin(c, 0)) return;
	char n[32] = "";
	g_str("neden", n, sizeof n);
	int acik = M.kamera_acik;
	kamera_kapat(NULL, c->ip);
	if (!acik) { char b[64]; if (dosya_oku(kamera_bayrak, b, sizeof b) >= 0 && !strncmp(b, "merkez", 6)) unlink(kamera_bayrak); }
	denetim(c->ip, "kamera kapatildi%s", !strcmp(n, "hareketsizlik") ? " (hareketsizlik)" : "");
	ok_yanit(c);
}

static void api_kamera_durum(struct bag *c)
{
	if (izin(c, 0)) return;
	struct jy y = { .azami = 2048 };
	kamera_ozet_json(&y);
	yanit_json(c, 200, &y, NULL);
}

static void api_kamera_akis(struct bag *c)
{
	if (izin(c, 0)) return;
	if (!M.kamera_acik) { hata_yanit(c, 423, "Kamera kapalı"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık"); return; }
	if (izleyici_sayisi() >= AZAMI_IZLEYICI) { hata_yanit(c, 503, "En fazla 3 canlı izleyici"); return; }
	c->keepalive = 0;
	yanit_bas(c, 200, "multipart/x-mixed-replace; boundary=kare", -1, NULL);
	c->tur = B_CANLI;
	c->son_seq = 0;
	if (M.son_kare >= 0) kare_bolum_gonder(c);
}

/* v8: CEK bicimli canli kare. son=<sira>'dan yeni kare varsa tek JPEG (kopyasiz, kref), yoksa 204. Istemci bir kareyi
 * aldiktan sonra yenisini ister: yavas/aktarici baglantida tamponlarda kare birikmez. */
static void api_kamera_kare(struct bag *c)
{
	if (izin(c, 0)) return;
	if (!M.kamera_acik) { hata_yanit(c, 423, "Kamera kapalı"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık"); return; }
	M.kare_istek_t = simdi();
	char sb[16] = "";
	unsigned long son = 0;
	if (sorgu_al(c->r.sorgu, "son", sb, sizeof sb) == 1) son = strtoul(sb, NULL, 10);
	if (M.son_kare < 0 || !M.kare[M.son_kare].b || M.kare[M.son_kare].seq <= son) {
		yanit(c, 204, NULL, NULL, 0, NULL);
		return;
	}
	struct kare *k = &M.kare[M.son_kare];
	int kalan = M.kamera_bitis > simdi() ? (int)(M.kamera_bitis - simdi()) : 0;
	char ek[128];
	snprintf(ek, sizeof ek, "X-Seq: %u\r\nX-Yas-Ms: %d\r\nX-Kalan-Sn: %d\r\n", k->seq, (int)((simdi() - k->t) * 1000), kalan);
	yanit_bas(c, 200, "image/jpeg", (long long)k->n, ek);
	c->tur = B_YANIT;
	if (c->head || c->kapat_sonra) return;
	c->kref = k;
	k->ref++;
	c->kofs = 0;
}

static int kayit_izin_aktif(int o) { return o >= 0 && izin_aktif(o, O[o].kayit_bitis); }

static void kayit_izin_ver(int o)
{
	O[o].kayit_bitis = simdi() + KAYIT_IZIN_SN;
	O[o].oyn_var = 0;
	O[o].kucuk_n = 0;
}

/* v2 giris noktalari (oynat/kucuk/indir/anlik kayit): tek_giris'te kayit izni yoksa verilir (denetime yazilir), varsa
 * tam sureye tazelenir. Akis donguleri bunu DEGIL kayit_izin_aktif/oynatma_gecerli'yi kullanir (Kilitle hemen etkili). */
static void kayit_izin_oto(struct bag *c)
{
	if (M.siki || !oturum_gecerli(c->oturum)) return;
	if (kayit_izin_aktif(c->oturum)) { O[c->oturum].kayit_bitis = simdi() + KAYIT_IZIN_SN; return; }
	kayit_izin_ver(c->oturum);
	denetim(c->ip, "KAYIT IZNI VERILDI %d dk" TEK_EK, KAYIT_IZIN_SN / 60);
}

static void api_anlik(struct bag *c)
{
	if (izin(c, 0)) return;
	char k[16] = "";
	g_str("kaynak", k, sizeof k);
	if (!strcmp(k, "canli")) {
		if (!M.kamera_acik) { hata_yanit(c, 423, "Kamera kapalı"); return; }
		denetim(c->ip, "kamera anlik goruntu tarayiciya indirildi");
	} else if (!strcmp(k, "kayit")) {
		kayit_izin_oto(c);
		if (!kayit_izin_aktif(c->oturum)) { hata_yanit(c, 423, NULL); return; }
		denetim(c->ip, "kayit karesi tarayiciya indirildi");
	} else { hata_yanit(c, 400, NULL); return; }
	ok_yanit(c);
}

/* ------------------------------------------------------------------ kayitlar (SD, salt okuma) */
/* kayit_sd_v1.h islevleri bir dosya tanimlayicisina yazar: bellek dosyasi (memfd) uzerinden geri okunur */
static char *bellek_cagir(int (*f)(int, const char *, const char *), const char *a, const char *b, size_t *n)
{
	int fd = memfd_create("kole", MFD_CLOEXEC);
	if (fd < 0) return NULL;
	f(fd, a, b);
	off_t L = lseek(fd, 0, SEEK_END);
	char *m = NULL;
	if (L >= 0 && L < 8 * 1024 * 1024 && (m = malloc((size_t)L + 1))) {
		if (pread(fd, m, (size_t)L, 0) != L) { free(m); m = NULL; }
		else { m[L] = 0; *n = (size_t)L; }
	}
	close(fd);
	return m;
}

static int liste_sar(int fd, const char *a, const char *b) { (void)a; (void)b; return kayit_liste(fd); }
static int olay_sar(int fd, const char *a, const char *b) { return kayit_olaylar(fd, a, b); }

struct kayit_oge { char ad[20]; long long bayt; };
struct puan { char ad[20]; int skor, kare, tepe, tepe_sn; long long bayt; };

static int ad_karsilastir_azalan(const void *a, const void *b) { return strcmp(((const struct kayit_oge *)b)->ad, ((const struct kayit_oge *)a)->ad); }

/* donus: oge sayisi (-1 hata); *o malloc */
static int kayit_listesi(struct kayit_oge **o)
{
	size_t n = 0;
	char *m = bellek_cagir(liste_sar, NULL, NULL, &n);
	if (!m) return -1;
	int k = 0, cap = 0;
	struct kayit_oge *l = NULL;
	char *s = m, *sat;
	int son = 0;
	while ((sat = strsep(&s, "\n"))) {
		if (!strcmp(sat, "SON")) { son = 1; break; }
		char ad[32];
		long long b;
		if (sscanf(sat, "%31s %lld", ad, &b) != 2 || !kayit_ad_gecerli(ad) || b < 0 || b > KAYIT_AZAMI) continue;
		if (k == cap) {
			cap = cap ? cap * 2 : 64;
			if (cap > 16384) break;
			struct kayit_oge *nl = realloc(l, (size_t)cap * sizeof *l);
			if (!nl) break;
			l = nl;
		}
		snprintf(l[k].ad, sizeof l[k].ad, "%s", ad);
		l[k++].bayt = b;
	}
	free(m);
	if (!son) { free(l); return -1; }
	if (k) qsort(l, (size_t)k, sizeof *l, ad_karsilastir_azalan);
	*o = l;
	return k;
}

static void api_kayit_liste(struct bag *c)
{
	if (izin(c, 0)) return;
	struct kayit_oge *l = NULL;
	int n = kayit_listesi(&l);
	if (n < 0) { hata_yanit(c, 502, "Kayıt listesi okunamadı"); return; }
	struct jy y = { .azami = CIKIS_AZAMI / 2 };
	jy_ham(&y, "{\"liste\":[");
	for (int i = 0; i < n; i++)
		jy_bicim(&y, "%s{\"ad\":\"%s\",\"bayt\":%lld,\"gun\":\"%.8s\",\"saat\":\"%.2s\",\"dakika\":\"%.2s\"}", i ? "," : "",
			 l[i].ad, l[i].bayt, l[i].ad, l[i].ad + 9, l[i].ad + 11);
	jy_bicim(&y, "],\"izin_kalan_sn\":%d}", c->oturum >= 0 ? izin_kalan(c->oturum, O[c->oturum].kayit_bitis) : 0);
	free(l);
	yanit_json(c, 200, &y, NULL);
}

static void elle_kapali_isaret(int kapatildi);	/* v13 (B5) */

static void api_kayit_mod_uygula(struct bag *c, int acik)
{
	if (acik && kalkan()) {
		denetim(c->ip, "SD KAYIT ACMA ENGELLENDI (panelde gizlilik kalkani acik)");
		hata_yanit(c, 409, "Panelde gizlilik kalkanı açık; kayıt uzaktan açılamaz");
		return;
	}
	char yol[600];
	mkdir(kok, 0755);
	snprintf(yol, sizeof yol, "%s/kayit-acik", kok);
	bayrak_yaz(yol, acik);
	sync();
	if (dosya_var(yol) != acik) {
		denetim(c->ip, "SD kayit %s BASARISIZ (SD)", acik ? "acma" : "kapatma");
		hata_yanit(c, 502, "Cihaz kayıt isteğini onaylamadı");
		return;
	}
	elle_kapali_isaret(!acik);	/* v13 (B5) */
	if (acik) denetim(c->ip, "SD KAYIT ACILDI%s", dog_ek);
	else denetim(c->ip, "SD kayit kapatildi");
	struct jy y = { .azami = 64 };
	jy_bicim(&y, "{\"ok\":true,\"kayit\":%s}", acik ? "true" : "false");
	yanit_json(c, 200, &y, NULL);
}

static void kayit_mod_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	api_kayit_mod_uygula(c, 1);
}

static void api_kayit_mod(struct bag *c)
{
	if (izin(c, 0)) return;
	int acik = g_bool("acik");
	if (acik < 0) { hata_yanit(c, 400, NULL); return; }
	if (!acik) { api_kayit_mod_uygula(c, 0); return; }
	if (kalkan()) { api_kayit_mod_uygula(c, 1); return; }
	hassas(c, "SD KAYIT ACMA", kayit_mod_devam);
}

static void api_kayit_izin_get(struct bag *c)
{
	if (izin(c, 0)) return;
	struct jy y = { .azami = 64 };
	jy_bicim(&y, "{\"kalan_sn\":%d}", izin_kalan(c->oturum, O[c->oturum].kayit_bitis));
	yanit_json(c, 200, &y, NULL);
}

static void kayit_izin_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	kayit_izin_ver(c->oturum);
	denetim(c->ip, "KAYIT IZNI VERILDI %d dk%s", KAYIT_IZIN_SN / 60, dog_ek);
	struct jy y = { .azami = 64 };
	jy_bicim(&y, "{\"ok\":true,\"kalan_sn\":%d}", KAYIT_IZIN_SN);
	yanit_json(c, 200, &y, NULL);
}

static void api_kayit_izin_post(struct bag *c)
{
	if (izin(c, 0)) return;
	hassas(c, "KAYIT IZNI", kayit_izin_devam);
}

/* 0 ok, -1 yanit gonderildi */
static int oynatma_param(struct bag *c, struct oynatma *o)
{
	char ad[32];
	long long hiz = 1, bas = 0;
	if (g_str("ad", ad, sizeof ad) || !kayit_ad_gecerli(ad)) { hata_yanit(c, 400, "Geçersiz kayıt adı"); return -1; }
	int rh = g_int("hiz", &hiz), rb = g_int("bas", &bas);
	if (rh == -1 || rb == -1 || (hiz != 1 && hiz != 2 && hiz != 4) || bas < 0 || bas > 59) {
		hata_yanit(c, 400, "Hız 1/2/4, başlangıç 0..59 sn olmalı");
		return -1;
	}
	memset(o, 0, sizeof *o);
	snprintf(o->ad, sizeof o->ad, "%s", ad);
	o->hiz = (int)hiz;
	o->bas = (int)bas;
	rastgele(&o->belirtec, sizeof o->belirtec);
	return 0;
}

static void api_kayit_oynat(struct bag *c)
{
	if (izin(c, 0)) return;
	struct oynatma o;
	if (oynatma_param(c, &o)) return;
	kayit_izin_oto(c);
	if (!kayit_izin_aktif(c->oturum)) { hata_yanit(c, 423, "Kayıt izni yok ya da süresi doldu; parolanızı girin"); return; }
	O[c->oturum].oyn = o;
	O[c->oturum].oyn_var = 1;
	char bas_m[16] = "";
	if (o.bas) snprintf(bas_m, sizeof bas_m, " +%dsn", o.bas);
	denetim(c->ip, "KAYIT ACILDI %s%s%s", o.ad, o.hiz == 2 ? " 2x" : o.hiz == 4 ? " 4x" : "", bas_m);
	struct jy y = { .azami = 128 };
	jy_bicim(&y, "{\"ok\":true,\"akis\":\"/api/kayit/akis\",\"kalan_sn\":%d}", izin_kalan(c->oturum, O[c->oturum].kayit_bitis));
	yanit_json(c, 200, &y, NULL);
}

static void kayit_ac_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	struct oynatma o;
	if (oynatma_param(c, &o)) return;
	kayit_izin_ver(c->oturum);
	O[c->oturum].oyn = o;
	O[c->oturum].oyn_var = 1;
	denetim(c->ip, "KAYIT ACILDI %s%s", o.ad, dog_ek);
	struct jy y = { .azami = 128 };
	jy_bicim(&y, "{\"ok\":true,\"akis\":\"/api/kayit/akis\",\"kalan_sn\":%d}", KAYIT_IZIN_SN);
	yanit_json(c, 200, &y, NULL);
}

static void api_kayit_ac(struct bag *c)
{
	if (izin(c, 0)) return;
	hassas(c, "KAYIT ACMA", kayit_ac_devam);
}

static void api_kayit_kapat(struct bag *c)
{
	if (izin(c, 0)) return;
	int de = g_bool("izin_de") == 1;
	if (de) { O[c->oturum].kayit_bitis = 0; O[c->oturum].oyn_var = 0; }
	else O[c->oturum].oyn_var = 0;
	denetim(c->ip, de ? "kayit izni kilitlendi" : "kayit oynatma kapatildi");
	ok_yanit(c);
}

static int oynatma_gecerli(struct bag *c)
{
	int o = c->oyn_oturum;
	return kayit_izin_aktif(o) && O[o].oyn_var && O[o].oyn.belirtec == c->oyn_belirtec && !kalkan();
}

static void oynatma_birak(struct bag *c)
{
	free(c->oyn_k);
	c->oyn_k = NULL;
	free(c->oyn_tampon);
	c->oyn_tampon = NULL;
	if (c->oyn_fd >= 0) close(c->oyn_fd);
	c->oyn_fd = -1;
	if (M.oyn_bag == (int)(c - B)) M.oyn_bag = -1;
	int o = c->oyn_oturum;
	if (o >= 0 && O[o].var && O[o].oyn_var && O[o].oyn.belirtec == c->oyn_belirtec) O[o].oyn_var = 0;
	c->oyn_oturum = -1;
}

/* v13 (inceleme B2): kare arasi bekleme idx ms'den (kamera-kayit v11: normalde 2, harekette 8 kare/sn); ms yoksa ya da
 * artmiyorsa eski 1/KAYIT_FPS; en fazla 1 sn */
static double oyn_ara(const struct kidx *k, int n, int i, int hiz)
{
	if (hiz < 1) hiz = 1;
	if (i >= 0 && i + 1 < n && k[i].ms >= 0 && k[i + 1].ms > k[i].ms) {
		double d = (k[i + 1].ms - k[i].ms) / 1000.0 / hiz;
		return d < 1.0 ? d : 1.0;
	}
	return 1.0 / KAYIT_FPS / hiz;
}

/* v13: bas. saniyeden ilk kare: ms varsa ms >= bas*1000 olan ilk kare (yoksa son kare), ms hic yoksa bas*KAYIT_FPS */
static int oyn_bas_kare(const struct kidx *k, int n, int bas)
{
	int ms_var = 0;
	for (int i = 0; i < n; i++) {
		if (k[i].ms < 0) continue;
		ms_var = 1;
		if (k[i].ms >= bas * 1000) return i;
	}
	if (ms_var) return n - 1;
	return bas * KAYIT_FPS < n - 1 ? bas * KAYIT_FPS : n - 1;
}

static void api_kayit_akis(struct bag *c)
{
	if (izin(c, 0)) return;
	struct oturum *ot = &O[c->oturum];
	if (!kayit_izin_aktif(c->oturum) || !ot->oyn_var || ot->oyn.kullanildi) { hata_yanit(c, 423, NULL); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık"); return; }
	if (M.oyn_bag >= 0) {
		struct bag *e = &B[M.oyn_bag];
		if (e->fd >= 0 && e->tur == B_KAYIT && oynatma_gecerli(e)) { hata_yanit(c, 409, NULL); return; }
		if (e->fd >= 0) bag_kapat(e, "yeni oynatma");
		M.oyn_bag = -1;
	}
	ot->oyn.kullanildi = 1;
	unsigned long long boyut;
	int tarandi;
	int n = idx_yukle(ot->oyn.ad, &boyut, &tarandi);
	char tam[600];
	snprintf(tam, sizeof tam, "%s/%s", kok, ot->oyn.ad);
	int fd = n > 0 ? open(tam, O_RDONLY | O_CLOEXEC) : -1;
	if (n <= 0 || fd < 0 || boyut > KAYIT_AZAMI || !(c->oyn_k = malloc((size_t)n * sizeof(struct kidx))) ||
	    !(c->oyn_tampon = malloc(KARE_AZAMI))) {
		if (fd >= 0) close(fd);
		free(c->oyn_k);
		c->oyn_k = NULL;
		ot->oyn_var = 0;
		hata_yanit(c, 502, "Kayıt okunamadı");
		return;
	}
	memcpy(c->oyn_k, kx, (size_t)n * sizeof(struct kidx));
	c->oyn_n = n;
	c->oyn_fd = fd;
	c->oyn_boyut = boyut;
	c->oyn_hiz = ot->oyn.hiz;
	c->oyn_belirtec = ot->oyn.belirtec;
	c->oyn_oturum = c->oturum;
	c->oyn_i = oyn_bas_kare(c->oyn_k, n, ot->oyn.bas);	/* v13 (B2) */
	c->oyn_sonraki = simdi();
	M.oyn_bag = (int)(c - B);
	c->keepalive = 0;
	yanit_bas(c, 200, "multipart/x-mixed-replace; boundary=kare", -1, NULL);
	c->tur = B_KAYIT;
}

/* oynatma: zamani gelen kareyi kuyruga koy; bitince baglanti kapanir */
static void oynatma_adim(struct bag *c)
{
	if (cikis_bekleyen(c) || simdi() < c->oyn_sonraki) return;
	if (!oynatma_gecerli(c)) { bag_kapat(c, "izin/oynatma bitti"); return; }
	for (; c->oyn_i < c->oyn_n; c->oyn_i++) {
		struct kidx *k = &c->oyn_k[c->oyn_i];
		if (k->len < 4 || k->len > KARE_AZAMI) continue;
		ssize_t r = pread(c->oyn_fd, c->oyn_tampon, k->len, k->ofs);
		if (r != (ssize_t)k->len || c->oyn_tampon[0] != 0xff || c->oyn_tampon[1] != 0xd8) continue;
		int sn = k->ms >= 0 ? k->ms / 1000 : (c->oyn_i / KAYIT_FPS < 59 ? c->oyn_i / KAYIT_FPS : 59);
		char h[256];
		int n = snprintf(h, sizeof h, "--kare\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\nX-Kare: %d\r\nX-Ilerleme: %.3f\r\n"
				 "X-Hiz: %d\r\nX-Sn: %d\r\n\r\n", k->len, c->oyn_i,
				 c->oyn_boyut ? (double)(k->ofs + k->len) / (double)c->oyn_boyut : 1.0, c->oyn_hiz, sn);
		if (cikis_ekle(c, h, (size_t)n) || cikis_ekle(c, c->oyn_tampon, k->len) || cikis_ekle(c, "\r\n", 2)) { c->kapat_sonra = 1; return; }
		c->oyn_i++;
		c->oyn_sonraki = simdi() + oyn_ara(c->oyn_k, c->oyn_n, c->oyn_i - 1, c->oyn_hiz);	/* v13 (B2): idx ms */
		return;
	}
	c->kapat_sonra = 1;	/* dakika bitti */
}

static int ad_param(struct bag *c, char *ad)
{
	int r = sorgu_al(c->r.sorgu, "ad", ad, 32);
	if (r != 1 || !kayit_ad_gecerli(ad)) { hata_yanit(c, 400, "Geçersiz kayıt adı"); return -1; }
	return 0;
}

/* oturumda bu anahtar ilk kez mi (denetim tekrarini onler; kucuk resim listesiyle ortak). v12: son 24 anahtarlik halka
 * (v11 dizi dolunca her cagriyi "ilk" sayiyordu); kucuk_n toplam eklenen sayisi */
static int oturumda_ilk(struct bag *c, const char *anahtar)
{
	struct oturum *o = &O[c->oturum];
	int n = o->kucuk_n < 24 ? o->kucuk_n : 24;
	for (int k = 0; k < n; k++) if (!strcmp(o->kucuk[k], anahtar)) return 0;
	snprintf(o->kucuk[o->kucuk_n++ % 24], 12, "%s", anahtar);
	if (o->kucuk_n >= 48) o->kucuk_n -= 24;	/* tasma olmasin; halka konumu ayni */
	return 1;
}

static void api_kayit_kucuk(struct bag *c)
{
	if (izin(c, 0)) return;
	char ad[32];
	if (ad_param(c, ad)) return;
	kayit_izin_oto(c);
	if (!kayit_izin_aktif(c->oturum)) { hata_yanit(c, 423, "Küçük resimler için kayıt kilidini parolayla açın"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık"); return; }
	unsigned long long boyut;
	int tarandi;
	int n = idx_yukle(ad, &boyut, &tarandi);
	if (n <= 0) { hata_yanit(c, 502, "Küçük resim okunamadı"); return; }
	int i = temsilci(n);
	char tam[600];
	snprintf(tam, sizeof tam, "%s/%s", kok, ad);
	int fd = open(tam, O_RDONLY | O_CLOEXEC);
	if (fd < 0 || i < 0 || i >= n || kx[i].len > sizeof tarama_t) { if (fd >= 0) close(fd); hata_yanit(c, 502, "Küçük resim okunamadı"); return; }
	ssize_t r = pread(fd, tarama_t, kx[i].len, kx[i].ofs);
	close(fd);
	if (r != (ssize_t)kx[i].len || tarama_t[0] != 0xff || tarama_t[1] != 0xd8) { hata_yanit(c, 502, "Küçük resim okunamadı"); return; }
	char anahtar[12];
	snprintf(anahtar, sizeof anahtar, "%.11s", ad);
	if (oturumda_ilk(c, anahtar)) denetim(c->ip, "KAYIT KUCUK RESIM %.8s %.2s:xx", ad, ad + 9);	/* v12: ortak halka */
	yanit(c, 200, "image/jpeg", tarama_t, kx[i].len, NULL);
	memset(tarama_t, 0, kx[i].len);
}

static void api_kayit_indir(struct bag *c)
{
	if (izin(c, 0)) return;
	char ad[32];
	if (ad_param(c, ad)) return;
	kayit_izin_oto(c);
	if (!kayit_izin_aktif(c->oturum)) { hata_yanit(c, 423, "Dışa aktarmak için kayıt kilidini parolayla açın"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık"); return; }
	if (M.yan_bag >= 0) { hata_yanit(c, 503, NULL); return; }
	char tam[600];
	snprintf(tam, sizeof tam, "%s/%s", kok, ad);
	int fd = open(tam, O_RDONLY | O_CLOEXEC);
	struct stat st;
	if (fd < 0 || fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size <= 0 || st.st_size > KAYIT_AZAMI) {
		if (fd >= 0) close(fd);
		hata_yanit(c, 502, "Kayıt okunamadı");
		return;
	}
	denetim(c->ip, "KAYIT DISA AKTARILDI %s (%lld bayt, yalniz tarayiciya)", ad, (long long)st.st_size);
	char ek[160];
	snprintf(ek, sizeof ek, "Content-Disposition: attachment; filename=\"kole-%.8s-%.4s.mjpg\"\r\n", ad, ad + 9);
	c->keepalive = 0;
	yanit_bas(c, 200, "video/x-motion-jpeg", (long long)st.st_size, ek);
	c->dosya_fd = fd;
	c->dosya_kalan = (unsigned long long)st.st_size;
	c->hiz = AKTARIM_HIZ;
	c->hiz_t0 = simdi();
	c->giden = 0;
	c->tur = B_INDIR;
	M.yan_bag = (int)(c - B);
}

/* ------------------------------------------------------------------ v10: zaman atlamali (time-lapse) + fotograflar
 * SD SALT OKUMA (yalniz zaman-acik / zaman-aralik bayraklari yazilir, panel 0.55 ile ayni). Oynatma CEK bicimi: istemci
 * her kareyi ister -> Tailscale aktaricisinda kare birikmez (v8 canli kare ile ayni gerekce). */
#define ZAMAN_ONCE 131072u	/* once bu kadar okunur; kare siniri bulunamazsa KARE_AZAMI'ye kadar */
#define ZAMAN_ATLA_AZAMI 7
#define FOTO_AZAMI 400
#define FOTO_TARAMA_AZAMI 2000	/* v16 (B7): klasor basina stat edilen en cok ad */

static int zaman_gun_gecerli(const char *g)
{
	if (strlen(g) != 8 || g[0] != '2' || g[1] != '0') return 0;
	for (int i = 0; i < 8; i++) if (g[i] < '0' || g[i] > '9') return 0;
	return 1;
}

/* kamera-kayit zaman_ekle JPEG'leri arka arkaya ekler. ofs'tan sonraki kare [*bas, *bas + *uz): sinir FF D9 FF D8 FF, son
 * kare FF D9 ile biten dosya sonu. 1 bulundu, 0 son (ya da yazilmakta olan yarim kare), -1 okuma hatasi, 2 sinir yok
 * (kare cok buyuk/bozuk: cagiran *bas + *uz'dan devam eder) */
static int zaman_kare_bul(int fd, unsigned long long boyut, unsigned long long ofs, uint8_t *t, size_t tn,
			  unsigned long long *bas, uint32_t *uz)
{
	unsigned long long p = ofs, sinir = ofs + 1048576;
	for (;;) {	/* SOI: FF D8 FF */
		if (p + 3 > boyut || p > sinir) return 0;
		size_t n = boyut - p < 65536 ? (size_t)(boyut - p) : 65536;
		ssize_t r = pread(fd, t, n, (off_t)p);
		if (r < 0) return -1;
		if (r < 3) return 0;
		size_t i = 0;
		while (i + 2 < (size_t)r && !(t[i] == 0xFF && t[i + 1] == 0xD8 && t[i + 2] == 0xFF)) i++;
		if (i + 2 < (size_t)r) { p += i; break; }
		p += (unsigned long long)r - 2;
	}
	size_t oku = ZAMAN_ONCE < tn ? ZAMAN_ONCE : tn;
	for (;;) {
		size_t n = boyut - p < oku ? (size_t)(boyut - p) : oku;
		ssize_t r = pread(fd, t, n, (off_t)p);
		if (r < 0) return -1;
		if (r < 4) return 0;
		for (size_t i = 4; i + 2 < (size_t)r; i++)
			if (t[i] == 0xFF && t[i + 1] == 0xD8 && t[i + 2] == 0xFF && t[i - 2] == 0xFF && t[i - 1] == 0xD9) {
				*bas = p;
				*uz = (uint32_t)i;
				return 1;
			}
		if (p + (unsigned long long)r >= boyut) {	/* dosya sonu: son kare tam mi */
			if (t[r - 2] == 0xFF && t[r - 1] == 0xD9) { *bas = p; *uz = (uint32_t)r; return 1; }
			return 0;
		}
		if (oku >= tn) { *bas = p; *uz = (uint32_t)r; return 2; }
		oku = tn;
	}
}

/* ofs'tan sonraki (atla kadar kare gecilerek) kare; zaman_kare_bul donusleri (2 kendi icinde gecilir) */
static int zaman_kare_atla(int fd, unsigned long long boyut, unsigned long long ofs, int atla, uint8_t *t, size_t tn,
			   unsigned long long *bas, uint32_t *uz)
{
	int r = 0;
	for (int k = 0; k <= atla; k++) {
		for (int g = 0; g < 64; g++) {
			r = zaman_kare_bul(fd, boyut, ofs, t, tn, bas, uz);
			if (r != 2) break;
			ofs = *bas + *uz;
		}
		if (r != 1) return r == 2 ? 0 : r;
		if (k < atla) ofs = *bas + *uz;
	}
	return r;
}

static void zaman_bayraklari(int *acik, int *aralik)
{
	char y[600], b[16];
	snprintf(y, sizeof y, "%s/zaman-acik", kok);
	*acik = dosya_var(y);
	snprintf(y, sizeof y, "%s/zaman-aralik", kok);
	*aralik = 60;
	if (dosya_oku(y, b, sizeof b) > 0) {
		int v = atoi(b);
		if (v >= 1 && v <= 3600) *aralik = v;	/* v11: 1 sn (kamera-kayit v9) */
	}
}

/* v11: olay tespiti bayragi (kamera-kayit v9: zaman-olay) */
static int zaman_olay_acik(void)
{
	char y[600];
	snprintf(y, sizeof y, "%s/zaman-olay", kok);
	return dosya_var(y);
}

/* v11: kamera-kayit v9 durumundan time-lapse doluluk (yoksa mb -1, sinir -1, dolu 0) */
static void zaman_doluluk(double *mb, int *sinir, int *dolu)
{
	static char b[2048];
	static struct jtok tk[256];
	*mb = -1;
	*sinir = -1;
	*dolu = 0;
	int t = 0;
	if (dosya_oku(kayit_durum_yol, b, sizeof b) > 0) t = json_ayristir(b, (int)strlen(b), tk, 256);
	if (t < 1 || tk[0].tip != J_NESNE) return;
	int k;
	double v;
	if ((k = j_al(b, tk, t, 0, "zaman_mb")) >= 0 && !j_sayi(b, &tk[k], &v)) *mb = v;
	if ((k = j_al(b, tk, t, 0, "zaman_sinir_mb")) >= 0 && !j_sayi(b, &tk[k], &v)) *sinir = (int)v;
	if ((k = j_al(b, tk, t, 0, "zaman_dolu")) >= 0) *dolu = j_bool(b, &tk[k]) == 1;
}

/* v11: zaman/<gun>-olay.csv "HHMM,skor,ofs" satirlari; yazilmakta olan (yeni satirsiz) son satir ve bozuklar atlanir */
struct zo_satir { int dk, skor; long long ofs; };
static int zaman_olay_coz(const char *b, size_t n, struct zo_satir *o, int azami)
{
	int k = 0;
	size_t i = 0;
	while (i < n && k < azami) {
		const char *nl = memchr(b + i, '\n', n - i);
		if (!nl) break;
		size_t l = (size_t)(nl - (b + i));
		char st[64];
		if (l >= 6 && l < sizeof st) {
			memcpy(st, b + i, l);
			st[l] = 0;
			int hhmm, skor;
			long long ofs;
			char ek;
			if (st[4] == ',' && strspn(st, "0123456789") == 4 && sscanf(st, "%d,%d,%lld%c", &hhmm, &skor, &ofs, &ek) == 3 &&
			    hhmm / 100 <= 23 && hhmm % 100 <= 59 && skor >= 0 && skor <= 1000 && ofs >= -1) {
				o[k].dk = hhmm / 100 * 60 + hhmm % 100;
				o[k].skor = skor;
				o[k].ofs = ofs;
				k++;
			}
		}
		i += l + 1;
	}
	return k;
}

static int zo_skor_azalan(const void *a, const void *b)
{
	const struct zo_satir *x = a, *y = b;
	return y->skor != x->skor ? y->skor - x->skor : x->dk - y->dk;
}

struct zaman_oge { char gun[9]; long long bayt; int olay; };	/* v11: olay csv var mi */
static int zaman_azalan(const void *a, const void *b) { return strcmp(((const struct zaman_oge *)b)->gun, ((const struct zaman_oge *)a)->gun); }

static void api_zaman_liste(struct bag *c)
{
	if (izin(c, 0)) return;
	char dz[600], y[700];
	snprintf(dz, sizeof dz, "%s/zaman", kok);
	static struct zaman_oge l[64];
	int n = 0;
	DIR *d = opendir(dz);
	struct dirent *e;
	while (d && (e = readdir(d))) {
		char g[9];
		if (strlen(e->d_name) != 13 || strcmp(e->d_name + 8, ".mjpg")) continue;
		memcpy(g, e->d_name, 8);
		g[8] = 0;
		if (!zaman_gun_gecerli(g)) continue;
		struct stat st;
		snprintf(y, sizeof y, "%s/%s", dz, e->d_name);
		if (stat(y, &st) || !S_ISREG(st.st_mode) || st.st_size < 64) continue;
		int k = n;
		if (n < 64) n++;
		else {	/* dolu: en eskinin yerine (daha yeniyse) */
			k = 0;
			for (int i = 1; i < 64; i++) if (strcmp(l[i].gun, l[k].gun) < 0) k = i;
			if (strcmp(g, l[k].gun) <= 0) continue;
		}
		snprintf(l[k].gun, sizeof l[k].gun, "%s", g);
		l[k].bayt = (long long)st.st_size;
		snprintf(y, sizeof y, "%s/%s-olay.csv", dz, g);	/* v11 */
		l[k].olay = dosya_var(y);
	}
	if (d) closedir(d);
	if (n) qsort(l, (size_t)n, sizeof l[0], zaman_azalan);
	int acik, aralik;
	zaman_bayraklari(&acik, &aralik);
	struct jy y2 = { .azami = 8192 };
	jy_ham(&y2, "{\"liste\":[");
	for (int i = 0; i < n; i++)
		jy_bicim(&y2, "%s{\"gun\":\"%s\",\"bayt\":%lld,\"olay\":%s}", i ? "," : "", l[i].gun, l[i].bayt, l[i].olay ? "true" : "false");
	double zmb;
	int zsinir, zdolu;
	zaman_doluluk(&zmb, &zsinir, &zdolu);	/* v11 */
	jy_bicim(&y2, "],\"acik\":%s,\"aralik\":%d,\"olay\":%s,\"mb\":%.1f,\"sinir_mb\":%d,\"dolu\":%s,\"kalkan\":%s,\"izin_kalan_sn\":%d}",
		 acik ? "true" : "false", aralik, zaman_olay_acik() ? "true" : "false", zmb, zsinir, zdolu ? "true" : "false",
		 kalkan() ? "true" : "false", c->oturum >= 0 ? izin_kalan(c->oturum, O[c->oturum].kayit_bitis) : 0);
	yanit_json(c, 200, &y2, NULL);
}

static void api_zaman_kare(struct bag *c)
{
	if (izin(c, 0)) return;
	char g[16], s[24];
	if (sorgu_al(c->r.sorgu, "gun", g, sizeof g) != 1 || !zaman_gun_gecerli(g)) { hata_yanit(c, 400, "Geçersiz gün"); return; }
	unsigned long long ofs = 0;
	int atla = 0;
	if (sorgu_al(c->r.sorgu, "ofs", s, sizeof s) == 1) {
		for (const char *p = s; *p; p++) if (*p < '0' || *p > '9') { hata_yanit(c, 400, NULL); return; }
		ofs = strtoull(s, NULL, 10);
	}
	if (sorgu_al(c->r.sorgu, "atla", s, sizeof s) == 1) {
		atla = atoi(s);
		if (atla < 0) atla = 0;
		if (atla > ZAMAN_ATLA_AZAMI) atla = ZAMAN_ATLA_AZAMI;
	}
	kayit_izin_oto(c);
	if (!kayit_izin_aktif(c->oturum)) { hata_yanit(c, 423, "Time-lapse için kayıt kilidini parolayla açın"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık"); return; }
	char tam[600];
	snprintf(tam, sizeof tam, "%s/zaman/%s.mjpg", kok, g);
	int fd = open(tam, O_RDONLY | O_CLOEXEC);
	struct stat st;
	if (fd < 0 || fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size <= 0) {
		if (fd >= 0) close(fd);
		hata_yanit(c, 404, "Bu gün için time-lapse yok");
		return;
	}
	unsigned long long boyut = (unsigned long long)st.st_size, bas = 0;
	uint32_t uz = 0;
	int r = ofs < boyut ? zaman_kare_atla(fd, boyut, ofs, atla, tarama_t, sizeof tarama_t, &bas, &uz) : 0;
	close(fd);
	if (r < 0) { hata_yanit(c, 502, "Time-lapse okunamadı"); return; }
	if (r != 1) { yanit(c, 204, NULL, NULL, 0, NULL); return; }
	char an[12];
	snprintf(an, sizeof an, "Z%s", g);
	if (oturumda_ilk(c, an)) denetim(c->ip, "TIME-LAPSE IZLENDI %s", g);
	char ek[200];
	snprintf(ek, sizeof ek, "X-Sonraki: %llu\r\nX-Ilerleme: %.4f\r\nX-Boyut: %llu\r\n", bas + uz,
		 boyut ? (double)(bas + uz) / (double)boyut : 1.0, boyut);
	yanit(c, 200, "image/jpeg", tarama_t, uz, ek);
	memset(tarama_t, 0, uz);
}

static void zaman_mod_uygula(struct bag *c, int acik, int aralik, int olay)	/* v11: olay -1 dokunma, 0/1 */
{
	char yol[600];
	mkdir(kok, 0755);
	if (aralik > 0) {
		snprintf(yol, sizeof yol, "%s/zaman-aralik", kok);
		int fd = open(yol, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		char b[16];
		int n = snprintf(b, sizeof b, "%d\n", aralik);
		if (fd < 0 || write(fd, b, (size_t)n) != n) {
			if (fd >= 0) close(fd);
			hata_yanit(c, 502, "Aralık SD'ye yazılamadı");
			return;
		}
		close(fd);
	}
	if (acik >= 0) {
		snprintf(yol, sizeof yol, "%s/zaman-acik", kok);
		bayrak_yaz(yol, acik);
	}
	if (olay >= 0) {	/* v11 */
		snprintf(yol, sizeof yol, "%s/zaman-olay", kok);
		bayrak_yaz(yol, olay);
	}
	sync();
	int a2, ar2, o2 = zaman_olay_acik();
	zaman_bayraklari(&a2, &ar2);
	if ((acik >= 0 && a2 != acik) || (aralik > 0 && ar2 != aralik) || (olay >= 0 && o2 != olay)) {
		denetim(c->ip, "time-lapse ayari BASARISIZ (SD)");
		hata_yanit(c, 502, "Cihaz time-lapse ayarını onaylamadı");
		return;
	}
	if (acik == 1) denetim(c->ip, "TIME-LAPSE ACILDI (%d sn)%s", ar2, dog_ek);
	else if (acik == 0) denetim(c->ip, "time-lapse kapatildi");
	else if (aralik > 0) denetim(c->ip, "time-lapse araligi %d sn", ar2);
	if (olay == 1) denetim(c->ip, "TIME-LAPSE OLAY TESPITI ACILDI%s", dog_ek);	/* v11 */
	else if (olay == 0) denetim(c->ip, "time-lapse olay tespiti kapatildi");
	struct jy y = { .azami = 128 };
	jy_bicim(&y, "{\"ok\":true,\"acik\":%s,\"aralik\":%d,\"olay\":%s}", a2 ? "true" : "false", ar2, o2 ? "true" : "false");
	yanit_json(c, 200, &y, NULL);
}

/* 0 yok, -1 gecersiz, >0 saniye */
static int zaman_aralik_al(void)
{
	long long v;
	int r = g_int("aralik", &v);
	if (r == -2) return 0;
	if (r || v < 1 || v > 3600) return -1;	/* v11: 1 sn */
	return (int)v;
}

static void zaman_mod_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	int a = zaman_aralik_al();
	zaman_mod_uygula(c, g_bool("acik"), a > 0 ? a : 0, g_bool("olay"));	/* v11: acik ve/veya olay */
}

static void api_zaman_mod(struct bag *c)
{
	if (izin(c, 0)) return;
	int acik = g_bool("acik"), olay = g_bool("olay");	/* v11: olay */
	int aralik = zaman_aralik_al();
	if (aralik < 0 || (acik < 0 && olay < 0 && aralik == 0)) { hata_yanit(c, 400, "acik, aralik (1..3600 sn) ve/veya olay"); return; }
	if (acik == 1 || olay == 1) {
		if (kalkan()) {
			denetim(c->ip, "TIME-LAPSE ACMA ENGELLENDI (panelde gizlilik kalkani acik)");
			hata_yanit(c, 409, "Panelde gizlilik kalkanı açık; time-lapse uzaktan açılamaz");
			return;
		}
		hassas(c, acik == 1 ? "TIME-LAPSE ACMA" : "TIME-LAPSE OLAY TESPITI ACMA", zaman_mod_devam);
		return;
	}
	zaman_mod_uygula(c, acik, aralik, olay);
}

/* v11: time-lapse olaylari (yalniz okuma, goruntu yok -> yalniz oturum izni; /api/olaylar gibi) */
static void api_zaman_olay(struct bag *c)
{
	if (izin(c, 0)) return;
	char g[16], e[16], y[600], b2[16];
	if (sorgu_al(c->r.sorgu, "gun", g, sizeof g) != 1 || !zaman_gun_gecerli(g)) { hata_yanit(c, 400, "Geçersiz gün"); return; }
	int esik = 120;
	snprintf(y, sizeof y, "%s/bekci-esik", kok);
	if (dosya_oku(y, b2, sizeof b2) > 0) {
		int v = atoi(b2);
		if (v >= 20 && v <= 1000) esik = v;
	}
	int r = sorgu_al(c->r.sorgu, "esik", e, sizeof e);
	if (r < 0 || (r == 1 && (!e[0] || strspn(e, "0123456789") != strlen(e) || strlen(e) > 4 || atoi(e) > 1000))) {
		hata_yanit(c, 400, "Eşik 0..1000 olmalı");
		return;
	}
	if (r == 1) esik = atoi(e);
	static char buf[65536];
	static struct zo_satir s[1500], o[1500];
	snprintf(y, sizeof y, "%s/zaman/%s-olay.csv", kok, g);
	int fd = open(y, O_RDONLY | O_CLOEXEC);
	ssize_t n = fd >= 0 ? read(fd, buf, sizeof buf) : 0;
	if (fd >= 0) close(fd);
	int k = n > 0 ? zaman_olay_coz(buf, (size_t)n, s, 1500) : 0, on = 0;
	for (int i = 0; i < k; i++) if (s[i].skor >= esik) o[on++] = s[i];
	if (on) qsort(o, (size_t)on, sizeof o[0], zo_skor_azalan);
	struct jy j = { .azami = 65536 };
	jy_bicim(&j, "{\"gun\":\"%s\",\"esik\":%d,\"acik\":%s,\"dakikalar\":[", g, esik, zaman_olay_acik() ? "true" : "false");
	for (int i = 0; i < k; i++) jy_bicim(&j, "%s[%d,%d,%lld]", i ? "," : "", s[i].dk, s[i].skor, s[i].ofs);
	jy_ham(&j, "],\"olaylar\":[");
	for (int i = 0; i < on && i < 100; i++)
		jy_bicim(&j, "%s{\"dk\":%d,\"skor\":%d,\"ofs\":%lld}", i ? "," : "", o[i].dk, o[i].skor, o[i].ofs);
	jy_ham(&j, "]}");
	yanit_json(c, 200, &j, NULL);
}

/* fotograflar: k=kamera (<kamera>/fotolar, kamera-kayit) | cerceve (/mnt/sd/fotograf, panel cercevesi) */
static int foto_ad_gecerli(const char *a)
{
	size_t n = strlen(a);
	if (n < 5 || n > 80 || a[0] == '.' || strstr(a, "..")) return 0;
	for (size_t i = 0; i < n; i++) {
		char ch = a[i];
		if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' || ch == '.'))
			return 0;
	}
	const char *u = strrchr(a, '.');
	char k[6] = "";
	for (int i = 0; u && u[i] && i < 5; i++) k[i] = (char)(u[i] >= 'A' && u[i] <= 'Z' ? u[i] - 'A' + 'a' : u[i]);
	return !strcmp(k, ".jpg") || !strcmp(k, ".jpeg");
}

static const char *foto_kaynak(const char *k, char *o, size_t n)
{
	if (!strcmp(k, "kamera")) { snprintf(o, n, "%s/fotolar", kok); return o; }
	if (!strcmp(k, "cerceve")) { snprintf(o, n, "%s", foto_dizin); return o; }
	return NULL;
}

struct foto_oge { char k[8]; char ad[84]; long long bayt; long long t; };
static int foto_azalan(const void *a, const void *b)
{
	const struct foto_oge *x = a, *y = b;
	return x->t != y->t ? (x->t < y->t ? 1 : -1) : strcmp(y->ad, x->ad);
}

/* v12: liste doluysa en eskinin yerine daha yenisi (vfat eskiden yeniye okur: en yeni fotolar dusmesin) */
static void foto_ekle(struct foto_oge *l, int *n, int azami, const struct foto_oge *yeni)
{
	int k = *n;
	if (*n < azami) (*n)++;
	else {
		k = 0;
		for (int i = 1; i < azami; i++) if (foto_azalan(&l[i], &l[k]) > 0) k = i;
		if (foto_azalan(yeni, &l[k]) >= 0) return;
	}
	l[k] = *yeni;
}

static void api_foto_liste(struct bag *c)
{
	if (izin(c, 0)) return;
	static struct foto_oge l[FOTO_AZAMI];
	int n = 0;
	static const char *const kaynaklar[2] = {"kamera", "cerceve"};
	for (int s = 0; s < 2; s++) {
		char dz[600], y[700];
		if (!foto_kaynak(kaynaklar[s], dz, sizeof dz)) continue;
		DIR *d = opendir(dz);
		struct dirent *e;
		int taranan = 0;
		/* shortcut: v16 (B7) klasor basina ilk FOTO_TARAMA_AZAMI gecerli ad stat edilir (FAT'ta her stat pahali); readdir
		 * sirasi rastgele oldugundan daha fazlasinda en yeniler kacabilir; kamera klasoru zaten 300'de tutulur. Cerceve
		 * klasoru binleri gecerse ada gore (tarih adli) secime gec. */
		while (d && (e = readdir(d))) {
			if (!foto_ad_gecerli(e->d_name)) continue;
			if (++taranan > FOTO_TARAMA_AZAMI) break;
			struct stat st;
			snprintf(y, sizeof y, "%s/%s", dz, e->d_name);
			if (stat(y, &st) || !S_ISREG(st.st_mode) || st.st_size < 128) continue;
			struct foto_oge f;
			snprintf(f.k, sizeof f.k, "%s", kaynaklar[s]);
			snprintf(f.ad, sizeof f.ad, "%s", e->d_name);
			f.bayt = (long long)st.st_size;
			f.t = (long long)st.st_mtime;
			foto_ekle(l, &n, FOTO_AZAMI, &f);	/* v12 */
		}
		if (d) closedir(d);
	}
	if (n) qsort(l, (size_t)n, sizeof l[0], foto_azalan);
	struct jy y = { .azami = CIKIS_AZAMI / 2 };
	jy_ham(&y, "{\"liste\":[");
	for (int i = 0; i < n; i++)
		jy_bicim(&y, "%s{\"k\":\"%s\",\"ad\":\"%s\",\"bayt\":%lld,\"t\":%lld}", i ? "," : "", l[i].k, l[i].ad, l[i].bayt, l[i].t);
	jy_bicim(&y, "],\"kalkan\":%s,\"izin_kalan_sn\":%d}", kalkan() ? "true" : "false",
		 c->oturum >= 0 ? izin_kalan(c->oturum, O[c->oturum].kayit_bitis) : 0);
	yanit_json(c, 200, &y, NULL);
}

static void api_foto_al(struct bag *c)
{
	if (izin(c, 0)) return;
	char k[16], ad[96], dz[600];
	if (sorgu_al(c->r.sorgu, "k", k, sizeof k) != 1 || !foto_kaynak(k, dz, sizeof dz) ||
	    sorgu_al(c->r.sorgu, "ad", ad, sizeof ad) != 1 || !foto_ad_gecerli(ad)) {
		hata_yanit(c, 400, "Geçersiz fotoğraf");
		return;
	}
	kayit_izin_oto(c);
	if (!kayit_izin_aktif(c->oturum)) { hata_yanit(c, 423, "Fotoğraflar için kayıt kilidini parolayla açın"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık"); return; }
	char tam[700];
	snprintf(tam, sizeof tam, "%s/%s", dz, ad);
	int fd = open(tam, O_RDONLY | O_CLOEXEC);
	struct stat st;
	if (fd < 0 || fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size <= 0 || st.st_size > KAYIT_AZAMI) {
		if (fd >= 0) close(fd);
		hata_yanit(c, 404, "Fotoğraf yok");
		return;
	}
	if (oturumda_ilk(c, "FOTO")) denetim(c->ip, "FOTOGRAFLAR ACILDI (%s/%s ...)", k, ad);
	if ((unsigned long long)st.st_size <= sizeof tarama_t) {
		ssize_t r = pread(fd, tarama_t, (size_t)st.st_size, 0);
		close(fd);
		if (r != st.st_size) { hata_yanit(c, 502, "Fotoğraf okunamadı"); return; }
		yanit(c, 200, "image/jpeg", tarama_t, (size_t)r, NULL);	/* v12: /api/ no-store (etkisiz ikinci baslik kaldirildi) */
		memset(tarama_t, 0, (size_t)r);
		return;
	}
	if (M.yan_bag >= 0) { close(fd); hata_yanit(c, 503, NULL); return; }	/* buyuk: indirme kanali (tek) */
	c->keepalive = 0;
	yanit_bas(c, 200, "image/jpeg", (long long)st.st_size, NULL);
	c->dosya_fd = fd;
	c->dosya_kalan = (unsigned long long)st.st_size;
	c->hiz = AKTARIM_HIZ;
	c->hiz_t0 = simdi();
	c->giden = 0;
	c->tur = B_INDIR;
	M.yan_bag = (int)(c - B);
}

/* ------------------------------------------------------------------ olaylar / ozet */
struct olay_veri { struct kayit_oge *l; int ln; struct puan *p; int pn; char gunler[400][9]; int gn; };

static int puan_karsilastir(const void *a, const void *b)
{
	const struct puan *x = a, *y = b;
	if (x->skor != y->skor) return y->skor - x->skor;
	return strcmp(x->ad, y->ad);
}

static int olay_verisi(const char *gun, struct olay_veri *v)
{
	memset(v, 0, sizeof *v);
	v->ln = kayit_listesi(&v->l);
	if (v->ln < 0) return -1;
	size_t n = 0;
	char *m = bellek_cagir(olay_sar, gun[0] ? gun : "*", "0", &n);
	if (!m) { free(v->l); return -1; }
	int cap = 0, son = 0;
	char *s = m, *sat;
	while ((sat = strsep(&s, "\n"))) {
		if (!strcmp(sat, "SON")) { son = 1; break; }
		char ad[32];
		int sk, kr, tp, tms;
		long long by;
		if (sscanf(sat, "%31s %d %d %lld %d %d", ad, &sk, &kr, &by, &tp, &tms) != 6 || !kayit_ad_gecerli(ad) || sk < 0 || sk > 1000) continue;
		if (v->pn == cap) {
			cap = cap ? cap * 2 : 128;
			if (cap > 1440 * 64) break;
			struct puan *np = realloc(v->p, (size_t)cap * sizeof *np);
			if (!np) break;
			v->p = np;
		}
		struct puan *p = &v->p[v->pn++];
		snprintf(p->ad, sizeof p->ad, "%s", ad);
		p->skor = sk; p->kare = kr; p->bayt = by; p->tepe = tp;
		p->tepe_sn = tms >= 0 ? (tms / 1000 < 59 ? tms / 1000 : 59) : (tp / KAYIT_FPS < 59 ? tp / KAYIT_FPS : 59);
	}
	free(m);
	if (!son) { free(v->l); free(v->p); return -1; }
	for (int i = 0; i < v->ln; i++) {	/* liste azalan sirada: gunler de azalan */
		if (v->gn && !strncmp(v->gunler[v->gn - 1], v->l[i].ad, 8)) continue;
		int var = 0;
		for (int k = 0; k < v->gn; k++) if (!strncmp(v->gunler[k], v->l[i].ad, 8)) { var = 1; break; }
		if (!var && v->gn < 400) { memcpy(v->gunler[v->gn], v->l[i].ad, 8); v->gunler[v->gn++][8] = 0; }
	}
	return 0;
}

static void olay_bitir(struct olay_veri *v) { free(v->l); free(v->p); }

static void gun_ozeti(struct jy *y, const struct olay_veri *v, const char *gun, int esik)
{
	int saat[24] = { 0 }, olay_saat[24] = { 0 }, dakika = 0, olay_n = 0;
	long long top = 0;
	const char *ilk = NULL, *son = NULL;
	for (int i = 0; i < v->ln; i++) {
		if (strncmp(v->l[i].ad, gun, 8)) continue;
		saat[atoi((char[3]){ v->l[i].ad[9], v->l[i].ad[10], 0 })]++;
		dakika++;
		top += v->l[i].bayt;
		if (!ilk || strcmp(v->l[i].ad, ilk) < 0) ilk = v->l[i].ad;
		if (!son || strcmp(v->l[i].ad, son) > 0) son = v->l[i].ad;
	}
	const struct puan *en = NULL;
	for (int i = 0; i < v->pn; i++) {
		if (strncmp(v->p[i].ad, gun, 8)) continue;
		if (!en || v->p[i].skor > en->skor) en = &v->p[i];
		if (v->p[i].skor >= esik) { olay_saat[atoi((char[3]){ v->p[i].ad[9], v->p[i].ad[10], 0 })]++; olay_n++; }
	}
	jy_bicim(y, "{\"gun\":\"%s\",\"dakika\":%d,\"toplam_mb\":%.1f,\"ilk\":", gun, dakika, top / 1048576.0);
	if (ilk) jy_bicim(y, "\"%.4s\"", ilk + 9); else jy_ham(y, "null");
	jy_ham(y, ",\"son\":");
	if (son) jy_bicim(y, "\"%.4s\"", son + 9); else jy_ham(y, "null");
	jy_ham(y, ",\"saatler\":[");
	for (int h = 0; h < 24; h++) jy_bicim(y, "%s%d", h ? "," : "", saat[h]);
	jy_ham(y, "],\"olay_saatleri\":[");
	for (int h = 0; h < 24; h++) jy_bicim(y, "%s%d", h ? "," : "", olay_saat[h]);
	int zirve = 0;
	for (int h = 1; h < 24; h++) if (olay_saat[h] > olay_saat[zirve]) zirve = h;
	jy_bicim(y, "],\"olay_sayisi\":%d,\"en_hareketli_saat\":", olay_n);
	if (olay_n) jy_bicim(y, "%d", zirve); else jy_ham(y, "null");
	jy_ham(y, ",\"en_yuksek\":");
	if (en) jy_bicim(y, "{\"ad\":\"%s\",\"skor\":%d}", en->ad, en->skor); else jy_ham(y, "null");
	jy_ham(y, "}");
}

/* 0 ok, -1 yanit gonderildi */
static int gun_esik_param(struct bag *c, char *gun, int *esik)
{
	char g[16] = "", e[16] = "";
	int r = sorgu_al(c->r.sorgu, "gun", g, sizeof g);
	if (r < 0) { hata_yanit(c, 400, "Gün YYYYAAGG olmalı"); return -1; }
	if (r == 1 && g[0] && strcmp(g, "*")) {
		if (!gun_gecerli(g)) { hata_yanit(c, 400, "Gün YYYYAAGG olmalı"); return -1; }
		strcpy(gun, g);
	} else gun[0] = 0;
	*esik = OLAY_ESIK;
	r = sorgu_al(c->r.sorgu, "esik", e, sizeof e);
	if (r < 0 || (r == 1 && (!e[0] || strspn(e, "0123456789") != strlen(e) || strlen(e) > 4))) { hata_yanit(c, 400, NULL); return -1; }
	if (r == 1) *esik = atoi(e);
	if (*esik < 0 || *esik > 1000) { hata_yanit(c, 400, "Eşik 0..1000 olmalı"); return -1; }
	return 0;
}

static void api_olaylar(struct bag *c, int sadece_ozet)
{
	if (izin(c, 0)) return;
	char gun[16];
	int esik;
	if (gun_esik_param(c, gun, &esik)) return;
	struct olay_veri v;
	if (olay_verisi(gun, &v)) { hata_yanit(c, 502, sadece_ozet ? "Özet okunamadı" : "Olaylar okunamadı"); return; }
	char secili[9] = "";
	if (gun[0]) snprintf(secili, sizeof secili, "%s", gun);
	else if (v.gn) snprintf(secili, sizeof secili, "%s", v.gunler[0]);
	struct jy y = { .azami = CIKIS_AZAMI / 2 };
	jy_ham(&y, "{\"kaynak\":\"olcum\"");
	if (!sadece_ozet) jy_bicim(&y, ",\"esik\":%d,\"gun\":\"%s\"", esik, secili);
	jy_ham(&y, ",\"gunler\":[");
	for (int i = 0; i < v.gn; i++) jy_bicim(&y, "%s\"%s\"", i ? "," : "", v.gunler[i]);
	jy_ham(&y, "],\"ozet\":");
	if (secili[0]) gun_ozeti(&y, &v, secili, esik); else jy_ham(&y, "null");
	if (!sadece_ozet) {
		jy_ham(&y, ",\"puanlar\":[");
		int ilk = 1;
		for (int i = 0; i < v.pn; i++) {
			if (secili[0] && strncmp(v.p[i].ad, secili, 8)) continue;
			jy_bicim(&y, "%s{\"ad\":\"%s\",\"skor\":%d,\"kare\":%d,\"bayt\":%lld,\"tepe\":%d,\"tepe_sn\":%d}", ilk ? "" : ",",
				 v.p[i].ad, v.p[i].skor, v.p[i].kare, v.p[i].bayt, v.p[i].tepe, v.p[i].tepe_sn);
			ilk = 0;
		}
		jy_ham(&y, "],\"kayitlar\":[");
		ilk = 1;
		for (int i = v.ln - 1; i >= 0; i--) {	/* artan sira */
			if (strncmp(v.l[i].ad, secili, 8)) continue;
			jy_bicim(&y, "%s\"%s\"", ilk ? "" : ",", v.l[i].ad);
			ilk = 0;
		}
		jy_ham(&y, "],\"olaylar\":[");
		struct puan *s = v.pn ? malloc((size_t)v.pn * sizeof *s) : NULL;
		int sn = 0;
		for (int i = 0; s && i < v.pn; i++) if (v.p[i].skor >= esik) s[sn++] = v.p[i];
		if (sn) qsort(s, (size_t)sn, sizeof *s, puan_karsilastir);
		for (int i = 0; i < sn && i < 200; i++)
			jy_bicim(&y, "%s{\"ad\":\"%s\",\"skor\":%d,\"kare\":%d,\"bayt\":%lld,\"tepe\":%d,\"tepe_sn\":%d}", i ? "," : "",
				 s[i].ad, s[i].skor, s[i].kare, s[i].bayt, s[i].tepe, s[i].tepe_sn);
		free(s);
		jy_bicim(&y, "],\"izin_kalan_sn\":%d", izin_kalan(c->oturum, O[c->oturum].kayit_bitis));
	}
	jy_ham(&y, "}");
	olay_bitir(&v);
	yanit_json(c, 200, &y, NULL);
}

/* ------------------------------------------------------------------ kayit plani */
static int kural_aktif(const struct kural *k, int g, int dk)
{
	if (k->bas < k->bit) return (k->gun >> g & 1) && k->bas <= dk && dk < k->bit;
	return ((k->gun >> g & 1) && dk >= k->bas) || ((k->gun >> ((g + 6) % 7) & 1) && dk < k->bit);	/* gece yarisini asan */
}

static int plan_istenen_kural(const struct kural *p, int n, const struct tm *t)
{
	int g = (t->tm_wday + 6) % 7, dk = t->tm_hour * 60 + t->tm_min;
	for (int i = 0; i < n; i++) if (kural_aktif(&p[i], g, dk)) return i;
	return -1;
}

static void plan_adim(void)
{
	if (!M.plan_acik) { M.plan_onceki = -1; M.plan_bildirildi = 0; return; }
	time_t t = (time_t)simdi_unix();
	if (t < SAAT_GECERLI) return;	/* v13 (B6): saat ayarlanmadan karar yok; ilk gecerli adim acilis sayilir */
	struct tm tm;
	localtime_r(&t, &tm);
	int k = plan_istenen_kural(M.plan, M.plan_n, &tm);
	int ist = k >= 0;
	char yol[600], elle[600];
	snprintf(yol, sizeof yol, "%s/kayit-acik", kok);
	snprintf(elle, sizeof elle, "%s/kayit-elle-kapali", kok);
	if (M.plan_onceki < 0) {	/* v13 (B1, B5): acilista yalniz planin kendi bayragi; elle acma/kapatma korunur */
		char b[16];
		long bn = dosya_oku(yol, b, sizeof b);
		if (ist) M.plan_onceki = bn >= 0 || dosya_var(elle);	/* bayrak yok ve elle kapatilmadiysa acilir */
		else M.plan_onceki = bn >= 0 && !strcmp(b, "plan");	/* yalniz planin yazdigi bayrak kalkar (denetim v1 #12) */
	}
	/* v15 (OTONOM 152, B5): penceredeyken bayrak baskasinca (kopru KAYIT 0, panel, sesli komut) silindiyse elle kapatma
	 * isareti; elle yeniden acilinca silinir. kamera-kayit kayit-acik'i silmez. */
	if (ist && M.plan_onceki == 1) {
		int bv = dosya_var(yol), ev = dosya_var(elle);
		if (!bv && !ev) bayrak_yaz(elle, 1);
		else if (bv && ev) unlink(elle);
	}
	if (ist == M.plan_onceki) return;
	int ok = 0;
	if (!(ist && kalkan())) {
		mkdir(kok, 0755);
		bayrak_yaz_icerik(yol, ist, "plan\n");	/* v13 (B1): sahibi belli (kamera-kayit yalniz varligina bakar) */
		sync();
		ok = dosya_var(yol) == ist;
	}
	if (ok) {
		unlink(elle);	/* v13 (B5): planin gercek gecisi elle kapatma isaretini siler */
		M.plan_onceki = ist;
		strftime(M.plan_son, sizeof M.plan_son, "%Y-%m-%dT%H:%M:%S", &tm);
		M.plan_hata[0] = 0;
		M.plan_bildirildi = 0;
		char km[96];
		if (ist) { kural_metni(&M.plan[k], km, sizeof km); denetim("-", "PLAN: SD KAYIT ACILDI (kural %s)", km); }
		else denetim("-", "PLAN: SD kayit kapatildi (plan penceresi bitti)");
		return;
	}
	int kl = ist && kalkan();
	snprintf(M.plan_hata, sizeof M.plan_hata, "%s", kl ? "Panelde gizlilik kalkanı açık" : "SD karta yazılamadı");
	if (!M.plan_bildirildi) {
		denetim("-", "PLAN: SD kayit %s BASARISIZ (%s); yeniden denenecek", ist ? "acma" : "kapatma", kl ? "gizlilik kalkani acik" : "SD");
		M.plan_bildirildi = 1;
	}
}

/* v13 (inceleme B5): plan penceresinde kayit elle kapatilinca <kok>/kayit-elle-kapali; varken plan acilista kaydi yeniden
 * acmaz, planin bir sonraki gercek gecisi siler. Elle acma da siler. */
static void elle_kapali_isaret(int kapatildi)
{
	char y[600];
	snprintf(y, sizeof y, "%s/kayit-elle-kapali", kok);
	if (!kapatildi) { unlink(y); return; }
	time_t t = (time_t)simdi_unix();
	struct tm tm;
	localtime_r(&t, &tm);
	if (M.plan_acik && t >= SAAT_GECERLI && plan_istenen_kural(M.plan, M.plan_n, &tm) >= 0) bayrak_yaz(y, 1);
}

static void plan_ozet(struct jy *y)
{
	time_t t = (time_t)simdi_unix();
	struct tm tm;
	localtime_r(&t, &tm);
	jy_bicim(y, "{\"acik\":%s,\"kurallar\":", M.plan_acik ? "true" : "false");
	plan_json(y);
	jy_bicim(y, ",\"simdi_pencerede\":%s,\"sonraki\":", plan_istenen_kural(M.plan, M.plan_n, &tm) >= 0 ? "true" : "false");
	int bulundu = 0;
	if (M.plan_acik && M.plan_n) {
		time_t b = t - tm.tm_sec;
		struct tm x;
		localtime_r(&b, &x);
		int simdi_ = plan_istenen_kural(M.plan, M.plan_n, &x) >= 0;
		for (int i = 1; i <= 8 * 1440; i++) {
			time_t z = b + (time_t)i * 60;
			localtime_r(&z, &x);
			if ((plan_istenen_kural(M.plan, M.plan_n, &x) >= 0) != simdi_) {
				char s[24];
				strftime(s, sizeof s, "%Y-%m-%dT%H:%M", &x);
				jy_bicim(y, "{\"zaman\":\"%s\",\"eylem\":\"%s\"}", s, simdi_ ? "durdur" : "baslat");
				bulundu = 1;
				break;
			}
		}
	}
	if (!bulundu) jy_ham(y, "null");
	jy_ham(y, ",\"son_uygulama\":");
	if (M.plan_son[0]) jy_dizge(y, M.plan_son); else jy_ham(y, "null");
	jy_ham(y, ",\"son_hata\":");
	jy_dizge(y, M.plan_hata);
	jy_bicim(y, ",\"azami_kural\":%d}", PLAN_AZAMI_KURAL);
}

static void api_plan_get(struct bag *c)
{
	if (izin(c, 0)) return;
	struct jy y = { .azami = 8192 };
	plan_ozet(&y);
	yanit_json(c, 200, &y, NULL);
}

static struct kural plan_bekleyen[PLAN_AZAMI_KURAL];
static int plan_bekleyen_n, plan_bekleyen_acik;

static void plan_uygula(struct bag *c)
{
	int eski_acik = M.plan_acik;
	int degisti = plan_bekleyen_n != M.plan_n || memcmp(plan_bekleyen, M.plan, sizeof(struct kural) * (size_t)M.plan_n);
	struct kural yedek[PLAN_AZAMI_KURAL];
	int yn = M.plan_n;
	memcpy(yedek, M.plan, sizeof yedek);
	memcpy(M.plan, plan_bekleyen, sizeof plan_bekleyen);
	M.plan_n = plan_bekleyen_n;
	M.plan_acik = plan_bekleyen_acik;
	if (ayar_yaz()) {
		memcpy(M.plan, yedek, sizeof yedek);
		M.plan_n = yn;
		M.plan_acik = eski_acik;
		hata_yanit(c, 502, "Ayar yazılamadı");
		return;
	}
	if (!M.plan_acik) { M.plan_onceki = -1; M.plan_hata[0] = 0; M.plan_bildirildi = 0; }
	char metin[400] = "";
	size_t w = 0;
	for (int i = 0; i < M.plan_n; i++) {
		char km[96];
		kural_metni(&M.plan[i], km, sizeof km);
		w += (size_t)snprintf(metin + w, sizeof metin > w ? sizeof metin - w : 0, "%s%s", i ? "; " : "", km);
	}
	if (!M.plan_n) snprintf(metin, sizeof metin, "kural yok");
	if (M.plan_acik && !eski_acik) { denetim(c->ip, "KAYIT PLANI ACILDI (%s)%s", metin, dog_ek); plan_adim(); }
	else if (!M.plan_acik && eski_acik) denetim(c->ip, "kayit plani kapatildi");
	if (degisti) denetim(c->ip, "KAYIT PLANI DEGISTI (%s)%s", metin, M.plan_acik ? dog_ek : "");
	struct jy y = { .azami = 8192 };
	plan_ozet(&y);
	yanit_json(c, 200, &y, NULL);
}

/* govdeden bekleyen plani kur: -1 yanit gonderildi, 0 parolasiz uygulanabilir, 1 parola gerekir */
static int plan_hazirla(struct bag *c)
{
	int acik = g_al("acik") >= 0 ? g_bool("acik") : M.plan_acik;
	if (acik < 0) { hata_yanit(c, 400, NULL); return -1; }
	memcpy(plan_bekleyen, M.plan, sizeof plan_bekleyen);
	plan_bekleyen_n = M.plan_n;
	int k = g_al("kurallar");
	if (k >= 0) {
		const char *e;
		if (plan_ayristir(G.s, G.tk, G.t, k, plan_bekleyen, &plan_bekleyen_n, &e)) { hata_yanit(c, 400, e); return -1; }
	}
	int degisti = plan_bekleyen_n != M.plan_n || memcmp(plan_bekleyen, M.plan, sizeof(struct kural) * (size_t)M.plan_n);
	if (acik && !plan_bekleyen_n) { hata_yanit(c, 400, "Plan açmak için en az bir kural gerekli"); return -1; }
	plan_bekleyen_acik = acik;
	return (acik && !M.plan_acik) || (degisti && acik);
}

static void plan_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	if (plan_hazirla(c) >= 0) plan_uygula(c);
}

static void api_plan_post(struct bag *c)
{
	if (izin(c, 0)) return;
	int r = plan_hazirla(c);
	if (r < 0) return;
	if (r) { hassas(c, "KAYIT PLANI", plan_devam); return; }
	plan_uygula(c);
}

/* ------------------------------------------------------------------ uzaktan erisim (Tailscale durumu; yalniz okuma) */
static void api_uzak(struct bag *c)
{
	if (izin(c, 0)) return;
	static char b[65536];
	static struct jtok tk[4096];
	struct jy y = { .azami = 65536 };
	int kurulu = dosya_var(ts_ikili_yol);
	jy_bicim(&y, "{\"kurulu\":%s,\"bayrak\":%s", kurulu ? "true" : "false", dosya_var(ts_bayrak_yol) ? "true" : "false");
	long long yas = yas_ms_m(ts_durum_yol);
	int t = 0;
	if (yas >= 0 && yas <= 120000 && dosya_oku(ts_durum_yol, b, sizeof b) > 0) t = json_ayristir(b, (int)strlen(b), tk, 4096);
	char dns[256] = "";
	if (t > 0 && tk[0].tip == J_NESNE) {
		char s1[256];
		int ben = j_al(b, tk, t, 0, "Self"), tn = j_al(b, tk, t, 0, "CurrentTailnet");
		jy_ham(&y, ",\"calisiyor\":true,\"durum\":");
		int k = j_al(b, tk, t, 0, "BackendState");
		jy_dizge(&y, k >= 0 && !j_dizge_al(b, &tk[k], s1, 41) ? s1 : "");
		int giris_gerek = k >= 0 && !strcmp(s1, "NeedsLogin");
		jy_ham(&y, ",\"surum\":");
		k = j_al(b, tk, t, 0, "Version");
		jy_dizge(&y, k >= 0 && !j_dizge_al(b, &tk[k], s1, 41) ? s1 : "");
		jy_ham(&y, ",\"cihaz\":");
		k = j_al(b, tk, t, ben, "HostName");
		jy_dizge(&y, k >= 0 && !j_dizge_al(b, &tk[k], s1, 81) ? s1 : "");
		k = j_al(b, tk, t, ben, "DNSName");
		if (k >= 0 && !j_dizge_al(b, &tk[k], dns, 200)) { size_t L = strlen(dns); if (L && dns[L - 1] == '.') dns[L - 1] = 0; }
		else dns[0] = 0;
		jy_ham(&y, ",\"dns\":");
		jy_dizge(&y, dns);
		k = j_al(b, tk, t, ben, "Online");
		jy_bicim(&y, ",\"cevrimici\":%s,\"ip\":[", k >= 0 && j_bool(b, &tk[k]) == 1 ? "true" : "false");
		k = j_al(b, tk, t, ben, "TailscaleIPs");
		for (int i = 0; k >= 0 && tk[k].tip == J_DIZI && i < tk[k].boyut && i < 4; i++) {
			int x = j_oge(tk, t, k, i);
			if (x >= 0 && !j_dizge_al(b, &tk[x], s1, 46)) { jy_ham(&y, i ? "," : ""); jy_dizge(&y, s1); }
		}
		jy_ham(&y, "],\"tailnet\":");
		k = j_al(b, tk, t, tn, "Name");
		jy_dizge(&y, k >= 0 && !j_dizge_al(b, &tk[k], s1, 121) ? s1 : "");
		k = j_al(b, tk, t, tn, "MagicDNSEnabled");
		jy_bicim(&y, ",\"magicdns\":%s,\"saglik\":[", k >= 0 && j_bool(b, &tk[k]) == 1 ? "true" : "false");
		k = j_al(b, tk, t, 0, "Health");
		for (int i = 0; k >= 0 && tk[k].tip == J_DIZI && i < tk[k].boyut && i < 8; i++) {
			int x = j_oge(tk, t, k, i);
			if (x >= 0 && !j_dizge_al(b, &tk[x], s1, 201)) { jy_ham(&y, i ? "," : ""); jy_dizge(&y, s1); }
		}
		jy_ham(&y, "],\"esler\":[");
		k = j_al(b, tk, t, 0, "Peer");
		int en = 0;
		if (k >= 0 && tk[k].tip == J_NESNE) {
			int i = k + 1;
			for (int p = 0; p < tk[k].boyut && en < 50; p++) {
				int v = i + 1;
				char a[64] = "", os[24] = "";
				int h = j_al(b, tk, t, v, "HostName"), o = j_al(b, tk, t, v, "OS"), on = j_al(b, tk, t, v, "Online");
				if (h >= 0) j_dizge_al(b, &tk[h], a, sizeof a);
				if (o >= 0) j_dizge_al(b, &tk[o], os, sizeof os);
				jy_ham(&y, en++ ? ",{\"ad\":" : "{\"ad\":");
				jy_dizge(&y, a);
				jy_ham(&y, ",\"os\":");
				jy_dizge(&y, os);
				jy_bicim(&y, ",\"cevrimici\":%s}", on >= 0 && j_bool(b, &tk[on]) == 1 ? "true" : "false");
				i = j_atla(tk, t, v);
			}
		}
		jy_ham(&y, "]");
		jy_ham(&y, ",\"adres\":");
		if (dns[0]) jy_bicim(&y, "\"https://%s/\"", dns); else jy_ham(&y, "null");
		jy_ham(&y, ",\"giris_adresi\":");
		k = j_al(b, tk, t, 0, "AuthURL");
		if (giris_gerek && k >= 0 && !j_dizge_al(b, &tk[k], s1, 200) && !strncmp(s1, "https://", 8)) jy_dizge(&y, s1);
		else jy_ham(&y, "null");
	} else {
		jy_ham(&y, ",\"calisiyor\":false");
	}
	/* serve ozeti */
	int sv_aktif = 0, merkez = 0, funnel = 0;
	jy_ham(&y, ",\"serve\":{\"hedefler\":[");
	t = 0;
	if (yas_ms_m(ts_serve_yol) >= 0 && yas_ms_m(ts_serve_yol) <= 120000 && dosya_oku(ts_serve_yol, b, sizeof b) > 0)
		t = json_ayristir(b, (int)strlen(b), tk, 4096);
	if (t > 0 && tk[0].tip == J_NESNE) {
		int w = j_al(b, tk, t, 0, "Web"), hn = 0;
		if (w >= 0 && tk[w].tip == J_NESNE) {
			int i = w + 1;
			for (int p = 0; p < tk[w].boyut; p++) {
				char host[128] = "";
				j_dizge_al(b, &tk[i], host, sizeof host);
				int v = i + 1, hs = j_al(b, tk, t, v, "Handlers");
				if (hs >= 0 && tk[hs].tip == J_NESNE) {
					int q = hs + 1;
					for (int r = 0; r < tk[hs].boyut; r++) {
						char yol[64] = "", px[128] = "";
						j_dizge_al(b, &tk[q], yol, sizeof yol);
						int pr = j_al(b, tk, t, q + 1, "Proxy");
						if (pr >= 0 && !j_dizge_al(b, &tk[pr], px, sizeof px) && hn < 10) {
							jy_ham(&y, hn++ ? ",{\"adres\":" : "{\"adres\":");
							jy_dizge(&y, host);
							jy_ham(&y, ",\"yol\":");
							jy_dizge(&y, yol);
							jy_ham(&y, ",\"hedef\":");
							jy_dizge(&y, px);
							jy_ham(&y, "}");
							char port[16];
							snprintf(port, sizeof port, ":%d", M.port);
							if ((strstr(px, "127.0.0.1") || strstr(px, "localhost")) && strstr(px, port)) merkez = 1;
							sv_aktif = 1;
						}
						q = j_atla(tk, t, q + 1);
					}
				}
				i = j_atla(tk, t, v);
			}
		}
		int tcp = j_al(b, tk, t, 0, "TCP");
		if (tcp >= 0 && tk[tcp].tip == J_NESNE && tk[tcp].boyut) sv_aktif = 1;
		int f = j_al(b, tk, t, 0, "AllowFunnel");
		if (f >= 0 && tk[f].tip == J_NESNE) {
			int i = f + 1;
			for (int p = 0; p < tk[f].boyut; p++) { if (j_bool(b, &tk[i + 1]) == 1) funnel = 1; i = j_atla(tk, t, i + 1); }
		}
	}
	jy_bicim(&y, "],\"aktif\":%s,\"merkez\":%s,\"funnel\":%s}", sv_aktif ? "true" : "false", merkez ? "true" : "false",
		 funnel ? "true" : "false");
	jy_bicim(&y, ",\"komutlar\":[{\"ad\":\"Tailscale'i aç (SD bayrağı, sonraki açılışta ya da gözeticiyle)\",\"komut\":\"touch %s\"},"
		 "{\"ad\":\"Merkezi tailnet içinde HTTPS ile yayınla (cihazda)\",\"komut\":\"/mnt/sd/velinux/tailscale/tailscale serve --bg http://127.0.0.2:%d\"},"
		 "{\"ad\":\"Yayını denetle\",\"komut\":\"/mnt/sd/velinux/tailscale/tailscale serve status\"},"
		 "{\"ad\":\"Yayını kapat\",\"komut\":\"/mnt/sd/velinux/tailscale/tailscale serve --https=443 off\"}]",
		 ts_bayrak_yol, M.port);
	jy_ham(&y, ",\"bu_baglanti\":{\"tailscale_kullanici\":");
	if (c->vekil && c->r.ts_login[0]) jy_dizge(&y, c->r.ts_login); else jy_ham(&y, "null");
	jy_bicim(&y, ",\"yerel\":%s,\"https\":%s}", ip_yerel(c->ip) ? "true" : "false", c->https ? "true" : "false");
	jy_bicim(&y, ",\"guvenlik\":[{\"ad\":\"İlk kurulum tamamlandı (kurulum anahtarı silindi)\",\"ok\":%s},"
		 "{\"ad\":\"Merkez yalnız 127.0.0.1 dinliyor (port açılmadı)\",\"ok\":%s},"
		 "{\"ad\":\"Tailscale Funnel (herkese açık internet) KAPALI\",\"ok\":%s},"
		 "{\"ad\":\"Kısayol jetonu kameraya erişemez\",\"ok\":true},"
		 "{\"ad\":\"Açık oturum sayısı: %d\",\"ok\":%s},"
		 "{\"ad\":\"%s\",\"ok\":%s}]}",
		 dosya_var(kurulum_yol) ? "false" : "true", M.lan ? "false" : "true", funnel ? "false" : "true",
		 oturum_sayisi(), oturum_sayisi() <= 4 ? "true" : "false",
		 M.plan_acik ? "Kayıt planı AÇIK (bilinçli olmalı)" : "Kayıt planı kapalı", M.plan_acik ? "false" : "true");
	memset(b, 0, sizeof b);
	yanit_json(c, 200, &y, NULL);
}

/* ------------------------------------------------------------------ denetim gunlugu API */
static const char *denetim_tur(const char *olay)
{
	char u[320];
	size_t i = 0;
	for (; olay[i] && i < sizeof u - 1; i++) u[i] = (char)toupper((unsigned char)olay[i]);
	u[i] = 0;
	if (strstr(u, "GUVENLIK")) return "guvenlik";	/* v2: mod degisimi giris sayilmasin (PC v7 ile ayni) */
	if (strstr(u, "GIRIS")) return "giris";
	if (strstr(u, "SES DINLEME") || strstr(u, "KONUSMA") || strstr(u, "SES IZIN")) return "ses";
	if (strstr(u, "KAMERA") || strstr(u, "RTSP")) return "kamera";
	if (strstr(u, "KAYIT") || strstr(u, "PLAN")) return "kayit";
	if (strstr(u, "OTURUM") || strstr(u, "PAROLA") || strstr(u, "JETON") || strstr(u, "KURULUM") || strstr(u, "KILIT")) return "guvenlik";
	return "diger";
}

static void denetim_ac_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	O[c->oturum].denetim_bitis = simdi() + DENETIM_IZIN_SN;
	denetim(c->ip, "denetim gunlugu goruntulendi%s", dog_ek);
	struct jy y = { .azami = 64 };
	jy_bicim(&y, "{\"ok\":true,\"kalan_sn\":%d}", DENETIM_IZIN_SN);
	yanit_json(c, 200, &y, NULL);
}

static void api_denetim(struct bag *c)
{
	if (izin(c, 0)) return;
	if (!M.siki) {	/* v2 tek_giris: izin yoksa verilir (denetime yazilir), varsa tazelenir */
		int yeni = !izin_aktif(c->oturum, O[c->oturum].denetim_bitis);
		O[c->oturum].denetim_bitis = simdi() + DENETIM_IZIN_SN;
		if (yeni) denetim(c->ip, "denetim gunlugu goruntulendi" TEK_EK);
	}
	if (!izin_aktif(c->oturum, O[c->oturum].denetim_bitis)) { hata_yanit(c, 423, "Denetim günlüğü için parolanızı girin"); return; }
	char ns[16] = "", tur[16] = "";
	int n = 200;
	int r = sorgu_al(c->r.sorgu, "n", ns, sizeof ns);
	if (r < 0 || (r == 1 && (!ns[0] || strspn(ns, "0123456789") != strlen(ns) || strlen(ns) > 6))) { hata_yanit(c, 400, NULL); return; }
	if (r == 1) n = atoi(ns);
	if (n < 1) n = 1;
	if (n > 500) n = 500;
	if (sorgu_al(c->r.sorgu, "tur", tur, sizeof tur) < 0) { hata_yanit(c, 400, NULL); return; }
	/* son 256 KB (gerekirse bir onceki dosyadan da) */
	const char *yol = denetim_yolu();
	char y1[320];
	snprintf(y1, sizeof y1, "%s.1", yol);
	size_t cap = 256 * 1024 + 1;
	char *m = malloc(cap * 2);
	if (!m) { hata_yanit(c, 503, NULL); return; }
	size_t mn = 0;
	const char *dosyalar[2] = { y1, yol };
	for (int d = 0; d < 2; d++) {
		int fd = open(dosyalar[d], O_RDONLY | O_CLOEXEC);
		if (fd < 0) continue;
		off_t L = lseek(fd, 0, SEEK_END);
		off_t bas = L > (off_t)(cap - 1) ? L - (off_t)(cap - 1) : 0;
		ssize_t k = pread(fd, m + mn, (size_t)(L - bas), bas);
		close(fd);
		if (k > 0) mn += (size_t)k;
	}
	m[mn] = 0;
	struct jy y = { .azami = 512 * 1024 };
	jy_ham(&y, "{\"kayitlar\":[");
	int adet = 0;
	char *son = m + mn;
	while (son > m && adet < n) {
		char *bas = son;
		while (bas > m && bas[-1] != '\n') bas--;
		size_t L = (size_t)(son - bas);
		char sat[512];
		if (L && L < sizeof sat) {
			memcpy(sat, bas, L);
			sat[L] = 0;
			if (sat[L - 1] == '\n') sat[--L] = 0;
			char *s1 = strchr(sat, ' '), *s2 = s1 ? strchr(s1 + 1, ' ') : NULL;
			if (s1 && s2 && s1 != sat && s2 != s1 + 1) {
				*s1 = 0;
				*s2 = 0;
				const char *olay = s2 + 1, *tr = denetim_tur(olay);
				if (!tur[0] || !strcmp(tur, tr)) {
					jy_ham(&y, adet ? ",{\"zaman\":" : "{\"zaman\":");
					jy_dizge(&y, sat);
					jy_ham(&y, ",\"ip\":");
					jy_dizge(&y, s1 + 1);
					jy_ham(&y, ",\"olay\":");
					jy_dizge(&y, olay);
					jy_ham(&y, ",\"tur\":");
					jy_dizge(&y, tr);
					jy_bicim(&y, ",\"uyari\":%s}", strstr(olay, "REDDEDILDI") || strstr(olay, "BASARISIZ") || strstr(olay, "ENGELLENDI")
							 ? "true" : "false");
					adet++;
				}
			}
		}
		son = bas > m ? bas - 1 : m;
		if (bas == m) break;
	}
	free(m);
	jy_bicim(&y, "],\"kalan_sn\":%d}", izin_kalan(c->oturum, O[c->oturum].denetim_bitis));
	yanit_json(c, 200, &y, NULL);
}

/* ------------------------------------------------------------------ ses API */
static int ses_dk_gecerli(long long d) { return d == 2 || d == 5 || d == 15 || d == 30; }

static void api_ses_durum(struct bag *c)
{
	if (izin(c, 0)) return;
	struct oturum *o = &O[c->oturum];
	const char *h = hoparlor_durumu();
	const char *hm = !strcmp(h, "dogrulanmadi") ? "Panel hoparlörü henüz çalışmıyor (ses sorunu araştırılıyor): konuşmanız cihaza "
		"iletilir ama duyulmayabilir." : !strcmp(h, "yok") ? "Cihazda ses çıkışı yok: konuşma gönderilemez." : "";
	struct jy y = { .azami = 4096 };
	jy_bicim(&y, "{\"destek\":true,\"neden\":\"\",\"hoparlor\":\"%s\",\"hoparlor_metin\":", h);
	jy_dizge(&y, hm);
	jy_bicim(&y, ",\"kalkan\":%s,\"mik_aktif\":%s,\"izin\":{\"dinle\":%d,\"konus\":%d},\"dinleyen\":%d,\"konusan\":%s,\"ust\":",
		 kalkan() ? "true" : "false", dosya_var(mik_bayrak) ? "true" : "false", izin_kalan(c->oturum, o->dinle_bitis),
		 izin_kalan(c->oturum, o->konus_bitis), dinleyici_sayisi(), M.kon_bag >= 0 ? "true" : "false");
	jy_dizge(&y, mic_durum_metni());
	jy_ham(&y, ",\"hata\":");
	jy_dizge(&y, M.mic_durum == 3 ? M.mic_hata : "");
	jy_bicim(&y, ",\"asistan\":false,\"azami_konusma_sn\":%d,\"bicim\":{\"hz\":16000,\"kanal\":1,\"ornek\":\"s16le\"}}", KONUSMA_AZAMI_SN);
	yanit_json(c, 200, &y, NULL);
}

static int ses_tur_bekleyen;	/* 0 dinle, 1 konus */

static void ses_izin_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	long long dk = 5;
	char t[16];
	g_str("tur", t, sizeof t);
	g_int("dakika", &dk);
	int konus = !strcmp(t, "konus");
	if (konus) O[c->oturum].konus_bitis = simdi() + dk * 60.0;
	else O[c->oturum].dinle_bitis = simdi() + dk * 60.0;
	denetim(c->ip, "%s IZNI VERILDI %lld dk%s", konus ? "KONUSMA" : "SES DINLEME", dk, dog_ek);
	struct jy y = { .azami = 64 };
	jy_bicim(&y, "{\"ok\":true,\"kalan_sn\":%lld}", dk * 60);
	yanit_json(c, 200, &y, NULL);
}

static void api_ses_izin(struct bag *c)
{
	if (izin(c, 0)) return;
	char t[16];
	if (g_str("tur", t, sizeof t) || (strcmp(t, "dinle") && strcmp(t, "konus"))) { hata_yanit(c, 400, "tur: dinle ya da konus"); return; }
	long long dk = 5;
	int r = g_int("dakika", &dk);
	if (r == -1 || !ses_dk_gecerli(dk)) { hata_yanit(c, 400, "Süre 2/5/15/30 dk olmalı"); return; }
	ses_tur_bekleyen = !strcmp(t, "konus");
	hassas(c, ses_tur_bekleyen ? "KONUSMA IZNI" : "SES DINLEME IZNI", ses_izin_devam);
}

static void api_ses_kilitle(struct bag *c)
{
	if (izin(c, 0)) return;
	char t[16] = "";
	g_str("tur", t, sizeof t);
	int d = !strcmp(t, "dinle"), k = !strcmp(t, "konus");
	if (d || (!d && !k)) O[c->oturum].dinle_bitis = 0;
	if (k || (!d && !k)) O[c->oturum].konus_bitis = 0;
	denetim(c->ip, "ses izinleri kilitlendi%s", d ? " (dinle)" : k ? " (konus)" : "");
	ok_yanit(c);
}

/* WebSocket el sikismasi denetimi; 0 ok (101 kuyrukta), -1 yanit gonderildi */
static int ws_el_sikis(struct bag *c)
{
	if (strcasecmp(c->r.upgrade, "websocket") || !baglanti_upgrade_mi(&c->r)) { hata_yanit(c, 400, "WebSocket bağlantısı gerekli"); return -1; }
	if (strcmp(c->r.ws_ver, "13")) { hata_yanit(c, 400, "WebSocket sürümü 13 olmalı"); return -1; }
	uint8_t k[32];
	if (b64_coz(c->r.ws_key, k, sizeof k) != 16) { hata_yanit(c, 400, "WebSocket anahtarı geçersiz"); return -1; }
	if (!koken_uygun(c, 1)) {
		denetim(c->ip, "WEBSOCKET REDDEDILDI (koken)");
		hata_yanit(c, 403, "Kaynak (Origin) uygun değil");
		return -1;
	}
	char g[128];
	uint8_t h[20];
	snprintf(g, sizeof g, "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", c->r.ws_key);
	sha1_tek(g, strlen(g), h);
	char a[40], o[256];
	b64_kodla(h, 20, a, sizeof a);
	int n = snprintf(o, sizeof o, "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\n"
			 "Sec-WebSocket-Accept: %s\r\n\r\n", a);
	if (cikis_ekle(c, o, (size_t)n)) return -1;
	size_t tuk = c->r.baslik_n + c->govde_n;	/* HTTP istegi tuketilir; ardindan gelen baytlar WebSocket cercevesidir */
	if (tuk > c->gn) tuk = c->gn;
	memmove(c->gelen, c->gelen + tuk, c->gn - tuk);
	c->gn -= tuk;
	c->keepalive = 0;
	c->t0 = simdi();
	c->ws_son_veri = simdi();
	c->ses_bayt = c->ses_atilan = 0;
	c->ornek = 0;
	c->neden[0] = 0;
	return 0;
}

static void api_ses_dinle(struct bag *c)
{
	if (izin(c, 0)) return;
	if (!izin_aktif(c->oturum, O[c->oturum].dinle_bitis)) { hata_yanit(c, 423, "Dinleme izni yok ya da süresi doldu; parolanızı girin"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık: mikrofon/hoparlör uzaktan kullanılamaz"); return; }
	if (dinleyici_sayisi() >= AZAMI_DINLEYICI) { hata_yanit(c, 409, "En fazla 2 dinleyici"); return; }
	if (ws_el_sikis(c)) return;
	c->tur = B_WS_DINLE;
	c->ws_seq = M.mic_seq - 1;
	denetim(c->ip, "SES DINLEME BASLADI");
	struct jy y = { .azami = 256 };
	jy_bicim(&y, "{\"t\":\"durum\",\"hz\":16000,\"kanal\":1,\"bicim\":\"s16le\",\"kalan_sn\":%d}", izin_kalan(c->oturum, O[c->oturum].dinle_bitis));
	ws_metin(c, &y);
	if (M.mic_pid <= 0 && simdi() >= M.mic_yeniden_t) mic_baslat();
}

static void api_ses_konus(struct bag *c)
{
	if (izin(c, 0)) return;
	if (!izin_aktif(c->oturum, O[c->oturum].konus_bitis)) { hata_yanit(c, 423, "Konuşma izni yok ya da süresi doldu; parolanızı girin"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık: mikrofon/hoparlör uzaktan kullanılamaz"); return; }
	if (M.kon_bag >= 0 || M.kon_pid > 0) { hata_yanit(c, 409, "Şu anda başka biri konuşuyor"); return; }
	if (ws_el_sikis(c)) return;
	c->tur = B_WS_KONUS;
	c->kon_bitiyor = 0;
	M.kon_bag = (int)(c - B);
	denetim(c->ip, "KONUSMA BASLADI (panel hoparlorune)");
	if (kon_baslat()) {
		struct jy y = { .azami = 256 };
		jy_ham(&y, "{\"t\":\"hata\",\"metin\":\"Ses süreci başlatılamadı\"}");
		ws_metin(c, &y);
		snprintf(c->neden, sizeof c->neden, "ses süreci başlatılamadı");
		c->kapat_sonra = 1;
		M.kon_bag = -1;
	}
}

/* ------------------------------------------------------------------ RTSP yonetimi (sunucu kole-kopru-v8 rtsp) */
static int lan_ip(char *o, size_t n)
{
	o[0] = 0;
	FILE *f = fopen("/proc/net/route", "r");
	char s[256], ifn[32] = "";
	while (f && fgets(s, sizeof s, f)) {
		char ad[32];
		unsigned long hedef;
		if (sscanf(s, "%31s %lx", ad, &hedef) == 2 && hedef == 0) { snprintf(ifn, sizeof ifn, "%s", ad); break; }
	}
	if (f) fclose(f);
	if (!ifn[0]) return -1;
	int s2 = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
	if (s2 < 0) return -1;
	struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons(9), .sin_addr.s_addr = htonl(0x08080808) };
	struct sockaddr_in yer;
	socklen_t L = sizeof yer;
	int r = connect(s2, (struct sockaddr *)&a, sizeof a) || getsockname(s2, (struct sockaddr *)&yer, &L);
	close(s2);
	if (r) return -1;
	inet_ntop(AF_INET, &yer.sin_addr, o, (socklen_t)n);
	return 0;
}

static void api_rtsp_get(struct bag *c)
{
	if (izin(c, 0)) return;
	static char b[1024];
	static struct jtok tk[128];
	struct jy y = { .azami = 16384 };
	jy_bicim(&y, "{\"destek\":true,\"kullanici\":\"" RTSP_KULLANICI "\",\"bayrak\":%s,\"parola_dosyasi\":%s,\"kalkan\":%s,\"durum\":",
		 dosya_var(rtsp_bayrak_m) ? "true" : "false", dosya_var(rtsp_parola_m) ? "true" : "false", kalkan() ? "true" : "false");
	long long yas = yas_ms_m(rtsp_durum_m);
	int t = 0;
	if (yas >= 0 && yas <= 10000 && dosya_oku(rtsp_durum_m, b, sizeof b) > 0) t = json_ayristir(b, (int)strlen(b), tk, 128);
	if (t > 0 && tk[0].tip == J_NESNE) {
		static const char *IB[] = { "calisiyor", "acik", "dinliyor", "parola", "kilitli", "dht_standart" };
		static const char *II[] = { "port", "istemci", "oynatan", "azami_oynatan", "kare", "red", "fps" };
		static const char *IM[] = { "video", "ses", "video_hata", "ses_hata", "jpeg_uyum" };
		int ilk = 1;
		jy_ham(&y, "{");
		for (size_t i = 0; i < sizeof IB / sizeof IB[0]; i++) {
			int k = j_al(b, tk, t, 0, IB[i]);
			double v;
			int bv = k >= 0 ? j_bool(b, &tk[k]) : -1;
			if (k < 0 || (bv < 0 && j_sayi(b, &tk[k], &v))) continue;
			if (bv < 0) bv = v != 0;
			jy_bicim(&y, "%s\"%s\":%s", ilk ? "" : ",", IB[i], bv ? "true" : "false");
			ilk = 0;
		}
		for (size_t i = 0; i < sizeof II / sizeof II[0]; i++) {
			int k = j_al(b, tk, t, 0, II[i]);
			long long v;
			if (k < 0 || j_tamsayi(b, &tk[k], &v)) continue;
			jy_bicim(&y, "%s\"%s\":%lld", ilk ? "" : ",", II[i], v);
			ilk = 0;
		}
		for (size_t i = 0; i < sizeof IM / sizeof IM[0]; i++) {
			int k = j_al(b, tk, t, 0, IM[i]);
			char s[128] = "";
			if (k >= 0 && j_dizge_al(b, &tk[k], s, sizeof s)) s[0] = 0;
			s[120] = 0;
			jy_bicim(&y, "%s\"%s\":", ilk ? "" : ",", IM[i]);
			jy_dizge(&y, s);
			ilk = 0;
		}
		jy_ham(&y, "}");
	} else jy_ham(&y, "null");
	jy_bicim(&y, ",\"durum_yas_ms\":");
	sayi_null(&y, yas);
	char ip[64];
	jy_ham(&y, ",\"adresler\":[");
	int ilk = 1;
	if (!lan_ip(ip, sizeof ip)) {
		jy_bicim(&y, "{\"ad\":\"Ev ağı (Wi-Fi)\",\"url\":\"rtsp://" RTSP_KULLANICI "@%s:%d/kamera\",\"not\":\"görüntü + ses\"},"
			 "{\"ad\":\"Ev ağı, yalnız görüntü\",\"url\":\"rtsp://" RTSP_KULLANICI "@%s:%d/video\",\"not\":\"daha az bant\"}",
			 ip, RTSP_PORT, ip, RTSP_PORT);
		ilk = 0;
	}
	/* tailnet adresi (cihazda Tailscale; serve --tcp 8554 gerekir) */
	static char tb[65536];
	static struct jtok tt[4096];
	long long ty = yas_ms_m(ts_durum_yol);
	int tn = ty >= 0 && ty <= 120000 && dosya_oku(ts_durum_yol, tb, sizeof tb) > 0 ? json_ayristir(tb, (int)strlen(tb), tt, 4096) : 0;
	if (tn > 0 && tt[0].tip == J_NESNE) {
		int ben = j_al(tb, tt, tn, 0, "Self"), ips = j_al(tb, tt, tn, ben, "TailscaleIPs"), x = j_oge(tt, tn, ips, 0);
		char tip[64];
		if (x >= 0 && !j_dizge_al(tb, &tt[x], tip, sizeof tip) && strspn(tip, "0123456789.") == strlen(tip)) {
			jy_bicim(&y, "%s{\"ad\":\"Dışarıdan (cihazda Tailscale)\",\"url\":\"rtsp://" RTSP_KULLANICI "@%s:%d/kamera\","
				 "\"not\":\"cihazda: tailscale serve --bg --tcp %d tcp://localhost:%d\"}", ilk ? "" : ",", tip, RTSP_PORT, RTSP_PORT, RTSP_PORT);
		}
	}
	jy_ham(&y, "],\"aktarici\":{\"acik\":false,\"baglanti\":0,\"toplam\":0,\"yerel\":null}}");
	yanit_json(c, 200, &y, NULL);
}

static void rtsp_uygula(struct bag *c, int acik)
{
	if (acik && kalkan()) {
		denetim(c->ip, "RTSP ACMA ENGELLENDI (panelde gizlilik kalkani acik)");
		hata_yanit(c, 409, "Panelde gizlilik kalkanı açık; RTSP uzaktan açılamaz");
		return;
	}
	bayrak_yaz(rtsp_bayrak_m, acik);
	sync();
	if (dosya_var(rtsp_bayrak_m) != acik) {
		denetim(c->ip, "RTSP %s BASARISIZ (SD)", acik ? "acma" : "kapatma");
		hata_yanit(c, 502, "Cihaz RTSP isteğini onaylamadı");
		return;
	}
	denetim(c->ip, acik ? "RTSP ACILDI (IP kamera yayini; parolali, yalniz yerel ag/Tailscale)" : "RTSP kapatildi");
	struct jy y = { .azami = 64 };
	jy_bicim(&y, "{\"ok\":true,\"acik\":%s}", acik ? "true" : "false");
	yanit_json(c, 200, &y, NULL);
}

static void rtsp_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	rtsp_uygula(c, 1);
}

static void api_rtsp_post(struct bag *c)
{
	if (izin(c, 0)) return;
	int acik = g_bool("acik");
	if (acik < 0) { hata_yanit(c, 400, NULL); return; }
	if (!acik) { rtsp_uygula(c, 0); return; }
	yeniden_parola(c, "RTSP ACMA", rtsp_devam);
}

static void rtsp_parola_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	static const char abc[] = "abcdefghjkmnpqrstuvwxyz23456789";
	uint8_t r[20];
	char p[32], ha1[33];
	rastgele(r, sizeof r);
	int k = 0;
	for (int i = 0; i < 20; i++) { if (i && i % 5 == 0) p[k++] = '-'; p[k++] = abc[r[i] % 31]; }
	p[k] = 0;
	const char *parca[] = { RTSP_KULLANICI, RTSP_REALM, p, NULL };
	md5_hex_parcalar(ha1, parca);
	char tmp[300], satir[64];
	snprintf(tmp, sizeof tmp, "%s.tmp", rtsp_parola_m);
	int n = snprintf(satir, sizeof satir, "%s %s\n", RTSP_KULLANICI, ha1);
	int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
	int yaz = fd >= 0 && !fchmod(fd, 0600) && !tam_yaz(fd, satir, (size_t)n) && !fsync(fd);
	if (fd >= 0) close(fd);
	if (!yaz || rename(tmp, rtsp_parola_m)) {
		unlink(tmp);
		denetim(c->ip, "RTSP PAROLASI BASARISIZ (/data)");
		hata_yanit(c, 502, "Cihaz parolayı kaydedemedi");
		memset(p, 0, sizeof p);
		return;
	}
	sync();
	denetim(c->ip, "RTSP PAROLASI YENILENDI (eski parola gecersiz)");
	struct jy y = { .azami = 256 };
	jy_ham(&y, "{\"ok\":true,\"kullanici\":\"" RTSP_KULLANICI "\",\"parola\":");
	jy_dizge(&y, p);
	jy_ham(&y, "}");
	yanit_json(c, 200, &y, NULL);
	memset(p, 0, sizeof p);
	memset(r, 0, sizeof r);
	memset(ha1, 0, sizeof ha1);
}

static void api_rtsp_parola(struct bag *c)
{
	if (izin(c, 0)) return;
	yeniden_parola(c, "RTSP PAROLASI", rtsp_parola_devam);
}

/* ------------------------------------------------------------------ v2: guvenlik modu (/api/guvenlik) */
static int bosta_saat(void) { return (int)(bosta_sn() / 3600); }

static void guvenlik_ozet(struct bag *c, struct jy *y)
{
	double t = simdi(), bk = 0, mk = 0;
	if (oturum_gecerli(c->oturum)) {
		bk = O[c->oturum].son_m + bosta_sn() - t;
		mk = O[c->oturum].bitis_m - t;
	}
	jy_bicim(y, "{\"mod\":\"%s\",\"bosta_saat\":%d,\"mutlak_gun\":%d,\"oturum\":{\"bosta_kalan_sn\":%d,\"mutlak_kalan_sn\":%d}}",
		 M.siki ? "siki" : "tek_giris", bosta_saat(), (int)(OTURUM_OMUR / 86400), bk > 0 ? (int)bk : 0, mk > 0 ? (int)mk : 0);
}

static void api_guvenlik_get(struct bag *c)
{
	if (izin(c, 0)) return;	/* Bearer jetonu yetmez */
	struct jy y = { .azami = 512 };
	guvenlik_ozet(c, &y);
	yanit_json(c, 200, &y, NULL);
}

/* govdeyi dogrula (PC v7 guvenlik_yaz ile ayni kurallar): -1 yanit gonderildi, 0 parolasiz, 1 parola gerekir.
 * tek_giris'e gecis ve bosta suresini UZATMA parola ister; siki'ye gecis ve kisaltma istemez. */
static int guvenlik_hazirla(struct bag *c, int *siki, int *saat)
{
	*siki = M.siki;
	*saat = bosta_saat();
	if (g_al("mod") >= 0) {
		char m[16];
		if (g_str("mod", m, sizeof m) || (strcmp(m, "tek_giris") && strcmp(m, "siki"))) {
			hata_yanit(c, 400, "mod: tek_giris ya da siki olmalı");
			return -1;
		}
		*siki = !strcmp(m, "siki");
	}
	if (g_al("bosta_saat") >= 0) {
		long long v;
		if (g_int("bosta_saat", &v) || v < BOSTA_ALT || v > BOSTA_UST) { hata_yanit(c, 400, "Boşta kalma süresi 1..72 saat olmalı"); return -1; }
		*saat = (int)v;
	}
	return (!*siki && M.siki) || *saat > bosta_saat();
}

static void guvenlik_uygula(struct bag *c)
{
	int siki, saat;
	if (guvenlik_hazirla(c, &siki, &saat) < 0) return;
	int eski_siki = M.siki, eski_saat = bosta_saat();
	if (siki != eski_siki || saat != eski_saat) {
		M.siki = siki;
		M.bosta_saat = saat;
		if (ayar_yaz()) {
			M.siki = eski_siki;
			M.bosta_saat = eski_saat;
			hata_yanit(c, 502, "Ayar /data'ya yazılamadı");
			return;
		}
		if (siki != eski_siki) {
			if (siki) {
				tum_izinleri_sil();	/* siki mod temiz baslar: tek_giris'te verilmis sureli izinler gecersiz */
				denetim(c->ip, "GUVENLIK MODU: siki (hassas islemler her seferinde parola ister)");
			} else {
				denetim(c->ip, "GUVENLIK MODU: tek dogrulama (oturum acikken hassas islemler parola istemez)");
			}
		}
		if (saat != eski_saat) denetim(c->ip, "OTURUM BOSTA SURESI %d -> %d saat", eski_saat, saat);
	}
	struct jy y = { .azami = 512 };
	guvenlik_ozet(c, &y);
	yanit_json(c, 200, &y, NULL);
}

static void guvenlik_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	guvenlik_uygula(c);
}

static void api_guvenlik_post(struct bag *c)
{
	if (izin(c, 0)) return;
	int siki, saat;
	int r = guvenlik_hazirla(c, &siki, &saat);
	if (r < 0) return;
	if (r) {
		yeniden_parola(c, !siki && M.siki ? "GUVENLIK MODU DEGISIMI" : "OTURUM BOSTA SURESI ARTIRMA", guvenlik_devam);
		return;
	}
	guvenlik_uygula(c);
}

/* ------------------------------------------------------------------ statik dosyalar (PWA, SD) */
static const char *icerik_turu(const char *ad)
{
	const char *n = strrchr(ad, '.');
	if (!n) return NULL;
	if (!strcmp(n, ".html")) return "text/html; charset=utf-8";
	if (!strcmp(n, ".js")) return "text/javascript; charset=utf-8";
	if (!strcmp(n, ".css")) return "text/css; charset=utf-8";
	if (!strcmp(n, ".png")) return "image/png";
	if (!strcmp(n, ".webmanifest")) return "application/manifest+json";
	if (!strcmp(n, ".json")) return "application/json";
	if (!strcmp(n, ".svg")) return "image/svg+xml";
	if (!strcmp(n, ".ico")) return "image/x-icon";
	if (!strcmp(n, ".txt")) return "text/plain; charset=utf-8";
	return NULL;
}

/* yalniz duz ad: [A-Za-z0-9][A-Za-z0-9._-]*, '..' yok, alt dizin yok */
static int statik_ad_gecerli(const char *ad)
{
	size_t n = strlen(ad);
	if (!n || n > 64 || !isalnum((unsigned char)ad[0]) || strstr(ad, "..")) return 0;
	return strspn(ad, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-") == n && icerik_turu(ad);
}

/* v8: gomulu hizli canli izleme sayfasi (SD'deki PWA'ya dokunmaz) */
#include "merkez_canli_v1.h"
static void canli_sayfa(struct bag *c)
{
	if (strcmp(c->r.yontem, "GET") && strcmp(c->r.yontem, "HEAD")) { hata_yanit(c, 405, "Yöntem desteklenmiyor"); return; }
	const char *y = c->r.yol;
	int js = !strcmp(y, "/canli.js"), css = !strcmp(y, "/canli.css");
	const char *g = js ? CANLI_JS : css ? CANLI_CSS : CANLI_HTML;
	yanit(c, 200, js ? "text/javascript; charset=utf-8" : css ? "text/css; charset=utf-8" : "text/html; charset=utf-8", g,
	      strlen(g), "Cache-Control: no-cache\r\n");
}

static void statik(struct bag *c)
{
	if (strcmp(c->r.yontem, "GET") && strcmp(c->r.yontem, "HEAD")) { hata_yanit(c, 405, "Yöntem desteklenmiyor"); return; }
	const char *ad = !strcmp(c->r.yol, "/") ? "index.html" : c->r.yol + 1;
	char coz[128];
	if (url_coz(ad, strlen(ad), coz, sizeof coz) || !statik_ad_gecerli(coz)) {
		yanit(c, 404, "text/plain; charset=utf-8", "Bulunamadı\n", 12, NULL);
		return;
	}
	char yol[400];
	snprintf(yol, sizeof yol, "%s/%s", web_kok, coz);
	int fd = open(yol, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
	struct stat st;
	if (fd < 0 || fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size > 4 * 1024 * 1024) {
		if (fd >= 0) close(fd);
		yanit(c, 404, "text/plain; charset=utf-8", "Bulunamadı\n", 12, NULL);
		return;
	}
	yanit_bas(c, 200, icerik_turu(coz), (long long)st.st_size, !strcmp(coz, "sw.js") ? "Service-Worker-Allowed: /\r\n" : NULL);
	c->tur = B_YANIT;
	if (c->head) { close(fd); return; }
	c->dosya_fd = fd;
	c->dosya_kalan = (unsigned long long)st.st_size;
	c->hiz = 0;
}

/* ------------------------------------------------------------------ v18: asistana sor / panelde soyle / hatirlatici */
#define SOR_AZAMI_SN 60
#define SOYLE_AZAMI_SN 90

/* en yuksek surumlu kole-gemini-vN (autorun ile ayni secim); yoksa -1 */
static int gemini_bul(char *o, size_t n)
{
	for (int v = 99; v >= 2; v--) {
		snprintf(o, n, "%s/kole-gemini-v%d", gemini_dizin, v);
		if (!access(o, X_OK)) return 0;
	}
	return -1;
}

/* Go cocugu (autorun Pano/Telegram tek atislariyla ayni bellek siniri); tut >= 0 ise stdout oraya */
static pid_t gemini_cocuk(int tut, const char *ikili, const char *kip, const char *metin)
{
	fflush(stdout);
	pid_t p = fork();
	if (p) return p;
	cocuk_hazirla(tut);
	int nul = open("/dev/null", O_RDWR);
	if (nul >= 0) dup2(nul, 0);
	dup2(tut >= 0 ? tut : nul, 1);
	setenv("SSL_CERT_FILE", "/mnt/sd/velinux/tailscale/ca-certificates.crt", 1);
	setenv("GOMEMLIMIT", "12MiB", 1);
	setenv("GOGC", "50", 1);
	if (!strcmp(kip, "--akis")) {
		setenv("KOLE_KONUS_METIN", metin, 1);
		execl(ikili, "kole-gemini", kip, (char *)NULL);
	} else execl(ikili, "kole-gemini", kip, metin, (char *)NULL);
	_exit(127);
}

/* 0 basladi; -1 mesgul / ikili yok / kalkan */
static int soyle_baslat(const char *metin)
{
	char ik[400];
	if (M.soyle_pid > 0 || kalkan() || gemini_bul(ik, sizeof ik)) return -1;
	pid_t p = gemini_cocuk(-1, ik, "--akis", metin);
	if (p < 0) return -1;
	M.soyle_pid = p;
	M.soyle_bitis = simdi() + SOYLE_AZAMI_SN;
	return 0;
}

static void api_soyle(struct bag *c)
{
	if (izin(c, 1)) return;
	char m[1024];
	govde_ayristir(c);
	if (g_str("metin", m, sizeof m) || !m[0] || strlen(m) > 900) { hata_yanit(c, 400, "Metin boş ya da çok uzun"); return; }
	if (kalkan()) { hata_yanit(c, 409, "Panelde gizlilik kalkanı açık: hoparlör uzaktan kullanılamaz"); return; }
	if (M.soyle_pid > 0) { hata_yanit(c, 409, "Panel şu an başka bir şey söylüyor; birazdan yeniden deneyin"); return; }
	if (soyle_baslat(m)) { hata_yanit(c, 503, "Cihazda kole-gemini bulunamadı"); return; }
	denetim(c->ip, "PANELDE SOYLETILDI (%d bayt)", (int)strlen(m));
	static const char e[] = "{\"ok\":true,\"hedef\":\"panel\"}";
	yanit(c, 200, "application/json", e, sizeof e - 1, NULL);
}

static void api_sor(struct bag *c)
{
	if (izin(c, 1)) return;
	char m[1024], ses[8] = "", ik[400];
	govde_ayristir(c);
	if (g_str("metin", m, sizeof m) || !m[0] || strlen(m) > 900) { hata_yanit(c, 400, "Soru boş ya da çok uzun"); return; }
	g_str("sesli", ses, sizeof ses);
	if (M.sor_fd >= 0) { hata_yanit(c, 409, "Asistan başka bir soruyu cevaplıyor; birazdan yeniden deneyin"); return; }
	if (gemini_bul(ik, sizeof ik)) { hata_yanit(c, 503, "Cihazda kole-gemini bulunamadı"); return; }
	int p[2];
	if (pipe2(p, O_CLOEXEC)) { hata_yanit(c, 503, NULL); return; }
	pid_t pid = gemini_cocuk(p[1], ik, "--metin", m);
	close(p[1]);
	if (pid < 0) { close(p[0]); hata_yanit(c, 503, NULL); return; }
	fcntl(p[0], F_SETFL, O_NONBLOCK);
	M.sor_pid = pid;
	M.sor_fd = p[0];
	M.sor_n = 0;
	M.sor_bag = (int)(c - B);
	M.sor_bag_fd = c->fd;
	M.sor_sesli = ses[0] != 0;
	M.sor_bitis = simdi() + SOR_AZAMI_SN;
	c->tur = B_BEKLE;
	denetim(c->ip, "ASISTANA SORULDU (%d bayt)", (int)strlen(m));
}

/* cocuk ciktisi bitti (ya da zaman asimi): bekleyen istege {"yanit"} */
static void sor_bitir(const char *hata)
{
	if (M.sor_fd >= 0) { close(M.sor_fd); M.sor_fd = -1; }
	if (M.sor_pid > 0) kill(M.sor_pid, SIGKILL);	/* zaman asimi; cikmissa toplayici zaten sifirlar */
	M.sor_cikti[M.sor_n < sizeof M.sor_cikti ? M.sor_n : sizeof M.sor_cikti - 1] = 0;
	static struct jtok tk[512];
	char cevap[4096] = "";
	int t = hata ? -1 : json_ayristir(M.sor_cikti, (int)M.sor_n, tk, 512);
	int i = t > 0 && tk[0].tip == J_NESNE ? j_al(M.sor_cikti, tk, t, 0, "cevap") : -1;
	if (i < 0 || tk[i].tip != J_DIZGE || j_dizge_al(M.sor_cikti, &tk[i], cevap, sizeof cevap)) cevap[0] = 0;
	if (cevap[0] && M.sor_sesli) soyle_baslat(cevap);	/* mesgulse ya da kalkan aciksa sessiz gecer */
	struct bag *c = M.sor_bag >= 0 ? &B[M.sor_bag] : NULL;
	M.sor_bag = -1;
	M.sor_n = 0;
	if (!c || c->fd < 0 || c->fd != M.sor_bag_fd || c->tur != B_BEKLE) return;
	c->tur = B_YANIT;
	if (!cevap[0]) {
		hata_yanit(c, 502, hata && !strcmp(hata, "zaman aşımı") ? "Asistan 60 sn içinde cevap vermedi"
			   : "Asistan şu an cevap veremedi (Gemini kotası ya da bağlantı)");
		return;
	}
	struct jy y = { .azami = 8192 };
	jy_ham(&y, "{\"yanit\":");
	jy_dizge(&y, cevap);
	jy_ham(&y, "}");
	yanit_json(c, 200, &y, NULL);
}

static void sor_oku(void)
{
	ssize_t r = read(M.sor_fd, M.sor_cikti + M.sor_n, sizeof M.sor_cikti - 1 - M.sor_n);
	if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
	if (r > 0) {
		M.sor_n += (size_t)r;
		if (M.sor_n < sizeof M.sor_cikti - 1) return;
	}
	sor_bitir(NULL);
}

struct htm { long long z; char t; char m[210]; };

/* panelin hatirlatici dosyasi (dosya sirasi = panel dizisi sirasi) */
static int ht_oku(struct htm *l, int az)
{
	static char b[8192];
	if (dosya_oku(hatirlat_yol, b, sizeof b) <= 0) return 0;
	int k = 0;
	for (char *s = b; *s && k < az;) {
		char *e = strchr(s, '\n');
		if (e) *e = 0;
		long long z;
		char t;
		int o = 0;
		if (sscanf(s, "%lld|%c|%n", &z, &t, &o) == 2 && o > 0 && z > 0) {
			l[k].z = z;
			l[k].t = t == 'a' || t == 'g' ? t : 'h';
			snprintf(l[k].m, sizeof l[k].m, "%s", s + o);
			k++;
		}
		if (!e) break;
		s = e + 1;
	}
	return k;
}

/* panel eylem dosyasi; mtime ve seq saniyesi her yazimda farkli (panel ikisini de saniyeyle karsilastirir) */
static int eylem_yaz(const char *ui)
{
	static long long son;
	char b[1200], g[400];
	long long t = (long long)simdi_unix();
	if (t <= son) t = son + 1;
	if (strlen(ui) >= 1000) return -1;	/* panel ey_bekleyen 1024 */
	int n = snprintf(b, sizeof b, "{\"seq\":%lld,\"ui\":%s}\n", t * 1000000LL, ui);
	if (n <= 0 || n >= (int)sizeof b) return -1;
	snprintf(g, sizeof g, "%s.merkez", eylem_yol);
	int fd = open(g, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd < 0) return -1;
	struct timespec ts[2] = { { (time_t)t, 0 }, { (time_t)t, 0 } };
	int ok = !tam_yaz(fd, b, (size_t)n) && !futimens(fd, ts);
	close(fd);
	if (!ok || rename(g, eylem_yol)) { unlink(g); return -1; }
	son = t;
	return 0;
}

static void ht_ui_ekle(struct jy *u, const struct htm *h)
{
	jy_bicim(u, "%s{\"hatirlat\":{\"z\":%lld,\"t\":\"%c\",\"m\":", u->n > 1 ? "," : "", h->z, h->t);
	jy_dizge(u, h->m);
	jy_ham(u, "}}");
}

static void api_hatirlat_liste(struct bag *c)
{
	if (izin(c, 1)) return;
	static struct htm l[30];
	int n = ht_oku(l, 30);
	struct jy y = { .azami = 32768 };
	jy_ham(&y, "{\"liste\":[");
	for (int i = 0; i < n; i++) {
		jy_bicim(&y, "%s{\"id\":\"%lld_%d\",\"zaman\":%lld,\"tur\":\"%c\",\"hedef\":\"cihaz\",\"metin\":", i ? "," : "",
			 l[i].z, i, l[i].z, l[i].t);
		jy_dizge(&y, l[i].m);
		jy_ham(&y, "}");
	}
	jy_ham(&y, "]}");
	yanit_json(c, 200, &y, NULL);
}

static void api_hatirlat_ekle(struct bag *c)
{
	if (izin(c, 1)) return;
	char m[256], h[16] = "";
	long long dk;
	govde_ayristir(c);
	if (g_str("metin", m, sizeof m) || !m[0] || strlen(m) > 200) { hata_yanit(c, 400, "Hatırlatma metni boş ya da çok uzun"); return; }
	if (g_int("dakika", &dk) || dk < 1 || dk > 7 * 24 * 60) { hata_yanit(c, 400, "Süre 1 dk ile 7 gün arasında olmalı"); return; }
	g_str("hedef", h, sizeof h);
	if (!strcmp(h, "pc")) { hata_yanit(c, 400, "Cihaz merkezinde hatırlatıcı panelde söylenir"); return; }
	for (char *p = m; *p; p++) if (*p == '|' || *p == '\n' || *p == '\r') *p = ' ';
	struct htm e = { (long long)simdi_unix() + dk * 60, 'h', "" };
	snprintf(e.m, sizeof e.m, "%s", m);
	struct jy u = { .azami = 1100 };
	jy_ham(&u, "[");
	ht_ui_ekle(&u, &e);
	jy_ham(&u, "]");
	int r = u.hata ? -1 : eylem_yaz(u.b);
	free(u.b);
	if (r) { hata_yanit(c, 500, "Panele iletilemedi"); return; }
	denetim(c->ip, "HATIRLATICI EKLENDI (%lld dk)", dk);
	struct jy y = { .azami = 1024 };
	jy_bicim(&y, "{\"ok\":true,\"hatirlatma\":{\"zaman\":%lld,\"hedef\":\"cihaz\",\"metin\":", e.z);
	jy_dizge(&y, e.m);
	jy_ham(&y, "},\"not\":\"Panel kurar (panelde asistan açık olmalı)\"}");
	yanit_json(c, 200, &y, NULL);
}

static void api_hatirlat_sil(struct bag *c)
{
	if (izin(c, 1)) return;
	char id[48];
	long long z;
	int i, o = 0;
	govde_ayristir(c);
	if (g_str("id", id, sizeof id) || sscanf(id, "%lld_%d%n", &z, &i, &o) != 2 || id[o]) { hata_yanit(c, 400, "Geçersiz hatırlatıcı"); return; }
	static struct htm l[30];
	int n = ht_oku(l, 30);
	if (i < 0 || i >= n || l[i].z != z) { hata_yanit(c, 404, "Hatırlatıcı bulunamadı (liste değişmiş olabilir)"); return; }
	struct jy u = { .azami = 1100 };
	if (i == n - 1) jy_ham(&u, "[{\"hatirlat_sil\":\"son\"}]");	/* panel dizisinin son ogesi */
	else {
		jy_ham(&u, "[{\"hatirlat_sil\":\"hepsi\"}");
		for (int k = 0; k < n; k++) if (k != i) ht_ui_ekle(&u, &l[k]);
		jy_ham(&u, "]");
	}
	int r = u.hata ? -2 : eylem_yaz(u.b);
	free(u.b);
	if (r) { hata_yanit(c, r == -2 || n > 3 ? 413 : 500, r == -2 || n > 3 ? "Çok fazla hatırlatıcı; bunu panelden silin" : "Panele iletilemedi"); return; }
	denetim(c->ip, "HATIRLATICI SILINDI");
	ok_yanit(c);
}

/* ------------------------------------------------------------------ yonlendirme */
static void yalniz_pc(struct bag *c)
{
	hata_yanit(c, 501, "Panel ekran görüntüsü ve panel komutları yalnız bilgisayardaki KÖLE Merkez uygulamasında var");	/* v18 */
}

#ifdef KOLE_TEST
static void api_test_saat(struct bag *c)
{
	long long ileri = 0;
	if (g_int("ileri", &ileri) || ileri < 0 || ileri > 30 * 86400) { hata_yanit(c, 400, NULL); return; }
	saat_kayma += (double)ileri;
	ok_yanit(c);
}
#endif

/* ------------------------------------------------------------------ v4: kablosuz guncelleme (OTA) */
#define OTA_AZAMI (40LL * 1024 * 1024)
#define OTA_PAY (64LL * 1024 * 1024)
#define OTA_SURE_SN 600.0
static struct {
	int var, yukleniyor, oturum;
	long oturum_surum;
	uint8_t jeton_ozet[32], sha[32];
	char ad[48], goreli[96];
	long long boyut;
	double bitis;
} OTA;
static struct { struct bag *c; char ad[48], goreli[96]; long long boyut; uint8_t sha[32]; } ota_bekleyen;

/* izinli ad -> ota_kok altindaki goreli yol. 0 uygun, -1 degil */
static int ota_ad_coz(const char *ad, char *goreli, size_t n)
{
	static const struct { const char *on, *son, *dizin; } K[] = {
		{ "kole-asistan-v", "", "" }, { "kole-gemini-v", "", "asistan/" }, { "kamera-kayit-v", "", "" },
		{ "kole-kopru-v", "", "" }, { "kole-cihaz-merkez-v", "", "" }, { "autorun_v", ".sh", "" },
		{ "kamera_kopru_sd_v", ".sh", "" }, { "wifi-ekran-v", "", "" },	/* v9 */
	};
	if (!ad || strlen(ad) >= 48) return -1;
	if (!strcmp(ad, "panel")) { snprintf(goreli, n, "panel"); return 0; }
	for (size_t i = 0; i < sizeof K / sizeof *K; i++) {
		size_t a = strlen(K[i].on), s = strlen(K[i].son), L = strlen(ad);
		if (L <= a + s || strncmp(ad, K[i].on, a) || strcmp(ad + L - s, K[i].son)) continue;
		size_t r = L - a - s;
		if (r < 1 || r > 3 || strspn(ad + a, "0123456789") != r || ad[a] == '0') return -1;
		snprintf(goreli, n, "%s%s", K[i].dizin, ad);
		return 0;
	}
	return -1;
}

static void ota_yol(char *o, size_t n, const char *goreli) { snprintf(o, n, "%s/%s", ota_kok, goreli); }
static void ota_part_yol(char *o, size_t n) { snprintf(o, n, "%s/.ota-%s.part", ota_kok, OTA.ad); }

static int ota_gecerli(void)
{
	if (OTA.var && simdi() > OTA.bitis && !OTA.yukleniyor) memset(&OTA, 0, sizeof OTA);
	return OTA.var;
}

static void ota_baslat_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (ota_bekleyen.c != c) { hata_yanit(c, 409, NULL); return; }
	ota_bekleyen.c = NULL;
	if (parola_red(c, ok)) return;
	if (!oturum_gecerli(c->oturum)) { hata_yanit(c, 401, NULL); return; }
	if (ota_gecerli()) { hata_yanit(c, 409, "Başka bir güncelleme sürüyor"); return; }
	uint8_t j[32];
	char hex[65], sh[65];
	rastgele(j, sizeof j);
	hex_yaz(j, 32, hex);
	memset(&OTA, 0, sizeof OTA);
	sha256_tek(j, 32, OTA.jeton_ozet);
	memset(j, 0, sizeof j);
	OTA.var = 1;
	OTA.oturum = c->oturum;
	OTA.oturum_surum = O[c->oturum].surum;
	OTA.bitis = simdi() + OTA_SURE_SN;
	OTA.boyut = ota_bekleyen.boyut;
	memcpy(OTA.sha, ota_bekleyen.sha, 32);
	snprintf(OTA.ad, sizeof OTA.ad, "%s", ota_bekleyen.ad);
	snprintf(OTA.goreli, sizeof OTA.goreli, "%s", ota_bekleyen.goreli);
	hex_yaz(OTA.sha, 32, sh);
	denetim(c->ip, "GUNCELLEME IZNI %s %lld bayt sha %.16s", OTA.ad, OTA.boyut, sh);
	struct jy y = { .azami = 512 };
	jy_bicim(&y, "{\"ok\":true,\"jeton\":\"%s\",\"gecerlilik_sn\":%d,\"hedef\":", hex, (int)OTA_SURE_SN);
	jy_dizge(&y, OTA.goreli);
	jy_ham(&y, "}");
	memset(hex, 0, sizeof hex);
	yanit_json(c, 200, &y, NULL);
}

static void api_ota_baslat(struct bag *c)
{
	if (izin(c, 0)) return;
	if (kalkan()) { hata_yanit(c, 423, "Gizlilik kalkanı açık"); return; }
	char ad[64], sh[80], goreli[96];
	long long boyut = 0;
	uint8_t sha[32];
	if (g_str("ad", ad, sizeof ad) || ota_ad_coz(ad, goreli, sizeof goreli)) { hata_yanit(c, 400, "İzin verilmeyen dosya adı"); return; }
	if (g_int("boyut", &boyut) || boyut < 1 || boyut > OTA_AZAMI) { hata_yanit(c, 400, "Boyut 1 bayt..40 MB olmalı"); return; }
	if (g_str("sha256", sh, sizeof sh) || strlen(sh) != 64 || hex_coz(sh, sha, 32)) { hata_yanit(c, 400, "sha256 (64 hex) gerekli"); return; }
	if (sd_yazilabilir() != 2) { hata_yanit(c, 503, "SD kart yazılabilir değil"); return; }
	struct statvfs v;
	if (statvfs(ota_kok, &v) || (long long)v.f_bavail * (long long)v.f_frsize < boyut + OTA_PAY) { hata_yanit(c, 507, "SD kartta yer yok"); return; }
	if (ota_gecerli()) { hata_yanit(c, 409, "Başka bir güncelleme sürüyor"); return; }
	if (ota_bekleyen.c && ota_bekleyen.c != c && ota_bekleyen.c->fd >= 0 && ota_bekleyen.c->tur == B_PBKDF2) { hata_yanit(c, 409, NULL); return; }
	ota_bekleyen.c = c;
	snprintf(ota_bekleyen.ad, sizeof ota_bekleyen.ad, "%s", ad);
	snprintf(ota_bekleyen.goreli, sizeof ota_bekleyen.goreli, "%s", goreli);
	ota_bekleyen.boyut = boyut;
	memcpy(ota_bekleyen.sha, sha, 32);
	if (yeniden_parola(c, "GUNCELLEME YUKLEME", ota_baslat_devam)) ota_bekleyen.c = NULL;
}

/* v9: parolali YENIDEN BASLATMA (cihaz basinda olmadan OTA guncellemeleri etkinlessin). Iki kipte de parola
 * (yeniden_parola, OTA gibi); guncelleme surerken 409; yanittan 3 sn sonra sync + reboot(RB_AUTOBOOT). FEL yok. */
static double yeniden_baslat_t;

static void yeniden_baslat_devam(struct bag *c, int ok, const uint8_t *h2)
{
	(void)h2;
	if (parola_red(c, ok)) return;
	if (!oturum_gecerli(c->oturum)) { hata_yanit(c, 401, NULL); return; }
	if (ota_gecerli() || OTA.yukleniyor) { hata_yanit(c, 409, "Güncelleme sürüyor; bitince yeniden deneyin"); return; }
	yeniden_baslat_t = simdi() + 3;
	denetim(c->ip, "YENIDEN BASLATMA istendi (3 sn sonra)");
	struct jy y = { .azami = 128 };
	jy_ham(&y, "{\"ok\":true,\"sn\":3}");
	yanit_json(c, 200, &y, NULL);
}

static void api_yeniden_baslat(struct bag *c)
{
	if (izin(c, 0)) return;
	if (ota_gecerli() || OTA.yukleniyor) { hata_yanit(c, 409, "Güncelleme sürüyor; bitince yeniden deneyin"); return; }
	yeniden_parola(c, "YENIDEN BASLATMA", yeniden_baslat_devam);
}

static void yeniden_baslat_tick(void)
{
	if (!yeniden_baslat_t || simdi() < yeniden_baslat_t) return;
	yeniden_baslat_t = 0;
	int fd = open(yb_isaret_yol, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd >= 0) close(fd);
	unlink(wifi_isaret_yol);	/* 387: bilerek yeniden baslatma "Wi-Fi coktu" sayilmasin (fel_gir_v2 gibi) */
	sync();
#ifndef KOLE_TEST
	/* v13 (denetim v3 A19): MCU kalbine hizali betik (son B2'den hemen sonra reboot -f; .wifi-deneme siler, sync).
	 * Betik calismaz ya da geri donerse eski yol. */
	printf("YENIDEN BASLATMA: %s\n", YB_BETIK);
	fflush(stdout);
	pid_t p = fork();
	if (!p) {
		execl("/bin/sh", "sh", YB_BETIK, (char *)NULL);
		_exit(127);
	}
	if (p > 0) waitpid(p, NULL, 0);	/* betik reboot -f yaparsa buraya donulmez */
	printf("YENIDEN BASLATMA: betik yeniden baslatmadi, reboot\n");
	fflush(stdout);
	reboot(RB_AUTOBOOT);
	perror("reboot");
#endif
}

static void ota_iptal(struct bag *c, const char *neden)
{
	if (c->tur != B_OTA) return;
	char p[400];
	if (c->ota_fd >= 0) close(c->ota_fd);
	c->ota_fd = -1;
	ota_part_yol(p, sizeof p);
	unlink(p);
	denetim(c->ip, "GUNCELLEME YARIM KALDI %s (%s, %llu bayt eksik)", OTA.ad, neden, c->ota_kalan);
	memset(&OTA, 0, sizeof OTA);
	c->tur = B_YANIT;
}

static void ota_bitir(struct bag *c)
{
	uint8_t h[32];
	char p[400], hedef[400], yedek[460], sh[65];
	sha256_bitir(&c->ota_sha, h);
	int hata = fsync(c->ota_fd);
	hata |= close(c->ota_fd);
	c->ota_fd = -1;
	ota_part_yol(p, sizeof p);
	c->tur = B_YANIT;
	c->keepalive = 0;
	hex_yaz(h, 32, sh);
	if (hata) {
		unlink(p);
		denetim(c->ip, "GUNCELLEME HATASI %s (SD yazma)", OTA.ad);
		memset(&OTA, 0, sizeof OTA);
		hata_yanit(c, 507, "SD karta yazılamadı");
		return;
	}
	if (!sabit_esit(h, OTA.sha, 32)) {
		unlink(p);
		denetim(c->ip, "GUNCELLEME REDDEDILDI %s (SHA uyusmadi %.16s)", OTA.ad, sh);
		memset(&OTA, 0, sizeof OTA);
		hata_yanit(c, 422, "SHA-256 uyuşmadı; dosya yazılmadı");
		return;
	}
	ota_yol(hedef, sizeof hedef, OTA.goreli);
	yedek[0] = 0;
	if (dosya_var(hedef)) {
		time_t t = (time_t)simdi_unix();
		struct tm tm;
		char ts[32];
		localtime_r(&t, &tm);
		strftime(ts, sizeof ts, "%Y%m%d-%H%M%S", &tm);
		snprintf(yedek, sizeof yedek, "%s.ota-yedek-%s", hedef, ts);
		if (rename(hedef, yedek)) {
			unlink(p);
			denetim(c->ip, "GUNCELLEME HATASI %s (yedek alinamadi)", OTA.ad);
			memset(&OTA, 0, sizeof OTA);
			hata_yanit(c, 500, "Eski dosya yedeklenemedi; değiştirilmedi");
			return;
		}
	}
	if (rename(p, hedef)) {
		if (yedek[0]) rename(yedek, hedef);
		unlink(p);
		denetim(c->ip, "GUNCELLEME HATASI %s (yerine konamadi)", OTA.ad);
		memset(&OTA, 0, sizeof OTA);
		hata_yanit(c, 500, "Dosya yerine konamadı");
		return;
	}
	chmod(hedef, 0755);
	sync();
	denetim(c->ip, "GUNCELLEME YUKLENDI %s %lld bayt sha %.16s%s", OTA.goreli, OTA.boyut, sh, yedek[0] ? " (eskisi yedeklendi)" : "");
	struct jy y = { .azami = 1024 };
	jy_bicim(&y, "{\"ok\":true,\"boyut\":%lld,\"sha256\":\"%s\",\"hedef\":", OTA.boyut, sh);
	jy_dizge(&y, OTA.goreli);
	jy_ham(&y, ",\"yedek\":");
	jy_dizge(&y, yedek[0] ? strrchr(yedek, '/') + 1 : "");
	jy_ham(&y, "}");
	memset(&OTA, 0, sizeof OTA);
	yanit_json(c, 200, &y, NULL);
}

/* gelen[] basindaki govde baytlarini dosyaya yaz */
static void ota_veri(struct bag *c)
{
	size_t n = c->gn;
	if ((unsigned long long)n > c->ota_kalan) n = (size_t)c->ota_kalan;
	size_t yaz = 0;
	while (yaz < n) {
		ssize_t w = write(c->ota_fd, c->gelen + yaz, n - yaz);
		if (w < 0 && errno == EINTR) continue;
		if (w <= 0) {
			ota_iptal(c, "SD yazma hatasi");
			c->keepalive = 0;
			hata_yanit(c, 507, "SD karta yazılamadı");
			c->kapat_sonra = 1;
			c->gn = 0;
			return;
		}
		yaz += (size_t)w;
	}
	sha256_ekle(&c->ota_sha, c->gelen, n);
	c->ota_kalan -= n;
	c->gn = 0;	/* govdeden sonra gelen bayt yok sayilir (baglanti kapanacak) */
	if (!c->ota_kalan) ota_bitir(c);
}

static void ota_yukle_basla(struct bag *c)
{
	c->keepalive = 0;
	if (izin(c, 0)) { c->kapat_sonra = 1; return; }
	char j[80];
	uint8_t jb[32], oz[32];
	if (!ota_gecerli() || OTA.yukleniyor || sorgu_al(c->r.sorgu, "j", j, sizeof j) != 1 || strlen(j) != 64 || hex_coz(j, jb, 32)) {
		hata_yanit(c, 403, "Geçersiz ya da süresi dolmuş güncelleme jetonu");
		c->kapat_sonra = 1;
		return;
	}
	sha256_tek(jb, 32, oz);
	if (!sabit_esit(oz, OTA.jeton_ozet, 32) || c->oturum != OTA.oturum || !oturum_gecerli(c->oturum) ||
	    O[c->oturum].surum != OTA.oturum_surum) {
		hata_yanit(c, 403, "Geçersiz ya da süresi dolmuş güncelleme jetonu");
		c->kapat_sonra = 1;
		return;
	}
	if (c->r.icerik_uzunluk != OTA.boyut) {
		hata_yanit(c, 400, "Content-Length bildirilen boyutla aynı olmalı");
		c->kapat_sonra = 1;
		return;
	}
	char p[400];
	ota_part_yol(p, sizeof p);
	if (!strncmp(OTA.goreli, "asistan/", 8)) {
		char d[400];
		snprintf(d, sizeof d, "%s/asistan", ota_kok);
		mkdir(d, 0755);
	}
	c->ota_fd = open(p, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (c->ota_fd < 0) { hata_yanit(c, 507, "SD karta yazılamadı"); c->kapat_sonra = 1; return; }
	OTA.yukleniyor = 1;
	denetim(c->ip, "GUNCELLEME AKTARIMI BASLADI %s", OTA.ad);
	c->tur = B_OTA;
	c->ota_kalan = (unsigned long long)OTA.boyut;
	sha256_bas(&c->ota_sha);
	static const char devam[] = "HTTP/1.1 100 Continue\r\n\r\n";	/* istemci govdeyi kabulden sonra gonderir (Expect) */
	cikis_ekle(c, devam, sizeof devam - 1);
	size_t b = c->r.baslik_n < c->gn ? c->r.baslik_n : c->gn;	/* baslik tuketilir; kalan govdedir */
	memmove(c->gelen, c->gelen + b, c->gn - b);
	c->gn -= b;
	c->r.baslik_n = 0;	/* yanit basliklari icin c->r korunur */
	if (c->gn) ota_veri(c);
}

static void istek_isle(struct bag *c)
{
	struct http_istek *r = &c->r;
	M.istek_sayi++;
	c->head = !strcmp(r->yontem, "HEAD");
	c->keepalive = r->keepalive;
	/* istemci IP'si: yalniz geri donus (Tailscale serve) baglantisinda X-Forwarded-For'un SON ogesi */
	if (c->vekil && r->xff[0]) {	/* v17 (A20) */
		const char *s = strrchr(r->xff, ',');
		s = s ? s + 1 : r->xff;
		while (*s == ' ') s++;
		if (strlen(s) < sizeof c->ip && strspn(s, "0123456789abcdefABCDEF.:") == strlen(s) && *s) snprintf(c->ip, sizeof c->ip, "%s", s);
	}
	c->https = c->vekil && !strcasecmp(r->xfproto, "https");
	if ((r->surum == 1 && !r->host[0]) || (r->host[0] && !host_uygun(r->host))) {
		hata_yanit(c, 421, "Bu ad ile erişim kabul edilmiyor");
		c->keepalive = 0;
		return;
	}
	if (!strcmp(r->yol, "/saglik")) {	/* gozetici icin, yalniz yerel; bilgi icermez */
		if (!c->peer_lo) { hata_yanit(c, 404, NULL); return; }
		yanit(c, 200, "text/plain", "ok\n", 3, NULL);
		return;
	}
	if (!strcmp(r->yol, "/canli") || !strcmp(r->yol, "/canli.js") || !strcmp(r->yol, "/canli.css")) { canli_sayfa(c); return; }
	if (strncmp(r->yol, "/api/", 5)) { statik(c); return; }
	c->oturum = oturum_bul(r->cookie);
	int post = !strcmp(r->yontem, "POST"), get = !strcmp(r->yontem, "GET");
	if (post) govde_ayristir(c);
	else G.t = 0;
	const char *y = r->yol;
#define YOL(a) (!strcmp(y, a))
	if (get && YOL("/api/oturum")) {
		struct jy j = { .azami = 512 };
		jy_bicim(&j, "{\"ok\":%s,\"surum\":\"" SURUM "\",\"merkez\":\"cihaz\",\"kurulum\":%s", c->oturum >= 0 ? "true" : "false",
			 M.ayar_var ? "false" : "true");
		if (c->oturum >= 0) jy_bicim(&j, ",\"guvenlik\":{\"mod\":\"%s\",\"bosta_saat\":%d}", M.siki ? "siki" : "tek_giris", bosta_saat());
		jy_ham(&j, "}");
		yanit_json(c, 200, &j, NULL);
	} else if (post && YOL("/api/giris")) api_giris(c);
	else if (post && YOL("/api/kurulum")) api_kurulum(c);
	else if (post && YOL("/api/cikis")) api_cikis(c);
	else if (get && YOL("/api/ping")) {
		if (izin(c, 0)) return;
		struct jy j = { .azami = 128 };
		jy_bicim(&j, "{\"t\":%.3f,\"surum\":\"" SURUM "\"}", simdi_unix());
		yanit_json(c, 200, &j, NULL);
	} else if (post && YOL("/api/parola")) api_parola(c);
	else if (post && YOL("/api/jeton")) api_jeton(c);
	else if (post && YOL("/api/jeton/iptal")) api_jeton_iptal(c);
	else if (post && YOL("/api/ota/baslat")) api_ota_baslat(c);	/* v4 */
	else if (post && YOL("/api/yeniden-baslat")) api_yeniden_baslat(c);	/* v9 */
	else if (post && YOL("/api/ota/yukle")) ota_yukle_basla(c);
	else if (get && YOL("/api/guvenlik")) api_guvenlik_get(c);
	else if (post && YOL("/api/guvenlik")) api_guvenlik_post(c);
	else if (get && YOL("/api/oturumlar")) api_oturumlar(c);
	else if (post && YOL("/api/oturumlar/kapat")) { if (!izin(c, 0)) yeniden_parola(c, "OTURUM KAPATMA", oturum_kapat_devam); }
	else if (post && YOL("/api/oturumlar/hepsi")) { if (!izin(c, 0)) yeniden_parola(c, "TUM OTURUMLARI KAPATMA", oturum_hepsi_devam); }
	else if (get && YOL("/api/durum")) api_durum(c);
	else if (get && YOL("/api/cihaz")) api_cihaz(c);
	else if (get && YOL("/api/uzak")) api_uzak(c);
	else if (post && YOL("/api/kamera/ac")) api_kamera_ac(c);
	else if (post && YOL("/api/kamera/kapat")) api_kamera_kapat(c);
	else if (get && YOL("/api/kamera/durum")) api_kamera_durum(c);
	else if (get && YOL("/api/kamera/akis")) api_kamera_akis(c);
	else if (get && YOL("/api/kamera/kare")) api_kamera_kare(c);	/* v8 */
	else if (post && YOL("/api/anlik")) api_anlik(c);
	else if (get && YOL("/api/kayit/liste")) api_kayit_liste(c);
	else if (get && YOL("/api/kayit/durum")) api_kayit_durum(c);
	else if (post && YOL("/api/kayit/mod")) api_kayit_mod(c);
	else if (get && YOL("/api/kayit/izin")) api_kayit_izin_get(c);
	else if (post && YOL("/api/kayit/izin")) api_kayit_izin_post(c);
	else if (post && YOL("/api/kayit/oynat")) api_kayit_oynat(c);
	else if (post && YOL("/api/kayit/ac")) api_kayit_ac(c);
	else if (post && YOL("/api/kayit/kapat")) api_kayit_kapat(c);
	else if (get && YOL("/api/kayit/akis")) api_kayit_akis(c);
	else if (get && YOL("/api/kayit/kucuk")) api_kayit_kucuk(c);
	else if (get && YOL("/api/kayit/indir")) api_kayit_indir(c);
	else if (get && YOL("/api/zaman/liste")) api_zaman_liste(c);	/* v10 */
	else if (get && YOL("/api/zaman/kare")) api_zaman_kare(c);
	else if (get && YOL("/api/zaman/olay")) api_zaman_olay(c);	/* v11 */
	else if (post && YOL("/api/zaman/mod")) api_zaman_mod(c);
	else if (get && YOL("/api/foto/liste")) api_foto_liste(c);
	else if (get && YOL("/api/foto/al")) api_foto_al(c);
	else if (get && YOL("/api/olaylar")) api_olaylar(c, 0);
	else if (get && YOL("/api/ozet")) api_olaylar(c, 1);
	else if (get && YOL("/api/plan")) api_plan_get(c);
	else if (post && YOL("/api/plan")) api_plan_post(c);
	else if (post && YOL("/api/denetim/ac")) { if (!izin(c, 0)) hassas(c, "DENETIM GORUNTULEME", denetim_ac_devam); }
	else if (get && YOL("/api/denetim")) api_denetim(c);
	else if (get && YOL("/api/ses/durum")) api_ses_durum(c);
	else if (post && YOL("/api/ses/izin")) api_ses_izin(c);
	else if (post && YOL("/api/ses/kilitle")) api_ses_kilitle(c);
	else if (get && YOL("/api/ses/dinle")) api_ses_dinle(c);
	else if (get && YOL("/api/ses/konus")) api_ses_konus(c);
	else if (get && YOL("/api/rtsp")) api_rtsp_get(c);
	else if (post && YOL("/api/rtsp")) api_rtsp_post(c);
	else if (post && YOL("/api/rtsp/parola")) api_rtsp_parola(c);
	else if (get && YOL("/api/asistan")) {
		if (izin(c, 0)) return;
		static const char a[] = "{\"calisiyor\":false,\"durum\":\"Sesli asistan panelde Hey Köle ile çalışır\",\"cikis\":\"cihaz\",\"gecmis\":[]}";
		yanit(c, 200, "application/json", a, sizeof a - 1, NULL);
	} else if (get && YOL("/api/hatirlat")) api_hatirlat_liste(c);	/* v18 */
	else if (post && YOL("/api/hatirlat")) api_hatirlat_ekle(c);
	else if (post && YOL("/api/hatirlat/sil")) api_hatirlat_sil(c);
	else if (post && YOL("/api/sor")) api_sor(c);
	else if (post && YOL("/api/soyle")) api_soyle(c);
	else if (YOL("/api/ekran.jpg") || YOL("/api/ui") || YOL("/api/soyle") || YOL("/api/sor") || YOL("/api/asistan") ||
		   YOL("/api/hatirlat") || YOL("/api/hatirlat/sil")) {
		if (izin(c, 1)) return;
		yalniz_pc(c);
	}
#ifdef KOLE_TEST
	else if (post && YOL("/api/_test/saat")) api_test_saat(c);
#endif
	else hata_yanit(c, 404, NULL);
#undef YOL
}

/* ------------------------------------------------------------------ baglanti yasam dongusu */
static void bag_kapat(struct bag *c, const char *neden)
{
	if (c->fd < 0) return;
	int i = (int)(c - B);
	int canliydi = c->tur == B_CANLI;
	if (c->tur == B_OTA) ota_iptal(c, neden);	/* v4: .part silinir, jeton yanar */
	if (c->tur == B_WS_DINLE) {
		denetim(c->ip, "SES DINLEME BITTI %.0f sn (%s; %llu sn ses aktarildi%s)", simdi() - c->t0, c->neden[0] ? c->neden : neden,
			c->ses_bayt / 32000, c->ses_atilan ? ", yavas baglantida ses atildi" : "");
	}
	if (c->tur == B_WS_KONUS) {
		if (M.kon_bag == i) {
			if (M.kon_fd >= 0 && M.kon_hazir) { uint8_t z[2] = { 0, 0 }; send(M.kon_fd, z, 2, MSG_NOSIGNAL | MSG_DONTWAIT); }
			kon_dur();
		}
		denetim(c->ip, "KONUSMA BITTI %.1f sn (%s)%s", c->ornek / 16000.0, c->neden[0] ? c->neden : neden,
			c->ses_atilan ? " [fazla veri atildi]" : "");
	}
	if (c->tur == B_KAYIT || c->oyn_k) oynatma_birak(c);
	if (M.yan_bag == i) M.yan_bag = -1;
	if (M.pb_bag == i) M.pb_bag = -1;
	kare_birak(c);
	if (c->dosya_fd >= 0) close(c->dosya_fd);
	c->dosya_fd = -1;
	/* okunmamis gelen veri varken close() RST gonderir ve istemci son yaniti (431/413) kaybedebilir: once boşalt */
	shutdown(c->fd, SHUT_WR);
	for (int k = 0; k < 16; k++) {
		uint8_t at[16384];
		if (recv(c->fd, at, sizeof at, MSG_DONTWAIT) <= 0) break;
	}
	close(c->fd);
	c->fd = -1;
	free(c->cb_);
	c->cb_ = NULL;
	c->ccap = c->cbas = c->cson = 0;
	free(c->wsb);
	c->wsb = NULL;
	memset(c->gelen, 0, c->gn);
	c->gn = 0;
	c->tur = B_BOS;
	if (canliydi && !izleyici_sayisi()) M.kamera_bos_t = simdi();
}

static void bag_sifirla_istek(struct bag *c)
{
	size_t tuketilen = c->r.baslik_n + c->govde_n;
	if (tuketilen > c->gn) tuketilen = c->gn;
	memmove(c->gelen, c->gelen + tuketilen, c->gn - tuketilen);
	memset(c->gelen + c->gn - tuketilen, 0, tuketilen);
	c->gn -= tuketilen;
	memset(&c->r, 0, sizeof c->r);
	c->govde_n = 0;
	c->tur = B_ISTEK;
	c->kapat_sonra = 0;
	c->son_t = simdi();
	c->oturum = -1;
	c->head = 0;
	if (c->peer_lo) snprintf(c->ip, sizeof c->ip, "127.0.0.1");	/* XFF yalniz istek basina */
	cikis_serbest(c);
}

/* gelen verideki tam istegi isle */
static void istek_dene(struct bag *c)
{
	if (c->tur != B_ISTEK) return;
	long r = http_ayristir((const char *)c->gelen, c->gn, &c->r);
	if (!r) return;
	if (r < 0) {
		c->keepalive = 0;
		int kod = r == -2 ? 431 : r == -3 ? 501 : 400;
		memset(&c->r, 0, sizeof c->r);
		hata_yanit(c, kod, kod == 501 ? "Desteklenmeyen istek" : NULL);
		c->kapat_sonra = 1;
		return;
	}
	long long L = c->r.icerik_uzunluk < 0 ? 0 : c->r.icerik_uzunluk;
	if (!strcmp(c->r.yontem, "POST") && !strcmp(c->r.yol, "/api/ota/yukle")) {	/* v4: govde bellege alinmaz, SD'ye akar */
		c->govde_n = 0;
		istek_isle(c);
		return;
	}
	if (L > GOVDE_AZAMI) {
		c->keepalive = 0;
		hata_yanit(c, 413, NULL);
		c->kapat_sonra = 1;
		return;
	}
	if (c->gn < (size_t)r + (size_t)L) return;	/* govde bekleniyor */
	c->govde_n = (size_t)L;
	istek_isle(c);
}

/* 0 devam, -1 kapat */
static int bag_yaz(struct bag *c)
{
	if (c->gecikme_t > simdi()) return 0;
	for (int dongu = 0; dongu < 64; dongu++) {
		if (cikis_bekleyen(c)) {
			ssize_t w = send(c->fd, c->cb_ + c->cbas, cikis_bekleyen(c), MSG_NOSIGNAL | MSG_DONTWAIT);
			if (w < 0 && (errno == EAGAIN || errno == EINTR)) return 0;
			if (w <= 0) return -1;
			c->cbas += (size_t)w;
			c->cikis_t = mono();	/* v13 */
			if (c->cbas == c->cson) c->cbas = c->cson = 0;
			continue;
		}
		if (c->kref) {
			ssize_t w = send(c->fd, c->kref->b + c->kofs, c->kref->n - c->kofs, MSG_NOSIGNAL | MSG_DONTWAIT);
			if (w < 0 && (errno == EAGAIN || errno == EINTR)) return 0;
			if (w <= 0) return -1;
			c->kofs += (size_t)w;
			c->cikis_t = mono();	/* v13 */
			if (c->kofs == c->kref->n) {
				kare_birak(c);
				if (c->tur == B_CANLI && cikis_ekle(c, "\r\n", 2)) return -1;	/* v8: tek kare yanitinda ayrac yok */
			}
			continue;
		}
		if (c->dosya_fd >= 0 && c->dosya_kalan) {
			if (c->hiz > 0 && c->giden > c->hiz * (simdi() - c->hiz_t0)) return 0;
			if (c->tur == B_INDIR && (!kayit_izin_aktif(c->oturum) || kalkan())) return -1;
			uint8_t t[16384];
			size_t k = c->dosya_kalan < sizeof t ? (size_t)c->dosya_kalan : sizeof t;
			ssize_t r = read(c->dosya_fd, t, k);
			if (r <= 0) return -1;
			c->dosya_kalan -= (unsigned long long)r;
			c->giden += (unsigned long long)r;
			if (cikis_ekle(c, t, (size_t)r)) return -1;
			if (!c->dosya_kalan) { close(c->dosya_fd); c->dosya_fd = -1; }
			continue;
		}
		break;
	}
	if (cikis_bekleyen(c) || c->kref || (c->dosya_fd >= 0 && c->dosya_kalan)) return 0;
	/* her sey gonderildi */
	if (c->kapat_sonra) return -1;
	switch (c->tur) {
	case B_YANIT:
		if (!c->keepalive) return -1;
		bag_sifirla_istek(c);
		istek_dene(c);
		return 0;
	case B_INDIR:
		return -1;
	case B_CANLI:
		kare_bolum_gonder(c);
		return 0;
	case B_KAYIT:
		oynatma_adim(c);
		return 0;
	default:
		return 0;
	}
}

static void ws_isle(struct bag *c)
{
	for (;;) {
		struct ws_cerceve f;
		long r = ws_coz(c->gelen, c->gn, WS_AZAMI, &f);
		if (!r) return;
		if (r < 0) {
			uint8_t kp[2] = { 0x03, (uint8_t)(r == -2 ? 0xf1 : 0xea) };	/* 1009 / 1002 */
			ws_gonder(c, 8, kp, 2);
			snprintf(c->neden, sizeof c->neden, "%s", r == -2 ? "ileti çok büyük" : "protokol hatası");
			c->kapat_sonra = 1;
			c->gn = 0;
			return;
		}
		uint8_t veri[WS_AZAMI];
		size_t vn = f.n;
		memcpy(veri, f.veri, vn);
		int fin = f.fin, op = f.op;
		memmove(c->gelen, c->gelen + r, c->gn - (size_t)r);
		c->gn -= (size_t)r;
		if (op >= 8) {
			if (op == 8) {
				uint8_t kp[2] = { 0x03, 0xe8 };
				ws_gonder(c, 8, kp, 2);
				if (!c->neden[0]) snprintf(c->neden, sizeof c->neden, "istemci kapattı");
				c->kapat_sonra = 1;
				return;
			}
			if (op == 9) ws_gonder(c, 10, veri, vn);
			continue;
		}
		/* parcali ileti birlestirme */
		if ((op == 0) == (c->ws_op == 0)) {
			uint8_t kp[2] = { 0x03, 0xea };
			ws_gonder(c, 8, kp, 2);
			c->kapat_sonra = 1;
			return;
		}
		if (op) c->ws_op = op;
		if (!c->wsb && !(c->wsb = malloc(WS_AZAMI))) { c->kapat_sonra = 1; return; }
		if (c->wsn + vn > WS_AZAMI) { uint8_t kp[2] = { 0x03, 0xf1 }; ws_gonder(c, 8, kp, 2); c->kapat_sonra = 1; return; }
		memcpy(c->wsb + c->wsn, veri, vn);
		c->wsn += vn;
		if (!fin) continue;
		int tur = c->ws_op;
		size_t n = c->wsn;
		c->ws_op = 0;
		c->wsn = 0;
		if (c->tur == B_WS_DINLE) {
			if (tur == 1 && memmem(c->wsb, n, "\"bitir\"", 7)) {
				snprintf(c->neden, sizeof c->neden, "kullanıcı durdurdu");
				uint8_t kp[2] = { 0x03, 0xe8 };
				ws_gonder(c, 8, kp, 2);
				c->kapat_sonra = 1;
				return;
			}
			continue;
		}
		/* B_WS_KONUS */
		if (c->kon_bitiyor) continue;
		double t = simdi();
		if (tur == 1) {
			if (memmem(c->wsb, n, "\"bitti\"", 7)) {
				c->kon_bitiyor = 1;
				c->ws_son_veri = t;
				if (M.kon_fd >= 0 && M.kon_hazir) {
					if (M.kon_cn + 2 <= sizeof M.kon_cikis) { M.kon_cikis[M.kon_cn++] = 0; M.kon_cikis[M.kon_cn++] = 0; }
					kon_yaz();
				} else konusma_bitti(c, "bitti", "");
			}
			continue;
		}
		if (!M.kon_hazir) { c->ses_atilan += n; continue; }
		if (!n || (n & 1) || n > 8192) {
			snprintf(c->neden, sizeof c->neden, "geçersiz ses çerçevesi");
			c->kon_bitiyor = 1;
			if (M.kon_cn + 2 <= sizeof M.kon_cikis) { M.kon_cikis[M.kon_cn++] = 0; M.kon_cikis[M.kon_cn++] = 0; }
			kon_yaz();
			continue;
		}
		c->ws_son_veri = t;
		if ((c->ornek + n / 2) / 16000.0 > (t - c->t0) * 1.5 + 1.0) { c->ses_atilan += n; continue; }	/* gercek zamandan hizli */
		if (c->ornek + n / 2 > 16000UL * KONUSMA_AZAMI_SN) {
			snprintf(c->neden, sizeof c->neden, "%d sn sınırı", KONUSMA_AZAMI_SN);
			c->kon_bitiyor = 1;
			if (M.kon_cn + 2 <= sizeof M.kon_cikis) { M.kon_cikis[M.kon_cn++] = 0; M.kon_cikis[M.kon_cn++] = 0; }
			kon_yaz();
			continue;
		}
		if (M.kon_cn + 2 + n > sizeof M.kon_cikis) { c->ses_atilan += n; continue; }	/* cihaz yetisemiyor */
		M.kon_cikis[M.kon_cn++] = (uint8_t)n;
		M.kon_cikis[M.kon_cn++] = (uint8_t)(n >> 8);
		memcpy(M.kon_cikis + M.kon_cn, c->wsb, n);
		M.kon_cn += n;
		c->ornek += n / 2;
		c->ses_bayt += n;
		kon_yaz();
	}
}

static void bag_oku(struct bag *c)
{
	if (c->gn >= GELEN_AZAMI) {
		if (c->tur == B_ISTEK) { c->keepalive = 0; hata_yanit(c, 431, NULL); c->kapat_sonra = 1; }
		return;
	}
	ssize_t r = recv(c->fd, c->gelen + c->gn, GELEN_AZAMI - c->gn, MSG_DONTWAIT);
	if (r < 0 && (errno == EAGAIN || errno == EINTR)) return;
	if (r <= 0) { if (!c->neden[0]) snprintf(c->neden, sizeof c->neden, "istemci kapattı"); bag_kapat(c, "istemci kapatti"); return; }
	c->gn += (size_t)r;
	c->son_t = simdi();
	switch (c->tur) {
	case B_ISTEK: istek_dene(c); break;
	case B_WS_DINLE:
	case B_WS_KONUS: ws_isle(c); break;
	case B_OTA: ota_veri(c); break;	/* v4 */
	case B_YANIT: case B_PBKDF2: case B_BEKLE: break;	/* ardisik istek: yanit bitince islenir */
	default: c->gn = 0; break;	/* akislarda istemci verisi yok sayilir */
	}
}

static void kabul(int lfd)
{
	struct sockaddr_storage a;
	socklen_t al = sizeof a;
	int fd = accept4(lfd, (struct sockaddr *)&a, &al, SOCK_CLOEXEC | SOCK_NONBLOCK);
	if (fd < 0) return;
	struct bag *c = NULL;
	for (int i = 0; i < AZAMI_BAG; i++) if (B[i].fd < 0) { c = &B[i]; break; }
	if (!c) {
		static const char d[] = "HTTP/1.1 503 Service Unavailable\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
		send(fd, d, sizeof d - 1, MSG_NOSIGNAL | MSG_DONTWAIT);
		close(fd);
		return;
	}
	memset(c, 0, offsetof(struct bag, gelen));
	memset((char *)c + offsetof(struct bag, r), 0, sizeof *c - offsetof(struct bag, r));
	c->fd = fd;
	c->dosya_fd = -1;
	c->oyn_fd = -1;
	c->oyn_oturum = -1;
	c->oturum = -1;
	c->tur = B_ISTEK;
	c->son_t = simdi();
	if (a.ss_family == AF_INET) {
		struct sockaddr_in *s = (struct sockaddr_in *)&a;
		inet_ntop(AF_INET, &s->sin_addr, c->ip, sizeof c->ip);
		c->peer_lo = (ntohl(s->sin_addr.s_addr) >> 24) == 127;
	} else if (a.ss_family == AF_INET6) {
		struct sockaddr_in6 *s = (struct sockaddr_in6 *)&a;
		inet_ntop(AF_INET6, &s->sin6_addr, c->ip, sizeof c->ip);
		c->peer_lo = IN6_IS_ADDR_LOOPBACK(&s->sin6_addr);
	} else snprintf(c->ip, sizeof c->ip, "-");
	if (c->peer_lo) snprintf(c->ip, sizeof c->ip, "127.0.0.1");
	struct sockaddr_in yer;	/* v17 (A20): yerel adres 127.0.0.2 ise Tailscale serve vekili */
	socklen_t yl = sizeof yer;
	c->vekil = c->peer_lo && !getsockname(fd, (struct sockaddr *)&yer, &yl) && yer.sin_family == AF_INET &&
		   ntohl(yer.sin_addr.s_addr) == 0x7f000002u;
	int bir = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &bir, sizeof bir);
}

/* ------------------------------------------------------------------ periyodik isler */
static void merkez_durum_yaz(void)
{
	char b[512], tmp[300];
	int n = snprintf(b, sizeof b, "{\"surum\":\"" SURUM_TAM "\",\"pid\":%d,\"t\":%.0f,\"rss_kb\":%ld,\"baglanti\":%d,\"izleyici\":%d,"
			 "\"dinleyici\":%d,\"konusan\":%s,\"kamera\":%s,\"kurulum\":%s,\"istek\":%lu}\n",
			 getpid(), simdi_unix(), rss_kb(), ({ int k = 0; for (int i = 0; i < AZAMI_BAG; i++) k += B[i].fd >= 0; k; }),
			 izleyici_sayisi(), dinleyici_sayisi(), M.kon_bag >= 0 ? "true" : "false", M.kamera_acik ? "true" : "false",
			 M.ayar_var ? "false" : "true", M.istek_sayi);
	snprintf(tmp, sizeof tmp, "%s.tmp", merkez_durum_yol);
	int fd = open(tmp, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd < 0) return;
	int ok = !tam_yaz(fd, b, (size_t)n);
	close(fd);
	if (ok) rename(tmp, merkez_durum_yol); else unlink(tmp);
}

/* v13 (denetim v3 A4): cikisi bekleyen ama 60 sn (kapat_sonra ise 10 sn) gonderim ilerlemesi olmayan baglanti kapanir
 * (okumayan dinleyici mikrofonu acik tutmasin). Bekleyen yoksa sayac tazelenir. t: mono() (test saati kaymaz) */
static int cikis_takildi(struct bag *c, double t)
{
	int bekleyen = cikis_bekleyen(c) || c->kref || (c->dosya_fd >= 0 && c->dosya_kalan);
	if (!bekleyen || c->cikis_t <= 0) { c->cikis_t = t; return 0; }
	return t - c->cikis_t > (c->kapat_sonra ? 10 : 60);
}

static void tick(void)
{
	yeniden_baslat_tick();	/* v9 */
	if (M.sor_fd >= 0 && simdi() > M.sor_bitis) sor_bitir("zaman aşımı");	/* v18 */
	if (M.soyle_pid > 0 && simdi() > M.soyle_bitis) { kill(M.soyle_pid, SIGKILL); M.soyle_bitis = simdi() + 3600; }
	double t = simdi();
	/* kalkan: acilinca kamera, ses ve oynatma hemen durur */
	int k = kalkan();
	if (k && !M.kalkan_onceki) {
		if (M.kamera_acik) kamera_kapat("kamera KAPANDI (panelde gizlilik kalkani acildi)", "-");
		for (int i = 0; i < AZAMI_BAG; i++) {
			struct bag *c = &B[i];
			if (c->fd < 0) continue;
			if (c->tur == B_WS_DINLE || c->tur == B_WS_KONUS) {
				struct jy y = { .azami = 256 };
				jy_ham(&y, "{\"t\":\"son\",\"neden\":\"Panelde gizlilik kalkanı açıldı\"}");
				ws_metin(c, &y);
				snprintf(c->neden, sizeof c->neden, "gizlilik kalkanı");
				c->kapat_sonra = 1;
			} else if (c->tur == B_KAYIT || c->tur == B_INDIR) bag_kapat(c, "kalkan");
		}
		mic_dur();
		if (M.kon_bag < 0) kon_dur();
	}
	M.kalkan_onceki = k;
	/* kamera suresi / izleyicisiz */
	if (M.kamera_acik && M.kamera_bitis <= t) kamera_kapat("kamera sure doldu, KAPANDI", "-");
	if (M.kamera_acik) {
		int iz = izleyici_sayisi();
		if (iz) M.kamera_bos_t = t;
		else if (t - M.kamera_bos_t > KAMERA_IZLEYICISIZ_SN) {
			char m[96];
			snprintf(m, sizeof m, "kamera %d sn izleyicisiz kaldi, KAPANDI", KAMERA_IZLEYICISIZ_SN);
			kamera_kapat(m, "-");
		}
		if (M.kamera_acik && iz && M.kam_pid <= 0 && t >= M.kam_yeniden_t && !k) kamera_cocuk_baslat();
		if (M.kamera_acik && !iz && M.kam_pid > 0) kamera_cocuk_dur();
		if (M.kamera_acik && !dosya_var(kamera_bayrak)) kamera_kapat("kamera KAPANDI (bayrak cihazda silindi)", "-");
	}
	/* mikrofon */
	int dn = dinleyici_sayisi();
	if (dn && M.mic_pid <= 0 && t >= M.mic_yeniden_t && !k) mic_baslat();
	if (!dn && M.mic_pid > 0) { mic_dur(); M.mic_durum = 0; }
	/* baglanti zaman asimlari, izin bitisleri */
	for (int i = 0; i < AZAMI_BAG; i++) {
		struct bag *c = &B[i];
		if (c->fd < 0) continue;
		if (cikis_takildi(c, mono())) { bag_kapat(c, "istemci okumuyor"); continue; }	/* v13 (A4) */
		if (c->tur == B_ISTEK && t - c->son_t > (c->gn ? 15 : 30)) { bag_kapat(c, "zaman asimi"); continue; }
		if (c->tur == B_OTA && t - c->son_t > 30) { bag_kapat(c, "zaman asimi (guncelleme)"); continue; }	/* v4 */
		if (c->tur == B_YANIT && t - c->son_t > 120 && !cikis_bekleyen(c) && c->dosya_fd < 0) { bag_kapat(c, "zaman asimi"); continue; }
		if ((c->tur == B_CANLI) && (!oturum_gecerli(c->oturum) || !M.kamera_acik)) { bag_kapat(c, "oturum/kamera"); continue; }
		if (c->tur == B_WS_DINLE && !c->kapat_sonra) {
			if (!izin_aktif(c->oturum, O[c->oturum].dinle_bitis)) {
				struct jy y = { .azami = 128 };
				jy_ham(&y, "{\"t\":\"son\",\"neden\":\"Dinleme izni bitti\"}");
				ws_metin(c, &y);
				snprintf(c->neden, sizeof c->neden, "izin bitti ya da oturum kapandı");
				c->kapat_sonra = 1;
			} else if (c->ws_seq != M.mic_seq) {
				c->ws_seq = M.mic_seq;
				struct jy y = { .azami = 512 };
				if (M.mic_durum == 3) { jy_ham(&y, "{\"t\":\"hata\",\"metin\":"); jy_dizge(&y, M.mic_hata); jy_ham(&y, "}"); }
				else { jy_ham(&y, "{\"t\":\"durum\",\"ust\":"); jy_dizge(&y, mic_durum_metni()); jy_ham(&y, "}"); }
				ws_metin(c, &y);
			}
		}
		if (c->tur == B_WS_KONUS && !c->kapat_sonra) {
			const char *n = NULL;
			if (!izin_aktif(c->oturum, O[c->oturum].konus_bitis)) n = "izin bitti";
			else if (M.kon_hazir && t - c->ws_son_veri > 5) n = c->kon_bitiyor ? "cihaz yanıt vermedi" : "5 sn veri gelmedi";
			else if (!M.kon_hazir && t - c->t0 > 8) n = "ses süreci yanıt vermedi";
			if (n && !c->kon_bitiyor && M.kon_fd >= 0 && M.kon_hazir) {
				snprintf(c->neden, sizeof c->neden, "%s", n);
				c->kon_bitiyor = 1;
				c->ws_son_veri = t;
				if (M.kon_cn + 2 <= sizeof M.kon_cikis) { M.kon_cikis[M.kon_cn++] = 0; M.kon_cikis[M.kon_cn++] = 0; }
				kon_yaz();
			} else if (n) {
				konusma_bitti(c, n, "");
				kon_dur();
			}
		}
	}
	/* oturum temizligi */
	for (int i = 0; i < OTURUM_AZAMI; i++) if (O[i].var && !oturum_gecerli(i)) oturum_temizle(i);
	/* plan */
	if (t - M.plan_t >= PLAN_ARALIK_SN) { M.plan_t = t; plan_adim(); }
	/* ayar dosyasi disaridan degisti mi (parola-ayarla) */
	if (t - M.ayar_kontrol_t >= 5) {
		M.ayar_kontrol_t = t;
		struct stat st;
		if (!stat(ayar_yol, &st) && (st.st_mtim.tv_sec != M.ayar_mtime.tv_sec || st.st_mtim.tv_nsec != M.ayar_mtime.tv_nsec)) {
			long eski = M.surum;
			int var = M.ayar_var;
			if (!ayar_oku()) {
				if (!var || M.surum != eski) {
					for (int i = 0; i < OTURUM_AZAMI; i++) oturum_temizle(i);
					denetim("-", "parola dosyasi disaridan degisti: tum oturumlar kapandi");
				}
				M.kurulum_var = 0;
			}
		}
	}
	olenleri_denetle();
	if (t - M.durum_t >= 10) { M.durum_t = t; merkez_durum_yaz(); }
}

static void sinyal(int s)
{
	(void)s;
	M.dur = 1;
}

static void cocuklari_topla(void)
{
	int st;
	pid_t p;
	while ((p = waitpid(-1, &st, WNOHANG)) > 0) {
		if (p == M.kam_pid) M.kam_pid = 0;
		if (p == M.mic_pid) M.mic_pid = 0;
		if (p == M.kon_pid) M.kon_pid = 0;
		if (p == M.sor_pid) M.sor_pid = 0;	/* v18 */
		if (p == M.soyle_pid) M.soyle_pid = 0;
		for (int i = 0; i < 8; i++) if (OLEN[i].pid == p) OLEN[i].pid = 0;
	}
}

static int dinle_ac(int port, uint32_t adr)
{
	int l = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, 0);
	if (l < 0) return -1;
	int bir = 1;
	setsockopt(l, SOL_SOCKET, SO_REUSEADDR, &bir, sizeof bir);
	struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons((uint16_t)port),
				 .sin_addr.s_addr = htonl(adr) };
	if (bind(l, (struct sockaddr *)&a, sizeof a) || listen(l, 16)) { close(l); return -1; }
	return l;
}

static void durum_hazirla(void)
{
	memset(&M, 0, sizeof M);
	M.lfd = M.vfd = -1;
	M.sor_fd = M.sor_bag = -1;	/* v18 */
	M.pb_fd = M.kam_fd = M.mic_fd = M.kon_fd = -1;
	M.pb_bag = M.kon_bag = M.oyn_bag = M.yan_bag = -1;
	M.son_kare = -1;
	M.kam_yuva = -1;
	M.plan_onceki = -1;
	M.bosta_saat = BOSTA_VARSAYILAN_SAAT;	/* v2; M.siki = 0 (tek_giris) */
	M.baslangic = mono();
	M.port = VARSAYILAN_PORT;
	for (int i = 0; i < AZAMI_BAG; i++) { B[i].fd = -1; B[i].dosya_fd = -1; B[i].oyn_fd = -1; B[i].oyn_oturum = -1; B[i].oturum = -1; }
	memset(O, 0, sizeof O);
	memset(IPK, 0, sizeof IPK);
}

static int sunucu(void)
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, sinyal);
	signal(SIGINT, sinyal);
	gethostname(M.ana_ad, sizeof M.ana_ad - 1);
	for (char *q = M.ana_ad; *q; q++) *q = (char)tolower((unsigned char)*q);
	ayar_oku();
	kurulum_hazirla();
	M.lfd = dinle_ac(M.port, M.lan ? INADDR_ANY : INADDR_LOOPBACK);
	if (M.lfd < 0) { perror("dinle"); return 1; }
	if (!M.lan && (M.vfd = dinle_ac(M.port, 0x7f000002u)) < 0) perror("dinle 127.0.0.2");	/* v17 (A20); lan'da 0.0.0.0 alir */
	/* acilista eski "merkez" kamera bayragi kaldiysa temizle (onceki surec coktu) */
	char b[64];
	if (dosya_oku(kamera_bayrak, b, sizeof b) >= 0 && !strncmp(b, "merkez", 6)) unlink(kamera_bayrak);
	printf(SURUM_TAM ": %s:%d dinleniyor (web %s, ayar %s)\n", M.lan ? "0.0.0.0" : "127.0.0.1", M.port, web_kok,
	       M.ayar_var ? "var" : "YOK -> ilk kurulum");
	fflush(stdout);
	denetim("-", "cihaz merkezi basladi (%s:%d)%s", M.lan ? "0.0.0.0" : "127.0.0.1", M.port, M.ayar_var ? "" : " [ILK KURULUM BEKLENIYOR]");
	while (!M.dur) {
		struct pollfd p[AZAMI_BAG + 8];	/* v18: + sor */
		int bi[AZAMI_BAG + 8];
		int n = 0;
		p[n] = (struct pollfd){ M.lfd, POLLIN, 0 }; bi[n++] = -1;
		double t = simdi();
		int zaman = 100;
		for (int i = 0; i < AZAMI_BAG; i++) {
			struct bag *c = &B[i];
			if (c->fd < 0) continue;
			short ev = 0;
			if (c->gn < GELEN_AZAMI && c->tur != B_PBKDF2) ev |= POLLIN;
			int bekleyen = cikis_bekleyen(c) || c->kref || (c->dosya_fd >= 0 && c->dosya_kalan);
			if (c->gecikme_t > t) { int ms = (int)((c->gecikme_t - t) * 1000) + 1; if (ms < zaman) zaman = ms; }
			else if (bekleyen) {
				if (c->dosya_fd >= 0 && c->hiz > 0 && c->giden > c->hiz * (t - c->hiz_t0)) zaman = zaman > 20 ? 20 : zaman;
				else ev |= POLLOUT;
			}
			if (c->tur == B_KAYIT && !bekleyen) {
				int ms = (int)((c->oyn_sonraki - t) * 1000) + 1;
				if (ms < 1) ms = 1;
				if (ms < zaman) zaman = ms;
			}
			if (!bekleyen && c->kapat_sonra && c->gecikme_t <= t) ev |= POLLOUT;
			p[n] = (struct pollfd){ c->fd, ev, 0 }; bi[n++] = i;
		}
		if (M.vfd >= 0) { p[n] = (struct pollfd){ M.vfd, POLLIN, 0 }; bi[n++] = -6; }	/* v17 */
		if (M.sor_fd >= 0) { p[n] = (struct pollfd){ M.sor_fd, POLLIN, 0 }; bi[n++] = -7; }	/* v18 */
		if (M.pb_fd >= 0) { p[n] = (struct pollfd){ M.pb_fd, POLLIN, 0 }; bi[n++] = -2; }
		if (M.kam_fd >= 0) { p[n] = (struct pollfd){ M.kam_fd, POLLIN, 0 }; bi[n++] = -3; }
		if (M.mic_fd >= 0) { p[n] = (struct pollfd){ M.mic_fd, POLLIN, 0 }; bi[n++] = -4; }
		if (M.kon_fd >= 0) { p[n] = (struct pollfd){ M.kon_fd, (short)(POLLIN | (M.kon_cn ? POLLOUT : 0)), 0 }; bi[n++] = -5; }
		int r = poll(p, (nfds_t)n, zaman);
		cocuklari_topla();
		if (r < 0 && errno != EINTR) { perror("poll"); break; }
		for (int k = 0; r > 0 && k < n; k++) {
			if (!p[k].revents) continue;
			int i = bi[k];
			if (i == -1) { for (int a = 0; a < 8; a++) kabul(M.lfd); continue; }
			if (i == -6) { for (int a = 0; a < 8; a++) kabul(M.vfd); continue; }	/* v17 */
			if (i == -7) { if (M.sor_fd >= 0) sor_oku(); continue; }	/* v18 */
			if (i == -2) { if (M.pb_fd >= 0) pbkdf2_oku(); continue; }
			if (i == -3) { if (M.kam_fd >= 0) kamera_oku(); continue; }
			if (i == -4) { if (M.mic_fd >= 0) mic_oku(); continue; }
			if (i == -5) {
				if (M.kon_fd >= 0 && (p[k].revents & POLLOUT)) kon_yaz();
				if (M.kon_fd >= 0 && (p[k].revents & (POLLIN | POLLHUP | POLLERR))) kon_oku();
				continue;
			}
			struct bag *c = &B[i];
			if (c->fd < 0 || c->fd != p[k].fd) continue;
			if (p[k].revents & (POLLIN | POLLHUP | POLLERR)) bag_oku(c);
			if (c->fd >= 0 && (p[k].revents & POLLOUT) && bag_yaz(c)) bag_kapat(c, c->neden[0] ? c->neden : "bitti");
		}
		/* gecikmeli yanitlar / oynatma zamanlayicisi / bekleyen PBKDF2 sonrasi */
		t = simdi();
		for (int i = 0; i < AZAMI_BAG; i++) {
			struct bag *c = &B[i];
			if (c->fd < 0) continue;
			if (c->tur == B_KAYIT && !cikis_bekleyen(c) && t >= c->oyn_sonraki) { oynatma_adim(c); if (c->fd < 0) continue; }
			if (c->gecikme_t && c->gecikme_t <= t) { c->gecikme_t = 0; if (bag_yaz(c)) { bag_kapat(c, "bitti"); continue; } }
		}
		if (t - M.tick_t >= 0.25) { M.tick_t = t; tick(); }
	}
	/* kapanis */
	if (M.kamera_acik) kamera_kapat("kamera KAPANDI (merkez durduruldu)", "-");
	mic_dur();
	kon_dur();
	for (int i = 0; i < AZAMI_BAG; i++) if (B[i].fd >= 0) bag_kapat(&B[i], "kapanis");
	denetim("-", "cihaz merkezi durdu");
	unlink(merkez_durum_yol);
	return 0;
}

/* ------------------------------------------------------------------ parola-ayarla (stdin) / pbkdf2-olc */
static int parola_ayarla_kipi(void)
{
	char s[1100];
	fprintf(stderr, "Yeni KOLE cihaz merkezi parolasi (en az 10 karakter, tek satir): ");
	if (!fgets(s, sizeof s, stdin)) return 1;
	s[strcspn(s, "\r\n")] = 0;
	if (strlen(s) < 10 || strlen(s) > 1024) { fprintf(stderr, "\nParola 10..1024 karakter olmali\n"); memset(s, 0, sizeof s); return 1; }
	durum_hazirla();
	int vardi = !ayar_oku();
	rastgele(M.tuz, 16);
	M.tur = pbkdf2_tur_yeni;
	pbkdf2_sha256(s, strlen(s), M.tuz, 16, M.tur, M.sifre, 32);
	memset(s, 0, sizeof s);
	M.surum = vardi ? M.surum + 1 : 1;
	if (ayar_yaz()) { fprintf(stderr, "\n%s yazilamadi\n", ayar_yol); return 1; }
	unlink(kurulum_yol);
	sync();
	fprintf(stderr, "\nParola ayarlandi (%s, PBKDF2 %u tur). Calisan merkez 5 sn icinde yeniden yukler; tum oturumlar kapanir.\n",
		ayar_yol, M.tur);
	denetim("-", "PAROLA stdin ile ayarlandi (parola-ayarla)");
	return 0;
}

static int pbkdf2_olc(void)
{
	uint8_t h[32], tuz[16] = { 1 };
	uint32_t turlar[] = { 10000, 50000, 100000 };
	for (size_t i = 0; i < 3; i++) {
		double t = mono();
		pbkdf2_sha256("olcum-parolasi", 14, tuz, 16, turlar[i], h, 32);
		double d = mono() - t;
		printf("PBKDF2-HMAC-SHA256 %6u tur: %.3f sn (%.2f us/tur)\n", turlar[i], d, d * 1e6 / turlar[i]);
	}
	printf("oneri: girisin ~0.5 sn surdugu tur sayisi; kullanilan varsayilan %u\n", PBKDF2_VARSAYILAN);
	return 0;
}

/* ------------------------------------------------------------------ self-test */
static int hs;
#define DENE(x, ...) do { if (!(x)) { printf("FAIL %s:%d: ", __func__, __LINE__); printf(__VA_ARGS__); puts(""); hs++; } } while (0)

static void hexs(const uint8_t *v, size_t n, char *o) { hex_yaz(v, n, o); }

static void test_kripto(void)
{
	uint8_t h[64];
	char x[160];
	sha256_tek("abc", 3, h); hexs(h, 32, x);
	DENE(!strcmp(x, "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"), "sha256 abc %s", x);
	sha256_tek("", 0, h); hexs(h, 32, x);
	DENE(!strcmp(x, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"), "sha256 bos %s", x);
	const char *m = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
	sha256_tek(m, strlen(m), h); hexs(h, 32, x);
	DENE(!strcmp(x, "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1"), "sha256 448 %s", x);
	{
		struct sha256 s;
		sha256_bas(&s);
		char a[1000];
		memset(a, 'a', sizeof a);
		for (int i = 0; i < 1000; i++) sha256_ekle(&s, a, sizeof a);
		sha256_bitir(&s, h); hexs(h, 32, x);
		DENE(!strcmp(x, "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0"), "sha256 1M a %s", x);
	}
	/* RFC 4231 */
	uint8_t k1[20]; memset(k1, 0x0b, 20);
	hmac_sha256(k1, 20, "Hi There", 8, h); hexs(h, 32, x);
	DENE(!strcmp(x, "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"), "hmac 1 %s", x);
	hmac_sha256("Jefe", 4, "what do ya want for nothing?", 28, h); hexs(h, 32, x);
	DENE(!strcmp(x, "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"), "hmac 2 %s", x);
	uint8_t k6[131]; memset(k6, 0xaa, sizeof k6);
	const char *m6 = "Test Using Larger Than Block-Size Key - Hash Key First";
	hmac_sha256(k6, sizeof k6, m6, strlen(m6), h); hexs(h, 32, x);
	DENE(!strcmp(x, "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54"), "hmac 6 %s", x);
	/* PBKDF2 */
	pbkdf2_sha256("password", 8, (const uint8_t *)"salt", 4, 1, h, 32); hexs(h, 32, x);
	DENE(!strcmp(x, "120fb6cffcf8b32c43e7225256c4f837a86548c92ccc35480805987cb70be17b"), "pbkdf2 c1 %s", x);
	pbkdf2_sha256("password", 8, (const uint8_t *)"salt", 4, 2, h, 32); hexs(h, 32, x);
	DENE(!strcmp(x, "ae4d0c95af6b46d32d0adff928f06dd02a303f8ef3c251dfd6e2d85a95474c43"), "pbkdf2 c2 %s", x);
	pbkdf2_sha256("password", 8, (const uint8_t *)"salt", 4, 4096, h, 32); hexs(h, 32, x);
	DENE(!strcmp(x, "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a"), "pbkdf2 c4096 %s", x);
	pbkdf2_sha256("passwd", 6, (const uint8_t *)"salt", 4, 1, h, 64); hexs(h, 64, x);
	DENE(!strcmp(x, "55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc49ca9cccf179b645991664b39d77ef317c71b845b1e30bd509112041d3a19783"),
	     "pbkdf2 rfc7914 %s", x);
	pbkdf2_sha256("passwordPASSWORDpassword", 24, (const uint8_t *)"saltSALTsaltSALTsaltSALTsaltSALTsalt", 36, 4096, h, 40); hexs(h, 40, x);
	DENE(!strcmp(x, "348c89dbcbd32b2f32d814b8116e84cf2b17347ebc1800181c4e2a1fb8dd53e1c635518c7dac47e9"), "pbkdf2 40 %s", x);
	/* SHA-1 + WebSocket kabul (RFC 6455 4.2.2) */
	sha1_tek("abc", 3, h); hexs(h, 20, x);
	DENE(!strcmp(x, "a9993e364706816aba3e25717850c26c9cd0d89d"), "sha1 abc %s", x);
	sha1_tek(m, strlen(m), h); hexs(h, 20, x);
	DENE(!strcmp(x, "84983e441c3bd26ebaae4aa1f95129e5e54670f1"), "sha1 448 %s", x);
	for (size_t L = 0; L < 130; L++) {	/* tum dolgu sinirlari: 55/56/63/64 */
		char a[130];
		memset(a, 'x', L);
		sha1_tek(a, L, h);
		(void)h;
	}
	const char *wk = "dGhlIHNhbXBsZSBub25jZQ==258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	sha1_tek(wk, strlen(wk), h);
	b64_kodla(h, 20, x, sizeof x);
	DENE(!strcmp(x, "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="), "ws kabul %s", x);
	uint8_t d[32];
	DENE(b64_coz("dGhlIHNhbXBsZSBub25jZQ==", d, sizeof d) == 16, "b64 16");
	DENE(b64_coz("dGhlIHNhbXBsZSBub25jZQ=", d, sizeof d) < 0, "b64 eksik");
	DENE(b64_coz("dGhl IHNh", d, sizeof d) < 0, "b64 bosluk");
	DENE(b64_coz("====", d, sizeof d) < 0, "b64 yalniz dolgu");
	DENE(b64_coz("YQ==", d, sizeof d) == 1 && d[0] == 'a', "b64 a");
	DENE(b64_coz("YQ=a", d, sizeof d) < 0, "b64 dolgu ortada");
	DENE(sabit_esit("abc", "abc", 3) && !sabit_esit("abc", "abd", 3), "sabit esit");
}

static void test_http(void)
{
	struct http_istek r;
	const char *ok = "GET /api/durum?gun=20261013&esik=80 HTTP/1.1\r\nHost: velinux.tail1234.ts.net\r\nCookie: a=1; kole_cm=abc\r\n"
			 "Connection: keep-alive\r\nX-Kole: 1\r\n\r\n";
	long n = http_ayristir(ok, strlen(ok), &r);
	DENE(n == (long)strlen(ok) && !strcmp(r.yontem, "GET") && !strcmp(r.yol, "/api/durum") && r.keepalive && !strcmp(r.xkole, "1") &&
	     !strcmp(r.host, "velinux.tail1234.ts.net"), "gecerli istek %ld", n);
	char v[64];
	DENE(sorgu_al(r.sorgu, "gun", v, sizeof v) == 1 && !strcmp(v, "20261013"), "sorgu gun");
	DENE(sorgu_al(r.sorgu, "yok", v, sizeof v) == 0, "sorgu yok");
	DENE(cerez_al(r.cookie, "kole_cm", v, sizeof v) == 1 && !strcmp(v, "abc"), "cerez");
	DENE(sorgu_al("ad=..%2F..%2Fetc", "ad", v, sizeof v) == 1 && !strcmp(v, "../../etc"), "yuzde coz");
	DENE(sorgu_al("ad=a%00b", "ad", v, sizeof v) == -1, "%%00 red");
	DENE(sorgu_al("ad=a%2", "ad", v, sizeof v) == -1, "yarim kacis");
	DENE(sorgu_al("ad=a%zz", "ad", v, sizeof v) == -1, "gecersiz kacis");
	static const struct { const char *s; long bek; } V[] = {
		{ "GET / HTTP/1.1\r\nHost: a\r\n", 0 },
		{ "GET / HTTP/1.1\nHost: a\n\n", 0 },
		{ "GET / HTTP/1.1\r\nHost: a\n\r\n\r\n", -1 },
		{ "GET / HTTP/2.0\r\n\r\n", -1 },
		{ "get / HTTP/1.1\r\n\r\n", -1 },
		{ "GET noslash HTTP/1.1\r\n\r\n", -1 },
		{ "GET / HTTP/1.1\r\n Host: a\r\n\r\n", -1 },
		{ "GET / HTTP/1.1\r\nHo st: a\r\n\r\n", -1 },
		{ "GET / HTTP/1.1\r\nHost a\r\n\r\n", -1 },
		{ "POST / HTTP/1.1\r\nContent-Length: 5\r\nContent-Length: 6\r\n\r\n", -1 },
		{ "POST / HTTP/1.1\r\nContent-Length: -5\r\n\r\n", -1 },
		{ "POST / HTTP/1.1\r\nContent-Length: 5x\r\n\r\n", -1 },
		{ "POST / HTTP/1.1\r\nTransfer-Encoding: chunked\r\n\r\n", -3 },
		{ "DELETE / HTTP/1.1\r\n\r\n", -3 },
		{ "GET /a b HTTP/1.1\r\n\r\n", -1 },
		{ "GET /\x01 HTTP/1.1\r\n\r\n", -1 },
		{ "GET / HTTP/1.1\r\nX: a\x01\r\n\r\n", -1 },
		{ "GET / HTTP/1.0\r\n\r\n", 18 },
	};
	for (size_t i = 0; i < sizeof V / sizeof V[0]; i++) {
		long g = http_ayristir(V[i].s, strlen(V[i].s), &r);
		DENE(g == V[i].bek, "http durum %zu: %ld beklenen %ld", i, g, V[i].bek);
	}
	DENE(http_ayristir("GET / HTTP/1.0\r\n\r\n", 18, &r) == 18 && !r.keepalive, "1.0 keepalive yok");
	{
		const char *s = "GET / HTTP/1.1\r\nConnection: Upgrade, keep-alive\r\nUpgrade: websocket\r\n\r\n";
		DENE(http_ayristir(s, strlen(s), &r) > 0 && baglanti_upgrade_mi(&r) && r.keepalive, "upgrade listesi");
	}
	/* cok buyuk */
	static char buyuk[HTTP_BASLIK_AZAMI + 100];
	memset(buyuk, 'a', sizeof buyuk);
	memcpy(buyuk, "GET / HTTP/1.1\r\nX: ", 19);
	DENE(http_ayristir(buyuk, sizeof buyuk, &r) == -2, "buyuk baslik");
	/* 65 satir */
	{
		char s[4096];
		size_t w = (size_t)snprintf(s, sizeof s, "GET / HTTP/1.1\r\n");
		for (int i = 0; i < 70; i++) w += (size_t)snprintf(s + w, sizeof s - w, "X%d: y\r\n", i);
		w += (size_t)snprintf(s + w, sizeof s - w, "\r\n");
		DENE(http_ayristir(s, w, &r) == -2, "cok satir");
	}
	/* bulanik: rastgele bayt dizileri ve kesilmis gecerli istekler cokme/okuma tasmasi yapmamali */
	uint32_t tohum = 12345;
	for (int i = 0; i < 20000; i++) {
		uint8_t b[300];
		size_t L = (tohum = tohum * 1103515245 + 12345) % sizeof b;
		for (size_t k = 0; k < L; k++) {
			tohum = tohum * 1103515245 + 12345;
			uint8_t x = (uint8_t)(tohum >> 16);
			b[k] = (tohum & 0x300) ? (uint8_t)"GET /HTTP1.\r\n: Hostxa"[x % 21] : x;
		}
		http_ayristir((const char *)b, L, &r);
	}
	for (size_t L = 0; L <= strlen(ok); L++) {
		long g = http_ayristir(ok, L, &r);
		DENE(L == strlen(ok) ? g > 0 : g == 0, "kesik %zu -> %ld", L, g);
	}
	char nl[64];
	DENE(!koken_netloc("https://velinux.tail1.ts.net", nl, sizeof nl) && !strcmp(nl, "velinux.tail1.ts.net"), "origin");
	DENE(koken_netloc("https://a.b/yol", nl, sizeof nl) < 0, "origin yol");
	DENE(koken_netloc("null", nl, sizeof nl) < 0, "origin null");
	DENE(koken_netloc("file://x", nl, sizeof nl) < 0, "origin file");
}

static void test_ws(void)
{
	uint8_t b[70000], h[10];
	struct ws_cerceve f;
	/* maskeli metin "Hello" (RFC 6455 5.7) */
	uint8_t m1[] = { 0x81, 0x85, 0x37, 0xfa, 0x21, 0x3d, 0x7f, 0x9f, 0x4d, 0x51, 0x58 };
	memcpy(b, m1, sizeof m1);
	long r = ws_coz(b, sizeof m1, 1024, &f);
	DENE(r == (long)sizeof m1 && f.fin && f.op == 1 && f.n == 5 && !memcmp(f.veri, "Hello", 5), "ws hello %ld", r);
	for (size_t L = 0; L < sizeof m1; L++) { memcpy(b, m1, sizeof m1); DENE(ws_coz(b, L, 1024, &f) == 0, "ws kesik %zu", L); }
	uint8_t m2[] = { 0x81, 0x05, 'H', 'e', 'l', 'l', 'o' };
	memcpy(b, m2, sizeof m2);
	DENE(ws_coz(b, sizeof m2, 1024, &f) == -1, "ws maskesiz");
	uint8_t m3[] = { 0xc1, 0x80, 0, 0, 0, 0 };
	memcpy(b, m3, sizeof m3);
	DENE(ws_coz(b, sizeof m3, 1024, &f) == -1, "ws rsv");
	uint8_t m4[] = { 0x09, 0x80, 0, 0, 0, 0 };
	memcpy(b, m4, sizeof m4);
	DENE(ws_coz(b, sizeof m4, 1024, &f) == -1, "ws fin'siz ping");
	uint8_t m5[] = { 0x89, 0xfe, 0x00, 0x7e };
	memcpy(b, m5, sizeof m5);
	DENE(ws_coz(b, sizeof m5, 1024, &f) == -1, "ws uzun ping");
	uint8_t m6[] = { 0x83, 0x80, 0, 0, 0, 0 };
	memcpy(b, m6, sizeof m6);
	DENE(ws_coz(b, sizeof m6, 1024, &f) == -1, "ws op 3");
	/* 126 / 127 uzunluk kodlamalari */
	size_t L = 300;
	b[0] = 0x82; b[1] = 0x80 | 126; b[2] = (uint8_t)(L >> 8); b[3] = (uint8_t)L;
	memset(b + 4, 0, 4);
	memset(b + 8, 7, L);
	DENE(ws_coz(b, 8 + L, 1024, &f) == (long)(8 + L) && f.n == L && f.veri[0] == 7, "ws 126");
	DENE(ws_coz(b, 8 + L, 100, &f) == -2, "ws sinir");
	b[0] = 0x82; b[1] = 0x80 | 126; b[2] = 0; b[3] = 5;
	DENE(ws_coz(b, 8 + 5, 1024, &f) == -1, "ws 126 kisa (asgari kodlama)");
	b[0] = 0x82; b[1] = 0x80 | 127;
	memset(b + 2, 0, 8);
	b[6] = 1;	/* 2^24 */
	DENE(ws_coz(b, 14, 65536, &f) == -2, "ws 127 buyuk");
	b[2] = 0x80;
	DENE(ws_coz(b, 14, 65536, &f) == -1, "ws 127 isaret biti");
	DENE(ws_baslik(h, 2, 5) == 2 && h[0] == 0x82 && h[1] == 5, "ws baslik 5");
	DENE(ws_baslik(h, 2, 200) == 4 && h[1] == 126 && h[2] == 0 && h[3] == 200, "ws baslik 200");
	DENE(ws_baslik(h, 2, 70000) == 10 && h[1] == 127 && h[7] == 1, "ws baslik 70000");
	uint32_t tohum = 7;
	for (int i = 0; i < 20000; i++) {
		size_t n = (tohum = tohum * 1103515245 + 12345) % 40;
		for (size_t k = 0; k < n; k++) b[k] = (uint8_t)((tohum = tohum * 1103515245 + 12345) >> 16);
		long g = ws_coz(b, n, 64, &f);
		DENE(g <= (long)n, "ws bulanik tasma");
	}
}

static void test_json(void)
{
	static struct jtok tk[64];
	const char *s = "{\"sifre\":\"p\\u00e7\\\"x\",\"dakika\":5,\"acik\":true,\"kurallar\":[{\"gunler\":[0,6],\"bas\":\"22:00\",\"bit\":\"06:30\"}],\"z\":null,\"f\":1.5e2}";
	int t = json_ayristir(s, (int)strlen(s), tk, 64);
	DENE(t > 0, "json gecerli %d", t);
	char v[64];
	long long n;
	DENE(!j_dizge_al(s, &tk[j_al(s, tk, t, 0, "sifre")], v, sizeof v) && !strcmp(v, "p\xc3\xa7\"x"), "json dizge %s", v);
	DENE(!j_tamsayi(s, &tk[j_al(s, tk, t, 0, "dakika")], &n) && n == 5, "json int");
	DENE(j_bool(s, &tk[j_al(s, tk, t, 0, "acik")]) == 1, "json bool");
	DENE(j_tamsayi(s, &tk[j_al(s, tk, t, 0, "f")], &n) == -1, "json kesir int degil");
	DENE(j_null(s, &tk[j_al(s, tk, t, 0, "z")]), "json null");
	struct kural kk[8];
	int kn;
	const char *e;
	DENE(!plan_ayristir(s, tk, t, j_al(s, tk, t, 0, "kurallar"), kk, &kn, &e) && kn == 1 && kk[0].gun == 0x41 && kk[0].bas == 1320 &&
	     kk[0].bit == 390, "plan ayristir");
	static const char *K[] = { "", "{", "{\"a\":}", "{\"a\":1,}", "[1,2", "{\"a\":01}", "{\"a\":tru}", "{\"a\":\"\x01\"}", "{\"a\":\"\\x\"}",
		"{\"a\":1} x", "{a:1}", "{\"a\":-}", "{\"a\":1.}", "{\"a\":\"\\u12\"}", "[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[1]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]" };
	for (size_t i = 0; i < sizeof K / sizeof K[0]; i++) DENE(json_ayristir(K[i], (int)strlen(K[i]), tk, 64) < 0, "json red %zu", i);
	const char *sur = "{\"a\":\"\\ud83d\\ude00\",\"b\":\"\\ud83d\",\"c\":\"\\u0000\"}";
	t = json_ayristir(sur, (int)strlen(sur), tk, 64);
	DENE(t > 0 && !j_dizge_al(sur, &tk[j_al(sur, tk, t, 0, "a")], v, sizeof v) && !strcmp(v, "\xf0\x9f\x98\x80"), "json vekil cift");
	DENE(j_dizge_al(sur, &tk[j_al(sur, tk, t, 0, "b")], v, sizeof v) == -1, "json yarim vekil");
	DENE(j_dizge_al(sur, &tk[j_al(sur, tk, t, 0, "c")], v, sizeof v) == -1, "json NUL");
	struct jy y = { .azami = 4096 };
	jy_dizge(&y, "a\"b\\c\n</script>\xe2\x80\xa8\xff");
	DENE(y.b && !strcmp(y.b, "\"a\\\"b\\\\c\\u000a\\u003c/script\\u003e\\u2028\\ufffd\""), "jy kacis %s", y.b ? y.b : "");
	jy_bitir(&y);
	struct jy z = { .azami = 16 };
	jy_ham(&z, "0123456789012345678");
	DENE(z.hata, "jy azami");
	jy_bitir(&z);
	uint32_t tohum = 99;
	for (int i = 0; i < 20000; i++) {
		char b[64];
		size_t L = (tohum = tohum * 1103515245 + 12345) % sizeof b;
		for (size_t k = 0; k < L; k++) b[k] = "{}[]\":,0123456789truefalsenull\\u -.eE ab"[((tohum = tohum * 1103515245 + 12345) >> 16) % 41];
		int r = json_ayristir(b, (int)L, tk, 64);
		for (int k = 0; r > 0 && k < r; k++) DENE(tk[k].son <= (int)L && tk[k].bas >= 0, "json bulanik sinir");
	}
}

static void test_oturum(void)
{
	durum_hazirla();
	M.surum = 3;
	saat_kayma = 0;
	char sid[49], ck[128];
	int i = oturum_ac("1.2.3.4", "Mozilla/5.0 (iPhone; CPU iPhone OS 17_0) Safari/604.1", sid);
	snprintf(ck, sizeof ck, "x=1; kole_cm=%s", sid);
	DENE(oturum_bul(ck) == i, "oturum bul");
	DENE(!strcmp(O[i].cihaz, "iPhone · Safari"), "cihaz ozet %s", O[i].cihaz);
	ck[strlen(ck) - 1] ^= 1;
	DENE(oturum_bul(ck) < 0, "yanlis sid");
	DENE(oturum_bul("kole_cm=zz") < 0, "bozuk sid");
	snprintf(ck, sizeof ck, "kole_cm=%s", sid);
	DENE(bosta_sn() == BOSTA_VARSAYILAN_SAAT * 3600.0, "bosta varsayilan 12 sa");
	saat_kayma = bosta_sn() - 60;	/* 11 sa 59 dk: hala gecerli */
	DENE(oturum_bul(ck) == i, "bosta siniri alti");
	saat_kayma = bosta_sn() + 1;	/* 12 saat bosta */
	DENE(oturum_bul(ck) < 0, "bosta suresi");
	saat_kayma = 0;
	O[i].son_m = simdi();
	DENE(oturum_bul(ck) == i, "bosta sonrasi etkinlik");
	M.bosta_saat = 1;	/* v2: ayarlanabilir */
	saat_kayma = 3601;
	DENE(oturum_bul(ck) < 0, "bosta 1 sa");
	M.bosta_saat = 72;
	DENE(oturum_bul(ck) == i, "bosta 72 sa (ayni oturum yeniden gecerli)");
	M.bosta_saat = 999;	/* gecersiz deger: varsayilan */
	DENE(bosta_sn() == BOSTA_VARSAYILAN_SAAT * 3600.0, "gecersiz bosta -> 12");
	M.bosta_saat = BOSTA_VARSAYILAN_SAAT;
	saat_kayma = 0;
	O[i].son_m = simdi();
	saat_kayma = OTURUM_OMUR + 1;
	O[i].son_m = simdi();
	DENE(oturum_bul(ck) < 0, "omur");
	saat_kayma = 0;
	i = oturum_ac("1.2.3.4", "", sid);
	snprintf(ck, sizeof ck, "kole_cm=%s", sid);
	M.surum++;
	DENE(oturum_bul(ck) < 0, "parola degisince surum");
	/* izin pencereleri */
	M.surum--;
	O[i].kayit_bitis = simdi() + 600;
	DENE(kayit_izin_aktif(i) && izin_kalan(i, O[i].kayit_bitis) >= 599, "kayit izni");
	saat_kayma = 601;
	O[i].son_m = simdi();
	DENE(!kayit_izin_aktif(i), "kayit izni bitti");
	saat_kayma = 0;
	/* dolu tablo: en eski kapanir */
	durum_hazirla();
	for (int k = 0; k < OTURUM_AZAMI + 3; k++) oturum_ac("9.9.9.9", "", sid);
	DENE(oturum_sayisi() == OTURUM_AZAMI, "oturum siniri %d", oturum_sayisi());
	/* kaba kuvvet */
	durum_hazirla();
	for (int k = 0; k < 4; k++) hata_ekle("5.5.5.5");
	DENE(!deneme_engeli("5.5.5.5"), "4 hata serbest");
	hata_ekle("5.5.5.5");
	DENE(deneme_engeli("5.5.5.5") == 1, "5 hata engel");
	DENE(!deneme_engeli("6.6.6.6"), "baska ip serbest");
	saat_kayma = 901;
	DENE(!deneme_engeli("5.5.5.5"), "15 dk sonra serbest");
	saat_kayma = 0;
	durum_hazirla();
	char ip[32];
	for (int k = 0; k < 20; k++) { snprintf(ip, sizeof ip, "10.0.0.%d", k); hata_ekle(ip); }
	DENE(deneme_engeli("10.9.9.9") == 2, "genel kilit");
	saat_kayma = 1801;
	DENE(!deneme_engeli("10.9.9.9"), "genel kilit bitti");
	saat_kayma = 0;
	/* 40 farkli IP: tablo tasmaz */
	durum_hazirla();
	for (int k = 0; k < 40; k++) { snprintf(ip, sizeof ip, "172.16.0.%d", k); hata_ekle(ip); }
	DENE(1, "ip tablosu");
}

static void test_yollar(void)
{
	DENE(statik_ad_gecerli("index.html") && statik_ad_gecerli("icon-192.png") && statik_ad_gecerli("sw.js"), "statik iyi");
	static const char *K[] = { "", "../x.js", ".htaccess", "a/b.js", "x.sh", "a..js", "%2e%2e", "a b.js", "x.js\n", "kole-web.json/../a" };
	for (size_t i = 0; i < sizeof K / sizeof K[0]; i++) DENE(!statik_ad_gecerli(K[i]), "statik red %s", K[i]);
	DENE(kayit_ad_gecerli("20261013/0930.mjpg"), "kayit ad");
	static const char *R[] = { "../../data/kole-web.json", "20261013/0930.mjpg/..", "20261013/2460.mjpg", "20261313/0930.mjpg",
		"20261013/0930.mjpgx", "20261013\\0930.mjpg", "/20261013/0930.mjp", "2026101/30930.mjpg" };
	for (size_t i = 0; i < sizeof R / sizeof R[0]; i++) DENE(!kayit_ad_gecerli(R[i]), "kayit red %s", R[i]);
	durum_hazirla();
	snprintf(M.ana_ad, sizeof M.ana_ad, "velinux");
	DENE(host_uygun("velinux.tail9.ts.net") && host_uygun("127.0.0.1:8780") && host_uygun("192.168.1.60") && host_uygun("velinux") &&
	     host_uygun("velinux.local:8780") && host_uygun("localhost:8780") && host_uygun("[::1]:8780"), "host iyi");
	DENE(!host_uygun("evil.example.com") && !host_uygun("velinux.ts.net.evil.com") && !host_uygun("") && !host_uygun("a:b"), "host red");
}

static void test_plan(void)
{
	struct kural k = { 0x01, 22 * 60, 6 * 60 };	/* Pzt 22:00 - 06:00 (gece yarisini asar) */
	DENE(kural_aktif(&k, 0, 23 * 60) && kural_aktif(&k, 1, 5 * 60) && !kural_aktif(&k, 1, 23 * 60) && !kural_aktif(&k, 0, 21 * 60),
	     "gece yarisi");
	struct kural k2 = { 0x7f, 8 * 60, 9 * 60 };
	DENE(kural_aktif(&k2, 3, 8 * 60) && !kural_aktif(&k2, 3, 9 * 60), "yari acik aralik");
	struct kural k3 = { 0x40, 23 * 60, 1 * 60 };	/* Paz 23:00 -> Pzt 01:00 */
	DENE(kural_aktif(&k3, 0, 30), "pazar -> pazartesi");
	char m[96];
	kural_metni(&k, m, sizeof m);
	DENE(!strcmp(m, "Pzt 22:00-06:00"), "kural metni %s", m);
}

/* v2: ayar dosyasi guvenlik anahtarlari (yaz/oku, eski v1 dosyasi, gecersiz deger) + denetim turu */
static void test_guvenlik(const char *sab)
{
	static char ay[300];
	snprintf(ay, sizeof ay, "%s/kole-web.json", sab);
	const char *eski_yol = ayar_yol;
	ayar_yol = ay;
	durum_hazirla();
	DENE(!M.siki && M.bosta_saat == 12, "durum_hazirla varsayilan");
	M.tur = 2000;
	memset(M.tuz, 1, 16);
	memset(M.sifre, 2, 32);
	M.surum = 4;
	M.siki = 1;
	M.bosta_saat = 6;
	DENE(!ayar_yaz(), "ayar yaz");
	durum_hazirla();
	DENE(!ayar_oku() && M.ayar_var && M.siki == 1 && M.bosta_saat == 6 && M.surum == 4, "ayar oku siki/6");
	M.siki = 0;
	M.bosta_saat = 72;
	ayar_yaz();
	durum_hazirla();
	DENE(!ayar_oku() && !M.siki && M.bosta_saat == 72, "ayar oku tek_giris/72");
	static const char *V1 = "{\"bicim\":1,\"tur\":2000,\"tuz\":\"01010101010101010101010101010101\",\"sifre\":"
		"\"0202020202020202020202020202020202020202020202020202020202020202\",\"surum\":2,\"jeton\":null,"
		"\"plan\":{\"acik\":false,\"kurallar\":[]}}\n";
	int fd = open(ay, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
	DENE(fd >= 0 && !tam_yaz(fd, V1, strlen(V1)), "v1 ayar yaz");
	if (fd >= 0) close(fd);
	durum_hazirla();
	M.siki = 1;
	M.bosta_saat = 3;
	DENE(!ayar_oku() && M.ayar_var && !M.siki && M.bosta_saat == 12 && M.surum == 2, "v1 ayar dosyasi -> tek_giris/12");
	static const char *K = "{\"tur\":2000,\"tuz\":\"01010101010101010101010101010101\",\"sifre\":"
		"\"0202020202020202020202020202020202020202020202020202020202020202\",\"yeniden_dogrulama\":\"SIKI\",\"oturum_bosta_saat\":73}";
	fd = open(ay, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
	DENE(fd >= 0 && !tam_yaz(fd, K, strlen(K)), "gecersiz ayar yaz");
	if (fd >= 0) close(fd);
	durum_hazirla();
	DENE(!ayar_oku() && !M.siki && M.bosta_saat == 12, "gecersiz mod/saat -> varsayilan");
	unlink(ay);
	ayar_yol = eski_yol;
	durum_hazirla();
	static const struct { const char *o, *t; } T[] = {
		{ "GUVENLIK MODU: tek dogrulama (oturum acikken hassas islemler parola istemez)", "guvenlik" },
		{ "GUVENLIK MODU: siki (hassas islemler her seferinde parola ister)", "guvenlik" },
		{ "GUVENLIK MODU DEGISIMI REDDEDILDI (parola)", "guvenlik" },
		{ "OTURUM BOSTA SURESI 12 -> 6 saat", "guvenlik" },
		{ "KAMERA ACILDI 5 dk" TEK_EK, "kamera" },
		{ "KAYIT IZNI VERILDI 10 dk" TEK_EK, "kayit" },
		{ "SES DINLEME IZNI VERILDI 5 dk" TEK_EK, "ses" },
		{ "KONUSMA IZNI VERILDI 2 dk" TEK_EK, "ses" },
		{ "denetim gunlugu goruntulendi" TEK_EK, "diger" },
		{ "giris abcdef012345", "giris" },
		{ "KISAYOL JETONU REDDEDILDI (parola)", "guvenlik" },
	};
	for (size_t i = 0; i < sizeof T / sizeof T[0]; i++) DENE(!strcmp(denetim_tur(T[i].o), T[i].t), "denetim tur %s", T[i].o);
	DENE(!strstr(TEK_EK, "giris") && !strstr(TEK_EK, "GIRIS"), "ek giris icermez");
}

static void test_ota(void)
{
	char g[96];
	static const char *iyi[][2] = { { "panel", "panel" }, { "kole-asistan-v9", "kole-asistan-v9" },
		{ "kole-gemini-v12", "asistan/kole-gemini-v12" }, { "kamera-kayit-v7", "kamera-kayit-v7" },
		{ "kole-kopru-v10", "kole-kopru-v10" }, { "kole-cihaz-merkez-v4", "kole-cihaz-merkez-v4" },
		{ "autorun_v27.sh", "autorun_v27.sh" }, { "kamera_kopru_sd_v10.sh", "kamera_kopru_sd_v10.sh" },
		{ "wifi-ekran-v22", "wifi-ekran-v22" } };
	for (size_t i = 0; i < sizeof iyi / sizeof *iyi; i++)
		DENE(!ota_ad_coz(iyi[i][0], g, sizeof g) && !strcmp(g, iyi[i][1]), "ota adi %s -> %s", iyi[i][0], g);
	static const char *kotu[] = { "", "autorun.sh", "../panel", "panel2", "kole-asistan-v", "kole-asistan-v0",
		"kole-asistan-v1000", "kole-asistan-v1a", "kole-asistan-v1/../x", "autorun_v3", "autorun_v3.sh.x",
		"kole-gemini-v3/../../etc", "tailscaled", "sd_fel_switcher.bin", "kamera-kayit-v-1", ".ota-panel.part",
		"autorun_v.sh", "kole-kopru-v9 ", "Panel", "wifi-ekran", "wifi-ekran-v", "wifi-ekran-v22.sh" };
	for (size_t i = 0; i < sizeof kotu / sizeof *kotu; i++) DENE(ota_ad_coz(kotu[i], g, sizeof g), "ota adi reddedilmedi: '%s'", kotu[i]);
	char uzun[80];
	memset(uzun, '1', sizeof uzun);
	memcpy(uzun, "kole-asistan-v", 14);
	uzun[79] = 0;
	DENE(ota_ad_coz(uzun, g, sizeof g), "uzun ad reddedilmedi");
}

/* v10: zaman atlamali kare bulma, gun/foto adi, kullanici adi */
static void test_zaman(const char *sab)
{
	char y[300];
	snprintf(y, sizeof y, "%s/z.mjpg", sab);
	static uint8_t b[4096], t[1024];
	size_t n = 0;
	const uint8_t cop[] = {0x00, 0x11, 0xFF, 0x00};
	memcpy(b + n, cop, sizeof cop); n += sizeof cop;	/* cop onek */
	size_t bas[3], uz[3];
	for (int k = 0; k < 3; k++) {	/* 3 kare: FF D8 FF E0 ... (icinde FF D8 ama FF D9 yok) ... FF D9 */
		bas[k] = n;
		const uint8_t h[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10};
		memcpy(b + n, h, sizeof h); n += sizeof h;
		for (int i = 0; i < 50 + k * 30; i++) b[n++] = (uint8_t)(i * 7 + k);
		b[n++] = 0xFF; b[n++] = 0xD8; b[n++] = 0x12;	/* sinir degil (onunde FF D9 yok) */
		b[n++] = 0xFF; b[n++] = 0xD9;
		uz[k] = n - bas[k];
	}
	size_t tam = n;
	const uint8_t yarim[] = {0xFF, 0xD8, 0xFF, 0xE0, 0x01, 0x02};	/* yazilmakta olan yarim kare */
	memcpy(b + n, yarim, sizeof yarim); n += sizeof yarim;
	int fd = open(y, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	DENE(fd >= 0 && write(fd, b, n) == (ssize_t)n, "test dosyasi");
	close(fd);
	fd = open(y, O_RDONLY);
	unsigned long long kb = 0, ofs = 0;
	uint32_t ku = 0;
	for (int k = 0; k < 3; k++) {
		int r = zaman_kare_bul(fd, n, ofs, t, sizeof t, &kb, &ku);
		DENE(r == 1 && kb == bas[k] && ku == uz[k], "kare %d: r %d bas %llu uz %u (beklenen %zu %zu)", k, r, kb, ku, bas[k], uz[k]);
		ofs = kb + ku;
	}
	DENE(ofs == tam && zaman_kare_bul(fd, n, ofs, t, sizeof t, &kb, &ku) == 0, "yarim son kare gonderilmez");
	DENE(zaman_kare_atla(fd, n, 0, 2, t, sizeof t, &kb, &ku) == 1 && kb == bas[2], "atla 2 -> ucuncu kare");
	DENE(zaman_kare_atla(fd, n, 0, 5, t, sizeof t, &kb, &ku) == 0, "atla dosya sonunu asar -> 0");
	DENE(zaman_kare_bul(fd, tam, bas[2], t, sizeof t, &kb, &ku) == 1 && kb == bas[2] && ku == uz[2], "FF D9 ile biten son kare");
	DENE(zaman_kare_bul(fd, n, 0, t, 32, &kb, &ku) == 2 && kb == bas[0] && ku == 32, "tampondan (32) buyuk kare -> 2");
	close(fd);
	unlink(y);
	{	/* v11: olay csv */
		const char *ornek = "1000,250,1234\n1001,10,-1\nbozuk\n2460,5,0\n1002,1001,3\n1003,30,-2\n1004,40,7\n10a5,1,1\n1005,9";
		struct zo_satir zs[8];
		int zk = zaman_olay_coz(ornek, strlen(ornek), zs, 8);
		DENE(zk == 3 && zs[0].dk == 600 && zs[0].skor == 250 && zs[0].ofs == 1234 && zs[1].dk == 601 && zs[1].ofs == -1 &&
		     zs[2].dk == 604 && zs[2].ofs == 7, "olay csv (%d)", zk);
		DENE(zaman_olay_coz(ornek, strlen(ornek), zs, 1) == 1 && zaman_olay_coz("", 0, zs, 8) == 0, "olay csv sinir");
	}
	DENE(zaman_gun_gecerli("20261022") && !zaman_gun_gecerli("2026102") && !zaman_gun_gecerli("19991022") &&
	     !zaman_gun_gecerli("2026102a") && !zaman_gun_gecerli("../../x"), "gun adi");
	DENE(foto_ad_gecerli("foto-20261021-2342.jpg") && foto_ad_gecerli("IMG_01.JPEG") && !foto_ad_gecerli("../x.jpg") &&
	     !foto_ad_gecerli(".gizli.jpg") && !foto_ad_gecerli("a b.jpg") && !foto_ad_gecerli("x.png") && !foto_ad_gecerli("a..jpg") &&
	     !foto_ad_gecerli("x/y.jpg") && !foto_ad_gecerli(".jpg"), "foto adi");
	DENE(kullanici_uygun("efendi") && kullanici_uygun(" Efendi ") && kullanici_uygun("EFENDI") && !kullanici_uygun("efend") &&
	     !kullanici_uygun("efendim") && !kullanici_uygun("") && !kullanici_uygun("e fendi"), "kullanici adi");
}

static int t12_yaz(const char *y, const char *b, size_t n)
{
	int fd = open(y, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd < 0) return -1;
	ssize_t w = write(fd, b, n);
	close(fd);
	return w == (ssize_t)n ? 0 : -1;
}

/* v12: plan acilista kapat, denetim halkasi, foto_ekle en yeniler, cmdline hizmet adi */
static void test_v12(const char *sab)
{
	static char k12[300], ka[400];
	const char *eski_kok = kok, *eski_proc = proc_kok;
	snprintf(k12, sizeof k12, "%s/kamera12", sab);
	kok = k12;
	mkdir(k12, 0755);
	snprintf(ka, sizeof ka, "%s/kayit-acik", k12);
	t12_yaz(ka, "plan\n", 5);	/* v13: planin yazdigi bayrak (bos = elle acilmis, test_v13) */
	int eski_acik = M.plan_acik, eski_n = M.plan_n;
	M.plan_acik = 1;
	M.plan_n = 0;	/* hic kural yok: simdi kayit istenmez */
	M.plan_onceki = -1;
	plan_adim();
	DENE(!dosya_var(ka) && M.plan_onceki == 0, "v12/v13: plan acilista kendi bayragini kapatir (kayit-acik kalmamali)");
	M.plan_acik = eski_acik;
	M.plan_n = eski_n;
	M.plan_onceki = -1;
	rmdir(k12);
	kok = eski_kok;
	static struct bag tb;
	tb.oturum = 0;
	O[0].kucuk_n = 0;
	char an[12];
	int ilk = 0;
	for (int i = 0; i < 30; i++) { snprintf(an, sizeof an, "k%d", i); ilk += oturumda_ilk(&tb, an); }
	snprintf(an, sizeof an, "k%d", 29);
	int son_tekrar = oturumda_ilk(&tb, an);
	snprintf(an, sizeof an, "k%d", 6);
	int yakin = oturumda_ilk(&tb, an);
	snprintf(an, sizeof an, "k%d", 0);
	int eski = oturumda_ilk(&tb, an);
	DENE(ilk == 30 && !son_tekrar && !yakin && eski, "v12: denetim halkasi (ilk %d, son %d, k6 %d, k0 %d)", ilk, son_tekrar, yakin, eski);
	for (int i = 0; i < 100; i++) { snprintf(an, sizeof an, "x%d", i); oturumda_ilk(&tb, an); }
	DENE(O[0].kucuk_n >= 24 && O[0].kucuk_n < 48 && !oturumda_ilk(&tb, "x99"), "v12: halka sayaci sinirli (%d)", O[0].kucuk_n);
	O[0].kucuk_n = 0;
	struct foto_oge l[3], f;
	int n = 0;
	static const long long ts[6] = { 5, 1, 6, 2, 4, 3 };
	for (int i = 0; i < 6; i++) {
		memset(&f, 0, sizeof f);
		snprintf(f.ad, sizeof f.ad, "foto-%lld.jpg", ts[i]);
		f.t = ts[i];
		foto_ekle(l, &n, 3, &f);
	}
	long long top = 0, enk = 99;
	for (int i = 0; i < n; i++) { top += l[i].t; if (l[i].t < enk) enk = l[i].t; }
	DENE(n == 3 && top == 15 && enk == 4, "v12: foto_ekle en yeni 3 (toplam %lld, en kucuk %lld)", top, enk);
	static char pk[300], py[400];
	snprintf(pk, sizeof pk, "%s/proc", sab);
	mkdir(pk, 0755);
	snprintf(py, sizeof py, "%s/123", pk);
	mkdir(py, 0755);
	snprintf(py, sizeof py, "%s/123/comm", pk);
	DENE(!t12_yaz(py, "kamera-kayit-v1\n", 16), "comm yaz");
	snprintf(py, sizeof py, "%s/123/cmdline", pk);
	static const char cl[] = "/mnt/sd/velinux/kamera-kayit-v10\0--x\0";
	DENE(!t12_yaz(py, cl, sizeof cl - 1), "cmdline yaz");
	proc_kok = pk;
	struct hizmet hz[HZ_N];
	hizmet_tara(hz);
	DENE(hz[2].pid == 123 && !strcmp(hz[2].ad, "kamera-kayit-v10"), "v12: cmdline hizmet adi '%s'", hz[2].ad);
	proc_kok = eski_proc;
	unlink(py);
	snprintf(py, sizeof py, "%s/123/comm", pk);
	unlink(py);
	snprintf(py, sizeof py, "%s/123", pk);
	rmdir(py);
	rmdir(pk);
}

/* v13: plan acilisi (bayrak sahibi, elle kapatma isareti, gecersiz saat), oynatma hizi/atlama, okumayan baglanti,
 * oldurulen mikrofon cocugunun bayraklari */
static void test_v13(const char *sab)
{
	static char k13[300], ka[400], ke[400], m13[400], a13[400];
	const char *eski_kok = kok;
	snprintf(k13, sizeof k13, "%s/kamera13", sab);
	kok = k13;
	mkdir(k13, 0755);
	snprintf(ka, sizeof ka, "%s/kayit-acik", k13);
	snprintf(ke, sizeof ke, "%s/kayit-elle-kapali", k13);
	int eski_acik = M.plan_acik, eski_n = M.plan_n;
	struct kural eski_k0 = M.plan[0];
	struct stat st;
	char b[48];
	/* B1: pencere disi acilis */
	M.plan_acik = 1;
	M.plan_n = 0;
	long long d0 = stat(denetim_ram, &st) ? 0 : (long long)st.st_size;
	M.plan_onceki = -1;
	plan_adim();
	long long d1 = stat(denetim_ram, &st) ? 0 : (long long)st.st_size;
	DENE(M.plan_onceki == 0 && !dosya_var(ka) && d1 == d0, "v13: bayrak yokken acilista islem ve denetim satiri yok (%lld/%lld)", d0, d1);
	DENE(!t12_yaz(ka, "", 0), "bos bayrak yaz");
	M.plan_onceki = -1;
	plan_adim();
	DENE(dosya_var(ka) && M.plan_onceki == 0, "v13: elle acilmis (bos) bayrak acilista korunur");
	DENE(!t12_yaz(ka, "plan\n", 5), "plan bayragi yaz");
	M.plan_onceki = -1;
	plan_adim();
	DENE(!dosya_var(ka) && M.plan_onceki == 0, "v13: planin yazdigi bayrak acilista kalkar (denetim v1 #12)");
	/* B5: pencere ici (her gun 00:00-24:00) */
	M.plan[0] = (struct kural){ 0x7f, 0, 1440 };
	M.plan_n = 1;
	DENE(!t12_yaz(ke, "", 0), "isaret yaz");
	M.plan_onceki = -1;
	plan_adim();
	DENE(!dosya_var(ka) && dosya_var(ke) && M.plan_onceki == 1, "v13: elle kapatma isareti acilista yeniden acmayi engeller");
	unlink(ke);
	M.plan_onceki = -1;
	plan_adim();
	DENE(dosya_oku(ka, b, sizeof b) == 4 && !strcmp(b, "plan") && M.plan_onceki == 1, "v13: plan bayragi 'plan' icerikli ('%s')", b);
	elle_kapali_isaret(1);
	DENE(dosya_var(ke), "v13: penceredeyken elle kapatma isaret yazar");
	elle_kapali_isaret(0);
	DENE(!dosya_var(ke), "v13: elle acma isareti siler");
	elle_kapali_isaret(1);
	/* v15 (152): pencerede bayrak baskasinca silinirse plan_adim isareti yazar, elle yeniden acilinca siler */
	unlink(ke);
	unlink(ka);
	plan_adim();
	DENE(dosya_var(ke) && !dosya_var(ka) && M.plan_onceki == 1, "v15: kopru/panel kapatmasi isaret yazar");
	DENE(!t12_yaz(ka, "", 0), "bos bayrak yaz");
	plan_adim();
	DENE(!dosya_var(ke) && dosya_var(ka) && M.plan_onceki == 1, "v15: elle yeniden acma isareti siler");
	elle_kapali_isaret(1);
	M.plan_n = 0;	/* pencere bitti: gercek gecis */
	plan_adim();
	DENE(!dosya_var(ka) && !dosya_var(ke) && M.plan_onceki == 0, "v13: gercek gecis bayragi ve isareti siler");
	elle_kapali_isaret(1);
	DENE(!dosya_var(ke), "v13: pencere disinda isaret yazilmaz");
	/* B6: 2026 oncesi saatte plan adimi atlanir */
	M.plan_n = 1;
	M.plan_onceki = -1;
	double eski_kayma = saat_kayma;
	saat_kayma += 1700000000.0 - simdi_unix();
	plan_adim();
	saat_kayma = eski_kayma;
	DENE(M.plan_onceki == -1 && !dosya_var(ka), "v13: gecersiz saatte plan karar vermez");
	M.plan_acik = eski_acik;
	M.plan_n = eski_n;
	M.plan[0] = eski_k0;
	M.plan_onceki = -1;
	unlink(ka);
	unlink(ke);
	rmdir(k13);
	kok = eski_kok;
	/* B2: oynatma bekleme ve atlama idx ms'den */
	struct kidx kk[6] = { { 0, 1, 0 }, { 0, 1, 500 }, { 0, 1, 1000 }, { 0, 1, 1125 }, { 0, 1, 1250 }, { 0, 1, 9000 } };
	DENE(fabs(oyn_ara(kk, 6, 0, 1) - 0.5) < 1e-9 && fabs(oyn_ara(kk, 6, 2, 1) - 0.125) < 1e-9 &&
	     fabs(oyn_ara(kk, 6, 2, 2) - 0.0625) < 1e-9 && fabs(oyn_ara(kk, 6, 4, 4) - 1.0) < 1e-9 &&
	     fabs(oyn_ara(kk, 6, 5, 1) - 1.0 / KAYIT_FPS) < 1e-9, "v13: kare arasi idx ms'den (8 kare/sn 0.125 sn, en fazla 1 sn)");
	kk[3].ms = -1;
	DENE(fabs(oyn_ara(kk, 6, 2, 4) - 1.0 / KAYIT_FPS / 4) < 1e-9 && fabs(oyn_ara(kk, 6, 3, 1) - 1.0 / KAYIT_FPS) < 1e-9,
	     "v13: ms yoksa 1/KAYIT_FPS");
	kk[3].ms = 1125;
	kk[5].ms = 100;
	DENE(fabs(oyn_ara(kk, 6, 4, 1) - 1.0 / KAYIT_FPS) < 1e-9, "v13: azalan ms -> eski bekleme");
	kk[5].ms = 9000;
	DENE(oyn_bas_kare(kk, 6, 0) == 0 && oyn_bas_kare(kk, 6, 1) == 2 && oyn_bas_kare(kk, 6, 2) == 5 && oyn_bas_kare(kk, 6, 59) == 5,
	     "v13: saniyeye atlama idx ms'den");
	for (int i = 0; i < 6; i++) kk[i].ms = -1;
	DENE(oyn_bas_kare(kk, 6, 2) == 4 && oyn_bas_kare(kk, 6, 30) == 5, "v13: ms yoksa bas*KAYIT_FPS");
	/* A4: cikisi bekleyen ama ilerlemeyen baglanti */
	static struct bag tb13;
	memset(&tb13, 0, sizeof tb13);
	tb13.dosya_fd = -1;
	DENE(!cikis_takildi(&tb13, 100) && tb13.cikis_t == 100, "v13: bekleyen yokken sayac tazelenir");
	tb13.cson = 10;
	DENE(!cikis_takildi(&tb13, 159) && cikis_takildi(&tb13, 161), "v13: 60 sn ilerlemesiz cikis kapanir");
	tb13.kapat_sonra = 1;
	DENE(!cikis_takildi(&tb13, 109) && cikis_takildi(&tb13, 111), "v13: kapat_sonra iken 10 sn");
	tb13.cson = 0;
	DENE(!cikis_takildi(&tb13, 500) && tb13.cikis_t == 500, "v13: cikis bitince sayac tazelenir");
	/* A5: oldurulen mikrofon cocugunun bayraklari yalniz onunsa silinir */
	const char *em = mik_bayrak, *ea = asistan_birak_yol;
	snprintf(m13, sizeof m13, "%s/mik13", sab);
	snprintf(a13, sizeof a13, "%s/birak13", sab);
	mik_bayrak = m13;
	asistan_birak_yol = a13;
	for (int tur = 0; tur < 2; tur++) {
		fflush(stdout);
		pid_t p = fork();
		if (!p) for (;;) pause();
		snprintf(b, sizeof b, "%d\n", tur ? 1 : (int)p);
		bayrak_yaz_icerik(m13, 1, b);
		snprintf(b, sizeof b, "%s %d\n", tur ? "tts" : "kopru", (int)p);
		bayrak_yaz_icerik(a13, 1, b);
		cocuk_birak(p, mik_bayrak);
		for (int i = 0; i < 8; i++) if (OLEN[i].pid == p) OLEN[i].son = simdi() - 1;
		olenleri_denetle();
		waitpid(p, NULL, 0);
		for (int i = 0; i < 8; i++) if (OLEN[i].pid == p) OLEN[i].pid = 0;
		DENE(dosya_var(m13) == tur && dosya_var(a13) == tur, "v13: oldurulen mikrofon cocugunun bayraklari (tur %d)", tur);
		unlink(m13);
		unlink(a13);
	}
	mik_bayrak = em;
	asistan_birak_yol = ea;
}

static int self_test(void)
{
	char sab[] = "/tmp/kole-cm-test-XXXXXX";
	if (!mkdtemp(sab)) return 1;
	static char y1[300], y2[300];
	snprintf(y1, sizeof y1, "%s/denetim.log", sab);
	snprintf(y2, sizeof y2, "%s/mounts", sab);
	denetim_ram = denetim_sd = y1;
	mounts_yol = y2;
	test_kripto();
	test_http();
	test_ws();
	test_json();
	test_oturum();
	test_yollar();
	test_plan();
	test_guvenlik(sab);
	test_ota();	/* v4 */
	test_zaman(sab);	/* v10 */
	test_v12(sab);
	test_v13(sab);
	/* denetim satiri tek satir kalir */
	unlink(y1);
	denetim("1.2.3.4", "sahte\nsatir %s", "x\ry");
	char b[512];
	dosya_oku(y1, b, sizeof b);
	DENE(strstr(b, " 1.2.3.4 sahte satir x y") && !strchr(b, '\r') && !strchr(b, '\n'), "denetim satiri '%s'", b);
	unlink(y1);
	rmdir(sab);
	printf("self-test: %s (%d hata)\n", hs ? "FAIL" : "PASS", hs);
	return hs ? 1 : 0;
}

#ifdef KOLE_TEST
static char test_yollar_b[40][400];
static const char *tyol(int i, const char *k, const char *ek)
{
	snprintf(test_yollar_b[i], sizeof test_yollar_b[i], "%s%s", k, ek);
	return test_yollar_b[i];
}
static void test_kok_ata(const char *k)
{
	web_kok = tyol(0, k, "/mnt/sd/velinux/merkez");
	ayar_yol = tyol(1, k, "/data/kole-web.json");
	kurulum_yol = tyol(2, k, "/data/kole-web-kurulum");
	denetim_sd = tyol(3, k, "/mnt/sd/velinux/denetim.log");
	denetim_ram = tyol(4, k, "/tmp/kole-merkez-denetim.log");
	kayit_durum_yol = tyol(5, k, "/tmp/kamera-kayit.json");
	ts_durum_yol = tyol(6, k, "/tmp/tailscale-durum.json");
	ts_serve_yol = tyol(7, k, "/tmp/tailscale-serve.json");
	ts_ikili_yol = tyol(8, k, "/mnt/sd/velinux/tailscale/tailscaled");
	ts_bayrak_yol = tyol(9, k, "/mnt/sd/velinux/tailscale-acik");
	merkez_durum_yol = tyol(10, k, "/tmp/kole-cihaz-merkez.json");
	hop_dogrulandi_yol = tyol(11, k, "/data/hoparlor-dogrulandi");
	rtsp_bayrak_m = tyol(12, k, "/mnt/sd/velinux/rtsp-acik");
	rtsp_parola_m = tyol(13, k, "/data/rtsp-parola");
	rtsp_durum_m = tyol(14, k, "/tmp/rtsp-durum.json");
	kok = tyol(15, k, "/mnt/sd/kamera");
	sd_kok = tyol(16, k, "/mnt/sd");
	kalkan_yol = tyol(17, k, "/tmp/gizlilik-kalkani");
	kamera_bayrak = tyol(18, k, "/tmp/kamera-aktif");
	mik_bayrak = tyol(19, k, "/tmp/mik-aktif");
	hoparlor_bayrak = tyol(20, k, "/tmp/hoparlor-aktif");
	snd_kok = tyol(21, k, "/snd");
	kamera_son = tyol(22, k, "/tmp/kamera-son.jpg");
	mounts_yol = tyol(23, k, "/mounts");
	ota_kok = tyol(24, k, "/mnt/sd/velinux");	/* v4 */
	panel_durum_yol = tyol(25, k, "/tmp/pc-ekran-durum.json");	/* v5 */
	tg_jeton_yol = tyol(26, k, "/data/asistan/telegram-jeton");
	tg_ofs_yol = tyol(27, k, "/data/asistan/telegram-offset");
	gemini_yol = tyol(28, k, "/data/asistan/gemini-anahtar");
	asistan_kapali_yol = tyol(29, k, "/data/asistan/kapali");
	cerceve_bayrak_yol = tyol(30, k, "/data/panel-cerceve");
	foto_dizin = tyol(31, k, "/mnt/sd/fotograf");
	yb_isaret_yol = tyol(32, k, "/tmp/yeniden-baslat-istendi");	/* v9 */
	wifi_isaret_yol = tyol(33, k, "/data/.wifi-deneme");
	asistan_birak_yol = tyol(34, k, "/tmp/asistan-mik-birak");	/* v13 */
	gemini_dizin = tyol(35, k, "/mnt/sd/velinux/asistan");	/* v18 */
	hatirlat_yol = tyol(36, k, "/data/asistan/hatirlatici.txt");
	eylem_yol = tyol(37, k, "/tmp/asistan-eylem.json");
	bt_durum_yol = "/nonexistent/bt";
	kayit_pid = "/nonexistent/pid";
	ses_sahte = 1;
	test_kamera_dosya = 1;
}
#endif

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IOLBF, 0);
	if (argc == 2 && !strcmp(argv[1], "--self-test")) return self_test();
	if (argc == 2 && !strcmp(argv[1], "--pbkdf2-olc")) return pbkdf2_olc();
	durum_hazirla();
	int parola_kipi = 0;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--port") && i + 1 < argc) M.port = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--lan")) M.lan = 1;
		else if (!strcmp(argv[i], "--host") && i + 1 < argc && M.izinli_n < 4) {
			snprintf(M.izinli_host[M.izinli_n], sizeof M.izinli_host[0], "%s", argv[++i]);
			for (char *q = M.izinli_host[M.izinli_n]; *q; q++) *q = (char)tolower((unsigned char)*q);
			M.izinli_n++;
		} else if (!strcmp(argv[i], "parola-ayarla")) parola_kipi = 1;
		else if (!strcmp(argv[i], "--pbkdf2-tur") && i + 1 < argc) {
			long t = atol(argv[++i]);
#ifdef KOLE_TEST
			long alt = 1000;
#else
			long alt = 20000;	/* cihazda daha dusugu kabul edilmez */
#endif
			if (t < alt || t > 5000000) { fprintf(stderr, "--pbkdf2-tur %ld..5000000\n", alt); return 2; }
			pbkdf2_tur_yeni = (uint32_t)t;
		}
#ifdef KOLE_TEST
		else if (!strcmp(argv[i], "--test-kok") && i + 1 < argc) test_kok_ata(argv[++i]);
#endif
		else { fprintf(stderr, "kullanim: %s [--port N] [--lan] [--host AD] [--pbkdf2-tur N] | [--pbkdf2-tur N] parola-ayarla | --self-test | --pbkdf2-olc\n", argv[0]); return 2; }
	}
	if (M.port < 1 || M.port > 65535) return 2;
	if (parola_kipi) return parola_ayarla_kipi();
	int port = M.port, lan = M.lan, hn = M.izinli_n;
	char hosts[4][128];
	memcpy(hosts, M.izinli_host, sizeof hosts);
	durum_hazirla();
	M.port = port;
	M.lan = lan;
	M.izinli_n = hn;
	memcpy(M.izinli_host, hosts, sizeof hosts);
	return sunucu();
}
