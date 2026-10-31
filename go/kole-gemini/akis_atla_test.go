// akis_atla_test.go (23.10.2026, Claude; v39): Gemini TTS hepsi soğumadayken ve yedek varken akış atlanır.
package main

import (
	"context"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync/atomic"
	"testing"
	"time"
)

func TestAkisAtla(t *testing.T) {
	d := t.TempDir()
	eski := []string{sogumaYol, ttsGunlukYol, ttsModelYol, ttsModellerYol, gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol, ttsURL, ttsKotaYol}
	eskiListe := ttsGeminiModelleri
	defer func() {
		sogumaYol, ttsGunlukYol, ttsModelYol, ttsModellerYol = eski[0], eski[1], eski[2], eski[3]
		gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol, ttsURL, ttsKotaYol = eski[4], eski[5], eski[6], eski[7], eski[8]
		ttsGeminiModelleri = eskiListe
	}()
	sogumaYol, ttsGunlukYol, ttsKotaYol = filepath.Join(d, "s.json"), filepath.Join(d, "tts.log"), filepath.Join(d, "kota")
	ttsModelYol, ttsModellerYol = filepath.Join(d, "model"), filepath.Join(d, "yok")
	_ = os.WriteFile(ttsModelYol, []byte("m1-tts\n"), 0644)
	ttsGeminiModelleri = []string{"m2-tts"}
	gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol = filepath.Join(d, "g"), filepath.Join(d, "a"), filepath.Join(d, "e")
	var istek atomic.Int32
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		istek.Add(1)
		w.WriteHeader(429)
		_, _ = w.Write([]byte(`{"error":{"message":"Rate limit exceeded"}}`))
	}))
	defer srv.Close()
	ttsURL = srv.URL
	sogumaKoy("tts:gemini:m1-tts", time.Hour, "429")
	sogumaKoy("tts:gemini:m2-tts", time.Hour, "429")
	sogumaKoy("tts:gcloud-gemini-anahtari", time.Hour, "403")
	ctx := context.Background()
	// 1) hepsi soğumada + ElevenLabs: akış atlanır, Gemini'ye istek yok, anında döner
	_ = os.WriteFile(elevenAnahtarYol, []byte("ELEVENKEY1234567890xyz\n"), 0600)
	if y := akisAtlaNedeni("GEMKEY1234567890"); y != "eleven" {
		t.Fatalf("atlama nedeni %q", y)
	}
	t0 := time.Now()
	r, err := akisCal(ctx, "GEMKEY1234567890", "Merhaba.", t0)
	if r.basladi || err == nil || !strings.Contains(err.Error(), "yedek eleven") || istek.Load() != 0 || time.Since(t0) > time.Second {
		t.Fatalf("atlanmadı: %+v %v istek %d %v", r, err, istek.Load(), time.Since(t0))
	}
	g, _ := os.ReadFile(ttsGunlukYol)
	if !strings.Contains(string(g), "akış atlandı (yedek: eleven)") {
		t.Fatalf("günlük: %s", g)
	}
	// 2) ElevenLabs de soğumada (402): yedek yok -> eski davranış, Gemini denenir
	sogumaKoy("tts:eleven", time.Hour, "402")
	if y := akisAtlaNedeni("GEMKEY1234567890"); y != "" {
		t.Fatalf("yedek soğumadayken atlanmamalı: %q", y)
	}
	_, _ = akisCal(ctx, "GEMKEY1234567890", "Merhaba.", time.Now())
	if istek.Load() == 0 {
		t.Fatal("yedek yokken Gemini denenmeli")
	}
	// 3) bir Gemini modeli hazırsa atlanmaz
	sogumaKaldir("tts:eleven")
	sogumaKaldir("tts:gemini:m2-tts")
	if y := akisAtlaNedeni("GEMKEY1234567890"); y != "" {
		t.Fatalf("Gemini hazırken atlanmamalı: %q", y)
	}
}
