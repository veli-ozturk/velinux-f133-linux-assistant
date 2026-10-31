/* uyandirma_v4.h (14.10.2026, Claude): v3 + uy_esik_nn (en yakin komsu tabanli esik; scripts/174).
 * uyandirma_v3.h (14.10.2026, Claude): v2 + parca_ham (ortalama cikarilmamis kepstrum, olcum icin; scripts/172).
 * uyandirma_v2.h (14.10.2026, Claude): v1 + UY_SESSIZ 0.3 sn, uyumsuz ciftsiz esik, uy_sablon_sec (scripts/170).
 * --- v1 ---
 * uyandirma_v1.h (14.10.2026, Claude): "Hey Köle" uyandirma kelimesi algilama cekirdegi (cihazdan bagimsiz, host testli).
 * kullanıcı: "hey köle diyince cevap versin, buyurun efendim gibi"; PC'siz, cihazda. Yontem (model/indirme yok):
 *   16 kHz mono -> 25 ms pencere / 10 ms adim, on vurgu 0.97, Hamming, 512 FFT, 24 mel suzgec (100..7000 Hz), log,
 *   DCT -> 12 kepstrum (c1..c12) = KAREBOY. Enerji tabanli konusma algilama (uyarlanir gurultu tabani).
 *   Konusma parcasi bitince (>= UY_SESSIZ kare sessizlik) parca, kullanicinin kendi sesinden ogrenilmis 3..8 sablonla
 *   DTW (Sakoe-Chiba bandi, yol uzunluguyla normalize, parca ortalamasi cikarilmis kepstrum) karsilastirilir;
 *   en kucuk uzaklik esigin altindaysa UYANDI. Esik ogrenmede sablonlarin birbirine uzakligindan hesaplanir.
 * Ham ses saklanmaz: sablonlar yalniz kepstrum dizileridir. */
/* uyandirma_v6.h (21.10.2026, Claude; scripts/472): v5 + TEMBEL kepstrum: sessizlikte karede yalniz enerji (ayni dongu);
 * kepstrum konusma surerken (ve UY_ON_KARE onceki kare) ya da parca alinirken uzun ornek halkasindan hesaplanir. Enerji,
 * gurultu tabani ve parca kepstrumlari v5 ile bit bit ayni (tests/uyandirma_kiyas_v2.c).
 * uyandirma_v5.h (21.10.2026, Claude; scripts/430): v4 + CPU: pencere halka tamponu (ornek basina memmove yok), mel
 * suzgecleri yalniz sifir olmayan aralikta. Ozellikler v4 ile bit bit ayni (tests/uyandirma_kiyas_v1.c). */
#ifndef UYANDIRMA_V6_H
#define UYANDIRMA_V6_H
#include <math.h>
#include <stdint.h>
#include <string.h>

#define UY_HZ 16000
#define UY_PENCERE 400
#define UY_ADIM 160
#define UY_FFT 512
#define UY_MEL 24
#define UY_BOY 12		/* kepstrum boyu (c1..c12) */
#define UY_EN_KISA 25		/* parca en az 0.25 sn */
#define UY_EN_UZUN 180		/* parca en cok 1.8 sn */
#define UY_SESSIZ 30		/* parca sonu: 0.30 sn sessizlik (v1 0.22: "hey ... kole" bolunuyordu) */
#define UY_TAMPON 260		/* kare halka tamponu (2.6 sn) */
#define UY_SABLON_N 8
#define UY_ON_KARE 4		/* parcanin onune/arkasina eklenen kare (yumusak baslangic) */
#define UY_UZUN (UY_TAMPON * UY_ADIM + UY_PENCERE + UY_ADIM)	/* v6: halka tamponundaki her karenin penceresi */

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct uy_sablon {
	int n;			/* kare sayisi */
	float k[UY_EN_UZUN + 2 * UY_ON_KARE][UY_BOY];
};

