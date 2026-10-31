/* azalt_v2.h (21.10.2026, Claude; scripts/483, OTONOM Sira 9 madde 70): v1 + cikislar dorder dorder. Host gprof
 * (uyandirma_kiyas, v15 zinciri): sessizlikte zincirin %77'si, konusmada %52'si azalt(); her cikis 31 ardisik
 * carpim-toplam (s += h[t] * fir[t]) = FP toplama GECIKMESINE bagli zincir. v2 her cikisin zincirini v1 ile AYNI sirada
 * kurar (s = 0; t = 0..30; s *= g; lrintf), ama 4 bagimsiz cikisin (3 ornek arayla) zincirleri ic ice -> gecikme gizlenir.
 * Simetri (fir[t] == fir[30-t]) KULLANILMAZ: toplama sirasi degisir, bit bit aynilik bozulur. Gecmis halka yerine
 * dogrusal tampon (son TAP-1 ornek + cagrinin ornekleri; cagri sonunda kaydirma). Cikti v1 ile bit bit ayni
 * (tests/azalt_kiyas_v1.c: rastgele parca boylari; uyandirma_kiyas_v3: tum zincir). Kullanim v1 ile ayni:
 * fir_hazirla(); struct azalt a = {0}; k = azalt(&a, in, n, out, kazanc). --- v1 ---
 * azalt_v1.h (21.10.2026, Claude; scripts/430, OTONOM Sira 7 madde 39): 48 kHz -> 16 kHz FIR azaltici, HALKA tampon
 * (cift yazma). kole_asistan v12'nin ornek basina memmove'lu azalt()'i ile AYNI carpimlar AYNI sirada -> cikti bit bit
 * ayni (tests/uyandirma_kiyas_v1.c). */
#ifndef AZALT_V2_H
#define AZALT_V2_H
#include <math.h>
#include <stdint.h>
#include <string.h>
#ifndef TAP
#define TAP 31
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define AZ_PARCA 1024	/* bir turda islenen en cok ornek (daha uzun cagri parcalanir; mikrofon PER 960) */

static float fir[TAP];

static void fir_hazirla(void)
{
	double top = 0, fc = 7000.0 / 48000.0;
	for (int i = 0; i < TAP; i++) {
		int m = i - TAP / 2;
		double s = m ? sin(2 * M_PI * fc * m) / (M_PI * m) : 2 * fc;
		double w = 0.54 - 0.46 * cos(2 * M_PI * i / (TAP - 1));
		fir[i] = (float)(s * w);
		top += fir[i];
	}
	for (int i = 0; i < TAP; i++) fir[i] = (float)(fir[i] / top);
}

/* x[0..TAP-2] = son TAP-1 ornek (eskiden yeniye), x[TAP-1..] = bu turun ornekleri; faz v1 ile ayni anlam */
struct azalt { float x[TAP - 1 + AZ_PARCA]; int faz; };

static inline int16_t az_kirp(float s, float g)
{
	s *= g;
	return s > 32767 ? 32767 : s < -32768 ? -32768 : (int16_t)lrintf(s);
}

static int azalt(struct azalt *a, const int16_t *in, int n, int16_t *out, float g)
{
	int k = 0;
	while (n > 0) {
		int m = n < AZ_PARCA ? n : AZ_PARCA, i = 2 - a->faz;	/* v1: (faz + i + 1) % 3 == 0 olan ornekte cikis */
		float *x = a->x;
		for (int j = 0; j < m; j++) x[TAP - 1 + j] = in[j];
		for (; i + 9 < m; i += 12) {	/* 4 cikis: pencereler x + i, +3, +6, +9 */
			const float *w = x + i;
			float s0 = 0, s1 = 0, s2 = 0, s3 = 0;
			for (int t = 0; t < TAP; t++) {
				float f = fir[t];
				s0 += w[t] * f;
				s1 += w[t + 3] * f;
				s2 += w[t + 6] * f;
				s3 += w[t + 9] * f;
			}
			out[k++] = az_kirp(s0, g);
			out[k++] = az_kirp(s1, g);
			out[k++] = az_kirp(s2, g);
			out[k++] = az_kirp(s3, g);
		}
		for (; i < m; i += 3) {
			const float *w = x + i;
			float s = 0;
			for (int t = 0; t < TAP; t++) s += w[t] * fir[t];
			out[k++] = az_kirp(s, g);
		}
		a->faz = (a->faz + m) % 3;
		memmove(x, x + m, (TAP - 1) * sizeof *x);
		in += m;
		n -= m;
	}
	return k;
}
#endif
