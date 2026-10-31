package main

import (
	"context"
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

func TestJpBilgi(t *testing.T) {
	d := t.TempDir()
	eu, ea, eo := jpBilgiURL, jpAnahtarYol, jpOnbellekYol
	defer func() { jpBilgiURL, jpAnahtarYol, jpOnbellekYol = eu, ea, eo }()
	jpAnahtarYol, jpOnbellekYol = filepath.Join(d, "anahtar"), filepath.Join(d, "bilgi.txt")
	durum := http.StatusOK
	srv := httptest.NewServer(http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if c, err := r.Cookie("jp26"); err != nil || c.Value != "gizli" {
			w.WriteHeader(http.StatusUnauthorized)
			return
		}
		w.WriteHeader(durum)
		_, _ = w.Write([]byte("Otel: Shinjuku\n"))
	}))
	defer srv.Close()
	jpBilgiURL = srv.URL
	ctx := context.Background()
	if _, _, err := jpBilgi(ctx); err == nil || strings.Contains(err.Error(), "gizli") {
		t.Fatal("anahtar yokken hata beklenir")
	}
	_ = os.WriteFile(jpAnahtarYol, []byte("gizli\n"), 0600)
	m, eski, err := jpBilgi(ctx)
	if err != nil || eski || !strings.Contains(m, "Shinjuku") {
		t.Fatalf("ilk çekme: %q %v %v", m, eski, err)
	}
	durum = http.StatusInternalServerError // önbellek taze: siteye gidilmez
	if m, eski, err = jpBilgi(ctx); err != nil || eski {
		t.Fatalf("taze önbellek: %v %v", eski, err)
	}
	eskiZaman := time.Now().Add(-2 * jpTaze)
	_ = os.Chtimes(jpOnbellekYol, eskiZaman, eskiZaman)
	if m, eski, err = jpBilgi(ctx); err != nil || !eski || !strings.Contains(m, "Shinjuku") {
		t.Fatalf("site hatasında eski önbellek: %v %v", eski, err)
	}
	if s := jpKes(strings.Repeat("ç", 10), 5); s != "çççç…" {
		t.Fatalf("jpKes %q", s)
	}
	if !strings.Contains(jpKomut(ctx, ""), "/jp") {
		t.Fatal("boş soru kullanım metni")
	}
}
