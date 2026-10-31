/* kamera_saat_tani_v1 (15.10.2026, Claude): kamera saati yazmasi neden reddediliyor? (kamera-kayit v5 ilk denemede
 * "USB yazma hatasi" verdi; ardindan BOT akisi kaydi, 1-1/authorized 0/1 ile kurtarildi.)
 *   kamera-saat-tani            : TEST UNIT READY + MODE SENSE(6) (yazma korumasi biti) + READ(10) 1 sektor. Yazma YOK.
 *   kamera-saat-tani yaz        : yukaridakiler + TEK saat sektoru WRITE(10) (Melis cmd 0x13), her asama raporlanir;
 *                                 veri asamasi takilirsa CLEAR_HALT + CSW okunur (BOT 6.7), sonra REQUEST SENSE.
 * Kamera-kayit calisirken KULLANMA (tek USB sahibi). Baska hicbir kamera komutu yazilmaz. */
#include <errno.h>
#include <fcntl.h>
#include <linux/usbdevice_fs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define ELENE 4351u
static int ufd = -1;
static uint32_t etiket = 0x7a000000;

static int bulk(unsigned ep, void *v, unsigned n, int *hata)
{
	struct usbdevfs_bulktransfer t = { .ep = ep, .len = n, .timeout = 3000, .data = v };
	int r = ioctl(ufd, USBDEVFS_BULK, &t);
	*hata = r < 0 ? errno : 0;
	if (r < 0 && errno == EPIPE) {
		unsigned e = ep;
		ioctl(ufd, USBDEVFS_CLEAR_HALT, &e);
	}
	return r;
}

/* tek SCSI komutu; yon 0x80 IN / 0x00 OUT. Donus CSW durumu (0/1/2) ya da -1 (CSW yok) */
static int komut(const uint8_t *cb, int cbn, int yon, uint8_t *veri, unsigned n, const char *ad)
{
	uint8_t cbw[31] = { 'U', 'S', 'B', 'C' }, csw[13];
	int h, r;
	etiket++;
	memcpy(cbw + 4, &etiket, 4);
	memcpy(cbw + 8, &n, 4);
	cbw[12] = (uint8_t)yon;
	cbw[14] = (uint8_t)cbn;
	memcpy(cbw + 15, cb, (size_t)cbn);
	r = bulk(0x02, cbw, 31, &h);
	printf("%s: CBW %d (errno %d)\n", ad, r, h);
	if (r != 31)
		return -1;
	if (n) {
		r = bulk(yon ? 0x81 : 0x02, veri, n, &h);
		printf("%s: veri %d/%u (errno %d%s)\n", ad, r, n, h, h == EPIPE ? " STALL, halt temizlendi" : "");
	}
	r = bulk(0x81, csw, 13, &h);
	if (r < 0 && h == EPIPE)
		r = bulk(0x81, csw, 13, &h);
	if (r != 13 || memcmp(csw, "USBS", 4)) {
		printf("%s: CSW YOK (%d, errno %d)\n", ad, r, h);
		return -1;
	}
	uint32_t et, kalan;
	memcpy(&et, csw + 4, 4);
	memcpy(&kalan, csw + 8, 4);
	printf("%s: CSW durum %u, kalan %u, etiket %s\n", ad, csw[12], kalan, et == etiket ? "dogru" : "YANLIS");
	return csw[12];
}

static void sense(void)
{
	uint8_t cb[6] = { 0x03, 0, 0, 0, 18, 0 }, s[18] = { 0 };
	if (komut(cb, 6, 0x80, s, 18, "REQUEST SENSE") == 0)
		printf("SENSE: anahtar 0x%x ASC 0x%02x ASCQ 0x%02x%s\n", s[2] & 15, s[12], s[13],
		       (s[2] & 15) == 7 ? " (DATA PROTECT = yazma korumali)" : "");
}

