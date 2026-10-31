package main

import (
	"path/filepath"
	"testing"
	"time"
)

func TestButce(t *testing.T) {
	eskiBas, eskiSinir := surecBas, cikisSiniri
	defer func() { surecBas, cikisSiniri = eskiBas, eskiSinir }()
	cikisSiniri = sesliCikisSiniri
	surecBas = time.Now()
	if b := butce(15 * time.Second); b != 15*time.Second {
		t.Fatalf("basta azami: %v", b)
	}
	surecBas = time.Now().Add(-17 * time.Second) // LLM 17 sn surdu
	if b := butce(15 * time.Second); b < 9*time.Second || b > 10*time.Second {
		t.Fatalf("27-17 ~ 10 sn: %v", b)
	}
	surecBas = time.Now().Add(-30 * time.Second)
	if b := butce(8 * time.Second); b != 0 {
		t.Fatalf("sure bitti: %v", b)
	}
	cikisSiniri = metinCikisSiniri
	if b := butce(15 * time.Second); b < 11*time.Second || b > 12*time.Second {
		t.Fatalf("metin 42-30 ~ 12 sn: %v", b)
	}
}

func TestButceSifirdaKilitVeAkis(t *testing.T) {
	d := t.TempDir()
	eskiS, eskiH := ttsSureYol, akisHataYol
	defer func() { ttsSureYol, akisHataYol = eskiS, eskiH }()
	ttsSureYol, akisHataYol = filepath.Join(d, "sure"), filepath.Join(d, "hata")
	t0 := time.Now()
	if akisHazirBekle(0) {
		t.Fatal("0 surede akis hazir sayilmamali")
	}
	if time.Since(t0) > 100*time.Millisecond {
		t.Fatalf("0 surede beklememeli: %v", time.Since(t0))
	}
}
