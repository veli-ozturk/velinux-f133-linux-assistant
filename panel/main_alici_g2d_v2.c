/*
 * F133 masaustu panel arayuzu (LVGL 9.2, Linux fbdev + evdev, statik).
 *
 * 0.19 (11.10.2026, Claude): devir dikey fb'de de (flash cekirdegi, alici v13 mod 2: G2D mixer + dondurme).
 *   Devir geri alinirken alicinin onayi (pad[0]==nesil, pad[1]==0) beklenir (en cok 400 ms): bu arada panel
 *   ortak bellekteki eski goruntuyu cizmez/flush etmez (0.18 canli: "arada kare kare renk bozulmasi").
 * 0.18 (11.10.2026, Claude): ALICIYA DEVIR. Dogrudan cizim kosullari saglaninca /tmp/pc-dogrudan dosyasina
 *   {nesil, izin=1} yazilir; alici v12 basligin pad[0..1]'ine ayni nesli ve 1 yazinca (sahip) panel PC
 *   dikdortgenlerini cizmez, LVGL flush fb'ye yazmaz (alici G2D ile ciziyor). Cubuk/bildirim/cikis aninda izin=0;
 *   alici fb->ortak bellek esitler ve tam ekran dikdortgeni yayinlar. Alici v11 ise onay gelmez: 0.17-v3 yolu.
 *   PANEL_G2D_ALICI=0 devri kapatir.
 * 0.17-v3: kirli dikdortgenler 16x16 karo haritasinda birlestirilir (v2 canli: cizim basina 2.4 Mpiksel, ust uste
 *   binen alici dikdortgenleri; 172 ms/cizim), satir parcalari 64 bit yazmayla iki piksel birden yazilir.
 *   JSON'a dc_cv_us (yalniz donusum) eklendi.
 * 0.17-v2: ust katman denetimi yalniz GORUNUR nesnelere bakar (v1: gizli toast/ekran koruyucu/Wi-Fi penceresi
 *   hep katmanda oldugu icin dogrudan yol hic calismadi, canli olcum dc_n=0).
 * 0.17 DOGRUDAN CIZIM (11.10.2026, Claude): PC tam ekrandayken (ust cubuk/ust katman bos) alicinin kirli
 *   dikdortgenleri LVGL'siz, ortak bellekten (RGB565) dogrudan fb0'a (XRGB8888) donusturulur, tum kirli
 *   alanlar icin TEK FBIOPAN_DISPLAY (G2D dondurme). 0.16 olcumu: LVGL yenilemesi 80-195 ms/5-11 fps darbogazdi.
 *   PANEL_DOGRUDAN=0 ortam degiskeni eski LVGL yolunu zorlar. Cubuk gorunurken LVGL yolu kullanilir.
 * G2D prova: fb0 1600x600, rotate=1; son LVGL flush sonrasi FBIOPAN_DISPLAY.
 * Eski 600x1600 framebuffer icin yazilim 90 derece yolu da korunur.
 * Dokunmatik: GT913 ham 0..1023 x 0..599 (DTS'te iki eksen ters cevrili) -> 1600x600'e olceklenir.
 * PC baglantisi: panel TCP 8765'te dinler, PC ajani (work\panel_ajan_v3.py) baglanir. Her satir bir JSON:
 *   PC -> panel : {"t":"state","time":...,"pc":{...},"weather":{...},"fx":{...},"events":[...],
 *                  "macros":[{"id","label"}],"prompts":[{"id","label"}],"assistant":{"text","busy"},"toast":"..."}
 *   panel -> PC : {"t":"macro","id":"..."}  {"t":"ask","id":"..."}  {"t":"hello","ver":"..."}
 *   (Baglanti PC'den panele kuruldugu icin Windows guvenlik duvari gelen baglanti izni istemez.)
 */
#define _GNU_SOURCE
#include "lvgl/lvgl.h"
#include "cJSON.h"
#include "bt_status.h"
#include "pc_shm_v1.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define PANEL_VER "0.19-alici-g2d-v2 (11.10.2026)"
#define APP_ADI "VeLinux KÖLE"
LV_IMAGE_DECLARE(amblem_40);
LV_IMAGE_DECLARE(logo_tam);
#define W 1600
#define H 600
#define PORT 8765
#define MAX_BTN 12

/* ------------------------------------------------------------------ ekran */
static uint8_t *fbm;
static uint32_t fb_line, fb_xres, fb_yres;
static int fb_rot, fb_fd = -1, fb_hw_pan;
static struct fb_var_screeninfo fb_var;
static uint64_t pc_pan_us, pc_pan_calls, pc_pan_errors;
static uint64_t pc_dc_us, pc_dc_n, pc_dc_px, pc_dc_cv_us;	/* 0.17 dogrudan cizim: sure (donusum+pan), pan sayisi, piksel */
static int pc_dc_izin = 1;
static int pc_dg_fd = -1, pc_dg_izin = -1, pc_dg_acik = 1, pc_rv_sahip;	/* 0.18 aliciya devir */
static uint32_t pc_dg_gen, pc_rv_rects, pc_dg_t0, pc_dg_bekleme;
static int pc_dg_bekle;
static int pc_view_active;
static uint64_t pc_flush_us, pc_flush_calls;
/* 0.14: bir yenileme turunun (REFR_START -> son flush) suresi, PC goruntusu acikken */
static struct timespec pc_refr_t0;
static uint64_t pc_render_sum;
static uint32_t pc_render_n, pc_render_max;
static void refr_start_cb(lv_event_t *e)
{
	(void)e;
	clock_gettime(CLOCK_MONOTONIC, &pc_refr_t0);
}

static uint64_t elapsed_us(const struct timespec *a, const struct timespec *b)
{
	return (uint64_t)((b->tv_sec-a->tv_sec)*1000000 + (b->tv_nsec-a->tv_nsec)/1000);
}

static int fb_init(void)
{
	struct fb_var_screeninfo v;
	struct fb_fix_screeninfo f;
	int fd = open("/dev/fb0", O_RDWR);

	if (fd < 0 || ioctl(fd, FBIOGET_VSCREENINFO, &v) || ioctl(fd, FBIOGET_FSCREENINFO, &f)) {
		perror("fb0");
		return -1;
	}
	if (v.bits_per_pixel != 32) {
		fprintf(stderr, "fb0 %d bpp desteklenmiyor\n", v.bits_per_pixel);
		return -1;
	}
	fb_line = f.line_length;
	fb_xres = v.xres;
	fb_yres = v.yres;
	fb_rot = fb_yres > fb_xres;
	if ((fb_rot ? fb_yres : fb_xres) != W || (fb_rot ? fb_xres : fb_yres) != H ||
	    fb_line < fb_xres * 4u || f.smem_len < (size_t)fb_line * fb_yres ||
	    v.xoffset || v.yoffset) {
		fprintf(stderr, "fb0 geometri/uzunluk/ofset desteklenmiyor\n");
		close(fd);
		return -1;
	}
	fb_fd = fd;
	fb_var = v;
	fb_hw_pan = !fb_rot && v.rotate == 1;
	fbm = mmap(NULL, f.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fbm == MAP_FAILED)
		return -1;
	memset(fbm, 0, (size_t)fb_line * fb_yres);
	/* fbcon'un imleci ve yazilari arayuzun ustune cizmesin */
	int tty = open("/dev/tty1", O_RDWR);
	if (tty >= 0) {
		ioctl(tty, KDSETMODE, KD_GRAPHICS);
		close(tty);
	}
	printf("fb0 %ux%u satir %u, yazilim-rot=%d rotate=%u G2D-pan=%d\n", fb_xres, fb_yres, fb_line, fb_rot, v.rotate, fb_hw_pan);
	fflush(stdout);
	return 0;
}

static void flush_cb(lv_display_t *d, const lv_area_t *a, uint8_t *px)
{
	int32_t w = lv_area_get_width(a);
	struct timespec start, end;
	int last = lv_display_flush_is_last(d);
	if (pc_rv_sahip || pc_dg_bekle) {	/* 0.18: fb'yi alici G2D ile ciziyor; devir bitince tam ekran yeniden cizilir */
		lv_display_flush_ready(d);
		return;
	}
	if (pc_view_active) clock_gettime(CLOCK_MONOTONIC, &start);

	if (fb_rot) {
		for (int32_t x = a->x1; x <= a->x2; x++) {
			if (x < 0 || (uint32_t)x >= fb_yres)
				continue;
			uint32_t *dst = (uint32_t *)(fbm + (size_t)x * fb_line);
			const uint32_t *src = (const uint32_t *)px + (x - a->x1);
			for (int32_t y = a->y1; y <= a->y2; y++)
				dst[fb_xres - 1 - y] = src[(size_t)(y - a->y1) * w] | 0xFF000000u;
		}
	} else {
		for (int32_t y = a->y1; y <= a->y2; y++) {
			uint32_t *dst = (uint32_t *)(fbm + (size_t)y * fb_line) + a->x1;
			const uint32_t *src = (const uint32_t *)px + (size_t)(y - a->y1) * w;
			for (int32_t x = 0; x < w; x++)
				dst[x] = src[x] | 0xFF000000u;
		}
	}
	/* Pan only after all partial buffers of this refresh are copied. G2D rotates
	 * the entire mapped source; calling once per partial wastes bandwidth/time.
	 * panel is launched in the background so a driver stall leaves COM5 usable. */
	if (last && fb_hw_pan) {
		struct timespec p0, p1;
		clock_gettime(CLOCK_MONOTONIC, &p0);
		int r = ioctl(fb_fd, FBIOPAN_DISPLAY, &fb_var);
		clock_gettime(CLOCK_MONOTONIC, &p1);
		if (pc_view_active) {
			pc_pan_calls++;
			pc_pan_us += elapsed_us(&p0, &p1);
		}
		if (r < 0) {
			pc_pan_errors++;
			if (pc_pan_errors <= 2) perror("G2D FBIOPAN_DISPLAY");
		}
	}
	lv_display_flush_ready(d);
	if (pc_view_active) {
		clock_gettime(CLOCK_MONOTONIC, &end);
		pc_flush_us += (uint64_t)((end.tv_sec-start.tv_sec)*1000000 + (end.tv_nsec-start.tv_nsec)/1000);
		pc_flush_calls++;
		if (last) {
			uint32_t us = (uint32_t)((end.tv_sec - pc_refr_t0.tv_sec) * 1000000 + (end.tv_nsec - pc_refr_t0.tv_nsec) / 1000);
			pc_render_sum += us;
			pc_render_n++;
			if (us > pc_render_max)
				pc_render_max = us;
		}
	}
}

static uint32_t tick_ms(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (uint32_t)(t.tv_sec * 1000 + t.tv_nsec / 1000000);
}

/* ------------------------------------------------------------- dokunmatik */
static int ev_fd = -1, ev_x, ev_y, ev_down, ev_xmax = 1023, ev_ymax = 599;

static void touch_init(void)
{
	char p[32], name[64];

	for (int i = 0; i < 8; i++) {
		snprintf(p, sizeof p, "/dev/input/event%d", i);
		int fd = open(p, O_RDONLY | O_NONBLOCK);
		if (fd < 0)
			continue;
		name[0] = 0;
		ioctl(fd, EVIOCGNAME(sizeof name), name);
		if (strstr(name, "Goodix") || strstr(name, "Touch")) {
			struct input_absinfo ai;
			if (!ioctl(fd, EVIOCGABS(ABS_X), &ai) && ai.maximum > 0)
				ev_xmax = ai.maximum;
			if (!ioctl(fd, EVIOCGABS(ABS_Y), &ai) && ai.maximum > 0)
				ev_ymax = ai.maximum;
			ev_fd = fd;
			printf("dokunmatik %s (%s) %dx%d\n", p, name, ev_xmax, ev_ymax);
			return;
		}
		close(fd);
	}
	printf("dokunmatik bulunamadi\n");
}

static void touch_read(lv_indev_t *i, lv_indev_data_t *data)
{
	struct input_event ev;

	(void)i;
	while (ev_fd >= 0 && read(ev_fd, &ev, sizeof ev) == sizeof ev) {
		if (ev.type == EV_ABS) {
			if (ev.code == ABS_X || ev.code == ABS_MT_POSITION_X)
				ev_x = ev.value;
			else if (ev.code == ABS_Y || ev.code == ABS_MT_POSITION_Y)
				ev_y = ev.value;
		} else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
			ev_down = ev.value;
		}
	}
	data->point.x = (int32_t)((int64_t)ev_x * (W - 1) / ev_xmax);
	data->point.y = (int32_t)((int64_t)ev_y * (H - 1) / ev_ymax);
	data->state = ev_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

/* ------------------------------------------------------------- parlaklik */
/* Tina disp2: DISP_LCD_SET_BRIGHTNESS 0x102, GET 0x103; args {ekran, deger} */
/* Current board/flash PWM is inverse: increasing the driver level dims the
 * physical panel (kullanıcı confirmed). Keep UI, PC and screensaver levels logical:
 * larger means brighter. Mirror only at this device boundary, within 3..255.
 * Recheck this mapping if a future kernel/DT fixes the PWM polarity. */
static int disp_brightness(int set)
{
	int logical = set < 3 ? 3 : set > 255 ? 255 : set;
	unsigned long a[4] = {0, (unsigned long)(258 - logical), 0, 0};
	int fd = open("/dev/disp", O_RDWR), r = -1;

	if (fd < 0)
		return -1;
	r = ioctl(fd, set >= 0 ? 0x102 : 0x103, a);
	close(fd);
	if (set < 0 && r >= 0) {
		r = 258 - r;
		r = r < 3 ? 3 : r > 255 ? 255 : r;
	}
	return r;
}

