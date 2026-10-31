/* kamera_kayit_v13 (25.10.2026, Claude; scripts/645, inceleme B11): v12 + kayit karari kayit_durum_guncelle /
 *   kayit_kare_karari fonksiyonlarinda (ana dongu ve self-test ayni kod). Davranis v12 ile ayni. */
/* kamera_kayit_v12 (24.10.2026, Claude; denetim-2026-10-24-v3 B9, A18e; scripts/632): v11 + kayit esigi saniyede bir
 *   okunur; BOS canli bayragi (kole-kopru KAMERA 1) 60 sn okunmazsa silinir (PC KAMERA 0 gondermeden giderse kamera
 *   sonsuza dek canli kaliyordu). Kamera komutlari v11 ile ayni. */
/* kamera_kayit_v11 (24.10.2026, Claude; OTONOM 115, kullanıcı: hareket varken 8, yoksa 2 kare/sn; scripts/618): v10 + kayit
 *   sirasinda bekci ile ayni hareket puani (kare boyutu penceresi, ~2 ornek/sn, bekci-esik); esik asilinca KAYIT_HAREKET_SN (20)
 *   sn KAYIT_HAREKET_FPS (8) kare/sn. Hizli kayitta dongu sonu uykusu yok. Durum: kayit_fps (o anki), kayit_hareket.
 *   KAYIT_HAREKET_FPS=0 v10 davranisi. Kamera komutlari v10 ile ayni. */
/* kamera_kayit_v10 (23.10.2026, Claude; docs/denetim-2026-10-23-v2.md #2/#3/#8/#12/#14/#15/#16): v9 +
 *   - bekci/telegram fotosu /tmp'ye KOPYALANMAZ: kuyruga SD yolu yazilir (tuketici yalniz /tmp yolunu siler);
 *     kuyrukta 50'den fazla .txt varsa yeni bildirim eklenmez (bir kez gunluge).
 *   - yil < 2026 (NTP yok, 1970) iken kayit parcasi, time-lapse karesi ve olay csv'si YAZILMAZ; durum "saat gecersiz"
 *     (saat_sektoru ile ayni 2026..2099 kurali).
 *   - kayit gostergesi /tmp/kamera-aktif kamera-kayit yarattiysa icerik "kayit\n": yalniz gosterge (v8 yavas kayit yolu,
 *     her kayit karesi /tmp/kamera-son.jpg'ye -> canli ~2 kare/sn); kayit kapaninca YALNIZ bu isaretli bayrak silinir.
 *     Baska icerik (bos, "merkez ..", "telegram ..", "panel ..", "rtsp") = gercek canli. Izleyiciler bayrak varken
 *     kendininkini yazmadigi icin: isaretli bayrakta son kare okunduysa (tmpfs relatime, atime > mtime) 5 sn tam hiz.
 *   - fotolar/ temizligi zaman damgasina gore (alphasort tum bekci-*'yi foto-*'dan once siliyordu).
 *   - gecerli kare yoksa 100 ms uyku; yarim kalan kare yazmasinda parca kapatilir (yeniden acilista gercek son ofset).
 *   - stdout gunlugu 256 KB'i asinca ftruncate (kabuk O_APPEND acar); ilk hareket alarmi acilistan 120 sn beklemez.
 *   Kamera komutlari v9 ile ayni (scripts/600).
 * kamera_kayit v9 (22.10.2026, Claude): v8 + TIME-LAPSE 1..3600 sn, boyut siniri (zaman-sinir-mb, vars. 4096; en eski
 *   gun silinir; bugun dolarsa durur) ve OLAY TESPITI (zaman-olay: bekci ornekleme -> zaman/<gun>-olay.csv "HHMM,skor,ofs",
 *   esik ustunde ek kare, Telegram uyarisi). Kamera komutlari v8 ile ayni (scripts/540).
 * kamera_kayit v8 (21.10.2026, Claude): v7 + canli yokken kare yalniz gerektiginde okunur (okuma_bekle; kayitta ~8 -> 2
 *   USB okuma/sn). Kamera komutlari v7 ile ayni (scripts/457).
 * kamera_kayit v7 (15.10.2026, Claude): v6 + saat esitlendiyse foto aciklamasinda not yok (scripts/236).
 * kamera_kayit v6 (15.10.2026, Claude): v5 + USB BOT kurtarma (authorized 0/1: acilista, 2 ardisik hata, saat
 *   yazma hatasi) + SIGTERM ile temiz cikis (scripts/233).
 * kamera_kayit v5 (15.10.2026, Claude): v4 + KAMERA SAATI: kameraya tek izinli yazma = Melis saat komutu (0x13),
 *   USB acilinca + 6 saatte bir + /tmp/kamera-saat-ayarla (scripts/231). Baska kamera yazmasi YOK.
 * kamera_kayit v4 (15.10.2026, Claude): v3 + Telegram fotograf aciklamasinda cihaz saati (scripts/229).
 * kamera_kayit v3 (15.10.2026, Claude): v2 + BEKCI (hareket -> Telegram fotograf), FOTOGRAF CEK, ZAMAN ATLAMALI (scripts/221).
 *   bayraklar: /mnt/sd/kamera/bekci-acik, /tmp/kamera-foto-iste, /mnt/sd/kamera/zaman-acik (gizlilik kalkani hepsini bastirir)
 * kamera_kayit v2 (13.10.2026, Claude): v1 + parca yan dizini ve ucuz olay puani. Kamera erisimi v1 ile BIREBIR
 * ayni: usbdevfs, YALNIZ SCSI READ(10) (0x28). Kameraya hicbir komut/veri YAZILMAZ; FORMAT/SIL/0x46/ISP yok.
 * Yeni (yalniz SD karta, kayit acikken):
 *   YYYYMMDD/HHMM.idx  "KIDX1" + her kare icin "<ofs> <len> <ms>" (ms: dakika icindeki ms). KOLE Merkez/kole-kopru-v7
 *                      bununla parca icinde aninda atlar, tek kare (kucuk resim) okur, tum dosyayi taramaz.
 *   YYYYMMDD/olay.csv  parca kapaninca "HHMM,skor,kare,bayt,tepe,tepe_ms". skor 0..1000 = kare BOYUTU farklarindan
 *                      hareket puani (JPEG cozulmez, CPU ~0). Ayrinti kamera_olay_v1.h.
 *   Kare uzunlugu artik JPEG segmentleri izlenerek kesin bulunur (v1: sondan FF D9 arama; yedek olarak duruyor).
 *   Saklama .mjpg ile birlikte .idx'i de siler; gunde .mjpg kalmazsa olay.csv silinip klasor kaldirilir.
 *   /tmp/gizlilik-kalkani varken (panel 0.22 Gizlilik kalkani) ne canli ne kayit: USB birakilir.
 *   Durum JSON'u: surum kamera-kayit-v2, son_skor, kalkan, idx.
 * Calistirma: kamera-kayit [--self-test | --durum-ornek | --indeksle]
 *   --indeksle : yan dizini olmayan eski parcalar icin .idx + olay.csv satiri uretir (yalniz SD; kamera acilmaz).
 * --- v1 ---
 * kamera_kayit v1 (12.10.2026, Claude): Generalplus 1b3f:8301 kamera modulunun TEK sahibi (usbdevfs, YALNIZ READ(10)).
 *   canli : /tmp/kamera-aktif varken her kare /tmp/kamera-son.jpg'ye (atomik rename) yazilir (~8 kare/sn)
 *   kayit : /mnt/sd/kamera/kayit-acik varken KAYIT_FPS (2) kare/sn, 1 dk'lik parcalar /mnt/sd/kamera/YYYYMMDD/HHMM.mjpg
 *           Saklama: en fazla 360 parca (6 saat) ve en az 1 GB bos alan; asilirsa en eski parca silinir.
 *   durum : /tmp/kamera-kayit.json (her saniye). Bayrak yoksa USB birakilir, CPU harcanmaz. */
#define _GNU_SOURCE
#include <dirent.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <linux/usbdevice_fs.h>
#include "kamera_olay_v1.h"

#define ELENE 4351u
#define YUVA 600u
#define YUVA_SAYI 8u
#define ASGARI_BOS_MB 1024
static double KAYIT_FPS = 2.0;
static int azami_parca = 360;

/* yollar: self-test gecici klasore yonlendirir */
static const char *kok = "/mnt/sd/kamera";
static const char *sd_kok = "/mnt/sd";
static const char *kalkan_yol = "/tmp/gizlilik-kalkani";
static const char *canli_bayrak = "/tmp/kamera-aktif";
static const char *son_kare = "/tmp/kamera-son.jpg";	/* v10: test gecici klasore alir */
static const char *kuyruk_dz = "/tmp/bildirim-kuyruk";	/* v10 */
#define KAYIT_ISARET "kayit\n"	/* v10: kamera-kayit'in yarattigi kamera-aktif (yalniz gosterge) */
#define KUYRUK_AZAMI 50	/* v10: kuyrukta bundan fazla .txt varsa kendi en eski ogemiz silinir (inceleme) */
#define GUNLUK_AZAMI (256 * 1024)	/* v10: stdout gunlugu */
/* v3 */
#define BEKCI_PENCERE 6
static int bekci_esik = 120, zaman_aralik = 60;
/* v9: time-lapse boyut siniri + olay tespiti */
#define ZAMAN_DOSYA_AZAMI (3900LL * 1048576)	/* FAT32 tek dosya 4 GiB: payli */
static int zaman_sinir_mb = 4096, zaman_dolu, zaman_olay;
static long long zaman_bayt = -1;
static double son_olay_ek = -1e9, son_zaman_denetim = -1e9;
static char zo_gun[16];
static int zo_dk = -1, zo_skor = -1, zo_ofs_skor = -1;
static long long zo_ofs = -1;
static const char *zaman_olay_yol(void);	/* asagida */
static double bekci_bas = 0, son_alarm = -1e9, son_zaman = 0, son_bekci_kare = 0;	/* v10: son_alarm -1e9 (ilk alarm acilista) */
static uint32_t bekci_boy[BEKCI_PENCERE];
static double KAYIT_HAREKET_FPS = 8.0, KAYIT_HAREKET_SN = 20.0;	/* v11: hareket varken kayit hizi ve suresi */
static double hareket_bitis;	/* v11: mono; bu ana kadar hizli kayit */
static uint32_t kh_boy[BEKCI_PENCERE];	/* v11: kayit hareket penceresi (bekciden bagimsiz) */
static int kh_n, kayit_esik = 120;

/* v11: kayit sirasinda hareket (bekci ile ayni kayan pencere ve puan, ~2 ornek/sn); 1 = esik asildi */
static int kayit_hareket(uint32_t boy, int esik)
{
	if (kh_n < BEKCI_PENCERE) kh_boy[kh_n++] = boy;
	else { memmove(kh_boy, kh_boy + 1, (BEKCI_PENCERE - 1) * sizeof *kh_boy); kh_boy[BEKCI_PENCERE - 1] = boy; }
	if (kh_n < BEKCI_PENCERE) return 0;
	int tepe;
	return olay_skor(kh_boy, kh_n, &tepe) >= esik;
}

/* v11: o anki kayit hizi (hareketten sonra KAYIT_HAREKET_SN boyunca hizli) */
static double kayit_hizi(double t)
{
	return KAYIT_HAREKET_FPS > KAYIT_FPS && t < hareket_bitis ? KAYIT_HAREKET_FPS : KAYIT_FPS;
}

/* v13 (inceleme B11): kayit karari ana dongu ve self-test icin tek kod. Saatler mono (okuma basi). */
struct kayit_saat { double son_kayit, son_ornek; int onceki; };
static int sayi_oku(const char *yol, int varsayilan, int en_az, int en_cok);	/* asagida */

/* dongu basi: kayit basinda pencere ve esik yenilenir (v11); kayit yokken hizli kip biter */
static void kayit_durum_guncelle(struct kayit_saat *s, int kayit)
{
	if (kayit && !s->onceki) {
		kh_n = 0;
		kayit_esik = sayi_oku("/mnt/sd/kamera/bekci-esik", 120, 20, 1000);
	}
	if (!kayit) hareket_bitis = 0;
	s->onceki = kayit;
}

/* okunan her kayit karesi: ~2 ornek/sn hareket puani hizli kipi uzatir (v11); 1 = bu kare yazilmali (v8: 1/hiz) */
static int kayit_kare_karari(struct kayit_saat *s, double kt, uint32_t boy)
{
	if (KAYIT_HAREKET_FPS > KAYIT_FPS && kt - s->son_ornek >= 0.495) {
		s->son_ornek = kt;
		if (kayit_hareket(boy, kayit_esik)) {
			if (kt >= hareket_bitis) printf("kayit: hareket, %.0f kare/sn (%.0f sn)\n", KAYIT_HAREKET_FPS, KAYIT_HAREKET_SN);
			hareket_bitis = kt + KAYIT_HAREKET_SN;
		}
	}
	return kt - s->son_kayit >= 1.0 / kayit_hizi(kt) - 0.005;
}
static int bekci_n, bekci_skor = -1;
static long son_alarm_unix;
static const char *bekci_yol(void);	/* asagida */
static const char *zaman_yol(void);

static int ufd = -1;
static uint32_t uetiket = 0x4b4b5932;
static uint8_t b[YUVA * 512];

