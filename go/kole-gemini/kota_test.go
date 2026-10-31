package main

import (
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync/atomic"
	"testing"
)

// tüm testlerde kota/günlük dosyaları geçici dizinde (host /tmp'ye yazılmasın)
func TestMain(m *testing.M) {
	d, _ := os.MkdirTemp("", "kole-gemini-test")
	ttsKotaYol, ttsGunlukYol = filepath.Join(d, "tts-kota"), filepath.Join(d, "tts.log")
	sogumaYol = filepath.Join(d, "soguma.json") // v17
	gcloudURL = "http://127.0.0.1:1/yok"        // v17: testte dis istek yok
	gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol = filepath.Join(d, "yok-g"), filepath.Join(d, "yok-a"), filepath.Join(d, "yok-e")
	ttsModellerYol = filepath.Join(d, "yok-m")
	ttsGeminiModelleri = []string{ttsModelVarsayilan, ttsModelYedek} // v16 testleri iki modelle
	k := m.Run()
	_ = os.RemoveAll(d)
	os.Exit(k)
}

func TestKotaCoz(t *testing.T) {
	g := `{"error":{"code":429,"message":"Rate limit exceeded for model x","status":"RESOURCE_EXHAUSTED","details":[` +
		`{"@type":"type.googleapis.com/google.rpc.QuotaFailure","violations":[{"quotaMetric":"generativelanguage.googleapis.com/generate_requests",` +
		`"quotaId":"GenerateRequestsPerMinutePerProjectPerModel-FreeTier","quotaValue":"5"}]},` +
		`{"@type":"type.googleapis.com/google.rpc.RetryInfo","retryDelay":"36.5s"}]}}`
	kota, deger, bekle, mesaj := kotaCoz([]byte(g), "")
	if kota != "GenerateRequestsPerMinutePerProjectPerModel-FreeTier" || deger != "5" || bekle != 37 || !strings.HasPrefix(mesaj, "Rate limit") {
		t.Fatalf("%q %q %d %q", kota, deger, bekle, mesaj)
	}
	if _, _, b, _ := kotaCoz([]byte(`{"error":{"message":"x"}}`), " 12 "); b != 12 {
		t.Fatal("Retry-After")
	}
	if k, dg, _, _ := kotaCoz([]byte(`{"error":{"details":[{"violations":[{"quotaMetric":"m/x","quotaValue":7}]}]}}`), ""); k != "m/x" || dg != "7" {
		t.Fatalf("metrik/sayi %q %q", k, dg)
	}
	if k, _, b, m := kotaCoz([]byte(`<html>`), "x"); k != "" || b != 0 || m != "" {
		t.Fatal("JSON olmayan govde")
	}
}

func TestKotaBeklemeVeDurum(t *testing.T) {
	d := t.TempDir()
	eskiKota, eskiGunluk := ttsKotaYol, ttsGunlukYol
	defer func() { ttsKotaYol, ttsGunlukYol = eskiKota, eskiGunluk }()
	ttsKotaYol, ttsGunlukYol = filepath.Join(d, "kota"), filepath.Join(d, "tts.log")
	_ = os.Remove(sogumaYol)
	hazirKok, hazirDizin, hazirKilit = d, filepath.Join(d, "sesler"), filepath.Join(d, "kilit")
	ttsKapaliYol, ttsSesYol = filepath.Join(d, "kapali"), filepath.Join(d, "ses-ad")
	ttsStilYol, ttsModelYol = filepath.Join(d, "stil"), filepath.Join(d, "model")
	anahtar := filepath.Join(d, "anahtar")
	_ = os.WriteFile(anahtar, []byte("GIZLI555\n"), 0600)
	var istek int32
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		atomic.AddInt32(&istek, 1)
		w.WriteHeader(429)
		_, _ = w.Write([]byte(`{"error":{"message":"anahtar ` + r.Header.Get("x-goog-api-key") + ` sinirda","details":[` +
			`{"violations":[{"quotaId":"Q1","quotaValue":"5"}]},{"retryDelay":"120s"}]}}`))
	}))
	defer srv.Close()
	ttsURL = srv.URL
	if n := hazirSeslerHazirla(anahtar); n != 0 {
		t.Fatal("429'da uretmemeli")
	}
	k, ok := kotaOku()
	if !ok || k.Bekle != 120 || k.Kota != "Q1" || k.Deger != "5" || k.Model != ttsModelYedek || strings.Contains(k.Mesaj, "GIZLI555") {
		t.Fatalf("kota kaydi %+v", k)
	}
	once := atomic.LoadInt32(&istek)
	if once != 2 {
		t.Fatalf("ana + yedek model: %d istek", once)
	}
	if n := hazirSeslerHazirla(anahtar); n != 0 || atomic.LoadInt32(&istek) != once {
		t.Fatal("kota beklerken istek atmamali")
	}
	s := sesDurumu()
	for _, p := range []string{"Ses: Puck", "Hazır cümleler: 0/19", "kota Q1 = 5", "bekleme 120 sn", ttsModelYedek} {
		if !strings.Contains(s, p) {
			t.Fatalf("sesdurum'da yok %q:\n%s", p, s)
		}
	}
	b, _ := os.ReadFile(ttsGunlukYol)
	if strings.Contains(string(b)+string(mustOku(ttsKotaYol)), "GIZLI555") || !strings.Contains(string(b), "kota beklemesi") {
		t.Fatalf("gunluk:\n%s", b)
	}
	_ = os.Remove(ttsKotaYol)
	if !strings.Contains(sesDurumu(), "Kota hatası yok") {
		t.Fatal("kota yokken")
	}
}

func mustOku(y string) []byte {
	b, _ := os.ReadFile(y)
	return b
}
