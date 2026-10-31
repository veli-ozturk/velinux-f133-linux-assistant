/* izinli_ip_v1.h (23.10.2026, Claude; docs/denetim-2026-10-23-v2.md bulgu 6): yerel ag servislerine (panel 8765,
 * kole-kopru 8767, wifi-ekran 8766) baglanti izni. Her zaman izinli: 127.0.0.0/8 (cihazin kendisi) ve 100.64.0.0/10
 * (Tailscale). /data/izinli-ip VARSA bunlara ek olarak yalniz dosyadaki IPv4 adresleri (satir basina bir, # yorum) kabul
 * edilir; dosya YOKSA herkes (eski davranis). Dosya kullanıcının PC adresiyle bilincli olusturulur: PC'nin IP'si degisirse
 * PC baglanamaz. Dosya her baglantida okunur (degisiklik yeniden baslatmasiz gecerli). */
#ifndef IZINLI_IP_V1_H
#define IZINLI_IP_V1_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

/* ip: ana bilgisayar bayt sirasi. 1 izinli, 0 degil. dosya NULL ise /data/izinli-ip */
static inline int izinli_ip(uint32_t ip, const char *dosya)
{
	if ((ip >> 24) == 127 || (ip & 0xFFC00000u) == 0x64400000u)
		return 1;
	FILE *f = fopen(dosya ? dosya : "/data/izinli-ip", "r");
	if (!f)
		return 1;
	char s[80];
	int ok = 0;
	while (!ok && fgets(s, sizeof s, f)) {
		char *p = s + strspn(s, " \t\xEF\xBB\xBF");	/* 614: Windows'ta yazilan dosyanin BOM'u */
		p[strcspn(p, " \t\r\n#")] = 0;
		struct in_addr a;
		if (*p && inet_aton(p, &a) && ntohl(a.s_addr) == ip)
			ok = 1;
	}
	fclose(f);
	return ok;
}

/* kabul edilen soket: IPv4 degilse (bu servisler AF_INET dinler) izinli sayilir */
static inline int izinli_soket(int fd, const char *dosya)
{
	struct sockaddr_in a;
	socklen_t n = sizeof a;
	if (getpeername(fd, (struct sockaddr *)&a, &n) || a.sin_family != AF_INET)
		return 1;
	return izinli_ip(ntohl(a.sin_addr.s_addr), dosya);
}

/* self-test: 0 tamam. gecici dosya yolu verilir */
static inline int izinli_ip_test(const char *gecici)
{
	remove(gecici);
	if (!izinli_ip(0xC0A80167u, gecici))	/* dosya yok: herkes */
		return 1;
	FILE *f = fopen(gecici, "w");
	if (!f)
		return 2;
	fputs("\xEF\xBB\xBF" "192.168.1.10  # masaustu, BOM'lu ilk satir\n# PC\n\n10.0.0.5\n", f);
	fclose(f);
	int r = !izinli_ip(0xC0A80167u, gecici) ? 3 : izinli_ip(0xC0A80168u, gecici) ? 4 : !izinli_ip(0x0A000005u, gecici) ? 5
		: !izinli_ip(0x7F000001u, gecici) ? 6 : !izinli_ip(0x64500A42u, gecici) ? 7 : izinli_ip(0x64000001u, gecici) ? 8 : 0;
	remove(gecici);
	return r;
}
#endif
