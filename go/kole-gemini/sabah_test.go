// sabah_test.go (15.10.2026, Claude): giyim önerisi, hava uyarıları, şehir haber süzgeci, Türkçe küçük harf.
package main

import (
	"strings"
	"testing"
)

func TestGiyim(t *testing.T) {
	durumlar := []struct {
		g    gunHava
		var_ []string
		yok  []string
	}{
		{gunHava{HisMax: 33, HisMin: 24, UV: 8}, []string{"ince, açık renkli", "güneş kremi"}, []string{"şemsiye", "mont"}},
		{gunHava{HisMax: 15, HisMin: 3, YagisOlas: 70, YagisMM: 4}, []string{"kazak", "katmanlı", "şemsiye alın"}, []string{"su geçirmez"}},
		{gunHava{HisMax: 1, HisMin: -6, Kod: 73, YagisMM: 12}, []string{"kalın kaban", "kaymaz tabanlı bot", "su geçirmez"}, nil},
		{gunHava{HisMax: 20, HisMin: 14, Hamle: 75}, []string{"hafif bir ceket", "rüzgarlık"}, []string{"katmanlı"}},
	}
	for i, d := range durumlar {
		s := giyimOnerisi(d.g)
		for _, v := range d.var_ {
			if !strings.Contains(s, v) {
				t.Errorf("%d: %q yok: %s", i, v, s)
			}
		}
		for _, v := range d.yok {
			if strings.Contains(s, v) {
				t.Errorf("%d: %q olmamaliydi: %s", i, v, s)
			}
		}
	}
}

func TestHavaUyarilari(t *testing.T) {
	if u := havaUyarilari(gunHava{HisMax: 20, HisMin: 10, Max: 21, Min: 11}); len(u) != 0 {
		t.Fatalf("sakin gunde uyari: %v", u)
	}
	u := strings.Join(havaUyarilari(gunHava{Kod: 96, Hamle: 80, YagisMM: 35, Min: 5, HisMax: 25}), "|")
	for _, v := range []string{"fırtına", "kuvvetli rüzgar", "kuvvetli yağış"} {
		if !strings.Contains(u, v) {
			t.Errorf("%q yok: %s", v, u)
		}
	}
	u = strings.Join(havaUyarilari(gunHava{Kod: 75, Min: -3, YagisMM: 5, HisMin: -12, HisMax: -2}), "|")
	for _, v := range []string{"kar", "buzlanma", "aşırı soğuk"} {
		if !strings.Contains(u, v) {
			t.Errorf("%q yok: %s", v, u)
		}
	}
}

func TestSehirSuz(t *testing.T) {
	b := []string{"16 EKİM İSTANBUL'DA OKULLAR TATİL Mİ?", "Ankara'da yağış", "İstanbul'da sağanak", "İstanbul'da sağanak",
		"İzmir haberi", "istanbul trafiği", "Kastamonu"}
	o := sehirSuz(b, "İstanbul", 5)
	// v10: BÜYÜK HARF SEO başlığı (b[0]) artık elenir
	if len(o) != 2 || o[0] != "İstanbul'da sağanak" || o[1] != "istanbul trafiği" {
		t.Fatalf("istanbul: %v", o)
	}
	if o := sehirSuz(b, "Ankara", 5); len(o) != 1 {
		t.Fatalf("ankara: %v", o)
	}
	if o := sehirSuz(b, "", 5); o != nil {
		t.Fatal("bos sehir")
	}
	if trKucuk("IŞIK İZMİR") != "ışık izmir" {
		t.Fatal(trKucuk("IŞIK İZMİR"))
	}
}
