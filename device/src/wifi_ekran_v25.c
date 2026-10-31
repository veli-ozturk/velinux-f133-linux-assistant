/* wifi_ekran_v25 (29.10.2026, Claude; scripts/671, §194): v24 + G2D ara tamponu ayrilamazsa (CMA anlik EBUSY)
 * /proc/sys/vm/compact_memory + 300 ms bekleme ile 5 kez yeniden denenir (v24 dogrudan cizimi kapatiyordu).
 * --- v24 ---
 * wifi_ekran_v24 (26.10.2026, Claude; docs §185): v23 + v22'nin 180 derece destegi (pad[10]=1, /tmp/pc-dogrudan 3. alan
 * ters -> G2D ROT_270). v23 (24.10) eski v21 kaynagindan derlenmisti; SD'deki wifi-ekran-v21 ise 21.10'da v22 koduyla
 * guncellenmisti. Panel 0.64+ 180 derecede ancak pad[10] bildirene devreder -> v23 hic sahip olamadi, ikinci ekran
 * yavas yoldan (dk 454 kare / 308 takilma; v21(=v22 kodu) 183 / 0). */
/* wifi_ekran_v23 (23.10.2026, Claude; docs/denetim-2026-10-23-v2.md bulgu 6, 7; scripts/603): v21 + bozuk akis
 * dogrulamasi: JPEG DHT/DQT/SOF/SOS bolum sinirlari ve SOS tablo secicisi <= 3 (yoksa yazilim cozucu); h264_ve_v3.h (ref
 * sayisi <= 32, yalniz 1600x608 SPS, tampon ayirmadan once); 5 VE hatasinda yalniz JPEG VE yolu kapanir (v21 ve_bitir
 * H.264'un kullandigi yazmac/tamponlari birakiyordu); 8766'da izinli_ip_v1.h; --self-test. v22 taslagi (180 derece)
 * kurulmadi, bu v21 tabanli. */
/* wifi_ekran_v21 (15.10.2026, Claude): v20 + G2D sahiplik degisimi gunlugu (scripts/270). */
/* wifi_ekran_v20 (15.10.2026, Claude): v19 + mlockall(MCL_ONFAULT) (takas kaynakli donma) + takilma gunlugu (scripts/267). */
/* wifi-ekran v19 (15.10.2026, Claude): v18 + h264_ve_v2.h: JPEG VE tamponlari H.264 ile paylasilir (OOM; scripts/258). */
/* wifi-ekran v18 (15.10.2026, Claude): v17 + H.264 VE donanim yolu (rf bit 2, tam kare), --h264 dosya kipi (scripts/256). */
/* v17 (14.10.2026, Claude): v16 + panel 0.27 IKI PARMAK kayitlari da iletilir: tip 4/5 teker yukari/asagi, 6/7
 * yakinlastir/uzaklastir (PC'de Ctrl+teker), 8/9 yatay teker sola/saga; x,y parmaklarin ortasi. Baska degisiklik yok.
 * PC gondericisi v27 (fare_v2) gerekir; eski gondericiler 4..9'u yok sayar (fare_v1 yalniz imleci tasir).
 */
/* v16 (11.10.2026, Claude): v15 + (1) dogru UV sirasi varsayilan: G2D'nin YUV420UVC_V1U1V0U0 bicimi bizim NV12'mizi
 * dogru okuyor (v15 olcumu: U1V1U0V0 ile kirmizi->mavi; kullanıcı "renk bozulmasi", "daha mavi"). DG_NV12=1 eskiye doner.
 * (2) sahipken kopya dikdortgeni ara tamponda/fb'de yalniz o alan kadar kopyalanir + dondurulur; tam ekran
 * fb->ortak bellek esitlemesi (3.84 MB onbelleksiz okuma, 40 sn'de 7 kez, fps 12-14'e dusus) yalniz devirde.
 */
/* v15 (11.10.2026, Claude): v14 + RENK TONU OLCUMU. kullanıcı: "goruntu genel olarak daha mavi". Cekirdek g2d_bld.c
 * Ycbcr2rgb_601 "full to full" tablosu Y'den fazladan 16 cikariyor (R/G/B ofsetleri -194.5/119.4/-242.9, beklenen
 * -179.5/135.4/-226.8). --file ... --g2d --ornek: G2D sonucu ile ayni JPEG'in CPU (VE+yazilim donusumu) sonucunu
 * test blogunun serit/rampa noktalarinda R,G,B olarak yazar.
 */
/* v14 (11.10.2026, Claude): v13 + RENK BOZULMASI TANISI. kullanıcı: G2D yolunda "arada kare kare renk bozulmasi"
 * (v12 yatay ve v13 dikey fb'de; devirle ilgisiz). Supheli: VE durum biti geldikten sonra cikis yazimi suruyor,
 * G2D erken okuyor (CPU yolu once DMA_BUF esitlemesiyle ms'ler bekliyordu). DG_DOGRULA=1: VE bittikten hemen
 * sonra ve 2 ms sonra cikisin son luma + chroma satirlarinin toplami karsilastirilir; fark sayisi pad[8],
 * denetim sayisi pad[9] (gunlukte de). DG_BEKLE_US=n: G2D'den once n us bekle (deneme).
 */
/* v13 (11.10.2026, Claude): v12 + DIKEY FB (flash cekirdegi: fb0 600x1600, G2D fb dondurmesi yok). Mod 2'de
 * VE NV12 -> (G2D mixer) yatay ARGB ara tampon (ION 1600x600, VE IOMMU adresi) -> (G2D dondurme birimi, ROT_90)
 * dikey fb0'a; mantiksal (x,y) -> fiziksel (599-y, x) panel flush ile ayni. FBIOPAN yok. Ara tampon ekranin tam
 * yatay kopyasidir: devir birakilirken ortak bellek bundan esitlenir. DG_ROT270=1 yonu cevirir (prova icin).
 * Mod 1 (fb 1600x600 rotate=1, RAM G2D cekirdegi) v12 ile ayni. v12 RAM provasinda calisti; kullanıcı: "baya iyi".
 */
/* v12 (11.10.2026, Claude): G2D DOGRUDAN CIZIM. Panel 0.18 /tmp/pc-dogrudan dosyasina {nesil, izin} yazar
 * (PC tam ekran, cubuk/ust katman kapali). Alici izni ortak bellek basligina (pad[0]=nesil, pad[1]=izin) yazarak
 * onaylar ve sahip olur: VE'nin NV12 ciktisi tek G2D bitblt (renk donusumu + olcek2 icin buyutme) ile fb0'a
 * (1600x600 ARGB, smem_start, use_phy_addr) yazilir, kare sonunda FBIOPAN (cekirdek G2D 90 derece dondurme).
 * Islemci NV12->RGB565 ve RGB565->XRGB donusumu yapmaz. Yazilim cozulen (4:4:4) ve kopya dikdortgenleri ortak
 * bellege cozulur ve islemciyle fb'ye kopyalanir. Sahiplik alininca ortak bellek -> fb tam kopya (panelin
 * cizmedigi dikdortgen kalmasin); birakilinca fb -> ortak bellek tam kopya + tam ekran dikdortgeni (LVGL guncel
 * goruntuyu cizsin). G2D yoksa / fb dondurmesiz cekirdekte v11 davranisi. --file ... --g2d: VE+G2D tek kare testi.
 * Istatistik pad[2..7]: g2d sayisi, g2d us, pan sayisi, pan us, cpu dikdortgen, esitleme sayisi. */
/* v11: true 4:4:4 software text refresh, existing VE420 motion path preserved. */
/* wifi-ekran v10 (11.10.2026, Claude): v9 + HIZLI NV12->RGB565. v9 olcumu: tam kare 4:2:0 58.5 ms (VE ~9-16 ms, kalan
 * donusum). v10: olcek2'siz ve dogrudan (onbellekli) okumada 2x2 blok basina renk katkisi bir kez, 565'e kaydirilmis
 * kirpma tablolari, iki piksel tek 32 bit yazma. Olcek2 / tek sayili kenar / onbelleksiz yol v9 ile ayni.
 * --- v9 ---
 * wifi-ekran v9 (11.10.2026, Claude): v8 + DMA-BUF SENKRON YOLU. v8 olcumu: C906 dcache komutlari kullanici kipinde
 * SIGILL (izin yok); onbelleksiz satir kopyasiyla tam kare 96 ms (VE 16 ms + okuma/donusum). v9: cikis tamponu
 * ION_FLAG_CACHED, VE bittikten sonra DMA_BUF_IOCTL_SYNC(START|READ) -> cekirdek dma_wbinv_range (VE eklentisi
 * MAP_DMA_BUF ile esli oldugu icin), okuma onbellekli; sonra SYNC(END|READ). Sira: dcache komutu > dma-buf sync >
 * onbelleksiz satir kopyasi. Dosya kipinde --ikinci <jpg>: ayni surecte ikinci JPEG (bayat onbellek denetimi).
 * --- v8 ---
 * wifi-ekran v8 (11.10.2026, Claude): v7 + HIZLI NV12 OKUMA. v7 akista VE yolu yazilimdan YAVASTI (tam kare 457 ms):
 * VE cikis tamponu onbelleksiz eslenmisti ve NV12->RGB565 donusumu her bayti ayri DRAM erisimiyle okuyordu (~190 ns).
 * v8: cikis tamponu ION_FLAG_CACHED; VE'den once C906 'dcache.civa' (temizle+gecersiz), sonra 'dcache.iva' (gecersiz)
 * kullanici kipinden (mxstatus.UCME=1, stub 0x638000). Acilista SIGILL denemesi: izin yoksa onbelleksiz tampon +
 * satir satir 8 baytlik okuma (memcpy) yolu. Olcum icin her donusum/cozme suresi ACK decode_us'e dahil.
 * --- v7 ---
 * wifi-ekran v7 (11.10.2026, Claude): v6 + CEDAR VE DONANIMSAL JPEG COZME. Her dikdortgen once VE ile cozulur
 * (ve_jpeg_v1 prototipi: 1600x600 masaustu 8.4 ms, yazilim 164 ms); NV12 cikis RGB565'e (olcek2 ise 2x) donusturulur.
 * VE yoksa (/dev/cedar_dev yok: flash DTB'de VE kapali), JPEG uygun degilse (gri, 4:1:1...) ya da VE hata verirse
 * yazilima (TJpgDec) donulur; 5 VE hatasindan sonra VE kapatilir. Kesme acilmaz (durum yoklanir), bellek ION+IOMMU.
 * --- v6 ---
 * wifi-ekran v6 (11.10.2026, Claude): v5 + DOKUNMA ILETIMI. Panel (0.15) tam ekrandaki dokunmalari FIFO
 * /tmp/pc-dokunma'ya 8 baytlik kayitlar olarak yazar (tip:u8 1=bas 2=surukle 3=birak, 0, x:u16le, y:u16le, 0:u16);
 * alici bunlari ayni TCP baglantisindan PC'ye 'VED4' tip x y (4 x u32 big-endian, 16 B) mesaji olarak gonderir.
 * PC'de yeni gelen baglanti / guvenlik duvari izni gerekmez. Dokunmalar yalniz alici veri BEKLERKEN iletilir
 * (ACK yazilirken araya girmez). FIFO alici tarafindan O_RDWR acik tutulur (panel her zaman yazabilir).
 * --- v5 ---
 * wifi-ekran v5 (11.10.2026, Claude): v4 + KOPYA dikdortgeni (VNC CopyRect benzeri). rflags bit1: veri 4 B
 * (kaynak x:u16, y:u16); cihaz onceki karedeki kaynak blogu hedefe kopyalar (pencere surukleme/kaydirma: JPEG cozme yok,
 * tam kalite). Bir karedeki ardisik kopyalar once HEPSI okunup (tampon) sonra yazilir: ust uste binen kaynak/hedef
 * dogru calisir. Kopyalar JPEG'lerden once uygulanir (PC once kopyalari yollar). Geri kalan her sey v4 ile ayni.
 * --- v4 ---
 * wifi-ekran v4 (11.10.2026, Claude): Wi-Fi uzerinden kirli-dikdortgen JPEG alicisi -> ortak bellek (pc_shm_v1.h).
 * v3'ten farki: tam kare yerine yalniz degisen dikdortgenler; JPEG'ler dogrudan /tmp/pc-ekran.shm'deki
 * 1600x600 RGB565 kareye cozulur (dosya yazma/okuma/checksum yok); "olcek2" bayragiyla yarim cozunurluklu
 * JPEG 2x buyutulerek yazilir (hareketli icerik). Panel ayni bellegi okuyup yalniz kirli alanlari cizer.
 * Framebuffer, UART, SD, flash ERISIMI YOK. Sure sinirli (--seconds 5..3600), tek istemci.
 *
 * Tel protokolu (big-endian):
 *   PC -> cihaz kare : 'VEK4' seq nrect flags(bit0 tam ekran iste)                    16 B
 *   her dikdortgen   : x:u16 y:u16 w:u16 h:u16 rflags:u16(bit0 olcek2) 0:u16 len:u32  16 B + len B baseline JPEG
 *   cihaz -> PC ACK  : 'VEA4' seq status decode_us nrect bytes                         24 B
 *   nrect=0 canlilik karesidir (PC en az 1 sn'de bir gonderir). Hata: status!=0 ACK + baglanti kapanir.
 *   wifi-ekran-v4 --file x.jpg [--x X --y Y --olcek2]: yalniz cozme testi (host). */
