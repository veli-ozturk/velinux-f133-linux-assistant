/* kamera_akis_v2.h (24.10.2026, Claude; scripts/624, denetim-2026-10-24-v3 A18e): v1 + dogrudan USB dongusu saniyede bir
 * hizmet_var() bakar; kamera-kayit (kayitci) geri geldiyse USB arayuzu ona birakilir ve akis kalan surece
 * /tmp/kamera-son.jpg'den surer (v1 bir akis kamerayi 2 saate kadar kayitcidan alabiliyordu). --- v1 --- */
/* kamera_akis_v1.h (13.10.2026, Claude): canli kamera kare kaynagi (kole-kopru v7 KAMERA_AKIS kodu, BIREBIR).
 * Kamera erisimi YALNIZ OKUMA: usbdevfs + SCSI READ(10) (0x28); kameraya hicbir komut/veri yazilmaz.
 * kamera-kayit hizmeti calisiyorsa USB'ye hic dokunulmaz, /tmp/kamera-son.jpg degistikce okunur.
 * Cikti: [u32 LE n][JPEG] tekrar; hata durumunda "HATA ...\n". /tmp/kamera-aktif yoksa ya da gizlilik kalkani
 * varsa durur. kamera_sahte_jpeg (rtsp-test) verilirse USB/hizmet yerine o dosya tekrar tekrar gonderilir.
 * Kullananlar: kole-kopru v8 KAMERA_AKIS, RTSP video kaynagi, ileride cihaz-ici web merkezi. */
#ifndef KAMERA_AKIS_V1_H
#define KAMERA_AKIS_V1_H
#define KAMERA_AKIS_V2_H	/* v1 yerine: sonraki v1 dahil etmeleri bos gecer */
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>
#include "kopru_ortak_v1.h"
#include "kamera_olay_v1.h"

static const char *kamera_sahte_jpeg = NULL;
static int ufd = -1;
static uint32_t uetiket = 0x4b4f4c45;

static int ubulk(unsigned ep, void *b, unsigned n)
{
	struct usbdevfs_bulktransfer t = { .ep = ep, .len = n, .timeout = 3000, .data = b };
	int r = ioctl(ufd, USBDEVFS_BULK, &t);
	if (r < 0 && errno == EPIPE) {
		unsigned e = ep;
		ioctl(ufd, USBDEVFS_CLEAR_HALT, &e);
	}
	return r;
}

/* yalniz READ(10): lba'dan k sektor (k<=32) */
static int uoku(uint32_t lba, unsigned k, uint8_t *veri)
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

#define ELENE 4351u
#define YUVA 600u	/* 0x4B000 / 512 */
#define YUVA_SAYI 8u

static int hizmet_var(void)
{
	char b2[32];
	if (dosya_oku(kayit_pid, b2, sizeof b2) <= 0) return 0;
	char yol[48];
	snprintf(yol, sizeof yol, "/proc/%d/comm", atoi(b2));
	return dosya_oku(yol, b2, sizeof b2) > 0 && !strncmp(b2, "kamera-kayit", 12);
}

static int kare_gonder(int s, const uint8_t *j, uint32_t n)
{
	uint8_t h[4] = { (uint8_t)n, (uint8_t)(n >> 8), (uint8_t)(n >> 16), (uint8_t)(n >> 24) };
	return tam_yaz(s, h, 4) || tam_yaz(s, j, n);
}

/* kamera-kayit'in yazdigi son kareyi degistikce akit (v6/v7 ile ayni) */
static int kamera_akis_dosya(int s, double sn, double fps)
{
	struct stat st;
	double bitis = mono() + (sn > 0 && sn <= 7200 ? sn : 600), aralik_ = fps > 0 ? 1.0 / fps : 0.2, son = 0;
	ino_t son_ino = 0;
	struct timespec son_t = { 0, 0 };
	static uint8_t j[300000];
	unsigned kareler = 0;
	while (mono() < bitis && !stat(kamera_bayrak, &st) && !dosya_var(kalkan_yol)) {
		if (stat(kamera_son, &st) || (st.st_ino == son_ino && st.st_mtim.tv_sec == son_t.tv_sec &&
		    st.st_mtim.tv_nsec == son_t.tv_nsec) || mono() - son < aralik_) {
			struct timespec d = { .tv_nsec = 20000000 };
			nanosleep(&d, NULL);
			continue;
		}
		int fd = open(kamera_son, O_RDONLY | O_CLOEXEC);
		if (fd < 0) continue;
		ssize_t n = read(fd, j, sizeof j);
		close(fd);
		son_ino = st.st_ino;
		son_t = st.st_mtim;
		if (n < 1000) continue;
		if (kare_gonder(s, j, (uint32_t)n)) break;
		son = mono();
		kareler++;
	}
	printf("KAMERA_AKIS (hizmet) bitti: %u kare\n", kareler);
	return 0;
}

