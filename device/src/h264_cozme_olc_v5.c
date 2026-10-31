/* h264_cozme_olc_v5 (15.10.2026, Claude): v4 + H264_SKIP_LF=1 (deblocking atla), H264_FAST=1 (flags2 fast) denemeleri.
 * h264_cozme_olc_v4 (15.10.2026, Claude): v3 + islemci zamani (CLOCK_PROCESS_CPUTIME_ID): diger hizmetlerin payi ayrilir.
 * h264_cozme_olc_v3 (15.10.2026, Claude): v2 + son kare: bitis yalniz bosaltma cagrisinda (v2 son baytlari tuketince duruyordu).
 * h264_cozme_olc_v2 (15.10.2026, Claude): v1 + dogru akis sonu bosaltma (v1 son kareyi saymiyordu), yalniz-cozme kare/sn.
 * h264_cozme_olc_v1 (15.10.2026, Claude): OTONOM 5 asama 2 - C906 uzerinde yazilim H.264 cozme olcumu (FFmpeg 7.1
 * libavcodec, yalniz h264 cozucu + ayristirici, asm/RVV yok, tek is parcacigi; scripts/245).
 * Kullanim: h264-cozme-olc <akis.h264> [tekrar]
 * Cikti: kare sayisi, I/P kare suresi ort/en cok, toplam kare/sn, tepe RSS (VmHWM), son karenin Y saglamasi (PC'deki
 * PyAV cozumuyle karsilastirilir: work/h264_cozme_hazirla_v1.py). Yalniz okur; hicbir yere yazmaz. */
#include <libavcodec/avcodec.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static double simdi_ms(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

static double cpu_ms(void)
{
	struct timespec t;
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t);
	return t.tv_sec * 1e3 + t.tv_nsec / 1e6;
}

static long vm(const char *ad)
{
	FILE *f = fopen("/proc/self/status", "r");
	char s[256];
	long v = -1;
	while (f && fgets(s, sizeof s, f))
		if (!strncmp(s, ad, strlen(ad))) v = atol(s + strlen(ad) + 1);
	if (f) fclose(f);
	return v;
}

struct ist { int n; double top, enc; };

int main(int argc, char **argv)
{
	if (argc < 2) { fprintf(stderr, "kullanim: %s akis.h264 [tekrar]\n", argv[0]); return 2; }
	int tekrar = argc > 2 ? atoi(argv[2]) : 1;
	FILE *f = fopen(argv[1], "rb");
	if (!f) { perror("akis"); return 2; }
	fseek(f, 0, SEEK_END);
	long boy = ftell(f);
	rewind(f);
	uint8_t *veri = malloc((size_t)boy + AV_INPUT_BUFFER_PADDING_SIZE);
	if (!veri || fread(veri, 1, (size_t)boy, f) != (size_t)boy) return 2;
	memset(veri + boy, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	fclose(f);
	long rss0 = vm("VmRSS:");
	const AVCodec *kod = avcodec_find_decoder(AV_CODEC_ID_H264);
	AVCodecParserContext *ay = av_parser_init(AV_CODEC_ID_H264);
	AVCodecContext *c = avcodec_alloc_context3(kod);
	c->flags |= AV_CODEC_FLAG_LOW_DELAY;
	c->thread_count = 1;
	if (getenv("H264_SKIP_LF")) c->skip_loop_filter = AVDISCARD_ALL;
	if (getenv("H264_FAST")) c->flags2 |= AV_CODEC_FLAG2_FAST;
	if (!kod || !ay || !c || avcodec_open2(c, kod, NULL) < 0) { fputs("cozucu acilamadi\n", stderr); return 1; }
	AVPacket *pk = av_packet_alloc();
	AVFrame *fr = av_frame_alloc();
	struct ist I = {0}, P = {0};
	uint32_t sagla = 0;
	int w = 0, h = 0, kare = 0;
	double t_bas = simdi_ms(), t_ilk_cikti = -1;
	double t_coz = 0, c_coz = 0;
	for (int t = 0; t < tekrar; t++) {
		const uint8_t *p = veri;
		long kalan = boy;
		for (int bitti = 0; !bitti;) {	/* FFmpeg decode_video ornegi: veri bitince ayristirici NULL,0 ile bosaltilir */
			uint8_t *pd = NULL;
			int pn = 0, bos = kalan <= 0;
			int k = av_parser_parse2(ay, c, &pd, &pn, kalan > 0 ? p : NULL, (int)(kalan > 0 ? kalan : 0), AV_NOPTS_VALUE,
						 AV_NOPTS_VALUE, 0);
			if (k < 0) { fputs("ayristirma hatasi\n", stderr); return 1; }
			p += k;
			kalan -= k;
			if (bos && !pn)
				bitti = 1;
			if (!pn && !bitti)
				continue;
			pk->data = pn ? pd : NULL;
			pk->size = pn;
			double t0 = simdi_ms(), c0 = cpu_ms();
			if (avcodec_send_packet(c, pk) < 0 && pn) { fputs("send_packet hatasi\n", stderr); continue; }
			while (avcodec_receive_frame(c, fr) == 0) {
				double d = simdi_ms() - t0;
				t_coz += d;
				c_coz += cpu_ms() - c0;
				struct ist *s = fr->pict_type == AV_PICTURE_TYPE_I ? &I : &P;
				s->n++;
				s->top += d;
				if (d > s->enc) s->enc = d;
				if (t_ilk_cikti < 0) t_ilk_cikti = simdi_ms() - t_bas;
				w = fr->width;
				h = fr->height;
				kare++;
				sagla = 0;
				for (int y = 0; y < h; y++)
					for (int x = 0; x < w; x++) sagla = sagla * 31u + fr->data[0][(size_t)y * fr->linesize[0] + x];
				t0 = simdi_ms();
				c0 = cpu_ms();
			}
		}
		avcodec_flush_buffers(c);
	}
	double top = simdi_ms() - t_bas;
	printf("akis %s: %ld bayt, %dx%d, %d kare\n", argv[1], boy, w, h, kare);
	printf("  I kare: %d adet, ort %.1f ms, en cok %.1f ms\n", I.n, I.n ? I.top / I.n : 0, I.enc);
	printf("  P kare: %d adet, ort %.1f ms, en cok %.1f ms\n", P.n, P.n ? P.top / P.n : 0, P.enc);
	printf("  toplam %.0f ms -> %.1f kare/sn (saglama dahil); yalniz cozme %.0f ms -> %.1f kare/sn; ilk kare %.0f ms\n", top,
	       kare * 1000.0 / top, t_coz, kare * 1000.0 / t_coz, t_ilk_cikti);
	printf("  islemci: cozme %.0f ms -> kare basi %.1f ms islemci (duvar %.1f ms; fark = diger hizmetler)\n", c_coz,
	       kare ? c_coz / kare : 0, kare ? t_coz / kare : 0);
	printf("  bellek: RSS basta %ld kB, tepe VmHWM %ld kB\n", rss0, vm("VmHWM:"));
	printf("  son kare Y saglamasi %08x\n", sagla);
	av_frame_free(&fr);
	av_packet_free(&pk);
	avcodec_free_context(&c);
	av_parser_close(ay);
	free(veri);
	return 0;
}
