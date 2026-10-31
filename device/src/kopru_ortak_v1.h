/* kopru_ortak_v1.h (13.10.2026, Claude): kole-kopru v8 ve ileride cihaz-ici web merkezinin ORTAK yardimcilari.
 * Yollar degisken: self-test ve rtsp-test gecici klasore yonlendirir; sunucu kipinde degismez.
 * Bu basligi icerenler: ses_akis_v1.h, kamera_akis_v1.h, kayit_sd_v1.h, rtsp_sunucu_v1.h, kole_kopru_v8.c */
#ifndef KOPRU_ORTAK_V1_H
#define KOPRU_ORTAK_V1_H
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static const char *kok = "/mnt/sd/kamera";			/* SD kayitlari */
static const char *sd_kok = "/mnt/sd";
static const char *kalkan_yol = "/tmp/gizlilik-kalkani";	/* panel 0.22 gizlilik kalkani */
static const char *kamera_bayrak = "/tmp/kamera-aktif";		/* panel KAMERA gostergesi + kamera-kayit canli */
static const char *mik_bayrak = "/tmp/mik-aktif";		/* panel MIKROFON gostergesi */
static const char *hoparlor_bayrak = "/tmp/hoparlor-aktif";	/* uzaktan konusma suruyor (panel 0.24 icin) */
static const char *snd_kok = "/dev/snd";			/* self-test: var olmayan klasor (ses aygiti acilmaz) */
static const char *kamera_son = "/tmp/kamera-son.jpg";		/* kamera-kayit canli karesi */
static const char *kayit_pid = "/tmp/kamera-kayit.pid";

static double mono(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec + t.tv_nsec / 1e9;
}

static int tam_yaz(int s, const void *b, size_t n)
{
	const char *p = b;
	while (n) {
		ssize_t w = send(s, p, n, MSG_NOSIGNAL);
		if (w < 0 && errno == ENOTSOCK) w = write(s, p, n);	/* boru/dosya (self-test) */
		if (w <= 0) {
			if (w < 0 && errno == EINTR)
				continue;
			return -1;
		}
		p += w;
		n -= (size_t)w;
	}
	return 0;
}

static long dosya_oku(const char *yol, char *b, size_t n)
{
	int fd = open(yol, O_RDONLY | O_CLOEXEC);
	if (fd < 0) { b[0] = 0; return -1; }
	ssize_t r = read(fd, b, n - 1);
	close(fd);
	if (r < 0) r = 0;
	b[r] = 0;
	while (r > 0 && (b[r - 1] == '\n' || b[r - 1] == '\r')) b[--r] = 0;
	return r;
}

static int dosya_var(const char *p)
{
	struct stat st;
	return !stat(p, &st);
}

static int hata(int s, const char *m)
{
	char b2[96];
	int n = snprintf(b2, sizeof b2, "HATA %s\n", m);
	tam_yaz(s, b2, (size_t)n);
	return 2;
}

/* bayrak dosyasi; icerik verilirse yazilir (sahiplik: "rtsp <pid>" gibi) */
static void bayrak_yaz_icerik(const char *yol, int ac, const char *icerik)
{
	if (ac) {
		int kf = open(yol, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (kf >= 0) {
			if (icerik) tam_yaz(kf, icerik, strlen(icerik));
			close(kf);
		}
	} else {
		unlink(yol);
	}
}

static void bayrak_yaz(const char *yol, int ac)
{
	bayrak_yaz_icerik(yol, ac, NULL);
}

/* recv ile tam n bayt (zaman asimi SO_RCVTIMEO ile). 0 ok, -1 kapandi/zaman asimi */
static int tam_oku_fd(int s, void *b, size_t n)
{
	char *p = b;
	while (n) {
		ssize_t r = recv(s, p, n, 0);
		if (r < 0 && errno == ENOTSOCK) r = read(s, p, n);
		if (r < 0 && errno == EINTR) continue;
		if (r <= 0) return -1;
		p += r;
		n -= (size_t)r;
	}
	return 0;
}
#endif
