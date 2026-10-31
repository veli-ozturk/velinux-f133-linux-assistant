// assembly_test.go (21.10.2026, Claude; kole-gemini v23): sahte AssemblyAI + sahte Gemini ile konuşma -> metin yedeği.
package main

import (
	"context"
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

const asmAnahtar = "0123456789abcdef0123456789abcdef"

type sahteAsm struct {
	mu       sync.Mutex
	yuklenen []byte
	dokum    map[string]any
	sorgu    int
	silinen  string
	durum    string // "completed" | "error"
	metin    string
	yetkisiz bool
}

func yeniSahteAsm(t *testing.T) *sahteAsm {
	s := &sahteAsm{durum: "completed", metin: "Saat kaç?"}
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		s.mu.Lock()
		defer s.mu.Unlock()
		if s.yetkisiz || r.Header.Get("authorization") != asmAnahtar {
			w.WriteHeader(401)
			_, _ = io.WriteString(w, `{"error":"Invalid API key `+r.Header.Get("authorization")+`"}`)
			return
		}
		switch {
		case r.Method == "POST" && r.URL.Path == "/v2/upload":
			s.yuklenen, _ = io.ReadAll(r.Body)
			_, _ = io.WriteString(w, `{"upload_url":"https://cdn.example/ses-1"}`)
		case r.Method == "POST" && r.URL.Path == "/v2/transcript":
			_ = json.NewDecoder(r.Body).Decode(&s.dokum)
			_, _ = io.WriteString(w, `{"id":"d1","status":"queued"}`)
		case r.Method == "GET" && r.URL.Path == "/v2/transcript/d1":
			s.sorgu++
			if s.sorgu < 2 {
				_, _ = io.WriteString(w, `{"id":"d1","status":"processing"}`)
				return
			}
			b, _ := json.Marshal(map[string]string{"id": "d1", "status": s.durum, "text": s.metin, "error": "ses bozuk"})
			_, _ = w.Write(b)
		case r.Method == "DELETE" && r.URL.Path == "/v2/transcript/d1":
			s.silinen = "d1"
			_, _ = io.WriteString(w, `{"id":"d1","status":"completed"}`)
		default:
			w.WriteHeader(404)
		}
	}))
	eski, eskiB := assemblyTaban, assemblyBekleme
	assemblyTaban, assemblyBekleme = srv.URL, 10*time.Millisecond
	t.Cleanup(func() { srv.Close(); assemblyTaban, assemblyBekleme = eski, eskiB })
	return s
}

func TestAssemblyYaziya(t *testing.T) {
	s := yeniSahteAsm(t)
	wav := []byte(strings.Repeat("RIFF", 500))
	m, err := assemblyYaziya(context.Background(), asmAnahtar, wav)
	if err != nil || m != "Saat kaç?" {
		t.Fatalf("%q %v", m, err)
	}
	if string(s.yuklenen) != string(wav) || s.dokum["language_code"] != "tr" || s.dokum["audio_url"] != "https://cdn.example/ses-1" {
		t.Fatalf("istek: %d bayt %v", len(s.yuklenen), s.dokum)
	}
	if s.sorgu != 2 || s.silinen != "d1" {
		t.Fatalf("sorgu %d, silinen %q (gizlilik: döküm silinmeli)", s.sorgu, s.silinen)
	}
}

func TestAssemblyHatalar(t *testing.T) {
	s := yeniSahteAsm(t)
	s.durum = "error"
	if _, err := assemblyYaziya(context.Background(), asmAnahtar, make([]byte, 2000)); err == nil || !strings.Contains(err.Error(), "ses bozuk") {
		t.Fatalf("döküm hatası: %v", err)
	}
	if s.silinen != "d1" {
		t.Fatal("hatada da silinmeli")
	}
	s.yetkisiz = true
	_, err := assemblyYaziya(context.Background(), "yanlis-anahtar-1234567890", make([]byte, 2000))
	if err == nil || !strings.Contains(err.Error(), "401") || strings.Contains(err.Error(), "yanlis-anahtar") {
		t.Fatalf("401 ve anahtar gizli olmalı: %v", err)
	}
}

