package main

import (
	"context"
	"encoding/base64"
	"encoding/json"
	"io"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"sync"
	"testing"
	"time"
)

func TestPasifikGeceYarisi(t *testing.T) {
	for _, c := range []struct{ t, beklenen time.Time }{
		{time.Date(2026, 10, 20, 6, 10, 0, 0, time.UTC), time.Date(2026, 10, 20, 7, 0, 0, 0, time.UTC)}, // TR 09:10 -> 10:00
		{time.Date(2026, 11, 5, 12, 0, 0, 0, time.UTC), time.Date(2026, 11, 6, 8, 0, 0, 0, time.UTC)},   // kış: TR 11:00
		{time.Date(2026, 1, 15, 7, 59, 0, 0, time.UTC), time.Date(2026, 1, 15, 8, 0, 0, 0, time.UTC)},
	} {
		if g := pasifikGeceYarisi(c.t); !g.Equal(c.beklenen) {
			t.Fatalf("%v -> %v, beklenen %v", c.t, g.UTC(), c.beklenen)
		}
	}
}

func TestSogumaSirala(t *testing.T) {
	sogumaYol = filepath.Join(t.TempDir(), "s.json")
	sogumaKoy("x:a", time.Hour, "429")
	sogumaKoy("x:b", time.Hour, "403")
	sogumaKoy("x:c", -time.Second, "429") // süresi geçmiş
	if g := strings.Join(sogumaSirala([]string{"a", "b", "c", "d"}, "x:"), ","); g != "c,d,a" {
		t.Fatalf("sira %s", g)
	}
	if g := strings.Join(sogumaSirala([]string{"b"}, "x:"), ","); g != "b" {
		t.Fatalf("tek kapali aday yine denenmeli: %s", g)
	}
	sogumaKaldir("x:a")
	if _, ok := sogumaDurum("x:a"); ok {
		t.Fatal("kaldirilmadi")
	}
	if d := sogumaKoyKota("x:e", []byte(`{"error":{"details":[{"violations":[{"quotaId":"GenerateRequestsPerDayPerProjectPerModel-FreeTier"}]}]}}`), ""); d < time.Minute || d > 25*time.Hour {
		t.Fatalf("gunluk kota suresi %v", d)
	}
	if d := sogumaKoyKota("x:f", []byte(`{}`), "90"); d != 90*time.Second {
		t.Fatalf("Retry-After %v", d)
	}
	if d := sogumaKoyKota("x:g", []byte(`{}`), ""); d != 30*time.Minute {
		t.Fatalf("varsayilan %v", d)
	}
}

type sahteSaglayici struct {
	mu       sync.Mutex
	istekler []string
	m3Kota   bool
}

func (s *sahteSaglayici) kaydet(x string) {
	s.mu.Lock()
	s.istekler = append(s.istekler, x)
	s.mu.Unlock()
}

func (s *sahteSaglayici) al() []string {
	s.mu.Lock()
	defer s.mu.Unlock()
	return append([]string(nil), s.istekler...)
}

