/* VeLinux kablosuz ekran: ortak bellek duzeni v1 (11.10.2026, Claude).
 * Alici (initramfs/src/wifi_ekran_v4.c) /tmp/pc-ekran.shm dosyasini olusturur, JPEG'leri DOGRUDAN buradaki
 * 1600x600 RGB565 (LVGL LV_COLOR_FORMAT_RGB565, little-endian) piksellere cozer; panel dosyayi YALNIZ OKUR
 * (PROT_READ) ve LVGL goruntu kaynagi olarak kopyasiz kullanir. Her degisen dikdortgen bir halkaya yazilir;
 * panel yalniz o alanlari yeniden cizer. Kopya/dosya yazma/checksum yok (v3 tasariminda tam kare ~4 fps idi).
 * Yirtilma (alici yazarken panelin okumasi) kabul edilir; bir sonraki dikdortgen bildirimi duzeltir.
 * Ag, framebuffer, UART yok. Iki surec de ayni CLOCK_MONOTONIC ms degerini kullanir. */
#ifndef PC_SHM_V1_H
#define PC_SHM_V1_H
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define PC_SHM_PATH "/tmp/pc-ekran.shm"
#define PC_SHM_MAGIC 0x56534831u /* "VSH1" */
#define PC_SHM_W 1600u
#define PC_SHM_H 600u
#define PC_SHM_HDR 4096u
#define PC_SHM_SIZE (PC_SHM_HDR + PC_SHM_W * PC_SHM_H * 2u)
#define PC_SHM_RING 64u
#define PC_SHM_ALIVE_MS 2500u	/* bu kadar yeni kare/canlilik yoksa akis "durdu" */

enum { PC_SHM_LISTEN = 1, PC_SHM_CLIENT = 2, PC_SHM_CLOSED = 3 };

typedef struct {
	uint32_t magic, version, w, h, stride, offset, gen, conn;
	uint32_t alive_ms, state, want_full, frames;
	uint32_t rect_count, errors, last_decode_us, last_bytes;
	uint32_t fps_x10, kbps, pad[14];
	uint16_t rects[PC_SHM_RING][4];	/* x, y, w, h */
} pc_shm_hdr_t;

_Static_assert(sizeof(pc_shm_hdr_t) <= PC_SHM_HDR, "shm header");

static inline uint32_t pc_shm_now_ms(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (uint32_t)(t.tv_sec * 1000 + t.tv_nsec / 1000000);
}

/* ------------------------------------------------------------ okuyucu (panel) */
typedef struct {
	const volatile pc_shm_hdr_t *h;
	const uint8_t *px;
	uint32_t last_count, gen;
	dev_t dev;
	ino_t ino;
} pc_shm_t;

static inline int pc_shm_valid_hdr(const volatile pc_shm_hdr_t *h)
{
	return h->magic == PC_SHM_MAGIC && h->version == 1 && h->w == PC_SHM_W && h->h == PC_SHM_H &&
	       h->stride == PC_SHM_W * 2 && h->offset == PC_SHM_HDR;
}

/* 1: eslendi; -1: yok/gecersiz. Dosya normal dosya ve tam boyutta olmali, sembolik baglanti reddedilir. */
static inline int pc_shm_open(pc_shm_t *s, const char *path)
{
	struct stat st;
	int fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) || !S_ISREG(st.st_mode) || st.st_size != (off_t)PC_SHM_SIZE) {
		close(fd);
		return -1;
	}
	void *m = mmap(NULL, PC_SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	if (m == MAP_FAILED)
		return -1;
	const volatile pc_shm_hdr_t *h = m;
	if (!pc_shm_valid_hdr(h)) {
		munmap(m, PC_SHM_SIZE);
		return -1;
	}
	s->h = h;
	s->px = (const uint8_t *)m + PC_SHM_HDR;
	s->gen = h->gen;
	s->last_count = __atomic_load_n(&h->rect_count, __ATOMIC_ACQUIRE);
	s->dev = st.st_dev;
	s->ino = st.st_ino;
	return 1;
}

static inline void pc_shm_close(pc_shm_t *s)
{
	if (s->h)
		munmap((void *)s->h, PC_SHM_SIZE);
	memset(s, 0, sizeof *s);
}

/* Eslenen dosya hala yerinde mi (alici kapanip yenisi acildiysa inode degisir)? 1 evet, 0 hayir. */
static inline int pc_shm_still_current(const pc_shm_t *s, const char *path)
{
	struct stat st;
	if (!s->h || s->h->state == PC_SHM_CLOSED || s->h->gen != s->gen)
		return 0;
	if (lstat(path, &st) || st.st_dev != s->dev || st.st_ino != s->ino)
		return 0;
	return 1;
}

static inline int pc_shm_alive(const pc_shm_t *s, uint32_t now)
{
	return s->h && s->h->state == PC_SHM_CLIENT && (uint32_t)(now - s->h->alive_ms) <= PC_SHM_ALIVE_MS;
}

/* Yeni kirli dikdortgenler: >0 adet (out'a, ekrana kirpilmis x,y,w,h), 0 degisiklik yok,
 * -1 tam ekran yeniden ciz (halka tasti / okuma sirasinda ustune yazildi / cap kucuk). */
static inline int pc_shm_take(pc_shm_t *s, uint16_t out[][4], int cap)
{
	const volatile pc_shm_hdr_t *h = s->h;
	uint32_t c = __atomic_load_n(&h->rect_count, __ATOMIC_ACQUIRE);
	uint32_t d = c - s->last_count;
	if (!d)
		return 0;
	if (d > (uint32_t)cap || d > PC_SHM_RING / 2) {
		s->last_count = c;
		return -1;
	}
	int n = 0;
	for (uint32_t i = 0; i < d; i++) {
		const volatile uint16_t *r = h->rects[(s->last_count + i) % PC_SHM_RING];
		uint32_t x = r[0], y = r[1], w = r[2], hh = r[3];
		if (!w || !hh || x >= PC_SHM_W || y >= PC_SHM_H)
			continue;
		if (x + w > PC_SHM_W)
			w = PC_SHM_W - x;
		if (y + hh > PC_SHM_H)
			hh = PC_SHM_H - y;
		out[n][0] = (uint16_t)x;
		out[n][1] = (uint16_t)y;
		out[n][2] = (uint16_t)w;
		out[n][3] = (uint16_t)hh;
		n++;
	}
	uint32_t c2 = __atomic_load_n(&h->rect_count, __ATOMIC_ACQUIRE);
	if (c2 - s->last_count > PC_SHM_RING) {
		s->last_count = c2;
		return -1;
	}
	s->last_count = c;
	return n ? n : 0;
}

#endif