func TestAssemblyYedek(t *testing.T) {
	yeniSahteAsm(t)
	var gemini []string
	var gmu sync.Mutex
	gsrv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		b, _ := io.ReadAll(r.Body)
		gmu.Lock()
		gemini = append(gemini, string(b))
		gmu.Unlock()
		_, _ = io.WriteString(w, `{"candidates":[{"content":{"parts":[{"text":"{\"soru\":\"saat kaç\",\"cevap\":\"Saat on efendim.\"}"}]}}]}`)
	}))
	eskiL := llmURLTaban
	llmURLTaban = gsrv.URL + "/"
	d := t.TempDir()
	eskiA, eskiS := assemblyAnahtarYol, sogumaYol
	assemblyAnahtarYol, sogumaYol = filepath.Join(d, "assembly"), filepath.Join(d, "s.json")
	eskiBas, eskiSinir := surecBas, cikisSiniri
	surecBas, cikisSiniri = time.Now(), sesliCikisSiniri
	t.Cleanup(func() {
		gsrv.Close()
		llmURLTaban, assemblyAnahtarYol, sogumaYol, surecBas, cikisSiniri = eskiL, eskiA, eskiS, eskiBas, eskiSinir
	})
	ist := istek{Contents: []icerik{{Role: "user", Parts: []parca{{Text: "eski soru"}}}, {Role: "model", Parts: []parca{{Text: "eski cevap"}}},
		{Role: "user", Parts: []parca{{InlineData: &inlineData{MimeType: "audio/wav", Data: "AAAA"}}, {Text: "sesli talimat"}}}}}
	wav := make([]byte, 4000)
	if _, _, _, ok := assemblyYedek(context.Background(), wav, ist, []string{"m1"}, "G"); ok {
		t.Fatal("anahtar yokken yedek çalışmamalı")
	}
	if err := os.WriteFile(assemblyAnahtarYol, []byte(asmAnahtar+"\n"), 0600); err != nil {
		t.Fatal(err)
	}
	y, model, metin, ok := assemblyYedek(context.Background(), wav, ist, []string{"m1"}, "G")
	if !ok || metin != "Saat kaç?" || model != "m1 (assembly)" || len(y.Candidates) != 1 {
		t.Fatalf("yedek: %v %q %q", ok, model, metin)
	}
	if len(gemini) != 1 || strings.Contains(gemini[0], "inline_data") || !strings.Contains(gemini[0], "Saat kaç?") ||
		!strings.Contains(gemini[0], "eski soru") {
		t.Fatalf("Gemini'ye giden istek: ses yerine metin + geçmiş olmalı: %s", gemini)
	}
	surecBas = time.Now().Add(-sesliCikisSiniri + 5*time.Second) // 5 sn kaldı
	if _, _, _, ok := assemblyYedek(context.Background(), wav, ist, []string{"m1"}, "G"); ok {
		t.Fatal("süre yetmezken yedek çalışmamalı")
	}
}

func TestAnahtarAssembly(t *testing.T) {
	d := t.TempDir()
	eski := assemblyAnahtarYol
	assemblyAnahtarYol = filepath.Join(d, "assembly")
	t.Cleanup(func() { assemblyAnahtarYol = eski })
	c := anahtarKomutu([]string{"assembly", asmAnahtar})
	b, err := os.ReadFile(assemblyAnahtarYol)
	if err != nil || strings.TrimSpace(string(b)) != asmAnahtar || !strings.Contains(c, "assembly") {
		t.Fatalf("kayıt: %q %v", c, err)
	}
	if st, _ := os.Stat(assemblyAnahtarYol); st.Mode().Perm() != 0600 {
		t.Fatalf("izin %v", st.Mode().Perm())
	}
	if !strings.Contains(anahtarKomutu(nil), "assembly") {
		t.Fatal("listede assembly görünmeli")
	}
	if !strings.Contains(anahtarKomutu([]string{"sil", "assembly"}), "silindi") || dosyaVar(assemblyAnahtarYol) {
		t.Fatal("silme")
	}
}
