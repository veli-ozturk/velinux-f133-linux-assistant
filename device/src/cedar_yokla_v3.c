/* cedar-yokla v3 (11.10.2026, Claude): v2 + DOGRU SIRA. v1/v2 ENABLE_VE (yalniz VE cekirdek saati) cagirip reset'teki,
 * veri yolu saati kapali birimin yazmaclarini okumaya calisiyordu. Surucude reset kaldirma + bus/mbus/ve saatleri
 * ENGINE_REQ (0x206, ilk istekte) ile yapilir; surucu hatasi nedeniyle YALNIZ BIR KEZ cagrilir. Sonda ENGINE_REL.
 * Adim 4 = ENGINE_REQ (v2'de ENABLE_VE), adim 13 = ENGINE_REL.
 * --- v2 ---
 * cedar-yokla v2 (11.10.2026, Claude): v1 + ADIM ADIM ve GUNLUK. v1 seri kabukta kesilemez bicimde takildi; hangi
 * adimda oldugu gorulemedi. v2: her adim once /tmp/cedar-yokla.log'a "ADIM n BASLIYOR" yazip fsync eder, sonra calisir.
 * Arka planda calistirilmali:  /tmp/cedar-yokla-v2 [son_adim] > /dev/null 2>&1 &   ; cat /tmp/cedar-yokla.log
 * Adimlar: 1 open  2 GET_ENV_INFO  3 GET_IC_VER  4 ENABLE_VE  5 mmap yazmaclar  6 yazmac OKU  7 ion heap query
 *          8 ion alloc 1 MB (DMA/CMA)  9 mmap+desen  10 MAP_DMA_BUF  11 FLUSH_CACHE_RANGE  12 UNMAP + kapat
 * VE'ye IS VERMEZ, kesme ACMAZ. Varsayilan son_adim 12. */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

