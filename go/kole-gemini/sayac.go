// sayac.go v2 (23.10.2026, Claude; kole-gemini v37): OOM mesajı oomMesaji (oom_kmsg.go: kurban + tetikleyen).
// sayac.go (19.10.2026, Claude): kole-gemini v11 — K4 sağlık sayaçları ve eşik uyarıları (yol haritası v2 K4).
// Dakikada bir: MemAvailable, wlan0 hata/düşme sayaçları (/proc/net/dev), oom_kill (/proc/vmstat). Satır /tmp/saglik.log'a
// (RAM). Eşikler /data/asistan/saglik-bellek-esik-mb (12) ve saglik-wifi-esik (200 / 10 dk). Durum /tmp/saglik-sayac.json.
package main

import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

const (
	sayacDurumYol    = "/tmp/saglik-sayac.json"
	bellekEsikYol    = "/data/asistan/saglik-bellek-esik-mb"
	wifiEsikYol      = "/data/asistan/saglik-wifi-esik"
	sayacAralik      = 55  // sn: ölçüm en sık
	wifiPencere      = 600 // sn: hata artışının bakıldığı pencere
	oomUyariAralik   = 600 // sn
	saglikLogAzami   = 65536
	varsayilanBellek = 12  // MB
	varsayilanWifi   = 200 // 10 dk'da hata+düşme
)

var saglikLogYol = "/tmp/saglik.log"

type sayaclar struct {
	MemKB  int64  `json:"mem_kb"`
	WifiOK bool   `json:"wifi_ok"`
	RxErr  uint64 `json:"rx_err"`
	RxDrop uint64 `json:"rx_drop"`
	TxErr  uint64 `json:"tx_err"`
	TxDrop uint64 `json:"tx_drop"`
	Oom    int64  `json:"oom"` // -1 okunamadı
	Zaman  int64  `json:"zaman"`
}

func (s sayaclar) wifiToplam() uint64 { return s.RxErr + s.RxDrop + s.TxErr + s.TxDrop }

type wifiNokta struct {
	Z int64  `json:"z"`
	T uint64 `json:"t"` // tabandan beri birikmiş toplam (sıfırlanmalar düzeltilmiş)
}

type sayacDurum struct {
	Onceki      sayaclar    `json:"onceki"`
	Birikim     uint64      `json:"birikim"` // sıfırlanmalara dayanıklı wifi toplamı
	Noktalar    []wifiNokta `json:"noktalar"`
	DusukSay    int         `json:"dusuk_say"`
	BellekUyari bool        `json:"bellek_uyari"`
	SonBellek   int64       `json:"son_bellek_uyari"`
	SonOom      int64       `json:"son_oom_uyari"`
	WifiUyari   bool        `json:"wifi_uyari"`
	SonWifi     int64       `json:"son_wifi_uyari"`
	EnDusukKB   int64       `json:"en_dusuk_kb"`
	EnDusukGun  string      `json:"en_dusuk_gun"`
}

// sayacOku: procKok altından (test sahte /proc kullanır)
func sayacOku(simdi int64) sayaclar {
	s := sayaclar{MemKB: memAvailableKB(), Oom: -1, Zaman: simdi}
	if b, err := os.ReadFile(filepath.Join(procKok, "net/dev")); err == nil {
		for _, l := range strings.Split(string(b), "\n") {
			l = strings.TrimSpace(l)
			if !strings.HasPrefix(l, "wlan0:") {
				continue
			}
			f := strings.Fields(strings.TrimPrefix(l, "wlan0:"))
			if len(f) >= 12 {
				u := func(i int) uint64 { v, _ := strconv.ParseUint(f[i], 10, 64); return v }
				s.WifiOK, s.RxErr, s.RxDrop, s.TxErr, s.TxDrop = true, u(2), u(3), u(10), u(11)
			}
		}
	}
	if b, err := os.ReadFile(filepath.Join(procKok, "vmstat")); err == nil {
		for _, l := range strings.Split(string(b), "\n") {
			if f := strings.Fields(l); len(f) == 2 && f[0] == "oom_kill" {
				s.Oom, _ = strconv.ParseInt(f[1], 10, 64)
			}
		}
	}
	return s
}

func esikOku(yol string, varsayilan int64) int64 {
	v, err := strconv.ParseInt(strings.TrimSpace(oku(yol, "")), 10, 64)
	if err != nil || v <= 0 {
		return varsayilan
	}
	return v
}

