// zaman_kip.go (22.10.2026, Claude; kole-gemini v34, madde 113/114/121): time-lapse kip seçimi Telegram'da ve sesle.
// kullanıcı: "mod seçme hem panelde, hem merkezde hem botta olsun". kamera-kayit v9: <kamera>/zaman-acik, zaman-aralik
// (1..3600 sn, varsayılan 60), zaman-olay (time-lapse sırasında dakikalık hareket puanı, eşik üstünde ek kare + Telegram
// fotoğrafı); /tmp/kamera-kayit.json zaman_mb (tüm günler), zaman_sinir_mb, zaman_dolu.
package main

import (
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"strings"
)

type zamanYollar struct{ Acik, Aralik, Olay, Durum string }

var zamanY = zamanYollar{zamanBayrak, "/mnt/sd/kamera/zaman-aralik", "/mnt/sd/kamera/zaman-olay", "/tmp/kamera-kayit.json"}

const zamanKullanim = "Kullanım efendim: /zaman ac | kapat, /zaman 3 (kare aralığı sn, 1..3600; 1dk da olur), " +
	"/zaman olay ac | kapat"

// zamanAralikCoz: "3", "3sn", "3 sn", "1dk", "1 dk" -> saniye (1..3600); geçersizse 0
func zamanAralikCoz(s string) int {
	s = strings.ReplaceAll(strings.ToLower(strings.TrimSpace(s)), " ", "")
	k := 1
	switch {
	case strings.HasSuffix(s, "dk"):
		s, k = strings.TrimSuffix(s, "dk"), 60
	case strings.HasSuffix(s, "sn"):
		s = strings.TrimSuffix(s, "sn")
	case strings.HasSuffix(s, "s"):
		s = strings.TrimSuffix(s, "s")
	}
	n, err := strconv.Atoi(s)
	if err != nil || n < 1 || n*k > 3600 {
		return 0
	}
	return n * k
}

// zamanAralikOku: dosyadaki aralık (kamera-kayit v9 ile aynı: geçersiz/yoksa 60)
func zamanAralikOku(y string) int {
	b, err := os.ReadFile(y)
	if err != nil {
		return 60
	}
	n, err := strconv.Atoi(strings.TrimSpace(string(b)))
	if err != nil || n < 1 || n > 3600 {
		return 60
	}
	return n
}

func aralikMetni(sn int) string {
	if sn >= 60 && sn%60 == 0 {
		return fmt.Sprintf("%d dk", sn/60)
	}
	return fmt.Sprintf("%d sn", sn)
}

// zamanKisa: /durum satırı için
func zamanKisa(z zamanYollar, durumAcik bool) string {
	if !durumAcik && !dosyaVar(z.Acik) {
		return "kapalı"
	}
	s := "açık (her " + aralikMetni(zamanAralikOku(z.Aralik))
	if dosyaVar(z.Olay) {
		s += ", olay tespiti"
	}
	return s + ")"
}

// zamanDurumMetni: ⏱ menüsü
func zamanDurumMetni(z zamanYollar) string {
	s := "⏱ Time-lapse: " + acikKapali(z.Acik) + " · her " + aralikMetni(zamanAralikOku(z.Aralik)) +
		"\n🔎 Olay tespiti: " + acikKapali(z.Olay) + " (hareket anları ek kare + Telegram fotoğrafı)"
	var kd struct {
		MB    *float64 `json:"zaman_mb"`
		Sinir int      `json:"zaman_sinir_mb"`
		Dolu  bool     `json:"zaman_dolu"`
	}
	if b, err := os.ReadFile(z.Durum); err == nil && json.Unmarshal(b, &kd) == nil && kd.Sinir > 0 && kd.MB != nil && *kd.MB >= 0 {
		s += fmt.Sprintf("\n💾 %.0f / %d MB", *kd.MB, kd.Sinir)
		if kd.Dolu {
			s += " · BUGÜN DOLDU: yeni kare eklenmiyor"
		} else {
			s += " · sınır aşılınca en eski gün silinir"
		}
	}
	if dosyaVar(kalkanBayrak) {
		s += "\n🛡 Gizlilik kalkanı açık: kalkan kapanana kadar kare alınmaz."
	}
	return s
}

