package main

// veri.go (21.10.2026, Claude; v28, scripts/476; OTONOM Sıra 9 madde 67): Pano verisi (hava, döviz, namaz, haber)
// SÜREKLİ --telegram sürecinin içinde. Önce autorun 10 dk'da bir AYRI bir --veri süreci başlatıyordu: azami RSS ~12 MB
// (host ölçümü; cihazda boş bellek ~19 MB), çalışma zamanı + kök sertifikalar her seferinde baştan. Süreç içinde
// (tests/kole_gemini_veri_olcum_v1) ilk çağrı +5.4 MB, sonrakiler +0.3 MB; Telegram süreci TLS'i zaten yüklü tutar.
//   - Telegram süreci 30 sn'de bir <veri>.nabiz dosyasına dokunur; dosya 10 dk'dan eskiyse (ya da yoksa) ve son deneme
//     (<veri>.deneme) 10 dk'dan eskiyse veriYaz çalışır. Aralık eskisi gibi 10 dk; başarısız deneme 10 dk sonra yinelenir
//     (süreç kısa turlarla çıkıp yeniden başlasa bile, deneme damgası dosyada).
//   - --veri: nabız 2 dk'dan tazeyse ağa çıkmaz (yalnız eksik hazır sesler); değilse eskisi gibi (Telegram jetonu yoksa
//     ya da süreç ölmüşse Pano verisi yine 10 dk'da bir gelir). autorun değişmedi.

import (
	"fmt"
	"os"
	"sync"
	"time"
)

var (
	veriDosya  = "/tmp/panel-veri.json"
	veriAralik = 10 * time.Minute // Pano verisi yenileme aralığı (autorun --veri döngüsüyle aynı)
	veriBakis  = 30 * time.Second // Telegram sürecinin bakma ve nabız aralığı
	veriNabiz  = 2 * time.Minute  // --veri bu yaştan taze nabızda ağa çıkmaz
	veriFn     = veriYaz
)

// dosyaYasi: değişiklik zamanından bu yana geçen süre; dosya yoksa ya da zamanı 1 dk'dan fazla ilerideyse (saat geri
// atladı: NTP) ok=false (eski sayılır).
func dosyaYasi(yol string) (time.Duration, bool) {
	st, err := os.Stat(yol)
	if err != nil {
		return 0, false
	}
	y := time.Since(st.ModTime())
	if y < -time.Minute {
		return 0, false
	}
	return y, true
}

func dokun(yol string) {
	simdi := time.Now()
	if os.Chtimes(yol, simdi, simdi) != nil {
		_ = os.WriteFile(yol, nil, 0644)
	}
}

// veriSirasi: Telegram süreci şimdi veriYaz çalıştırmalı mı?
func veriSirasi(yol string) bool {
	if y, ok := dosyaYasi(yol); ok && y < veriAralik {
		return false
	}
	if y, ok := dosyaYasi(yol + ".deneme"); ok && y < veriAralik {
		return false
	}
	return true
}

// veriAtla: --veri ağa çıkmasın mı (sürekli Telegram süreci yaşıyor)?
func veriAtla(yol string) bool {
	y, ok := dosyaYasi(yol + ".nabiz")
	return ok && y < veriNabiz
}

func veriTur(yol, sehirYol string) {
	dokun(yol + ".nabiz")
	if !veriSirasi(yol) {
		return
	}
	dokun(yol + ".deneme")
	if err := veriFn(yol, sehirYol); err != nil {
		fmt.Fprintln(os.Stderr, "veri:", err)
	}
}

// veriBaslat: telegramDinle'nin arka plan işi; dönen işlev durdurur ve süren veriYaz'ı (en çok 25 sn) bekler.
func veriBaslat(sehirYol string) func() {
	if veriDosya == "" {
		return func() {}
	}
	dur := make(chan struct{})
	var wg sync.WaitGroup
	wg.Add(1)
	go func() {
		defer wg.Done()
		for {
			veriTur(veriDosya, sehirYol)
			select {
			case <-dur:
				return
			case <-time.After(veriBakis):
			}
		}
	}()
	return func() {
		close(dur)
		wg.Wait()
	}
}
