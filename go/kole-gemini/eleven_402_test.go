// eleven_402_test.go (23.10.2026, Claude; v38): ElevenLabs kütüphane sesi ücretsizde 402 -> soğuma + günlük;
// ses kimliği yazılmamış anahtar varsayılan premade sesle 200.
package main

import (
	"context"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestEleven402(t *testing.T) {
	d := t.TempDir()
	eski := []string{sogumaYol, ttsKotaYol, ttsGunlukYol, ttsSesYol, ttsStilYol, ttsModellerYol, ttsModelYol, gcloudAnahtarYol,
		azureAnahtarYol, elevenAnahtarYol, ttsURL, gcloudURL, elevenURLKalip}
	eskiListe := ttsGeminiModelleri
	defer func() {
		sogumaYol, ttsKotaYol, ttsGunlukYol, ttsSesYol, ttsStilYol, ttsModellerYol, ttsModelYol = eski[0], eski[1], eski[2], eski[3], eski[4], eski[5], eski[6]
		gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol, ttsURL, gcloudURL, elevenURLKalip = eski[7], eski[8], eski[9], eski[10], eski[11], eski[12]
		ttsGeminiModelleri = eskiListe
	}()
	sogumaYol, ttsKotaYol, ttsGunlukYol = filepath.Join(d, "s.json"), filepath.Join(d, "kota"), filepath.Join(d, "tts.log")
	ttsSesYol, ttsStilYol, ttsModellerYol, ttsModelYol = filepath.Join(d, "ses"), filepath.Join(d, "stil"), filepath.Join(d, "yok"), filepath.Join(d, "model")
	_ = os.WriteFile(ttsModelYol, []byte("m1-tts\n"), 0644)
	ttsGeminiModelleri = nil
	gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol = filepath.Join(d, "g"), filepath.Join(d, "a"), filepath.Join(d, "e")
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		switch {
		case r.URL.Path == "/gemini":
			w.WriteHeader(429)
			_, _ = w.Write([]byte(`{"error":{"message":"Rate limit exceeded (limit: 0 requests per day)"}}`))
		case r.URL.Path == "/gcloud":
			w.WriteHeader(403)
		case r.URL.Path == "/eleven/"+elevenSesVarsayilan:
			_, _ = w.Write(make([]byte, 3200))
		default:
			w.WriteHeader(402)
			_, _ = w.Write([]byte(`{"detail":{"type":"payment_required","code":"paid_plan_required","message":"Free users cannot use library voices via the API."}}`))
		}
	}))
	defer srv.Close()
	ttsURL, gcloudURL, elevenURLKalip = srv.URL+"/gemini", srv.URL+"/gcloud", srv.URL+"/eleven/%s?output_format=pcm_16000"
	ctx := context.Background()
	// 1) eski varsayılan (kütüphane) sesi: 402 -> hata, 24 sa soğuma, günlükte görünür
	_ = os.WriteFile(elevenAnahtarYol, []byte("ELEVENKEY1234567890xyz TxGEqnHWrfWFTfGW9XjX\n"), 0600)
	if _, _, err := ttsUretSec(ctx, "GEMKEY1234567890", "Merhaba.", false); err == nil {
		t.Fatal("402'de ses olmamalı")
	}
	if v, ok := sogumaDurum("tts:eleven"); !ok || v.Neden != "402" {
		t.Fatalf("eleven soğuması %+v %v", v, ok)
	}
	g, _ := os.ReadFile(ttsGunlukYol)
	if !strings.Contains(string(g), "eleven HTTP 402 (ücretli plan gerekiyor)") || strings.Contains(string(g), "ELEVENKEY") {
		t.Fatalf("günlük:\n%s", g)
	}
	// 2) ses kimliği yazılmamış anahtar -> varsayılan premade ses -> 200
	sogumaKaldir("tts:eleven")
	_ = os.WriteFile(elevenAnahtarYol, []byte("ELEVENKEY1234567890xyz\n"), 0600)
	if x, et, err := ttsUretSec(ctx, "GEMKEY1234567890", "Merhaba.", false); err != nil || et != "ElevenLabs" || len(x) == 0 {
		t.Fatalf("varsayılan ses: %v %q %d", err, et, len(x))
	}
	if elevenSesVarsayilan != "TX3LPaxmHKxFdv7VOQHJ" {
		t.Fatal("varsayılan premade Liam olmalı")
	}
}
