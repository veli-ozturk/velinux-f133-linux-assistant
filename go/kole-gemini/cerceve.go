// cerceve.go (15.10.2026, Claude): kole-gemini v8 — Telegram'dan gelen fotoğrafı dijital çerçeve klasörüne
// (/mnt/sd/fotograf, panel 0.37 cerceve_v1.h okur) koyar. Panel tjpgd ile çözer: YALNIZ baseline JPEG.
// Progresif JPEG ve PNG burada Go image/jpeg ile baseline'a çevrilir (kalite 88); baseline JPEG olduğu gibi yazılır.
// Yazım .part + rename (yarım dosyayı panel görmez). Çözülecek görüntü büyükse (JPEG >6.6 MP, PNG >3 MP) bellek için reddedilir.
package main

import (
	"bytes"
	"errors"
	"fmt"
	"image"
	"image/jpeg"
	_ "image/png"
	"os"
	"path/filepath"
	"strings"
	"time"
)

const (
	cerceveDizin = "/mnt/sd/fotograf"
	cerceveAzami = 10 << 20 // indirme sınırı (Telegram bot getFile en çok 20 MB)
	// Telegram döngüsü GOMEMLIMIT=12MiB, cihazda ~18 MB boş: yalnız çözülmesi gerekenler (progresif/PNG) sınırlanır
	cerceveAzamiP   = 6600000 // JPEG piksel (2560x2560)
	cerceveAzamiPNG = 3000000 // PNG piksel
)

// jpegTuru: "baseline", "progresif", "diger" (SOF3/5.. kayıpsız/hiyerarşik) ya da "" (JPEG değil / bozuk)
func jpegTuru(b []byte) string {
	if len(b) < 4 || b[0] != 0xFF || b[1] != 0xD8 {
		return ""
	}
	i := 2
	for i+4 <= len(b) {
		if b[i] != 0xFF {
			return ""
		}
		m := b[i+1]
		if m == 0xFF { // dolgu
			i++
			continue
		}
		if m == 0xD8 || m == 0x01 || (m >= 0xD0 && m <= 0xD7) {
			i += 2
			continue
		}
		n := int(b[i+2])<<8 | int(b[i+3])
		switch m {
		case 0xC0, 0xC1:
			return "baseline"
		case 0xC2:
			return "progresif"
		case 0xC3, 0xC5, 0xC6, 0xC7, 0xC9, 0xCA, 0xCB, 0xCD, 0xCE, 0xCF:
			return "diger"
		case 0xD9, 0xDA: // SOF'tan önce görüntü sonu / tarama
			return ""
		}
		if n < 2 {
			return ""
		}
		i += 2 + n
	}
	return ""
}

// cerceveHazirla: panelin çözebileceği baseline JPEG baytlarını ve yapılan işlemi döndürür
func cerceveHazirla(veri []byte) ([]byte, string, error) {
	tur := jpegTuru(veri)
	if tur == "baseline" {
		return veri, "baseline", nil
	}
	cfg, bicim, err := image.DecodeConfig(bytes.NewReader(veri))
	if err != nil {
		return nil, "", fmt.Errorf("görüntü tanınmadı")
	}
	sinir := cerceveAzamiP // YCbCr 4:2:0 ~1.5 B/piksel
	if bicim != "jpeg" {
		sinir = cerceveAzamiPNG // NRGBA 4 B/piksel
	}
	if cfg.Width*cfg.Height > sinir {
		return nil, "", fmt.Errorf("görüntü çok büyük (%dx%d); normal fotoğraf olarak gönderin", cfg.Width, cfg.Height)
	}
	img, _, err := image.Decode(bytes.NewReader(veri))
	if err != nil {
		return nil, "", fmt.Errorf("görüntü çözülemedi")
	}
	var g bytes.Buffer
	if err := jpeg.Encode(&g, img, &jpeg.Options{Quality: 88}); err != nil {
		return nil, "", err
	}
	if bicim == "jpeg" {
		bicim = tur
	}
	return g.Bytes(), bicim + "->baseline", nil
}

// cerceveKaydet: dizine tg-YYYYMMDD-HHMMSS[-n].jpg yazar; yolu ve işlemi döndürür
func cerceveKaydet(dizin string, veri []byte, simdi time.Time) (string, string, error) {
	jpg, islem, err := cerceveHazirla(veri)
	if err != nil {
		return "", "", err
	}
	if err := os.MkdirAll(dizin, 0755); err != nil {
		return "", "", errors.New("SD kart yazılamıyor")
	}
	ad := "tg-" + simdi.Format("20060102-150405")
	yol := filepath.Join(dizin, ad+".jpg")
	for k := 2; dosyaVar(yol) && k < 100; k++ {
		yol = filepath.Join(dizin, fmt.Sprintf("%s-%d.jpg", ad, k))
	}
	tmp := yol + ".part"
	if err := os.WriteFile(tmp, jpg, 0644); err != nil {
		os.Remove(tmp)
		return "", "", errors.New("SD kart yazılamıyor")
	}
	if err := os.Rename(tmp, yol); err != nil {
		os.Remove(tmp)
		return "", "", errors.New("SD kart yazılamıyor")
	}
	return yol, islem, nil
}

// cerceveSay: dizindeki .jpg/.jpeg sayısı (panel en çok 400 tanesini listeler)
func cerceveSay(dizin string) int {
	es, err := os.ReadDir(dizin)
	if err != nil {
		return 0
	}
	n := 0
	for _, e := range es {
		a := strings.ToLower(e.Name())
		if !e.IsDir() && (strings.HasSuffix(a, ".jpg") || strings.HasSuffix(a, ".jpeg")) {
			n++
		}
	}
	return n
}