// sayacAdimi: saf karar; gönderilecek mesajlar döner. gun: TR gün damgası (en düşük bellek günlük sıfırlanır).
func sayacAdimi(d *sayacDurum, s sayaclar, bellekMB, wifiEsik int64, gun string) []string {
	var m []string
	simdi := s.Zaman
	// bellek
	if s.MemKB >= 0 {
		if d.EnDusukGun != gun || d.EnDusukKB <= 0 || s.MemKB < d.EnDusukKB {
			if d.EnDusukGun != gun {
				d.EnDusukGun = gun
			}
			d.EnDusukKB = s.MemKB
		}
		if s.MemKB < bellekMB*1024 {
			d.DusukSay++
		} else {
			d.DusukSay = 0
		}
		if d.DusukSay >= 3 && !d.BellekUyari && (d.SonBellek == 0 || simdi-d.SonBellek >= uyariAralik) {
			m = append(m, fmt.Sprintf("⚠️ Boş bellek az efendim: %d MB (eşik %d MB). Panel/kamera yavaşlayabilir.", s.MemKB/1024, bellekMB))
			d.BellekUyari, d.SonBellek = true, simdi
		} else if d.BellekUyari && s.MemKB >= (bellekMB+4)*1024 {
			m = append(m, fmt.Sprintf("✅ Bellek yeniden rahat: %d MB boş.", s.MemKB/1024))
			d.BellekUyari = false
		}
	}
	// OOM
	if s.Oom >= 0 && d.Onceki.Zaman != 0 && d.Onceki.Oom >= 0 && s.Oom > d.Onceki.Oom &&
		(d.SonOom == 0 || simdi-d.SonOom >= oomUyariAralik) {
		n := s.Oom - d.Onceki.Oom
		m = append(m, oomMesaji(n, oomOlayOku(int(n)))) // v37
		d.SonOom = simdi
	}
	// Wi-Fi: sıfırlanmaya dayanıklı birikim
	if s.WifiOK {
		if d.Onceki.WifiOK && d.Onceki.Zaman != 0 && s.wifiToplam() >= d.Onceki.wifiToplam() {
			d.Birikim += s.wifiToplam() - d.Onceki.wifiToplam()
		}
		d.Noktalar = append(d.Noktalar, wifiNokta{simdi, d.Birikim})
		ilk := 0
		for ilk < len(d.Noktalar)-1 && simdi-d.Noktalar[ilk+1].Z >= wifiPencere {
			ilk++
		}
		d.Noktalar = d.Noktalar[ilk:]
		if len(d.Noktalar) > 0 && simdi-d.Noktalar[0].Z >= wifiPencere/2 {
			artis := d.Birikim - d.Noktalar[0].T
			if int64(artis) > wifiEsik && !d.WifiUyari && (d.SonWifi == 0 || simdi-d.SonWifi >= uyariAralik) {
				m = append(m, fmt.Sprintf("⚠️ Wi-Fi hata/düşme artıyor efendim: son %d dk'da %d (eşik %d). Bağlantı takılabilir.",
					(simdi-d.Noktalar[0].Z+30)/60, artis, wifiEsik))
				d.WifiUyari, d.SonWifi = true, simdi
			} else if d.WifiUyari && int64(artis)*10 <= wifiEsik {
				m = append(m, "✅ Wi-Fi hataları azaldı.")
				d.WifiUyari = false
			}
		}
	}
	d.Onceki = s
	return m
}

func saglikSatiri(s sayaclar, d *sayacDurum, t time.Time) string {
	oom := "?"
	if s.Oom >= 0 {
		oom = strconv.FormatInt(s.Oom, 10)
	}
	w := "wlan0 yok"
	if s.WifiOK {
		w = fmt.Sprintf("wlan0 rx_err %d rx_drop %d tx_err %d tx_drop %d", s.RxErr, s.RxDrop, s.TxErr, s.TxDrop)
	}
	return fmt.Sprintf("%s bellek %d MB (bugün en düşük %d) %s oom %s\n", t.Format("2006-01-02 15:04:05"), s.MemKB/1024,
		d.EnDusukKB/1024, w, oom)
}

func saglikLogEkle(satir string) {
	if st, err := os.Stat(saglikLogYol); err == nil && st.Size() > saglikLogAzami {
		if b, err := os.ReadFile(saglikLogYol); err == nil {
			b = b[len(b)/2:]
			if i := strings.IndexByte(string(b), '\n'); i >= 0 {
				b = b[i+1:]
			}
			_ = os.WriteFile(saglikLogYol, b, 0644)
		}
	}
	f, err := os.OpenFile(saglikLogYol, os.O_WRONLY|os.O_CREATE|os.O_APPEND, 0644)
	if err != nil {
		return
	}
	_, _ = f.WriteString(satir)
	_ = f.Close()
}

func sayacDurumOku() sayacDurum {
	var d sayacDurum
	if b, err := os.ReadFile(sayacDurumYol); err == nil {
		_ = json.Unmarshal(b, &d)
	}
	return d
}

// sayacTuru: zamanlanmisIsler her turda çağırır; ölçüm en sık sayacAralik sn'de bir.
func sayacTuru(ctx context.Context, jeton, sahip string, uyar bool, simdi int64) {
	d := sayacDurumOku()
	if d.Onceki.Zaman != 0 && simdi-d.Onceki.Zaman < sayacAralik {
		return
	}
	t := time.Unix(simdi, 0).In(trZaman)
	s := sayacOku(simdi)
	m := sayacAdimi(&d, s, esikOku(bellekEsikYol, varsayilanBellek), esikOku(wifiEsikYol, varsayilanWifi), t.Format("2006-01-02"))
	saglikLogEkle(saglikSatiri(s, &d, t))
	if b, err := json.Marshal(d); err == nil {
		_ = os.WriteFile(sayacDurumYol, b, 0600)
	}
	if uyar {
		for _, x := range m {
			tgHerkese(ctx, jeton, sahip, x) // v45
		}
	}
}

// sayacOzeti: /saglik için tek satır
func sayacOzeti() string {
	d := sayacDurumOku()
	s := sayacOku(time.Now().Unix())
	p := fmt.Sprintf("Bellek: boş %d MB", s.MemKB/1024)
	if d.EnDusukKB > 0 {
		p += fmt.Sprintf(" (bugün en düşük %d MB)", d.EnDusukKB/1024)
	}
	if s.WifiOK {
		son := uint64(0)
		if len(d.Noktalar) > 0 {
			son = d.Birikim - d.Noktalar[0].T
		}
		p += fmt.Sprintf(" · Wi-Fi hata/düşme: son 10 dk %d (toplam rx %d/%d, tx %d/%d)", son, s.RxErr, s.RxDrop, s.TxErr, s.TxDrop)
	}
	if s.Oom >= 0 {
		p += fmt.Sprintf(" · OOM: %d", s.Oom)
	}
	return p + "."
}
