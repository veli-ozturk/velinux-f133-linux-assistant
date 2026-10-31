/* usb_bot_oku v3 (12.10.2026, Claude): v2 + kamera ELENE kanali (docs/kamera-elene-protokol-20261012.md):
 *   tara <lba> <adet>            : sektor sektor oku; 55AA basligi, FF D8 (ofset 17 ya da herhangi) olanlari yaz
 *   dosya <lba> <adet> <yol>     : en fazla 640 sektoru (320 KB) RAM dosyasina yaz (kare inceleme, /tmp)
 *   komut <id> [arg]             : 10 baytlik 55AA komut basligini YALNIZ ELENE kontrol sektorune (LBA 4351) yazar.
 *                                  Izinli id YALNIZ 0x09 (durum) ve 0x1D (surum). 0x11 FORMAT, 0x14 SIL, 0x46, ISP YOK.
 * Izinli SCSI: v2 listesi + 2A WRITE(10) yalniz komut() icinden, yalniz LBA 4351, yalniz 1 sektor.
 * --- v2 ---
 * usb_bot_oku v2 (12.10.2026, Claude): v1 + "oku <lba> <adet> [ofset] [uzunluk]" istenen araligi hex basar
 * (MBR bolum tablosu 0x1BE, dizin girdileri). Izinli komut listesi AYNI.
 * --- v1 ---
 * usb_bot_oku v1 (12.10.2026, Claude): Generalplus 1b3f:8301 kamera modulu icin YALNIZ OKUMA yoklamasi.
 * usb-storage YUKLENMEDEN usbdevfs ile ham Bulk-Only Transport (BOT). Izinli SCSI komutlari yalniz:
 *   00 TEST UNIT READY, 03 REQUEST SENSE, 12 INQUIRY, 1A MODE SENSE(6), 25 READ CAPACITY(10), 28 READ(10) (en fazla 8 sektor).
 * Yazma, format, firmware, ureticiye ozel (C0..FF) komutlar KODDA YOK. Kamera SD kartina hicbir sey yazilmaz.
 *   usb-bot-oku yokla              : GET MAX LUN + TUR + INQUIRY + REQUEST SENSE + READ CAPACITY + MODE SENSE
 *   usb-bot-oku oku <lba> <adet>   : READ(10), ilk 64 bayt hex + imza taramasi (FAT/MBR, JPEG, AVI, PK) - veri diske yazilmaz
 * Aygit /sys/bus/usb/devices/1-1 (busnum/devnum), arayuz 0, EP 0x81 IN / 0x02 OUT. Zaman asimi 3 sn. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/usbdevice_fs.h>
#include <linux/usb/ch9.h>

static int fd;
static uint32_t etiket = 0x4b4f4c45;

static int bulk(unsigned ep, void *b, unsigned n)
{
	struct usbdevfs_bulktransfer t = { .ep = ep, .len = n, .timeout = 3000, .data = b };
	int r = ioctl(fd, USBDEVFS_BULK, &t);
	if (r < 0 && errno == EPIPE) {	/* stall: yalniz ilgili ucu temizle */
		unsigned e = ep;
		ioctl(fd, USBDEVFS_CLEAR_HALT, &e);
	}
	return r;
}

static int yazma_izni;	/* yalniz komut() acar */

static int izinli(uint8_t op)
{
	if (op == 0x2A) return yazma_izni;
	return op == 0x00 || op == 0x03 || op == 0x12 || op == 0x1A || op == 0x25 || op == 0x28;
}

