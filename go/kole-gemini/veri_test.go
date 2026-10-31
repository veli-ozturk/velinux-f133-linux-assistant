package main

// veri_test.go (21.10.2026, Claude; v28, scripts/476): Pano verisinin Telegram sürecine taşınması.

import (
	"errors"
	"os"
	"path/filepath"
	"sync/atomic"
	"testing"
	"time"
)

func veriAyar(t *testing.T, aralik, bakis, nabiz time.Duration, fn func(string, string) error) string {
	t.Helper()
	ed, ea, eb, en, ef := veriDosya, veriAralik, veriBakis, veriNabiz, veriFn
	veriDosya = filepath.Join(t.TempDir(), "panel-veri.json")
	veriAralik, veriBakis, veriNabiz, veriFn = aralik, bakis, nabiz, fn
	t.Cleanup(func() { veriDosya, veriAralik, veriBakis, veriNabiz, veriFn = ed, ea, eb, en, ef })
	return veriDosya
}

func yaz(t *testing.T, yol string, yas time.Duration) {
	t.Helper()
	if err := os.WriteFile(yol, []byte("{}"), 0644); err != nil {
		t.Fatal(err)
	}
	z := time.Now().Add(-yas)
	if err := os.Chtimes(yol, z, z); err != nil {
		t.Fatal(err)
	}
}

func TestVeriSirasi(t *testing.T) {
	yol := veriAyar(t, 10*time.Minute, time.Second, 2*time.Minute, nil)
	if !veriSirasi(yol) {
		t.Fatal("dosya yok: çalışmalı")
	}
	yaz(t, yol, 9*time.Minute)
	if veriSirasi(yol) {
		t.Fatal("9 dk'lık veri: beklemeli")
	}
	yaz(t, yol, 11*time.Minute)
	if !veriSirasi(yol) {
		t.Fatal("11 dk'lık veri: çalışmalı")
	}
	yaz(t, yol+".deneme", 3*time.Minute)
	if veriSirasi(yol) {
		t.Fatal("3 dk önce denenmiş (başarısız): beklemeli")
	}
	yaz(t, yol+".deneme", 10*time.Minute+time.Second)
	if !veriSirasi(yol) {
		t.Fatal("deneme 10 dk'dan eski: çalışmalı")
	}
	yaz(t, yol, -time.Hour) // saat geri atladı: dosya gelecekte -> eski say
	if !veriSirasi(yol) {
		t.Fatal("gelecekteki veri zamanı eski sayılmalı")
	}
}

func TestVeriAtla(t *testing.T) {
	yol := veriAyar(t, 10*time.Minute, time.Second, 2*time.Minute, nil)
	if veriAtla(yol) {
		t.Fatal("nabız yok: --veri eskisi gibi çalışmalı")
	}
	yaz(t, yol+".nabiz", 30*time.Second)
	if !veriAtla(yol) {
		t.Fatal("taze nabız: --veri ağa çıkmamalı")
	}
	yaz(t, yol+".nabiz", 3*time.Minute)
	if veriAtla(yol) {
		t.Fatal("eski nabız (Telegram süreci öldü): --veri çalışmalı")
	}
	yaz(t, yol+".nabiz", -time.Hour)
	if veriAtla(yol) {
		t.Fatal("gelecekteki nabız eski sayılmalı")
	}
}

// Telegram süreci: dosya yoksa hemen yazar, aralık içinde bir daha yazmaz, aralık dolunca yazar; nabız her bakışta.
func TestVeriBaslatAralik(t *testing.T) {
	var n atomic.Int32
	yol := veriAyar(t, 300*time.Millisecond, 20*time.Millisecond, time.Second, func(y, _ string) error {
		n.Add(1)
		return os.WriteFile(y, []byte("{}"), 0644)
	})
	dur := veriBaslat("")
	defer dur()
	bekle := func(kosul func() bool) bool { // v29: yük altında sabit uyku yetmiyordu (WSL yük 4: ilk yazım > 150 ms)
		for son := time.Now().Add(2 * time.Second); time.Now().Before(son); time.Sleep(5 * time.Millisecond) {
			if kosul() {
				return true
			}
		}
		return false
	}
	if !bekle(func() bool { return n.Load() >= 1 }) {
		t.Fatal("ilk yazım 2 sn'de gelmedi")
	}
	if y, ok := dosyaYasi(yol + ".nabiz"); !ok || y > 250*time.Millisecond {
		t.Fatalf("nabız taze değil: %v %v", y, ok)
	}
	time.Sleep(150 * time.Millisecond) // aralık (300 ms) dolmadan ikinci yazım olmamalı
	if k := n.Load(); k != 1 {
		t.Fatalf("aralık dolmadan %d yazım", k)
	}
	if !bekle(func() bool { return n.Load() >= 2 }) {
		t.Fatal("aralık dolunca ikinci yazım gelmedi")
	}
}

// Başarısız deneme her bakışta değil, aralık dolunca yinelenir (ağ yokken 30 sn'de bir 25 sn'lik istek olmasın).
func TestVeriBaslatHataGeriCekilme(t *testing.T) {
	var n atomic.Int32
	veriAyar(t, 300*time.Millisecond, 20*time.Millisecond, time.Second, func(string, string) error {
		n.Add(1)
		return errors.New("ag yok")
	})
	dur := veriBaslat("")
	time.Sleep(200 * time.Millisecond)
	dur()
	if k := n.Load(); k != 1 {
		t.Fatalf("200 ms'de %d deneme (1 bekleniyordu)", k)
	}
	// süreç yeniden başlasa da (autorun 15 sn sonra) deneme damgası dosyada: hemen yinelenmez
	dur = veriBaslat("")
	time.Sleep(50 * time.Millisecond)
	dur()
	if k := n.Load(); k != 1 {
		t.Fatalf("yeni süreç hemen yeniden denedi: %d", k)
	}
}

// --veri yazdıysa (Telegram süreci sonradan başladı) Telegram aralık dolmadan yazmaz.
func TestVeriBaslatTazeDosya(t *testing.T) {
	var n atomic.Int32
	yol := veriAyar(t, 300*time.Millisecond, 20*time.Millisecond, time.Second, func(string, string) error {
		n.Add(1)
		return nil
	})
	yaz(t, yol, 0)
	dur := veriBaslat("")
	time.Sleep(150 * time.Millisecond)
	dur()
	if k := n.Load(); k != 0 {
		t.Fatalf("taze dosyada %d yazım", k)
	}
}

// Durdurma süren veriYaz'ı bekler (yarım .part kalmasın).
func TestVeriBaslatDurBekler(t *testing.T) {
	var bitti atomic.Bool
	veriAyar(t, time.Minute, time.Minute, time.Second, func(string, string) error {
		time.Sleep(80 * time.Millisecond)
		bitti.Store(true)
		return nil
	})
	dur := veriBaslat("")
	time.Sleep(10 * time.Millisecond)
	dur()
	if !bitti.Load() {
		t.Fatal("durdurma süren veriYaz'ı beklemedi")
	}
}

// veriDosya boşsa (testler, eski çağrılar) arka plan işi açılmaz.
func TestVeriBaslatKapali(t *testing.T) {
	var n atomic.Int32
	veriAyar(t, time.Millisecond, time.Millisecond, time.Second, func(string, string) error { n.Add(1); return nil })
	veriDosya = ""
	dur := veriBaslat("")
	time.Sleep(20 * time.Millisecond)
	dur()
	if n.Load() != 0 {
		t.Fatal("veriDosya boşken çalıştı")
	}
}