static double mono(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec / 1e9;
}

/* gercek saat (ms); self-test sabit bir zaman verir */
static long long test_ms = -1;
static long long gercek_ms(void)
{
	if (test_ms >= 0) return test_ms;
	struct timespec t;
	clock_gettime(CLOCK_REALTIME, &t);
	return (long long)t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

static int var(const char *p)
{
	struct stat st;
	return !stat(p, &st);
}

/* v10: dosya adlari duvar saatinden; 2026 oncesi (NTP yok, 1970) gun klasorleri saklamaya gorunmez (gun_gecerli) */
static int yil_gecerli(int yil) { return yil >= 2026 && yil <= 2099; }
static int saat_gecerli(void)
{
	time_t t = (time_t)(gercek_ms() / 1000);
	struct tm tm;
	localtime_r(&t, &tm);
	return yil_gecerli(tm.tm_year + 1900);
}

/* ---------------- kamera: v1 ile BIREBIR (yalniz READ(10)) ---------------- */
static int ubulk(unsigned ep, void *v, unsigned n)
{
	struct usbdevfs_bulktransfer t = { .ep = ep, .len = n, .timeout = 3000, .data = v };
	int r = ioctl(ufd, USBDEVFS_BULK, &t);
	if (r < 0 && errno == EPIPE) {
		unsigned e = ep;
		ioctl(ufd, USBDEVFS_CLEAR_HALT, &e);
	}
	return r;
}

static int uoku(uint32_t lba, unsigned k, uint8_t *veri)	/* YALNIZ READ(10) */
{
	unsigned n = k * 512;
	uint8_t cbw[31] = { 'U', 'S', 'B', 'C' };
	uetiket++;
	memcpy(cbw + 4, &uetiket, 4);
	memcpy(cbw + 8, &n, 4);
	cbw[12] = 0x80;
	cbw[14] = 10;
	uint8_t cb[10] = { 0x28, 0, (uint8_t)(lba >> 24), (uint8_t)(lba >> 16), (uint8_t)(lba >> 8), (uint8_t)lba, 0, 0, (uint8_t)k, 0 };
	memcpy(cbw + 15, cb, 10);
	if (ubulk(0x02, cbw, 31) != 31) return -1;
	int got = ubulk(0x81, veri, n);
	uint8_t csw[13];
	int c = ubulk(0x81, csw, 13);
	if (c < 0 && errno == EPIPE) c = ubulk(0x81, csw, 13);
	if (c != 13 || memcmp(csw, "USBS", 4) || csw[12]) return -1;
	return got;
}

static int uac(void)
{
	char yol[64];
	int bus = 0, dev = 0;
	FILE *f = fopen("/sys/bus/usb/devices/1-1/busnum", "r");
	if (!f || fscanf(f, "%d", &bus) != 1) { if (f) fclose(f); return -1; }
	fclose(f);
	f = fopen("/sys/bus/usb/devices/1-1/devnum", "r");
	if (!f || fscanf(f, "%d", &dev) != 1) { if (f) fclose(f); return -1; }
	fclose(f);
	snprintf(yol, sizeof yol, "/dev/bus/usb/%03d/%03d", bus, dev);
	ufd = open(yol, O_RDWR | O_CLOEXEC);
	if (ufd < 0) return -1;
	unsigned a = 0;
	if (ioctl(ufd, USBDEVFS_CLAIMINTERFACE, &a)) { close(ufd); ufd = -1; return -1; }
	return 0;
}

static void ukapat(void)
{
	unsigned a = 0;
	if (ufd >= 0) { ioctl(ufd, USBDEVFS_RELEASEINTERFACE, &a); close(ufd); }
	ufd = -1;
}

/* v6: BOT kurtarma. Kameraya komut gondermez: cekirdek aygiti yetkisizlestirip yeniden yapilandirir. */
static volatile sig_atomic_t cik_iste;
static void cik_sinyal(int s) { (void)s; cik_iste = 1; }

static void usb_sifirla(const char *neden)
{
	ukapat();
	FILE *f = fopen("/sys/bus/usb/devices/1-1/authorized", "w");
	if (!f) { printf("usb sifirla (%s): authorized acilamadi\n", neden); return; }
	fputs("0\n", f);
	fclose(f);
	usleep(1500000);
	f = fopen("/sys/bus/usb/devices/1-1/authorized", "w");
	if (f) { fputs("1\n", f); fclose(f); }
	usleep(3500000);
	printf("usb sifirla (%s): yeniden yapilandirildi\n", neden);
}

/* yuvadaki gecerli karenin JPEG uzunlugu: once segment izleme (kesin), olmazsa v1 sondan FF D9 kirpma */
static long kare_uzunluk(const uint8_t *j, uint32_t sz)
{
	long e = jpeg_son_c(j, sz);
	if (e > 0) return e;
	uint32_t n = sz;
	while (n > 2 && !(j[n - 2] == 0xff && j[n - 1] == 0xd9)) n--;
	return n > 2 ? (long)n : (long)sz;
}

/* bir sonraki yuvayi sirayla oku (v1 ile ayni); gecerli kare varsa JPEG uzunlugu (b+17), yoksa 0, hata -1 */
static unsigned yuva;
static long kare_al(void)
{
	uint32_t l = ELENE + yuva * YUVA, sz = 0;
	unsigned okunan = 0;
	int gecerli = 0;
	yuva = (yuva + 1) % YUVA_SAYI;
	while (okunan < YUVA) {
		unsigned k = YUVA - okunan < 32 ? YUVA - okunan : 32;
		if (uoku(l + okunan, k, b + okunan * 512) != (int)(k * 512)) return -1;
		if (!okunan) {
			sz = b[0] | b[1] << 8 | b[2] << 16 | (uint32_t)b[3] << 24;
			gecerli = sz >= 1024 && sz <= 256000 && b[17] == 0xff && b[18] == 0xd8;
		}
		okunan += k;
		if (gecerli && okunan * 512 >= sz + 17) break;
	}
	if (!gecerli) return 0;
	return kare_uzunluk(b + 17, sz);
}

/* ---------------- v5: KAMERA SAATI (kameraya TEK izinli yazma) ----------------
 * Melis wcm saat esitleme (cmd 0x13, len 6): "55 aa 13 00 00 00 06 00 | 00 00 | YY MM DD hh mm ss" (YY = yil-2000),
 * sektorun kalani sifir. Baska hicbir kamera komutu yazilmaz. */
static long saat_ayar_unix;	/* son basarili gonderim */
static double saat_ayar_mono = -1e9;
static const char *saat_hata = "";

static int saat_sektoru(uint8_t s[512], const struct tm *tm)
{
	if (!yil_gecerli(tm->tm_year + 1900)) return -1;	/* v10: kayit adlari ile ayni kural */
	memset(s, 0, 512);
	static const uint8_t bas[8] = { 0x55, 0xaa, 0x13, 0x00, 0x00, 0x00, 0x06, 0x00 };
	memcpy(s, bas, 8);
	s[10] = (uint8_t)(tm->tm_year + 1900 - 2000);
	s[11] = (uint8_t)(tm->tm_mon + 1);
	s[12] = (uint8_t)tm->tm_mday;
	s[13] = (uint8_t)tm->tm_hour;
	s[14] = (uint8_t)tm->tm_min;
	s[15] = (uint8_t)(tm->tm_sec > 59 ? 59 : tm->tm_sec);
	return 0;
}

static int saat_sektoru_dogru(const uint8_t *s)
{
	static const uint8_t bas[8] = { 0x55, 0xaa, 0x13, 0x00, 0x00, 0x00, 0x06, 0x00 };
	if (memcmp(s, bas, 8) || s[8] || s[9]) return 0;
	if (s[10] < 26 || s[10] > 99 || s[11] < 1 || s[11] > 12 || s[12] < 1 || s[12] > 31 || s[13] > 23 || s[14] > 59 || s[15] > 59) return 0;
	for (int i = 16; i < 512; i++) if (s[i]) return 0;
	return 1;
}

static int saat_lba_dogru(uint32_t lba)
{
	return lba >= ELENE && lba < ELENE + YUVA * YUVA_SAYI && (lba - ELENE) % YUVA == 0;
}

/* WRITE10, 1 blok: YALNIZ dogrulanmis saat sektoru, YALNIZ ELENE yuva basina */
static int uyaz_saat(uint32_t lba, const uint8_t *s)
{
	if (!saat_sektoru_dogru(s) || !saat_lba_dogru(lba)) return -2;
	unsigned n = 512;
	uint8_t cbw[31] = { 'U', 'S', 'B', 'C' };
	uetiket++;
	memcpy(cbw + 4, &uetiket, 4);
	memcpy(cbw + 8, &n, 4);
	cbw[12] = 0x00;	/* OUT */
	cbw[14] = 10;
	uint8_t cw[10] = { 0x2a, 0, (uint8_t)(lba >> 24), (uint8_t)(lba >> 16), (uint8_t)(lba >> 8), (uint8_t)lba, 0, 0, 1, 0 };
	memcpy(cbw + 15, cw, 10);
	if (ubulk(0x02, cbw, 31) != 31) return -1;
	uint8_t kopya[512];
	memcpy(kopya, s, 512);
	if (ubulk(0x02, kopya, 512) != 512) return -1;
	uint8_t csw[13];
	int c = ubulk(0x81, csw, 13);
	if (c < 0 && errno == EPIPE) c = ubulk(0x81, csw, 13);
	if (c != 13 || memcmp(csw, "USBS", 4) || csw[12]) return -1;
	return 0;
}

/* USB acikken cagrilir: sistem saati gecerliyse kamerayi esitler (siradaki yuva, Melis gibi) */
static void saat_esitle(const char *neden)
{
	time_t t = (time_t)(gercek_ms() / 1000);
	struct tm tm;
	uint8_t s[512];
	localtime_r(&t, &tm);
	saat_ayar_mono = mono();
	if (saat_sektoru(s, &tm)) { saat_ayar_mono = mono() - 6 * 3600.0 + 60; saat_hata = "sistem saati gecersiz"; printf("kamera saati: atlandi (%s)\n", saat_hata); return; }
	uint32_t lba = ELENE + yuva * YUVA;
	yuva = (yuva + 1) % YUVA_SAYI;
	int r = uyaz_saat(lba, s);
	if (r) {
		saat_ayar_mono = mono() - 6 * 3600.0 + 60;
		saat_hata = r == -2 ? "dogrulama reddetti" : "USB yazma hatasi";
		printf("kamera saati: HATA %s (LBA %u)\n", saat_hata, lba);
		if (r == -1) usb_sifirla("saat yazma hatasi");	/* v6: BOT yarim kalmasin */
		return;
	}
	saat_ayar_unix = (long)t;
	saat_hata = "";
	printf("kamera saati ayarlandi (%s): 20%02u-%02u-%02u %02u:%02u:%02u LBA %u\n", neden, s[10], s[11], s[12], s[13], s[14], s[15], lba);
}

/* v10: son kare yazildiktan sonra okundu mu (relatime: ilk okuma atime'i simdiye tasir; yazma mtime'i atime'in ustunde birakir) */
static int son_kare_okundu(void)
{
	struct stat st;
	if (stat(son_kare, &st)) return 0;
	return st.st_atim.tv_sec > st.st_mtim.tv_sec || (st.st_atim.tv_sec == st.st_mtim.tv_sec && st.st_atim.tv_nsec > st.st_mtim.tv_nsec);
}
static double izlendi_t = -1e9;
static double bos_bayrak_t;	/* v12: bos bayragin ilk goruldugu an (0 = yok) */

static void son_kare_yaz(const uint8_t *j, long n)
{
	if (son_kare_okundu()) izlendi_t = mono();	/* inceleme: eski kare yenisi gelmeden once okunduysa izleyici var */
	char t[300];
	snprintf(t, sizeof t, "%s.tmp", son_kare);
	int fd = open(t, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd < 0) return;
	ssize_t w = write(fd, j, (size_t)n);
	close(fd);
	if (w == n) rename(t, son_kare);
}

/* ---------------- saklama ---------------- */
static int ad_karsilastir(const void *a, const void *c)
{
	return strcmp(*(char *const *)a, *(char *const *)c);
}

/* v8: canli yokken bir sonraki kare okumasina kalan sure (sn); 0 = simdi oku. Kayit 1/fps, bekci 0.5 sn, zaman atlamali
 * aralik; en cok 0.2 sn (bayraklar yine 0.2 sn'de bir okunur). v7 50 ms'de bir okuyordu (kayitta ~8 okuma/sn, 2 yazim). */
static double okuma_bekle(double t, int kayit, double son_kayit, double fps, int bekci, double son_bekci, int zaman,
			  double son_zaman, int aralik)
{
	double s = t + 0.2;
	if (kayit && fps > 0 && son_kayit + 1.0 / fps < s) s = son_kayit + 1.0 / fps;
	if (bekci && son_bekci + 0.5 < s) s = son_bekci + 0.5;
	if (zaman && son_zaman + aralik < s) s = son_zaman + aralik;
	return s > t ? s - t : 0;
}

static int mjpg_adi(const char *d)
{
	return strlen(d) == 9 && !strcmp(d + 4, ".mjpg") && strspn(d, "0123456789") == 4;
}

/* tum parcalari "YYYYMMDD/HHMM.mjpg" olarak sirali listele; donus sayi, *liste malloc */
static int parcalar(char ***liste, unsigned long long *toplam)
{
	int n = 0, kap = 0;
	char **l = NULL;
	*toplam = 0;
	DIR *d = opendir(kok);
	struct dirent *e;
	while (d && (e = readdir(d))) {
		if (!gun_gecerli(e->d_name)) continue;
		char gun[512];
		snprintf(gun, sizeof gun, "%s/%s", kok, e->d_name);
		DIR *d2 = opendir(gun);
		struct dirent *e2;
		while (d2 && (e2 = readdir(d2))) {
			if (!mjpg_adi(e2->d_name)) continue;
			if (n == kap) { kap = kap ? kap * 2 : 512; l = realloc(l, (size_t)kap * sizeof *l); }
			l[n] = malloc(20);
			snprintf(l[n], 20, "%.8s/%.9s", e->d_name, e2->d_name);
			char tam[600];
			struct stat st;
			snprintf(tam, sizeof tam, "%s/%s", gun, e2->d_name);
			if (!stat(tam, &st)) *toplam += (unsigned long long)st.st_size;
			n++;
		}
		if (d2) closedir(d2);
	}
	if (d) closedir(d);
	if (n) qsort(l, (size_t)n, sizeof *l, ad_karsilastir);
	*liste = l;
	return n;
}

static long long bos_mb(void)
{
	struct statvfs v;
	if (statvfs(sd_kok, &v)) return -1;
	return (long long)v.f_bavail * (long long)v.f_frsize / (1024 * 1024);
}

/* gun klasorunde .mjpg kalmadiysa yan dosyalari (olay.csv, artik .idx) silip klasoru kaldir */
static void gun_temizle(const char *gun8)
{
	char gun[512], tam[600];
	snprintf(gun, sizeof gun, "%s/%.8s", kok, gun8);
	DIR *d = opendir(gun);
	struct dirent *e;
	int mjpg = 0;
	while (d && (e = readdir(d)))
		if (mjpg_adi(e->d_name)) mjpg = 1;
	if (d) closedir(d);
	if (mjpg) return;
	d = opendir(gun);
	while (d && (e = readdir(d))) {
		size_t L = strlen(e->d_name);
		if (!strcmp(e->d_name, "olay.csv") || (L == 8 && !strcmp(e->d_name + 4, ".idx") && strspn(e->d_name, "0123456789") == 4)) {
			snprintf(tam, sizeof tam, "%s/%s", gun, e->d_name);
			unlink(tam);
		}
	}
	if (d) closedir(d);
	rmdir(gun);
}

static int son_sayi;
static unsigned long long son_toplam;
static void sakla(const char *simdiki)
{
	char **l;
	unsigned long long toplam;
	int n = parcalar(&l, &toplam);
	int i = 0;
	while (i < n && (n - i > azami_parca || (bos_mb() >= 0 && bos_mb() < ASGARI_BOS_MB))) {
		if (strcmp(l[i], simdiki)) {
			char tam[600];
			snprintf(tam, sizeof tam, "%s/%s", kok, l[i]);
			struct stat st;
			if (!stat(tam, &st)) toplam -= (unsigned long long)st.st_size;
			unlink(tam);
			strcpy(tam + strlen(tam) - 5, ".idx");	/* HHMM.mjpg -> HHMM.idx */
			unlink(tam);
			gun_temizle(l[i]);
			printf("saklama: %s silindi\n", l[i]);
		}
		i++;
	}
	son_sayi = n - i;
	son_toplam = toplam;
	for (int j = 0; j < n; j++) free(l[j]);
	free(l);
}

/* ---------------- parca + yan dizin ---------------- */
static FILE *parca, *parca_idx;
static char parca_ad[20];
static uint64_t parca_ofs;
static uint32_t parca_len[OLAY_AZAMI_KARE];
static int32_t parca_ms[OLAY_AZAMI_KARE];
static int parca_n;
static int son_skor = -1;
static struct kidx gecici[OLAY_AZAMI_KARE];

/* olay.csv'ye parca ozeti ekle */
static void olay_yaz(const char *ad, const uint32_t *len, const int32_t *ms, int n)
{
	if (n <= 0) return;
	int tepe = 0;
	int skor = olay_skor(len, n, &tepe);
	unsigned long long bayt = 0;
	for (int i = 0; i < n; i++) bayt += len[i];
	char yol[600];
	snprintf(yol, sizeof yol, "%s/%.8s/olay.csv", kok, ad);
	FILE *f = fopen(yol, "a");
	if (!f) return;
	fprintf(f, "%.4s,%d,%d,%llu,%d,%d\n", ad + 9, skor, n, bayt, tepe, ms ? ms[tepe] : -1);
	fclose(f);
	son_skor = skor;
}

static void parca_kapat(void)
{
	if (parca) {
		fclose(parca);
		parca = NULL;
		olay_yaz(parca_ad, parca_len, parca_ms, parca_n);
	}
	if (parca_idx) { fclose(parca_idx); parca_idx = NULL; }
	parca_n = 0;
}

static void idx_yolu(char *yol, size_t n, const char *ad)
{
	snprintf(yol, n, "%s/%.13s.idx", kok, ad);	/* YYYYMMDD/HHMM */
}

/* yeni (ya da yeniden acilan) parca: ayni dakika icin onceki kareleri yan dizinden (yoksa taramayla) devral */
static int parca_ac(const char *ad)
{
	char gun[512], tam[600], iy[600];
	snprintf(gun, sizeof gun, "%s/%.8s", kok, ad);
	mkdir(kok, 0755);
	mkdir(gun, 0755);
	snprintf(tam, sizeof tam, "%s/%s", kok, ad);
	parca = fopen(tam, "ab");
	if (!parca) return -1;
	fseek(parca, 0, SEEK_END);
	parca_ofs = (uint64_t)ftell(parca);
	snprintf(parca_ad, sizeof parca_ad, "%s", ad);
	parca_n = 0;
	idx_yolu(iy, sizeof iy, ad);
	int idx_var = var(iy);
	if (parca_ofs > 0) {
		int n = idx_var ? idx_oku(iy, parca_ofs, gecici, OLAY_AZAMI_KARE) : 0;
		if (!idx_var) {
			int fd = open(tam, O_RDONLY | O_CLOEXEC);
			static uint8_t t[300 * 1024];
			if (fd >= 0) { n = idx_tara(fd, parca_ofs, gecici, OLAY_AZAMI_KARE, t, sizeof t); close(fd); }
		}
		for (int i = 0; i < n; i++) { parca_len[i] = gecici[i].len; parca_ms[i] = gecici[i].ms; }
		parca_n = n;
		if (!idx_var) {	/* taranan kareleri yeni yan dizine yaz */
			FILE *f = fopen(iy, "w");
			if (f) {
				fputs("KIDX1\n", f);
				for (int i = 0; i < n; i++) fprintf(f, "%u %u %d\n", gecici[i].ofs, gecici[i].len, gecici[i].ms);
				fclose(f);
			}
			idx_var = 1;
		}
	}
	parca_idx = fopen(iy, "a");
	if (parca_idx && !idx_var) fputs("KIDX1\n", parca_idx);
	sakla(parca_ad);
	return 0;
}

static int parca_yaz(const uint8_t *j, long n)
{
	long long ms = gercek_ms();
	time_t t = (time_t)(ms / 1000);
	struct tm tm;
	localtime_r(&t, &tm);
	if (!yil_gecerli(tm.tm_year + 1900)) return -2;	/* v10: 1970 klasoru yok ("saat gecersiz") */
	char ad[20];
	snprintf(ad, sizeof ad, "%04d%02d%02d/%02d%02d.mjpg", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
	if (!parca || strcmp(ad, parca_ad)) {
		parca_kapat();
		if (parca_ac(ad)) return -1;
	}
	if (fwrite(j, 1, (size_t)n, parca) != (size_t)n || fflush(parca)) {
		parca_kapat();	/* v10: yarim kare; yeniden acilis gercek son ofseti okur, .idx kaymaz */
		return -1;
	}
	int32_t dms = (int32_t)(tm.tm_sec * 1000 + ms % 1000);
	if (parca_idx) {
		fprintf(parca_idx, "%llu %ld %d\n", (unsigned long long)parca_ofs, n, dms);
		fflush(parca_idx);
	}
	if (parca_n < OLAY_AZAMI_KARE) { parca_len[parca_n] = (uint32_t)n; parca_ms[parca_n] = dms; parca_n++; }
	parca_ofs += (uint64_t)n;
	return 0;
}

/* yan dizini olmayan parcalari indeksle (--indeksle); donus: islenen parca sayisi */
static int indeksle(void)
{
	char **l;
	unsigned long long toplam;
	int n = parcalar(&l, &toplam), islenen = 0;
	static uint8_t t[300 * 1024];
	static uint32_t len[OLAY_AZAMI_KARE];
	static int32_t ms[OLAY_AZAMI_KARE];
	for (int i = 0; i < n; i++) {
		char tam[600], iy[600];
		snprintf(tam, sizeof tam, "%s/%s", kok, l[i]);
		idx_yolu(iy, sizeof iy, l[i]);
		struct stat st;
		if (var(iy) || stat(tam, &st) || (parca && !strcmp(l[i], parca_ad))) continue;
		int fd = open(tam, O_RDONLY | O_CLOEXEC);
		if (fd < 0) continue;
		int k = idx_tara(fd, (uint64_t)st.st_size, gecici, OLAY_AZAMI_KARE, t, sizeof t);
		close(fd);
		FILE *f = fopen(iy, "w");
		if (!f) continue;
		fputs("KIDX1\n", f);
		for (int j = 0; j < k; j++) {
			fprintf(f, "%u %u %d\n", gecici[j].ofs, gecici[j].len, gecici[j].ms);
			len[j] = gecici[j].len;
			ms[j] = gecici[j].ms;
		}
		fclose(f);
		olay_yaz(l[i], len, ms, k);
		printf("indeks: %s %d kare\n", l[i], k);
		islenen++;
	}
	for (int j = 0; j < n; j++) free(l[j]);
	free(l);
	return islenen;
}

static void durum_yaz(FILE *f, int canli, int kayit, int kalkan, double fps, unsigned long kareler, const char *hata)
{
	fprintf(f, "{\"surum\":\"kamera-kayit-v12\",\"kayit_hareket\":%s,", mono() < hareket_bitis ? "true" : "false");	/* v11 */
	fprintf(f, "\"saat_ayar\":%ld,\"saat_ayar_hata\":\"%s\",\"bekci\":%s,\"zaman\":%s,\"son_alarm\":%ld,\"bekci_skor\":%d,",
		saat_ayar_unix, saat_hata, var(bekci_yol()) ? "true" : "false", var(zaman_yol()) ? "true" : "false", son_alarm_unix, bekci_skor);
	fprintf(f, "\"zaman_aralik\":%d,\"zaman_olay\":%s,\"zaman_mb\":%.1f,\"zaman_sinir_mb\":%d,\"zaman_dolu\":%s,",	/* v9 */
		zaman_aralik, var(zaman_olay_yol()) ? "true" : "false", zaman_bayt < 0 ? -1.0 : zaman_bayt / 1048576.0, zaman_sinir_mb,
		zaman_dolu ? "true" : "false");
	fprintf(f, "\"canli\":%s,\"kayit\":%s,\"kalkan\":%s,\"fps\":%.1f,\"kareler\":%lu,"
		"\"parca\":\"%s\",\"parca_sayi\":%d,\"toplam_mb\":%.1f,\"bos_mb\":%lld,\"azami_parca\":%d,\"kayit_fps\":%.1f,"
		"\"son_skor\":%d,\"parca_kare\":%d,\"olay_esik\":%d,\"idx\":true,\"hata\":\"%s\"}\n",
		canli ? "true" : "false", kayit ? "true" : "false", kalkan ? "true" : "false", fps, kareler,
		parca ? parca_ad : "", son_sayi, son_toplam / 1048576.0, bos_mb(), azami_parca, kayit ? kayit_hizi(mono()) : KAYIT_FPS,
		son_skor, parca ? parca_n : 0, OLAY_ESIK_VARSAYILAN, hata ? hata : "");
}

static void durum_dosya(int canli, int kayit, int kalkan, double fps, unsigned long kareler, const char *hata)
{
	FILE *f = fopen("/tmp/kamera-kayit.json.tmp", "w");
	if (!f) return;
	durum_yaz(f, canli, kayit, kalkan, fps, kareler, hata);
	fclose(f);
	rename("/tmp/kamera-kayit.json.tmp", "/tmp/kamera-kayit.json");
}

/* v10: gozetici gunlugu yalniz cocuk cikinca kirpiyordu (haftalarca /tmp'de buyur). fd duzenli dosya ve siniri
 * asmissa sifirlanir (kabuk >> ile O_APPEND acar: sonraki yazim 0'dan). Donus: kirpilan boyut, yoksa 0. */
static long long gunluk_kirp(int fd, long long sinir)
{
	struct stat st;
	if (fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size <= sinir || ftruncate(fd, 0)) return 0;
	lseek(fd, 0, SEEK_SET);	/* inceleme: O_APPEND'siz acildiysa NUL deligi olmasin */
	return (long long)st.st_size;
}

/* ---------------- v3: bekci / fotograf / zaman atlamali ---------------- */
static char kb_bekci[300], kb_zaman[300];
static const char *bekci_yol(void) { snprintf(kb_bekci, sizeof kb_bekci, "%s/bekci-acik", kok); return kb_bekci; }
static const char *zaman_yol(void) { snprintf(kb_zaman, sizeof kb_zaman, "%s/zaman-acik", kok); return kb_zaman; }
static char kb_zolay[300];	/* v9 */
static const char *zaman_olay_yol(void) { snprintf(kb_zolay, sizeof kb_zolay, "%s/zaman-olay", kok); return kb_zolay; }

static int sayi_oku(const char *yol, int varsayilan, int en_az, int en_cok)
{
	FILE *f = fopen(yol, "r");
	int v = varsayilan;
	if (f) {
		if (fscanf(f, "%d", &v) != 1 || v < en_az || v > en_cok) v = varsayilan;
		fclose(f);
	}
	return v;
}

static void zaman_damgasi(char *o, size_t n)
{
	time_t t = (time_t)(gercek_ms() / 1000);
	struct tm tm;
	localtime_r(&t, &tm);
	strftime(o, n, "%Y%m%d-%H%M%S", &tm);
}

static int dosya_yaz(const char *yol, const uint8_t *j, long n)
{
	char g[340];
	snprintf(g, sizeof g, "%s.part", yol);
	int fd = open(g, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd < 0) return -1;
	ssize_t w = write(fd, j, (size_t)n);
	close(fd);
	if (w != n) { unlink(g); return -1; }
	return rename(g, yol);
}

/* fotolar/ en cok 300 dosya: en eski silinir. v10: siralama onek degil zaman damgasi ("<onek>-YYYYMMDD-HHMMSS.jpg") */
static int jpg_adi(const struct dirent *e) { size_t l = strlen(e->d_name); return l > 4 && !strcmp(e->d_name + l - 4, ".jpg"); }
static const char *foto_zaman(const char *a) { const char *t = strchr(a, '-'); return t ? t + 1 : a; }
static int foto_sira(const struct dirent **a, const struct dirent **c)
{
	int r = strcmp(foto_zaman((*a)->d_name), foto_zaman((*c)->d_name));
	return r ? r : strcmp((*a)->d_name, (*c)->d_name);
}
static void fotolar_temizle(const char *dz)
{
	struct dirent **l;
	int n = scandir(dz, &l, jpg_adi, foto_sira);
	for (int i = 0; i < n; i++) {
		if (n - i > 300) {
			char y[600];
			snprintf(y, sizeof y, "%s/%s", dz, l[i]->d_name);
			unlink(y);
		}
		free(l[i]);
	}
	if (n >= 0) free(l);
}

/* v10: kuyruktaki .txt sayisi (gonderici yoksa/agsizken RAM'de birikmesin) */
static int kuyruk_say(void)
{
	DIR *d = opendir(kuyruk_dz);
	struct dirent *e;
	int n = 0;
	while (d && (e = readdir(d))) {
		size_t l = strlen(e->d_name);
		n += l > 4 && !strcmp(e->d_name + l - 4, ".txt");
	}
	if (d) closedir(d);
	return n;
}

/* kuyruga bildirim: metin + istege bagli foto. v10: dolu ise eklenmez (-1), bir kez gunluge */
static int kuyruk_yaz(const char *metin, const char *foto)
{
	static unsigned sira;
	static int dolu_yazildi;
	mkdir(kuyruk_dz, 0700);
	int n = kuyruk_say();
	if (n > KUYRUK_AZAMI) {	/* inceleme: kendi en eski ogemiz (<sn>-k<sira>.txt) silinir; en yeni alarm atilmasin */
		long long et = -1, t;
		unsigned es = 0, s;
		char c, ea[300] = "";
		DIR *d = opendir(kuyruk_dz);
		struct dirent *e;
		while (d && (e = readdir(d)))
			if (sscanf(e->d_name, "%lld-k%u.tx%c", &t, &s, &c) == 3 && c == 't' && strlen(e->d_name) < sizeof ea &&
			    (et < 0 || t < et || (t == et && s < es))) {
				et = t; es = s;
				snprintf(ea, sizeof ea, "%s", e->d_name);
			}
		if (d) closedir(d);
		if (!ea[0]) {
			if (!dolu_yazildi) printf("bildirim kuyrugu dolu (%d > %d .txt), kendi ogemiz yok: eklenmedi\n", n, KUYRUK_AZAMI);
			dolu_yazildi = 1;
			return -1;
		}
		char ey[600];
		snprintf(ey, sizeof ey, "%s/%s", kuyruk_dz, ea);
		unlink(ey);
		if (!dolu_yazildi) printf("bildirim kuyrugu dolu (%d > %d .txt): en eski kendi ogemiz silindi (%s)\n", n, KUYRUK_AZAMI, ea);
		dolu_yazildi = 1;
	} else
		dolu_yazildi = 0;
	char y[400], g[420];
	snprintf(y, sizeof y, "%s/%lld-k%u.txt", kuyruk_dz, gercek_ms() / 1000, sira++);
	snprintf(g, sizeof g, "%s.part", y);
	FILE *f = fopen(g, "w");
	if (!f) return -1;
	if (foto) fprintf(f, "foto:%s\n", foto);
	fprintf(f, "%s\n", metin);
	if (fclose(f) || rename(g, y)) { unlink(g); return -1; }
	return 0;
}

/* foto: SD fotolar/<onek>-<zaman>.jpg; telegram ise kuyruga SD yolu (v10: /tmp kopyasi yok; tuketici yalniz /tmp siler) */
static void foto_kaydet(const uint8_t *j, long n, const char *onek, const char *baslik, int telegram)
{
	char dz[300], ts[32], y[400];
	snprintf(dz, sizeof dz, "%s/fotolar", kok);
	mkdir(dz, 0755);
	zaman_damgasi(ts, sizeof ts);
	snprintf(y, sizeof y, "%s/%s-%s.jpg", dz, onek, ts);
	int ok = !dosya_yaz(y, j, n);
	fotolar_temizle(dz);
	if (telegram) {
		char bs[400];
		if (!strcmp(onek, "foto"))	/* v4/v7: dogru zaman aciklamada; kamera saati esitlenmediyse not */
			snprintf(bs, sizeof bs, "%s\n🕒 %.2s.%.2s.%.4s %.2s:%.2s:%.2s%s", baslik, ts + 6, ts + 4, ts, ts + 9, ts + 11, ts + 13,
				 saat_ayar_unix ? "" : " (resimdeki tarih kamera modülünün kendi saati, dikkate almayın)");
		else
			snprintf(bs, sizeof bs, "%s", baslik);
		kuyruk_yaz(bs, ok ? y : NULL);	/* v10: SD yazilamadiysa yalniz metin */
	}
	printf("foto %s: %s (%ld bayt)%s\n", onek, ok ? y : "SD YAZILAMADI", n, telegram ? " + telegram" : "");
}

/* bekci: kayan pencere puani; donus 1 = alarm */
static int bekci_karar(uint32_t boy, double simdi)
{
	if (bekci_n < BEKCI_PENCERE) bekci_boy[bekci_n++] = boy;
	else { memmove(bekci_boy, bekci_boy + 1, (BEKCI_PENCERE - 1) * sizeof *bekci_boy); bekci_boy[BEKCI_PENCERE - 1] = boy; }
	if (bekci_n < BEKCI_PENCERE) return 0;
	int tepe;
	bekci_skor = olay_skor(bekci_boy, bekci_n, &tepe);
	return simdi - bekci_bas >= 10.0 && simdi - son_alarm >= 120.0 && bekci_skor >= bekci_esik;
}

/* v9: zaman/ altindaki time-lapse (YYYYMMDD.mjpg) ve olay (YYYYMMDD-olay.csv) dosyalari */
static int zaman_dosyasi(const char *a)
{
	size_t l = strlen(a);
	return strspn(a, "0123456789") == 8 && ((l == 13 && !strcmp(a + 8, ".mjpg")) || (l == 17 && !strcmp(a + 8, "-olay.csv")));
}

/* v9: toplam sinir asilinca bugun haric en eski gun silinir; bugun tek basina siniri asarsa zaman_dolu */
static void zaman_boyut_denetle(const char *bugun)
{
	char dz[300];
	snprintf(dz, sizeof dz, "%s/zaman", kok);
	for (int tur = 0; tur < 64; tur++) {
		DIR *dd = opendir(dz);
		if (!dd) { zaman_bayt = 0; return; }
		long long top = 0, bugun_b = 0;
		char en_eski[16] = "";
		struct dirent *e;
		while ((e = readdir(dd))) {
			if (!zaman_dosyasi(e->d_name)) continue;
			char y[600];
			struct stat st;
			snprintf(y, sizeof y, "%s/%s", dz, e->d_name);
			if (stat(y, &st)) continue;
			top += st.st_size;
			if (!strncmp(e->d_name, bugun, 8)) bugun_b += st.st_size;
			else if (!en_eski[0] || strncmp(e->d_name, en_eski, 8) < 0) { memcpy(en_eski, e->d_name, 8); en_eski[8] = 0; }
		}
		closedir(dd);
		zaman_bayt = top;
		long long sinir = (long long)zaman_sinir_mb * 1048576;
		if (top <= sinir || !en_eski[0]) {
			if (bugun_b >= sinir && !zaman_dolu) printf("zaman: bugun %lld MB >= %d MB sinir, ekleme durdu\n", bugun_b / 1048576, zaman_sinir_mb);
			zaman_dolu = bugun_b >= sinir;
			return;
		}
		char y[600];
		snprintf(y, sizeof y, "%s/%s.mjpg", dz, en_eski);
		unlink(y);
		snprintf(y, sizeof y, "%s/%s-olay.csv", dz, en_eski);
		unlink(y);
		printf("zaman: %lld MB > %d MB sinir, %s silindi\n", top / 1048576, zaman_sinir_mb, en_eski);
	}
}

/* zaman atlamali: gunluk mjpg'ye ekle; 30 gunden eski gunleri sil (gun degisince). v9: donus eklenen karenin gunluk
 * dosyadaki ofseti (-1: eklenmedi); boyut siniri gun degisince ve 60 sn'de bir denetlenir; dolu iken eklenmez. */
static long long zaman_ekle(const uint8_t *j, long n)
{
	static char son_gun[16];
	char dz[300], ts[32], gun[16], y[400];
	if (!saat_gecerli()) return -1;	/* v10: 19700101.mjpg yok */
	snprintf(dz, sizeof dz, "%s/zaman", kok);
	mkdir(dz, 0755);
	zaman_damgasi(ts, sizeof ts);
	memcpy(gun, ts, 8);
	gun[8] = 0;
	int yeni_gun = strcmp(son_gun, gun) != 0;
	if (yeni_gun) {
		snprintf(son_gun, sizeof son_gun, "%s", gun);
		zaman_dolu = 0;
		time_t esik = (time_t)(gercek_ms() / 1000) - 30 * 86400;
		struct tm tm;
		localtime_r(&esik, &tm);
		char eski[16];
		strftime(eski, sizeof eski, "%Y%m%d", &tm);
		DIR *dd = opendir(dz);
		struct dirent *e;
		while (dd && (e = readdir(dd))) {
			if (zaman_dosyasi(e->d_name) && strncmp(e->d_name, eski, 8) < 0) {
				char sy[600];
				snprintf(sy, sizeof sy, "%s/%s", dz, e->d_name);
				unlink(sy);
			}
		}
		if (dd) closedir(dd);
	}
	double simdi = gercek_ms() / 1000.0;
	if (yeni_gun || simdi - son_zaman_denetim >= 60) {
		char sy[340];
		snprintf(sy, sizeof sy, "%s/zaman-sinir-mb", kok);
		zaman_sinir_mb = sayi_oku(sy, 4096, 256, 12288);
		son_zaman_denetim = simdi;
		zaman_boyut_denetle(gun);
	}
	if (zaman_dolu) return -1;
	snprintf(y, sizeof y, "%s/%s.mjpg", dz, gun);
	FILE *f = fopen(y, "ab");
	if (!f) return -1;
	long long ofs = -1;
	if (!fseeko(f, 0, SEEK_END)) ofs = ftello(f);
	if (ofs < 0 || ofs + n > ZAMAN_DOSYA_AZAMI) {
		fclose(f);
		if (ofs >= 0) { zaman_dolu = 1; printf("zaman: %s FAT32 payina ulasti (%lld MB), ekleme durdu\n", gun, ofs / 1048576); }
		return -1;
	}
	if (fwrite(j, 1, (size_t)n, f) != (size_t)n) ofs = -1;
	fclose(f);
	if (ofs >= 0 && zaman_bayt >= 0) zaman_bayt += n;
	return ofs;
}

/* v9: olay tespiti dakika satiri: zaman/<gun>-olay.csv "HHMM,skor,ofs" (ofs: dakikanin en hareketli eklenen karesi, -1 yok) */
static void zo_yaz(void)
{
	if (zo_dk < 0 || !zo_gun[0]) return;
	char y[400];
	snprintf(y, sizeof y, "%s/zaman/%s-olay.csv", kok, zo_gun);
	FILE *f = fopen(y, "a");
	if (f) {
		fprintf(f, "%02d%02d,%d,%lld\n", zo_dk / 60, zo_dk % 60, zo_skor < 0 ? 0 : zo_skor, zo_ofs);
		fclose(f);
	}
	zo_dk = -1;
	zo_skor = zo_ofs_skor = -1;
	zo_ofs = -1;
}

/* v9: her puan orneginde (ofs -1) ve her eklenen time-lapse karesinde (ofs >= 0, skor o anki puan) cagrilir */
static void zo_isle(int skor, long long ofs)
{
	char ts[32];
	if (!saat_gecerli()) return;	/* v10: 19700101-olay.csv yok */
	zaman_damgasi(ts, sizeof ts);
	int dk = ((ts[9] - '0') * 10 + (ts[10] - '0')) * 60 + (ts[11] - '0') * 10 + (ts[12] - '0');
	if (zo_dk >= 0 && (dk != zo_dk || strncmp(zo_gun, ts, 8))) zo_yaz();
	if (zo_dk < 0) { zo_dk = dk; memcpy(zo_gun, ts, 8); zo_gun[8] = 0; }
	if (ofs >= 0 && (zo_ofs < 0 || skor > zo_ofs_skor)) { zo_ofs = ofs; zo_ofs_skor = skor; }
	if (skor > zo_skor) zo_skor = skor;
}

/* v10: kamera-aktif icerigi tam olarak KAYIT_ISARET mi (kendi gostergemiz) */
static int isaretli_bayrak(const char *y)
{
	char t[16];
	int fd = open(y, O_RDONLY | O_CLOEXEC);
	if (fd < 0) return 0;
	ssize_t r = read(fd, t, sizeof t);
	close(fd);
	return r == (ssize_t)strlen(KAYIT_ISARET) && !memcmp(t, KAYIT_ISARET, (size_t)r);
}

/* bayraklar: kalkan her seyi bastirir (USB birakilir) */
static void bayraklar(int *canli, int *kayit, int *kalkan)
{
	char kb[600];
	snprintf(kb, sizeof kb, "%s/kayit-acik", kok);
	*kalkan = var(kalkan_yol);
	*kayit = !*kalkan && var(kb);
	if (*kayit && !var(canli_bayrak)) {	/* gizlilik: kayit surerken panel gostergesi yanik kalir */
		int fd = open(canli_bayrak, O_WRONLY | O_CREAT | O_EXCL | O_CLOEXEC, 0644);	/* v10: EXCL, baskasininkine yazma */
		if (fd >= 0) {
			ssize_t w = write(fd, KAYIT_ISARET, strlen(KAYIT_ISARET));
			(void)w;
			close(fd);
		}
	}
	int isaret = isaretli_bayrak(canli_bayrak);
	if (isaret && son_kare_okundu()) izlendi_t = mono();
	if (isaret && !*kayit && (*kalkan || mono() - izlendi_t >= 5)) { unlink(canli_bayrak); isaret = 0; }	/* v10: yalniz kendi
		isaretimiz; inceleme: okuyan izleyici varken kalir (kayit sirasinda acilan canli kesilmesin) */
	struct stat bs;	/* v12: bos bayrak (KAMERA 1) 60 sn okunmazsa silinir */
	if (!isaret && !stat(canli_bayrak, &bs) && bs.st_size == 0) {
		if (son_kare_okundu()) izlendi_t = mono();
		if (!bos_bayrak_t) bos_bayrak_t = mono();
		if (mono() - bos_bayrak_t > 60 && mono() - izlendi_t > 60) {
			unlink(canli_bayrak);
			bos_bayrak_t = 0;
			puts("canli: bos bayrak (KAMERA 1) 60 sn okunmadi, kaldirildi");
		}
	} else
		bos_bayrak_t = 0;
	*canli = !*kalkan && var(canli_bayrak) && (!isaret || mono() - izlendi_t < 5);	/* v10: isaret = gosterge; okuyan varsa canli */
}

/* ---------------- self-test (USB/kamera ACILMAZ; yalniz gecici klasor) ---------------- */
static int hata_say;
#define DENE(x, ...) do { if (!(x)) { printf("FAIL %s:%d: ", __func__, __LINE__); printf(__VA_ARGS__); puts(""); hata_say++; } } while (0)

/* yapisal olarak gecerli sahte JPEG: SOI, APP0 (icinde sahte FF D9), DQT, SOS, entropi (FF 00 + RST), EOI */
static size_t sahte_jpeg(uint8_t *o, size_t govde, uint8_t tohum)
{
	size_t i = 0;
	o[i++] = 0xff; o[i++] = 0xd8;
	o[i++] = 0xff; o[i++] = 0xe0; o[i++] = 0; o[i++] = 8;
	o[i++] = 'J'; o[i++] = 'F'; o[i++] = 0xff; o[i++] = 0xd9; o[i++] = 0; o[i++] = tohum;
	o[i++] = 0xff; o[i++] = 0xdb; o[i++] = 0; o[i++] = 0x43;
	for (int k = 0; k < 65; k++) o[i++] = (uint8_t)(k + 1);
	o[i++] = 0xff; o[i++] = 0xda; o[i++] = 0; o[i++] = 8;
	for (int k = 0; k < 6; k++) o[i++] = (uint8_t)k;
	for (size_t k = 0; k < govde; k++) {
		uint8_t v = (uint8_t)(k * 31 + tohum);
		if (v == 0xff) { o[i++] = 0xff; o[i++] = 0; continue; }
		o[i++] = v;
		if (k == govde / 2) { o[i++] = 0xff; o[i++] = 0xd3; }	/* RST3 */
	}
	o[i++] = 0xff; o[i++] = 0xd9;
	return i;
}

static long long dosya_boyut(const char *y)
{
	struct stat st;
	return stat(y, &st) ? -1 : (long long)st.st_size;
}

static int satir_say(const char *y)
{
	FILE *f = fopen(y, "r");
	int n = 0, c;
	while (f && (c = fgetc(f)) != EOF) n += c == '\n';
	if (f) fclose(f);
	return n;
}

static long long yerel_ms(int yil, int ay, int gun, int sa, int dk, int sn, int ms)
{
	struct tm tm = { .tm_year = yil - 1900, .tm_mon = ay - 1, .tm_mday = gun, .tm_hour = sa, .tm_min = dk, .tm_sec = sn,
		.tm_isdst = -1 };
	return (long long)mktime(&tm) * 1000 + ms;
}

static int self_test(void)
{
	/* 1) v1: sondan FF D9 kirpma */
	memset(b, 0xff, sizeof b);
	uint32_t sz = 3000;
	b[0] = (uint8_t)sz; b[1] = (uint8_t)(sz >> 8); b[2] = 0; b[3] = 0;
	b[17] = 0xff; b[18] = 0xd8;
	memset(b + 19, 0x11, 2000);
	b[17 + 2019] = 0xff; b[17 + 2020] = 0xd9;
	uint32_t n = sz;
	while (n > 2 && !(b[17 + n - 2] == 0xff && b[17 + n - 1] == 0xd9)) n--;
	DENE(n == 2021, "v1 kirpma n=%u", n);

	/* 2) jpeg_son_c: APP icindeki sahte EOI, bayt doldurma, RST, eksik ve bozuk */
	static uint8_t j[4][4000];
	size_t jn[4];
	for (int i = 0; i < 4; i++) jn[i] = sahte_jpeg(j[i], 600 + (size_t)i * 700, (uint8_t)(i * 17));
	DENE(jpeg_son_c(j[0], jn[0]) == (long)jn[0], "jpeg_son tam %ld/%zu", jpeg_son_c(j[0], jn[0]), jn[0]);
	DENE(jpeg_son_c(j[0], 12) == 0 && jpeg_son_c(j[0], jn[0] - 1) == 0, "jpeg_son eksik");
	DENE(jpeg_son_c(j[0] + 1, jn[0] - 1) == -1, "jpeg_son bozuk");
	uint8_t cift[8000];
	memcpy(cift, j[0], jn[0]); memcpy(cift + jn[0], j[1], jn[1]);
	DENE(jpeg_son_c(cift, jn[0] + jn[1]) == (long)jn[0], "ard arda iki kare");
	/* segment izleme cop sonrasi FF D9'a aldanmaz; v1 kirpmasi aldanirdi */
	uint8_t copl[5000];
	memcpy(copl, j[0], jn[0]); memset(copl + jn[0], 0x22, 100); copl[jn[0] + 50] = 0xff; copl[jn[0] + 51] = 0xd9;
	DENE(kare_uzunluk(copl, (uint32_t)jn[0] + 100) == (long)jn[0], "kesin uzunluk");

	/* 2b) v8 okuma_bekle: tablo + 10 sn kayit benzetimi (okuma 70 ms surer) */
	DENE(okuma_bekle(100, 1, 99.9, 2, 0, 0, 0, 0, 60) > 0.199 && okuma_bekle(100, 1, 99.9, 2, 0, 0, 0, 0, 60) < 0.201, "kayit tavan");
	DENE(okuma_bekle(100, 1, 99.55, 2, 0, 0, 0, 0, 60) > 0.049 && okuma_bekle(100, 1, 99.55, 2, 0, 0, 0, 0, 60) < 0.051, "kayit kalan");
	DENE(okuma_bekle(100, 1, 99.4, 2, 0, 0, 0, 0, 60) == 0, "kayit zamani gelmis");
	DENE(okuma_bekle(100, 0, 0, 2, 1, 99.6, 0, 0, 60) > 0.099 && okuma_bekle(100, 0, 0, 2, 1, 99.6, 0, 0, 60) < 0.101, "bekci");
	DENE(okuma_bekle(100, 0, 0, 2, 0, 0, 1, 40.05, 60) > 0.049 && okuma_bekle(100, 0, 0, 2, 0, 0, 1, 40.05, 60) < 0.051, "zaman");
	DENE(okuma_bekle(100, 0, 0, 2, 0, 0, 0, 0, 60) > 0.199, "hicbiri: 0.2");
	DENE(okuma_bekle(100, 1, 0, 2, 1, 0, 0, 0, 60) == 0, "ilk okuma hemen");
	{
		double t = 0, sk = 0;
		int okuma = 0, yazim = 0, v7 = 0;
		while (t < 10) {
			double w = okuma_bekle(t, 1, sk, 2, 0, 0, 0, 0, 60);
			if (w > 0.005) { t += w; continue; }
			double kt = t;	/* main ile ayni: okuma basi saati */
			okuma++;
			t += 0.07;
			if (kt - sk >= 0.495) { yazim++; sk = kt; }
		}
		for (double u = 0; u < 10; u += 0.07 + 0.05) v7++;
		DENE(okuma >= 19 && okuma <= 21 && yazim == okuma, "v8 benzetim: %d okuma, %d yazim", okuma, yazim);
		printf("okuma_bekle: 10 sn kayit -> v8 %d okuma / %d yazim (v7 kalibi %d okuma)\n", okuma, yazim, v7);
	}

	/* 3) olay puani */
	uint32_t sabit[10], hareket[10];
	for (int i = 0; i < 10; i++) { sabit[i] = 80000 + (i % 2) * 400; hareket[i] = 80000; }
	hareket[6] = 120000; hareket[7] = 118000;
	int tepe = -1;
	int s1 = olay_skor(sabit, 10, NULL), s2 = olay_skor(hareket, 10, &tepe);
	DENE(s1 < 10, "sabit sahne puani %d", s1);
	DENE(s2 > 250 && tepe == 6, "hareket puani %d tepe %d", s2, tepe);
	DENE(olay_skor(sabit, 1, &tepe) == 0 && tepe == 0, "tek kare");

	/* 4) kayit simulasyonu (gecici SD agaci) */
	char sab[] = "/tmp/kamera-kayit-v2-testXXXXXX";
	if (!mkdtemp(sab)) { perror("mkdtemp"); return 1; }
	static char k1[256], k2[256], k3[256], k4[256];
	snprintf(k1, sizeof k1, "%s/kamera", sab); kok = k1;
	sd_kok = sab;
	snprintf(k2, sizeof k2, "%s/kalkan", sab); kalkan_yol = k2;
	snprintf(k3, sizeof k3, "%s/kamera-aktif", sab); canli_bayrak = k3;
	static char k5[256];
	snprintf(k5, sizeof k5, "%s/kamera-son.jpg", sab); son_kare = k5;
	int boy[6] = { 0, 1, 0, 3, 3, 2 };	/* hangi sahte kare */
	for (int i = 0; i < 6; i++) {
		test_ms = yerel_ms(2026, 10, 12, 22, 55, 10 + i, 250);
		DENE(!parca_yaz(j[boy[i]], (long)jn[boy[i]]), "parca_yaz %d", i);
	}
	test_ms = yerel_ms(2026, 10, 12, 22, 56, 0, 100);
	parca_yaz(j[1], (long)jn[1]);
	parca_yaz(j[1], (long)jn[1]);
	char y[600], iy[600], csv[600];
	snprintf(y, sizeof y, "%s/20261012/2255.mjpg", kok);
	idx_yolu(iy, sizeof iy, "20261012/2255.mjpg");
	snprintf(csv, sizeof csv, "%s/20261012/olay.csv", kok);
	long long beklenen = 0;
	for (int i = 0; i < 6; i++) beklenen += (long long)jn[boy[i]];
	DENE(dosya_boyut(y) == beklenen, "mjpg boyutu %lld/%lld", dosya_boyut(y), beklenen);
	static struct kidx ki[64];
	int kn = idx_oku(iy, (uint64_t)beklenen, ki, 64);
	DENE(kn == 6, "idx kare sayisi %d", kn);
	FILE *f = fopen(y, "rb");
	static uint8_t okunan[8000];
	for (int i = 0; f && i < kn; i++) {
		fseek(f, (long)ki[i].ofs, SEEK_SET);
		size_t r = fread(okunan, 1, ki[i].len, f);
		DENE(r == ki[i].len && !memcmp(okunan, j[boy[i]], r), "idx kare %d icerigi", i);
		DENE(ki[i].ms == (10 + i) * 1000 + 250, "idx ms %d = %d", i, ki[i].ms);
	}
	if (f) fclose(f);
	DENE(satir_say(csv) == 1, "2255 kapaninca olay.csv tek satir (%d)", satir_say(csv));
	uint32_t l6[6];
	for (int i = 0; i < 6; i++) l6[i] = (uint32_t)jn[boy[i]];
	int t6;
	int s6 = olay_skor(l6, 6, &t6);
	char bek[96], sat[96] = "";
	snprintf(bek, sizeof bek, "2255,%d,6,%lld,%d,%d\n", s6, beklenen, t6, (10 + t6) * 1000 + 250);
	f = fopen(csv, "r");
	if (f) { if (!fgets(sat, sizeof sat, f)) sat[0] = 0; fclose(f); }
	DENE(!strcmp(sat, bek), "olay.csv satiri '%s' beklenen '%s'", sat, bek);
	/* ayni dakikaya yeniden baslama: yan dizin devam eder */
	parca_kapat();
	test_ms = yerel_ms(2026, 10, 12, 22, 56, 30, 0);
	parca_yaz(j[2], (long)jn[2]);
	DENE(parca_n == 3, "yeniden acilan parca onceki kareleri devraldi (%d)", parca_n);
	parca_kapat();
	DENE(satir_say(csv) == 3, "2256 iki kez kapandi -> 3 satir (%d)", satir_say(csv));

	/* 5) --indeksle: yan dizini silinen parca ayni ofsetlerle yeniden indekslenir */
	unlink(iy);
	DENE(indeksle() == 1, "indeksle 1 parca");
	static struct kidx kt[64];
	int tn = idx_oku(iy, (uint64_t)beklenen, kt, 64);
	DENE(tn == 6, "tarama kare sayisi %d", tn);
	for (int i = 0; i < tn && i < kn; i++) DENE(kt[i].ofs == ki[i].ofs && kt[i].len == ki[i].len && kt[i].ms == -1, "tarama %d", i);
	DENE(indeksle() == 0, "ikinci indeksle bos");
	/* tarama: kareler arasinda cop varsa hizalanir */
	snprintf(k4, sizeof k4, "%s/20261012/2257.mjpg", kok);
	f = fopen(k4, "wb");
	if (f) { fwrite(j[0], 1, jn[0], f); fwrite("COPCOP\xff\xd9", 1, 8, f); fwrite(j[3], 1, jn[3], f); fwrite(j[1], 1, 40, f); fclose(f); }
	int fd = open(k4, O_RDONLY);
	static uint8_t tt[300 * 1024];
	int cn = idx_tara(fd, (uint64_t)dosya_boyut(k4), kt, 64, tt, sizeof tt);
	close(fd);
	DENE(cn == 2 && kt[1].ofs == jn[0] + 8 && kt[1].len == jn[3], "copten sonra hizalama cn=%d", cn);
	/* kucuk pencere ile tarama da ayni sonucu verir */
	fd = open(y, O_RDONLY);
	cn = idx_tara(fd, (uint64_t)beklenen, kt, 64, tt, 3500);
	close(fd);
	DENE(cn == 6 && kt[5].ofs == ki[5].ofs, "kucuk pencere tarama cn=%d", cn);

	/* 6) saklama: en eski parca .idx ile silinir; gunde mjpg kalmazsa olay.csv + klasor gider */
	test_ms = yerel_ms(2026, 10, 13, 1, 1, 0, 0);
	azami_parca = 1;
	parca_yaz(j[0], (long)jn[0]);
	parca_kapat();
	char gun12[600];
	snprintf(gun12, sizeof gun12, "%s/20261012", kok);
	DENE(!var(y) && !var(iy) && !var(csv) && !var(gun12), "saklama: eski gun tamamen temizlendi");
	snprintf(y, sizeof y, "%s/20261013/0101.mjpg", kok);
	DENE(var(y), "saklama: simdiki parca duruyor");

	/* 7) kalkan; v10: kayit gostergesi isaretli (yalniz gosterge), baskasinin bayragi gercek canli ve silinmez */
	int c, k, kl;
	snprintf(y, sizeof y, "%s/kayit-acik", kok);
	f = fopen(y, "w"); if (f) fclose(f);
	bayraklar(&c, &k, &kl);
	DENE(c == 0 && k == 1 && kl == 0 && isaretli_bayrak(canli_bayrak), "v10: kayit gostergeyi isaretli acar, canli degil");
	son_kare_yaz(j[0], (long)jn[0]);
	bayraklar(&c, &k, &kl);
	DENE(c == 0, "v10: yazilip okunmamis son kare canli acmaz");
	usleep(30000);	/* kaba saat: okuma yazmadan sonraki tike dussun */
	f = fopen(son_kare, "rb");
	if (f) { char ok[16]; DENE(fread(ok, 1, sizeof ok, f) > 0, "son kare oku"); fclose(f); }
	bayraklar(&c, &k, &kl);
	DENE(c == 1 && k == 1, "v10: isaretli bayrakta son kare okununca canli (izleyici)");
	izlendi_t = -1e9;
	usleep(30000);
	f = fopen(son_kare, "rb");
	if (f) { char ok[16]; DENE(fread(ok, 1, sizeof ok, f) > 0, "son kare oku 2"); fclose(f); }
	son_kare_yaz(j[0], (long)jn[0]);	/* inceleme: okuma yeni kare gelmeden once yakalanir */
	bayraklar(&c, &k, &kl);
	DENE(c == 1 && k == 1, "inceleme: okunan kare yenisiyle degisse de izleyici canli");
	izlendi_t = -1e9;
	bayraklar(&c, &k, &kl);
	DENE(c == 0, "v10: yeni kare okunmadan ve 5 sn gecince yavas yola doner");
	static const char *baska[4] = { "", "merkez 12\n", "telegram 1700000000\n", "kayit\nx" };
	for (int i = 0; i < 4; i++) {
		DENE(!dosya_yaz(canli_bayrak, (const uint8_t *)baska[i], (long)strlen(baska[i])), "bayrak yaz %d", i);
		bayraklar(&c, &k, &kl);
		DENE(c == 1 && k == 1, "v10: '%s' bayragi gercek canli", baska[i]);
	}
	unlink(y);	/* kayit kapandi */
	bayraklar(&c, &k, &kl);
	DENE(c == 1 && k == 0 && var(canli_bayrak), "v10: baskasinin bayragi silinmez");
	DENE(!dosya_yaz(canli_bayrak, (const uint8_t *)KAYIT_ISARET, (long)strlen(KAYIT_ISARET)), "isaret yaz 0");
	izlendi_t = mono();
	bayraklar(&c, &k, &kl);
	DENE(c == 1 && k == 0 && isaretli_bayrak(canli_bayrak), "inceleme: kayit bitince okuyan izleyici varken isaret kalir");
	izlendi_t = -1e9;
	/* v12: bos bayrak okunurken kalir, 60 sn okunmazsa silinir */
	DENE(!dosya_yaz(canli_bayrak, (const uint8_t *)"", 0), "bos bayrak yaz");
	bayraklar(&c, &k, &kl);
	DENE(c == 1 && var(canli_bayrak), "v12: yeni bos bayrak canli");
	bos_bayrak_t = mono() - 61;
	izlendi_t = mono() - 10;
	bayraklar(&c, &k, &kl);
	DENE(c == 1 && var(canli_bayrak), "v12: 60 sn icinde okunan bos bayrak kalir");
	izlendi_t = -1e9;
	bayraklar(&c, &k, &kl);
	DENE(c == 0 && !var(canli_bayrak) && !bos_bayrak_t, "v12: 60 sn okunmayan bos bayrak silinir");
	DENE(!dosya_yaz(canli_bayrak, (const uint8_t *)KAYIT_ISARET, (long)strlen(KAYIT_ISARET)), "isaret yaz");
	bayraklar(&c, &k, &kl);
	DENE(c == 0 && k == 0 && !var(canli_bayrak), "v10: kayit kapaninca yalniz kendi isaretli gostergemiz silinir");
	f = fopen(y, "w"); if (f) fclose(f);
	f = fopen(kalkan_yol, "w"); if (f) fclose(f);
	bayraklar(&c, &k, &kl);
	DENE(c == 0 && k == 0 && kl == 1, "kalkan her seyi bastirir");

	/* 9a) v10: son_alarm baslangici acilisin ilk 120 sn'sinde de alarm verir */
	bekci_n = 0;
	bekci_bas = 5;
	bekci_esik = 120;
	for (int i = 0; i < 6; i++) bekci_karar(20000, 20 + i);
	DENE(bekci_karar(30000, 26), "v10: acilistan 26 sn sonra ilk alarm (skor %d)", bekci_skor);

	/* 9) v3 bekci karari: sabit boy alarm vermez; isinmadan once vermez; sicrama verir; 120 sn tekrar yok */
	bekci_n = 0;
	bekci_bas = 1000;
	son_alarm = 0;
	bekci_esik = 120;
	int al = 0;
	for (int i = 0; i < 6; i++) al |= bekci_karar(20000, 1005 + i);
	DENE(!al, "isinma suresinde alarm yok");
	for (int i = 0; i < 6; i++) al |= bekci_karar(20000, 1020 + i);
	DENE(!al, "sabit boyda alarm yok (skor %d)", bekci_skor);
	DENE(bekci_karar(30000, 1030), "%%40 sicramada alarm (skor %d)", bekci_skor);
	son_alarm = 1030;
	DENE(!bekci_karar(18000, 1040), "120 sn icinde ikinci alarm yok");

	/* 9c) v11: kayit hareket penceresi ve hizi */
	{
		kh_n = 0;
		int h = 0;
		for (int i = 0; i < 6; i++) h |= kayit_hareket(20000, 120);
		DENE(!h, "v11: sabit boyda kayit hareketi yok");
		DENE(kayit_hareket(30000, 120), "v11: %%40 sicramada kayit hareketi");
		hareket_bitis = 100;
		DENE(kayit_hizi(99) == 8.0 && kayit_hizi(100.5) == KAYIT_FPS, "v11: hareket suresince 8, sonra %.1f", KAYIT_FPS);
		KAYIT_HAREKET_FPS = 0;
		DENE(kayit_hizi(99) == KAYIT_FPS, "v11: KAYIT_HAREKET_FPS=0 v10 davranisi");
		KAYIT_HAREKET_FPS = 8;
		double w8 = okuma_bekle(100, 1, 99.9, kayit_hizi(99), 0, 0, 0, 0, 60);
		DENE(w8 > 0.024 && w8 < 0.026, "v11: 8 kare/sn bekleme %.3f", w8);
		/* v13 (B11): 10 sn benzetim ana dongunun karar koduyla. Eski pencere hareketli dolu (onceki kayit) ama kayit basinda
		 * sifirlanir; 2. sn'ye kadar sabit sahne (~%0.5), sonra 20/30 KB donusumlu kareler (puan ~400) -> 20 sn hizli.
		 * Okuma 70 ms; yazim ~ 2 sn x 2 + 8 sn x 8. */
		hareket_bitis = 0;
		for (kh_n = 0; kh_n < BEKCI_PENCERE; kh_n++) kh_boy[kh_n] = kh_n & 1 ? 30000 : 20000;
		struct kayit_saat ks = { -1, -1, 0 };
		double t = 0, ilk_hareket = -1;
		int yazim = 0, i = 0;
		kayit_durum_guncelle(&ks, 1);
		DENE(kh_n == 0 && ks.onceki == 1, "v13: kayit basinda hareket penceresi sifirlanir");
		while (t < 10) {
			kayit_durum_guncelle(&ks, 1);
			double w = okuma_bekle(t, 1, ks.son_kayit, kayit_hizi(t), 0, 0, 0, 0, 60);
			if (w > 0.005) { t += w; continue; }
			double kt = t;
			t += 0.07;
			uint32_t boy = kt < 2 ? 20000 + (uint32_t)(i++ % 3) * 40 : (i++ & 1 ? 30000 : 20000);
			if (kayit_kare_karari(&ks, kt, boy)) { yazim++; ks.son_kayit = kt; }
			if (ilk_hareket < 0 && hareket_bitis > 0) ilk_hareket = kt;
		}
		printf("v13 benzetim: 10 sn (2. sn'de hareket) -> %d kare yazildi, hareket %.2f sn'de algilandi\n", yazim, ilk_hareket);
		DENE(ilk_hareket >= 2.0 && ilk_hareket < 2.6, "v13: hareket puani 2. sn'den sonra ilk ornekte (%.2f)", ilk_hareket);
		DENE(yazim >= 60 && yazim <= 72, "v13: hizli kayit yazim sayisi %d", yazim);
		kayit_durum_guncelle(&ks, 0);
		DENE(hareket_bitis == 0 && ks.onceki == 0, "v13: kayit kapaninca hizli kip biter");
		hareket_bitis = 0;
		kh_n = 0;
	}

	/* 9b) v9 time-lapse: ofset, olay csv, boyut siniri, dolu, FAT32 payi (gecici SD agaci) */
	{
		char zdz[300], zy[400];
		mkdir(kok, 0755);
		snprintf(zdz, sizeof zdz, "%s/zaman", kok);
		mkdir(zdz, 0755);
		test_ms = yerel_ms(2026, 10, 22, 10, 0, 0, 0);
		zaman_sinir_mb = 4096; zaman_dolu = 0; son_zaman_denetim = -1e9; zaman_bayt = -1;
		long long o1 = zaman_ekle(j[0], (long)jn[0]), o2 = zaman_ekle(j[1], (long)jn[1]);
		DENE(o1 == 0 && o2 == (long long)jn[0], "zaman ofset %lld %lld", o1, o2);
		DENE(zaman_bayt == (long long)(jn[0] + jn[1]), "zaman_bayt %lld", zaman_bayt);
		zo_dk = -1;
		zo_isle(30, -1); zo_isle(200, o2); zo_isle(250, -1); zo_isle(90, o1);
		test_ms = yerel_ms(2026, 10, 22, 10, 1, 5, 0);
		zo_isle(10, -1);
		zo_yaz();
		snprintf(zy, sizeof zy, "%s/20261022-olay.csv", zdz);
		FILE *zf = fopen(zy, "r");
		char zs[200] = "";
		if (zf) { size_t zn = fread(zs, 1, sizeof zs - 1, zf); zs[zn] = 0; fclose(zf); }
		char beklenen[120];
		snprintf(beklenen, sizeof beklenen, "1000,250,%lld\n1001,10,-1\n", o2);
		DENE(!strcmp(zs, beklenen), "olay csv '%s' != '%s'", zs, beklenen);
		/* boyut siniri: eski gunler silinir (csv ile), bugun kalir */
		const char *ad[3] = { "20261020.mjpg", "20261020-olay.csv", "20261021.mjpg" };
		long long boy3[3] = { 3 << 20, 100, 3 << 20 };
		for (int i = 0; i < 3; i++) {
			snprintf(zy, sizeof zy, "%s/%s", zdz, ad[i]);
			int zfd = open(zy, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			DENE(zfd >= 0 && !ftruncate(zfd, boy3[i]), "sahte gun %s", ad[i]);
			if (zfd >= 0) close(zfd);
		}
		zaman_sinir_mb = 4;
		zaman_boyut_denetle("20261022");
		char z20[400], z20c[400], z21[400], z22[400];
		snprintf(z20, sizeof z20, "%s/20261020.mjpg", zdz); snprintf(z20c, sizeof z20c, "%s/20261020-olay.csv", zdz);
		snprintf(z21, sizeof z21, "%s/20261021.mjpg", zdz); snprintf(z22, sizeof z22, "%s/20261022.mjpg", zdz);
		DENE(!var(z20) && !var(z20c) && var(z21) && var(z22) && !zaman_dolu, "en eski gun silindi, bugun kaldi");
		int zfd = open(z22, O_WRONLY);
		DENE(zfd >= 0 && !ftruncate(zfd, 5 << 20), "bugun 5 MB");
		if (zfd >= 0) close(zfd);
		zaman_boyut_denetle("20261022");
		DENE(!var(z21) && var(z22) && zaman_dolu, "bugun tek basina siniri asinca dolu (dun silindi)");
		son_zaman_denetim = gercek_ms() / 1000.0;
		DENE(zaman_ekle(j[0], (long)jn[0]) == -1, "dolu iken ekleme yok");
		/* FAT32 payi */
		zaman_dolu = 0;
		zfd = open(z22, O_WRONLY);
		DENE(zfd >= 0 && !ftruncate(zfd, ZAMAN_DOSYA_AZAMI - 10), "seyrek 3.9 GB");
		if (zfd >= 0) close(zfd);
		DENE(zaman_ekle(j[0], (long)jn[0]) == -1 && zaman_dolu, "FAT32 payinda ekleme durur");
		unlink(z22);
		test_ms = -1;
		zaman_sinir_mb = 4096; zaman_dolu = 0; zaman_bayt = -1;
	}

	/* 10) v5 saat sektoru: Melis bicimi, dogrulama, LBA siniri */
	{
		struct tm tm = { .tm_year = 126, .tm_mon = 9, .tm_mday = 15, .tm_hour = 2, .tm_min = 7, .tm_sec = 9 };
		uint8_t s[512];
		static const uint8_t bek[16] = { 0x55, 0xaa, 0x13, 0, 0, 0, 6, 0, 0, 0, 26, 10, 15, 2, 7, 9 };
		DENE(!saat_sektoru(s, &tm) && !memcmp(s, bek, 16) && saat_sektoru_dogru(s), "saat sektoru Melis bicimi");
		s[100] = 1;
		DENE(!saat_sektoru_dogru(s), "kirli sektor reddedilir");
		s[100] = 0;
		s[2] = 0x11;
		DENE(!saat_sektoru_dogru(s), "baska komut (0x11 FORMAT) reddedilir");
		s[2] = 0x13;
		s[11] = 13;
		DENE(!saat_sektoru_dogru(s), "gecersiz ay reddedilir");
		struct tm eski = { .tm_year = 117, .tm_mon = 4, .tm_mday = 1 };
		DENE(saat_sektoru(s, &eski), "2017 sistem saati gonderilmez");
		DENE(saat_lba_dogru(ELENE) && saat_lba_dogru(ELENE + 7 * YUVA) && !saat_lba_dogru(ELENE + 8 * YUVA) &&
		     !saat_lba_dogru(ELENE - 1) && !saat_lba_dogru(ELENE + 1) && !saat_lba_dogru(0), "LBA yalniz ELENE yuva basi");
		DENE(!saat_sektoru(s, &tm) && uyaz_saat(0, s) == -2, "uyaz_saat yanlis LBA'yi USB'ye gitmeden reddeder");	/* v10: 512 baytlik sektor (v9 16 baytlik bek ile tasiyordu, ASAN) */
	}

	/* 11) v10: yil < 2026 iken parca, time-lapse ve olay csv yazilmaz */
	{
		parca_kapat();
		azami_parca = 360;
		test_ms = yerel_ms(1970, 1, 2, 3, 4, 5, 0);
		char g70[600], z70[600];
		snprintf(g70, sizeof g70, "%s/19700102", kok);
		snprintf(z70, sizeof z70, "%s/zaman/19700102.mjpg", kok);
		DENE(!saat_gecerli() && parca_yaz(j[0], (long)jn[0]) == -2 && !parca && !var(g70), "v10: 1970 kayit parcasi reddedilir");
		DENE(zaman_ekle(j[0], (long)jn[0]) == -1 && !var(z70), "v10: 1970 time-lapse karesi reddedilir");
		zo_dk = -1;
		zo_isle(500, 0);
		DENE(zo_dk < 0, "v10: 1970 olay dakikasi acilmaz");
		test_ms = yerel_ms(2026, 1, 1, 0, 0, 0, 0);
		DENE(saat_gecerli(), "2026 gecerli");
	}

	/* 12) v10: kare yazmasi yarim kalirsa parca kapanir; yeniden acilista .idx gercek son ofsetten surer (RLIMIT_FSIZE) */
	{
		char py[600], piy[600];
		snprintf(py, sizeof py, "%s/20261014/1000.mjpg", kok);
		idx_yolu(piy, sizeof piy, "20261014/1000.mjpg");
		test_ms = yerel_ms(2026, 10, 14, 10, 0, 1, 0);
		DENE(!parca_yaz(j[0], (long)jn[0]), "yarim: ilk kare");
		struct rlimit eski, yeni;
		getrlimit(RLIMIT_FSIZE, &eski);
		yeni = eski;
		yeni.rlim_cur = (rlim_t)(jn[0] + jn[3] / 2);
		signal(SIGXFSZ, SIG_IGN);
		setrlimit(RLIMIT_FSIZE, &yeni);
		test_ms += 500;
		int yr = parca_yaz(j[3], (long)jn[3]);
		int acik = parca != NULL;
		setrlimit(RLIMIT_FSIZE, &eski);
		signal(SIGXFSZ, SIG_DFL);
		long long yarim = dosya_boyut(py);
		DENE(yr == -1 && !acik, "v10: yarim kare -> -1 ve parca kapandi (%d %d)", yr, acik);
		DENE(yarim > (long long)jn[0] && yarim < (long long)(jn[0] + jn[3]), "yarim bayt diskte (%lld)", yarim);
		test_ms += 500;
		DENE(!parca_yaz(j[1], (long)jn[1]), "yarimdan sonra kare");
		parca_kapat();
		static struct kidx kp[8];
		int pn = idx_oku(piy, (uint64_t)dosya_boyut(py), kp, 8);
		DENE(pn == 2 && kp[0].ofs == 0 && kp[1].ofs == (uint32_t)yarim && kp[1].len == jn[1], "v10: idx gercek ofset (pn %d ofs %u)",
		     pn, pn > 1 ? kp[1].ofs : 0);
		f = fopen(py, "rb");
		size_t r = 0;
		if (f && pn > 1 && !fseek(f, (long)kp[1].ofs, SEEK_SET)) r = fread(okunan, 1, kp[1].len, f);
		if (f) fclose(f);
		DENE(r == jn[1] && !memcmp(okunan, j[1], r), "v10: idx karesi dogru icerik");
	}

	/* 13) v10: telegram fotosu kuyruga SD yolu ile (/tmp kopyasi yok); kuyrukta 50'den fazla .txt varsa eklenmez */
	{
		static char kq[300];
		snprintf(kq, sizeof kq, "%s/kuyruk", sab);
		kuyruk_dz = kq;
		test_ms = yerel_ms(2026, 10, 23, 9, 8, 7, 0);
		foto_kaydet(j[0], (long)jn[0], "bekci", "test", 1);
		char fy[600], ky[700] = "", ks[800] = "", bek2[800];
		snprintf(fy, sizeof fy, "%s/fotolar/bekci-20261023-090807.jpg", kok);
		DENE(var(fy) && kuyruk_say() == 1, "v10: SD fotosu + 1 kuyruk ogesi (%d)", kuyruk_say());
		DIR *qd = opendir(kq);
		struct dirent *qe;
		while (qd && (qe = readdir(qd)))
			if (strstr(qe->d_name, ".txt")) snprintf(ky, sizeof ky, "%s/%s", kq, qe->d_name);
		if (qd) closedir(qd);
		FILE *qf = fopen(ky, "r");
		if (qf) { size_t qn = fread(ks, 1, sizeof ks - 1, qf); ks[qn] = 0; fclose(qf); }
		snprintf(bek2, sizeof bek2, "foto:%s\ntest\n", fy);
		DENE(!strcmp(ks, bek2), "v10: kuyruk '%s' != '%s'", ks, bek2);
		DENE(!var("/tmp/kamera-bekci-20261023-090807.jpg"), "v10: /tmp kopyasi yok");
		char yab[400];
		snprintf(yab, sizeof yab, "%s/1-panel.txt", kq);	/* baska ureticinin ogesi: dokunulmaz */
		DENE(!dosya_yaz(yab, (const uint8_t *)"p\n", 2), "yabanci oge");
		for (int i = 0; kuyruk_say() <= KUYRUK_AZAMI && i < 100; i++) DENE(!kuyruk_yaz("x", NULL), "kuyruk %d", i);
		DENE(kuyruk_say() == KUYRUK_AZAMI + 1 && !kuyruk_yaz("y", NULL) && kuyruk_say() == KUYRUK_AZAMI + 1 && !var(ky) && var(yab),
		     "inceleme: kuyruk dolu -> kendi en eski (bekci) silinir, yeni yazilir, yabanci kalir (%d)", kuyruk_say());
		kuyruk_dz = "/tmp/bildirim-kuyruk";
	}

	/* 14) v10: fotolar/ zaman damgasina gore temizlenir: 300 eski foto-* + yeni bekci-* -> en eski foto-* silinir */
	{
		char fd_[300], fy[600];
		snprintf(fd_, sizeof fd_, "%s/foto-sira", sab);
		mkdir(fd_, 0755);
		for (int i = 0; i < 300; i++) {
			snprintf(fy, sizeof fy, "%s/foto-20261001-%06d.jpg", fd_, i);
			int t = open(fy, O_WRONLY | O_CREAT, 0644);
			if (t >= 0) close(t);
		}
		snprintf(fy, sizeof fy, "%s/bekci-20261023-000000.jpg", fd_);
		int t = open(fy, O_WRONLY | O_CREAT, 0644);
		if (t >= 0) close(t);
		fotolar_temizle(fd_);
		char f0[600], f1[600];
		snprintf(f0, sizeof f0, "%s/foto-20261001-000000.jpg", fd_);
		snprintf(f1, sizeof f1, "%s/foto-20261001-000001.jpg", fd_);
		int say = 0;
		DIR *dd = opendir(fd_);
		struct dirent *e;
		while (dd && (e = readdir(dd))) say += jpg_adi(e);
		if (dd) closedir(dd);
		DENE(var(fy) && !var(f0) && var(f1) && say == 300, "v10: yeni bekci kaldi, en eski foto silindi (%d)", say);
	}

	/* 15) v10: gunluk kirpma (O_APPEND: sonraki yazim 0'dan); kucuk ya da duzenli olmayan dosyaya dokunulmaz */
	{
		char gy[300];
		snprintf(gy, sizeof gy, "%s/gunluk.log", sab);
		int gfd = open(gy, O_WRONLY | O_CREAT | O_APPEND, 0644);
		static char dolgu[4096];
		memset(dolgu, 'a', sizeof dolgu);
		for (int i = 0; i < 70; i++) if (write(gfd, dolgu, sizeof dolgu) != (ssize_t)sizeof dolgu) break;
		DENE(gunluk_kirp(gfd, GUNLUK_AZAMI) == 70 * 4096 && dosya_boyut(gy) == 0, "v10: 280 KB gunluk kirpildi");
		DENE(write(gfd, "x\n", 2) == 2 && dosya_boyut(gy) == 2 && !gunluk_kirp(gfd, GUNLUK_AZAMI), "v10: kirpma sonrasi 0'dan");
		close(gfd);
		int bp[2];
		DENE(!pipe(bp) && !gunluk_kirp(bp[1], 0), "v10: boru kirpilmaz");
		close(bp[0]);
		close(bp[1]);
	}
	test_ms = -1;

	/* 8) durum JSON'u */
	FILE *dj = tmpfile();
	durum_yaz(dj, 0, 1, 1, 2.0, 5, "x");
	rewind(dj);
	char djs[1024] = "";
	if (!fgets(djs, sizeof djs, dj)) djs[0] = 0;
	fclose(dj);
	DENE(strstr(djs, "\"surum\":\"kamera-kayit-v12\"") && strstr(djs, "\"kayit_hareket\":") && strstr(djs, "\"bekci\":") && strstr(djs, "\"kalkan\":true") && strstr(djs, "\"son_skor\":") &&
	     strstr(djs, "\"zaman_olay\":false") && strstr(djs, "\"zaman_sinir_mb\":4096") && strstr(djs, "\"zaman_dolu\":false"), "durum %s", djs);

	char sil[400];
	snprintf(sil, sizeof sil, "rm -rf '%s'", sab);
	if (system(sil)) puts("uyari: gecici klasor silinemedi");
	if (hata_say) { printf("SELF-TEST FAIL (%d)\n", hata_say); return 1; }
	puts("SELF-TEST PASS: kirpma, jpeg_son, okuma_bekle (v8), olay puani, parca+idx+olay.csv, indeksle, saklama, kalkan, "
	     "v9 time-lapse (ofset, olay csv, boyut siniri, dolu, FAT32 payi), saat sektoru; v10: isaretli kayit gostergesi, ilk alarm, "
	     "saat<2026 reddi, yarim kare kapatma, SD foto kuyrugu + kuyruk siniri, foto zaman sirasi, gunluk kirpma; "
	     "v11: kayit hareketi + 8 kare/sn; v12: bos bayrak 60 sn; USB/kamera acilmadi");
	return 0;
}

int main(int argc, char **argv)
{
	if (argc > 1 && !strcmp(argv[1], "--self-test")) return self_test();
	if (argc > 1 && !strcmp(argv[1], "--durum-ornek")) { durum_yaz(stdout, 1, 1, 0, 7.5, 42, ""); return 0; }
	if (argc > 1 && !strcmp(argv[1], "--indeksle")) { printf("%d parca indekslendi\n", indeksle()); return 0; }
	if (getenv("KAYIT_FPS")) KAYIT_FPS = atof(getenv("KAYIT_FPS"));
	if (getenv("KAYIT_HAREKET_FPS")) KAYIT_HAREKET_FPS = atof(getenv("KAYIT_HAREKET_FPS"));	/* v11; 0 = kapali */
	if (getenv("KAYIT_HAREKET_SN")) KAYIT_HAREKET_SN = atof(getenv("KAYIT_HAREKET_SN"));
	setvbuf(stdout, NULL, _IOLBF, 0);
	FILE *pf = fopen("/tmp/kamera-kayit.pid", "w");
	if (pf) { fprintf(pf, "%d\n", getpid()); fclose(pf); }
	printf("kamera-kayit v12 basladi (bos bayrak 60 sn; hareket varken %.0f kare/sn %.0f sn; ", KAYIT_HAREKET_FPS, KAYIT_HAREKET_SN);
	printf("v10: SD foto kuyrugu, saat<2026 kayit yok, isaretli kayit gostergesi; bekci/foto/zaman atlamali 1..3600 sn + olay tespiti + boyut siniri/kamera saati/USB kurtarma) ");
	printf("kamera-kayit v2 tabani (kayit %.1f kare/sn, %d parca, %d MB bos alt sinir, yan dizin + olay puani)\n",
		KAYIT_FPS, azami_parca, ASGARI_BOS_MB);
	signal(SIGTERM, cik_sinyal);	/* v6 */
	signal(SIGINT, cik_sinyal);
	signal(SIGPIPE, SIG_IGN);
	usb_sifirla("acilis");	/* onceki sahip aktarim ortasinda oldurulmus olabilir */
	int ardisik_hata = 0;
	double son_durum = 0, fps_t = mono();
	struct kayit_saat ks = { 0, 0, 0 };	/* v13 (B11) */
	unsigned long kareler = 0, fps_say = 0;
	double fps = 0;
	const char *hata = NULL;
	for (;;) {
		if (cik_iste) {
			ukapat(); parca_kapat(); zo_yaz();
			if (isaretli_bayrak(canli_bayrak)) unlink(canli_bayrak);	/* inceleme: kendi gostergemiz kalmasin */
			puts("kamera-kayit: sinyal, temiz cikis");
			return 0;
		}
		int canli, kayit, kalkan;
		bayraklar(&canli, &kayit, &kalkan);
		double simdi = mono();
		kayit_durum_guncelle(&ks, kayit);	/* v13 (B11) */
		/* v3 bayraklari (kalkan hepsini bastirir) */
		int bekci_bayrak = !kalkan && var(bekci_yol()), zaman = !kalkan && var(zaman_yol());
		zaman_olay = zaman && var(zaman_olay_yol());	/* v9 */
		int bekci = bekci_bayrak || zaman_olay;	/* v9: olay tespiti bekci ornekleme + Telegram uyarisini kullanir */
		if (!zaman_olay && zo_dk >= 0) zo_yaz();
		int foto = !kalkan && var("/tmp/kamera-foto-iste");
		int saat_iste = !kalkan && var("/tmp/kamera-saat-ayarla");	/* v5 */
		if (kalkan && var("/tmp/kamera-foto-iste")) unlink("/tmp/kamera-foto-iste");
		if (bekci && !bekci_bas) {
			bekci_bas = simdi;
			bekci_n = 0;
			bekci_esik = sayi_oku("/mnt/sd/kamera/bekci-esik", 120, 20, 1000);
			int fd = open("/tmp/kamera-bekci", O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
			if (fd >= 0) close(fd);
			printf("bekci acik (esik %d)\n", bekci_esik);
		} else if (!bekci && bekci_bas) {
			bekci_bas = 0;
			bekci_skor = -1;
			unlink("/tmp/kamera-bekci");
			puts("bekci kapali");
		}
		if (zaman) zaman_aralik = sayi_oku("/mnt/sd/kamera/zaman-aralik", 60, 1, 3600);	/* v9: 1 sn */
		if (simdi - son_durum >= 1) {
			if (simdi - fps_t >= 1) { fps = fps_say / (simdi - fps_t); fps_say = 0; fps_t = simdi; }
			durum_dosya(canli, kayit, kalkan, fps, kareler, hata);
			son_durum = simdi;
			long long kb = gunluk_kirp(1, GUNLUK_AZAMI);	/* v10 */
			if (kayit) kayit_esik = sayi_oku("/mnt/sd/kamera/bekci-esik", 120, 20, 1000);	/* v12: esik degisikligi hemen */
			if (kb) printf("kamera-kayit: gunluk %lld KB idi, kirpildi\n", kb / 1024);
		}
		if (!canli && !kayit && !bekci && !foto && !saat_iste && !(zaman && simdi - son_zaman >= zaman_aralik - 1)) {
			if (ufd >= 0) { ukapat(); puts(kalkan ? "kalkan: USB birakildi" : "bosta: USB birakildi"); }
			parca_kapat();
			usleep(300000);
			continue;
		}
		if (ufd < 0 && uac()) { hata = "kamera USB acilamadi"; usleep(2000000); continue; }
		if (!var("/mnt/sd/kamera/saat-ayar-kapali") && (saat_iste || simdi - saat_ayar_mono >= 6 * 3600.0)) {	/* v5: ilk, 6 saatte bir, istek */
			saat_esitle(saat_iste ? "istek" : saat_ayar_unix ? "6 saat" : "ilk");
			if (saat_iste) unlink("/tmp/kamera-saat-ayarla");
		} else if (saat_iste) unlink("/tmp/kamera-saat-ayarla");
		if (!canli && !foto) {	/* v8: kare yalniz kayit/bekci/zaman zamani gelince */
			double w = okuma_bekle(mono(), kayit, ks.son_kayit, kayit_hizi(mono()), bekci, son_bekci_kare, zaman, son_zaman, zaman_aralik);
			if (w > 0.005) {
				usleep((useconds_t)(w * 1e6));
				continue;
			}
		}
		double kt = mono();	/* v8: kare zamani = okuma basi (v7 yazimdan sonraki saati aliyordu: 2 yerine ~1.7 kare/sn) */
		long n = kare_al();
		if (n < 0) {
			hata = "USB okuma hatasi";
			ukapat();
			if (++ardisik_hata >= 2) { usb_sifirla("ardisik okuma hatasi"); ardisik_hata = 0; }
			usleep(1000000);
			continue;
		}
		ardisik_hata = 0;
		if (!n) { usleep(100000); continue; }	/* v10: gecerli kare yok: uyumadan donme */
		hata = (kayit || zaman) && !saat_gecerli() ? "saat gecersiz" : NULL;	/* v10 */
		kareler++;
		fps_say++;
		if (canli) son_kare_yaz(b + 17, n);
		if (foto) {	/* v3: tek kare */
			char ic[32] = "";
			FILE *ff = fopen("/tmp/kamera-foto-iste", "r");
			if (ff) { if (!fgets(ic, sizeof ic, ff)) ic[0] = 0; fclose(ff); }
			unlink("/tmp/kamera-foto-iste");
			foto_kaydet(b + 17, n, "foto", "📸 VeLinux KÖLE fotoğrafı", !strncmp(ic, "telegram", 8));
		}
		if (bekci && kt - son_bekci_kare >= 0.495) {	/* v3: ~2 kare/sn hareket puani (v8: okuma basi saati) */
			son_bekci_kare = kt;
			if (bekci_karar((uint32_t)n, son_bekci_kare)) {
				son_alarm = son_bekci_kare;
				son_alarm_unix = (long)(gercek_ms() / 1000);
				char t[128], ts[32];
				zaman_damgasi(ts, sizeof ts);
				snprintf(t, sizeof t, "🚨 Hareket algılandı efendim (%.2s:%.2s:%.2s, puan %d)%s", ts + 9, ts + 11, ts + 13, bekci_skor,
					 bekci_bayrak ? "" : " · time-lapse olay tespiti");	/* v9 */
				foto_kaydet(b + 17, n, "bekci", t, 1);
			}
			if (zaman_olay && bekci_skor >= 0) {	/* v9: dakika puani; esik ustunde ek time-lapse karesi (en cok 1/sn) */
				long long eo = -1;
				if (bekci_skor >= bekci_esik && kt - son_olay_ek >= 1.0 && kt - son_zaman >= 0.9) {
					eo = zaman_ekle(b + 17, n);
					if (eo >= 0) son_olay_ek = kt;
				}
				zo_isle(bekci_skor, eo);
			}
		}
		if (zaman && kt - son_zaman >= zaman_aralik - 0.005) {	/* v3: zaman atlamali (v8: okuma basi saati) */
			son_zaman = kt;
			long long zo2 = zaman_ekle(b + 17, n);	/* v9: ofset */
			if (zaman_olay) zo_isle(bekci_skor < 0 ? 0 : bekci_skor, zo2);
		}
		if (kayit && kayit_kare_karari(&ks, kt, (uint32_t)n)) {	/* v13 (B11): ornek + yazma zamani; v8/v11 */
			int pr = parca_yaz(b + 17, n);
			if (pr) hata = pr == -2 ? "saat gecersiz" : "SD yazma hatasi";	/* v10 */
			if (!canli) son_kare_yaz(b + 17, n);	/* v10: isaretli gosterge: canli izleyici kayit karelerini gorur */
			ks.son_kayit = kt;
		}
		if (!kayit) parca_kapat();
		if (!canli && !(kayit && kayit_hizi(mono()) > KAYIT_FPS))	/* v11: hizli kayitta okuma_bekle yeter */
			usleep(bekci || kayit ? 50000 : 200000);	/* yalniz kayit/bekci: gereksiz hiz yok */
	}
}
