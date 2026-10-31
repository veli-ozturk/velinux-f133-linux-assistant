// pano_haber_test.go (22.10.2026, Claude; v32): haber önbelleği, tazelik, bağlam, haberCumlesi/siberCumlesi önbellekten.
package main

import (
	"context"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

func TestHaberOnbellek(t *testing.T) {
	eski := haberOnbellekYol
	haberOnbellekYol = filepath.Join(t.TempDir(), "h.json")
	defer func() { haberOnbellekYol = eski }()
	simdi := time.Now()
	if _, _, _, ok := haberOnbellekOku(simdi); ok {
		t.Fatal("dosya yokken ok")
	}
	if haberOnbellekYaz(nil, nil, simdi); dosyaVar(haberOnbellekYol) {
		t.Fatal("boş liste yazılmamalı")
	}
	g := []string{"G1", "G2?", "G3", "G4", "G5", "G6", "G7", "G8", "G9"}
	s := []string{"S1", "S2", "S3", "S4", "S5", "S6"}
	haberOnbellekYaz(g, s, simdi.Add(-10*time.Minute))
	gg, ss, yas, ok := haberOnbellekOku(simdi)
	if !ok || len(gg) != 8 || len(ss) != 5 || yas < 9*time.Minute || yas > 11*time.Minute {
		t.Fatalf("oku: %v %d %d %v", ok, len(gg), len(ss), yas)
	}
	b := panoHaberBaglam(simdi)
	if !strings.Contains(b, "10 dk önce") || !strings.Contains(b, "G6") || strings.Contains(b, "G7") || !strings.Contains(b, "siber güvenlik: S1 | S2") {
		t.Fatalf("bağlam: %q", b)
	}
	if c, err := haberCumlesi(context.Background()); err != nil || c != "Öne çıkan başlıklar: G1. G2? G3." {
		t.Fatalf("haberCumlesi: %q %v", c, err)
	}
	if c, err := siberCumlesi(context.Background()); err != nil || c != "Siber güvenlikte öne çıkanlar: S1. S2. S3." {
		t.Fatalf("siberCumlesi: %q %v", c, err)
	}
	haberOnbellekYaz(g, s, simdi.Add(-2*time.Hour)) // bayat
	if _, _, _, ok := haberOnbellekOku(simdi); ok || panoHaberBaglam(simdi) != "" {
		t.Fatal("bayat önbellek kullanılmamalı")
	}
}