struct uy {
	/* ozellik cikarma */
	float pencere[UY_PENCERE], mel[UY_MEL][UY_FFT / 2 + 1], dct[UY_BOY][UY_MEL];
	float ornek2[2 * UY_PENCERE];	/* v5: halka (cift yazma); pencere = ornek2[oyaz .. oyaz+UY_PENCERE) */
	int oyaz;
	short mel_bas[UY_MEL], mel_son[UY_MEL];	/* v5: mel suzgecinin sifir olmayan kutu araligi [bas, son) */
	int dolu, adim_say;
	float onceki;			/* on vurgu durumu */
	/* konusma algilama */
	float gurultu, enerji_son;
	int konusma, sessiz_say, uzun;	/* uzun: cok uzun konusma birakildi, sessizlige kadar yeni parca yok */
	long parca_bas, kare_no;	/* mutlak kare numaralari */
	float halka[UY_TAMPON][UY_BOY];
	float halka_e[UY_TAMPON];
	long halka_no[UY_TAMPON];	/* v6: halka[h] kepstrumunun ait oldugu kare (-1: hesaplanmadi) */
	long halka_s[UY_TAMPON];	/* v6: karenin hesaplandigi andaki ornek sayaci (pencere = [s-UY_PENCERE, s)) */
	float ornek_uzun[UY_UZUN];		/* v6: on vurgulu ornek halkasi */
	int ornek_uzun_i;
	long ornek_say;
	/* parca sonucu */
	int parca_hazir, parca_n;
	struct uy_sablon parca;
	struct uy_sablon parca_ham;	/* v3: ortalama cikarilmadan (cevrimdisi olcum) */
};

/* ---------------------------------------------------------------- FFT (yerinde, radix-2, karmasik) */
static void uy_fft(float *re, float *im, int n)
{
	for (int i = 1, j = 0; i < n; i++) {
		int bit = n >> 1;
		for (; j & bit; bit >>= 1)
			j ^= bit;
		j ^= bit;
		if (i < j) {
			float t = re[i]; re[i] = re[j]; re[j] = t;
			t = im[i]; im[i] = im[j]; im[j] = t;
		}
	}
	for (int len = 2; len <= n; len <<= 1) {
		double a = -2 * M_PI / len;
		float wr = (float)cos(a), wi = (float)sin(a);
		for (int i = 0; i < n; i += len) {
			float cr = 1, ci = 0;
			for (int j = 0; j < len / 2; j++) {
				int u = i + j, v = i + j + len / 2;
				float tr = re[v] * cr - im[v] * ci, ti = re[v] * ci + im[v] * cr;
				re[v] = re[u] - tr; im[v] = im[u] - ti;
				re[u] += tr; im[u] += ti;
				float ncr = cr * wr - ci * wi;
				ci = cr * wi + ci * wr;
				cr = ncr;
			}
		}
	}
}

static float uy_hz_mel(float f) { return 1127.0f * logf(1.0f + f / 700.0f); }
static float uy_mel_hz(float m) { return 700.0f * (expf(m / 1127.0f) - 1.0f); }

static void uy_baslat(struct uy *u)
{
	memset(u, 0, sizeof *u);
	for (int i = 0; i < UY_PENCERE; i++)
		u->pencere[i] = (float)(0.54 - 0.46 * cos(2 * M_PI * i / (UY_PENCERE - 1)));
	float m0 = uy_hz_mel(100), m1 = uy_hz_mel(7000);
	float merkez[UY_MEL + 2];
	for (int i = 0; i < UY_MEL + 2; i++)
		merkez[i] = uy_mel_hz(m0 + (m1 - m0) * i / (UY_MEL + 1)) * UY_FFT / UY_HZ;
	for (int m = 0; m < UY_MEL; m++)
		for (int b = 0; b <= UY_FFT / 2; b++) {
			float a = merkez[m], c = merkez[m + 1], d = merkez[m + 2], w = 0;
			if (b > a && b <= c) w = (b - a) / (c - a);
			else if (b > c && b < d) w = (d - b) / (d - c);
			u->mel[m][b] = w;
		}
	for (int m = 0; m < UY_MEL; m++) {	/* v5 */
		int b0 = 0, b1 = 0;
		for (int b = 0; b <= UY_FFT / 2; b++)
			if (u->mel[m][b] > 0) {
				if (b1 == 0) b0 = b;
				b1 = b + 1;
			}
		u->mel_bas[m] = (short)b0;
		u->mel_son[m] = (short)b1;
	}
	for (int k = 0; k < UY_BOY; k++)
		for (int m = 0; m < UY_MEL; m++)
			u->dct[k][m] = (float)cos(M_PI * (k + 1) * (m + 0.5) / UY_MEL);
	u->gurultu = -1;	/* ilk karelerden ogrenilir */
	u->parca_bas = -1;
	for (int h = 0; h < UY_TAMPON; h++)	/* v6 */
		u->halka_no[h] = -1;
}

