/* saat_kayma_v1.h (24.10.2026, Claude; panel 0.79, scripts/622, docs/denetim-2026-10-24-v3.md A11): PC mesajindaki
 * "time" sistem saatine uygulansin mi. 0.77 kurali (1 gunden buyuk atlama yok) bir onceki mesajin kaydirdigi saatten
 * olculuyordu; LAN'daki bir cihaz 250 ms'de bir +86000 sn ile saati yillarca ileri alabiliyordu. Simdi saat gecerliyken
 * (>= 1.7e9) PC'nin TOPLAM kaydirmasi 1 gunun altinda kalir; saat gecersizken PC saati eskisi gibi alinir ve birikim
 * sifirlanir. Saf; tests/saat_kayma_test_v1.c sinar. */
#ifndef SAAT_KAYMA_V1_H
#define SAAT_KAYMA_V1_H

/* simdi: sistem saati (sn), pc: PC'nin saati (sn), birikim: bu surecte PC ile yapilan toplam kayma (sn).
 * Donus 1: settimeofday(pc) yapilmali (*birikim guncellendi); 0: dokunma. */
static inline int saat_kayma_uygula(double simdi, double pc, double *birikim)
{
	double fark = pc - simdi, top;
	if (!(pc > 1.7e9 && pc < 4.1e9))	/* gecersiz ya da time_t'ye sigmayacak kadar buyuk (2099 sonrasi) */
		return 0;
	if (fark <= 2 && fark >= -2)
		return 0;
	if (simdi < 1.7e9) {	/* saat hic ayarlanmamis: PC saati alinir, yeni taban */
		*birikim = 0;
		return 1;
	}
	top = *birikim + fark;
	if (top >= 86400 || top <= -86400)
		return 0;
	*birikim = top;
	return 1;
}
#endif