#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE	/* v23: izinli_ip_v1.h inet_aton */
#include "tjpgd.h"
#include "pc_shm_v1.h"
#include <arpa/inet.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <setjmp.h>
#include <linux/dma-buf.h>
#include <linux/fb.h>
#include <stdbool.h>
#include "g2d_driver_tina.h"
#include "h264_ve_v3.h"	/* v23 (v19: v2) */
#include "izinli_ip_v1.h"	/* v23 */
#include <time.h>
#include <sys/mman.h>	/* v20 */
#include <sys/resource.h>
#ifndef MCL_ONFAULT
#define MCL_ONFAULT 4
#endif

#define MAX_JPEG (512u * 1024u)
#define MAX_RECT 256u
#define MAGIC_FRAME 0x56454B34u
#define MAGIC_ACK 0x56454134u

static uint8_t jpeg[MAX_JPEG], pool[65536];
static uint8_t *shm_map;
static volatile pc_shm_hdr_t *H;
static uint16_t *frame;
static uint16_t file_frame[PC_SHM_W * PC_SHM_H];	/* --file modu */
static uint16_t kopya_tampon[PC_SHM_W * PC_SHM_H] __attribute__((aligned(8)));	/* v16: 32 bit kopya icin hizali */
struct kopya { unsigned sx, sy, x, y, w, h; };
static struct kopya kq[MAX_RECT];
static unsigned kn;
static volatile sig_atomic_t stopped;
static uint32_t deadline_ms;

static void stop(int s) { (void)s; stopped = 1; }
static uint32_t now(void) { return pc_shm_now_ms(); }
static int timeup(void) { return stopped || (int32_t)(now() - deadline_ms) >= 0; }

struct src { size_t size, off; unsigned ox, oy, sc; };

static size_t in_cb(JDEC *jd, uint8_t *buf, size_t n)
{
	struct src *s = jd->device;
	if (n > s->size - s->off)
		n = s->size - s->off;
	if (buf)
		memcpy(buf, jpeg + s->off, n);
	s->off += n;
	return n;
}

/* LVGL'nin TJpgDec kopyasi B,G,R verir -> RGB565 (R yuksek bitlerde). */
static inline uint16_t c565(const uint8_t *p)
{
	return (uint16_t)(((p[2] & 0xf8) << 8) | ((p[1] & 0xfc) << 3) | (p[0] >> 3));
}

static int out_cb(JDEC *jd, void *bitmap, JRECT *r)
{
	struct src *s = jd->device;
	const uint8_t *p = bitmap;
	if (r->left > r->right || r->top > r->bottom || r->right >= jd->width || r->bottom >= jd->height)
		return 0;
	unsigned w = r->right - r->left + 1;
	for (unsigned y = r->top; y <= r->bottom; y++) {
		if (!s->sc) {
			uint16_t *d = frame + (size_t)(s->oy + y) * PC_SHM_W + s->ox + r->left;
			for (unsigned x = 0; x < w; x++, p += 3)
				d[x] = c565(p);
		} else {
			uint16_t *d0 = frame + (size_t)(s->oy + 2 * y) * PC_SHM_W + s->ox + 2 * r->left, *d1 = d0 + PC_SHM_W;
			for (unsigned x = 0; x < w; x++, p += 3) {
				uint16_t v = c565(p);
				d0[2 * x] = d0[2 * x + 1] = d1[2 * x] = d1[2 * x + 1] = v;
			}
		}
	}
	return !stopped;
}

#define VE_RESET 0x104
#define VE_ENGINE_REQ 0x206
#define VE_ENGINE_REL 0x207
#define VE_MAP_DMA_BUF 0x504
#define VE_UNMAP_DMA_BUF 0x505
struct dma_buf_param { int fd; unsigned int phy_addr; };
struct ion_allocation_data { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
struct ion_heap_data { char name[32]; uint32_t type, heap_id, r0, r1, r2; };
struct ion_heap_query { uint32_t cnt, r0; uint64_t heaps; uint32_t r1, r2; };
#define ION_IOC_ALLOC _IOWR('I', 0, struct ion_allocation_data)
#define ION_IOC_HEAP_QUERY _IOWR('I', 8, struct ion_heap_query)

/* yazmaclar (VE tabani 0x01C0E000'a gore) */
#define VE_MODE 0x00
#define VE_PRIMARY_CHROMA_BUF_LEN 0xc4
#define VE_PRIMARY_FB_LINE_STRIDE 0xc8
#define VE_SECONDARY_FB_LINE_STRIDE 0xcc
#define VE_CHROMA_BUF_LEN 0xe8
#define VE_PRIMARY_OUT_FMT 0xec
#define M 0x100
#define MPEG_CTRL (M + 0x14)
#define MPEG_TRIGGER (M + 0x18)
#define MPEG_STATUS (M + 0x1c)
#define MPEG_VLD_ADDR (M + 0x28)
#define MPEG_VLD_OFFSET (M + 0x2c)
#define MPEG_VLD_LEN (M + 0x30)
#define MPEG_VLD_END (M + 0x34)
#define MPEG_IQMINPUT (M + 0x80)
#define MPEG_JPEG_SIZE (M + 0xb8)
#define MPEG_JPEG_RES_INT (M + 0xc0)
#define MPEG_ERROR (M + 0xc4)
#define MPEG_ROT_LUMA (M + 0xcc)
#define MPEG_ROT_CHROMA (M + 0xd0)
#define MPEG_SD_ROT_DBLK_CTL (M + 0xd4)
#define MPEG_SRAM_RW_OFFSET (M + 0xe0)
#define MPEG_SRAM_RW_DATA (M + 0xe4)
#define TRIG_JPEG (3u << 24)
#define TRIG_HW_JPEG_VLD 0x0e
#define CTRL_FORCE_420 (1u << 18)

static volatile uint32_t *R;
static void w(unsigned off, uint32_t v) { R[off / 4] = v; }
static uint32_t r(unsigned off) { return R[off / 4]; }
static double ms(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec * 1e3 + t.tv_nsec / 1e6; }

/* ------------------------------------------------------------ JPEG basligi */
struct jpg {
	unsigned w, h, ncomp, hs[3], vs[3], qsel[3], dcsel[3], acsel[3], dri;
	const uint8_t *qt[4]; unsigned qlen[4];		/* 64 (8 bit) ya da 128 (16 bit) */
	const uint8_t *ht[2][4]; unsigned hlen[2][4];	/* [sinif dc=0/ac=1][id]: 16 sayi + semboller */
	size_t ecs;					/* entropi kodlu verinin dosyadaki ofseti */
};

static int jpeg_ayristir(const uint8_t *d, size_t n, struct jpg *j)
{
	memset(j, 0, sizeof *j);
	if (n < 4 || d[0] != 0xff || d[1] != 0xd8) return -1;
	size_t p = 2;
	int sof = 0;
	while (p + 4 <= n) {
		if (d[p] != 0xff) return -2;
		uint8_t mk = d[p + 1];
		if (mk == 0xff) { p++; continue; }
		unsigned len = d[p + 2] << 8 | d[p + 3];
		const uint8_t *s = d + p + 4, *e = d + p + 2 + len;
		if (p + 2 + len > n || len < 2) return -3;
		if (mk == 0xc0) {				/* SOF0 baseline */
			if (len < 17 || s[0] != 8) return -10;	/* v23: 3 bilesen = 17 bayt */
			j->h = s[1] << 8 | s[2]; j->w = s[3] << 8 | s[4]; j->ncomp = s[5];
			if (j->ncomp != 3) return -11;
			for (unsigned i = 0; i < 3; i++) {
				j->hs[i] = s[7 + 3 * i] >> 4; j->vs[i] = s[7 + 3 * i] & 15; j->qsel[i] = s[8 + 3 * i] & 3;
			}
			sof = 1;
		} else if (mk >= 0xc1 && mk <= 0xcf && mk != 0xc4 && mk != 0xc8 && mk != 0xcc) {
			return -12;				/* progressive vb. */
		} else if (mk == 0xdb) {			/* DQT (birden cok tablo olabilir) */
			while (s < e) {
				unsigned pq = s[0] >> 4, tq = s[0] & 3, l = pq ? 128 : 64;
				if (s + 1 + l > e) return -15;	/* v23: tablo bolum disina tasmasin */
				j->qt[tq] = s + 1; j->qlen[tq] = l; s += 1 + l;
			}
		} else if (mk == 0xc4) {			/* DHT */
			while (s < e) {
				unsigned tc = s[0] >> 4, th = s[0] & 3, cnt = 0;
				if (s + 17 > e) return -13;	/* v23 */
				for (int i = 0; i < 16; i++) cnt += s[1 + i];
				if (tc > 1 || cnt > 256 || s + 17 + cnt > e) return -13;
				j->ht[tc][th] = s + 1; j->hlen[tc][th] = 16 + cnt; s += 17 + cnt;
			}
		} else if (mk == 0xdd) {
			if (len < 4) return -17;	/* 614: 2 bayt tasma */
			j->dri = s[0] << 8 | s[1];
		} else if (mk == 0xda) {			/* SOS */
			if (!sof || len < 12 || s[0] != 3) return -14;
			for (unsigned i = 0; i < 3; i++) {
				j->dcsel[i] = s[2 + 2 * i] >> 4; j->acsel[i] = s[2 + 2 * i] & 15;
				if (j->dcsel[i] > 3 || j->acsel[i] > 3) return -16;	/* v23: ht[][4] */
			}
			j->ecs = (size_t)(e - d);
			return 0;
		}
		p += 2 + len;
	}
	return -4;
}

/* ------------------------------------------------------------ VE tablolari */
static void huff_baslik(const uint8_t *t)
{
	uint16_t sc[16]; uint8_t of[16]; unsigned cnt = 0, code = 0;
	for (int i = 0; i < 16; i++) { of[i] = (uint8_t)cnt; sc[i] = (uint16_t)code; cnt += t[i]; code += t[i]; code *= 2; }
	for (int i = 15; i >= 0 && !t[i]; i--) sc[i] = 0xffff;
	for (int i = 0; i < 8; i++) w(MPEG_SRAM_RW_DATA, (uint32_t)sc[2 * i] | (uint32_t)sc[2 * i + 1] << 16);
	for (int i = 0; i < 4; i++) w(MPEG_SRAM_RW_DATA, of[4 * i] | of[4 * i + 1] << 8 | of[4 * i + 2] << 16 | (uint32_t)of[4 * i + 3] << 24);
	for (int i = 0; i < 4; i++) w(MPEG_SRAM_RW_DATA, 0);
}

static int huff_tablolar(const struct jpg *j)
{
	const uint8_t *t[4]; unsigned l[4];
	t[0] = j->ht[0][j->dcsel[0]]; l[0] = j->hlen[0][j->dcsel[0]];
	t[1] = j->ht[1][j->acsel[0]]; l[1] = j->hlen[1][j->acsel[0]];
	t[2] = j->ht[0][j->dcsel[1]]; l[2] = j->hlen[0][j->dcsel[1]];
	t[3] = j->ht[1][j->acsel[1]]; l[3] = j->hlen[1][j->acsel[1]];
	for (int i = 0; i < 4; i++) if (!t[i]) return -1;
	w(MPEG_SRAM_RW_OFFSET, 0);
	for (int i = 0; i < 4; i++) huff_baslik(t[i]);
	for (int i = 0; i < 192; i++) w(MPEG_SRAM_RW_DATA, 0);
	for (int i = 0; i < 4; i++) {
		unsigned n = l[i] - 16, words = (n + 3) / 4;
		const uint8_t *s = t[i] + 16;
		for (unsigned k = 0; k < words; k++) {
			uint32_t v = 0;
			for (unsigned b = 0; b < 4; b++) if (4 * k + b < n) v |= (uint32_t)s[4 * k + b] << (8 * b);
			w(MPEG_SRAM_RW_DATA, v);
		}
		for (unsigned k = words; k < 64; k++) w(MPEG_SRAM_RW_DATA, 0);
	}
	return 0;
}

static void niceleme(const uint8_t *q, unsigned len, uint32_t bayrak)
{
	for (unsigned i = 0; i < 64; i++) {
		uint32_t v = len > 64 ? (uint32_t)(q[2 * i] << 8 | q[2 * i + 1]) : q[i];
		w(MPEG_IQMINPUT, ((i & 63) << 8) | (v & 0xff) | bayrak);
	}
}

/* ------------------------------------------------------------ C906 kullanici kipi onbellek yonetimi (v8) */
#define ONBELLEK_SATIR 64
static int ob_var;		/* 1: dcache.iva/civa kullanilabilir (UCME) */
static int dmabuf_sync;		/* 1: cikis ION_FLAG_CACHED + DMA_BUF_IOCTL_SYNC */
static sigjmp_buf ob_jb;
static void ob_sigill(int s) { (void)s; siglongjmp(ob_jb, 1); }
#if defined(__riscv)
static inline void dc_civa(const void *p)
{
	register const void *a0 __asm__("a0") = p;
	__asm__ volatile(".long 0x0275000b" : : "r"(a0) : "memory");	/* th.dcache.civa a0 */
}
static inline void dc_iva(const void *p)
{
	register const void *a0 __asm__("a0") = p;
	__asm__ volatile(".long 0x0265000b" : : "r"(a0) : "memory");	/* th.dcache.iva a0 */
}
static inline void th_sync(void) { __asm__ volatile(".long 0x0180000b" : : : "memory"); }	/* th.sync */
#else
static inline void dc_civa(const void *p) { (void)p; }
static inline void dc_iva(const void *p) { (void)p; }
static inline void th_sync(void) { }
#endif
static void ob_aralik(const void *p, size_t n, int temizle)
{
	uintptr_t a = (uintptr_t)p & ~(uintptr_t)(ONBELLEK_SATIR - 1), e = (uintptr_t)p + n;
	for (; a < e; a += ONBELLEK_SATIR)
		temizle ? dc_civa((const void *)a) : dc_iva((const void *)a);
	th_sync();
}
static void ob_dene(void)
{
	static volatile uint64_t deneme[16];
	struct sigaction yeni = { .sa_handler = ob_sigill }, eski;
	sigaction(SIGILL, &yeni, &eski);
	if (!sigsetjmp(ob_jb, 1)) {
		deneme[0] = 1;
		ob_aralik((const void *)deneme, sizeof deneme, 1);
		ob_aralik((const void *)deneme, sizeof deneme, 0);
		ob_var = 1;
	}
	sigaction(SIGILL, &eski, NULL);
	printf("C906 kullanici kipi onbellek yonetimi: %s\n", ob_var ? "VAR (onbellekli VE cikisi)" : "YOK (SIGILL; onbelleksiz + satir kopyasi)");
}

/* ------------------------------------------------------------ VE durumu (v7) */
#define VE_CIKIS_W 1600u
#define VE_CIKIS_H 608u
static int ve_fd = -1, ve_ion = -1, ve_ok, ve_hata, ve_bfd = -1, ve_ofd = -1, ve_req;
static size_t ve_bsz, ve_osz;
static uint8_t *ve_bm, *ve_om;
static uint32_t ve_badr, ve_oadr;
static unsigned ve_sayac, sw_sayac;
static uint32_t ve_maske;	/* v13: ara tampon icin ION yigini */
static int16_t t_rv[256], t_gu[256], t_gv[256], t_bu[256];
static uint8_t t_kirp[1024];
static uint16_t t_r5[1024], t_g6[1024], t_b5[1024];

/* ayrilan her tampon cikista UNMAP_DMA_BUF ile birakilmali: surucu kendisi birakmiyor (CMA sizintisi, 11.10 olculdu) */
static int ve_ion_ayir(size_t n, uint32_t mask, uint8_t **map, uint32_t *adr, int *fdp, uint32_t bayrak)
{
	struct ion_allocation_data a = { .len = n, .heap_id_mask = mask, .flags = bayrak };
	if (ioctl(ve_ion, ION_IOC_ALLOC, &a)) return -1;
	*fdp = (int)a.fd;
	*map = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, (int)a.fd, 0);
	if (*map == MAP_FAILED) { *map = NULL; return -2; }
	struct dma_buf_param p = { .fd = (int)a.fd };
	if (ioctl(ve_fd, VE_MAP_DMA_BUF, &p)) { munmap(*map, n); *map = NULL; return -3; }
	*adr = p.phy_addr;
	return 0;
}

