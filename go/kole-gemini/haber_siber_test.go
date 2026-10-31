package main

// haber_siber_test.go (21.10.2026, Claude; v29, scripts/490)

import (
	"strings"
	"testing"
	"time"
)

func TestHaberKatCozSiber(t *testing.T) {
	for _, g := range []string{"siber", "Siber", "güvenlik", "guvenlik", "siber güvenlik", "hack"} {
		if k, ok := haberKatCoz(g); !ok || k != katSiber {
			t.Fatalf("%q -> %q %v (siber bekleniyordu)", g, k, ok)
		}
	}
}

func TestHaberSiberKaynaklar(t *testing.T) {
	n := 0
	for _, k := range haberKaynaklari {
		if k.Kat == katSiber {
			n++
			if !strings.HasPrefix(k.URL, "https://") {
				t.Fatalf("siber kaynak https değil: %+v", k)
			}
		}
	}
	if n != 5 {
		t.Fatalf("siber kaynak sayısı %d (5 bekleniyordu)", n)
	}
	for ad := range siberPanoKaynak {
		var var_ bool
		for _, k := range haberKaynaklari {
			var_ = var_ || (k.Ad == ad && k.Kat == katSiber)
		}
		if !var_ {
			t.Fatalf("Pano kaynağı %s listede yok", ad)
		}
	}
	if haberKategoriAd[katSiber] == "" {
		t.Fatal("siber kategori adı yok")
	}
	var sirada bool
	for _, k := range haberKategoriSira {
		sirada = sirada || k == katSiber
	}
	if !sirada {
		t.Fatal("siber kategori bülten sırasında yok")
	}
}

func TestHaberKumeleSiberOncelik(t *testing.T) {
	simdi := time.Date(2026, 10, 21, 22, 0, 0, 0, time.UTC)
	oge := []haberOge{
		{"Fidye yazılımı saldırısı hastane sistemlerini durdurdu", "ShiftDelete", katSiber, simdi.Add(-time.Hour)},
		{"Fidye yazılımı saldırısı hastane sistemlerini durdurdu iddiası", "Webtekno", katTeknoloji, simdi.Add(-2 * time.Hour)},
		{"Bakanlık siber saldırı açıklaması yaptı veri sızıntısı yok", "AA", katGundem, simdi.Add(-time.Hour)},
		{"Bakanlık siber saldırı açıklaması yaptı veri sızıntısı yok dedi", "BleepingComputer", katSiber, simdi.Add(-time.Hour)},
		{"Chrome tarayıcısında kritik açık kapatıldı güncelleme yayınlandı", "SecurityWeek", katSiber, simdi.Add(-3 * time.Hour)},
	}
	k := haberKumele(oge, simdi)
	kat := map[string]string{}
	for _, c := range k {
		kat[c.Baslik] = c.Kat
	}
	if kat["Fidye yazılımı saldırısı hastane sistemlerini durdurdu"] != katSiber {
		t.Fatalf("teknoloji + siber aynı olay siberde olmalı: %v", kat)
	}
	for b, kk := range kat {
		if strings.HasPrefix(b, "Bakanlık") && kk != katGundem {
			t.Fatalf("gündem + siber aynı olay gündemde olmalı: %v", kat)
		}
	}
	b := haberBulteniYaz(k, 0, 8, "")
	if !strings.Contains(b, haberKategoriAd[katSiber]+":") || !strings.Contains(b, "Chrome") {
		t.Fatal("bültende siber bölümü yok:\n" + b)
	}
	if s := haberBulteniYaz(k, 0, 8, katSiber); strings.Contains(s, "Bakanlık") || !strings.Contains(s, "Fidye") {
		t.Fatal("yalnız siber bülteni yanlış:\n" + s)
	}
}

func TestPanoHaberBirlestir(t *testing.T) {
	g := []string{"g1", "g2", "g3", "g4", "g5", "g6", "g7", "g8"}
	o := panoHaberBirlestir(g, []string{" s1 ", "s2"}, 8)
	if len(o) != 8 || o[5] != "g6" || o[6] != "Siber: s1" || o[7] != "Siber: s2" {
		t.Fatalf("8 genel + 2 siber: %q", o)
	}
	if o := panoHaberBirlestir(g, nil, 8); len(o) != 8 || o[7] != "g8" {
		t.Fatalf("siber yokken genel 8: %q", o)
	}
	if o := panoHaberBirlestir(g[:3], []string{"s1", "s2"}, 8); len(o) != 5 || o[3] != "Siber: s1" {
		t.Fatalf("3 genel + 2 siber: %q", o)
	}
	if o := panoHaberBirlestir(nil, []string{"s1"}, 8); len(o) != 1 || o[0] != "Siber: s1" {
		t.Fatalf("genel yokken yalnız siber: %q", o)
	}
	if o := panoHaberBirlestir(g, []string{"a", "b", "c", "d", "e", "f", "g", "h", "i"}, 8); len(o) != 8 || o[0] != "Siber: a" {
		t.Fatalf("siber n'den çok: %q", o)
	}
}