func TestSaglayiciRotasyonu(t *testing.T) {
	d := t.TempDir()
	sogumaYol, ttsKotaYol, ttsGunlukYol = filepath.Join(d, "s.json"), filepath.Join(d, "kota"), filepath.Join(d, "tts.log")
	ttsSesYol, ttsStilYol, ttsModellerYol = filepath.Join(d, "ses"), filepath.Join(d, "stil"), filepath.Join(d, "yok")
	ttsModelYol = filepath.Join(d, "model")
	_ = os.WriteFile(ttsModelYol, []byte("m1-tts\n"), 0644)
	eskiListe := ttsGeminiModelleri
	defer func() { ttsGeminiModelleri = eskiListe }()
	ttsGeminiModelleri = []string{"m2-tts", "m3-tts"}
	gcloudAnahtarYol, azureAnahtarYol, elevenAnahtarYol = filepath.Join(d, "g"), filepath.Join(d, "a"), filepath.Join(d, "e")
	azureSesYol, gcloudSesYol = filepath.Join(d, "as"), filepath.Join(d, "gs")
	ss := &sahteSaglayici{}
	pcm := func(w http.ResponseWriter) {
		_, _ = w.Write(make([]byte, 3200))
	}
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, _ := io.ReadAll(r.Body)
		switch {
		case r.URL.Path == "/gemini":
			var g struct {
				Model string `json:"model"`
			}
			_ = json.Unmarshal(b, &g)
			stilli := strings.Contains(string(b), "speech_metadata")
			ss.kaydet(g.Model + map[bool]string{true: "+stil", false: ""}[stilli])
			switch {
			case g.Model == "m1-tts":
				w.WriteHeader(429)
				_, _ = w.Write([]byte(`{"error":{"message":"x","details":[{"retryDelay":"120s"}]}}`))
			case g.Model == "m2-tts" || ss.m3Kota:
				w.WriteHeader(429)
				_, _ = w.Write([]byte(`{"error":{"message":"Rate limit exceeded"}}`))
			case stilli:
				w.WriteHeader(400)
				_, _ = w.Write([]byte(`{"error":{"message":"speech_metadata annotations are not supported"}}`))
			default:
				_, _ = w.Write([]byte(`{"steps":[{"content":[{"type":"audio","mime_type":"audio/l16;rate=24000","data":"` +
					pcmB64(sinus(24000, 440, 2400, 3000)) + `"}]}]}`))
			}
		case r.URL.Path == "/gcloud":
			ss.kaydet("gcloud:" + r.Header.Get("x-goog-api-key"))
			w.WriteHeader(403)
			_, _ = w.Write([]byte(`{"error":{"message":"Cloud TTS API has not been used; key ` + r.Header.Get("x-goog-api-key") + `"}}`))
		case strings.HasPrefix(r.URL.Path, "/azure/"):
			ss.kaydet("azure:" + strings.TrimPrefix(r.URL.Path, "/azure/") + ":" + r.Header.Get("X-Microsoft-OutputFormat"))
			if r.Header.Get("Ocp-Apim-Subscription-Key") != "AZUREKEY1234567890abcd" || !strings.Contains(string(b), "tr-TR-AhmetNeural") ||
				!strings.Contains(string(b), "Tom &amp; Jerry") {
				w.WriteHeader(401)
				return
			}
			pcm(w)
		case strings.HasPrefix(r.URL.Path, "/eleven/"):
			ss.kaydet("eleven:" + strings.TrimPrefix(r.URL.Path, "/eleven/") + ":" + r.URL.Query().Get("output_format"))
			if r.Header.Get("xi-api-key") != "ELEVENKEY1234567890xyz" {
				w.WriteHeader(401)
				return
			}
			pcm(w)
		}
	}))
	defer srv.Close()
	ttsURL, gcloudURL = srv.URL+"/gemini", srv.URL+"/gcloud"
	azureURLKalip, elevenURLKalip = srv.URL+"/azure/%s", srv.URL+"/eleven/%s?output_format=pcm_16000"
	ctx := context.Background()
	// 1) m1 429 (120 sn), m2 429 (30 dk), m3 stil 400 -> stilsiz başarı
	x, et, err := ttsUretSec(ctx, "GEMKEY1234567890", "Merhaba efendim.", false)
	if err != nil || len(x) != 1600 || !strings.Contains(et, "m3-tts") {
		t.Fatalf("1: %v %d %q %v", err, len(x), et, ss.al())
	}
	if g := strings.Join(ss.al(), ","); g != "m1-tts+stil,m2-tts+stil,m3-tts+stil,m3-tts" {
		t.Fatalf("1 istekler %s", g)
	}
	if v, ok := sogumaDurum("tts:gemini:m1-tts"); !ok || v.Kadar-time.Now().Unix() > 121 || v.Kadar-time.Now().Unix() < 100 {
		t.Fatalf("m1 sogumasi %+v", v)
	}
	// 2) m3 hazır ve stilsiz hatırlanıyor: tek istek
	ss.istekler = nil
	if _, _, err := ttsUretSec(ctx, "GEMKEY1234567890", "İkinci.", false); err != nil || strings.Join(ss.al(), ",") != "m3-tts" {
		t.Fatalf("2: %v %v", err, ss.al())
	}
	// 3) m3 de 429 -> gcloud (Gemini anahtarıyla) 403 -> azure başarılı
	ss.istekler, ss.m3Kota = nil, true
	_ = os.WriteFile(azureAnahtarYol, []byte("westeurope AZUREKEY1234567890abcd\n"), 0600)
	x, et, err = ttsUretSec(ctx, "GEMKEY1234567890", "Tom & Jerry", false)
	if err != nil || len(x) != 1600 || et != "Azure tr-TR-AhmetNeural" {
		t.Fatalf("3: %v %q %v", err, et, ss.al())
	}
	if g := strings.Join(ss.al(), ","); g != "m3-tts,gcloud:GEMKEY1234567890,azure:westeurope:raw-16khz-16bit-mono-pcm" {
		t.Fatalf("3 istekler %s", g)
	}
	if _, ok := sogumaDurum("tts:gcloud-gemini-anahtari"); !ok {
		t.Fatal("gcloud 403 -> 24 sa")
	}
	// 4) yalnız Gemini (hazır ses): hepsi 429 -> hata, azure çağrılmaz
	ss.istekler = nil
	if _, _, err := ttsUretGemini(ctx, "GEMKEY1234567890", "Hazır."); err == nil || strings.Contains(strings.Join(ss.al(), ","), "azure") {
		t.Fatalf("4: %v %v", err, ss.al())
	}
	// 5) ElevenLabs: azure silinince sıraya o girer
	_ = os.Remove(azureAnahtarYol)
	_ = os.WriteFile(elevenAnahtarYol, []byte("ELEVENKEY1234567890xyz SesID123456\n"), 0600)
	ss.istekler = nil
	if _, et, err := ttsUretSec(ctx, "GEMKEY1234567890", "Son.", false); err != nil || et != "ElevenLabs" ||
		!strings.Contains(strings.Join(ss.al(), ","), "eleven:SesID123456:pcm_16000") {
		t.Fatalf("5: %v %q %v", err, et, ss.al())
	}
	g, _ := os.ReadFile(ttsGunlukYol)
	k, _ := os.ReadFile(ttsKotaYol)
	if strings.Contains(string(g)+string(k), "GEMKEY") || strings.Contains(string(g), "AZUREKEY") {
		t.Fatalf("anahtar gunlukte:\n%s\n%s", g, k)
	}
	durum := sesDurumu()
	for _, p := range []string{"Ses sağlayıcıları", "gemini:m1-tts", "○ gemini:m2-tts — 429", "● eleven — hazır", "/anahtar azure"} {
		if !strings.Contains(durum, p) {
			t.Fatalf("sesdurum'da yok %q:\n%s", p, durum)
		}
	}
}

