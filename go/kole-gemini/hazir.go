// hazir.go (20.10.2026, Claude): v15 HAZIR SESLER. kullanıcı: "hey köle deyince default olarak gelen sesleri kaydet, onlar
// da konuşsun; mesela 'bir saniye düşünüyorum' sessiz". kole-asistan v10'un sabit cümleleri geçerli sesle BİR KEZ
// üretilip SD'ye yazılır; asistan cümleyi gösterirken aynı adı (FNV-1a 32) hesaplayıp köprüden çalar (scripts/357).
package main

import (
	"context"
	"fmt"
	"os"
	"path/filepath"
	"strings"
	"time"
)

var (
	hazirKok   = "/mnt/sd/velinux/asistan" // SD bağlı değilse üretilmez
	hazirDizin = "/mnt/sd/velinux/asistan/sesler"
	hazirKilit = "/tmp/hazir-ses-uretim"
	hazirAzami = 8               // bir çalıştırmada en çok (362: TTS kotası cevaplara kalsın)
	hazirAra   = 2 * time.Second // istekler arası
)

// kole-asistan v10 ile BİREBİR aynı metinler (host provası 361 karşılaştırır). Sıra = üretim önceliği.
var hazirCumleler = []string{
	"Bir saniye efendim, düşünüyorum...",
	"Buyurun efendim.", "Emredin efendim.", "Sizi dinliyorum efendim.", "Buyurun efendim, emrinizdeyim.",
	"Sadık köleniz burada efendim.",
	"Bir şey duyamadım efendim. Gerekirse yine seslenin.",
	"Üzgünüm efendim, şu an bağlantı kuramıyorum.",
	"Üzgünüm efendim, cevap çok gecikti. Bir daha sorar mısınız?",
	"Şu an cevap veremiyorum efendim.",
	"Sorunuzu kaydedemedim efendim.",
	"Buyurunuz efendim, hizmetinizdeyim.", "Emirlerinizi bekliyorum efendim.", "Sizi dinliyorum, efendim.",
	"Emredersiniz efendim.",
	"Bip bop! Buyurun efendim!", "Kölenizi çağırdınız mı efendim? Buradayım!", "Emret efendim, hemen hallederim!",
	"Hazır ve nazırım efendim!",
}

func fnv1a32(s string) uint32 {
	h := uint32(2166136261)
	for i := 0; i < len(s); i++ {
		h ^= uint32(s[i])
		h *= 16777619
	}
	return h
}

func hazirYol(metin string) string {
	return filepath.Join(hazirDizin, fmt.Sprintf("%08x.pcm", fnv1a32(metin)))
}

// ses adı + stil özeti: biri değişince eski sesler çalınmasın
func hazirDamga() string {
	return fmt.Sprintf("%s %08x", ttsSesAdi(), fnv1a32(oku(ttsStilYol, ttsStilVarsayilan)))
}

func hazirGunluk(f string, a ...any) {
	if g, err := ttsGunlukAc(); err == nil { // v42: 64 KB sınırı (yalnız /ses yolu kırpıyordu)
		fmt.Fprintf(g, time.Now().Format("15:04:05")+" "+f+"\n", a...)
		g.Close()
	}
}

func hazirSil() {
	es, _ := filepath.Glob(filepath.Join(hazirDizin, "*.pcm"))
	for _, y := range es {
		_ = os.Remove(y)
	}
	_ = os.Remove(filepath.Join(hazirDizin, "ses"))
}

func hazirEksik() int {
	n := 0
	for _, m := range hazirCumleler {
		if !dosyaVar(hazirYol(m)) {
			n++
		}
	}
	return n
}

// eksik hazır sesleri sırayla üret; üretilen sayısı. Hepsi varsa ve damga geçerliyse ağ kullanmaz.
func hazirSeslerHazirla(anahtarYol string) int {
	if !ttsAcik() || !dosyaVar(hazirKok) {
		return 0
	}
	if n := sessizGoc(hazirDizin, selamPCMYol); n >= 0 { // v44: eski sesler bir kez kırpılır
		hazirGunluk("hazır ses: %d dosyanın sessizliği kırpıldı (v44)", n)
	}
	damgaYol := filepath.Join(hazirDizin, "ses")
	dm := hazirDamga()
	if oku(damgaYol, "") == dm && hazirEksik() == 0 {
		return 0
	}
	if b, kalan := kotaBekliyor(); b { // v16: 429 retryDelay bitmeden istek yok
		hazirGunluk("hazır ses: kota beklemesi %d sn, sonraki tura", kalan)
		return 0
	}
	anahtar := oku(anahtarYol, "")
	if anahtar == "" {
		return 0
	}
	if st, err := os.Stat(hazirKilit); err == nil && time.Since(st.ModTime()) > 10*time.Minute {
		_ = os.Remove(hazirKilit)
	}
	kf, err := os.OpenFile(hazirKilit, os.O_CREATE|os.O_EXCL|os.O_WRONLY, 0644)
	if err != nil {
		return 0 // başka süreç üretiyor
	}
	fmt.Fprintf(kf, "%d\n", os.Getpid())
	kf.Close()
	defer os.Remove(hazirKilit)
	if oku(damgaYol, "") != dm {
		hazirSil()
		if err := os.MkdirAll(hazirDizin, 0755); err != nil {
			hazirGunluk("hazır ses: dizin yok: %v", err)
			return 0
		}
		if err := os.WriteFile(damgaYol, []byte(dm+"\n"), 0644); err != nil {
			hazirGunluk("hazır ses: damga yazılamadı: %v", err)
			return 0
		}
	}
	t0, n := time.Now(), 0
	var liste strings.Builder
	for _, m := range hazirCumleler {
		y := hazirYol(m)
		fmt.Fprintf(&liste, "%08x\t%s\n", fnv1a32(m), m)
		if dosyaVar(y) {
			continue
		}
		if n >= hazirAzami {
			break // kalanlar sonraki --veri (10 dk)
		}
		if n > 0 {
			time.Sleep(hazirAra)
		}
		if hazirDamga() != dm {
			hazirGunluk("hazır ses: ses değişti, durdu (%d yeni)", n)
			return n
		}
		ctx, iptal := context.WithTimeout(context.Background(), 25*time.Second)
		x, _, err := ttsUretGemini(ctx, anahtar, m) // v17: yalniz Gemini (ses tutarliligi)
		iptal()
		if err == nil && len(x) == 0 {
			err = fmt.Errorf("boş ses")
		}
		if err != nil {
			hazirGunluk("hazır ses: üretilemedi (%d yeni, %d eksik): %v", n, hazirEksik(), err)
			return n
		}
		if err := dosyaYazAtomik(y, pcmHazirla(x), 0644); err != nil {
			hazirGunluk("hazır ses: yazılamadı: %v", err)
			return n
		}
		n++
	}
	if n < hazirAzami {
		_ = dosyaYazAtomik(filepath.Join(hazirDizin, "liste.txt"), []byte(liste.String()), 0644)
	}
	if n > 0 {
		hazirGunluk("hazır ses: %s ile %d cümle üretildi (%d ms), eksik %d", ttsSesAdi(), n, time.Since(t0).Milliseconds(),
			hazirEksik())
	}
	return n
}
