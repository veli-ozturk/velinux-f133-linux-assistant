// saglik.go (15.10.2026, Claude): kole-gemini v9 — cihaz sağlığı, sabah özeti ve hizmet uyarıları (Telegram).
// kullanıcı dışarıdayken: /durum ve /saglik hizmetleri gösterir; her sabah (varsayılan 08:00, 3 saatlik pencere) özet gelir (sabah.go);
// önemli bir hizmet art arda 3 kontrolde (~3 dk) yoksa uyarı, geri gelince bilgi (hizmet başına saatte en çok bir uyarı).
// Ayarlar (/data/asistan): sabah-ozeti-saat "HH:MM", sabah-ozeti-kapali (varsa gönderilmez), sabah-ozeti-son (gün damgası),
// saglik-uyari-kapali (varsa uyarı yok). Durum /tmp/saglik-durum.json (RAM). Sırların içeriği okunmaz.
package main

import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"sort"
	"strconv"
	"strings"
	"syscall"
	"time"
)

const (
	sabahSaatYol   = "/data/asistan/sabah-ozeti-saat"
	sabahKapaliYol = "/data/asistan/sabah-ozeti-kapali"
	sabahSonYol    = "/data/asistan/sabah-ozeti-son"
	uyariKapaliYol = "/data/asistan/saglik-uyari-kapali"
	saglikDurumYol = "/tmp/saglik-durum.json"
	esikKacirma    = 3
	uyariAralik    = 3600
)

var procKok = "/proc"

type hizmetTanim struct{ anahtar, ad, onek string }

// Önemli hizmetler (PC ekranı alıcısı istenince çalıştığı için listede değil)
var hizmetler = []hizmetTanim{
	{"panel", "Panel", "panel"},
	{"kamera_kayit", "Kamera kaydı", "kamera-kayit-v"},
	{"kole_kopru", "Kamera köprüsü", "kole-kopru-v"},
	{"merkez", "Cihaz merkezi", "kole-cihaz-merk"},
	{"tailscale", "Tailscale (uzaktan erişim)", "tailscaled"},
	{"asistan", "Hey Köle asistanı", "kole-asistan-v"},
	{"mcu", "MCU hizmeti", "mcu-servis-v"},
}

// hizmetTara: anahtar -> süreç adı ("" yok). comm çekirdekte 15 karakterde kesilir: önekle eşlenir.
func hizmetTara() map[string]string {
	sonuc := map[string]string{}
	girdiler, _ := os.ReadDir(procKok)
	for _, g := range girdiler {
		if g.Name() == "" || g.Name()[0] < '1' || g.Name()[0] > '9' {
			continue
		}
		b, err := os.ReadFile(filepath.Join(procKok, g.Name(), "comm"))
		if err != nil {
			continue
		}
		c := strings.TrimSpace(string(b))
		for _, h := range hizmetler {
			esit := strings.HasPrefix(c, h.onek)
			if h.onek == "panel" {
				esit = c == "panel"
			}
			if esit && sonuc[h.anahtar] == "" {
				sonuc[h.anahtar] = c
			}
		}
	}
	return sonuc
}

func memAvailableKB() int64 {
	b, err := os.ReadFile(filepath.Join(procKok, "meminfo"))
	if err != nil {
		return -1
	}
	for _, s := range strings.Split(string(b), "\n") {
		if strings.HasPrefix(s, "MemAvailable:") {
			f := strings.Fields(s)
			if len(f) >= 2 {
				v, _ := strconv.ParseInt(f[1], 10, 64)
				return v
			}
		}
	}
	return -1
}

func calismaSuresi() string {
	b, err := os.ReadFile(filepath.Join(procKok, "uptime"))
	if err != nil {
		return "?"
	}
	f := strings.Fields(string(b))
	if len(f) == 0 {
		return "?"
	}
	sn, _ := strconv.ParseFloat(f[0], 64)
	d := time.Duration(sn) * time.Second
	if d >= 24*time.Hour {
		return fmt.Sprintf("%d gün %d sa", int(d.Hours())/24, int(d.Hours())%24)
	}
	return fmt.Sprintf("%d sa %d dk", int(d.Hours()), int(d.Minutes())%60)
}

func sdBosMB(yol string) int64 {
	var st syscall.Statfs_t
	if syscall.Statfs(yol, &st) != nil {
		return -1
	}
	return int64(st.Bavail) * int64(st.Bsize) / 1048576
}

func asistanKapali() bool { return dosyaVar("/data/asistan/kapali") }

// saglikOzeti: Telegram için kısa çok satırlı metin
func saglikOzeti() string {
	hz := hizmetTara()
	var p []string
	p = append(p, fmt.Sprintf("Cihaz: %s açık, boş bellek %d MB, SD boş %s.", calismaSuresi(), memAvailableKB()/1024, mbMetin(sdBosMB("/mnt/sd"))))
	for _, h := range hizmetler {
		ad := hz[h.anahtar]
		switch {
		case ad != "":
			p = append(p, fmt.Sprintf("● %s (%s)", h.ad, ad))
		case h.anahtar == "asistan" && asistanKapali():
			p = append(p, "○ "+h.ad+" (ayarlardan kapalı)")
		default:
			p = append(p, "○ "+h.ad+" ÇALIŞMIYOR")
		}
	}
	p = append(p, sayacOzeti()) // v11
	var kd struct {
		SaatAyar int64 `json:"saat_ayar"`
	}
	if b, err := os.ReadFile("/tmp/kamera-kayit.json"); err == nil && json.Unmarshal(b, &kd) == nil && kd.SaatAyar > 0 {
		p = append(p, "Kamera saati son eşitleme: "+trTarih(time.Unix(kd.SaatAyar, 0).In(trZaman))+".")
	}
	return strings.Join(p, "\n")
}