/* ------------------------------------------------------------------- ag */
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static char *pending;		/* PC'den gelen son durum satiri */
static char *ui_pending[8];	/* {"t":"ui",...} komutlari (durum satiri ezmesin diye ayri kuyruk) */
static int ui_n;
static char outq[16][192];
static int outn;
static volatile int pc_conn;
static volatile time_t pc_last;	/* monoton saniye (saat ayari atlamasindan etkilenmez) */
static time_t mono_s(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec;
}
static char pc_peer[48] = "-";

static void send_event(const char *json)
{
	pthread_mutex_lock(&mu);
	if (outn < 16)
		snprintf(outq[outn++], sizeof outq[0], "%s\n", json);
	pthread_mutex_unlock(&mu);
}

static void *net_thread(void *arg)
{
	int ls = socket(AF_INET, SOCK_STREAM, 0), one = 1;
	struct sockaddr_in a = {.sin_family = AF_INET, .sin_port = htons(PORT)};

	(void)arg;
	setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
	if (bind(ls, (struct sockaddr *)&a, sizeof a) || listen(ls, 2)) {
		perror("dinleme");
		return NULL;
	}
	for (;;) {
		struct sockaddr_in ca;
		socklen_t cl = sizeof ca;
		int c = accept(ls, (struct sockaddr *)&ca, &cl);
		if (c < 0)
			continue;
		setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
		snprintf(pc_peer, sizeof pc_peer, "%s", inet_ntoa(ca.sin_addr));
		pc_conn = 1;
		pc_last = mono_s();
		printf("PC baglandi: %s\n", pc_peer);
		char hello[96];
		snprintf(hello, sizeof hello, "{\"t\":\"hello\",\"ver\":\"%s\"}\n", PANEL_VER);
		send(c, hello, strlen(hello), MSG_NOSIGNAL);
		size_t cap = 1 << 16, len = 0;
		char *buf = malloc(cap);
		for (;;) {
			struct pollfd pf = {.fd = c, .events = POLLIN};
			int r = poll(&pf, 1, 100);
			if (r > 0) {
				if (len + 4096 > cap) {
					if (cap >= (1 << 20)) {
						len = 0;	/* asiri uzun satir: at */
					} else {
						cap *= 2;
						buf = realloc(buf, cap);
					}
				}
				ssize_t n = recv(c, buf + len, cap - len - 1, 0);
				if (n <= 0)
					break;
				len += n;
				buf[len] = 0;
				char *nl;
				while ((nl = memchr(buf, '\n', len))) {
					*nl = 0;
					if (nl > buf) {
						pthread_mutex_lock(&mu);
						if (strstr(buf, "\"t\":\"ui\"") || strstr(buf, "\"t\": \"ui\"")) {
							if (ui_n < 8)
								ui_pending[ui_n++] = strdup(buf);
						} else {
							free(pending);
							pending = strdup(buf);
						}
						pthread_mutex_unlock(&mu);
						pc_last = mono_s();
					}
					len -= (nl + 1 - buf);
					memmove(buf, nl + 1, len);
				}
			}
			pthread_mutex_lock(&mu);
			int n = outn;
			char q[16][192];
			memcpy(q, outq, sizeof q);
			outn = 0;
			pthread_mutex_unlock(&mu);
			for (int i = 0; i < n; i++)
				if (send(c, q[i], strlen(q[i]), MSG_NOSIGNAL) < 0)
					break;
			if (mono_s() - pc_last > 30)
				break;	/* 30 sn veri yok: baglantiyi birak */
		}
		free(buf);
		close(c);
		pc_conn = 0;
		printf("PC baglantisi kapandi\n");
	}
	return NULL;
}

/* ----------------------------------------------------------------- yazitipi */
static lv_font_t *f_s, *f_m, *f_mb, *f_l, *f_xl, *f_clock;

LV_FONT_DECLARE(ubuntu_r22)
LV_FONT_DECLARE(ubuntu_r28)
LV_FONT_DECLARE(ubuntu_b28)
LV_FONT_DECLARE(ubuntu_b40)
LV_FONT_DECLARE(ubuntu_b64)
LV_FONT_DECLARE(ubuntu_b150)
static lv_font_t F_s, F_m, F_mb, F_l, F_xl, F_clock;

static lv_font_t *fcopy(lv_font_t *dst, const lv_font_t *src, const lv_font_t *fb)
{
	*dst = *src;
	dst->fallback = fb;
	return dst;
}

static void fonts_init(void)
{
	f_s = fcopy(&F_s, &ubuntu_r22, &lv_font_montserrat_20);
	f_m = fcopy(&F_m, &ubuntu_r28, &lv_font_montserrat_28);
	f_mb = fcopy(&F_mb, &ubuntu_b28, &lv_font_montserrat_28);
	f_l = fcopy(&F_l, &ubuntu_b40, &lv_font_montserrat_28);
	f_xl = fcopy(&F_xl, &ubuntu_b64, &lv_font_montserrat_28);
	f_clock = fcopy(&F_clock, &ubuntu_b150, &lv_font_montserrat_28);
	printf("yazitipi: Ubuntu bitmap (22/28/40/64/150)\n");
}

/* ---------------------------------------------------------------------- arayuz */
static const char *gunler[] = {"Pazar", "Pazartesi", "Salı", "Çarşamba", "Perşembe", "Cuma", "Cumartesi"};
static const char *aylar[] = {"Ocak", "Şubat", "Mart", "Nisan", "Mayıs", "Haziran", "Temmuz",
			      "Ağustos", "Eylül", "Ekim", "Kasım", "Aralık"};

/* Temalar: renkler tek tablodan; C_* makrolari etkin temayi okur */
typedef struct {
	const char *ad;
	uint32_t bg, card, acc, acc2, text, dim, bar, btn, btn2, sep, toast, toast_text;
	int dark;
} tema_t;
static const tema_t temalar[] = {
	{"Mavi", 0x0B1220, 0x152036, 0x3B82F6, 0x0EA5E9, 0xE5E7EB, 0x94A3B8, 0x0F1A2E, 0x1F2A44, 0x1E3A5F, 0x24324D,
	 0x1F2937, 0xF9FAFB, 1},
	{"Kırmızı & Siyah", 0x050505, 0x161010, 0xE11D48, 0xF97316, 0xF5F5F5, 0xA8A29E, 0x0E0909, 0x2A1414, 0x3A1418,
	 0x3A1E1E, 0x262626, 0xFAFAFA, 1},
	{"Beyaz", 0xEEF2F7, 0xFFFFFF, 0x2563EB, 0x0891B2, 0x0F172A, 0x64748B, 0xE2E8F0, 0xE2E8F0, 0xDBEAFE, 0xCBD5E1,
	 0x1E293B, 0xF8FAFC, 0},
};
#define TEMA_N ((int)(sizeof temalar / sizeof temalar[0]))
static int tema;
#define TH (temalar[tema])
#define C_BG lv_color_hex(TH.bg)
#define C_CARD lv_color_hex(TH.card)
#define C_ACC lv_color_hex(TH.acc)
#define C_OK lv_color_hex(0x22C55E)
#define C_WARN lv_color_hex(0xF59E0B)
#define C_BAD lv_color_hex(0xEF4444)
#define C_DIM lv_color_hex(TH.dim)

static lv_obj_t *l_hdr_date, *l_hdr_pc, *l_hdr_net;
static lv_obj_t *l_clock, *l_sec, *l_date, *l_weather, *l_weather2;
static lv_obj_t *l_pcname, *b_cpu, *l_cpu, *b_ram, *l_ram, *b_disk, *l_disk, *l_netio, *l_pcup;
static lv_obj_t *l_fx, *l_events;
static lv_obj_t *macro_cont, *prompt_cont, *l_answer, *l_asst_state;
static lv_obj_t *l_sys, *l_bt, *toast, *tabview;
static lv_obj_t *pc_image, *l_pc_image, *pc_full, *pc_full_img, *pc_bar, *l_pc_bar;
static pc_shm_t pc_shm;
static lv_image_dsc_t pc_dsc;
static uint32_t pc_updates, pc_inv_rects, pc_inv_full, pc_dismiss_gen, pc_dismiss_conn, pc_bar_until, pc_last_alive;
static int pc_dismissed;
static char macro_sig[2048], prompt_sig[2048];
static char btn_ids[2][MAX_BTN][48];

static lv_obj_t *card(lv_obj_t *par, const char *title)
{
	lv_obj_t *c = lv_obj_create(par);

	lv_obj_set_style_bg_color(c, C_CARD, 0);
	lv_obj_set_style_border_width(c, 0, 0);
	lv_obj_set_style_radius(c, 18, 0);
	lv_obj_set_style_pad_all(c, 18, 0);
	lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(c, 8, 0);
	lv_obj_set_height(c, LV_PCT(100));
	lv_obj_set_flex_grow(c, 1);
	lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE);
	if (title) {
		lv_obj_t *t = lv_label_create(c);
		lv_label_set_text(t, title);
		lv_obj_set_style_text_font(t, f_s, 0);
		lv_obj_set_style_text_color(t, C_DIM, 0);
	}
	return c;
}

static lv_obj_t *label(lv_obj_t *par, const lv_font_t *f, const char *txt)
{
	lv_obj_t *l = lv_label_create(par);

	lv_obj_set_style_text_font(l, f, 0);
	lv_label_set_text(l, txt);
	return l;
}

static lv_obj_t *metric(lv_obj_t *par, const char *name, lv_obj_t **bar)
{
	lv_obj_t *row = lv_obj_create(par), *l;

	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	label(row, f_m, name);
	l = label(row, f_mb, "--");
	*bar = lv_bar_create(par);
	lv_obj_set_size(*bar, LV_PCT(100), 14);
	lv_bar_set_range(*bar, 0, 100);
	return l;
}

static void show_toast(const char *txt)
{
	lv_label_set_text(toast, txt);
	lv_obj_remove_flag(toast, LV_OBJ_FLAG_HIDDEN);
	lv_obj_move_foreground(toast);
	lv_obj_set_user_data(toast, (void *)(uintptr_t)(tick_ms() + 2500));
}

static void btn_cb(lv_event_t *e)
{
	uintptr_t v = (uintptr_t)lv_event_get_user_data(e);
	int kind = v >> 8, idx = v & 0xFF;
	char j[160];
	cJSON *o = cJSON_CreateObject();

	cJSON_AddStringToObject(o, "t", kind ? "ask" : "macro");
	cJSON_AddStringToObject(o, "id", btn_ids[kind][idx]);
	char *s = cJSON_PrintUnformatted(o);
	snprintf(j, sizeof j, "%s", s);
	free(s);
	cJSON_Delete(o);
	send_event(j);
	if (!pc_conn)
		show_toast(LV_SYMBOL_WARNING "  PC bağlı değil, istek kuyrukta");
	else if (kind)
		lv_label_set_text(l_asst_state, "Yanıt bekleniyor...");
	else
		show_toast(LV_SYMBOL_OK "  Gönderildi");
}

/* id/label listesinden buton izgarasi kur (kind 0 makro, 1 asistan istemi) */
static void build_buttons(lv_obj_t *cont, cJSON *arr, int kind, int bw, int bh)
{
	cJSON *it;
	int i = 0;

	lv_obj_clean(cont);
	cJSON_ArrayForEach(it, arr) {
		cJSON *id = cJSON_GetObjectItem(it, "id"), *lb = cJSON_GetObjectItem(it, "label");
		if (!cJSON_IsString(id) || !cJSON_IsString(lb) || i >= MAX_BTN)
			continue;
		snprintf(btn_ids[kind][i], sizeof btn_ids[0][0], "%s", id->valuestring);
		lv_obj_t *b = lv_button_create(cont);
		lv_obj_set_size(b, bw, bh);
		lv_obj_set_style_radius(b, 16, 0);
		lv_obj_set_style_bg_color(b, kind ? lv_color_hex(TH.btn2) : lv_color_hex(TH.btn), 0);
		lv_obj_set_style_bg_color(b, C_ACC, LV_STATE_PRESSED);
		lv_obj_t *l = label(b, kind ? f_m : f_mb, lb->valuestring);
		lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
		lv_obj_set_width(l, bw - 24);
		lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_center(l);
		lv_obj_add_event_cb(b, btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)((kind << 8) | i));
		i++;
	}
}

static const char *default_macros =
	"[{\"id\":\"playpause\",\"label\":\"" LV_SYMBOL_PLAY " Oynat / Duraklat\"},"
	"{\"id\":\"prev\",\"label\":\"" LV_SYMBOL_PREV " Önceki\"},"
	"{\"id\":\"next\",\"label\":\"" LV_SYMBOL_NEXT " Sonraki\"},"
	"{\"id\":\"mute\",\"label\":\"" LV_SYMBOL_MUTE " Sessiz\"},"
	"{\"id\":\"voldown\",\"label\":\"" LV_SYMBOL_VOLUME_MID " Ses -\"},"
	"{\"id\":\"volup\",\"label\":\"" LV_SYMBOL_VOLUME_MAX " Ses +\"},"
	"{\"id\":\"lock\",\"label\":\"" LV_SYMBOL_EYE_CLOSE " Ekranı kilitle\"},"
	"{\"id\":\"calc\",\"label\":\"" LV_SYMBOL_EDIT " Hesap makinesi\"}]";