/* BOT tek komut: cb (len), veri IN (n bayt). Donus: alinan bayt, CSW durumu *st */
static int scsi(const uint8_t *cb, int cblen, uint8_t *veri, unsigned n, int *st)
{
	if (!izinli(cb[0])) {
		fprintf(stderr, "REDDEDILDI: SCSI %02x izinli degil\n", cb[0]);
		return -1;
	}
	uint8_t cbw[31] = { 'U', 'S', 'B', 'C' };
	etiket++;
	memcpy(cbw + 4, &etiket, 4);
	memcpy(cbw + 8, &n, 4);
	int out = cb[0] == 0x2A;
	cbw[12] = (n && !out) ? 0x80 : 0;
	cbw[13] = 0;
	cbw[14] = (uint8_t)cblen;
	memcpy(cbw + 15, cb, (size_t)cblen);
	if (bulk(0x02, cbw, 31) != 31) { perror("CBW"); return -1; }
	int got = 0;
	if (n && out) {
		got = bulk(0x02, veri, n);
		if (got < 0) { perror("DATA OUT"); got = 0; }
	} else if (n) {
		got = bulk(0x81, veri, n);
		if (got < 0) { perror("DATA"); got = 0; }
	}
	uint8_t csw[13];
	int c = bulk(0x81, csw, 13);
	if (c < 0 && errno == EPIPE)
		c = bulk(0x81, csw, 13);
	if (c != 13 || memcmp(csw, "USBS", 4)) { fprintf(stderr, "CSW hatali (%d)\n", c); return -1; }
	*st = csw[12];
	return got;
}

static void hex(const uint8_t *b, int n)
{
	for (int i = 0; i < n; i++)
		printf("%02x%s", b[i], (i % 16 == 15 || i == n - 1) ? "\n" : " ");
}

static void ascii(const char *ad, const uint8_t *b, int n)
{
	printf("%s: \"", ad);
	for (int i = 0; i < n; i++)
		putchar(b[i] >= 32 && b[i] < 127 ? b[i] : '.');
	printf("\"\n");
}

static int ac(void)
{
	char yol[64];
	int bus = 0, dev = 0;
	FILE *f = fopen("/sys/bus/usb/devices/1-1/busnum", "r");
	if (!f || fscanf(f, "%d", &bus) != 1) return -1;
	fclose(f);
	f = fopen("/sys/bus/usb/devices/1-1/devnum", "r");
	if (!f || fscanf(f, "%d", &dev) != 1) return -1;
	fclose(f);
	snprintf(yol, sizeof yol, "/dev/bus/usb/%03d/%03d", bus, dev);
	fd = open(yol, O_RDWR | O_CLOEXEC);
	if (fd < 0) { perror(yol); return -1; }
	unsigned arayuz = 0;
	if (ioctl(fd, USBDEVFS_CLAIMINTERFACE, &arayuz)) { perror("CLAIMINTERFACE (surucu bagli mi?)"); return -1; }
	printf("acik: %s arayuz 0\n", yol);
	return 0;
}

static void kapat(void)
{
	unsigned arayuz = 0;
	ioctl(fd, USBDEVFS_RELEASEINTERFACE, &arayuz);
	close(fd);
}

static int yokla(void)
{
	uint8_t b[256];
	int st, n;
	/* GET MAX LUN: sinif istegi, IN, 1 bayt (yalniz okuma) */
	struct usbdevfs_ctrltransfer c = { .bRequestType = 0xA1, .bRequest = 0xFE, .wValue = 0, .wIndex = 0,
					   .wLength = 1, .timeout = 3000, .data = b };
	n = ioctl(fd, USBDEVFS_CONTROL, &c);
	printf("GET MAX LUN: %s %d\n", n == 1 ? "lun_sayisi-1 =" : "desteklenmiyor/hata", n == 1 ? b[0] : n);
	const uint8_t tur[6] = { 0x00 };
	n = scsi(tur, 6, NULL, 0, &st);
	printf("TEST UNIT READY: durum %d (0=hazir)\n", n < 0 ? -1 : st);
	const uint8_t inq[6] = { 0x12, 0, 0, 0, 36, 0 };
	memset(b, 0, sizeof b);
	n = scsi(inq, 6, b, 36, &st);
	printf("INQUIRY: %d bayt durum %d tur 0x%02x cikarilabilir %d\n", n, st, b[0] & 0x1f, b[1] >> 7);
	if (n >= 36) {
		ascii("  uretici", b + 8, 8);
		ascii("  urun", b + 16, 16);
		ascii("  surum", b + 32, 4);
	}
	const uint8_t rs[6] = { 0x03, 0, 0, 0, 18, 0 };
	memset(b, 0, sizeof b);
	n = scsi(rs, 6, b, 18, &st);
	printf("REQUEST SENSE: %d bayt key 0x%x asc 0x%02x ascq 0x%02x\n", n, b[2] & 0xf, b[12], b[13]);
	const uint8_t rc[10] = { 0x25 };
	memset(b, 0, sizeof b);
	n = scsi(rc, 10, b, 8, &st);
	if (n == 8) {
		uint32_t son = (uint32_t)b[0] << 24 | b[1] << 16 | b[2] << 8 | b[3];
		uint32_t blok = (uint32_t)b[4] << 24 | b[5] << 16 | b[6] << 8 | b[7];
		printf("READ CAPACITY: son LBA %u, blok %u bayt -> %.1f MB (durum %d)\n", son, blok,
		       ((double)son + 1) * blok / 1048576.0, st);
	} else
		printf("READ CAPACITY: %d bayt durum %d\n", n, st);
	const uint8_t ms[6] = { 0x1A, 0, 0x3F, 0, 192, 0 };
	memset(b, 0, sizeof b);
	n = scsi(ms, 6, b, 192, &st);
	printf("MODE SENSE(6): %d bayt durum %d yazma_korumali %d\n", n, st, n >= 3 ? b[2] >> 7 : -1);
	if (n > 0)
		hex(b, n > 32 ? 32 : n);
	return 0;
}