int main(int argc, char **argv)
{
	int yaz = argc > 1 && !strcmp(argv[1], "yaz");
	char yol[64];
	int bus = 0, dev = 0;
	FILE *f = fopen("/sys/bus/usb/devices/1-1/busnum", "r");
	if (!f || fscanf(f, "%d", &bus) != 1)
		return 2;
	fclose(f);
	f = fopen("/sys/bus/usb/devices/1-1/devnum", "r");
	if (!f || fscanf(f, "%d", &dev) != 1)
		return 2;
	fclose(f);
	snprintf(yol, sizeof yol, "/dev/bus/usb/%03d/%03d", bus, dev);
	ufd = open(yol, O_RDWR | O_CLOEXEC);
	unsigned a = 0;
	if (ufd < 0 || ioctl(ufd, USBDEVFS_CLAIMINTERFACE, &a)) {
		printf("USB acilamadi (kamera-kayit calisiyor olabilir)\n");
		return 2;
	}
	uint8_t tur[6] = { 0 };
	if (komut(tur, 6, 0x80, NULL, 0, "TEST UNIT READY"))
		sense();
	uint8_t ms[6] = { 0x1a, 0, 0x3f, 0, 192, 0 }, mv[192] = { 0 };
	if (komut(ms, 6, 0x80, mv, 192, "MODE SENSE(6)") == 0)
		printf("MODE SENSE: uzunluk %u, ortam %02x, aygit parametresi %02x -> yazma korumasi %s\n", mv[0], mv[1], mv[3],
		       mv[2] & 0x80 ? "VAR" : "yok");
	else
		sense();
	uint8_t rd[10] = { 0x28, 0, (uint8_t)(ELENE >> 24), (uint8_t)(ELENE >> 16), (uint8_t)(ELENE >> 8), (uint8_t)ELENE, 0, 0, 1, 0 };
	uint8_t s[512];
	if (komut(rd, 10, 0x80, s, 512, "READ(10) ELENE") == 0)
		printf("ELENE ilk baytlar: %02x %02x %02x %02x ... [17]=%02x [18]=%02x\n", s[0], s[1], s[2], s[3], s[17], s[18]);
	if (yaz) {
		time_t t = time(NULL);
		struct tm tm;
		localtime_r(&t, &tm);
		if (tm.tm_year + 1900 < 2026) {
			printf("sistem saati gecersiz, yazilmadi\n");
		} else {
			memset(s, 0, sizeof s);
			static const uint8_t bas[8] = { 0x55, 0xaa, 0x13, 0x00, 0x00, 0x00, 0x06, 0x00 };
			memcpy(s, bas, 8);
			s[10] = (uint8_t)(tm.tm_year + 1900 - 2000);
			s[11] = (uint8_t)(tm.tm_mon + 1);
			s[12] = (uint8_t)tm.tm_mday;
			s[13] = (uint8_t)tm.tm_hour;
			s[14] = (uint8_t)tm.tm_min;
			s[15] = (uint8_t)tm.tm_sec;
			uint32_t lba = ELENE + 600u;	/* siradaki 0x4B000 yuvasi */
			uint8_t wr[10] = { 0x2a, 0, (uint8_t)(lba >> 24), (uint8_t)(lba >> 16), (uint8_t)(lba >> 8), (uint8_t)lba, 0, 0, 1, 0 };
			printf("saat sektoru: 20%02u-%02u-%02u %02u:%02u:%02u LBA %u\n", s[10], s[11], s[12], s[13], s[14], s[15], lba);
			if (komut(wr, 10, 0x00, s, 512, "WRITE(10) saat"))
				sense();
			if (komut(tur, 6, 0x80, NULL, 0, "TEST UNIT READY (sonra)"))
				sense();
			if (komut(rd, 10, 0x80, s, 512, "READ(10) ELENE (sonra)") == 0)
				printf("ELENE ilk baytlar: %02x %02x %02x %02x %02x %02x ... [17]=%02x\n", s[0], s[1], s[2], s[3], s[4], s[5], s[17]);
		}
	}
	ioctl(ufd, USBDEVFS_RELEASEINTERFACE, &a);
	close(ufd);
	return 0;
}