/* bir pencereden log enerji; cep NULL degilse kepstrum da (v6: v5 uy_kare ile ayni pencereleme/enerji dongusu) */
static float uy_kare_o(struct uy *u, const float *o, float *cep)
{
	float re[UY_FFT], im[UY_FFT];
	double e = 0;
	for (int i = 0; i < UY_PENCERE; i++) {
		re[i] = o[i] * u->pencere[i];
		e += (double)o[i] * o[i];
	}
	if (!cep)
		return (float)(10.0 * log10(e / UY_PENCERE + 1.0));	/* dB */
	memset(re + UY_PENCERE, 0, sizeof re - UY_PENCERE * sizeof re[0]);
	memset(im, 0, sizeof im);
	uy_fft(re, im, UY_FFT);
	float g[UY_FFT / 2 + 1], lm[UY_MEL];
	for (int b = 0; b <= UY_FFT / 2; b++)
		g[b] = re[b] * re[b] + im[b] * im[b];
	for (int m = 0; m < UY_MEL; m++) {
		double s = 1e-3;
		for (int b = u->mel_bas[m]; b < u->mel_son[m]; b++)	/* v5: disarida agirlik 0 */
			if (u->mel[m][b] > 0)
				s += u->mel[m][b] * g[b];
		lm[m] = (float)log(s);
	}
	for (int k = 0; k < UY_BOY; k++) {
		float s = 0;
		for (int m = 0; m < UY_MEL; m++)
			s += u->dct[k][m] * lm[m];
		cep[k] = s;
	}
	return (float)(10.0 * log10(e / UY_PENCERE + 1.0));	/* dB */
}

/* v6: f karesinin kepstrumu yoksa uzun halkadaki penceresinden hesapla (uy_kare_o: ayni sayilar) */
static void uy_kep_hazirla(struct uy *u, long f)
{
	int h = (int)(f % UY_TAMPON);
	if (u->halka_no[h] == f)
		return;
	float o[UY_PENCERE];
	int j = (int)((u->halka_s[h] - UY_PENCERE) % UY_UZUN);
	for (int i = 0; i < UY_PENCERE; i++) {
		o[i] = u->ornek_uzun[j];
		if (++j == UY_UZUN)
			j = 0;
	}
	uy_kare_o(u, o, u->halka[h]);
	u->halka_no[h] = f;
}

/* parcayi halka tamponundan cikar, kepstrum ortalamasini cikar */
static void uy_parca_al(struct uy *u, long bas, long son)
{
	bas -= UY_ON_KARE;
	son += UY_ON_KARE;
	if (bas < u->kare_no - UY_TAMPON + 1)
		bas = u->kare_no - UY_TAMPON + 1;
	if (bas < 0)
		bas = 0;
	if (son > u->kare_no)
		son = u->kare_no;
	int n = (int)(son - bas);
	if (n > UY_EN_UZUN + 2 * UY_ON_KARE)
		n = UY_EN_UZUN + 2 * UY_ON_KARE;
	float ort[UY_BOY] = {0};
	for (int i = 0; i < n; i++)
		uy_kep_hazirla(u, bas + i);	/* v6: konusmada hesaplanmayan (ornegin parca sonrasi) kare */
	for (int i = 0; i < n; i++)
		for (int k = 0; k < UY_BOY; k++) {
			u->parca.k[i][k] = u->halka[(bas + i) % UY_TAMPON][k];
			ort[k] += u->parca.k[i][k];
		}
	memcpy(u->parca_ham.k, u->parca.k, sizeof(float) * UY_BOY * (size_t)(n > 0 ? n : 0));
	u->parca_ham.n = n;
	for (int k = 0; k < UY_BOY; k++)
		ort[k] /= n > 0 ? n : 1;
	for (int i = 0; i < n; i++)
		for (int k = 0; k < UY_BOY; k++)
			u->parca.k[i][k] -= ort[k];
	u->parca.n = n;
	u->parca_n = n;
	u->parca_hazir = 1;
}

/* 16 kHz ornekleri isle. Bir parca tamamlaninca HEMEN 1 doner (u->parca dolu); *tuketilen o ana kadar islenen ornek
 * sayisidir, cagiran kalan ornekleri yeniden verir. Parca yoksa 0 doner ve tum ornekler tuketilmistir. */