static void ve_tampon_birak(void)
{
	struct dma_buf_param p;
	if (ve_bm) { p = (struct dma_buf_param){ .fd = ve_bfd }; ioctl(ve_fd, VE_UNMAP_DMA_BUF, &p); munmap(ve_bm, ve_bsz); ve_bm = NULL; }
	if (ve_om) { p = (struct dma_buf_param){ .fd = ve_ofd }; ioctl(ve_fd, VE_UNMAP_DMA_BUF, &p); munmap(ve_om, ve_osz); ve_om = NULL; }
	if (ve_bfd >= 0) { close(ve_bfd); ve_bfd = -1; }
	if (ve_ofd >= 0) { close(ve_ofd); ve_ofd = -1; }
	if (ve_req) { ioctl(ve_fd, VE_ENGINE_REL, 0); ve_req = 0; }
}

static void ve_baslat(void)
{
	for (int i = 0; i < 256; i++) {
		t_rv[i] = (int16_t)((1436 * (i - 128)) >> 10);		/* 1.402 */
		t_gu[i] = (int16_t)((352 * (i - 128)) >> 10);		/* 0.344 */
		t_gv[i] = (int16_t)((731 * (i - 128)) >> 10);		/* 0.714 */
		t_bu[i] = (int16_t)((1815 * (i - 128)) >> 10);		/* 1.772 */
	}
	for (int i = 0; i < 1024; i++) {
		int v = i - 384;
		t_kirp[i] = (uint8_t)(v < 0 ? 0 : v > 255 ? 255 : v);
		t_r5[i] = (uint16_t)((t_kirp[i] >> 3) << 11);
		t_g6[i] = (uint16_t)((t_kirp[i] >> 2) << 5);
		t_b5[i] = (uint16_t)(t_kirp[i] >> 3);
	}
	if (getenv("VE_KAPALI")) { puts("VE: ortam degiskeniyle kapali, yazilim cozme"); return; }
	ve_fd = open("/dev/cedar_dev", O_RDWR | O_CLOEXEC);
	ve_ion = open("/dev/ion", O_RDWR | O_CLOEXEC);
	if (ve_fd < 0 || ve_ion < 0) { puts("VE: /dev/cedar_dev ya da /dev/ion yok -> yazilim cozme"); return; }
	struct ion_heap_data hd[16];
	memset(hd, 0, sizeof hd);
	struct ion_heap_query q = { .cnt = 16, .heaps = (uintptr_t)hd };
	uint32_t mask = 0;
	if (!ioctl(ve_ion, ION_IOC_HEAP_QUERY, &q))
		for (unsigned i = 0; i < q.cnt && i < 16; i++)
			if (hd[i].type == 2) { mask = 1u << hd[i].heap_id; break; }
	/* ONCE ENGINE_REQ (reset kaldir + saatler), SONRA MAP_DMA_BUF: VE saati hic acilmamisken yapilan ilk IOMMU
	 * eslemesi cekirdekte kesilemez beklemeye girdi (11.10.2026, iki kez; guc sok-tak gerekti). */
	if (ioctl(ve_fd, VE_ENGINE_REQ, 0)) { puts("VE: ENGINE_REQ basarisiz"); return; }
	ve_req = 1;
	ve_bsz = MAX_JPEG;
	ve_osz = (VE_CIKIS_W * VE_CIKIS_H * 3 / 2 + 4095) & ~4095u;
	ob_dene();
	ve_maske = mask;
	if (!mask || ve_ion_ayir(ve_bsz, mask, &ve_bm, &ve_badr, &ve_bfd, 0) ||
	    ve_ion_ayir(ve_osz, mask, &ve_om, &ve_oadr, &ve_ofd, 1u /* ION_FLAG_CACHED */)) {
		puts("VE: ION ayirma/esleme basarisiz -> yazilim cozme");
		ve_tampon_birak();
		return;
	}
	R = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, ve_fd, 0);
	if (R == MAP_FAILED) { R = NULL; ve_tampon_birak(); return; }
	if (!ob_var) {
		struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
		dmabuf_sync = ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy) == 0;
		sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
		if (dmabuf_sync)
			ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy);
		printf("VE cikisi: %s\n", dmabuf_sync ? "onbellekli + DMA_BUF_IOCTL_SYNC" : "DMA-BUF senkron YOK (onbellekli tampon, satir kopyasi)");
	}
	ve_ok = 1;
	printf("VE: hazir, surum 0x%08x, bit akisi 0x%08x, cikis 0x%08x\n", r(0xf0), ve_badr, ve_oadr);
}

static void ve_bitir(void)
{
	if (R) { w(VE_MODE, 7); munmap((void *)R, 4096); R = NULL; }	/* v23: ve_ok JPEG hatalarinda 0 olabilir */
	ve_ok = 0;
	ve_tampon_birak();		/* UNMAP'ler, sonra ENGINE_REL */
}

/* ------------------------------------------------------------ v12 G2D dogrudan cizim */
#define DG_KONTROL "/tmp/pc-dogrudan"
static unsigned dg_ters;	/* v22: panel 180 derece (izin dosyasinin 3. alani; DG_ROT270 ortam degiskeni de) */
static void publish_rect(unsigned x, unsigned y, unsigned w, unsigned h);
static int dg_mod, dg_tfd = -1;
static uint8_t *dg_tm;
static uint32_t dg_tadr;
static int dg_g2d = -1, dg_fb = -1, dg_hazir, dg_sahip, dg_test, dg_kfd = -1, dg_son_g2d, dg_ciz_var, dg_shm_eski;
static uint32_t dg_fb_phys, dg_line, dg_gen, dg_kontrol_t;
static struct fb_var_screeninfo dg_var;
static uint8_t *dg_fbm;
static size_t dg_fblen;
static int dg_ornek;
static uint32_t dg_st[6], dg_dogrula, dg_bekle_us, dg_gec, dg_denetim;	/* g2d sayisi, g2d us, pan sayisi, pan us, cpu dikdortgen, esitleme */
static void dg_uyu(uint32_t us)
{
	struct timespec t = { (time_t)(us / 1000000u), (long)(us % 1000000u) * 1000L };
	nanosleep(&t, NULL);
}

static uint32_t dg_us(const struct timespec *a)
{
	struct timespec b;
	clock_gettime(CLOCK_MONOTONIC, &b);
	return (uint32_t)((b.tv_sec - a->tv_sec) * 1000000 + (b.tv_nsec - a->tv_nsec) / 1000);
}

/* v25: CMA anlik EBUSY (alloc_contig_range: PFNs busy) -> sikistir + yeniden dene. 0 tamam. */
static int dg_ara_ayir(void)
{
	for (int i = 1;; i++) {
		int r = ve_ion_ayir((size_t)PC_SHM_W * PC_SHM_H * 4u, ve_maske, &dg_tm, &dg_tadr, &dg_tfd, 0);
		if (r != -1 || i == 5)
			return r;
		printf("G2D ara tampon: ayrilamadi (%d. deneme), sikistirip yeniden\n", i);
		fflush(stdout);
		int f = open("/proc/sys/vm/compact_memory", O_WRONLY | O_CLOEXEC);
		if (f >= 0) {
			ssize_t w = write(f, "1", 1);
			(void)w;
			close(f);
		}
		dg_uyu(300000);
	}
}