func mbMetin(v int64) string {
	if v < 0 {
		return "bilinmiyor"
	}
	if v >= 1024 {
		return fmt.Sprintf("%.1f GB", float64(v)/1024)
	}
	return fmt.Sprintf("%d MB", v)
}

// ---- sabah özeti (saf karar fonksiyonu test edilir)

func sabahSaati() (int, int) {
	s := strings.TrimSpace(oku(sabahSaatYol, "08:00"))
	var sa, dk int
	if n, _ := fmt.Sscanf(s, "%d:%d", &sa, &dk); n != 2 || sa < 0 || sa > 23 || dk < 0 || dk > 59 {
		return 8, 0
	}
	return sa, dk
}

// sabahGonderilsinMi: şimdi (TR saati) pencerede (saat .. saat+3sa) ve bugün gönderilmediyse
func sabahGonderilsinMi(simdi time.Time, sa, dk int, sonGun string) bool {
	bas := time.Date(simdi.Year(), simdi.Month(), simdi.Day(), sa, dk, 0, 0, simdi.Location())
	return !simdi.Before(bas) && simdi.Before(bas.Add(3*time.Hour)) && sonGun != simdi.Format("2006-01-02")
}

// sabahOzeti: sabah.go

// ---- hizmet uyarıları (saf durum makinesi test edilir)

type uyariDurum struct {
	Kacirma map[string]int   `json:"kacirma"`
	SonUyar map[string]int64 `json:"son_uyari"`
	Dustu   map[string]bool  `json:"dustu"`
}

// uyariAdimi: bir kontrol turu; mesajlar (gönderilecek) döner, durum güncellenir
func uyariAdimi(d *uyariDurum, hz map[string]string, asistanKapali bool, simdi int64) []string {
	if d.Kacirma == nil {
		d.Kacirma, d.SonUyar, d.Dustu = map[string]int{}, map[string]int64{}, map[string]bool{}
	}
	var m []string
	for _, h := range hizmetler {
		if h.anahtar == "asistan" && asistanKapali {
			d.Kacirma[h.anahtar], d.Dustu[h.anahtar] = 0, false
			continue
		}
		if hz[h.anahtar] != "" {
			if d.Dustu[h.anahtar] {
				m = append(m, "✅ "+h.ad+" yeniden çalışıyor efendim.")
			}
			d.Kacirma[h.anahtar], d.Dustu[h.anahtar] = 0, false
			continue
		}
		d.Kacirma[h.anahtar]++
		if d.Kacirma[h.anahtar] >= esikKacirma && !d.Dustu[h.anahtar] && (d.SonUyar[h.anahtar] == 0 || simdi-d.SonUyar[h.anahtar] >= uyariAralik) {
			m = append(m, "⚠️ "+h.ad+" çalışmıyor efendim (≈3 dk'dır yok).")
			d.Dustu[h.anahtar] = true
			d.SonUyar[h.anahtar] = simdi
		}
	}
	sort.Strings(m)
	return m
}

// zamanlanmisIsler: telegramDinle her turda (≈15 sn) çağırır
func zamanlanmisIsler(ctx context.Context, jeton, sahip, sehirYol string) {
	simdi := time.Now().In(trZaman)
	if !dosyaVar(sabahKapaliYol) {
		sa, dk := sabahSaati()
		if sabahGonderilsinMi(simdi, sa, dk, strings.TrimSpace(oku(sabahSonYol, ""))) {
			_ = os.WriteFile(sabahSonYol, []byte(simdi.Format("2006-01-02")+"\n"), 0600) // önce damga: hata olsa da tekrarlamasın
			tgHerkese(ctx, jeton, sahip, sabahOzeti(ctx, sehirYol))                      // v45: eşine de
		}
	}
	sayacTuru(ctx, jeton, sahip, !dosyaVar(uyariKapaliYol), simdi.Unix()) // v11
	if dosyaVar(uyariKapaliYol) {
		return
	}
	var d uyariDurum
	if b, err := os.ReadFile(saglikDurumYol); err == nil {
		_ = json.Unmarshal(b, &d)
	}
	for _, m := range uyariAdimi(&d, hizmetTara(), asistanKapali(), simdi.Unix()) {
		tgHerkese(ctx, jeton, sahip, m) // v45
	}
	if b, err := json.Marshal(d); err == nil {
		_ = os.WriteFile(saglikDurumYol, b, 0600)
	}
}

// sabahKomutu: "/sabah" (şimdi gönder), "/sabah 07:45", "/sabah kapat|ac"
func sabahKomutu(ctx context.Context, arg, sehirYol string) string {
	switch {
	case arg == "":
		return sabahOzeti(ctx, sehirYol)
	case arg == "kapat":
		_ = os.WriteFile(sabahKapaliYol, nil, 0600)
		return "Sabah özeti kapatıldı efendim."
	case arg == "ac" || arg == "aç":
		_ = os.Remove(sabahKapaliYol)
		sa, dk := sabahSaati()
		return fmt.Sprintf("Sabah özeti açıldı efendim (%02d:%02d).", sa, dk)
	}
	var sa, dk int
	if n, _ := fmt.Sscanf(arg, "%d:%d", &sa, &dk); n == 2 && sa >= 0 && sa <= 23 && dk >= 0 && dk <= 59 {
		_ = os.WriteFile(sabahSaatYol, []byte(fmt.Sprintf("%02d:%02d\n", sa, dk)), 0600)
		_ = os.Remove(sabahKapaliYol)
		return fmt.Sprintf("Sabah özetini her gün %02d:%02d'de göndereceğim efendim.", sa, dk)
	}
	return "Kullanım: /sabah | /sabah 08:30 | /sabah kapat | /sabah ac"
}
