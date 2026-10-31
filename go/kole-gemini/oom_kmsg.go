// oom_kmsg.go (23.10.2026, Claude; kole-gemini v37, madde 125): OOM bildiriminde kapanan süreç ve tetikleyen.
// kullanıcı 23.10: "telegram bottan mesaj geliyor çekirdek 1 süreci kapattı diye ne olabilir?" (§124: parçalanma, order 2-3,
// kurban hep kendini yeniden başlatan kole-gemini --telegram). Çekirdek günlüğü /dev/kmsg'den okunur: O_NONBLOCK, ham
// syscall (Go'nun poller'ı EAGAIN'de beklerdi), her read bir kayıt "pri,sira,zaman,-;metin". Süreç AÇILMAZ (dmesg yok):
// OOM'u tetikleyenlerden biri tam da fork'tu.
package main

import (
	"fmt"
	"strconv"
	"strings"
	"syscall"
)

var kmsgYol = "/dev/kmsg"

type oomOlay struct {
	Kurban string // comm (çekirdek 15 karakterde keser)
	Tetik  string // "X invoked oom-killer" içindeki X
	Derece int    // order (-1 bilinmiyor)
}

// oomSatirlari: kmsg kayıt metinlerinden OOM olayları (sırayla)
func oomSatirlari(kayitlar []string) []oomOlay {
	var o []oomOlay
	tetik, derece := "", -1
	for _, k := range kayitlar {
		if i := strings.IndexByte(k, ';'); i >= 0 && strings.IndexByte(k[:i], ',') >= 0 {
			k = k[i+1:] // "6,1234,19317024660,-;" öneki
		}
		if i := strings.Index(k, " invoked oom-killer:"); i >= 0 {
			tetik, derece = strings.TrimSpace(k[:i]), -1
			if j := strings.Index(k, "order="); j >= 0 {
				s := k[j+len("order="):]
				if e := strings.IndexAny(s, ", "); e >= 0 {
					s = s[:e]
				}
				if n, err := strconv.Atoi(s); err == nil {
					derece = n
				}
			}
			continue
		}
		if i := strings.Index(k, "Killed process "); i >= 0 {
			s := k[i:]
			a, b := strings.IndexByte(s, '('), strings.IndexByte(s, ')')
			if a < 0 || b <= a {
				continue
			}
			o = append(o, oomOlay{Kurban: s[a+1 : b], Tetik: tetik, Derece: derece})
			tetik, derece = "", -1
		}
	}
	return o
}

// oomOlayOku: kmsg'deki son n OOM olayı (okunamazsa nil). Test değiştirir.
var oomOlayOku = func(n int) []oomOlay {
	if n <= 0 {
		return nil
	}
	fd, err := syscall.Open(kmsgYol, syscall.O_RDONLY|syscall.O_NONBLOCK|syscall.O_CLOEXEC, 0)
	if err != nil {
		return nil
	}
	defer syscall.Close(fd)
	var kayit []string
	buf := make([]byte, 8192)
	for i := 0; i < 20000; i++ { // 256 KB halka: en çok birkaç bin kayıt
		k, err := syscall.Read(fd, buf)
		if err == syscall.EPIPE { // okurken üzerine yazıldı: sonraki kayıttan sürer
			continue
		}
		if err != nil || k <= 0 { // EAGAIN: sona gelindi (düz dosyada 0)
			break
		}
		for _, s := range strings.Split(string(buf[:k]), "\n") { // kmsg: bir kayıt (+ devam satırları); düz dosya: hepsi
			if strings.Contains(s, "oom-killer") || strings.Contains(s, "Killed process") {
				kayit = append(kayit, s)
			}
		}
	}
	o := oomSatirlari(kayit)
	if len(o) > n {
		o = o[len(o)-n:]
	}
	return o
}

func oomSurecAdi(c string) string {
	switch {
	case strings.HasPrefix(c, "kole-gemini"):
		return "Telegram/Hey Köle istemcisi"
	case c == "panel":
		return "panel"
	case strings.HasPrefix(c, "kamera-kayit"):
		return "kamera kaydı"
	case strings.HasPrefix(c, "tailscale"):
		return "uzaktan erişim (Tailscale)"
	case strings.HasPrefix(c, "wifi-ekran"):
		return "PC ikinci ekranı"
	case strings.HasPrefix(c, "kole-cihaz-mer"):
		return "KÖLE Merkez"
	case strings.HasPrefix(c, "kole-asistan"):
		return "Hey Köle dinleyicisi"
	}
	return c
}

func oomTetikMetni(o oomOlay) string {
	if o.Tetik == "" {
		return ""
	}
	blok := ""
	if o.Derece >= 0 && o.Derece < 11 {
		blok = fmt.Sprintf(" %d KB bitişik blok", 4<<o.Derece)
	}
	if strings.HasPrefix(o.Tetik, "ssv6") {
		return "Wi-Fi sürücüsü" + blok + " istedi"
	}
	return o.Tetik + " süreci" + blok + " istedi"
}

// oomMesaji: n kapatma, olaylar kmsg'den (boş olabilir)
func oomMesaji(n int64, olaylar []oomOlay) string {
	if len(olaylar) == 0 {
		return fmt.Sprintf("⚠️ Bellek yetmedi: çekirdek %d süreci kapattı (OOM). /saglik ile hizmetlere bakın.", n)
	}
	var adlar, tetikler []string
	hepsiKendi := true
	for _, o := range olaylar {
		adlar = append(adlar, oomSurecAdi(o.Kurban)+" ("+o.Kurban+")")
		if t := oomTetikMetni(o); t != "" && !strings.Contains(strings.Join(tetikler, "|"), t) {
			tetikler = append(tetikler, t)
		}
		hepsiKendi = hepsiKendi && strings.HasPrefix(o.Kurban, "kole-gemini")
	}
	m := fmt.Sprintf("Bellek parçalandı: çekirdek %d süreci kapattı (OOM): %s.", n, strings.Join(adlar, ", "))
	if len(tetikler) > 0 {
		m += " Tetikleyen: " + strings.Join(tetikler, "; ") + "."
	}
	if hepsiKendi {
		return "ℹ️ " + m + " Kendiliğinden yeniden başladı, işlem gerekmez."
	}
	return "⚠️ " + m + " /saglik ile hizmetlere bakın."
}