static void dg_baslat(void)
{
	struct fb_fix_screeninfo f;
	if (!ve_ok || getenv("DG_KAPALI"))
		return;
	dg_g2d = open("/dev/g2d", O_RDWR | O_CLOEXEC);
	dg_fb = open("/dev/fb0", O_RDWR | O_CLOEXEC);
	int ok = dg_g2d >= 0 && dg_fb >= 0 && !ioctl(dg_fb, FBIOGET_FSCREENINFO, &f) &&
		 !ioctl(dg_fb, FBIOGET_VSCREENINFO, &dg_var) && dg_var.bits_per_pixel == 32 && f.smem_start;
	if (ok && dg_var.xres == PC_SHM_W && dg_var.yres == PC_SHM_H && dg_var.rotate == 1 && f.line_length == PC_SHM_W * 4u)
		dg_mod = 1;
	else if (ok && dg_var.xres == PC_SHM_H && dg_var.yres == PC_SHM_W && f.line_length == PC_SHM_H * 4u &&
		 !dg_ara_ayir())	/* v25 */
		dg_mod = 2;
	if (!dg_mod) {
		puts("G2D dogrudan: kapali (g2d/fb0 yok, desteklenmeyen fb ya da ara tampon ayrilamadi)");
		if (dg_g2d >= 0) close(dg_g2d);
		if (dg_fb >= 0) close(dg_fb);
		dg_g2d = dg_fb = -1;
		return;
	}
	dg_fb_phys = (uint32_t)f.smem_start;
	dg_line = f.line_length;
	dg_fblen = (size_t)dg_line * dg_var.yres;
	dg_fbm = mmap(NULL, dg_fblen, PROT_READ | PROT_WRITE, MAP_SHARED, dg_fb, 0);
	if (dg_fbm == MAP_FAILED) {
		dg_fbm = NULL;
		close(dg_g2d); close(dg_fb);
		dg_g2d = dg_fb = -1;
		return;
	}
	dg_hazir = 1;
	dg_dogrula = getenv("DG_DOGRULA") != NULL;
	dg_bekle_us = getenv("DG_BEKLE_US") ? (uint32_t)atoi(getenv("DG_BEKLE_US")) : 0;
	if (dg_bekle_us > 20000)
		dg_bekle_us = 20000;
	printf("G2D tani: dogrula=%u bekle=%u us\n", dg_dogrula, dg_bekle_us);
	printf("G2D dogrudan: hazir, mod %d (%s), fb 0x%08x satir %u, ara 0x%08x\n", dg_mod,
	       dg_mod == 1 ? "yatay fb + pan" : "dikey fb + G2D dondurme", dg_fb_phys, dg_line, dg_tadr);
}

static inline uint32_t dg_8888(uint32_t p)
{
	return 0xFF000000u | ((p & 0xF800u) << 8) | ((p & 0xE000u) << 3) | ((p & 0x07E0u) << 5) |
	       ((p & 0x0600u) >> 1) | ((p & 0x1Fu) << 3) | ((p & 0x1Cu) >> 2);
}

/* ortak bellek (RGB565) -> fb (ARGB8888), 64 bit yazma */
static int dg_dondur(unsigned x, unsigned y, unsigned w, unsigned h);
static void dg_rect_cpu(unsigned x, unsigned y, unsigned w, unsigned h)
{
	uint8_t *hedef = dg_mod == 2 ? dg_tm : dg_fbm;
	size_t satir_b = dg_mod == 2 ? PC_SHM_W * 4u : dg_line;
	for (unsigned j = 0; j < h; j++) {
		const uint16_t *s = frame + (size_t)(y + j) * PC_SHM_W + x;
		uint32_t *d = (uint32_t *)(hedef + (size_t)(y + j) * satir_b) + x;
		unsigned i = 0;
		if (((uintptr_t)d & 7u) && w) {
			d[0] = dg_8888(s[0]);
			i = 1;
		}
		uint64_t *d64 = (uint64_t *)(d + i);
		for (; i + 1 < w; i += 2)
			*d64++ = (uint64_t)dg_8888(s[i]) | (uint64_t)dg_8888(s[i + 1]) << 32;
		if (i < w)
			d[i] = dg_8888(s[i]);
	}
	dg_st[4]++;
	if (dg_mod == 2)
		dg_dondur(x, y, w, h);
}

/* fb (onbelleksiz) -> ortak bellek: satir once onbellekli tampona */
static void dg_fb_to_shm(void)
{
	static uint32_t satir[PC_SHM_W];
	for (unsigned y = 0; y < PC_SHM_H; y++) {
		memcpy(satir, dg_mod == 2 ? dg_tm + (size_t)y * PC_SHM_W * 4u : dg_fbm + (size_t)y * dg_line, PC_SHM_W * 4u);
		uint16_t *d = frame + (size_t)y * PC_SHM_W;
		for (unsigned x = 0; x < PC_SHM_W; x++) {
			uint32_t p = satir[x];
			d[x] = (uint16_t)(((p >> 8) & 0xF800u) | ((p >> 5) & 0x07E0u) | ((p >> 3) & 0x1Fu));
		}
	}
	dg_shm_eski = 0;
	dg_st[5]++;
}

static void dg_pan(void)
{
	if (dg_mod != 1)
		return;
	struct timespec a;
	clock_gettime(CLOCK_MONOTONIC, &a);
	if (ioctl(dg_fb, FBIOPAN_DISPLAY, &dg_var) == 0) {
		dg_st[2]++;
		dg_st[3] += dg_us(&a);
	}
}

static void dg_istat(void)
{
	if (!H)
		return;
	for (int i = 0; i < 6; i++)
		H->pad[2 + i] = dg_st[i];
	H->pad[8] = dg_gec;
	H->pad[9] = dg_denetim;
	H->pad[10] = 1;	/* v22: 180 derece dogrudan cizim destekli (panel 0.64) */
}