static const char *default_prompts =
	"[{\"id\":\"ozet\",\"label\":\"Günün özeti\"},{\"id\":\"hava\",\"label\":\"Hava nasıl?\"},"
	"{\"id\":\"takvim\",\"label\":\"Takvimim\"},{\"id\":\"saka\",\"label\":\"Bir şaka\"}]";

/* Parlaklik: otomatik (PC ajaninin gece/gunduz degeri, PC yoksa panelin kendi saati) ya da manuel (%5..%100).
 * Secim /data/panel.conf'ta kalici (flash yipranmasin diye yalniz anahtar degisince ve cubuk birakilinca yazilir). */
#define PANEL_CONF "/data/panel.conf"
static int br_auto = 1, br_manual = 80, br_pc = -1, br_cur = -1, ss_on = 1, ss_shown;
static lv_obj_t *sw_auto, *br_box, *sl_br, *l_br_pct, *l_br_auto;
static lv_obj_t *ss_obj, *tm_label, *wf_win;	/* koruyucu/zamanlayici (asagida tanimli), yeniden kurulumda sifirlanir */

static int pct_to_raw(int p) { return 3 + (p * 252 + 50) / 100; }
static int raw_to_pct(int r) { return (r - 3) * 100 / 252; }

static void br_load(void)
{
	FILE *f = fopen(PANEL_CONF, "r");
	char k[32];
	int v;

	while (f && fscanf(f, "%31[^=]=%d\n", k, &v) == 2) {
		if (!strcmp(k, "parlaklik_otomatik"))
			br_auto = !!v;
		else if (!strcmp(k, "parlaklik_manuel") && v >= 5 && v <= 100)
			br_manual = v;
		else if (!strcmp(k, "tema") && v >= 0 && v < TEMA_N)
			tema = v;
		else if (!strcmp(k, "koruyucu"))
			ss_on = !!v;
	}
	if (f)
		fclose(f);
}

static void br_save(void)
{
	FILE *f = fopen(PANEL_CONF ".yeni", "w");

	if (!f)
		return;
	fprintf(f, "parlaklik_otomatik=%d\nparlaklik_manuel=%d\ntema=%d\nkoruyucu=%d\n", br_auto, br_manual, tema, ss_on);
	fclose(f);
	rename(PANEL_CONF ".yeni", PANEL_CONF);
	sync();
}

/* PC bagli degilken otomatik mod: 23:00-07:00 %15, diger saatler %80 (saat ayarli degilse gunduz) */
static int br_schedule(void)
{
	time_t now = time(NULL);
	struct tm tm;

	localtime_r(&now, &tm);
	if (tm.tm_year < 120)
		return pct_to_raw(80);
	return (tm.tm_hour >= 23 || tm.tm_hour < 7) ? pct_to_raw(15) : pct_to_raw(80);
}

static void br_apply(void)
{
	int t = br_auto ? (br_pc > 0 ? br_pc : br_schedule()) : pct_to_raw(br_manual);

	if (ss_shown)
		t = t / 3 < 3 ? 3 : t / 3;

	if (t != br_cur) {
		br_cur = t;
		disp_brightness(t);
	}
	if (!l_br_auto)
		return;
	if (br_auto)
		lv_label_set_text_fmt(l_br_auto, "Otomatik · şu an %%%d (%s)", raw_to_pct(t),
				      br_pc > 0 ? "PC gece/gündüz ayarı" : "gece 23-07 kısık");
	else
		lv_label_set_text_fmt(l_br_auto, "Manuel: çubukla seç (%%5 - %%100)");
}

static lv_display_t *disp;
static char *last_state;	/* yeniden kurulumdan sonra son PC durumunu tekrar uygulamak icin */
static void ui_build(void);
static void apply_state(const char *line);

static void ui_rebuild_async(void *u)
{
	(void)u;
	lv_obj_clean(lv_layer_top());
	lv_obj_clean(lv_screen_active());
	macro_sig[0] = prompt_sig[0] = 0;
	l_br_auto = NULL;
	ss_obj = NULL;
	ss_shown = 0;
	wf_win = NULL;
	tm_label = NULL;
	pc_image = l_pc_image = NULL;
	if (pc_full)	/* ekranla birlikte silindi */
		lv_timer_set_period(lv_display_get_refr_timer(disp), LV_DEF_REFR_PERIOD);
	pc_full = pc_full_img = pc_bar = l_pc_bar = NULL;
	br_cur = -1;
	lv_display_set_theme(disp, lv_theme_default_init(disp, C_ACC, lv_color_hex(TH.acc2), TH.dark, f_s));
	ui_build();
	if (last_state)
		apply_state(last_state);
	lv_obj_invalidate(lv_screen_active());
	lv_obj_invalidate(lv_layer_top());
	printf("tema: %s\n", TH.ad);
}

static void tema_sec(int t)
{
	if (t < 0 || t >= TEMA_N || t == tema)
		return;
	tema = t;
	br_save();
	lv_async_call(ui_rebuild_async, NULL);
}

static void tema_cb(lv_event_t *e)
{
	tema_sec((int)(uintptr_t)lv_event_get_user_data(e));
}

/* ------------------------------------------------------------ ekran koruyucu */
#define SS_MS (3 * 60 * 1000)
static lv_obj_t *ss_obj, *ss_clock, *ss_date;
static lv_obj_t *sw_ss;

static void ss_hide(void)
{
	if (ss_obj) {
		lv_obj_delete(ss_obj);
		ss_obj = NULL;
	}
	ss_shown = 0;
	br_cur = -1;
	br_apply();
}

static void ss_click_cb(lv_event_t *e)
{
	ss_hide();
	lv_display_trigger_activity(disp);
}

static void ss_update(void)
{
	time_t now = time(NULL);
	struct tm tm;

	if (!ss_obj)
		return;
	localtime_r(&now, &tm);
	lv_label_set_text_fmt(ss_clock, "%02d:%02d", tm.tm_hour, tm.tm_min);
	if (tm.tm_year > 120)
		lv_label_set_text_fmt(ss_date, "%d %s, %s", tm.tm_mday, aylar[tm.tm_mon], gunler[tm.tm_wday]);
	/* ekran izi kalmasin: her dakika farkli konum */
	int k = tm.tm_min % 8;
	static int last_k = -1;
	if (k != last_k) {
		last_k = k;
		lv_obj_align(lv_obj_get_parent(ss_clock), LV_ALIGN_CENTER, (int)((k % 4 - 1.5) * 220), (k / 4 ? 1 : -1) * 70);
		lv_obj_invalidate(ss_obj);
	}
}

