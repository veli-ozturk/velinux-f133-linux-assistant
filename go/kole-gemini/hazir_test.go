package main

import (
	"context"
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync/atomic"
	"testing"
	"time"
)

func TestFnv(t *testing.T) {
	if fnv1a32("") != 0x811c9dc5 || fnv1a32("a") != 0xe40c292c {
		t.Fatal("fnv1a32 vektörleri")
	}
	gor := map[uint32]bool{}
	for _, m := range hazirCumleler {
		if gor[fnv1a32(m)] {
			t.Fatal("tekrar: " + m)
		}
		gor[fnv1a32(m)] = true
	}
}

func TestHazirSesler(t *testing.T) {
	d := t.TempDir()
	hazirKok, hazirDizin, hazirKilit = d, filepath.Join(d, "sesler"), filepath.Join(d, "kilit")
	ttsGunlukYol, ttsKapaliYol = filepath.Join(d, "tts.log"), filepath.Join(d, "kapali")
	ttsSesYol, ttsStilYol, ttsModelYol = filepath.Join(d, "ses-ad"), filepath.Join(d, "stil"), filepath.Join(d, "model")
	anahtar := filepath.Join(d, "anahtar")
	_ = os.WriteFile(anahtar, []byte("GIZLI777\n"), 0600)
	_ = os.Remove(sogumaYol)
	var istek int32
	var hata atomic.Bool
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		atomic.AddInt32(&istek, 1)
		if hata.Load() {
			w.WriteHeader(429)
			_, _ = w.Write([]byte(`{"error":{"message":"kota"}}`))
			return
		}
		x := sinus(24000, 440, 2400, 3000) // 0.1 sn
		_, _ = w.Write([]byte(`{"steps":[{"content":[{"type":"audio","mime_type":"audio/l16;rate=24000","data":"` + pcmB64(x) + `"}]}]}`))
	}))
	defer srv.Close()
	ttsURL = srv.URL
	hazirAra = 0
	if n := hazirSeslerHazirla(anahtar); n != hazirAzami || hazirEksik() != len(hazirCumleler)-hazirAzami {
		t.Fatalf("sinirli ilk calistirma %d", n)
	}
	hazirAzami = 100
	if n := hazirSeslerHazirla(anahtar); n != len(hazirCumleler)-8 || hazirEksik() != 0 {
		t.Fatalf("kalanlar %d", n)
	}
	defer func() { hazirAzami = 8 }()
	st, err := os.Stat(hazirYol("Bir saniye efendim, düşünüyorum..."))
	if err != nil || st.Size() != int64(2*(16000+1600+3200)) {
		t.Fatalf("pcm %v %v", err, st)
	}
	if !strings.Contains(oku(filepath.Join(hazirDizin, "liste.txt"), ""), "\tBir saniye efendim, düşünüyorum...") {
		t.Fatal("liste.txt")
	}
	once := atomic.LoadInt32(&istek)
	if n := hazirSeslerHazirla(anahtar); n != 0 || atomic.LoadInt32(&istek) != once {
		t.Fatal("hepsi varken ağ kullanmamalı")
	}
	_ = os.WriteFile(ttsSesYol, []byte("Leda\n"), 0644) // ses değişti
	hata.Store(true)
	if n := hazirSeslerHazirla(anahtar); n != 0 {
		t.Fatal("hata varken üretmemeli")
	}
	if hazirEksik() != len(hazirCumleler) {
		t.Fatal("eski sesle üretilenler silinmeli")
	}
	hata.Store(false)
	if n := hazirSeslerHazirla(anahtar); n != len(hazirCumleler) || oku(filepath.Join(hazirDizin, "ses"), "") != hazirDamga() {
		t.Fatal("yeni sesle hepsi + damga")
	}
	hazirSil()
	_ = os.WriteFile(hazirKilit, []byte("1\n"), 0644)
	if n := hazirSeslerHazirla(anahtar); n != 0 {
		t.Fatal("kilit varken üretmemeli")
	}
	_ = os.Remove(hazirKilit)
	hazirKok = filepath.Join(d, "yok")
	if hazirSeslerHazirla(anahtar) != 0 {
		t.Fatal("SD yokken üretmemeli")
	}
	b, _ := os.ReadFile(ttsGunlukYol)
	if strings.Contains(string(b), "GIZLI777") {
		t.Fatal("anahtar günlükte")
	}
}

func TestKilitAlBekle(t *testing.T) {
	d := t.TempDir()
	ttsKilitYol = filepath.Join(d, "kilit")
	_ = os.WriteFile(ttsKilitYol, []byte("asistan 1\n"), 0644)
	go func() {
		time.Sleep(300 * time.Millisecond)
		_ = os.Remove(ttsKilitYol)
	}()
	t0 := time.Now()
	if !kilitAlBekle(2*time.Second) || time.Since(t0) < 250*time.Millisecond {
		t.Fatal("kilit beklenmeli")
	}
	if kilitAlBekle(200 * time.Millisecond) {
		t.Fatal("dolu kilit alınmamalı")
	}
}

func TestTtsYedek429(t *testing.T) {
	d := t.TempDir()
	ttsSesYol, ttsStilYol, ttsModelYol = filepath.Join(d, "ses-ad"), filepath.Join(d, "stil"), filepath.Join(d, "model")
	_ = os.Remove(sogumaYol)
	var modeller []string
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		var g struct {
			Model string `json:"model"`
		}
		_ = json.NewDecoder(r.Body).Decode(&g)
		modeller = append(modeller, g.Model)
		if g.Model == ttsModelVarsayilan {
			w.WriteHeader(429)
			_, _ = w.Write([]byte(`{"error":{"message":"Rate limit exceeded"}}`))
			return
		}
		_, _ = w.Write([]byte(`{"steps":[{"content":[{"type":"audio","mime_type":"audio/l16;rate=24000","data":"` + pcmB64(sinus(24000, 440, 2400, 3000)) + `"}]}]}`))
	}))
	defer srv.Close()
	ttsURL = srv.URL
	x, _, err := ttsUret(context.Background(), "A", "Merhaba efendim.")
	if err != nil || len(x) != 1600 || len(modeller) != 2 || modeller[1] != ttsModelYedek {
		t.Fatalf("429 -> yedek model: %v %d %v", err, len(x), modeller)
	}
}