#define VE_GET_ENV_INFO 0x101
#define VE_ENGINE_REQ 0x206
#define VE_ENGINE_REL 0x207
#define VE_GET_IC_VER 0x209
#define VE_MAP_DMA_BUF 0x504
#define VE_UNMAP_DMA_BUF 0x505
#define VE_FLUSH_CACHE_RANGE 0x506
struct dma_buf_param { int fd; unsigned int phy_addr; };
struct cache_range { uint64_t start, end; };
struct env_info { unsigned int phymem_start; int phymem_total_size; uint64_t address_macc; };
struct ion_allocation_data { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
struct ion_heap_data { char name[32]; uint32_t type, heap_id, r0, r1, r2; };
struct ion_heap_query { uint32_t cnt, r0; uint64_t heaps; uint32_t r1, r2; };
#define ION_IOC_ALLOC _IOWR('I', 0, struct ion_allocation_data)
#define ION_IOC_HEAP_QUERY _IOWR('I', 8, struct ion_heap_query)

static int logfd = -1;
static void yaz(const char *f, ...)
{
	char b[512];
	va_list a;
	va_start(a, f);
	int n = vsnprintf(b, sizeof b, f, a);
	va_end(a);
	if (n > 0 && logfd >= 0) {
		if (write(logfd, b, (size_t)(n < (int)sizeof b ? n : (int)sizeof b - 1)) < 0) { /* yok say */ }
		fsync(logfd);
	}
}

int main(int argc, char **argv)
{
	int son = argc > 1 ? atoi(argv[1]) : 13;
	logfd = open("/tmp/cedar-yokla.log", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	int ve = -1, ion = -1, adim = 0;
	volatile uint32_t *r = NULL;
	uint8_t *m = NULL;
	struct ion_allocation_data al = {0};
	struct dma_buf_param p = {0};
#define ADIM(ad) if (++adim > son) goto bit; yaz("ADIM %d BASLIYOR: %s\n", adim, ad)
	ADIM("open /dev/cedar_dev");
	ve = open("/dev/cedar_dev", O_RDWR | O_CLOEXEC);
	yaz("  fd=%d errno=%d\n", ve, ve < 0 ? errno : 0);
	if (ve < 0) goto bit;
	ADIM("GET_ENV_INFO");
	struct env_info env = {0};
	yaz("  rc=%d start=%#x size=%d macc=%#llx\n", ioctl(ve, VE_GET_ENV_INFO, &env), env.phymem_start,
	    env.phymem_total_size, (unsigned long long)env.address_macc);
	ADIM("GET_IC_VER");
	yaz("  rc=%d\n", ioctl(ve, VE_GET_IC_VER, 0));
	ADIM("ENGINE_REQ (reset kaldir + saatler)");
	yaz("  rc=%d\n", ioctl(ve, VE_ENGINE_REQ, 0));
	ADIM("mmap VE yazmaclari");
	r = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, ve, 0);
	yaz("  %p errno=%d\n", (void *)r, r == MAP_FAILED ? errno : 0);
	if (r == MAP_FAILED) goto bit;
	ADIM("yazmac oku");
	static const unsigned off[] = { 0x00, 0x04, 0x08, 0x0c, 0x10, 0x20, 0x24, 0xe0, 0xe4, 0xe8, 0xec, 0xf0, 0xf4 };
	for (unsigned i = 0; i < sizeof off / sizeof *off; i++) {
		yaz("  VE[0x%03x] okunuyor...", off[i]);
		yaz(" 0x%08x\n", r[off[i] / 4]);
	}
	ADIM("ion heap query");
	ion = open("/dev/ion", O_RDWR | O_CLOEXEC);
	struct ion_heap_data hd[16];
	memset(hd, 0, sizeof hd);
	struct ion_heap_query q = { .cnt = 16, .heaps = (uintptr_t)hd };
	yaz("  ion fd=%d query rc=%d cnt=%u\n", ion, ioctl(ion, ION_IOC_HEAP_QUERY, &q), q.cnt);
	uint32_t mask = 0;
	for (unsigned i = 0; i < q.cnt && i < 16; i++) {
		yaz("  yigin '%s' tip %u id %u\n", hd[i].name, hd[i].type, hd[i].heap_id);
		if (hd[i].type == 2 && !mask)
			mask = 1u << hd[i].heap_id;
	}
	ADIM("ion alloc 1 MB");
	al = (struct ion_allocation_data){ .len = 1 << 20, .heap_id_mask = mask, .flags = 1 };
	yaz("  rc=%d fd=%u errno=%d\n", ioctl(ion, ION_IOC_ALLOC, &al), al.fd, errno);
	ADIM("mmap ion + desen");
	m = mmap(NULL, 1 << 20, PROT_READ | PROT_WRITE, MAP_SHARED, (int)al.fd, 0);
	yaz("  %p\n", (void *)m);
	if (m == MAP_FAILED) goto bit;
	for (int i = 0; i < 1 << 20; i++)
		m[i] = (uint8_t)(i * 7);
	ADIM("MAP_DMA_BUF");
	p.fd = (int)al.fd;
	yaz("  rc=%d VE adresi=0x%08x\n", ioctl(ve, VE_MAP_DMA_BUF, &p), p.phy_addr);
	ADIM("FLUSH_CACHE_RANGE 4 KB");
	struct cache_range cr = { (uintptr_t)m, (uintptr_t)m + 4096 };
	yaz("  rc=%d\n", ioctl(ve, VE_FLUSH_CACHE_RANGE, &cr));
	ADIM("UNMAP");
	yaz("  unmap rc=%d\n", ioctl(ve, VE_UNMAP_DMA_BUF, &p));
	ADIM("ENGINE_REL");
	yaz("  rc=%d\n", ioctl(ve, VE_ENGINE_REL, 0));
bit:
	yaz("BITTI (son adim %d)\n", adim > son ? son : adim);
	return 0;
}