static void ss_show(void)
{
	ss_obj = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(ss_obj);
	lv_obj_set_size(ss_obj, W, H);
	lv_obj_set_style_bg_opa(ss_obj, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(ss_obj, lv_color_black(), 0);
	lv_obj_add_flag(ss_obj, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ss_obj, ss_click_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_t *box = lv_obj_create(ss_obj);
	lv_obj_remove_style_all(box);
	lv_obj_set_size(box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(box, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(box, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_remove_flag(box, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_t *ssl = lv_image_create(box);
	lv_image_set_src(ssl, &logo_tam);
	lv_obj_set_style_image_opa(ssl, LV_OPA_50, 0);
	lv_image_set_scale(ssl, 160);	/* 256 = %100 */
	lv_obj_set_size(ssl, logo_tam.header.w * 160 / 256, logo_tam.header.h * 160 / 256);
	lv_image_set_inner_align(ssl, LV_IMAGE_ALIGN_CENTER);
	ss_clock = label(box, f_clock, "");
	lv_obj_set_style_text_color(ss_clock, lv_color_hex(0x9CA3AF), 0);
	ss_date = label(box, f_m, "");
	lv_obj_set_style_text_color(ss_date, lv_color_hex(0x6B7280), 0);
	ss_shown = 1;
	ss_update();
	br_cur = -1;
	br_apply();
}

static void ss_tick(void)
{
	if (ss_obj)
		ss_update();
	else if (ss_on && lv_display_get_inactive_time(disp) > SS_MS)
		ss_show();
}

static void ss_sw_cb(lv_event_t *e)
{
	ss_on = lv_obj_has_state(sw_ss, LV_STATE_CHECKED);
	br_save();
}

/* --------------------------------------------------------------- zamanlayici */
static lv_obj_t *tm_label, *tm_mode_l, *tm_arc, *tm_start_l;
static int tm_total = 25 * 60, tm_left = 25 * 60, tm_run;
static const char *tm_mode = "Odak";
static uint32_t tm_last_ms;

static void tm_draw(void)
{
	if (!tm_label)
		return;
	lv_label_set_text_fmt(tm_label, "%02d:%02d", tm_left / 60, tm_left % 60);
	lv_label_set_text_fmt(tm_mode_l, "%s · %d dk", tm_mode, tm_total / 60);
	lv_arc_set_value(tm_arc, tm_total ? (tm_total - tm_left) * 1000 / tm_total : 0);
	lv_label_set_text(tm_start_l, tm_run ? LV_SYMBOL_PAUSE "  Duraklat" : LV_SYMBOL_PLAY "  Başlat");
}

static void tm_set(const char *mode, int min)
{
	tm_mode = mode;
	tm_total = tm_left = min * 60;
	tm_run = 0;
	tm_draw();
}

static void tm_btn_cb(lv_event_t *e)
{
	int v = (int)(uintptr_t)lv_event_get_user_data(e);

	if (v == 25)
		tm_set("Odak", 25);
	else if (v == 5)
		tm_set("Mola", 5);
	else if (v == 15)
		tm_set("Uzun mola", 15);
	else if (v == 1) {	/* baslat/duraklat */
		if (!tm_left)
			tm_left = tm_total;
		tm_run = !tm_run;
		tm_last_ms = tick_ms();
	} else if (v == 0) {	/* sifirla */
		tm_left = tm_total;
		tm_run = 0;
	}
	tm_draw();
}

static void tm_tick(void)
{
	if (!tm_run)
		return;
	uint32_t now = tick_ms();
	while (now - tm_last_ms >= 1000 && tm_left > 0) {
		tm_left--;
		tm_last_ms += 1000;
	}
	if (tm_left == 0) {
		tm_run = 0;
		char ev[128];
		snprintf(ev, sizeof ev, "{\"t\":\"timer\",\"state\":\"done\",\"mode\":\"%s\"}", tm_mode);
		send_event(ev);
		if (ss_obj)
			ss_hide();
		lv_display_trigger_activity(disp);
		show_toast(LV_SYMBOL_BELL "  Süre doldu!");
	}
	tm_draw();
}

static lv_obj_t *tm_button(lv_obj_t *par, const char *txt, int v, int w)
{
	lv_obj_t *b = lv_button_create(par);
	lv_obj_set_size(b, w, 92);
	lv_obj_set_style_radius(b, 16, 0);
	lv_obj_set_style_bg_color(b, lv_color_hex(TH.btn), 0);
	lv_obj_set_style_bg_color(b, C_ACC, LV_STATE_PRESSED);
	lv_obj_t *l = label(b, f_mb, txt);
	lv_obj_center(l);
	lv_obj_add_event_cb(b, tm_btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)v);
	return l;
}

static void tm_build(lv_obj_t *t)
{
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t, 14, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *c = card(t, NULL);
	lv_obj_set_flex_grow(c, 5);
	lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	tm_arc = lv_arc_create(c);
	lv_obj_set_size(tm_arc, 400, 400);
	lv_arc_set_rotation(tm_arc, 270);
	lv_arc_set_bg_angles(tm_arc, 0, 360);
	lv_arc_set_range(tm_arc, 0, 1000);
	lv_obj_remove_style(tm_arc, NULL, LV_PART_KNOB);
	lv_obj_remove_flag(tm_arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_arc_width(tm_arc, 22, LV_PART_MAIN);
	lv_obj_set_style_arc_width(tm_arc, 22, LV_PART_INDICATOR);
	lv_obj_set_style_arc_color(tm_arc, C_ACC, LV_PART_INDICATOR);
	tm_label = label(tm_arc, f_xl, "");
	lv_obj_set_style_text_font(tm_label, f_clock, 0);
	lv_obj_align(tm_label, LV_ALIGN_CENTER, 0, -14);
	lv_obj_set_style_transform_scale(tm_label, 150, 0);	/* 150/256: halkaya sigsin */
	lv_obj_set_style_transform_pivot_x(tm_label, LV_PCT(50), 0);
	lv_obj_set_style_transform_pivot_y(tm_label, LV_PCT(50), 0);
	tm_mode_l = label(tm_arc, f_m, "");
	lv_obj_set_style_text_color(tm_mode_l, C_DIM, 0);
	lv_obj_align(tm_mode_l, LV_ALIGN_CENTER, 0, 88);

	lv_obj_t *r = card(t, "ZAMANLAYICI");
	lv_obj_set_flex_grow(r, 5);
	lv_obj_t *row = lv_obj_create(r);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 12, 0);
	tm_button(row, "Odak 25", 25, 220);
	tm_button(row, "Mola 5", 5, 200);
	tm_button(row, "Uzun 15", 15, 200);
	lv_obj_t *row2 = lv_obj_create(r);
	lv_obj_remove_style_all(row2);
	lv_obj_set_size(row2, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row2, 12, 0);
	lv_obj_set_style_pad_top(row2, 12, 0);
	tm_start_l = tm_button(row2, "", 1, 340);
	tm_button(row2, LV_SYMBOL_REFRESH "  Sıfırla", 0, 290);
	lv_obj_t *hint = label(r, f_s, "\nSüre dolunca panelde uyarı çıkar, PC ajanı bilgisayardan sesli bildirim verir.");
	lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(hint, LV_PCT(100));
	lv_obj_set_style_text_color(hint, C_DIM, 0);
	tm_draw();
}

/* ------------------------------------------------------------ Wi-Fi kurulumu */
#define WF_MAX 24
typedef struct { char ssid[64]; int sig; int sec; } wf_ag_t;
static wf_ag_t wf_list[WF_MAX];
static int wf_n, wf_state;	/* 0 bos, 1 taraniyor, 2 tarama bitti, 3 baglaniyor, 4 baglandi, 5 hata */
static char wf_msg[160];
static char wf_sel[64], wf_pw[96];
static lv_obj_t *wf_listobj, *wf_status, *wf_ta, *wf_sel_l, *wf_kb;
static int wf_drawn_state = -1;

static void sh_quote(char *o, size_t n, const char *in)
{
	size_t k = 0;
	if (n < 3)
		return;
	o[k++] = '\'';
	for (; *in && k + 5 < n; in++) {
		if (*in == '\'') {
			memcpy(o + k, "'\\''", 4);
			k += 4;
		} else
			o[k++] = *in;
	}
	o[k++] = '\'';
	o[k] = 0;
}

static void *wf_scan_thread(void *a)
{
	(void)a;
	if (system("lsmod | grep -q '^ssv6x5x' || insmod /lib/wifi/ssv6006.ko; i=0; "
	       "while [ ! -e /sys/class/net/wlan0 ] && [ $i -lt 8 ]; do sleep 1; i=$((i+1)); done; ip link set wlan0 up")) {
	}
	wf_ag_t tmp[WF_MAX];
	int n = 0, cur = -1;
	FILE *f = popen("iw dev wlan0 scan 2>/dev/null", "r");
	char line[256];
	while (f && fgets(line, sizeof line, f)) {
		char *q;
		if (!strncmp(line, "BSS ", 4)) {
			cur = n < WF_MAX ? n : -1;
			if (cur >= 0) {
				memset(&tmp[cur], 0, sizeof tmp[cur]);
				tmp[cur].sig = -100;
				n++;
			}
		} else if (cur >= 0 && (q = strstr(line, "signal:"))) {
			tmp[cur].sig = atoi(q + 7);
		} else if (cur >= 0 && (q = strstr(line, "SSID: "))) {
			q += 6;
			q[strcspn(q, "\r\n")] = 0;
			snprintf(tmp[cur].ssid, sizeof tmp[cur].ssid, "%s", q);
		} else if (cur >= 0 && (strstr(line, "RSN:") || strstr(line, "WPA:"))) {
			tmp[cur].sec = 1;
		}
	}
	if (f)
		pclose(f);
	/* ayni SSID'leri birlestir (en guclu), gizlileri at, sinyale gore sirala */
	wf_ag_t out[WF_MAX];
	int m = 0;
	for (int i = 0; i < n; i++) {
		if (!tmp[i].ssid[0])
			continue;
		int j;
		for (j = 0; j < m; j++)
			if (!strcmp(out[j].ssid, tmp[i].ssid))
				break;
		if (j == m)
			out[m++] = tmp[i];
		else if (tmp[i].sig > out[j].sig)
			out[j].sig = tmp[i].sig;
	}
	for (int i = 0; i < m; i++)
		for (int j = i + 1; j < m; j++)
			if (out[j].sig > out[i].sig) {
				wf_ag_t t = out[i];
				out[i] = out[j];
				out[j] = t;
			}
	pthread_mutex_lock(&mu);
	memcpy(wf_list, out, sizeof out);
	wf_n = m;
	wf_state = 2;
	snprintf(wf_msg, sizeof wf_msg, m ? "%d ağ bulundu. Bağlanmak için birine dokun." : "Ağ bulunamadı. Yenile'ye dokun.", m);
	pthread_mutex_unlock(&mu);
	return NULL;
}

static void *wf_connect_thread(void *a)
{
	char qs[200], qp[300], cmd[600];
	(void)a;
	sh_quote(qs, sizeof qs, wf_sel);
	sh_quote(qp, sizeof qp, wf_pw);
	snprintf(cmd, sizeof cmd, "wifi-kaydet %s %s > /tmp/wifi-kaydet.log 2>&1", qs, qp);
	memset(wf_pw, 0, sizeof wf_pw);
	if (system(cmd)) {
	}
	memset(cmd, 0, sizeof cmd);
	memset(qp, 0, sizeof qp);
	int ok = 0;
	for (int i = 0; i < 25 && !ok; i++) {
		sleep(1);
		ok = system("ip -4 addr show wlan0 | grep -q inet") == 0;
	}
	pthread_mutex_lock(&mu);
	wf_state = ok ? 4 : 5;
	snprintf(wf_msg, sizeof wf_msg, ok ? "Bağlandı: %s (kaydedildi)" : "Bağlanamadı: %s. Şifreyi kontrol et.", wf_sel);
	pthread_mutex_unlock(&mu);
	return NULL;
}

static void wf_start_scan(void)
{
	pthread_t t;
	pthread_mutex_lock(&mu);
	wf_state = 1;
	snprintf(wf_msg, sizeof wf_msg, "Taranıyor...");
	pthread_mutex_unlock(&mu);
	pthread_create(&t, NULL, wf_scan_thread, NULL);
	pthread_detach(t);
}

static void wf_close_cb(lv_event_t *e)
{
	if (wf_win) {
		lv_obj_delete(wf_win);
		wf_win = NULL;
	}
}

static void wf_rescan_cb(lv_event_t *e)
{
	if (wf_state != 1 && wf_state != 3)
		wf_start_scan();
}

static void wf_pick_cb(lv_event_t *e)
{
	int i = (int)(uintptr_t)lv_event_get_user_data(e);
	pthread_mutex_lock(&mu);
	if (i >= 0 && i < wf_n)
		snprintf(wf_sel, sizeof wf_sel, "%s", wf_list[i].ssid);
	pthread_mutex_unlock(&mu);
	lv_label_set_text_fmt(wf_sel_l, LV_SYMBOL_WIFI "  %s", wf_sel);
	lv_textarea_set_text(wf_ta, "");
	lv_obj_remove_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
}

static void wf_connect_cb(lv_event_t *e)
{
	pthread_t t;
	if (!wf_sel[0] || wf_state == 3)
		return;
	snprintf(wf_pw, sizeof wf_pw, "%s", lv_textarea_get_text(wf_ta));
	if (strlen(wf_pw) < 8) {
		lv_label_set_text(wf_status, "Şifre en az 8 karakter olmalı (WPA2).");
		return;
	}
	pthread_mutex_lock(&mu);
	wf_state = 3;
	snprintf(wf_msg, sizeof wf_msg, "Bağlanıyor: %s ...", wf_sel);
	pthread_mutex_unlock(&mu);
	lv_obj_add_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
	pthread_create(&t, NULL, wf_connect_thread, NULL);
	pthread_detach(t);
}

static void wf_kb_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	if (c == LV_EVENT_READY)
		wf_connect_cb(e);
	else if (c == LV_EVENT_CANCEL)
		lv_obj_add_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t *wf_btn(lv_obj_t *par, const char *t, lv_event_cb_t cb)
{
	lv_obj_t *b = lv_button_create(par);
	lv_obj_set_height(b, 56);
	lv_obj_set_style_bg_color(b, lv_color_hex(TH.btn), 0);
	lv_obj_set_style_bg_color(b, C_ACC, LV_STATE_PRESSED);
	lv_obj_t *l = label(b, f_mb, t);
	lv_obj_center(l);
	lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
	return b;
}

static void wf_open_cb(lv_event_t *e)
{
	if (wf_win)
		return;
	wf_win = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(wf_win);
	lv_obj_set_size(wf_win, W, H);
	lv_obj_set_style_bg_opa(wf_win, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(wf_win, C_BG, 0);
	lv_obj_set_style_text_color(wf_win, lv_color_hex(TH.text), 0);
	lv_obj_set_style_pad_all(wf_win, 12, 0);
	lv_obj_add_flag(wf_win, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_t *hdr = lv_obj_create(wf_win);
	lv_obj_remove_style_all(hdr);
	lv_obj_set_size(hdr, LV_PCT(100), 60);
	lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	label(hdr, f_l, LV_SYMBOL_WIFI "  Wi-Fi ağı");
	lv_obj_t *hb = lv_obj_create(hdr);
	lv_obj_remove_style_all(hb);
	lv_obj_set_size(hb, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(hb, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(hb, 12, 0);
	wf_btn(hb, LV_SYMBOL_REFRESH "  Yenile", wf_rescan_cb);
	wf_btn(hb, LV_SYMBOL_CLOSE "  Kapat", wf_close_cb);

	lv_obj_t *row = lv_obj_create(wf_win);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, LV_PCT(100), 220);
	lv_obj_set_y(row, 68);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 12, 0);
	wf_listobj = lv_list_create(row);
	lv_obj_set_size(wf_listobj, 720, LV_PCT(100));
	lv_obj_set_style_bg_color(wf_listobj, C_CARD, 0);
	lv_obj_set_style_border_width(wf_listobj, 0, 0);
	lv_obj_t *rp = card(row, NULL);
	wf_sel_l = label(rp, f_mb, "Bir ağ seç");
	wf_ta = lv_textarea_create(rp);
	lv_obj_set_width(wf_ta, LV_PCT(100));
	lv_textarea_set_one_line(wf_ta, true);
	lv_textarea_set_password_mode(wf_ta, true);
	lv_textarea_set_placeholder_text(wf_ta, "Şifre");
	lv_obj_set_style_text_font(wf_ta, f_m, 0);
	lv_obj_t *br = lv_obj_create(rp);
	lv_obj_remove_style_all(br);
	lv_obj_set_size(br, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(br, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(br, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	wf_status = label(br, f_s, "");
	lv_label_set_long_mode(wf_status, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(wf_status, LV_PCT(65));
	wf_btn(br, LV_SYMBOL_OK "  Bağlan", wf_connect_cb);

	wf_kb = lv_keyboard_create(wf_win);
	lv_obj_set_size(wf_kb, LV_PCT(100), 290);
	lv_obj_align(wf_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_text_font(wf_kb, f_mb, 0);
	lv_keyboard_set_textarea(wf_kb, wf_ta);
	lv_obj_add_event_cb(wf_kb, wf_kb_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
	wf_drawn_state = -1;
	wf_start_scan();
}

/* zamanlayicidan: is parcaciginin sonucunu pencereye yansit */
static void wf_tick(void)
{
	if (!wf_win)
		return;
	pthread_mutex_lock(&mu);
	int st = wf_state;
	char msg[160];
	snprintf(msg, sizeof msg, "%s", wf_msg);
	wf_ag_t l[WF_MAX];
	int n = wf_n;
	memcpy(l, wf_list, sizeof l);
	pthread_mutex_unlock(&mu);
	lv_label_set_text(wf_status, msg);
	if (st == wf_drawn_state)
		return;
	wf_drawn_state = st;
	if (st == 2) {
		lv_obj_clean(wf_listobj);
		for (int i = 0; i < n; i++) {
			char t[100];
			snprintf(t, sizeof t, "%s   %d dBm%s", l[i].ssid, l[i].sig, l[i].sec ? "" : "  (açık)");
			lv_obj_t *b = lv_list_add_button(wf_listobj, LV_SYMBOL_WIFI, t);
			lv_obj_set_style_text_font(b, f_m, 0);
			lv_obj_set_height(b, 56);
			lv_obj_add_event_cb(b, wf_pick_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
		}
	}
}

static void br_mode_cb(lv_event_t *e)
{
	br_auto = lv_obj_has_state(sw_auto, LV_STATE_CHECKED);
	if (br_auto)
		lv_obj_add_flag(br_box, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_remove_flag(br_box, LV_OBJ_FLAG_HIDDEN);
	br_save();
	br_apply();
}

static void br_slider_cb(lv_event_t *e)
{
	br_manual = lv_slider_get_value(sl_br);
	lv_label_set_text_fmt(l_br_pct, "%%%d", br_manual);
	br_apply();
	if (lv_event_get_code(e) == LV_EVENT_RELEASED)
		br_save();
}

/* ---------------------------------------------- kablosuz PC ekrani (0.14, pc_shm_v1.h) */
/* Alici (wifi-ekran-v4) JPEG'leri /tmp/pc-ekran.shm'deki 1600x600 RGB565 kareye cozer; panel ayni bellegi
 * kopyasiz LVGL goruntu kaynagi yapar ve yalniz bildirilen kirli dikdortgenleri yeniden cizer.
 * Tam ekran: ekranin ustunde 1600x600 goruntu; dokununca ust cubuk (panele don). Akis baslayinca (PC
 * "tam ekran iste" bayragiyla) otomatik acilir; kullanici kapattiysa ayni baglantida yeniden acilmaz. */
static void pc_bar_show(uint32_t ms);

#define PC_DG_DOSYA "/tmp/pc-dogrudan"
/* 0.18: {nesil, izin} yaz; izin degismediyse dokunma. izin=0 aninda sahiplik birakilmis sayilir. */
static void pc_dg_yaz(int izin)
{
	if (izin == pc_dg_izin)
		return;
	if (pc_dg_fd < 0) {
		/* silme/kesme yok: alicinin acik tanimlayicisi ayni dosyayi okumaya devam etsin */
		pc_dg_fd = open(PC_DG_DOSYA, O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0644);
		if (pc_dg_fd < 0)
			return;
	}
	pc_dg_gen++;
	uint32_t k[2] = { pc_dg_gen, (uint32_t)izin };
	int eski = pc_dg_izin;
	if (pwrite(pc_dg_fd, k, sizeof k, 0) == (ssize_t)sizeof k)
		pc_dg_izin = izin;
	if (!izin) {
		/* alici sahipken birakiliyorsa onay gelene kadar bekle (eski ortak bellek cizilmesin) */
		pc_dg_bekle = pc_rv_sahip || (eski == 1 && pc_shm.h && pc_shm.h->pad[1] == 1);
		pc_dg_t0 = tick_ms();
		pc_rv_sahip = 0;
	}
}

static void pc_full_exit(void)
{
	pc_dg_yaz(0);
	if (pc_full) {
		lv_obj_delete(pc_full);
		pc_full = pc_full_img = pc_bar = l_pc_bar = NULL;
		lv_timer_set_period(lv_display_get_refr_timer(disp), LV_DEF_REFR_PERIOD);
		lv_obj_invalidate(lv_screen_active());
	}
}

static void pc_full_back_cb(lv_event_t *e)
{
	(void)e;
	if (pc_shm.h) {
		pc_dismiss_gen = pc_shm.gen;
		pc_dismiss_conn = pc_shm.h->conn;
		pc_dismissed = 1;
	}
	pc_full_exit();
}

/* 0.15: tam ekranda dokunma -> PC faresi. Kayitlar FIFO /tmp/pc-dokunma'ya (alici wifi-ekran-v6 TCP ile PC'ye
 * iletir): 8 B = tip(1 bas, 2 surukle, 3 birak), 0, x(le16), y(le16), 0. Ust kenar (y < 28) dokunusu PC'ye gitmez,
 * "Panele don" cubugunu acar. Alici yoksa yazma sessizce atlanir (O_NONBLOCK, SIGPIPE yok sayiliyor). */
static int pc_dok_fd = -1, pc_dok_kenar;
static uint32_t pc_dok_son, pc_dok_sayac;

static void pc_dok_yaz(int tip, int x, int y)
{
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= W) x = W - 1;
	if (y >= H) y = H - 1;
	if (pc_dok_fd < 0)
		pc_dok_fd = open("/tmp/pc-dokunma", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (pc_dok_fd < 0)
		return;
	uint8_t m[8] = { (uint8_t)tip, 0, (uint8_t)(x & 255), (uint8_t)(x >> 8), (uint8_t)(y & 255), (uint8_t)(y >> 8), 0, 0 };
	if (write(pc_dok_fd, m, sizeof m) == (ssize_t)sizeof m)
		pc_dok_sayac++;
	else if (errno != EAGAIN) {
		close(pc_dok_fd);	/* alici yeniden basladi (FIFO yenilendi): sonraki dokunusta yeniden ac */
		pc_dok_fd = -1;
	}
}

static void pc_full_tap_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_point_t p;
	lv_indev_t *in = lv_indev_active();
	if (!in)
		return;
	lv_indev_get_point(in, &p);
	uint32_t now = tick_ms();
	if (code == LV_EVENT_PRESSED) {
		pc_dok_kenar = p.y < 28;
		if (pc_dok_kenar) {
			pc_bar_show(5000);
			return;
		}
		pc_dok_yaz(1, p.x, p.y);
		pc_dok_son = now;
	} else if (code == LV_EVENT_PRESSING) {
		if (!pc_dok_kenar && (uint32_t)(now - pc_dok_son) >= 30) {
			pc_dok_yaz(2, p.x, p.y);
			pc_dok_son = now;
		}
	} else if (code == LV_EVENT_RELEASED) {
		if (!pc_dok_kenar)
			pc_dok_yaz(3, p.x, p.y);
	}
}

static void pc_bar_show(uint32_t ms)
{
	if (!pc_bar)
		return;
	pc_dg_yaz(0);	/* cubuk LVGL ile cizilecek: fb'yi geri al */
	lv_obj_remove_flag(pc_bar, LV_OBJ_FLAG_HIDDEN);
	pc_bar_until = tick_ms() + ms;
}

static void pc_full_enter(void)
{
	if (pc_full || !pc_shm.h)
		return;
	if (ss_obj)
		ss_hide();
	lv_display_trigger_activity(disp);
	pc_full = lv_obj_create(lv_screen_active());
	lv_obj_remove_style_all(pc_full);	/* arka plan saydam: goruntu tamamen ortuyor, cift cizim yok */
	lv_obj_set_size(pc_full, W, H);
	lv_obj_set_pos(pc_full, 0, 0);
	lv_obj_add_flag(pc_full, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_remove_flag(pc_full, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(pc_full, pc_full_tap_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_add_event_cb(pc_full, pc_full_tap_cb, LV_EVENT_PRESSING, NULL);
	lv_obj_add_event_cb(pc_full, pc_full_tap_cb, LV_EVENT_RELEASED, NULL);
	pc_full_img = lv_image_create(pc_full);
	lv_image_set_src(pc_full_img, &pc_dsc);
	lv_obj_set_pos(pc_full_img, 0, 0);
	pc_bar = lv_obj_create(pc_full);
	lv_obj_remove_style_all(pc_bar);
	lv_obj_set_size(pc_bar, W, 64);
	lv_obj_set_style_bg_color(pc_bar, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(pc_bar, LV_OPA_70, 0);
	lv_obj_set_style_pad_hor(pc_bar, 16, 0);
	lv_obj_remove_flag(pc_bar, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(pc_bar, LV_OBJ_FLAG_CLICKABLE);	/* cubuga dokunus PC'ye gitmez */
	lv_obj_t *b = lv_button_create(pc_bar);
	lv_obj_set_size(b, 220, 48);
	lv_obj_align(b, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_add_event_cb(b, pc_full_back_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t *bl = label(b, f_mb, LV_SYMBOL_LEFT "  Panele dön");
	lv_obj_center(bl);
	l_pc_bar = label(pc_bar, f_s, "");
	lv_obj_set_style_text_color(l_pc_bar, lv_color_white(), 0);
	lv_obj_align(l_pc_bar, LV_ALIGN_RIGHT_MID, 0, 0);
	pc_bar_show(3000);
	lv_timer_set_period(lv_display_get_refr_timer(disp), 15);
}

static void pc_full_btn_cb(lv_event_t *e)
{
	(void)e;
	pc_dismissed = 0;
	pc_full_enter();
}

static void pc_attach(void)
{
	pc_dsc = (lv_image_dsc_t){ .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
					       .w = PC_SHM_W, .h = PC_SHM_H, .stride = PC_SHM_W * 2 },
				   .data_size = PC_SHM_W * PC_SHM_H * 2, .data = pc_shm.px };
	lv_image_cache_drop(&pc_dsc);
	if (pc_image) {
		lv_image_set_src(pc_image, &pc_dsc);
		lv_obj_remove_flag(pc_image, LV_OBJ_FLAG_HIDDEN);
	}
	printf("PC ekrani: ortak bellek baglandi (nesil %u)\n", pc_shm.gen);
}

/* Esleme kaldirilmadan ONCE butun LVGL baglantilari kesilir (silinmis bellegi cizme riski yok). */
static void pc_detach(void)
{
	pc_dg_yaz(0);
	pc_full_exit();
	if (pc_image) {
		lv_image_set_src(pc_image, NULL);
		lv_obj_add_flag(pc_image, LV_OBJ_FLAG_HIDDEN);
	}
	lv_image_cache_drop(&pc_dsc);
	pc_shm_close(&pc_shm);
	printf("PC ekrani: ortak bellek birakildi\n");
}

/* 0.17: RGB565 (ortak bellek) -> XRGB8888 (fb0), bit cogaltmali (565 -> 888 tam aralik). */
static inline uint32_t pc_565_8888(uint32_t p)
{
	return 0xFF000000u | ((p & 0xF800u) << 8) | ((p & 0xE000u) << 3) | ((p & 0x07E0u) << 5) |
	       ((p & 0x0600u) >> 1) | ((p & 0x1Fu) << 3) | ((p & 0x1Cu) >> 2);
}

static void pc_dc_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	const uint16_t *src = (const uint16_t *)pc_shm.px;
	if (!fb_rot) {
		for (uint32_t j = 0; j < h; j++) {
			const uint16_t *s = src + (size_t)(y + j) * PC_SHM_W + x;
			uint32_t *d = (uint32_t *)(fbm + (size_t)(y + j) * fb_line) + x;
			uint32_t i = 0;
			if (((uintptr_t)d & 7u) && w) {
				d[0] = pc_565_8888(s[0]);
				i = 1;
			}
			uint64_t *d64 = (uint64_t *)(d + i);
			for (; i + 1 < w; i += 2)
				*d64++ = (uint64_t)pc_565_8888(s[i]) | (uint64_t)pc_565_8888(s[i + 1]) << 32;
			if (i < w)
				d[i] = pc_565_8888(s[i]);
		}
	} else {	/* yazilim 90 derece (G2D'siz cekirdek): mantiksal (x,y) -> fiziksel (xres-1-y, x) */
		for (uint32_t j = 0; j < h; j++) {
			const uint16_t *s = src + (size_t)(y + j) * PC_SHM_W + x;
			uint32_t col = fb_xres - 1 - (y + j);
			for (uint32_t i = 0; i < w; i++)
				((uint32_t *)(fbm + (size_t)(x + i) * fb_line))[col] = pc_565_8888(s[i]);
		}
	}
	pc_dc_px += (uint64_t)w * h;
}

static int pc_katman_gorunur(lv_obj_t *k)
{
	uint32_t n = lv_obj_get_child_count(k);
	for (uint32_t i = 0; i < n; i++)
		if (!lv_obj_has_flag(lv_obj_get_child(k, (int32_t)i), LV_OBJ_FLAG_HIDDEN))
			return 1;
	return 0;
}

/* 0.17-v3: 16x16 karo haritasi; her karo bir kez cizilir, ayni satirdaki bitisik karolar tek parca. */
#define PC_KARO 16u
#define PC_KX (PC_SHM_W / PC_KARO)
#define PC_KY ((PC_SHM_H + PC_KARO - 1) / PC_KARO)
static uint8_t pc_karo[PC_KY][PC_KX];

static void pc_dc_karolar(uint16_t r[][4], int n)
{
	memset(pc_karo, 0, sizeof pc_karo);
	for (int i = 0; i < n; i++) {
		uint32_t x0 = r[i][0] / PC_KARO, y0 = r[i][1] / PC_KARO;
		uint32_t x1 = (r[i][0] + r[i][2] - 1u) / PC_KARO, y1 = (r[i][1] + r[i][3] - 1u) / PC_KARO;
		if (x1 >= PC_KX)
			x1 = PC_KX - 1;
		for (uint32_t ty = y0; ty <= y1 && ty < PC_KY; ty++)
			memset(&pc_karo[ty][x0], 1, x1 - x0 + 1);
	}
	for (uint32_t ty = 0; ty < PC_KY; ty++) {
		uint32_t y = ty * PC_KARO, h = y + PC_KARO > PC_SHM_H ? PC_SHM_H - y : PC_KARO;
		for (uint32_t tx = 0; tx < PC_KX;) {
			if (!pc_karo[ty][tx]) {
				tx++;
				continue;
			}
			uint32_t b = tx;
			while (tx < PC_KX && pc_karo[ty][tx])
				tx++;
			pc_dc_rect(b * PC_KARO, y, (tx - b) * PC_KARO, h);
		}
	}
}

/* 1: dogrudan cizildi; 0: LVGL yolu kullanilmali. n < 0: tam ekran. */
static int pc_dc_ciz(uint16_t r[][4], int n)
{
	if (!pc_dc_izin || !pc_full_img || !pc_shm.h || !fbm)
		return 0;
	if (pc_bar && !lv_obj_has_flag(pc_bar, LV_OBJ_FLAG_HIDDEN))
		return 0;
	if (pc_katman_gorunur(lv_layer_top()) || pc_katman_gorunur(lv_layer_sys()))
		return 0;	/* bildirim/ust katman acik: LVGL ciziyor */
	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	if (n < 0)
		pc_dc_rect(0, 0, PC_SHM_W, PC_SHM_H);
	else
		pc_dc_karolar(r, n);
	{
		struct timespec tc;
		clock_gettime(CLOCK_MONOTONIC, &tc);
		pc_dc_cv_us += elapsed_us(&t0, &tc);
	}
	if (fb_hw_pan) {
		struct timespec p0, p1;
		clock_gettime(CLOCK_MONOTONIC, &p0);
		int rr = ioctl(fb_fd, FBIOPAN_DISPLAY, &fb_var);
		clock_gettime(CLOCK_MONOTONIC, &p1);
		pc_pan_calls++;
		pc_pan_us += elapsed_us(&p0, &p1);
		if (rr < 0) {
			pc_pan_errors++;
			if (pc_pan_errors <= 2) perror("G2D FBIOPAN_DISPLAY (dogrudan)");
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);
	pc_dc_n++;
	pc_dc_us += elapsed_us(&t0, &t1);
	return 1;
}

static void pc_image_tick(lv_timer_t *timer)
{
	(void)timer;
	static uint32_t t_try, t_check, t_prev, t_label, t_report;
	uint32_t now = tick_ms();
	if (!pc_shm.h) {
		if ((uint32_t)(now - t_try) >= 500) {
			t_try = now;
			if (pc_shm_open(&pc_shm, PC_SHM_PATH) == 1)
				pc_attach();
		}
	} else if ((uint32_t)(now - t_check) >= 500) {
		t_check = now;
		if (!pc_shm_still_current(&pc_shm, PC_SHM_PATH))
			pc_detach();
	}
	int alive = pc_shm.h && pc_shm_alive(&pc_shm, now);
	int tab5 = tabview && lv_tabview_get_tab_active(tabview) == 5;
	pc_view_active = pc_full || tab5;
	if (alive) {
		pc_last_alive = now;
		if (pc_dismissed && (pc_dismiss_gen != pc_shm.gen || pc_dismiss_conn != pc_shm.h->conn))
			pc_dismissed = 0;
		if (pc_shm.h->want_full && !pc_full && !pc_dismissed)
			pc_full_enter();
	}
	if (pc_dg_acik) {
		int ist = pc_dc_izin && pc_full_img && pc_shm.h && fbm && (fb_hw_pan || fb_rot) && alive &&
			  !(pc_bar && !lv_obj_has_flag(pc_bar, LV_OBJ_FLAG_HIDDEN)) &&
			  !pc_katman_gorunur(lv_layer_top()) && !pc_katman_gorunur(lv_layer_sys());
		pc_dg_yaz(ist);
		pc_rv_sahip = ist && pc_dg_izin == 1 && pc_shm.h->pad[0] == pc_dg_gen && pc_shm.h->pad[1] == 1;
		if (pc_dg_bekle && (!pc_shm.h || (pc_shm.h->pad[0] == pc_dg_gen && pc_shm.h->pad[1] == 0) ||
				    (uint32_t)(now - pc_dg_t0) > 400)) {
			pc_dg_bekle = 0;
			pc_dg_bekleme++;
			if (pc_full_img)
				lv_obj_invalidate(pc_full_img);	/* esitlenmis ortak bellekten tam yeniden ciz */
		}
	}
	if (pc_shm.h) {
		uint16_t r[32][4];
		int n = pc_shm_take(&pc_shm, r, 32);
		if (n && (pc_rv_sahip || pc_dg_bekle)) {
			pc_updates++;
			pc_rv_rects += n > 0 ? (uint32_t)n : 1u;
		} else if (n) {
			pc_updates++;
			if (pc_full_img && pc_dc_ciz(r, n)) {
				if (n < 0)
					pc_inv_full++;
				else
					pc_inv_rects += (uint32_t)n;
			} else if (pc_full_img) {
				if (n < 0) {
					lv_obj_invalidate(pc_full_img);
					pc_inv_full++;
				} else
					for (int i = 0; i < n; i++) {
						lv_area_t a = { r[i][0], r[i][1], r[i][0] + r[i][2] - 1, r[i][1] + r[i][3] - 1 };
						lv_obj_invalidate_area(pc_full_img, &a);
						pc_inv_rects++;
					}
			} else if (tab5 && pc_image && (uint32_t)(now - t_prev) >= 250) {
				t_prev = now;	/* olcekli onizleme pahali: en fazla 4 Hz */
				lv_obj_invalidate(pc_image);
			}
		}
	}
	if (pc_full) {
		if (alive)
			lv_display_trigger_activity(disp);	/* izlerken ekran koruyucu acilmasin */
		else {
			pc_bar_show(1000);
			if ((uint32_t)(now - pc_last_alive) > 10000)
				pc_full_exit();	/* akis 10 sn durdu: panele don */
		}
		if (pc_bar && (int32_t)(now - pc_bar_until) > 0 && alive)
			lv_obj_add_flag(pc_bar, LV_OBJ_FLAG_HIDDEN);
	}
	if ((uint32_t)(now - t_label) >= 1000) {
		t_label = now;
		char s[160];
		if (!pc_shm.h)
			snprintf(s, sizeof s, "PC görüntüsü bekleniyor (alıcı kapalı)");
		else if (!alive)
			snprintf(s, sizeof s, "%s", pc_shm.h->frames ? "Görüntü akışı durdu" : "Alıcı hazır, PC bekleniyor");
		else
			snprintf(s, sizeof s, "Kablosuz · 1600 × 600 · %u.%u fps · %u kbit/s · çözme %u ms",
				 pc_shm.h->fps_x10 / 10, pc_shm.h->fps_x10 % 10, pc_shm.h->kbps, pc_shm.h->last_decode_us / 1000);
		if (l_pc_image)
			lv_label_set_text(l_pc_image, s);
		if (l_pc_bar)
			lv_label_set_text(l_pc_bar, s);
	}
	if ((uint32_t)(now - t_report) >= 1000) {
		t_report = now;
		FILE *f = fopen("/tmp/pc-ekran-durum.json.part", "w");
		if (f) {
			fprintf(f, "{\"ver\":\"%s\",\"attached\":%d,\"alive\":%d,\"full\":%d,\"tab5\":%d,\"conn\":%u,"
				"\"rx_frames\":%u,\"rx_errors\":%u,\"fps_x10\":%u,\"kbps\":%u,\"decode_us\":%u,"
				"\"updates\":%u,\"inv_rects\":%u,\"inv_full\":%u,\"render_n\":%u,\"render_avg_us\":%u,"
				"\"render_max_us\":%u,\"flush_us\":%llu,\"flush_calls\":%llu,\"touches\":%u,"
				"\"g2d_pan\":%d,\"pan_us\":%llu,\"pan_calls\":%llu,\"pan_errors\":%llu,"
				"\"direct\":%d,\"dc_n\":%llu,\"dc_us\":%llu,\"dc_px\":%llu,\"dc_cv_us\":%llu,"
				"\"dg_izin\":%d,\"rv_sahip\":%d,\"dg_bekleme\":%u,\"rv_rects\":%u,\"rv_g2d_n\":%u,\"rv_g2d_us\":%u,"
				"\"rv_pan_n\":%u,\"rv_pan_us\":%u,\"rv_cpu\":%u,\"rv_esit\":%u}\n",
				PANEL_VER, !!pc_shm.h, alive, !!pc_full, tab5, pc_shm.h ? pc_shm.h->conn : 0,
				pc_shm.h ? pc_shm.h->frames : 0, pc_shm.h ? pc_shm.h->errors : 0,
				pc_shm.h ? pc_shm.h->fps_x10 : 0, pc_shm.h ? pc_shm.h->kbps : 0,
				pc_shm.h ? pc_shm.h->last_decode_us : 0, pc_updates, pc_inv_rects, pc_inv_full, pc_render_n,
				pc_render_n ? (uint32_t)(pc_render_sum / pc_render_n) : 0, pc_render_max,
				(unsigned long long)pc_flush_us, (unsigned long long)pc_flush_calls, pc_dok_sayac, fb_hw_pan,
				(unsigned long long)pc_pan_us, (unsigned long long)pc_pan_calls, (unsigned long long)pc_pan_errors,
				pc_dc_izin, (unsigned long long)pc_dc_n, (unsigned long long)pc_dc_us, (unsigned long long)pc_dc_px, (unsigned long long)pc_dc_cv_us,
				pc_dg_izin, pc_rv_sahip, pc_dg_bekleme, pc_rv_rects,
				pc_shm.h ? pc_shm.h->pad[2] : 0, pc_shm.h ? pc_shm.h->pad[3] : 0, pc_shm.h ? pc_shm.h->pad[4] : 0,
				pc_shm.h ? pc_shm.h->pad[5] : 0, pc_shm.h ? pc_shm.h->pad[6] : 0, pc_shm.h ? pc_shm.h->pad[7] : 0);
			fclose(f);
			rename("/tmp/pc-ekran-durum.json.part", "/tmp/pc-ekran-durum.json");
		}
		pc_render_max = 0;	/* en yuksek deger son 1 sn icin */
	}
}

static void ui_build(void)
{
	lv_obj_t *scr = lv_screen_active();

	lv_obj_set_style_bg_color(scr, C_BG, 0);
	lv_obj_set_style_text_color(scr, lv_color_hex(TH.text), 0);

	/* ust serit */
	lv_obj_t *hdr = lv_obj_create(scr);
	lv_obj_remove_style_all(hdr);
	lv_obj_set_size(hdr, W, 56);
	lv_obj_set_style_pad_hor(hdr, 24, 0);
	lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *brand = lv_obj_create(hdr);
	lv_obj_remove_style_all(brand);
	lv_obj_set_size(brand, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(brand, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(brand, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(brand, 0, 0);
	lv_obj_t *am = lv_image_create(brand);
	lv_image_set_src(am, &amblem_40);
	lv_obj_set_style_image_recolor(am, lv_color_hex(TH.acc2), 0);
	lv_obj_set_style_pad_right(am, 10, 0);
	label(brand, f_mb, "Ve");
	lv_obj_t *bl = label(brand, f_mb, "Linux");
	lv_obj_set_style_text_color(bl, lv_color_hex(TH.acc2), 0);
	lv_obj_t *bk = label(brand, f_s, "KÖLE");
	lv_obj_set_style_text_color(bk, C_DIM, 0);
	lv_obj_set_style_text_letter_space(bk, 4, 0);
	lv_obj_set_style_pad_left(bk, 12, 0);
	l_hdr_pc = label(hdr, f_s, LV_SYMBOL_USB "  PC bekleniyor");
	l_hdr_date = label(hdr, f_s, "");
	l_hdr_net = label(hdr, f_s, LV_SYMBOL_WIFI "  -");

	/* sekmeler (alt) */
	lv_obj_t *tv = tabview = lv_tabview_create(scr);
	lv_obj_set_size(tv, W, H - 56);
	lv_obj_set_y(tv, 56);
	lv_obj_set_style_bg_color(tv, C_BG, 0);
	lv_tabview_set_tab_bar_position(tv, LV_DIR_BOTTOM);
	lv_tabview_set_tab_bar_size(tv, 64);
	lv_obj_t *bar = lv_tabview_get_tab_bar(tv);
	lv_obj_set_style_text_font(bar, f_mb, 0);
	lv_obj_set_style_bg_color(bar, lv_color_hex(TH.bar), 0);
	lv_obj_t *t1 = lv_tabview_add_tab(tv, LV_SYMBOL_HOME " Pano");
	lv_obj_t *t2 = lv_tabview_add_tab(tv, LV_SYMBOL_KEYBOARD " Kısayollar");
	lv_obj_t *t3 = lv_tabview_add_tab(tv, LV_SYMBOL_BELL " Asistan");
	lv_obj_t *t5 = lv_tabview_add_tab(tv, LV_SYMBOL_LOOP " Zamanlayıcı");
	lv_obj_t *t4 = lv_tabview_add_tab(tv, LV_SYMBOL_SETTINGS " Sistem");
	lv_obj_t *t6 = lv_tabview_add_tab(tv, LV_SYMBOL_IMAGE " PC Ekranı");
	lv_obj_t *tabs[] = {t1, t2, t3, t5, t4, t6};
	for (int i = 0; i < 6; i++) {
		lv_obj_set_style_pad_all(tabs[i], 14, 0);
		lv_obj_set_style_bg_color(tabs[i], C_BG, 0);
	}
	lv_obj_remove_flag(t6,LV_OBJ_FLAG_SCROLLABLE);
	l_pc_image=label(t6,f_s,"PC görüntüsü bekleniyor");
	lv_obj_align(l_pc_image,LV_ALIGN_TOP_LEFT,0,2);
	lv_obj_t *pcb=lv_button_create(t6);
	lv_obj_set_size(pcb,300,56);
	lv_obj_align(pcb,LV_ALIGN_TOP_RIGHT,0,0);
	lv_obj_add_event_cb(pcb,pc_full_btn_cb,LV_EVENT_CLICKED,NULL);
	lv_obj_center(label(pcb,f_mb,LV_SYMBOL_IMAGE "  Tam ekran"));
	/* onizleme: 1600x600 kaynak 800x300'e olcekli (LVGL donusumu, en fazla 4 Hz yenilenir) */
	pc_image=lv_image_create(t6);
	lv_obj_set_size(pc_image,800,300);
	lv_image_set_inner_align(pc_image,LV_IMAGE_ALIGN_STRETCH);
	lv_obj_align(pc_image,LV_ALIGN_BOTTOM_MID,0,-4);
	if (pc_shm.h) lv_image_set_src(pc_image,&pc_dsc);
	else lv_obj_add_flag(pc_image,LV_OBJ_FLAG_HIDDEN);

	/* Pano: saat | bilgisayar | piyasa+takvim */
	lv_obj_set_flex_flow(t1, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t1, 14, 0);
	lv_obj_remove_flag(t1, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *c1 = card(t1, NULL);
	lv_obj_set_flex_grow(c1, 5);
	lv_obj_set_flex_align(c1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *crow = lv_obj_create(c1);
	lv_obj_remove_style_all(crow);
	lv_obj_set_size(crow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(crow, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(crow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
	l_clock = label(crow, f_clock, "--:--");
	l_sec = label(crow, f_l, "");
	lv_obj_set_style_text_color(l_sec, C_DIM, 0);
	lv_obj_set_style_pad_bottom(l_sec, 26, 0);
	l_date = label(c1, f_m, "");
	lv_label_set_long_mode(l_date, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(l_date, LV_PCT(100));
	lv_obj_set_style_text_align(l_date, LV_TEXT_ALIGN_CENTER, 0);
	l_weather = label(c1, f_l, "");
	l_weather2 = label(c1, f_s, "Hava durumu PC'den gelecek");
	lv_obj_set_style_text_color(l_weather2, C_DIM, 0);

	lv_obj_t *c2 = card(t1, "BİLGİSAYAR");
	lv_obj_set_flex_grow(c2, 4);
	l_pcname = label(c2, f_l, "Bağlı değil");
	l_cpu = metric(c2, "İşlemci", &b_cpu);
	l_ram = metric(c2, "Bellek", &b_ram);
	l_disk = metric(c2, "Disk", &b_disk);
	l_netio = label(c2, f_m, LV_SYMBOL_DOWNLOAD " -   " LV_SYMBOL_UPLOAD " -");
	l_pcup = label(c2, f_s, "");
	lv_obj_set_style_text_color(l_pcup, C_DIM, 0);

	lv_obj_t *c3 = card(t1, "PİYASA & TAKVİM");
	lv_obj_set_flex_grow(c3, 4);
	l_fx = label(c3, f_m, "USD/TRY -\nEUR/TRY -");
	lv_obj_set_style_text_line_space(l_fx, 6, 0);
	lv_obj_t *sep = lv_obj_create(c3);
	lv_obj_remove_style_all(sep);
	lv_obj_set_size(sep, LV_PCT(100), 2);
	lv_obj_set_style_bg_opa(sep, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(sep, lv_color_hex(TH.sep), 0);
	l_events = label(c3, f_s, "Takvim bağlı değil");
	lv_label_set_long_mode(l_events, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(l_events, LV_PCT(100));
	lv_obj_set_style_text_line_space(l_events, 8, 0);

	/* Kisayollar */
	lv_obj_set_flex_flow(t2, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_pad_gap(t2, 16, 0);
	lv_obj_set_flex_align(t2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	macro_cont = t2;

	/* Asistan: istem dugmeleri | yanit */
	lv_obj_set_flex_flow(t3, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t3, 14, 0);
	lv_obj_remove_flag(t3, LV_OBJ_FLAG_SCROLLABLE);
	prompt_cont = lv_obj_create(t3);
	lv_obj_remove_style_all(prompt_cont);
	lv_obj_set_size(prompt_cont, 380, LV_PCT(100));
	lv_obj_set_flex_flow(prompt_cont, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(prompt_cont, 12, 0);
	lv_obj_t *ac = card(t3, "ASİSTAN");
	lv_obj_add_flag(ac, LV_OBJ_FLAG_SCROLLABLE);
	l_asst_state = label(ac, f_s, "Bir istem seç. Yanıtları PC'deki asistan (Claude) üretir.");
	lv_obj_set_style_text_color(l_asst_state, C_DIM, 0);
	l_answer = label(ac, f_m, "");
	lv_label_set_long_mode(l_answer, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(l_answer, LV_PCT(100));
	lv_obj_set_style_text_line_space(l_answer, 6, 0);

	/* Sistem */
	lv_obj_set_flex_flow(t4, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t4, 14, 0);
	lv_obj_t *s1 = card(t4, "PANEL");
	lv_obj_t *srow = lv_obj_create(s1);
	lv_obj_remove_style_all(srow);
	lv_obj_set_size(srow, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(srow, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(srow, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_set_style_pad_column(srow, 18, 0);
	lv_obj_t *slogo = lv_image_create(srow);
	lv_image_set_src(slogo, &logo_tam);
	lv_image_set_scale(slogo, 150);
	lv_obj_set_size(slogo, logo_tam.header.w * 150 / 256, logo_tam.header.h * 150 / 256);
	lv_image_set_inner_align(slogo, LV_IMAGE_ALIGN_CENTER);
	l_sys = label(srow, f_m, "");
	lv_obj_set_width(l_sys, 0);
	lv_obj_set_flex_grow(l_sys, 1);
	lv_label_set_long_mode(l_sys, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_line_space(l_sys, 6, 0);
	lv_obj_t *wfb = wf_btn(s1, LV_SYMBOL_WIFI "  Wi-Fi ağı", wf_open_cb);
	lv_obj_set_width(wfb, LV_PCT(100));
	lv_obj_t *s2 = card(t4, "EKRAN");
	br_load();
	lv_obj_t *mrow = lv_obj_create(s2);
	lv_obj_remove_style_all(mrow);
	lv_obj_set_size(mrow, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(mrow, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(mrow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	label(mrow, f_mb, "Parlaklık otomatik");
	sw_auto = lv_switch_create(mrow);
	lv_obj_set_size(sw_auto, 96, 48);
	if (br_auto)
		lv_obj_add_state(sw_auto, LV_STATE_CHECKED);
	lv_obj_add_event_cb(sw_auto, br_mode_cb, LV_EVENT_VALUE_CHANGED, NULL);
	l_br_auto = label(s2, f_s, "");
	lv_obj_set_style_text_color(l_br_auto, C_DIM, 0);
	lv_label_set_long_mode(l_br_auto, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(l_br_auto, LV_PCT(100));
	br_box = lv_obj_create(s2);
	lv_obj_remove_style_all(br_box);
	lv_obj_set_size(br_box, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(br_box, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(br_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_ver(br_box, 18, 0);
	sl_br = lv_slider_create(br_box);
	lv_obj_set_size(sl_br, LV_PCT(68), 22);	/* flex_grow varsayilan genisligi asmiyor: oransal genislik */
	lv_obj_set_ext_click_area(sl_br, 24);
	lv_slider_set_range(sl_br, 5, 100);
	lv_slider_set_value(sl_br, br_manual, LV_ANIM_OFF);
	lv_obj_add_event_cb(sl_br, br_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(sl_br, br_slider_cb, LV_EVENT_RELEASED, NULL);
	l_br_pct = label(br_box, f_l, "");
	lv_obj_set_width(l_br_pct, 130);	/* "%100" sigsin, kesilmesin */
	lv_obj_set_style_text_align(l_br_pct, LV_TEXT_ALIGN_RIGHT, 0);
	lv_label_set_text_fmt(l_br_pct, "%%%d", br_manual);
	if (br_auto)
		lv_obj_add_flag(br_box, LV_OBJ_FLAG_HIDDEN);
	lv_obj_t *info = label(s2, f_s, "\nPC ajanı: work\\panel_ajan_v4.py\nPanel portu: 8765 (TCP, satır başına JSON)\n"
				   "Wi-Fi kaydı: wifi-kaydet <ssid> <şifre>");
	lv_obj_set_style_text_color(info, C_DIM, 0);

	lv_obj_t *ssrow = lv_obj_create(s2);
	lv_obj_remove_style_all(ssrow);
	lv_obj_set_size(ssrow, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(ssrow, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(ssrow, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	label(ssrow, f_mb, "Ekran koruyucu (3 dk)");
	sw_ss = lv_switch_create(ssrow);
	lv_obj_set_size(sw_ss, 96, 48);
	if (ss_on)
		lv_obj_add_state(sw_ss, LV_STATE_CHECKED);
	lv_obj_add_event_cb(sw_ss, ss_sw_cb, LV_EVENT_VALUE_CHANGED, NULL);
	tm_build(t5);

	lv_obj_t *s3 = card(t4, "TEMA");
	for (int i = 0; i < TEMA_N; i++) {
		lv_obj_t *b = lv_button_create(s3);
		lv_obj_set_size(b, LV_PCT(100), 84);
		lv_obj_set_style_radius(b, 14, 0);
		lv_obj_set_style_bg_color(b, lv_color_hex(temalar[i].card), 0);
		lv_obj_set_style_border_width(b, i == tema ? 4 : 2, 0);
		lv_obj_set_style_border_color(b, lv_color_hex(temalar[i].acc), 0);
		lv_obj_set_flex_flow(b, LV_FLEX_FLOW_ROW);
		lv_obj_set_flex_align(b, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_column(b, 14, 0);
		lv_obj_t *sw = lv_obj_create(b);
		lv_obj_remove_style_all(sw);
		lv_obj_set_size(sw, 36, 36);
		lv_obj_set_style_radius(sw, 18, 0);
		lv_obj_set_style_bg_opa(sw, LV_OPA_COVER, 0);
		lv_obj_set_style_bg_color(sw, lv_color_hex(temalar[i].acc), 0);
		lv_obj_remove_flag(sw, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_t *l = label(b, f_mb, temalar[i].ad);
		lv_obj_set_style_text_color(l, lv_color_hex(temalar[i].text), 0);
		if (i == tema) {
			lv_obj_t *ok = label(b, f_mb, LV_SYMBOL_OK);
			lv_obj_set_style_text_color(ok, lv_color_hex(temalar[i].acc), 0);
		}
		lv_obj_add_event_cb(b, tema_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
	}
	label(s3, f_mb, "BLUETOOTH");
	l_bt = label(s3, f_s, "Bluetooth bilgisi bekleniyor");
	lv_obj_set_width(l_bt, LV_PCT(100));
	lv_label_set_long_mode(l_bt, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_line_space(l_bt, 6, 0);

	/* bildirim */
	toast = label(lv_layer_top(), f_mb, "");
	lv_obj_set_style_bg_color(toast, lv_color_hex(TH.toast), 0);
	lv_obj_set_style_text_color(toast, lv_color_hex(TH.toast_text), 0);
	lv_obj_set_style_bg_opa(toast, LV_OPA_90, 0);
	lv_obj_set_style_radius(toast, 14, 0);
	lv_obj_set_style_pad_all(toast, 16, 0);
	lv_obj_align(toast, LV_ALIGN_TOP_MID, 0, 64);
	lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);

	cJSON *m = cJSON_Parse(default_macros), *p = cJSON_Parse(default_prompts);
	build_buttons(macro_cont, m, 0, 360, 190);
	build_buttons(prompt_cont, p, 1, 380, 96);
	cJSON_Delete(m);
	cJSON_Delete(p);
}

/* ----------------------------------------------------------- PC durumunu uygula */
static double num(cJSON *o, const char *k, double d)
{
	cJSON *v = cJSON_GetObjectItem(o, k);
	return cJSON_IsNumber(v) ? v->valuedouble : d;
}

static const char *str(cJSON *o, const char *k, const char *d)
{
	cJSON *v = cJSON_GetObjectItem(o, k);
	return cJSON_IsString(v) ? v->valuestring : d;
}

static void set_metric(lv_obj_t *l, lv_obj_t *b, double v)
{
	if (v < 0) {
		lv_label_set_text(l, "--");
		return;
	}
	lv_label_set_text_fmt(l, "%d%%", (int)(v + 0.5));
	lv_bar_set_value(b, (int)v, LV_ANIM_ON);
	lv_obj_set_style_bg_color(b, v > 85 ? C_BAD : v > 60 ? C_WARN : C_OK, LV_PART_INDICATOR);
}

static void fmt_rate(char *o, size_t n, double bps)
{
	if (bps >= 1e6)
		snprintf(o, n, "%.1f MB/s", bps / 1e6);
	else
		snprintf(o, n, "%.0f KB/s", bps / 1e3);
}

/* PC'den uzaktan arayuz komutu (test ve otomasyon): {"t":"ui","tab":0..3} {"t":"ui","press":"macro:ID"|"ask:ID"} */
static void apply_ui(const char *line)
{
	cJSON *j = cJSON_Parse(line), *o;

	if (!j)
		return;
	/* uzaktan komut = etkinlik: koruyucu kapanir */
	if (ss_obj)
		ss_hide();
	lv_display_trigger_activity(disp);
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "br_mode")) && sw_auto) {
		cJSON *pc = cJSON_GetObjectItem(j, "br_pct");
		br_auto = !strcmp(o->valuestring, "auto");
		if (cJSON_IsNumber(pc) && pc->valueint >= 5 && pc->valueint <= 100)
			br_manual = pc->valueint;
		if (br_auto) {
			lv_obj_add_state(sw_auto, LV_STATE_CHECKED);
			lv_obj_add_flag(br_box, LV_OBJ_FLAG_HIDDEN);
		} else {
			lv_obj_remove_state(sw_auto, LV_STATE_CHECKED);
			lv_obj_remove_flag(br_box, LV_OBJ_FLAG_HIDDEN);
		}
		lv_slider_set_value(sl_br, br_manual, LV_ANIM_OFF);
		lv_label_set_text_fmt(l_br_pct, "%%%d", br_manual);
		br_save();
		br_apply();
	}
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "theme")))
		tema_sec(o->valueint);
	if (cJSON_IsTrue(cJSON_GetObjectItem(j, "wifi")))
		wf_open_cb(NULL);
	else if (cJSON_IsFalse(cJSON_GetObjectItem(j, "wifi")))
		wf_close_cb(NULL);
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "tab")) && o->valueint>=0 && o->valueint<6)
		lv_tabview_set_active(tabview, (uint32_t)o->valueint, LV_ANIM_OFF);
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "pc_full"))) {	/* 0.14: {"t":"ui","pc_full":1|0} */
		if (o->valueint) {
			pc_dismissed = 0;
			pc_full_enter();
		} else
			pc_full_back_cb(NULL);
	}
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "brightness"))) {
		/* PC'nin otomatik (gece/gunduz) degeri; yalniz otomatik modda uygulanir */
		br_pc = o->valueint < 1 ? 1 : o->valueint > 255 ? 255 : o->valueint;
		br_apply();
		printf("PC parlaklik %d (mod %s), ekran %d\n", br_pc, br_auto ? "otomatik" : "manuel", disp_brightness(-1));
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "press"))) {
		const char *v = o->valuestring;
		int kind = !strncmp(v, "ask:", 4);
		const char *id = strchr(v, ':') ? strchr(v, ':') + 1 : v;
		for (int i = 0; i < MAX_BTN; i++)
			if (!strcmp(btn_ids[kind][i], id)) {
				char ev[160];
				snprintf(ev, sizeof ev, "{\"t\":\"%s\",\"id\":\"%s\"}", kind ? "ask" : "macro", id);
				send_event(ev);
				if (kind)
					lv_label_set_text(l_asst_state, "Yanıt bekleniyor...");
				else
					show_toast(LV_SYMBOL_OK "  Gönderildi (uzaktan)");
				break;
			}
	}
	cJSON_Delete(j);
}

static void apply_state(const char *line)
{
	cJSON *j = cJSON_Parse(line), *o;
	char b1[32], b2[32];

	if (!j)
		return;
	if (strcmp(str(j, "t", "state"), "state")) {
		cJSON_Delete(j);
		return;
	}
	if (line != last_state) {
		free(last_state);
		last_state = strdup(line);
	}
	double t = num(j, "time", 0);
	if (t > 1.7e9) {
		struct timeval now;
		gettimeofday(&now, NULL);
		if (t - now.tv_sec > 2 || now.tv_sec - t > 2) {
			struct timeval tv = {.tv_sec = (time_t)t};
			settimeofday(&tv, NULL);
		}
	}
	if ((o = cJSON_GetObjectItem(j, "pc"))) {
		lv_label_set_text(l_pcname, str(o, "name", "PC"));
		set_metric(l_cpu, b_cpu, num(o, "cpu", -1));
		set_metric(l_ram, b_ram, num(o, "ram", -1));
		set_metric(l_disk, b_disk, num(o, "disk", -1));
		fmt_rate(b1, sizeof b1, num(o, "down", 0));
		fmt_rate(b2, sizeof b2, num(o, "up", 0));
		lv_label_set_text_fmt(l_netio, LV_SYMBOL_DOWNLOAD " %s    " LV_SYMBOL_UPLOAD " %s", b1, b2);
		lv_label_set_text(l_pcup, str(o, "info", ""));
	}
	if ((o = cJSON_GetObjectItem(j, "weather"))) {
		lv_label_set_text_fmt(l_weather, "%s  %.0f°C", str(o, "city", ""), num(o, "temp", 0));
		lv_label_set_text(l_weather2, str(o, "desc", ""));
	}
	if ((o = cJSON_GetObjectItem(j, "fx")) && cJSON_IsObject(o)) {
		char t2[256] = "";
		cJSON *it;
		cJSON_ArrayForEach(it, o) {
			if (cJSON_IsNumber(it) && strlen(t2) < 200)
				snprintf(t2 + strlen(t2), sizeof t2 - strlen(t2), "%s%s  %.2f TL", t2[0] ? "\n" : "", it->string,
					 it->valuedouble);
		}
		lv_label_set_text(l_fx, t2[0] ? t2 : "-");
	}
	if ((o = cJSON_GetObjectItem(j, "events")) && cJSON_IsArray(o)) {
		char t2[1024] = "";
		cJSON *it;
		cJSON_ArrayForEach(it, o) {
			if (strlen(t2) < 900)
				snprintf(t2 + strlen(t2), sizeof t2 - strlen(t2), "%s" LV_SYMBOL_BELL " %s  %s", t2[0] ? "\n" : "",
					 str(it, "time", ""), str(it, "title", ""));
		}
		lv_label_set_text(l_events, t2[0] ? t2 : "Bugün etkinlik yok");
	}
	if ((o = cJSON_GetObjectItem(j, "macros")) && cJSON_IsArray(o)) {
		char *s = cJSON_PrintUnformatted(o);
		if (s && strncmp(s, macro_sig, sizeof macro_sig - 1)) {
			snprintf(macro_sig, sizeof macro_sig, "%s", s);
			build_buttons(macro_cont, o, 0, 360, 190);
		}
		free(s);
	}
	if ((o = cJSON_GetObjectItem(j, "prompts")) && cJSON_IsArray(o)) {
		char *s = cJSON_PrintUnformatted(o);
		if (s && strncmp(s, prompt_sig, sizeof prompt_sig - 1)) {
			snprintf(prompt_sig, sizeof prompt_sig, "%s", s);
			build_buttons(prompt_cont, o, 1, 380, 96);
		}
		free(s);
	}
	if ((o = cJSON_GetObjectItem(j, "assistant"))) {
		const char *a = str(o, "text", NULL);
		if (a)
			lv_label_set_text(l_answer, a);
		lv_label_set_text(l_asst_state, cJSON_IsTrue(cJSON_GetObjectItem(o, "busy")) ? "Yanıt bekleniyor..."
											 : str(o, "status", ""));
	}
	if ((o = cJSON_GetObjectItem(j, "toast")) && cJSON_IsString(o) && o->valuestring[0])
		show_toast(o->valuestring);
	cJSON_Delete(j);
}

/* --------------------------------------------------------------- periyodik */
static char wl_ssid[64] = "-";

static void wlan_info(char *ip, size_t ipn, int *q)
{
	static int sig = 0;
	static time_t last;
	struct ifreq r;
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	snprintf(ip, ipn, "-");
	*q = -1;
	memset(&r, 0, sizeof r);
	snprintf(r.ifr_name, IFNAMSIZ, "wlan0");
	if (s >= 0 && !ioctl(s, SIOCGIFADDR, &r))
		snprintf(ip, ipn, "%s", inet_ntoa(((struct sockaddr_in *)&r.ifr_addr)->sin_addr));
	if (s >= 0)
		close(s);
	/* /proc/net/wireless yok (WEXT kapali): 10 sn'de bir iw ciktisindan sinyal ve SSID */
	if (time(NULL) - last >= 10 || last == 0) {
		last = time(NULL);
		FILE *f = strcmp(ip, "-") ? popen("iw dev wlan0 link 2>/dev/null", "r") : NULL;
		char line[256];
		sig = 0;
		snprintf(wl_ssid, sizeof wl_ssid, "-");
		while (f && fgets(line, sizeof line, f)) {
			char *p;
			if ((p = strstr(line, "signal:")))
				sig = atoi(p + 7);
			else if ((p = strstr(line, "SSID:"))) {
				p += 5;
				while (*p == ' ')
					p++;
				p[strcspn(p, "\r\n")] = 0;
				snprintf(wl_ssid, sizeof wl_ssid, "%s", p);
			}
		}
		if (f)
			pclose(f);
	}
	*q = sig ? sig : -1;
}

static void bt_tick(void)
{
	char data[1024];
	struct bt_status s = {0, -1, 0, 0};
	FILE *f = fopen("/tmp/bt-durum.json", "r");
	if (f) {
		size_t n = fread(data, 1, sizeof data - 1, f);
		int valid = !ferror(f) && n < sizeof data - 1;
		fclose(f);
		data[n] = 0;
		if (valid) {
			struct timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);
			s = bt_status_parse(data, now.tv_sec + now.tv_nsec / 1e9);
		}
	}
	if (!s.fresh) {
		lv_label_set_text(l_bt, "Bluetooth bilgisi güncel değil");
		lv_obj_set_style_text_color(l_bt, C_DIM, 0);
		return;
	}
	const char *connection = s.connection == 1 ? "Bağlı" :
				 s.connection == 0 ? "Bağlı değil" : "Bağlantı bilgisi bekleniyor";
	if (s.music_event)
		lv_label_set_text_fmt(l_bt, "Bluetooth: %s\nMüzik bildirimi alındı (%.0f sn önce)", connection, s.music_age);
	else
		lv_label_set_text_fmt(l_bt, "Bluetooth: %s\nMüzik bildirimi bekleniyor", connection);
	lv_obj_set_style_text_color(l_bt, s.connection == 1 ? C_OK : C_DIM, 0);
}

static void tick_cb(lv_timer_t *t)
{
	static time_t last;
	time_t now = time(NULL);
	struct tm tm;
	char ip[32];
	int q;

	(void)t;
	pthread_mutex_lock(&mu);
	char *p = pending;
	pending = NULL;
	pthread_mutex_unlock(&mu);
	if (p) {
		apply_state(p);
		free(p);
	}
	pthread_mutex_lock(&mu);
	int un = ui_n;
	char *uq[8];
	memcpy(uq, ui_pending, sizeof uq);
	ui_n = 0;
	pthread_mutex_unlock(&mu);
	for (int i = 0; i < un; i++) {
		apply_ui(uq[i]);
		free(uq[i]);
	}
	uintptr_t until = (uintptr_t)lv_obj_get_user_data(toast);
	if (until && tick_ms() > until) {
		lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);
		lv_obj_set_user_data(toast, NULL);
	}
	tm_tick();
	wf_tick();
	if (now == last)
		return;
	last = now;
	bt_tick();
	ss_tick();
	br_apply();
	localtime_r(&now, &tm);
	if (tm.tm_year > 120) {
		lv_label_set_text_fmt(l_clock, "%02d:%02d", tm.tm_hour, tm.tm_min);
		lv_label_set_text_fmt(l_sec, ":%02d", tm.tm_sec);
		lv_label_set_text_fmt(l_date, "%d %s %d, %s", tm.tm_mday, aylar[tm.tm_mon], tm.tm_year + 1900,
				      gunler[tm.tm_wday]);
		lv_label_set_text_fmt(l_hdr_date, "%d %s %s  %02d:%02d", tm.tm_mday, aylar[tm.tm_mon], gunler[tm.tm_wday],
				      tm.tm_hour, tm.tm_min);
	} else {
		lv_label_set_text(l_date, "Saat ayarlanmadı (Wi-Fi/NTP ya da PC ajanı bekleniyor)");
	}
	int pc_ok = pc_conn && mono_s() - pc_last < 10;
	lv_label_set_text_fmt(l_hdr_pc, "%s  %s", LV_SYMBOL_USB, pc_ok ? "PC bağlı" : "PC bağlı değil");
	lv_obj_set_style_text_color(l_hdr_pc, pc_ok ? C_OK : C_DIM, 0);
	if (now % 3 == 0 || l_sys) {
		wlan_info(ip, sizeof ip, &q);
		lv_label_set_text_fmt(l_hdr_net, LV_SYMBOL_WIFI "  %s%s", ip, q > -200 && q < 0 ? "" : "");
		lv_obj_set_style_text_color(l_hdr_net, strcmp(ip, "-") ? C_OK : C_DIM, 0);
		struct sysinfo si;
		sysinfo(&si);
		lv_label_set_text_fmt(l_sys,
				      APP_ADI " %s\nWi-Fi: %s  (%d dBm)\nIP: %s\nPC: %s (%s)\nÇalışma: %ld sa %ld dk\n"
				      "Bellek: %lu / %lu MB boş\nYük: %.2f",
				      PANEL_VER, wl_ssid, q, ip, pc_ok ? "bağlı" : "yok", pc_peer, si.uptime / 3600,
				      (si.uptime / 60) % 60, (unsigned long)(si.freeram * si.mem_unit >> 20),
				      (unsigned long)(si.totalram * si.mem_unit >> 20), si.loads[0] / 65536.0);
	}
}

static volatile int quit;
static void on_sig(int s)
{
	(void)s;
	quit = 1;
}

int main(void)
{
	pthread_t th;

	setvbuf(stdout, NULL, _IOLBF, 0);
	if (!getenv("TZ"))
		setenv("TZ", "<+03>-3", 1);
	tzset();
	signal(SIGTERM, on_sig);
	signal(SIGINT, on_sig);
	signal(SIGPIPE, SIG_IGN);
	if (fb_init())
		return 1;
	lv_init();
	lv_tick_set_cb(tick_ms);
	lv_display_t *d = lv_display_create(W, H);
	lv_display_set_color_format(d, LV_COLOR_FORMAT_XRGB8888);
	size_t bsz = (size_t)W * 60 * 4;
	lv_display_set_buffers(d, malloc(bsz), malloc(bsz), bsz, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(d, flush_cb);
	fonts_init();
	br_load();
	disp = d;
	lv_display_set_theme(d, lv_theme_default_init(d, C_ACC, lv_color_hex(TH.acc2), TH.dark, f_s));
	touch_init();
	lv_indev_t *in = lv_indev_create();
	lv_indev_set_type(in, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(in, touch_read);
	ui_build();
	lv_timer_create(tick_cb, 200, NULL);
	lv_display_add_event_cb(d, refr_start_cb, LV_EVENT_REFR_START, NULL);
	{
		const char *e = getenv("PANEL_DOGRUDAN");
		pc_dc_izin = !(e && e[0] == '0');
		printf("PC dogrudan cizim: %s\n", pc_dc_izin ? "acik" : "kapali (PANEL_DOGRUDAN=0)");
		const char *g = getenv("PANEL_G2D_ALICI");
		pc_dg_acik = !(g && g[0] == '0');
		pc_dg_gen = (uint32_t)getpid() << 16 ^ tick_ms();
		pc_dg_yaz(0);
		printf("Aliciya G2D devri: %s\n", pc_dg_acik ? "acik" : "kapali (PANEL_G2D_ALICI=0)");
	}
	lv_timer_create(pc_image_tick,10,NULL);
	pthread_create(&th, NULL, net_thread, NULL);
	printf("%s %s hazir, port %d\n", APP_ADI, PANEL_VER, PORT);
	while (!quit) {
		uint32_t w = lv_timer_handler();
		uint32_t cap = pc_full ? 5 : 30;	/* tam ekranda kirli alan bildirimine hizli tepki */
		usleep((w > cap ? cap : w) * 1000);
	}
	int tty = open("/dev/tty1", O_RDWR);
	if (tty >= 0)
		ioctl(tty, KDSETMODE, KD_TEXT);
	return 0;
}
