package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestSesModelleri(t *testing.T) {
	m := sesModelleri("gemini-3.8-flash", []string{"gemini-flash-lite-latest", " gemini-3.8-flash", "gemini-flash-latest", ""})
	if strings.Join(m, ",") != "gemini-3.8-flash,gemini-flash-lite-latest,gemini-flash-latest" {
		t.Fatalf("%v", m)
	}
}

func TestPcmHazirlaSure(t *testing.T) {
	x := make([]int16, 16000*2)
	x[0] = 7
	b := pcmHazirla(x)
	if len(b) != 2*(16000+32000+3200) {
		t.Fatalf("uzunluk %d", len(b))
	}
	for _, v := range b[:32000] {
		if v != 0 {
			t.Fatal("on sessizlik bos degil")
		}
	}
	if b[32000] != 7 {
		t.Fatal("ses on sessizlikten sonra baslamali")
	}
	if s := sureMetni(x); s != "on=1000 sure=2000\n" {
		t.Fatalf("sure metni %q", s)
	}
}

func TestSelamSil(t *testing.T) {
	d := t.TempDir()
	selamPCMYol, selamMetinYol, selamSesYol = filepath.Join(d, "p"), filepath.Join(d, "m"), filepath.Join(d, "s")
	for _, y := range []string{selamPCMYol, selamMetinYol, selamSesYol} {
		_ = os.WriteFile(y, []byte("x"), 0644)
	}
	selamSil()
	for _, y := range []string{selamPCMYol, selamMetinYol, selamSesYol} {
		if _, err := os.Stat(y); err == nil {
			t.Fatal("silinmedi: " + y)
		}
	}
}
