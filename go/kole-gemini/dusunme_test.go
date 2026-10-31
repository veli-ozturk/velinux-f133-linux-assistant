package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestDusunme(t *testing.T) {
	dir := t.TempDir()
	eskiY, eskiG := dusunmeYol, asistanGunluk
	dusunmeYol, asistanGunluk = filepath.Join(dir, "dusunme"), filepath.Join(dir, "asistan.log")
	defer func() { dusunmeYol, asistanGunluk = eskiY, eskiG }()

	if dusunmeSeviyesi() != "low" {
		t.Fatal("dosya yokken varsayılan low olmalı")
	}
	if c := dusunmeKomutu(""); !strings.Contains(c, "dengeli (varsayılan)") || !strings.Contains(c, "/dusunme hizli") {
		t.Fatalf("durum metni: %q", c)
	}
	for _, d := range []struct{ arg, seviye string }{
		{"hizli", "minimal"}, {"MEDIUM", "medium"}, {"dengeli", "low"}, {"minimal", "minimal"}, {"low", "low"},
	} {
		if c := dusunmeKomutu(d.arg); !strings.HasPrefix(c, "Tamam efendim") {
			t.Fatalf("%s: %q", d.arg, c)
		}
		if s := dusunmeSeviyesi(); s != d.seviye {
			t.Fatalf("%s -> %s (beklenen %s)", d.arg, s, d.seviye)
		}
	}
	if _, err := os.Stat(dusunmeYol); !os.IsNotExist(err) {
		t.Fatal("dengeli/low dosyayı silmeli (eski sürümle aynı davranış)")
	}
	if c := dusunmeKomutu("cokhizli"); !strings.Contains(c, "bilmiyorum") || dusunmeSeviyesi() != "low" {
		t.Fatalf("bilinmeyen seviye: %q", c)
	}
	_ = os.WriteFile(dusunmeYol, []byte("high\n"), 0644) // elle yazılmış desteklenmeyen değer -> low
	if dusunmeSeviyesi() != "low" {
		t.Fatal("listede olmayan değer low'a düşmeli")
	}

	if c := hizKomutu(); !strings.Contains(c, "günlüğü yok") {
		t.Fatalf("günlük yok: %q", c)
	}
	var g strings.Builder
	g.WriteString("soru bitti (sessizlik): 2.1 sn\n")
	for i := 0; i < 10; i++ {
		g.WriteString("model gemini-3.8-flash, " + strings.Repeat("9", i+1) + " ms (dusunme low)\n")
	}
	_ = os.WriteFile(asistanGunluk, []byte(g.String()), 0644)
	c := hizKomutu()
	if strings.Count(c, "gemini-3.8-flash") != 8 || strings.Contains(c, ", 9 ms") || !strings.Contains(c, "9999999999 ms") {
		t.Fatalf("son 8 satır: %q", c)
	}
}