static int uy_besle(struct uy *u, const int16_t *x, int n, int *tuketilen)
{
	int hazir = 0;
	*tuketilen = n;
	for (int i = 0; i < n; i++) {
		float v = x[i] - 0.97f * u->onceki;
		u->onceki = x[i];
		u->ornek2[u->oyaz] = u->ornek2[u->oyaz + UY_PENCERE] = v;	/* v5: memmove yerine halka */
		if (++u->oyaz == UY_PENCERE) u->oyaz = 0;
		u->ornek_uzun[u->ornek_uzun_i] = v;	/* v6: tembel kepstrum icin */
		if (++u->ornek_uzun_i == UY_UZUN) u->ornek_uzun_i = 0;
		u->ornek_say++;
		if (u->dolu < UY_PENCERE) { u->dolu++; continue; }
		if (++u->adim_say < UY_ADIM) continue;
		u->adim_say = 0;
		float e = uy_kare_o(u, u->ornek2 + u->oyaz, NULL);	/* v6: yalniz enerji; kepstrum gerekince */
		int h = (int)(u->kare_no % UY_TAMPON);
		u->halka_no[h] = -1;
		u->halka_s[h] = u->ornek_say;
		u->halka_e[h] = e;
		u->enerji_son = e;
		/* gurultu tabani: sessizde yavas izler, konusmada donar; asagi hizli iner */
		if (u->gurultu < 0) u->gurultu = e;
		else if (e < u->gurultu) u->gurultu = 0.9f * u->gurultu + 0.1f * e;
		else if (!u->konusma) u->gurultu = 0.995f * u->gurultu + 0.005f * e;
		int ses = e > u->gurultu + 9.0f && e > 22.0f;
		if (u->uzun) {
			u->sessiz_say = ses ? 0 : u->sessiz_say + 1;
			if (u->sessiz_say >= UY_SESSIZ) { u->uzun = 0; u->sessiz_say = 0; }
		} else if (ses) {
			if (!u->konusma) { u->konusma = 1; u->parca_bas = u->kare_no; }
			u->sessiz_say = 0;
		} else if (u->konusma && ++u->sessiz_say >= UY_SESSIZ) {
			long bas = u->parca_bas, son = u->kare_no - u->sessiz_say + 1;
			long uz = son - bas;
			u->konusma = 0;
			u->sessiz_say = 0;
			if (uz >= UY_EN_KISA && uz <= UY_EN_UZUN) {
				uy_parca_al(u, bas, son);
				hazir = 1;
			}
		}
		if (u->konusma && u->kare_no - u->parca_bas > UY_EN_UZUN + UY_SESSIZ) {
			u->konusma = 0;	/* cok uzun konusma: uyandirma degil, birak; sessizlige kadar bekle */
			u->uzun = 1;
			u->sessiz_say = 0;
		}
		if (u->konusma) {	/* v6: konusmada kepstrum hemen (UY_ON_KARE onceki kare dahil): parca sonunda yigilmasin */
			long f0 = u->parca_bas - UY_ON_KARE;
			if (f0 < u->kare_no - UY_TAMPON + 1) f0 = u->kare_no - UY_TAMPON + 1;
			if (f0 < 0) f0 = 0;
			for (long f = f0; f <= u->kare_no; f++)
				uy_kep_hazirla(u, f);
		}
		u->kare_no++;
		if (hazir) { *tuketilen = i + 1; return 1; }
	}
	return 0;
}

