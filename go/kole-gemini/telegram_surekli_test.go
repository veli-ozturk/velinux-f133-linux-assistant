package main

// telegram_surekli_test.go (21.10.2026, Claude; v25, scripts/447): --telegram aynı süreçte tur tekrarı.

import (
	"context"
	"errors"
	"io"
	"net"
	"net/http"
	"net/http/httptest"
	"sync"
	"testing"
	"time"
)

func surekliAyar(t *testing.T, butce, pay, kisa time.Duration, tur func(string) error) {
	t.Helper()
	eb, ep, ek, ef, ev := tgSurecButce, tgTurPayi, tgEnKisaTur, tgTurFn, veriDosya
	tgSurecButce, tgTurPayi, tgEnKisaTur, tgTurFn, veriDosya = butce, pay, kisa, tur, "" // v28: gerçek Pano verisi yok
	t.Cleanup(func() { tgSurecButce, tgTurPayi, tgEnKisaTur, tgTurFn, veriDosya = eb, ep, ek, ef, ev })
}

func TestTelegramDinleButce(t *testing.T) {
	n := 0
	surekliAyar(t, 400*time.Millisecond, 100*time.Millisecond, 5*time.Millisecond, func(string) error {
		n++
		time.Sleep(20 * time.Millisecond)
		return nil
	})
	bas := time.Now()
	if err := telegramDinle(""); err != nil {
		t.Fatal(err)
	}
	if d := time.Since(bas); n < 8 || n > 17 || d > 450*time.Millisecond {
		t.Fatalf("bütçe: %d tur, %v (300 ms civarı, ~15 tur bekleniyordu)", n, d)
	}
}

func TestTelegramDinleKisaTur(t *testing.T) {
	n := 0
	surekliAyar(t, time.Hour, time.Minute, 50*time.Millisecond, func(string) error { n++; return nil })
	if err := telegramDinle(""); err != nil || n != 3 {
		t.Fatalf("art arda kısa turlar: %d tur, %v (3 turda çıkmalı)", n, err)
	}
	// kısa sayacı uzun turla sıfırlanır: kısa, kısa, uzun, kısa, kısa, kısa -> 6 tur
	n = 0
	surekliAyar(t, time.Hour, time.Minute, 30*time.Millisecond, func(string) error {
		n++
		if n == 3 {
			time.Sleep(40 * time.Millisecond)
		}
		return nil
	})
	if err := telegramDinle(""); err != nil || n != 6 {
		t.Fatalf("uzun tur sayacı sıfırlamalı: %d tur, %v", n, err)
	}
}

func TestTelegramDinleHata(t *testing.T) {
	n := 0
	hata := errors.New("ağ yok")
	surekliAyar(t, time.Hour, time.Minute, time.Millisecond, func(string) error {
		n++
		time.Sleep(5 * time.Millisecond)
		if n == 2 {
			return hata
		}
		return nil
	})
	if err := telegramDinle(""); !errors.Is(err, hata) || n != 2 {
		t.Fatalf("hata ilk görüldüğü turda dönmeli: %d tur, %v", n, err)
	}
}

// v25'in kazancı: iki tur (v24'te iki ayrı süreç = iki bağlantı + iki TLS el sıkışma) tek süreçte TEK bağlantı kullanır.
func TestTelegramIkiTurTekBaglanti(t *testing.T) {
	var mu sync.Mutex
	yeni, guncelleme := 0, 0
	srv := httptest.NewUnstartedServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		mu.Lock()
		guncelleme++
		mu.Unlock()
		_, _ = io.Copy(io.Discard, r.Body)
		_, _ = io.WriteString(w, `{"ok":true,"result":[]}`)
	}))
	srv.Config.ConnState = func(_ net.Conn, s http.ConnState) {
		if s == http.StateNew {
			mu.Lock()
			yeni++
			mu.Unlock()
		}
	}
	srv.Start()
	defer srv.Close()
	eskiTaban, eskiUzun := tgTaban, tgUzunBekleme
	tgTaban, tgUzunBekleme = srv.URL, 1
	defer func() { tgTaban, tgUzunBekleme = eskiTaban, eskiUzun }()
	for tur := 0; tur < 2; tur++ {
		ctx, iptal := context.WithTimeout(context.Background(), 40*time.Second)
		if _, err := tgDinleDongu(ctx, "J", "42", 0, "", func(int64) {}); err != nil {
			t.Fatal(err)
		}
		iptal()
	}
	mu.Lock()
	defer mu.Unlock()
	if guncelleme != 6 || yeni != 1 {
		t.Fatalf("2 tur: %d getUpdates, %d yeni bağlantı (6 ve 1 bekleniyordu)", guncelleme, yeni)
	}
}