/* panelin izin dosyasini oku; gecislerde esitle ve onayla. Kareler arasinda ve beklerken (<=100 ms) cagrilir. */
static void dg_kontrol(void)
{
	if (!dg_hazir || dg_test || !H)
		return;
	uint32_t t = now();
	if (dg_kfd < 0) {
		if ((uint32_t)(t - dg_kontrol_t) < 1000)
			return;
		dg_kontrol_t = t;
		dg_kfd = open(DG_KONTROL, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
		if (dg_kfd < 0)
			return;
	}
	uint32_t k[3] = { 0, 0, 0 };
	ssize_t kn = pread(dg_kfd, k, sizeof k, 0);
	unsigned ters = dg_ters;
	if (kn != 8 && kn != (ssize_t)sizeof k) {
		close(dg_kfd);
		dg_kfd = -1;
		k[0] = dg_gen;
		k[1] = 0;
	} else {
		ters = (kn == (ssize_t)sizeof k && k[2] == 1) || getenv("DG_ROT270") != NULL;	/* v22; 8 bayt: eski panel */
	}
	unsigned yeni = k[1] == 1 && H->state == PC_SHM_CLIENT;
	if (k[0] == dg_gen && yeni == (unsigned)dg_sahip && H->pad[0] == dg_gen && ters == dg_ters && H->pad[10] == 1)
		return;
	if (ters != dg_ters) {	/* v22 */
		printf("%u yon: %s\n", t / 1000, ters ? "180 (ROT_270)" : "normal (ROT_90)");
		fflush(stdout);
		dg_ters = ters;
		if (yeni && dg_sahip) {
			dg_rect_cpu(0, 0, PC_SHM_W, PC_SHM_H);	/* sahipken yon degisti: tam kare yeni yonle */
			dg_pan();
		}
	}
	if (yeni != (unsigned)dg_sahip) {	/* v21 */
		printf("%u sahiplik %d->%u (izin %u nesil %u durum %u)\n", t / 1000, dg_sahip, yeni, k[1], k[0], (unsigned)H->state);
		fflush(stdout);
	}
	if (yeni && !dg_sahip) {
		dg_rect_cpu(0, 0, PC_SHM_W, PC_SHM_H);	/* panelin cizmedigi dikdortgen kalmasin */
		dg_pan();
		dg_sahip = 1;
	} else if (!yeni && dg_sahip) {
		if (dg_shm_eski)
			dg_fb_to_shm();
		dg_sahip = 0;
		H->pad[0] = k[0];
		H->pad[1] = 0;
		publish_rect(0, 0, PC_SHM_W, PC_SHM_H);
	}
	dg_gen = k[0];
	H->pad[0] = dg_gen;
	H->pad[1] = (uint32_t)dg_sahip;
	dg_istat();
}

static void dg_bitir(void)
{
	if (dg_tm) {
		struct dma_buf_param p = { .fd = dg_tfd };
		ioctl(ve_fd, VE_UNMAP_DMA_BUF, &p);
		munmap(dg_tm, (size_t)PC_SHM_W * PC_SHM_H * 4u);
		dg_tm = NULL;
	}
	if (dg_tfd >= 0) { close(dg_tfd); dg_tfd = -1; }
}

/* mod 2: ara tampondaki yatay (x,y,w,h) -> dikey fb (599-y-h+1.., x), G2D dondurme birimi; v22: ters (panel 180
 * derece) iken ROT_270 -> (y, 1599-x-w+1..) */
static int dg_dondur(unsigned x, unsigned y, unsigned w, unsigned h)
{
	g2d_blt_h b;
	memset(&b, 0, sizeof b);
	b.flag_h = dg_ters ? G2D_ROT_270 : G2D_ROT_90;	/* v22 */
	b.src_image_h.format = G2D_FORMAT_ARGB8888;
	b.src_image_h.laddr[0] = dg_tadr;
	b.src_image_h.width = PC_SHM_W;
	b.src_image_h.height = PC_SHM_H;
	b.src_image_h.clip_rect = (g2d_rect){ (int)x, (int)y, w, h };
	b.src_image_h.alpha = 0xff;
	b.src_image_h.use_phy_addr = 1;
	b.dst_image_h.format = G2D_FORMAT_ARGB8888;
	b.dst_image_h.laddr[0] = dg_fb_phys;
	b.dst_image_h.width = PC_SHM_H;
	b.dst_image_h.height = PC_SHM_W;
	b.dst_image_h.clip_rect = dg_ters ? (g2d_rect){ (int)y, (int)(PC_SHM_W - x - w), h, w }	/* v22: (x,y) -> (y, W-1-x) */
					  : (g2d_rect){ (int)(PC_SHM_H - y - h), (int)x, h, w };
	b.dst_image_h.alpha = 0xff;
	b.dst_image_h.use_phy_addr = 1;
	struct timespec a;
	clock_gettime(CLOCK_MONOTONIC, &a);
	if (ioctl(dg_g2d, G2D_CMD_BITBLT_H, &b)) {
		static int uyari;
		if (uyari++ < 3)
			perror("G2D dondurme");
		return -1;
	}
	dg_st[2]++;	/* mod 2'de 'pan' sayaci = dondurme */
	dg_st[3] += dg_us(&a);
	return 0;
}

/* VE ciktisi (NV12, satir AW, yukseklik AH) sw x sh -> fb (x, y, w, h); 0 tamam */
static int dg_ciz_g2d_k(uint32_t ky, uint32_t kc, unsigned x, unsigned y, unsigned w, unsigned h, unsigned sw, unsigned sh,
			unsigned AW, unsigned AH)
{
	g2d_blt_h b;
	memset(&b, 0, sizeof b);
	b.flag_h = G2D_BLT_NONE_H;
	b.src_image_h.format = getenv("DG_NV12") ? G2D_FORMAT_YUV420UVC_U1V1U0V0 : G2D_FORMAT_YUV420UVC_V1U1V0U0;
	b.src_image_h.laddr[0] = ky;
	b.src_image_h.laddr[1] = kc;
	b.src_image_h.width = AW;
	b.src_image_h.height = AH;
	b.src_image_h.clip_rect = (g2d_rect){ 0, 0, sw, sh };
	b.src_image_h.alpha = 0xff;
	b.src_image_h.mode = G2D_PIXEL_ALPHA;
	b.src_image_h.gamut = G2D_BT601;
	b.src_image_h.color_range = COLOR_RANGE_0_255;
	b.src_image_h.use_phy_addr = 1;
	b.dst_image_h.format = getenv("DG_XRGB") ? G2D_FORMAT_XRGB8888 : G2D_FORMAT_ARGB8888;
	b.dst_image_h.laddr[0] = dg_mod == 2 ? dg_tadr : dg_fb_phys;
	b.dst_image_h.width = PC_SHM_W;
	b.dst_image_h.height = PC_SHM_H;
	b.dst_image_h.clip_rect = (g2d_rect){ (int)x, (int)y, w, h };
	b.dst_image_h.alpha = 0xff;
	b.dst_image_h.mode = G2D_PIXEL_ALPHA;
	b.dst_image_h.gamut = G2D_BT601;
	b.dst_image_h.color_range = COLOR_RANGE_0_255;
	b.dst_image_h.use_phy_addr = 1;
	struct timespec a;
	clock_gettime(CLOCK_MONOTONIC, &a);
	int r = ioctl(dg_g2d, G2D_CMD_BITBLT_H, &b);
	if (r) {
		static int uyari;
		if (uyari++ < 3)
			perror("G2D_CMD_BITBLT_H");
		return -1;
	}
	dg_st[0]++;
	dg_st[1] += dg_us(&a);
	dg_shm_eski = 1;
	if (dg_mod == 2 && dg_dondur(x, y, w, h))
		return -1;
	return 0;
}

static int dg_ciz_g2d(unsigned x, unsigned y, unsigned w, unsigned h, unsigned sw, unsigned sh, unsigned AW, unsigned AH)
{
	return dg_ciz_g2d_k(ve_oadr, ve_oadr + AW * AH, x, y, w, h, sw, sh, AW, AH);
}

/* 0: cozuldu; >0: kesin hata (boyut uyusmazligi); -1: VE uygun degil/hata -> yazilima don */
static int ve_coz(size_t n, unsigned x, unsigned y, unsigned w_, unsigned h_, unsigned sc)
{
	struct jpg j;
	if (!ve_ok || n > MAX_JPEG || jpeg_ayristir(jpeg, n, &j))
		return -1;
	if (j.w << sc != w_ || j.h << sc != h_)
		return 6;
	unsigned idx = j.hs[0] << 20 | j.vs[0] << 16 | j.hs[1] << 12 | j.vs[1] << 8 | j.hs[2] << 4 | j.vs[2];
	/* v11 true444: small text refresh must not lose chroma via FORCE_420.
	 * Existing software decoder writes RGB565 directly; no new VE register mode. */
	if (idx == 0x111111)
		return -1;
	uint32_t ss;
	switch (idx) {
	case 0x221111: ss = 0u << 27; break;
	case 0x211111: ss = 2u << 27; break;
	case 0x111111: ss = 3u << 27; break;
	case 0x121111: ss = 4u << 27; break;
	default: return -1;
	}
	if (!j.qt[j.qsel[0]] || !j.qt[j.qsel[1]])
		return -1;
	unsigned AW = (j.w + 15) & ~15u, AH = (j.h + 15) & ~15u;
	if (AW > VE_CIKIS_W || AH > VE_CIKIS_H)
		return -1;
	size_t ysz = (size_t)AW * AH, csz = ysz / 2;
	memcpy(ve_bm, jpeg, n);
	w(VE_MODE, 0x00130000);
	w(VE_PRIMARY_OUT_FMT, (4u << 4) | 4u);
	w(VE_PRIMARY_CHROMA_BUF_LEN, (uint32_t)(csz / 2));
	w(VE_CHROMA_BUF_LEN, ((uint32_t)(csz / 2) & 0x0fffffff) | (1u << 29) | (1u << 30));
	w(VE_PRIMARY_FB_LINE_STRIDE, AW | (AW / 2) << 16);
	w(VE_SECONDARY_FB_LINE_STRIDE, AW | (AW / 2) << 16);
	w(MPEG_TRIGGER, TRIG_JPEG | ss);
	w(MPEG_JPEG_RES_INT, j.dri);
	w(MPEG_JPEG_SIZE, (((j.w + 8 * j.hs[0] - 1) / (8 * j.hs[0]) - 1) & 0x7ff) |
			  ((((j.h + 8 * j.vs[0] - 1) / (8 * j.vs[0]) - 1) & 0x7ff) << 16));
	niceleme(j.qt[j.qsel[0]], j.qlen[j.qsel[0]], 1u << 14);
	niceleme(j.qt[j.qsel[1]], j.qlen[j.qsel[1]], 0);
	if (huff_tablolar(&j)) { w(VE_MODE, 7); return -1; }
	w(MPEG_ROT_LUMA, ve_oadr);
	w(MPEG_ROT_CHROMA, ve_oadr + (uint32_t)ysz);
	w(MPEG_SD_ROT_DBLK_CTL, 0);
	w(MPEG_VLD_OFFSET, (uint32_t)(8 * j.ecs));
	w(MPEG_VLD_LEN, (uint32_t)(n * 8));
	w(MPEG_VLD_ADDR, (ve_badr & 0x0ffffff0u) | (ve_badr >> 28) | (1u << 28) | (1u << 29) | (1u << 30));
	w(MPEG_VLD_END, ve_badr + (uint32_t)n);
	if (ob_var)
		ob_aralik(ve_om, ysz + csz, 1);		/* kirli satir VE verisinin ustune yazilmasin */
	w(MPEG_STATUS, 0x7);
	w(MPEG_CTRL, ss ? CTRL_FORCE_420 : 0);		/* kesme YOK */
	double t1 = ms();
	w(MPEG_TRIGGER, TRIG_HW_JPEG_VLD | TRIG_JPEG | ss);
	uint32_t st = 0;
	while (ms() - t1 < 300 && !((st = r(MPEG_STATUS)) & 0x7))
		;
	w(MPEG_STATUS, 0x7);
	w(VE_MODE, 7);
	if (!(st & 1)) {
		if (!(st & 0x7))
			ioctl(ve_fd, VE_RESET, 0);
		/* v23: yalniz JPEG VE yolu kapanir; yazmac/tamponlari H.264 ve G2D de kullanir, cikista ve_bitir birakir */
		if (++ve_hata >= 5) { puts("VE: 5 hata, JPEG yazilima geciliyor"); ve_ok = 0; }
		printf("VE hata: durum 0x%08x\n", st);
		return -1;
	}
	if (dg_sahip && dg_dogrula && dmabuf_sync) {
		/* son luma satiri + son chroma satiri toplami: simdi ve 2 ms sonra */
		uint32_t t[2];
		for (int k = 0; k < 2; k++) {
			struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
			ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy);
			const uint8_t *ly = ve_om + (size_t)(j.h - 1) * AW, *lc = ve_om + ysz + (size_t)((j.h - 1) / 2) * AW;
			uint32_t s = 0;
			for (unsigned q = 0; q < j.w; q++)
				s = s * 31u + ly[q] + ((uint32_t)lc[q] << 8);
			t[k] = s;
			sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
			ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy);
			if (!k)
				dg_uyu(2000);
		}
		dg_denetim++;
		if (t[0] != t[1] && dg_gec++ < 20)
			printf("G2D tani: VE cikisi durum bitinden SONRA degisti (%ux%u @%u,%u)\n", j.w, j.h, x, y);
	}
	if (dg_sahip && dg_bekle_us)
		dg_uyu(dg_bekle_us);
	if (dg_sahip && !dg_ciz_g2d(x, y, w_, h_, j.w, j.h, AW, AH)) {
		dg_son_g2d = 1;
		ve_sayac++;
		return 0;
	}
	/* NV12 -> RGB565 (olcek2: 2x) */
	const uint8_t *Y = ve_om, *UV = ve_om + ysz;
	static uint8_t satir_y[VE_CIKIS_W], satir_uv[VE_CIKIS_W];
	struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
	int dogrudan = ob_var || dmabuf_sync;
	if (ob_var)
		ob_aralik(ve_om, ysz + csz, 0);
	else if (dmabuf_sync && ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy))
		dogrudan = 0;
	unsigned yy = 0;
	if (dogrudan && !sc && !(x & 1)) {	/* hizli yol: iki satir, 2x2 blok */
		unsigned w2 = j.w & ~1u;
		for (; yy + 1 < j.h; yy += 2) {
			const uint8_t *y0 = Y + (size_t)yy * AW, *y1 = y0 + AW, *uvp = UV + (size_t)(yy / 2) * AW;
			uint32_t *d0 = (uint32_t *)(frame + (size_t)(y + yy) * PC_SHM_W + x), *d1 = (uint32_t *)((uint16_t *)d0 + PC_SHM_W);
			for (unsigned xx = 0; xx < w2; xx += 2) {
				int u = uvp[xx], v = uvp[xx + 1];
				int rk = 384 + t_rv[v], gk = 384 - t_gu[u] - t_gv[v], bk = 384 + t_bu[u];
				int a = y0[xx], b = y0[xx + 1], c = y1[xx], d = y1[xx + 1];
				d0[xx >> 1] = (uint32_t)(t_r5[a + rk] | t_g6[a + gk] | t_b5[a + bk]) |
					      (uint32_t)(t_r5[b + rk] | t_g6[b + gk] | t_b5[b + bk]) << 16;
				d1[xx >> 1] = (uint32_t)(t_r5[c + rk] | t_g6[c + gk] | t_b5[c + bk]) |
					      (uint32_t)(t_r5[d + rk] | t_g6[d + gk] | t_b5[d + bk]) << 16;
			}
			if (j.w & 1) {		/* tek sayili genislikte son sutun */
				unsigned xx = j.w - 1;
				int u = uvp[xx & ~1u], v = uvp[(xx & ~1u) + 1];
				for (unsigned k = 0; k < 2; k++) {
					int yv = (k ? y1 : y0)[xx] + 384;
					frame[(size_t)(y + yy + k) * PC_SHM_W + x + xx] =
						(uint16_t)(t_r5[yv + t_rv[v]] | t_g6[yv - t_gu[u] - t_gv[v]] | t_b5[yv + t_bu[u]]);
				}
			}
		}
	}
	for (; yy < j.h; yy++) {
		const uint8_t *yp = Y + (size_t)yy * AW, *uvp = UV + (size_t)(yy / 2) * AW;
		if (!dogrudan) {		/* onbelleksiz: satiri 8 baytlik okumalarla onbellekli tampona al */
			memcpy(satir_y, yp, j.w);
			if (!(yy & 1) || yy == 0)
				memcpy(satir_uv, uvp, (j.w + 1) & ~1u);
			yp = satir_y;
			uvp = satir_uv;
		}
		uint16_t *d0 = frame + (size_t)(y + (yy << sc)) * PC_SHM_W + x, *d1 = d0 + PC_SHM_W;
		for (unsigned xx = 0; xx < j.w; xx++) {
			int yv = yp[xx] + 384, u = uvp[xx & ~1u], v = uvp[(xx & ~1u) + 1];
			unsigned R5 = t_kirp[yv + t_rv[v]] >> 3, G6 = t_kirp[yv - t_gu[u] - t_gv[v]] >> 2, B5 = t_kirp[yv + t_bu[u]] >> 3;
			uint16_t px = (uint16_t)(R5 << 11 | G6 << 5 | B5);
			if (!sc)
				d0[xx] = px;
			else
				d0[2 * xx] = d0[2 * xx + 1] = d1[2 * xx] = d1[2 * xx + 1] = px;
		}
	}
	if (dmabuf_sync && !ob_var) {
		sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
		ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy);
	}
	ve_sayac++;
	return 0;
}

/* hedef (x,y,w,h) ekran icinde dogrulanmis olmali; JPEG boyutu w,h (olcek2: w/2,h/2) ile birebir olmali */
/* ------------------------------------------------------------ v18 H.264 (VE donanim) */
static int h264_hazir = -1;	/* -1 denenmedi, 0 yok, 1 hazir */
static unsigned h264_sayac, h264_hata;
static double h264_ms;