func TestGcloudBasari(t *testing.T) {
	d := t.TempDir()
	sogumaYol, ttsGunlukYol, ttsKotaYol = filepath.Join(d, "s.json"), filepath.Join(d, "l"), filepath.Join(d, "k")
	ttsModelYol, ttsModellerYol, ttsSesYol = filepath.Join(d, "m"), filepath.Join(d, "y"), filepath.Join(d, "ses")
	_ = os.WriteFile(ttsModelYol, []byte("m1-tts\n"), 0644)
	eskiListe := ttsGeminiModelleri
	defer func() { ttsGeminiModelleri = eskiListe }()
	ttsGeminiModelleri = nil
	gcloudAnahtarYol, gcloudSesYol = filepath.Join(d, "g"), filepath.Join(d, "gs")
	azureAnahtarYol, elevenAnahtarYol = filepath.Join(d, "a"), filepath.Join(d, "e")
	_ = os.WriteFile(gcloudAnahtarYol, []byte("GCLOUDKEY1234567890ab\n"), 0600)
	var ses string
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, _ := io.ReadAll(r.Body)
		if r.URL.Path == "/gemini" {
			w.WriteHeader(429)
			return
		}
		var g struct {
			Voice struct {
				Name string `json:"name"`
			} `json:"voice"`
		}
		_ = json.Unmarshal(b, &g)
		ses = g.Voice.Name
		if r.Header.Get("x-goog-api-key") != "GCLOUDKEY1234567890ab" {
			w.WriteHeader(401)
			return
		}
		wav := make([]byte, 44+3200)
		copy(wav, "RIFF")
		copy(wav[8:], "WAVEfmt ")
		wav[16], wav[20], wav[22] = 16, 1, 1
		wav[24], wav[25] = 0x80, 0x3e // 16000
		wav[34] = 16
		copy(wav[36:], "data")
		wav[40], wav[41] = 0x80, 0x0c // 3200
		_, _ = w.Write([]byte(`{"audioContent":"` + base64.StdEncoding.EncodeToString(wav) + `"}`))
	}))
	defer srv.Close()
	ttsURL, gcloudURL = srv.URL+"/gemini", srv.URL+"/gcloud"
	x, et, err := ttsUretSec(context.Background(), "GEMKEY1234567890", "Merhaba.", false)
	if err != nil || len(x) != 1600 || et != "GCloud tr-TR-Chirp3-HD-Puck" || ses != "tr-TR-Chirp3-HD-Puck" {
		t.Fatalf("%v %d %q %q", err, len(x), et, ses)
	}
}

func TestAnahtarKomutu(t *testing.T) {
	d := t.TempDir()
	sogumaYol = filepath.Join(d, "s.json")
	azureAnahtarYol, gcloudAnahtarYol, elevenAnahtarYol = filepath.Join(d, "a"), filepath.Join(d, "g"), filepath.Join(d, "e")
	c := anahtarKomutu([]string{"azure", "WestEurope", "AbCdEf1234567890QwErTy"})
	b, err := os.ReadFile(azureAnahtarYol)
	st, _ := os.Stat(azureAnahtarYol)
	if err != nil || string(b) != "westeurope AbCdEf1234567890QwErTy\n" || st.Mode().Perm() != 0600 ||
		strings.Contains(c, "AbCdEf1234567890") || !strings.Contains(c, "…ErTy") {
		t.Fatalf("azure: %q %q %v", c, b, st.Mode())
	}
	if c := anahtarKomutu([]string{"gcloud", "kisa"}); !strings.Contains(c, "Biçim uymadı") || dosyaVar(gcloudAnahtarYol) {
		t.Fatalf("gecersiz anahtar kaydedildi: %q", c)
	}
	if c := anahtarKomutu([]string{"eleven", "ELEVENKEY1234567890xyz", "Ses<>bad"}); !strings.Contains(c, "Biçim uymadı") {
		t.Fatalf("gecersiz ses id: %q", c)
	}
	if c := anahtarKomutu(nil); !strings.Contains(c, "Kayıtlı: azure.") {
		t.Fatalf("liste: %q", c)
	}
	if c := anahtarKomutu([]string{"sil", "azure"}); !strings.Contains(c, "silindi") || dosyaVar(azureAnahtarYol) {
		t.Fatalf("sil: %q", c)
	}
}
