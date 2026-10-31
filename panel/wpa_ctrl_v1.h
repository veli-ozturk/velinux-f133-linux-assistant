/* wpa_ctrl_v1.h (23.10.2026, Claude; panel 0.76, scripts/569): wpa_supplicant kontrol soketinden sinyal ve SSID (iw popen
 * yerine; 10 sn'de bir sh + iw sureci aciliyordu). SIGNAL_POLL -> "RSSI=-45", STATUS -> "ssid=..". Ayristirma saf;
 * tests/wpa_ctrl_test_v1.c sahte wpa_supplicant ile sinar. */
#ifndef WPA_CTRL_V1_H
#define WPA_CTRL_V1_H
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/* "anahtar=deger" satiri (satir basinda) -> o; 0 bulundu, -1 yok */
static int wpa_alan(const char *cevap, const char *anahtar, char *o, size_t n)
{
	size_t k = strlen(anahtar);
	for (const char *p = cevap; p && *p;) {
		if (!strncmp(p, anahtar, k) && p[k] == '=') {
			const char *v = p + k + 1;
			size_t m = strcspn(v, "\r\n");
			if (m >= n)
				m = n - 1;
			memcpy(o, v, m);
			o[m] = 0;
			return 0;
		}
		p = strchr(p, '\n');
		if (p)
			p++;
	}
	return -1;
}

/* tek istek/cevap; soket bir kez kurulur (yerel adres sabit, once silinir). 0 cevap var, -1 yok */
static int wpa_sor(const char *kontrol, const char *yerel, const char *komut, char *cevap, size_t n, int ms)
{
	static int s = -1;
	static struct sockaddr_un uzak;
	if (s < 0) {
		struct sockaddr_un a;
		memset(&a, 0, sizeof a);
		memset(&uzak, 0, sizeof uzak);
		a.sun_family = uzak.sun_family = AF_UNIX;
		if (strlen(yerel) >= sizeof a.sun_path || strlen(kontrol) >= sizeof uzak.sun_path)
			return -1;
		memcpy(a.sun_path, yerel, strlen(yerel) + 1);
		memcpy(uzak.sun_path, kontrol, strlen(kontrol) + 1);
		s = socket(AF_UNIX, SOCK_DGRAM | SOCK_CLOEXEC, 0);
		if (s < 0)
			return -1;
		unlink(yerel);
		if (bind(s, (struct sockaddr *)&a, sizeof a)) {
			close(s);
			s = -1;
			return -1;
		}
	}
	char cop[256];
	while (recv(s, cop, sizeof cop, MSG_DONTWAIT) > 0)	/* gec gelen eski cevaplar */
		;
	if (sendto(s, komut, strlen(komut), 0, (struct sockaddr *)&uzak, sizeof uzak) < 0)
		return -1;
	struct pollfd p = {s, POLLIN, 0};
	if (poll(&p, 1, ms) <= 0)
		return -1;
	ssize_t r = recv(s, cevap, n - 1, 0);
	if (r <= 0)
		return -1;
	cevap[r] = 0;
	return 0;
}
#endif