static int h264_baslat(void)
{
	if (h264_hazir >= 0)
		return h264_hazir;
	h264_hazir = 0;
	if (!ve_ok)
		return 0;
	struct ion_heap_data hd[16];
	memset(hd, 0, sizeof hd);
	struct ion_heap_query q = { .cnt = 16, .heaps = (uintptr_t)hd };
	uint32_t m = 0;
	if (!ioctl(ve_ion, ION_IOC_HEAP_QUERY, &q))
		for (unsigned i = 0; i < q.cnt && i < 16; i++)
			if (hd[i].type == 0)
				m = 1u << hd[i].heap_id;	/* sistem yigini: ayrilmis yigindaki JPEG/G2D tamponlarina dokunmaz */
	struct hv_dis dis = { ve_bm, ve_badr, ve_bsz, ve_om, ve_oadr, ve_osz };	/* v19: JPEG VE tamponlari paylasilir */
	if (!m || hv_baslat(ve_fd, ve_ion, m, R, &dis)) {
		puts("H.264 VE: hazirlanamadi");
		hv_bitir();
		return 0;
	}
	h264_hazir = 1;
	puts("H.264 VE: hazir");
	return 1;
}

/* yedek yol: NV12 (onbelleksiz) -> RGB565 ortak bellek, BT601 0-255 (JPEG ile ayni tablolar) */
static void h264_cpu(const uint8_t *Y, const uint8_t *C, unsigned AW)
{
	static uint8_t ys[PC_SHM_W], cs[PC_SHM_W];
	for (unsigned y = 0; y < PC_SHM_H; y++) {
		memcpy(ys, Y + (size_t)y * AW, PC_SHM_W);
		if (!(y & 1))
			memcpy(cs, C + (size_t)(y / 2) * AW, PC_SHM_W);
		uint16_t *d = frame + (size_t)y * PC_SHM_W;
		for (unsigned x = 0; x < PC_SHM_W; x++) {
			int yy = ys[x], u = cs[x & ~1u], v = cs[x | 1u];
			d[x] = (uint16_t)(t_r5[yy + t_rv[v] + 384] | t_g6[yy - t_gu[u] - t_gv[v] + 384] | t_b5[yy + t_bu[u] + 384]);
		}
	}
}

/* jpeg[] tamponundaki H.264 erisim birimi. 0 tamam, >0 hata kodu */
static int h264_coz(size_t n)
{
	if (h264_baslat() != 1)
		return 30;
	struct hv_kare k;
	int r = hv_au(jpeg, n, &k);
	if (r == -2)
		ioctl(ve_fd, VE_RESET, 0);
	if (r < 0) {
		if (++h264_hata <= 5)
			printf("H.264 VE hata %d\n", r);
		return 31;
	}
	if (r == 1)
		return 0;	/* yalniz SPS/PPS/SEI */
	h264_sayac++;
	h264_ms += k.ms;
	if (k.w != PC_SHM_W || k.h != PC_SHM_H)
		return 32;
	if (dg_sahip) {
		if (dg_ciz_g2d_k(k.y, k.c, 0, 0, PC_SHM_W, PC_SHM_H, PC_SHM_W, PC_SHM_H, (unsigned)k.aw, (unsigned)k.ah))
			return 33;
		dg_son_g2d = 1;
		return 0;
	}
	if (k.konum == 0 && dmabuf_sync) {	/* v19: konum 0 = onbellekli ve_om */
		struct dma_buf_sync sy = { DMA_BUF_SYNC_START | DMA_BUF_SYNC_READ };
		ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy);
		h264_cpu(k.ym, k.ym + (size_t)k.aw * k.ah, (unsigned)k.aw);
		sy.flags = DMA_BUF_SYNC_END | DMA_BUF_SYNC_READ;
		ioctl(ve_ofd, DMA_BUF_IOCTL_SYNC, &sy);
		return 0;
	}
	h264_cpu(k.ym, k.ym + (size_t)k.aw * k.ah, (unsigned)k.aw);
	return 0;
}

static int decode(size_t n, unsigned x, unsigned y, unsigned w, unsigned h, unsigned sc)
{
	int vr = ve_coz(n, x, y, w, h, sc);
	if (vr >= 0)
		return vr;
	sw_sayac++;
	struct src s = { .size = n, .ox = x, .oy = y, .sc = sc };
	JDEC jd;
	int r = jd_prepare(&jd, in_cb, pool, sizeof pool, &s);
	if (r)
		return 10 + r;
	if ((unsigned)jd.width << sc != w || (unsigned)jd.height << sc != h)
		return 6;
	r = jd_decomp(&jd, out_cb, 0);
	return r ? 20 + r : 0;
}

#define DOKUNMA_FIFO "/tmp/pc-dokunma"
#define MAGIC_DOKUNMA 0x56454434u
static int dok_fd = -1, ist_fd = -1;
static uint32_t dok_sayac;

/* FIFO'daki dokunma kayitlarini istemciye ilet (yalniz alim beklerken cagrilir) */
static void dokunma_ilet(void)
{
	uint8_t b[8 * 32];
	ssize_t n = read(dok_fd, b, sizeof b);
	for (ssize_t i = 0; i + 8 <= n; i += 8) {
		unsigned tip = b[i], x = b[i + 2] | b[i + 3] << 8, y = b[i + 4] | b[i + 5] << 8;
		if (tip < 1 || tip > 9 || x >= PC_SHM_W || y >= PC_SHM_H || ist_fd < 0)	/* v17: 4..9 iki parmak */
			continue;
		uint32_t m[4] = { htonl(MAGIC_DOKUNMA), htonl(tip), htonl(x), htonl(y) };
		if (send(ist_fd, m, sizeof m, MSG_NOSIGNAL) == (ssize_t)sizeof m)
			dok_sayac++;
	}
}

static int waitfd(int fd, short ev, uint32_t until)
{
	while (!timeup() && (int32_t)(now() - until) < 0) {
		dg_kontrol();
		struct pollfd p[2] = { { .fd = fd, .events = ev }, { .fd = dok_fd, .events = POLLIN } };
		int np = (ev == POLLIN && fd == ist_fd && dok_fd >= 0) ? 2 : 1;
		int left = (int)(until - now());
		int r = poll(p, np, left > 100 ? 100 : (left < 1 ? 1 : left));
		if (r > 0 && np == 2 && (p[1].revents & POLLIN))
			dokunma_ilet();
		if (r > 0 && p[0].revents)
			return !!(p[0].revents & ev);
		if (r < 0 && errno != EINTR)
			return 0;
	}
	return 0;
}

static int recv_all(int fd, void *buf, size_t size, uint32_t ms)
{
	size_t off = 0;
	uint32_t until = now() + ms;
	while (off < size && waitfd(fd, POLLIN, until)) {
		ssize_t n = recv(fd, (char *)buf + off, size - off, MSG_DONTWAIT);
		if (n > 0)
			off += (size_t)n;
		else if (n == 0 || (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK))
			break;
	}
	return off == size;
}

static int send_ack(int fd, uint32_t seq, uint32_t st, uint32_t us, uint32_t nr, uint32_t bytes)
{
	uint32_t w[6] = { MAGIC_ACK, seq, st, us, nr, bytes };
	for (int i = 0; i < 6; i++)
		w[i] = htonl(w[i]);
	size_t off = 0;
	uint32_t until = now() + 3000;
	while (off < sizeof w && waitfd(fd, POLLOUT, until)) {
		ssize_t n = send(fd, (char *)w + off, sizeof w - off, MSG_NOSIGNAL | MSG_DONTWAIT);
		if (n > 0)
			off += (size_t)n;
		else if (n == 0 || (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK))
			break;
	}
	return off == sizeof w;
}

static void publish_rect(unsigned x, unsigned y, unsigned w, unsigned h)
{
	uint32_t c = H->rect_count;
	volatile uint16_t *r = H->rects[c % PC_SHM_RING];
	r[0] = (uint16_t)x; r[1] = (uint16_t)y; r[2] = (uint16_t)w; r[3] = (uint16_t)h;
	__atomic_store_n(&H->rect_count, c + 1, __ATOMIC_RELEASE);
}

/* bekleyen kopyalar: once tum kaynaklar tampona, sonra hedeflere (ust uste binme guvenli). 0 = tamam */
/* v16: sahipken kopya: kaynaklar once tampona (32 bit), sonra hedefler; ortak bellek eski kalir. 0 tamam, 1 sigmadi */
static int dg_kopya(void)
{
	uint8_t *b = dg_mod == 2 ? dg_tm : dg_fbm;
	size_t sat = dg_mod == 2 ? PC_SHM_W * 4u : dg_line, cap = sizeof kopya_tampon / 4u, off = 0;
	uint32_t *t = (uint32_t *)kopya_tampon;
	for (unsigned i = 0; i < kn; i++)
		if ((off += (size_t)kq[i].w * kq[i].h) > cap)
			return 1;
	off = 0;
	for (unsigned i = 0; i < kn; i++)
		for (unsigned r = 0; r < kq[i].h; r++, off += kq[i].w)
			memcpy(t + off, b + (size_t)(kq[i].sy + r) * sat + (size_t)kq[i].sx * 4u, kq[i].w * 4u);
	off = 0;
	for (unsigned i = 0; i < kn; i++) {
		for (unsigned r = 0; r < kq[i].h; r++, off += kq[i].w)
			memcpy(b + (size_t)(kq[i].y + r) * sat + (size_t)kq[i].x * 4u, t + off, kq[i].w * 4u);
		if (dg_mod == 2)
			dg_dondur(kq[i].x, kq[i].y, kq[i].w, kq[i].h);
		publish_rect(kq[i].x, kq[i].y, kq[i].w, kq[i].h);
	}
	kn = 0;
	dg_shm_eski = 1;
	dg_ciz_var = 1;
	return 0;
}

static int kopya_uygula(void)
{
	size_t off = 0;
	if (dg_sahip && !dg_kopya())
		return 0;
	if (dg_sahip && dg_shm_eski)
		dg_fb_to_shm();
	for (unsigned i = 0; i < kn; i++) {
		if (off + (size_t)kq[i].w * kq[i].h > PC_SHM_W * PC_SHM_H) {
			kn = 0;
			return 7;
		}
		for (unsigned r = 0; r < kq[i].h; r++, off += kq[i].w)
			memcpy(kopya_tampon + off, frame + (size_t)(kq[i].sy + r) * PC_SHM_W + kq[i].sx, kq[i].w * 2);
	}
	off = 0;
	for (unsigned i = 0; i < kn; i++) {
		for (unsigned r = 0; r < kq[i].h; r++, off += kq[i].w)
			memcpy(frame + (size_t)(kq[i].y + r) * PC_SHM_W + kq[i].x, kopya_tampon + off, kq[i].w * 2);
		publish_rect(kq[i].x, kq[i].y, kq[i].w, kq[i].h);
		if (dg_sahip) {
			dg_rect_cpu(kq[i].x, kq[i].y, kq[i].w, kq[i].h);
			dg_ciz_var = 1;
		}
	}
	kn = 0;
	return 0;
}

static char shm_path[256] = PC_SHM_PATH;