/* DTW: Sakoe-Chiba bandi (uzunlugun %40'i, en az 12), oklid uzakligi, yol uzunluguyla normalize */
static float uy_dtw(const struct uy_sablon *a, const struct uy_sablon *b)
{
	int n = a->n, m = b->n;
	if (n < 2 || m < 2 || n > 2 * m + 10 || m > 2 * n + 10)
		return 1e9f;
	int bant = (n > m ? n : m) * 2 / 5;
	if (bant < 12) bant = 12;
	static float D[UY_EN_UZUN + 2 * UY_ON_KARE + 1][UY_EN_UZUN + 2 * UY_ON_KARE + 1];
	static int L[UY_EN_UZUN + 2 * UY_ON_KARE + 1][UY_EN_UZUN + 2 * UY_ON_KARE + 1];
	for (int i = 0; i <= n; i++)
		for (int j = 0; j <= m; j++) { D[i][j] = 1e30f; L[i][j] = 0; }
	D[0][0] = 0;
	for (int i = 1; i <= n; i++) {
		int jm = (int)((long)i * m / n);
		int j0 = jm - bant < 1 ? 1 : jm - bant, j1 = jm + bant > m ? m : jm + bant;
		for (int j = j0; j <= j1; j++) {
			float d = 0;
			for (int k = 0; k < UY_BOY; k++) {
				float t = a->k[i - 1][k] - b->k[j - 1][k];
				d += t * t;
			}
			d = sqrtf(d);
			float best = D[i - 1][j - 1];
			int bl = L[i - 1][j - 1];
			if (D[i - 1][j] < best) { best = D[i - 1][j]; bl = L[i - 1][j]; }
			if (D[i][j - 1] < best) { best = D[i][j - 1]; bl = L[i][j - 1]; }
			D[i][j] = best + d;
			L[i][j] = bl + 1;
		}
	}
	return L[n][m] ? D[n][m] / L[n][m] : 1e9f;
}

/* sablonlara en kucuk uzaklik */
static float uy_en_yakin(const struct uy_sablon *p, const struct uy_sablon *s, int sn)
{
	float en = 1e9f;
	for (int i = 0; i < sn; i++) {
		float d = uy_dtw(p, &s[i]);
		if (d < en) en = d;
	}
	return en;
}

/* esik: sablonlarin ikili uzakliklarinin ortalamasi * carpan (varsayilan UY_ESIK_CARPAN); en az 2 sablon */
#define UY_ESIK_CARPAN 1.6f	/* host testi: ayni kelime 5.8..9.2, baska kelime >= 40 (ortalama 6.3) */
__attribute__((unused)) static float uy_esik_hesapla(const struct uy_sablon *s, int sn, float carpan, float *ort_cikti)
{
	double top = 0;
	int say = 0;
	for (int i = 0; i < sn; i++)
		for (int j = i + 1; j < sn; j++) {
			float d = uy_dtw(&s[i], &s[j]);
			if (d >= 1e8f)
				continue;	/* v2: uzunluk uyumsuz cift ortalamayi bozmasin */
			top += d;
			say++;
		}
	float ort = say ? (float)(top / say) : 0;
	if (ort_cikti) *ort_cikti = ort;
	return ort * carpan;
}

/* v4: esik = her sablonun DIGER sablonlara en kucuk uzakliginin ortalamasi x carpan (cihaz olcumu: 1.2).
 * Algilama en yakin sablona uzaklikla yapildigi icin dogru olcek budur (ikili ortalama 1.6x olcumde fazla gevsekti). */
#define UY_NN_CARPAN 1.2f
static float uy_esik_nn(const struct uy_sablon *s, int sn, float carpan, float *nn_ort)
{
	double top = 0;
	int say = 0;
	for (int i = 0; i < sn; i++) {
		float en = 1e9f;
		for (int j = 0; j < sn; j++)
			if (j != i) {
				float d = uy_dtw(&s[i], &s[j]);
				if (d < en) en = d;
			}
		if (en < 1e8f) { top += en; say++; }
	}
	float o = say ? (float)(top / say) : 0;
	if (nn_ort) *nn_ort = o;
	return o * carpan;
}

/* v2: ortanca uzunlugun 0.7..1.4 kati icindeki ornekleri out'a kopyalar; kopyalanan sayiyi dondurur */
static int uy_sablon_sec(const struct uy_sablon *in, int n, struct uy_sablon *out)
{
	int u[UY_SABLON_N], m = 0;
	for (int i = 0; i < n && i < UY_SABLON_N; i++)
		u[m++] = in[i].n;
	for (int i = 1; i < m; i++)
		for (int j = i; j > 0 && u[j - 1] > u[j]; j--) {
			int t = u[j]; u[j] = u[j - 1]; u[j - 1] = t;
		}
	if (!m)
		return 0;
	float ortanca = m % 2 ? u[m / 2] : (u[m / 2 - 1] + u[m / 2]) / 2.0f;
	int k = 0;
	for (int i = 0; i < n && i < UY_SABLON_N; i++)
		if (in[i].n >= 0.7f * ortanca && in[i].n <= 1.4f * ortanca)
			out[k++] = in[i];
	return k;
}
#endif