static int oku(uint32_t lba, unsigned adet, int ofs, int uz)
{
	if (adet < 1 || adet > 8) { fprintf(stderr, "adet 1..8\n"); return 2; }
	static uint8_t b[8 * 512];
	uint8_t cb[10] = { 0x28, 0, (uint8_t)(lba >> 24), (uint8_t)(lba >> 16), (uint8_t)(lba >> 8), (uint8_t)lba,
			   0, 0, (uint8_t)adet, 0 };
	int st, n = scsi(cb, 10, b, adet * 512, &st);
	printf("READ(10) lba %u adet %u: %d bayt durum %d\n", lba, adet, n, st);
	if (n <= 0) return 1;
	if (ofs < 0 || ofs >= n) ofs = 0;
	if (uz <= 0) uz = 64;
	if (ofs + uz > n) uz = n - ofs;
	printf("  ofset %d uzunluk %d:\n", ofs, uz);
	hex(b + ofs, uz);
	int ff = 1, sifir = 1;
	for (int i = 0; i < n; i++) {
		if (b[i] != 0xff) ff = 0;
		if (b[i]) sifir = 0;
	}
	if (ff) puts("  icerik: tamami 0xFF");
	if (sifir) puts("  icerik: tamami 0x00");
	if (n >= 512 && b[510] == 0x55 && b[511] == 0xaa) puts("  imza: 55AA (MBR/VBR)");
	for (int i = 0; i + 4 <= n; i++) {
		if (!memcmp(b + i, "\xff\xd8\xff", 3)) { printf("  imza: JPEG SOI @%d\n", i); break; }
	}
	for (int i = 0; i + 4 <= n; i++) {
		if (!memcmp(b + i, "RIFF", 4) || !memcmp(b + i, "AVI ", 4) || !memcmp(b + i, "PK\3\4", 4) ||
		    !memcmp(b + i, "FAT", 3) || !memcmp(b + i, "ftyp", 4)) {
			printf("  imza: \"%.4s\" @%d\n", b + i, i);
			break;
		}
	}
	return 0;
}