static int shm_create(void)
{
	char part[300];
	snprintf(part, sizeof part, "%s.part", shm_path);
	unlink(part);
	int fd = open(part, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0644);
	if (fd < 0 || ftruncate(fd, PC_SHM_SIZE)) {
		perror("shm");
		if (fd >= 0) { close(fd); unlink(part); }
		return 0;
	}
	shm_map = mmap(NULL, PC_SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if (shm_map == MAP_FAILED) {
		unlink(part);
		return 0;
	}
	H = (volatile pc_shm_hdr_t *)shm_map;
	frame = (uint16_t *)(shm_map + PC_SHM_HDR);
	H->version = 1; H->w = PC_SHM_W; H->h = PC_SHM_H; H->stride = PC_SHM_W * 2; H->offset = PC_SHM_HDR;
	H->gen = (uint32_t)getpid() ^ now();
	H->state = PC_SHM_LISTEN;
	H->alive_ms = now();
	__atomic_store_n(&H->magic, PC_SHM_MAGIC, __ATOMIC_RELEASE);
	if (rename(part, shm_path)) {
		unlink(part);
		return 0;
	}
	return 1;
}

static void shm_destroy(void)
{
	if (!H)
		return;
	H->state = PC_SHM_CLOSED;
	unlink(shm_path);
	munmap(shm_map, PC_SHM_SIZE);
	H = NULL;
}

/* bir istemciyle konus; donus: islenen kare sayisi */
static long majflt(void)	/* v20 */
{
	struct rusage ru;
	return getrusage(RUSAGE_SELF, &ru) ? 0 : ru.ru_majflt;
}

static unsigned tk_kare, tk_takilma;	/* v20: dakikalik ozet */
static long tk_mf;
static uint32_t tk_t;

static void serve(int c, unsigned *frames, unsigned *errors, uint64_t *bytes_all, uint64_t *dec_all)
{
	uint32_t sec_t = now(), sec_frames = 0, sec_bytes = 0;
	tk_t = now();
	tk_mf = majflt();
	int one = 1;
	setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
	ist_fd = c;
	H->conn++;
	H->state = PC_SHM_CLIENT;
	H->alive_ms = now();
	while (!timeup()) {
		uint32_t fh[4];
		if (!recv_all(c, fh, sizeof fh, 5000))
			break;	/* 5 sn canlilik karesi gelmedi ya da baglanti kapandi */
		uint32_t magic = ntohl(fh[0]), seq = ntohl(fh[1]), nr = ntohl(fh[2]), fl = ntohl(fh[3]);
		uint32_t st = 0, us = 0, fbytes = 0;
		double t_kare = ms();	/* v20 */
		long mf0 = majflt();
		if (magic != MAGIC_FRAME)
			st = 1;
		else if (nr > MAX_RECT)
			st = 2;
		for (uint32_t i = 0; !st && i < nr; i++) {
			uint16_t rh[8];
			if (!recv_all(c, rh, sizeof rh, 3000)) { st = 5; break; }
			unsigned x = ntohs(rh[0]), y = ntohs(rh[1]), w = ntohs(rh[2]), h = ntohs(rh[3]);
			unsigned rf = ntohs(rh[4]);
			uint32_t len;
			memcpy(&len, &rh[6], 4);	/* hizasiz okuma yok (RISC-V) */
			len = ntohl(len);
			unsigned sc = rf & 1, kop = (rf >> 1) & 1, h264 = (rf >> 2) & 1;	/* v18 */
			if (rf & ~7u || (sc && kop) || (h264 && (sc || kop || x || y || w != PC_SHM_W || h != PC_SHM_H)) || !w || !h || x + w > PC_SHM_W || y + h > PC_SHM_H || (sc && ((w | h) & 1))) { st = 3; break; }
			if (kop) {
				uint16_t src[2];
				if (len != 4) { st = 4; break; }
				if (!recv_all(c, src, 4, 3000)) { st = 5; break; }
				unsigned sx = ntohs(src[0]), sy = ntohs(src[1]);
				if (sx + w > PC_SHM_W || sy + h > PC_SHM_H) { st = 3; break; }
				kq[kn++] = (struct kopya){ sx, sy, x, y, w, h };
				fbytes += len;
				continue;
			}
			if (kn && (st = (uint32_t)kopya_uygula()))
				break;
			if (!len || len > MAX_JPEG) { st = 4; break; }
			if (!recv_all(c, jpeg, len, 3000)) { st = 5; break; }
			struct timespec a, b;
			clock_gettime(CLOCK_MONOTONIC, &a);
			dg_son_g2d = 0;
			st = h264 ? (uint32_t)h264_coz(len) : (uint32_t)decode(len, x, y, w, h, sc);
			if (!st && dg_sahip) {
				if (!dg_son_g2d)
					dg_rect_cpu(x, y, w, h);	/* yazilim/4:4:4: ortak bellekten fb'ye */
				dg_ciz_var = 1;
			}
			clock_gettime(CLOCK_MONOTONIC, &b);
			us += (uint32_t)((b.tv_sec - a.tv_sec) * 1000000 + (b.tv_nsec - a.tv_nsec) / 1000);
			fbytes += len;
			if (!st)
				publish_rect(x, y, w, h);
		}
		if (!st && kn)
			st = (uint32_t)kopya_uygula();
		kn = 0;
		if (dg_ciz_var) {
			if (dg_sahip)
				dg_pan();
			dg_ciz_var = 0;
			dg_istat();
		}
		dg_kontrol();
		H->alive_ms = now();
		H->want_full = fl & 1;
		if (!st) {
			(*frames)++;
			H->frames++;
			H->last_decode_us = us;
			H->last_bytes = fbytes;
			*bytes_all += fbytes;
			*dec_all += us;
			if (nr) sec_frames++;
			sec_bytes += fbytes;
		} else {
			(*errors)++;
			H->errors++;
		}
		if ((uint32_t)(now() - sec_t) >= 1000) {
			uint32_t dt = now() - sec_t;
			H->fps_x10 = sec_frames * 10000 / dt;
			H->kbps = (uint32_t)((uint64_t)sec_bytes * 8 / dt);
			sec_t = now(); sec_frames = sec_bytes = 0;
		}
		if (nr) {	/* v20: takilma gunlugu */
			double isl = ms() - t_kare;
			long mf = majflt() - mf0;
			tk_kare++;
			if (isl > 120.0) {
				tk_takilma++;
				printf("%u TAKILMA seq %u isleme %.0f ms cozme %u us bayt %u majflt +%ld sahip %d\n", now() / 1000, seq, isl, us, fbytes, mf, dg_sahip);
				fflush(stdout);
			}
		}
		if ((uint32_t)(now() - tk_t) >= 60000) {
			long mf = majflt();
			printf("%u dakika: kare %u takilma %u majflt +%ld\n", now() / 1000, tk_kare, tk_takilma, mf - tk_mf);
			fflush(stdout);
			tk_t = now();
			tk_mf = mf;
			tk_kare = tk_takilma = 0;
		}
		if (!send_ack(c, seq, st, us, nr, fbytes) || st)
			break;
	}
	ist_fd = -1;
	H->state = PC_SHM_LISTEN;
	H->fps_x10 = 0;
}

static int file_mode1(const char *yol, unsigned x, unsigned y, unsigned sc);
static int file_mode(int argc, char **argv)
{
	unsigned x = 0, y = 0, sc = 0;
	const char *ikinci = NULL;
	for (int i = 3; i < argc; i++) {
		if (!strcmp(argv[i], "--x") && i + 1 < argc) x = (unsigned)atoi(argv[++i]);
		else if (!strcmp(argv[i], "--y") && i + 1 < argc) y = (unsigned)atoi(argv[++i]);
		else if (!strcmp(argv[i], "--olcek2")) sc = 1;
		else if (!strcmp(argv[i], "--ikinci") && i + 1 < argc) ikinci = argv[++i];
		else if (!strcmp(argv[i], "--g2d")) dg_test = 1;
		else if (!strcmp(argv[i], "--ornek")) dg_ornek = 1;
		else return 2;
	}
	if (dg_test) {
		dg_baslat();
		if (!dg_hazir)
			return 4;
		dg_sahip = 1;
	}
	int r1 = file_mode1(argv[2], x, y, sc);
	if (dg_test) {
		if (!dg_son_g2d)
			r1 = r1 ? r1 : 5;
		dg_pan();
		printf("G2D test: mod=%d g2d=%u (%u us) pan/dondurme=%u (%u us) son_g2d=%d\n", dg_mod, dg_st[0], dg_st[1], dg_st[2], dg_st[3], dg_son_g2d);
		if (dg_ornek && !r1) {
			static const unsigned px[11] = { 40, 120, 200, 280, 360, 440, 0, 120, 240, 360, 479 };
			uint32_t g[11];
			const uint8_t *kay = dg_mod == 2 ? dg_tm : dg_fbm;
			size_t sat = dg_mod == 2 ? PC_SHM_W * 4u : dg_line;
			for (int i = 0; i < 11; i++) {
				unsigned yy = y + (i < 6 ? 70 : 170), xx = x + px[i];
				g[i] = *(const volatile uint32_t *)(kay + (size_t)yy * sat + (size_t)xx * 4u);
			}
			dg_sahip = 0;	/* ayni JPEG: VE + CPU donusumu (eski yol) */
			file_mode1(argv[2], x, y, sc);
			for (int i = 0; i < 11; i++) {
				unsigned yy = y + (i < 6 ? 70 : 170), xx = x + px[i];
				uint16_t p = file_frame[(size_t)yy * PC_SHM_W + xx];
				printf("ORNEK %-6s G2D R=%3u G=%3u B=%3u | CPU R=%3u G=%3u B=%3u\n",
				       i < 6 ? (const char *[]){ "kirmz", "yesil", "mavi", "beyaz", "sari", "camgob" }[i] : "rampa",
				       (g[i] >> 16) & 255, (g[i] >> 8) & 255, g[i] & 255,
				       (unsigned)((p >> 11) << 3 | (p >> 13)), (unsigned)(((p >> 5) & 63) << 2 | ((p >> 9) & 3)),
				       (unsigned)((p & 31) << 3 | ((p >> 2) & 7)));
			}
		}
		dg_bitir();
	}
	if (!r1 && ikinci) {
		puts("-- ikinci JPEG (ayni surec, ayni konum)");
		r1 = file_mode1(ikinci, x, y, sc);
	}
	return r1;
}

static int file_mode1(const char *yol, unsigned x, unsigned y, unsigned sc)
{
	FILE *f = fopen(yol, "rb");
	if (!f)
		return 2;
	size_t n = fread(jpeg, 1, sizeof jpeg, f);
	int extra = fgetc(f);
	fclose(f);
	if (!n || extra != EOF)
		return 2;
	frame = file_frame;
	/* boyutu JPEG'den al, sonra hedefle karsilastirarak coz */
	struct src s = { .size = n };
	JDEC jd;
	int r = jd_prepare(&jd, in_cb, pool, sizeof pool, &s);
	unsigned w = r ? 0 : (unsigned)jd.width << sc, h = r ? 0 : (unsigned)jd.height << sc;
	if (!r && (x + w > PC_SHM_W || y + h > PC_SHM_H))
		r = 3;
	else if (!r)
		r = decode(n, x, y, w, h, sc);
	printf("status=%d w=%u h=%u ilk=%04x son=%04x\n", r, w, h, file_frame[(size_t)y * PC_SHM_W + x],
	       w ? file_frame[(size_t)(y + h - 1) * PC_SHM_W + x + w - 1] : 0);
	return r ? 1 : 0;
}

/* v18: PC'siz prova: dosyadaki H.264 akisini erisim birimi erisim birimi VE -> G2D -> ekran */
static int h264_dosya(int argc, char **argv)
{
	int fps = 30;
	for (int i = 3; i < argc; i++)
		if (!strcmp(argv[i], "--fps") && i + 1 < argc) fps = atoi(argv[++i]);
	if (fps < 1 || fps > 120)
		return 2;
	FILE *f = fopen(argv[2], "rb");
	if (!f)
		return 2;
	fseek(f, 0, SEEK_END);
	long boy = ftell(f);
	rewind(f);
	uint8_t *v = malloc((size_t)boy + 4);
	if (!v || fread(v, 1, (size_t)boy, f) != (size_t)boy) { fclose(f); free(v); return 2; }
	fclose(f);
	dg_baslat();
	if (!dg_hazir) { free(v); return 4; }
	dg_sahip = 1;
	unsigned kare = 0, hata = 0;
	double t_coz = 0, t_top = 0, en = 0;
	long au_bas = -1;
	int au_dilim = 0;
	struct timespec t0;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	for (long i = 0; i <= boy; i++) {
		int nal = i + 3 < boy && v[i] == 0 && v[i + 1] == 0 && v[i + 2] == 1;
		int yeni_au = 0;
		if (nal) {
			int tur = v[i + 3] & 31;
			if ((tur == 1 || tur == 5) && hv_ilk_mb(v + i + 3, (size_t)(boy - i - 3) < 16 ? (size_t)(boy - i - 3) : 16) == 0 && au_dilim)
				yeni_au = 1;
			if ((tur == 7 || tur == 9) && au_dilim)
				yeni_au = 1;
		}
		if ((yeni_au || i == boy) && au_bas >= 0) {
			size_t n = (size_t)(i - au_bas);
			if (n <= MAX_JPEG) {
				memcpy(jpeg, v + au_bas, n);
				double a = hv_ms();
				int st = h264_coz(n);
				double b = hv_ms();
				if (st) hata++;
				else {
					dg_pan();
					kare++;
					t_top += b - a;
					if (b - a > en) en = b - a;
				}
				/* kare hizi */
				struct timespec t1;
				clock_gettime(CLOCK_MONOTONIC, &t1);
				double gecen = (t1.tv_sec - t0.tv_sec) * 1e3 + (t1.tv_nsec - t0.tv_nsec) / 1e6, hedef = kare * 1000.0 / fps;
				if (hedef > gecen) dg_uyu((uint32_t)((hedef - gecen) * 1000));
			} else
				hata++;
			au_bas = i;
			au_dilim = 0;
		}
		if (nal) {
			if (au_bas < 0) au_bas = i;
			int tur = v[i + 3] & 31;
			if (tur == 1 || tur == 5) au_dilim = 1;
			i += 2;
		}
	}
	t_coz = h264_ms;
	printf("H264 dosya: %u kare, %u hata; VE cozme ort %.2f ms; cozme+G2D ort %.2f ms en cok %.2f ms; G2D blit %u (%u us) dondurme %u (%u us)\n",
	       kare, hata, h264_sayac ? t_coz / h264_sayac : 0, kare ? t_top / kare : 0, en, dg_st[0], dg_st[1], dg_st[2], dg_st[3]);
	dg_bitir();
	free(v);
	return hata ? 1 : 0;
}

/* v23 --self-test: izinli_ip, JPEG bolum/secici denetimleri, H.264 SPS boyutu ve PPS ref siniri (yazilim ayristiricilari) */
struct bit_yaz { uint8_t b[64]; unsigned p; };
static void by_u(struct bit_yaz *y, uint32_t v, int n)
{
	for (int i = n - 1; i >= 0; i--, y->p++)
		if (v >> i & 1)
			y->b[y->p >> 3] |= (uint8_t)(0x80u >> (y->p & 7));
}
static void by_ue(struct bit_yaz *y, uint32_t v)
{
	uint64_t k = (uint64_t)v + 1;	/* v < 0xffffffff */
	int z = 0;
	while (k >> (z + 1))
		z++;
	by_u(y, 0, z);
	by_u(y, (uint32_t)k, z + 1);
}
/* NAL basligi + RBSP sonlandirma biti; boy doner */
static size_t by_bitir(struct bit_yaz *y)
{
	by_u(y, 1, 1);
	return (y->p + 7) / 8;
}
static int sps_dene_t(unsigned w_mb, unsigned h_mu, unsigned l2, unsigned ref)
{
	struct bit_yaz y = { { 0x67 }, 8 };
	by_u(&y, 66, 8);	/* baseline */
	by_u(&y, 30, 16);
	by_ue(&y, 0); by_ue(&y, l2); by_ue(&y, 0); by_ue(&y, 0);	/* sps_id, log2_fn-4, poc_tur 0, log2_poc-4 */
	by_ue(&y, ref); by_u(&y, 0, 1);					/* ref_sayi, bosluk */
	by_ue(&y, w_mb - 1); by_ue(&y, h_mu - 1);
	by_u(&y, 1, 1); by_u(&y, 1, 1); by_u(&y, 1, 1);			/* frame_mbs_only, d8x8, kirpma */
	by_ue(&y, 0); by_ue(&y, 0); by_ue(&y, 0); by_ue(&y, 4);	/* alt kirpma 8 piksel */
	by_u(&y, 0, 1);							/* vui yok */
	size_t n = by_bitir(&y);
	return hv_sps_coz(y.b, n);
}
static int sps_dene(unsigned w_mb, unsigned h_mu) { return sps_dene_t(w_mb, h_mu, 0, 1); }
static int pps_dene(uint32_t r0, uint32_t r1)
{
	struct bit_yaz y = { { 0x68 }, 8 };
	by_ue(&y, 0); by_ue(&y, 0); by_u(&y, 0, 2); by_ue(&y, 0);	/* pps/sps id, cavlc, bfpo, dilim grubu */
	by_ue(&y, r0); by_ue(&y, r1);
	by_u(&y, 0, 3); by_ue(&y, 0); by_ue(&y, 0); by_ue(&y, 0);	/* wp, wbi, qp, qs, cqp */
	by_u(&y, 4, 3);							/* dbk=1, cip=0, rpc=0 */
	size_t n = by_bitir(&y);
	return hv_pps_coz(y.b, n);
}
static int self_test(void)
{
	int hata = 0;
#define ST(x) do { if (!(x)) { printf("FAIL satir %d: %s\n", __LINE__, #x); hata++; } } while (0)
	char yol[64];
	snprintf(yol, sizeof yol, "/tmp/wifi-ekran-v24-izinli-%d", (int)getpid());
	int r = izinli_ip_test(yol);
	ST(r == 0);
	static const uint8_t ok[35] = { 0xff, 0xd8, 0xff, 0xc0, 0x00, 0x11, 0x08, 0x00, 0x08, 0x00, 0x08, 0x03,
		0x01, 0x11, 0x00, 0x02, 0x11, 0x01, 0x03, 0x11, 0x01,
		0xff, 0xda, 0x00, 0x0c, 0x03, 0x01, 0x00, 0x02, 0x11, 0x03, 0x11, 0x00, 0x3f, 0x00 };
	uint8_t t[sizeof ok];
	struct jpg j;
	ST(jpeg_ayristir(ok, sizeof ok, &j) == 0 && j.dcsel[1] == 1 && j.acsel[1] == 1 && j.ecs == sizeof ok);
	memcpy(t, ok, sizeof t); t[29] = 0x41;		/* 2. bilesen DC tablosu 4 */
	ST(jpeg_ayristir(t, sizeof t, &j) == -16);
	memcpy(t, ok, sizeof t); t[31] = 0x1f;		/* 3. bilesen AC tablosu 15 */
	ST(jpeg_ayristir(t, sizeof t, &j) == -16);
	memcpy(t, ok, sizeof t); t[24] = 0x0a;		/* SOS kisa */
	ST(jpeg_ayristir(t, sizeof t, &j) == -14);
	static const uint8_t dht[24] = { 0xff, 0xd8, 0xff, 0xc4, 0x00, 0x05, 0x00, 0x01, 0x02 };	/* v21: sayilar sifirlardan */
	ST(jpeg_ayristir(dht, sizeof dht, &j) == -13);	/* sayilar bolumden tasiyor */
	static const uint8_t dqt[] = { 0xff, 0xd8, 0xff, 0xdb, 0x00, 0x08, 0x00, 1, 2, 3, 4, 5, 0xff, 0xd9 };
	ST(jpeg_ayristir(dqt, sizeof dqt, &j) == -15);	/* 64 baytlik tablo 5 baytlik bolumde */
	ST(sps_dene(HV_W_MB, HV_H_MU) == 0 && HS.w_mb == HV_W_MB && HS.h_mu == HV_H_MU && HS.crop_b == 8);
	ST(sps_dene(120, 68) == -4);		/* 1920x1088: tampon ayrilmadan red */
	ST(sps_dene(HV_W_MB, HV_H_MU + 1) == -4);
	ST(sps_dene_t(HV_W_MB, HV_H_MU, 13, 1) == -4 && sps_dene_t(HV_W_MB, HV_H_MU, 12, 2) == 0);	/* 614: log2 > 12, ref 2 kabul */
	ST(sps_dene_t(HV_W_MB, HV_H_MU, 0, 3) == -4 && sps_dene(HV_W_MB, HV_H_MU) == 0);		/* 614: ref 3 red; HS geri */
	ST(pps_dene(31, 0) == 0 && HP.ref0 == 32 && HP.ref1 == 1);
	ST(pps_dene(32, 0) == -5);
	ST(pps_dene(0, 32) == -5);
	ST(pps_dene(0xfffffffeu, 0) == -5);
#undef ST
	if (hata) {
		printf("SELF-TEST FAIL (%d)\n", hata);
		return 1;
	}
	puts("SELF-TEST PASS: izinli_ip; JPEG SOS secici <=3, SOS/DHT/DQT bolum siniri; H.264 SPS yalniz 100x38 MB, PPS ref <=32");
	return 0;
}

int main(int argc, char **argv)
{
	signal(SIGINT, stop);
	signal(SIGTERM, stop);
	signal(SIGPIPE, SIG_IGN);
	if (argc == 2 && !strcmp(argv[1], "--self-test"))	/* v23: VE/ag/ortak bellek acilmaz */
		return self_test();
	ve_baslat();
	if (argc >= 3 && !strcmp(argv[1], "--h264")) {	/* v18 */
		int fr = h264_dosya(argc, argv);
		if (h264_hazir == 1)
			hv_bitir();
		ve_bitir();
		return fr;
	}
	if (argc >= 3 && !strcmp(argv[1], "--file")) {
		int fr = file_mode(argc, argv);
		printf("ve=%u sw=%u\n", ve_sayac, sw_sayac);
		ve_bitir();
		return fr;
	}
	int port = 8766, seconds = 60;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--port") && i + 1 < argc) port = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--seconds") && i + 1 < argc) seconds = atoi(argv[++i]);
		else if (!strcmp(argv[i], "--shm") && i + 1 < argc) snprintf(shm_path, sizeof shm_path, "%s", argv[++i]);
		else return 2;
	}
	if (port < 1024 || port > 65535 || seconds < 5 || seconds > 3600)
		return 2;
	deadline_ms = now() + (uint32_t)seconds * 1000u;
	int ls = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0), one = 1;
	if (ls < 0)
		return 2;
	setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
	struct sockaddr_in a = { .sin_family = AF_INET, .sin_port = htons((uint16_t)port) };
	if (bind(ls, (struct sockaddr *)&a, sizeof a) || listen(ls, 1)) {
		perror("listen");
		close(ls);
		return 2;
	}
	if (!shm_create()) {
		close(ls);
		return 3;
	}
	unlink(DOKUNMA_FIFO);
	if (mkfifo(DOKUNMA_FIFO, 0622) == 0)
		dok_fd = open(DOKUNMA_FIFO, O_RDWR | O_NONBLOCK | O_CLOEXEC);
	dg_baslat();
	if (mlockall(MCL_CURRENT | MCL_FUTURE | MCL_ONFAULT))	/* v20 */
		printf("mlockall: hata %d (kilitsiz devam)\n", errno);
	else
		printf("mlockall: tamam (ONFAULT; dokunulan sayfalar takasa gitmez)\n");
	printf("wifi-ekran v24 port=%d seconds=%d shm=%s %ux%u RGB565\n", port, seconds, shm_path, PC_SHM_W, PC_SHM_H);
	fflush(stdout);
	unsigned frames = 0, errors = 0;
	uint64_t bytes = 0, dec = 0;
	while (!timeup()) {
		H->alive_ms = now();
		if (!waitfd(ls, POLLIN, now() + 500))
			continue;
		struct sockaddr_in pa;
		socklen_t pl = sizeof pa;
		int c = accept(ls, (struct sockaddr *)&pa, &pl);
		if (c < 0)
			continue;
		if (!izinli_soket(c, NULL)) {	/* v23 */
			static unsigned red;
			if (red++ < 20)	/* gunluk sinirli: sure dolunca surec yeniden baslar */
				printf("%u RED %s (izinli-ip)\n", now() / 1000, inet_ntoa(pa.sin_addr));
			fflush(stdout);
			close(c);
			continue;
		}
		serve(c, &frames, &errors, &bytes, &dec);
		close(c);
	}
	close(ls);
	shm_destroy();
	dg_bitir();
	if (h264_hazir == 1)	/* v18 */
		hv_bitir();
	ve_bitir();
	if (dok_fd >= 0) {
		close(dok_fd);
		unlink(DOKUNMA_FIFO);
	}
	printf("G2D: blit=%u us=%u pan=%u pan_us=%u cpu_rect=%u esitleme=%u tani_gec=%u/%u\n", dg_st[0], dg_st[1], dg_st[2], dg_st[3], dg_st[4], dg_st[5], dg_gec, dg_denetim);
	printf("H264: kare=%u hata=%u ort_ms=%.2f\n", h264_sayac, h264_hata, h264_sayac ? h264_ms / h264_sayac : 0.0);
	printf("FINISHED frames=%u errors=%u touches=%u ve=%u sw=%u bytes=%llu mean_decode_us=%llu stopped=%d\n", frames, errors, dok_sayac, ve_sayac, sw_sayac,
	       (unsigned long long)bytes, (unsigned long long)(frames ? dec / frames : 0), stopped);
	return errors ? 1 : 0;
}
