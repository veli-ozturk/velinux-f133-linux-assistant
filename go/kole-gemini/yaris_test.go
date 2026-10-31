package main

import (
	"context"
	"io"
	"net/http"
	"net/http/httptest"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

const yarisCevap = `{"candidates":[{"content":{"parts":[{"text":"{\"soru\":\"s\",\"cevap\":\"c\"}"}]}}]}`

func yarisSunucu(t *testing.T) *httptest.Server {
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		govde, _ := io.ReadAll(r.Body)
		model := strings.TrimSuffix(strings.TrimPrefix(r.URL.Path, "/"), ":generateContent")
		switch model {
		case "m503":
			w.WriteHeader(503)
			_, _ = w.Write([]byte(`{"error":{"message":"high demand"}}`))
			return
		case "m429":
			w.WriteHeader(429)
			_, _ = w.Write([]byte(`{"error":{"message":"quota"}}`))
			return
		case "myavas":
			select {
			case <-time.After(1500 * time.Millisecond):
			case <-r.Context().Done():
				return
			}
		case "mthink":
			if strings.Contains(string(govde), "thinkingConfig") {
				w.WriteHeader(400)
				_, _ = w.Write([]byte(`{"error":{"message":"Thinking is not supported by this model."}}`))
				return
			}
		}
		_, _ = w.Write([]byte(yarisCevap))
	}))
	eski := llmURLTaban
	llmURLTaban = srv.URL + "/"
	sogumaYol = filepath.Join(t.TempDir(), "s.json")
	t.Cleanup(func() { srv.Close(); llmURLTaban = eski })
	return srv
}

func yarisIstek() istek {
	return istek{GenerationConfig: map[string]any{"temperature": 0.7, "thinkingConfig": map[string]any{"thinkingLevel": "low"}}}
}

func yarisCalistir(t *testing.T, sure, yedek time.Duration, modeller ...string) (yanit, string, int, time.Duration) {
	ctx, iptal := context.WithTimeout(context.Background(), sure)
	defer iptal()
	t0 := time.Now()
	y, m, k := llmYaris(ctx, modeller, yarisIstek(), "K", yedek, 2)
	return y, m, k, time.Since(t0)
}

func TestYaris503SonraHizli(t *testing.T) {
	yarisSunucu(t)
	y, m, k, s := yarisCalistir(t, 5*time.Second, 5*time.Second, "m503", "mhizli")
	if k != yarisTamam || m != "mhizli" || len(y.Candidates) != 1 || s > time.Second {
		t.Fatalf("503 sonra hemen sonraki model: %d %s %v", k, m, s)
	}
	if v, ok := sogumaOku()["llm:m503"]; !ok || v.Neden != "503" {
		t.Fatalf("503 sogumasi yok: %v", sogumaOku())
	} else if kalan := time.Until(time.Unix(v.Kadar, 0)); kalan < 14*time.Minute { // v31: 15 dk
		t.Fatalf("503 sogumasi kisa: %v", kalan)
	}
}

func TestYarisYavasYaninaYedek(t *testing.T) {
	yarisSunucu(t)
	_, m, k, s := yarisCalistir(t, 5*time.Second, 200*time.Millisecond, "myavas", "mhizli")
	if k != yarisTamam || m != "mhizli" || s > time.Second {
		t.Fatalf("yavasin yanina yedek kazanmali: %d %s %v", k, m, s)
	}
	if v := sogumaOku()["llm:myavas"]; v.Neden != "yavas" {
		t.Fatalf("yavas sogumasi yok: %v", sogumaOku())
	}
}

func TestYarisYavasTekBasina(t *testing.T) {
	yarisSunucu(t)
	_, m, k, _ := yarisCalistir(t, 5*time.Second, 200*time.Millisecond, "myavas")
	if k != yarisTamam || m != "myavas" {
		t.Fatalf("yedek yoksa yavas model beklenir: %d %s", k, m)
	}
}

func TestYarisDusunce400(t *testing.T) {
	yarisSunucu(t)
	_, m, k, _ := yarisCalistir(t, 5*time.Second, 5*time.Second, "mthink")
	if k != yarisTamam || m != "mthink" {
		t.Fatalf("dusunme ayarsiz yeniden deneme: %d %s", k, m)
	}
}

func TestYarisHepsiHata(t *testing.T) {
	yarisSunucu(t)
	_, _, k, s := yarisCalistir(t, 5*time.Second, 5*time.Second, "m503", "m429")
	if k != yarisYok || s > time.Second {
		t.Fatalf("hepsi hata: %d %v", k, s)
	}
}

func TestYarisGenelSure(t *testing.T) {
	yarisSunucu(t)
	_, _, k, s := yarisCalistir(t, 300*time.Millisecond, 5*time.Second, "myavas", "mhizli")
	if k != yarisZaman || s > time.Second {
		t.Fatalf("genel sure: %d %v", k, s)
	}
}

func TestYarisAgHatasi(t *testing.T) {
	sogumaYol = filepath.Join(t.TempDir(), "s.json")
	eski := llmURLTaban
	llmURLTaban = "http://127.0.0.1:1/"
	defer func() { llmURLTaban = eski }()
	_, _, k, s := yarisCalistir(t, 5*time.Second, 5*time.Second, "a", "b", "c")
	if k != yarisAg || s > 2*time.Second {
		t.Fatalf("ag hatasi: %d %v", k, s)
	}
}

func TestSesModelleriVirgullu(t *testing.T) {
	m := sesModelleri(sesModelVarsayilan, []string{"gemini-flash-lite-latest", "gemini-3.8-flash", "gemini-2.5-flash"})
	if strings.Join(m, ",") != "gemini-3.5-flash-lite,gemini-flash-lite-latest,gemini-3.8-flash,gemini-2.5-flash" { // v31
		t.Fatalf("sesli sira: %v", m)
	}
}
