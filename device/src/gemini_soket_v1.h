/* gemini_soket_v1.h (23.10.2026, Claude; kole-asistan v18, madde 131): Hey Kole sorusunu surekli calisan kole-gemini
 * --telegram surecine yerel Unix soketinden sorar (kole-gemini v40 soru_sunucu.go). Baglanilamazsa -1 (cagiran eskisi gibi
 * surec acar). Protokol: tek satir {"ses":"<wav>"}\n -> tek satir sonuc JSON ya da {"rc":N}, sonra kapanis. */
#ifndef GEMINI_SOKET_V1_H
#define GEMINI_SOKET_V1_H
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/* baglan + istegi yaz; basarida okumaya hazir, bloklamayan fd; olmazsa -1 */
static int gs_ac(const char *soket, const char *wav)
{
	struct sockaddr_un a;
	memset(&a, 0, sizeof a);
	a.sun_family = AF_UNIX;
	if (strlen(soket) >= sizeof a.sun_path || strchr(wav, '"') || strchr(wav, '\\'))
		return -1;
	memcpy(a.sun_path, soket, strlen(soket) + 1);
	int s = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
	if (s < 0)
		return -1;
	if (connect(s, (struct sockaddr *)&a, sizeof a)) {
		close(s);
		return -1;
	}
	char m[512];
	int n = snprintf(m, sizeof m, "{\"ses\":\"%s\"}\n", wav);
	if (n <= 0 || n >= (int)sizeof m || write(s, m, (size_t)n) != n) {
		close(s);
		return -1;
	}
	fcntl(s, F_SETFL, O_NONBLOCK);
	return s;
}

/* 1: bitti (EOF ya da tampon dolu), 0: suruyor, -1: hata. *n biriken bayt; tampon her donuste NUL ile biter */
static int gs_oku(int fd, char *b, size_t boy, size_t *n)
{
	for (;;) {
		if (*n >= boy - 1) {
			b[*n] = 0;
			return 1;
		}
		ssize_t r = read(fd, b + *n, boy - 1 - *n);
		if (r > 0) {
			*n += (size_t)r;
			continue;
		}
		b[*n] = 0;
		if (r == 0)
			return 1;
		return (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) ? 0 : -1;
	}
}

/* sonuc kodu: "cevap" alani varsa 0, {"rc":N} ise N, bossa 5 (sunucu cevapsiz kapandi) */
static int gs_rc(const char *b)
{
	if (strstr(b, "\"cevap\":"))
		return 0;
	const char *p = strstr(b, "\"rc\":");
	if (p) {
		int v = atoi(p + 5);
		return v ? v : 5;
	}
	return 5;
}
#endif