func zamanDugmeleri(z zamanYollar) map[string]any {
	ar := zamanAralikOku(z.Aralik)
	k := func(sn int) map[string]string {
		m := aralikMetni(sn)
		if sn == ar {
			m = "✓ " + m
		}
		return tgK(m, fmt.Sprintf("/zaman %d", sn))
	}
	return tgSatirIci(
		tgSatir(tgK("✅ Aç", "/zaman ac"), tgK("⏹ Kapat", "/zaman kapat")),
		tgSatir(k(1), k(3), k(10), k(60)),
		tgSatir(tgK("🔎 Olay tespiti aç", "/zaman olay ac"), tgK("Olay kapat", "/zaman olay kapat")))
}

// zamanKomut: /zaman argümanları (küçük harf, alanlara bölünmüş) -> cevap
func zamanKomut(z zamanYollar, arg []string) string {
	if len(arg) == 0 {
		return zamanDurumMetni(z)
	}
	kalkanNot := ""
	if dosyaVar(kalkanBayrak) {
		kalkanNot = " Gizlilik kalkanı açık: kalkan kapanınca başlar."
	}
	a := arg[0]
	switch {
	case a == "ac" || a == "aç" || a == "kapat":
		ac := a != "kapat"
		if err := bayrak(z.Acik, ac); err != nil {
			return "Ayarlayamadım efendim: SD kart yazılamıyor."
		}
		if !ac {
			return "⏹ Zaman atlamalı çekim kapatıldı efendim."
		}
		s := "⏱ Zaman atlamalı çekim açıldı efendim (her " + aralikMetni(zamanAralikOku(z.Aralik))
		if dosyaVar(z.Olay) {
			s += ", olay tespiti açık"
		}
		return s + ")." + kalkanNot
	case a == "olay":
		ac := !dosyaVar(z.Olay)
		if len(arg) > 1 {
			switch arg[1] {
			case "ac", "aç":
				ac = true
			case "kapat":
				ac = false
			default:
				return zamanKullanim
			}
		}
		if err := bayrak(z.Olay, ac); err != nil {
			return "Ayarlayamadım efendim: SD kart yazılamıyor."
		}
		if !ac {
			return "🔎 Time-lapse olay tespiti kapatıldı efendim."
		}
		s := "🔎 Time-lapse olay tespiti açıldı efendim: hareket anları time-lapse'e ek kare olarak eklenir, fotoğrafı buraya gelir."
		if !dosyaVar(z.Acik) {
			s += " Time-lapse şu an kapalı; açınca çalışır (/zaman ac)."
		}
		return s + kalkanNot
	}
	if a == "aralik" || a == "aralık" {
		if len(arg) < 2 {
			return zamanKullanim
		}
		a = strings.Join(arg[1:], "")
	} else {
		a = strings.Join(arg, "")
	}
	n := zamanAralikCoz(a)
	if n == 0 {
		return zamanKullanim
	}
	if err := os.WriteFile(z.Aralik, []byte(strconv.Itoa(n)+"\n"), 0644); err != nil {
		return "Ayarlayamadım efendim: SD kart yazılamıyor."
	}
	s := "⏱ Time-lapse kare aralığı " + aralikMetni(n) + " oldu efendim."
	if n <= 3 {
		s += " (Günde yaklaşık " + map[bool]string{true: "2.5-3.5 GB", false: "1 GB"}[n == 1] +
			"; 4 GB sınırında en eski gün silinir.)"
	}
	if !dosyaVar(z.Acik) {
		s += " Çekim kapalı; açmak için /zaman ac."
	}
	return s
}