/* rtsp-test: dosyadaki JPEG'i fps hizinda tekrarla (kamera/USB yok) */
static int kamera_akis_sahte(int s, double sn, double fps)
{
	static uint8_t j[300000];
	int fd = open(kamera_sahte_jpeg, O_RDONLY | O_CLOEXEC);
	if (fd < 0) return hata(s, "sahte jpeg yok");
	ssize_t n = read(fd, j, sizeof j);
	close(fd);
	if (n < 4) return hata(s, "sahte jpeg bos");
	double bitis = mono() + (sn > 0 ? sn : 600), ara = fps > 0 ? 1.0 / fps : 0.2;
	struct stat st;
	while (mono() < bitis && !stat(kamera_bayrak, &st) && !dosya_var(kalkan_yol)) {
		if (kare_gonder(s, j, (uint32_t)n)) break;
		struct timespec d = { (time_t)ara, (long)((ara - (time_t)ara) * 1e9) };
		nanosleep(&d, NULL);
	}
	return 0;
}

static int kamera_akis(int s, double sn, double fps)
{
	struct stat st;
	if (dosya_var(kalkan_yol)) return hata(s, "kalkan acik");
	if (stat(kamera_bayrak, &st)) { tam_yaz(s, "HATA kamera kapali\n", 19); return 2; }
	if (kamera_sahte_jpeg) return kamera_akis_sahte(s, sn, fps);
	if (hizmet_var()) return kamera_akis_dosya(s, sn, fps);
	if (uac()) { tam_yaz(s, "HATA kamera USB\n", 16); return 1; }
	static uint8_t b[YUVA * 512];
	double bitis = mono() + (sn > 0 && sn <= 7200 ? sn : 600), aralik_ = fps > 0 ? 1.0 / fps : 0.2, son = 0, hz_t = mono();
	unsigned y = 0, kareler = 0, bos = 0;
	int rc = 0, devret = 0;
	while (mono() < bitis && !stat(kamera_bayrak, &st) && !dosya_var(kalkan_yol)) {
		if (mono() - hz_t >= 1) {	/* v2: kayitci geri geldiyse USB ona birakilir */
			hz_t = mono();
			if (hizmet_var()) { devret = 1; break; }
		}
		uint32_t l = ELENE + y * YUVA;
		/* yuvayi sirayla oku: kamera kareyi sirali okumayla yaziyor */
		unsigned okunan = 0;
		uint32_t sz = 0;
		int gecerli = 0;
		while (okunan < YUVA) {
			unsigned k = YUVA - okunan < 32 ? YUVA - okunan : 32;
			if (uoku(l + okunan, k, b + okunan * 512) != (int)(k * 512)) { rc = 1; goto bitti; }
			if (!okunan) {
				sz = b[0] | b[1] << 8 | b[2] << 16 | (uint32_t)b[3] << 24;
				gecerli = sz >= 1024 && sz <= 256000 && b[17] == 0xff && b[18] == 0xd8;
			}
			okunan += k;
			if (gecerli && okunan * 512 >= sz + 17) break;	/* kare tamam; yuvanin kalanini atla */
		}
		if (gecerli) {
			long e = jpeg_son_c(b + 17, sz);
			uint32_t n = sz;
			if (e > 0) n = (uint32_t)e;
			else {
				while (n > 2 && !(b[17 + n - 2] == 0xff && b[17 + n - 1] == 0xd9)) n--;	/* son FF D9'a kirp */
				if (n <= 2) n = sz;
			}
			double simdi = mono();
			if (simdi - son >= aralik_) {
				if (kare_gonder(s, b + 17, n)) goto bitti;	/* istemci kapatti */
				son = simdi;
				kareler++;
			}
			bos = 0;
		} else if (++bos > 64) {
			rc = 3;	/* kamera kare uretmiyor */
			break;
		}
		y = (y + 1) % YUVA_SAYI;
	}
bitti:
	ukapat();
	memset(b, 0, sizeof b);
	printf("KAMERA_AKIS bitti: %u kare, rc=%d%s\n", kareler, rc, devret ? " (kayitci geri geldi: dosyadan devam)" : "");
	if (devret && bitis - mono() > 1) return kamera_akis_dosya(s, bitis - mono(), fps);	/* v2 */
	return rc;
}
#endif
