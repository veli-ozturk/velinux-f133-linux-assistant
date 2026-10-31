// tg_tek.go (23.10.2026, Claude; kole-gemini v41, scripts/586; Sıra 18 madde 135 A1): Telegram'ın sürekli döngüsü C'de
// (kole-telgraf-v1: uzun sorgu, offset, sahip denetimi, bildirim kuyruğu). Go yalnız iş geldikçe TEK ATIŞ çalışır:
//
//	--tg-isle <dosya>  tek güncelleme (getUpdates sonucundaki bir öğe, JSON) -> tgGuncellemeIsle AYNEN. Cevap gidince
//	                   KOLE_TG_HAZIR_FD (C'nin borusu) "hazir" yazılıp kapanır: C sıradaki güncellemeye geçer; süreç /canli
//	                   izlemesi ve ses akışı (arkaIs) bitene kadar yaşar.
//	--tg-zamanli       mikrofon bayrağı temizliği, menü kurulumu + logo (bir kez), zamanlanmisIsler (sabah özeti, hizmet /
//	                   bellek / OOM uyarıları), ölü canlı oturum dosyaları. C dakikada bir çalıştırır.
//
// İkisi de getUpdates GÖNDERMEZ (409 olmasın): jeton ve sohbet dosyadan (sohbet yoksa C ilk mesajdan yazar).
package main

import (
	"context"
	"encoding/json"
	"fmt"
	"os"
	"strconv"
	"time"
)

var (
	tgHazirFd  = "KOLE_TG_HAZIR_FD"
	tgIsleSure = 150 * time.Second
)

// tgHazirBildir: C'ye "cevap gitti, sıradakine geç" (boru; yoksa hiçbir şey). Bir kez yazar.
func tgHazirBildir() {
	n, err := strconv.Atoi(os.Getenv(tgHazirFd))
	if err != nil || n < 3 {
		return
	}
	_ = os.Unsetenv(tgHazirFd)
	f := os.NewFile(uintptr(n), "tg-hazir")
	if f == nil {
		return
	}
	_, _ = f.WriteString("hazir\n")
	_ = f.Close()
}

func tgTekBaglam() (jeton, sahip string, ok bool) {
	jeton, sahip = oku(telegramJeton, ""), oku(telegramSohbet, "")
	return jeton, sahip, jeton != "" && sahip != ""
}

// tgIsleIc: bir güncelleme JSON'u (test edilen çekirdek)
func tgIsleIc(ctx context.Context, jeton, sahip string, b []byte, sehirYol string) error {
	var u tgGuncelleme
	if err := json.Unmarshal(b, &u); err != nil {
		return err
	}
	tgGuncellemeIsle(ctx, jeton, sahip, u, sehirYol)
	return nil
}

// arkaIsBekle: süren ses akışı / soru işi (soru_sunucu.go arkaIs)
func arkaIsBekle(en time.Duration) {
	bitti := make(chan struct{})
	go func() { arkaIs.Wait(); close(bitti) }()
	select {
	case <-bitti:
	case <-time.After(en):
		fmt.Fprintln(os.Stderr, "tg: süren iş bitmedi")
	}
}

func tgIsleTek(yol, sehirYol string) int {
	b, err := os.ReadFile(yol)
	_ = os.Remove(yol)
	if err != nil {
		tgHazirBildir()
		fmt.Fprintln(os.Stderr, "tg-isle:", err)
		return 2
	}
	jeton, sahip, ok := tgTekBaglam()
	if !ok {
		tgHazirBildir()
		fmt.Fprintln(os.Stderr, "tg-isle: jeton ya da sohbet yok")
		return 9
	}
	ctx, iptal := context.WithTimeout(context.Background(), tgIsleSure)
	defer iptal()
	err = tgIsleIc(ctx, jeton, sahip, b, sehirYol)
	tgHazirBildir()
	if err != nil {
		fmt.Fprintln(os.Stderr, "tg-isle: güncelleme okunamadı:", err)
		return 2
	}
	canliBitmesiniBekle()
	arkaIsBekle(90 * time.Second)
	return 0
}

func tgZamanliTek(sehirYol string) int {
	mikBirakTemizle()
	canliPidTemizle()
	canliEskiTemizle()
	jeton, sahip, ok := tgTekBaglam()
	if !ok {
		fmt.Fprintln(os.Stderr, "tg-zamanli: jeton ya da sohbet yok")
		return 9
	}
	ctx, iptal := context.WithTimeout(context.Background(), 90*time.Second)
	defer iptal()
	tgKurulumYap(ctx, jeton, sahip)
	if err := tgLogoAyarla(ctx, jeton, false); err != nil {
		fmt.Fprintln(os.Stderr, "logo:", err)
	}
	zamanlanmisIsler(ctx, jeton, sahip, sehirYol)
	arkaIsBekle(60 * time.Second)
	return 0
}
