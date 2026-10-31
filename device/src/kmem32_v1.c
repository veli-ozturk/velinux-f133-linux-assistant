/* kmem32 v1 (11.10.2026, Claude): /dev/mem uzerinden ONBELLEKLI (O_SYNC yok) 32 bit okuma / karsilastir-yaz.
 * busybox devmem O_SYNC ile onbelleksiz esler; cekirdek verisi onbellekten okundugu icin yazi gorunmeyebilir.
 *   kmem32 oku <fiziksel_adres> <adet>
 *   kmem32 yaz <fiziksel_adres> <beklenen> <yeni>   (beklenen degilse yazmaz, cikis 3)
 * Kullanim: cekirdek G2D Ycbcr2rgb_601 "full to full" ofset duzeltmesi (yalniz RAM'de; yeniden acilista gider). */
#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc < 4)
		return 2;
	uint64_t adr = strtoull(argv[2], NULL, 0);
	if (adr < 0x40000000ull || adr >= 0x44000000ull || (adr & 3))
		return 2;	/* yalniz DRAM, hizali */
	int fd = open("/dev/mem", O_RDWR | O_CLOEXEC);
	if (fd < 0) { perror("/dev/mem"); return 1; }
	long sayfa = sysconf(_SC_PAGESIZE);
	uint64_t taban = adr & ~(uint64_t)(sayfa - 1);
	size_t uz = (size_t)(adr - taban) + 4096;
	uint8_t *m = mmap(NULL, uz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, (off_t)taban);
	if (m == MAP_FAILED) { perror("mmap"); return 1; }
	volatile uint32_t *p = (volatile uint32_t *)(m + (adr - taban));
	int r = 0;
	if (!strcmp(argv[1], "oku")) {
		unsigned n = (unsigned)atoi(argv[3]);
		if (n < 1 || n > 256) return 2;
		for (unsigned i = 0; i < n; i++)
			printf("0x%08llx 0x%08x\n", (unsigned long long)(adr + 4 * i), p[i]);
	} else if (!strcmp(argv[1], "yaz") && argc == 5) {
		uint32_t bek = (uint32_t)strtoul(argv[3], NULL, 0), yeni = (uint32_t)strtoul(argv[4], NULL, 0);
		uint32_t eski = p[0];
		if (eski != bek) {
			printf("YAZILMADI 0x%08llx: beklenen 0x%08x, bulunan 0x%08x\n", (unsigned long long)adr, bek, eski);
			r = 3;
		} else {
			p[0] = yeni;
			__sync_synchronize();
			printf("YAZILDI 0x%08llx: 0x%08x -> 0x%08x (okunan 0x%08x)\n", (unsigned long long)adr, eski, yeni, p[0]);
		}
	} else
		r = 2;
	munmap(m, uz);
	close(fd);
	return r;
}