static int tara(uint32_t lba, unsigned adet)
{
	static uint8_t b[8 * 512];
	unsigned bulunan = 0;
	for (unsigned i = 0; i < adet; i += 8) {
		unsigned k = adet - i < 8 ? adet - i : 8;
		uint32_t l = lba + i;
		uint8_t cb[10] = { 0x28, 0, (uint8_t)(l >> 24), (uint8_t)(l >> 16), (uint8_t)(l >> 8), (uint8_t)l, 0, 0, (uint8_t)k, 0 };
		int st, n = scsi(cb, 10, b, k * 512, &st);
		if (n != (int)(k * 512)) { printf("okuma hatasi lba %u (%d)\n", l, n); return 1; }
		for (unsigned s = 0; s < k; s++) {
			uint8_t *p = b + s * 512;
			int ff = 1;
			for (int j = 0; j < 512; j++) if (p[j] != 0xff) { ff = 0; break; }
			int soi = -1;
			for (int j = 0; j < 511; j++) if (p[j] == 0xff && p[j + 1] == 0xd8 && (j + 2 >= 512 || p[j + 2] == 0xff)) { soi = j; break; }
			if ((p[0] == 0x55 && p[1] == 0xaa) || soi >= 0) {
				printf("lba %u (+0x%x): %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x | u32@0=%u soi@%d%s\n",
				       l + s, (l + s - lba) * 512, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11],
				       (unsigned)(p[0] | p[1] << 8 | p[2] << 16 | (unsigned)p[3] << 24), soi, ff ? " (FF)" : "");
				if (++bulunan > 60) { puts("... (60 sinir)"); return 0; }
			}
		}
	}
	printf("tarama bitti: %u isaretli sektor\n", bulunan);
	return 0;
}

static int dosya(uint32_t lba, unsigned adet, const char *yol)
{
	if (adet < 1 || adet > 640 || strncmp(yol, "/tmp/", 5)) { fputs("adet 1..640, yol /tmp/ altinda\n", stderr); return 2; }
	FILE *f = fopen(yol, "wb");
	if (!f) { perror(yol); return 1; }
	static uint8_t b[8 * 512];
	for (unsigned i = 0; i < adet; i += 8) {
		unsigned k = adet - i < 8 ? adet - i : 8;
		uint32_t l = lba + i;
		uint8_t cb[10] = { 0x28, 0, (uint8_t)(l >> 24), (uint8_t)(l >> 16), (uint8_t)(l >> 8), (uint8_t)l, 0, 0, (uint8_t)k, 0 };
		int st, n = scsi(cb, 10, b, k * 512, &st);
		if (n != (int)(k * 512)) { printf("okuma hatasi lba %u\n", l); fclose(f); return 1; }
		fwrite(b, 1, (size_t)n, f);
	}
	fclose(f);
	printf("%u sektor -> %s\n", adet, yol);
	return 0;
}

/* kare: ELENE 0x4B000 (600 sektor) yuvalarini tara, ilk gecerli kareyi (boyut 1024..256000, FF D8 @17) HEMEN tam oku */
static int kare(const char *yol, unsigned yuva_sayisi)
{
	static uint8_t b[640 * 512];
	if (strncmp(yol, "/tmp/", 5)) return 2;
	/* kamera kareyi ana bilgisayar ELENE'yi SIRAYLA okudukca yaziyor: yuva basliklarini ardisik 8'lik okumalarla gec */
	static uint8_t gecis[8 * 512];
	for (unsigned y = 0; y < yuva_sayisi; y++) {
		uint32_t l = 4351 + y * 600;
		if (y) {	/* onceki yuvanin geri kalanini sirayla oku (tetikleme) */
			for (uint32_t m = l - 600 + 1; m < l; m += 8) {
				unsigned k = l - m < 8 ? l - m : 8;
				uint8_t c3[10] = { 0x28, 0, (uint8_t)(m >> 24), (uint8_t)(m >> 16), (uint8_t)(m >> 8), (uint8_t)m, 0, 0, (uint8_t)k, 0 };
				int st3;
				scsi(c3, 10, gecis, k * 512, &st3);
			}
		}
		uint8_t cb[10] = { 0x28, 0, (uint8_t)(l >> 24), (uint8_t)(l >> 16), (uint8_t)(l >> 8), (uint8_t)l, 0, 0, 1, 0 };
		int st, n = scsi(cb, 10, b, 512, &st);
		if (n != 512) continue;
		uint32_t sz = b[0] | b[1] << 8 | b[2] << 16 | (uint32_t)b[3] << 24;
		if (sz < 1024 || sz > 256000 || b[17] != 0xff || b[18] != 0xd8) continue;
		unsigned sek = (sz + 17 + 511) / 512;
		for (unsigned i = 1; i < sek; i += 8) {	/* ilk sektor zaten okundu */
			unsigned k = sek - i < 8 ? sek - i : 8;
			uint32_t m = l + i;
			uint8_t c2[10] = { 0x28, 0, (uint8_t)(m >> 24), (uint8_t)(m >> 16), (uint8_t)(m >> 8), (uint8_t)m, 0, 0, (uint8_t)k, 0 };
			if (scsi(c2, 10, b + i * 512, k * 512, &st) != (int)(k * 512)) return 1;
		}
		int son = b[17 + sz - 2] == 0xff && b[17 + sz - 1] == 0xd9;
		FILE *f = fopen(yol, "wb");
		if (!f) return 1;
		fwrite(b + 17, 1, sz, f);
		fclose(f);
		printf("kare: yuva %u lba %u jpeg %u bayt, son FF D9: %s -> %s\n", y, l, sz, son ? "evet" : "HAYIR", yol);
		return son ? 0 : 3;
	}
	puts("kare bulunamadi");
	return 1;
}

static int komut(unsigned id, unsigned arg)
{
	if (id != 0x09 && id != 0x1D) { fprintf(stderr, "REDDEDILDI: komut 0x%02x izinli degil (yalniz 09, 1D)\n", id); return 2; }
	if (arg > 0xffff) return 2;
	static uint8_t b[512];
	memset(b, 0, sizeof b);
	unsigned ck = (id + arg - 0x55AB) & 0xffff;
	b[0] = 0x55; b[1] = 0xAA; b[2] = (uint8_t)id; b[3] = (uint8_t)(id >> 8); b[4] = (uint8_t)arg; b[5] = (uint8_t)(arg >> 8);
	b[6] = 0; b[7] = 0;	/* Melis ofset 8'i (sagliga) hic doldurmuyor: sifir birakilir (ck yalniz bilgi icin) */
	const uint32_t l = 4351;	/* ELENE kontrol sektoru (kume 3); BASKA LBA'YA YAZILMAZ */
	uint8_t cb[10] = { 0x2A, 0, (uint8_t)(l >> 24), (uint8_t)(l >> 16), (uint8_t)(l >> 8), (uint8_t)l, 0, 0, 1, 0 };
	yazma_izni = 1;
	int st, n = scsi(cb, 10, b, 512, &st);
	yazma_izni = 0;
	printf("WRITE(10) lba %u komut 0x%02x arg %u cksum 0x%04x: %d bayt durum %d\n", l, id, arg, ck, n, st);
	return n == 512 && st == 0 ? 0 : 1;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fputs("kullanim: usb-bot-oku yokla | oku <lba> <adet 1..8> [ofset] [uzunluk] | tara <lba> <adet> | dosya <lba> <adet> <yol> | komut <09|1D> [arg]\n", stderr);
		return 2;
	}
	if (ac()) return 1;
	int rc = 2;
	if (!strcmp(argv[1], "yokla"))
		rc = yokla();
	else if (!strcmp(argv[1], "tara") && argc == 4)
		rc = tara((uint32_t)strtoul(argv[2], NULL, 0), (unsigned)atoi(argv[3]));
	else if (!strcmp(argv[1], "kare") && argc >= 3)
		rc = kare(argv[2], argc > 3 ? (unsigned)atoi(argv[3]) : 8);
	else if (!strcmp(argv[1], "dosya") && argc == 5)
		rc = dosya((uint32_t)strtoul(argv[2], NULL, 0), (unsigned)atoi(argv[3]), argv[4]);
	else if (!strcmp(argv[1], "komut") && argc >= 3)
		rc = komut((unsigned)strtoul(argv[2], NULL, 0), argc > 3 ? (unsigned)strtoul(argv[3], NULL, 0) : 0);
	else if (!strcmp(argv[1], "oku") && argc >= 4)
		rc = oku((uint32_t)strtoul(argv[2], NULL, 0), (unsigned)atoi(argv[3]),
			 argc > 4 ? (int)strtol(argv[4], NULL, 0) : 0, argc > 5 ? (int)strtol(argv[5], NULL, 0) : 64);
	kapat();
	return rc;
}
